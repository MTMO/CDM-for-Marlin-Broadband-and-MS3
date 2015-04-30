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

#define LOG_TAG "MS3InitFormatData"
#include "MarlinLog.h"

#include <cstring>
#include "json/json.h"

#include "MS3InitFormatData.h"
#include "MarlinConstants.h"

using namespace marlincdm;

MS3InitFormatData::MS3InitFormatData() {
    MARLINLOG_ENTER();
    mSuri.len = 0;
    mSuri.data = NULL;
    mOptionalParameters.clear();
}

MS3InitFormatData::~MS3InitFormatData() {
    MARLINLOG_ENTER();
    if (mSuri.data != NULL) {
        delete mSuri.data;
        mSuri.data = NULL;
    }
}

mcdm_data_t& MS3InitFormatData::getSuri() {
    return mSuri;
}

map<string, string>& MS3InitFormatData::getOptionalParameters() {
    return mOptionalParameters;
}

parse_status_t MS3InitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    string suri("");
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, ms3;

    json_ret = reader.parse(data, root);
    if (!json_ret || !root.isObject()) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES].isMember(KEY_MS3_DATA)){
        LOGE("nothing MPD value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES][KEY_MS3_DATA].isObject()){
        LOGE("MAS value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    ms3 = root[KEY_PROPERTIES][KEY_MS3_DATA];

    if(ms3.isMember(KEY_SURI_DATA)
            && ms3[KEY_SURI_DATA].isString()){
        suri = ms3[KEY_SURI_DATA].asString();
        if (suri.length() > 0) {
            mSuri.data = new uint8_t[suri.length()];
            memcpy(mSuri.data, reinterpret_cast<uint8_t*>(const_cast<char*>(suri.data())), suri.length());
            mSuri.len = suri.length();
        }
    }

EXIT:
    MARLINLOG_EXIT();
    return ret;
}
