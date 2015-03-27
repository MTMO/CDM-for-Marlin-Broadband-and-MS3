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

#define LOG_TAG "LTPInitFormatData"
#include "MarlinLog.h"

#include "LTPInitFormatData.h"
#include "MarlinCdmUtils.h"
#include "json/json.h"

using namespace marlincdm;

LTPInitFormatData::LTPInitFormatData() {
    MARLINLOG_ENTER();
    mLTPMessage.len = 0;
    mLTPMessage.data = NULL;
}

LTPInitFormatData::~LTPInitFormatData() {
    if (mLTPMessage.data != NULL) {
        delete mLTPMessage.data;
        mLTPMessage.data = NULL;
    }
}

mcdm_data_t& LTPInitFormatData::getMessage() {
    return mLTPMessage;
}

parse_status_t LTPInitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    string message("");
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, ltp;

    json_ret = reader.parse(data, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES].isMember(KEY_LTP_DATA)){
        LOGE("nothing cenc value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES][KEY_LTP_DATA].isObject()){
        LOGE("cenc value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    ltp = root[KEY_PROPERTIES][KEY_LTP_DATA];

    if(ltp.isMember(KEY_LTP_MESSAGE_DATA)
            || ltp[KEY_LTP_MESSAGE_DATA].isString()){
        message = ltp[KEY_LTP_MESSAGE_DATA].asString();
        if (message.length() > 0) {
            mLTPMessage.data = new uint8_t[message.length()/2];
            strToBin(message.data(), mLTPMessage.data, message.length());
            mLTPMessage.len = message.length()/2;
        }
    }
EXIT:
    MARLINLOG_EXIT();
    return ret;
}
