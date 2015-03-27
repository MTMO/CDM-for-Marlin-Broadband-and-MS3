/*
 * (c) 2015 - Copyright Marlin Trust Management Organization
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "Cenc"
#include "MarlinLog.h"

#include <cstring>

#include "Cenc.h"
#include "MarlinAgentHandler.h"

using namespace marlincdm;

const u_int8_t default_pssh[40] = {
    0x00, 0x00, 0x00, 0x28, // Box Size (40-bytes)
    'p',  's',  's',  'h',  // Box Type 'pssh'
    0x00, 0x00, 0x00, 0x00, // version and flags
    0x69, 0xF9, 0x08, 0xAF, 0x48, 0x16, 0x46, 0xEA, // PSSH SystemID
    0x91, 0x0C, 0xCD, 0x5D, 0xCC, 0xCB, 0x0A, 0x3A, // PSSH SystemID
    0x00, 0x00, 0x00, 0x08, // PSSH DataSize
    0x00, 0x00, 0x00, 0x00, // Size of 'marl'
    'm',  'a',  'r',  'l',  // Empty Marl
};

Cenc::Cenc() {
    init(default_pssh, sizeof(default_pssh), NULL);
}

Cenc::Cenc(const Cenc &other) {
    if(!other.mKidList.empty()) {
        init(other.mPsshData, other.mPsshSize, other.mKidList, other.mContentId);
    } else {
        init(other.mPsshData, other.mPsshSize, other.mContentId);
    }
}

Cenc::Cenc(const string cid) {
    init(default_pssh, sizeof(default_pssh), cid);
}

Cenc::Cenc(const u_int8_t *psshData, size_t psshSize, const string cid) {
    init(psshData, psshSize, cid);
}

Cenc::Cenc(const uint8_t *psshData, size_t psshSize, vector<uint8_t*> kidList, const string cid) {
    init(psshData, psshSize, kidList, cid);
}

void Cenc::init(const u_int8_t *psshData, size_t psshSize, const string cid) {
    MARLINLOG_ENTER();
    mMediaFormat = FORMAT_CENC;
    mDecryptMode = MODE_AES_CTR;
    mPsshSize = psshSize;
    mPsshData = new u_int8_t[psshSize];
    memcpy(mPsshData, psshData, psshSize);
    mParser = new ContentDataParser(psshData, psshSize);
    mContentId.assign(cid);
    MARLINLOG_EXIT();
}

void Cenc::init(const u_int8_t *psshData, size_t psshSize, vector<uint8_t*> kidList, const string cid) {
    MARLINLOG_ENTER();
    mMediaFormat = FORMAT_CENC;
    mDecryptMode = MODE_AES_CTR;
    mPsshSize = psshSize;
    mPsshData = new u_int8_t[psshSize];
    memcpy(mPsshData, psshData, psshSize);
    mParser = new ContentDataParser(psshData, psshSize);
    mContentId.assign(cid);

    uint8_t* p = NULL;
    if (!kidList.empty()) {
        for (uint32_t i = 0; i < kidList.size(); i++) {
            p = new uint8_t[16];
            memcpy(p, (uint8_t*)kidList.at(i), 16);
            mKidList.push_back(p);
        }
    }
    MARLINLOG_EXIT();
}

Cenc::~Cenc() {
    if (mPsshData != NULL) {
        delete[] mPsshData;
        mPsshData = NULL;
    }
    if (!mKidList.empty()) {
        vector<uint8_t*>::iterator itr = mKidList.begin();
        uint8_t *p = NULL;
        while (itr != mKidList.end()) {
            p = (*itr);
            delete[] p;
            p = NULL;
            itr++;
        }
    }
}

Cenc* Cenc::duplicate() {
    return new Cenc(*this);
}

mcdm_status_t Cenc::getCid(const uint8_t kid[16], string &cid) {
    MARLINLOG_ENTER();
    MarlinAgentHandler handler;
    MH_cid_t cid_;
    MH_status_t ret = MH_ERR_UNKNOWN;
    MH_memdata_t meta = {(MH_size_t)mParser->getData(FIRST_INDEX).len, mParser->getData(FIRST_INDEX).data};

    if (!mContentId.empty()) {
        cid = mContentId;
        return OK;
    }

    if (meta.bufdata == NULL || meta.length == 0) {
        meta.bufdata = mPsshData;
        meta.length = mPsshSize;
    }
    if (!mKidList.empty()) {
        kid = mKidList.front();
    }

    handler.initAgent(NULL);
    ret = handler.getContentIdByPssh(meta, kid, &cid_);
    if (ret != MH_ERR_OK) {
        LOGE("Couldn't get CID from PSSH: %d", ret);
        handler.finAgent(NULL);
        return ERROR_ILLEGAL_ARGUMENT;
    }
    handler.finAgent(NULL);

    if (cid_.empty()) {
        LOGE("Get empty CID from PSSH");
        return ERROR_UNKNOWN;
    }
    cid = cid_;
    MARLINLOG_EXIT();
    return OK;
}

mcdm_status_t Cenc::getCidWithIndex(const uint32_t index, string &cid) {
    MARLINLOG_ENTER();
    MarlinAgentHandler handler;
    uint8_t* kid = NULL;
    MH_cid_t cid_;
    MH_status_t parse_ret = MH_ERR_UNKNOWN;
    MH_memdata_t meta = {0, NULL};

    meta.length = (MH_size_t)mParser->getData(index).len;
    meta.bufdata = mParser->getData(index).data;
    if (meta.bufdata == NULL || meta.length == 0) {
        meta.bufdata = mPsshData;
        meta.length = mPsshSize;
    }
    if (!mKidList.empty() && index < mKidList.size()) {
        kid = mKidList.at(index);
    }

    handler.initAgent(NULL);
    parse_ret = handler.getContentIdByPssh(meta, kid, &cid_);
    if (parse_ret != MH_ERR_OK) {
        LOGE("Couldn't get CID from PSSH: %d", parse_ret);
        handler.finAgent(NULL);
        return ERROR_ILLEGAL_ARGUMENT;
    }
    handler.finAgent(NULL);

    if (cid_.empty()) {
        LOGE("Get empty CID from PSSH");
        return ERROR_UNKNOWN;
    }
    cid = cid_;
    MARLINLOG_EXIT();
    return OK;
}

mcdm_status_t Cenc::getCids(vector<string>& /*cids*/) {
    return OK;
}

mcdm_status_t Cenc::getIV(string& /*iv*/) {
    return OK;
}

mcdm_status_t Cenc::getOctopusBundle(vector<string>& /*octopusBundle*/) {
    return OK;
}
