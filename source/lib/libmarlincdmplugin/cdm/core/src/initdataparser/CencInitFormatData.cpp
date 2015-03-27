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

#define LOG_TAG "InitDataParser"
#include "MarlinLog.h"

#include "CencInitFormatData.h"
#include "MarlinCdmUtils.h"
#include "json/json.h"

using namespace marlincdm;

#define KID_LENGTH 16

CencInitFormatData::CencInitFormatData() {
    MARLINLOG_ENTER();
    mSessionId.clear();
    mPssh.len = 0;
    mPssh.data = NULL;
    mKidList.clear();
    mLicenseEmbedded = false;
    mCID.clear();
}

CencInitFormatData::~CencInitFormatData() {
    MARLINLOG_ENTER();
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
    if (mPssh.data != NULL) {
        delete mPssh.data;
        mPssh.data = NULL;
    }
}

bool& CencInitFormatData::getLicenseEmbedded() {
    return mLicenseEmbedded;
}

mcdm_data_t& CencInitFormatData::getPssh() {
    return mPssh;
}

vector<uint8_t*>& CencInitFormatData::getKidList() {
    return mKidList;
}

string& CencInitFormatData::getContentId() {
    return mCID;
}

parse_status_t CencInitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    string psshBox("");
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, cenc, kids;

    json_ret = reader.parse(data, root);
    if (!json_ret || !root.isObject()) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES].isMember(KEY_CENC_DATA)){
        LOGE("nothing cenc value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES][KEY_CENC_DATA].isObject()){
        LOGE("cenc value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    cenc = root[KEY_PROPERTIES][KEY_CENC_DATA];

    if(cenc.isMember(KEY_LICENSE_EMBEDDED)
            || cenc[KEY_LICENSE_EMBEDDED].isBool()){
        mLicenseEmbedded = cenc[KEY_PSSH_DATA].asBool();
    }

    if(cenc.isMember(KEY_PSSH_DATA)
            || cenc[KEY_PSSH_DATA].isString()){
        psshBox = cenc[KEY_PSSH_DATA].asString();
        if (psshBox.length() > 0) {
            mPssh.data = new uint8_t[psshBox.length()/2];
            strToBin(psshBox.data(), mPssh.data, psshBox.length());
            mPssh.len = psshBox.length()/2;
        }
    }

    if(cenc.isMember(KEY_KIDS_DATA)
            || cenc[KEY_KIDS_DATA].isArray()){
        kids = cenc[KEY_KIDS_DATA];
        if(kids.size() > 0) {
            string strKid;
            uint8_t* kid;
            for (size_t i = 0; i < kids.size(); i++) {
                kid = (uint8_t *) new unsigned char[KID_LENGTH];
                strKid = kids[static_cast<unsigned int>(i)].asString();
                if (strKid.length() != KID_LENGTH*2) {
                    LOGE("kid lenght is illegal. kids[%d].length() : %d", i, strKid.length());
                    delete[] kid;
                    kid = NULL;
                    ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
                    goto EXIT;
                }
                strToBin(strKid.data(), kid, KID_LENGTH*2);
                mKidList.push_back(kid);
            }
        }
    }

    if(cenc.isMember(KEY_CONTENT_ID)
            || cenc[KEY_CONTENT_ID].isString()){
        mCID = cenc[KEY_CONTENT_ID].asString();
    }

    if(cenc.isMember(KEY_SESSION_ID_DATA)
            || cenc[KEY_SESSION_ID_DATA].isString()){
        mSessionId = cenc[KEY_SESSION_ID_DATA].asString();
    }
EXIT:
    MARLINLOG_EXIT();
    return ret;
}

parse_status_t CencInitFormatData::parseKeyIds(const string& data)
{
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;

    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, keyids;

    json_ret = reader.parse(data, root);
    if (!json_ret || !root.isObject()) {
        LOGD("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if((!root[KEY_PROPERTIES].isMember(KEY_KEYIDS_DATA))
            && root[KEY_PROPERTIES][KEY_KEYIDS_DATA].isArray()){
        LOGD("nothing keyids value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }

    keyids = root[KEY_PROPERTIES][KEY_KEYIDS_DATA];
    if(keyids.size() > 0) {
        string strKeyid;
        for (size_t i = 0; i < keyids.size(); i++) {
            strKeyid = keyids[static_cast<unsigned int>(i)].asString();
            mCID.assign(strKeyid);
        }
    }

EXIT:
    MARLINLOG_EXIT();
    return ret;
}
