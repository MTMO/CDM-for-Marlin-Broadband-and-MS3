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

#define LOG_TAG "AtknInitFormatData"
#include "MarlinLog.h"

#include <cstring>
#include "json/json.h"

#include "AtknInitFormatData.h"
#include "MarlinConstants.h"

using namespace marlincdm;

AtknInitFormatData::AtknInitFormatData() {
    MARLINLOG_ENTER();
    mMatd.len = 0;
    mMatd.data = NULL;
    mMatu.len = 0;
    mMatu.data = NULL;
    mServiceId.clear();
    mAccountId.clear();
    mSubscriptionId.clear();
    mOptionalParameters.clear();
}

AtknInitFormatData::~AtknInitFormatData() {
    MARLINLOG_ENTER();
    if (mMatd.data != NULL) {
        delete mMatd.data;
        mMatd.data = NULL;
    }
    if (mMatu.data != NULL) {
        delete mMatu.data;
        mMatu.data = NULL;
    }
}

mcdm_data_t& AtknInitFormatData::getMatd() {
    return mMatd;
}

mcdm_data_t& AtknInitFormatData::getMatu() {
    return mMatu;
}

INIT_DATA_STRING_TYPE& AtknInitFormatData::getServiceId() {
    return mServiceId;
}
INIT_DATA_STRING_TYPE& AtknInitFormatData::getAccountId() {
    return mAccountId;
}

INIT_DATA_STRING_TYPE& AtknInitFormatData::getSubscriptionId() {
    return mSubscriptionId;
}

map<string, string>& AtknInitFormatData::getOptionalParameters() {
    return mOptionalParameters;
}

parse_status_t AtknInitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    string matd("");
    string matu("");
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, atkn;

    json_ret = reader.parse(data, root);
    if (!json_ret || !root.isObject()) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES].isMember(KEY_ATKN_DATA)){
        LOGE("nothing actiontoken value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!root[KEY_PROPERTIES][KEY_ATKN_DATA].isObject()){
        LOGE("actiontoken value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    atkn = root[KEY_PROPERTIES][KEY_ATKN_DATA];

    if(atkn.isMember(KEY_MATD_DATA)
            && atkn[KEY_MATD_DATA].isString()){
        matd = atkn[KEY_MATD_DATA].asString();
        if (matd.length() > 0) {
            mMatd.data = new uint8_t[matd.length()];
            memcpy(mMatd.data, reinterpret_cast<uint8_t*>(const_cast<char*>(matd.data())), matd.length());
            mMatd.len = matd.length();
        }
    }

    if(atkn.isMember(KEY_MATU_DATA)
            && atkn[KEY_MATU_DATA].isString()){
        matu = atkn[KEY_MATU_DATA].asString();
        if (matu.length() > 0) {
            mMatu.data = new uint8_t[matu.length()];
            memcpy(mMatu.data, reinterpret_cast<uint8_t*>(const_cast<char*>(matu.data())), matu.length());
            mMatu.len = matu.length();
        }
    }

    if (atkn.isMember(KEY_OPTINAL_DATA)
            && atkn[KEY_OPTINAL_DATA].isArray()) {
        string name;
        string value;
        Json::Value optinal;
        for (size_t i = 0; i < atkn[KEY_OPTINAL_DATA].size(); i++) {
            optinal = atkn[KEY_OPTINAL_DATA][static_cast<unsigned int>(i)];
            if (optinal.isMember(KEY_OPTIONAL_ACCOUNT_ID)
                    && !optinal[KEY_OPTIONAL_ACCOUNT_ID].isNull()
                    && optinal[KEY_OPTIONAL_ACCOUNT_ID].isString()) {
                mAccountId = optinal[KEY_OPTIONAL_ACCOUNT_ID].asString();
            }
            if (optinal.isMember(KEY_OPTIONAL_SERVICE_ID)
                    && !optinal[KEY_OPTIONAL_SERVICE_ID].isNull()
                    && optinal[KEY_OPTIONAL_SERVICE_ID].isString()) {
                mServiceId = optinal[KEY_OPTIONAL_SERVICE_ID].asString();
            }
            if (optinal.isMember(KEY_OPTIONAL_SUBSCRIPTION_ID)
                    && !optinal[KEY_OPTIONAL_SUBSCRIPTION_ID].isNull()
                    && optinal[KEY_OPTIONAL_SUBSCRIPTION_ID].isString()) {
                mSubscriptionId = optinal[KEY_OPTIONAL_SUBSCRIPTION_ID].asString();
            }
            name.clear();
            value.clear();
            for( Json::ValueIterator itr = optinal.begin() ; itr != optinal.end() ; itr++ ) {
                if (!itr.key().isNull()
                        && itr.key().isString()) {
                    if (!KEY_OPTINAL_DATA_NAME.compare(itr.key().asString())) {
                        name = optinal[itr.key().asString()].asString();
                    }
                    if (!KEY_OPTINAL_DATA_VALUE.compare(itr.key().asString())) {
                        value = optinal[itr.key().asString()].asString();
                    }
                }
            }
            if (!name.empty() && !value.empty()) {
                mOptionalParameters[name] = value;
            }
        }
    }

EXIT:
    MARLINLOG_EXIT();
    return ret;
}
