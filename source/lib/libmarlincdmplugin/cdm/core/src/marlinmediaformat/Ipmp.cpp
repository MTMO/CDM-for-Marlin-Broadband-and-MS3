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

#define LOG_TAG "Ipmp"
#include "MarlinLog.h"

#include <cstring>

#include "Ipmp.h"
#include "MarlinAgentHandler.h"

using namespace marlincdm;

const u_int8_t default_sinf[24] = {
    0x00, 0x00, 0x00, 0x18, // Box Size (24-bytes)
    's',  'i',  'n',  'f',  // Box Type 'sinf'
    0x00, 0x00, 0x00, 0x00, // size of schm
    's',  'c',  'h',  'm',  // Empty schm
    0x00, 0x00, 0x00, 0x00, // size of schi
    's',  'c',  'h',  'i',  // Empty schi
};

Ipmp::Ipmp() {
    init(default_sinf, sizeof(default_sinf), "");
}

Ipmp::Ipmp(const Ipmp &other) {
    init(other.mSinfData, other.mSinfSize, other.mContentId);
}

Ipmp::Ipmp(const u_int8_t *sinfData, size_t sinfSize, string cid) {
    init(sinfData, sinfSize, cid);
}

void Ipmp::init(const u_int8_t *sinfData, size_t sinfSize, string cid) {
    MARLINLOG_ENTER();
    mMediaFormat = FORMAT_IPMP;
    mDecryptMode = MODE_AES_CBC;
    mSinfSize = sinfSize;
    mSinfData = new u_int8_t[sinfSize];
    memcpy(mSinfData, sinfData, sinfSize);
    mParser = new ContentDataParser(sinfData, sinfSize);
    mContentId = cid;
    MARLINLOG_EXIT();
}

Ipmp::~Ipmp() {
    if (mSinfData != NULL) {
        delete[] mSinfData;
        mSinfData = NULL;
    }
}

Ipmp* Ipmp::duplicate() {
    return new Ipmp(*this);
}

mcdm_status_t Ipmp::getCid(const u_int8_t []/*kid[16]*/, string &cid) {
    MARLINLOG_ENTER();
    MarlinAgentHandler handler;
    MH_cid_t cid_;
    MH_status_t parse_ret = MH_ERR_UNKNOWN;
    MH_memdata_t meta = {(MH_size_t)mParser->getData(FIRST_INDEX).len, mParser->getData(FIRST_INDEX).data};

    if (!mContentId.empty()) {
        cid = mContentId;
        return OK;
    }

    parse_ret = handler.getContentIdByIpmp(meta, &cid_);
    if (parse_ret != MH_ERR_OK) {
        LOGE("Couldn't get CID from IPMP: %d", parse_ret);
        return ERROR_ILLEGAL_ARGUMENT;
    }

    if (cid_.empty()) {
        LOGE("Get empty CID from IPMP");
        return ERROR_UNKNOWN;
    }
    cid = cid_;
    MARLINLOG_EXIT();
    return OK;
}

mcdm_status_t Ipmp::getCidWithIndex(const uint32_t index, string &cid) {
    MARLINLOG_ENTER();
    MarlinAgentHandler handler;
    MH_cid_t cid_;
    MH_status_t parse_ret = MH_ERR_UNKNOWN;
    MH_memdata_t meta = {0, NULL};

    meta.length = (MH_size_t)mParser->getData(index).len;
    meta.bufdata = mParser->getData(index).data;

    parse_ret = handler.getContentIdByIpmp(meta, &cid_);
    if (parse_ret != MH_ERR_OK) {
        LOGE("Couldn't get CID from IPMP: %d", parse_ret);
        return ERROR_ILLEGAL_ARGUMENT;
    }

    if (cid_.empty()) {
        LOGE("Get empty CID from IPMP");
        return ERROR_UNKNOWN;
    }
    cid = cid_;
    MARLINLOG_EXIT();
    return OK;
}

mcdm_status_t Ipmp::getCids(vector<string>& /*cids*/) {
    return OK;
}

mcdm_status_t Ipmp::getIV(string& /*iv*/) {
    return OK;
}

mcdm_status_t Ipmp::getOctopusBundle(vector<string>& /*octopusBundle*/) {
    return OK;
}
