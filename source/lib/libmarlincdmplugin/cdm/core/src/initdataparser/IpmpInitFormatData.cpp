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

#define LOG_TAG "IpmpInitFormatData"
#include "MarlinLog.h"

#include "IpmpInitFormatData.h"
#include "MarlinCdmUtils.h"
#include "json/json.h"

using namespace marlincdm;

IpmpInitFormatData::IpmpInitFormatData() {
    MARLINLOG_ENTER();
    mSinf.len = 0;
    mSinf.data = NULL;
    mCID.clear();
}

IpmpInitFormatData::~IpmpInitFormatData() {
    if (mSinf.data != NULL) {
        delete mSinf.data;
        mSinf.data = NULL;
    }
}

mcdm_data_t& IpmpInitFormatData::getSinf() {
    return mSinf;
}

string& IpmpInitFormatData::getContentId() {
    return mCID;
}

parse_status_t IpmpInitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    string sinfBox("");
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, ipmp;

    json_ret = reader.parse(data, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES].isMember(KEY_IPMP_DATA)){
        LOGE("nothing ipmp value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES][KEY_IPMP_DATA].isObject()){
        LOGE("ipmp value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    ipmp = root[KEY_PROPERTIES][KEY_IPMP_DATA];

    if(ipmp.isMember(KEY_SINF_DATA)
            || ipmp[KEY_SINF_DATA].isString()){
        sinfBox = ipmp[KEY_SINF_DATA].asString();
        if (sinfBox.length() > 0) {
            mSinf.data = new uint8_t[sinfBox.length()/2];
            strToBin(sinfBox.data(), mSinf.data, sinfBox.length());
            mSinf.len = sinfBox.length()/2;
        }
    }

    if(ipmp.isMember(KEY_CONTENT_ID)
            || ipmp[KEY_CONTENT_ID].isString()){
        mCID = ipmp[KEY_CONTENT_ID].asString();
    }
EXIT:
    MARLINLOG_EXIT();
    return ret;
}
