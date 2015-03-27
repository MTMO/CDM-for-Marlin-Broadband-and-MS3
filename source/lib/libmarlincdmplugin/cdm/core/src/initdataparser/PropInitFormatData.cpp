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

#define LOG_TAG "PropInitFormatData"
#include "MarlinLog.h"

#include "MarlinConstants.h"
#include "PropInitFormatData.h"
#include "MarlinCdmUtils.h"
#include "json/json.h"

using namespace marlincdm;

PropInitFormatData::PropInitFormatData() {
    MARLINLOG_ENTER();
    mPropName.clear();
    mServiceId.clear();
    mAccountId.clear();
    mSubscriptionId.clear();
    mOptionalParameters.clear();
}

PropInitFormatData::~PropInitFormatData() {
}

QUERY_PROPERTY_NAME_TYPE PropInitFormatData::getName() {
    return mPropName;
}

INIT_DATA_STRING_TYPE& PropInitFormatData::getServiceId() {
    return mServiceId;
}
INIT_DATA_STRING_TYPE& PropInitFormatData::getAccountId() {
    return mAccountId;
}

INIT_DATA_STRING_TYPE& PropInitFormatData::getSubscriptionId() {
    return mSubscriptionId;
}

map<string, string>& PropInitFormatData::getOptionalParameters() {
    return mOptionalParameters;
}

parse_status_t PropInitFormatData::parse(const string& data) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, properties, propData;

    json_ret = reader.parse(data, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    properties = root[KEY_PROPERTIES];

    if(!properties.isMember(KEY_PROP_DATA)){
        LOGE("nothing Prop value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!properties[KEY_PROP_DATA].isObject()){
        LOGE("Prop value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }

    propData = properties[KEY_PROP_DATA];
    if(!propData.isMember(KEY_PROP_DATA_NAME)
            || !propData[KEY_PROP_DATA_NAME].isString()){
        LOGE("nothing name value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    mPropName = propData[KEY_PROP_DATA_NAME].asString();

    if (propData.isMember(KEY_OPTINAL_DATA)
            && propData[KEY_OPTINAL_DATA].isArray()) {
        Json::Value optinal;
        string name;
        string value;
        for (size_t i = 0; i < propData[KEY_OPTINAL_DATA].size(); i++) {
            optinal = propData[KEY_OPTINAL_DATA][static_cast<unsigned int>(i)];
            if (optinal.isMember(KEY_OPTIONAL_ACCOUNT_ID)
                    && !optinal[KEY_OPTIONAL_ACCOUNT_ID].isNull()
                    && optinal[KEY_OPTIONAL_ACCOUNT_ID].type() == Json::stringValue) {
                mAccountId = optinal[KEY_OPTIONAL_ACCOUNT_ID].asString();
            }
            if (optinal.isMember(KEY_OPTIONAL_SERVICE_ID)
                    && !optinal[KEY_OPTIONAL_SERVICE_ID].isNull()
                    && optinal[KEY_OPTIONAL_SERVICE_ID].type() == Json::stringValue) {
                mServiceId = optinal[KEY_OPTIONAL_SERVICE_ID].asString();
            }
            if (optinal.isMember(KEY_OPTIONAL_SUBSCRIPTION_ID)
                    && !optinal[KEY_OPTIONAL_SUBSCRIPTION_ID].isNull()
                    && optinal[KEY_OPTIONAL_SUBSCRIPTION_ID].type() == Json::stringValue) {
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
