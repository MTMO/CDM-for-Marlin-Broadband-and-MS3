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

#define LOG_TAG "CommonHeaderParser"
#include "MarlinLog.h"

#include <cstdlib>
#include <cstring>
#include "json/json.h"

#include "CommonHeaderParser.h"
#include "MarlinCdmUtils.h"

using namespace marlincdm;

CommonHeaderParser::CommonHeaderParser() {
    MARLINLOG_ENTER();
    mVersion.clear();
    mProcessTypeFlag.clear();
    mDataTypeFlag.clear();
}

CommonHeaderParser::~CommonHeaderParser() {
    MARLINLOG_ENTER();
}


INIT_DATA_VERSION &CommonHeaderParser::getVersion() {
    return mVersion;
}

PROCESS_TYPE_FLAG& CommonHeaderParser::getProcessTypeFlag() {
    return mProcessTypeFlag;
}

DATA_TYPE_FLAG& CommonHeaderParser::getDataTypeFlag() {
    return mDataTypeFlag;
}

parse_status_t CommonHeaderParser::parse(const string& json) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, properties;

    if (json.length() <= 0) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
    }

    json_ret = reader.parse(json, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }

    if(!root.isMember(KEY_PROPERTIES)){
        LOGE("nothing properties value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    properties = root[KEY_PROPERTIES];

    if(!properties.isMember(KEY_VERSION)
            || !properties.isMember(KEY_PROCESS_TYPE)
            || !properties.isMember(KEY_DATA_TYPE)){
        LOGE("nothing required value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(properties[KEY_VERSION].type() != Json::stringValue
            || properties[KEY_PROCESS_TYPE].type() != Json::stringValue
            || properties[KEY_DATA_TYPE].type() != Json::stringValue){
        LOGE("required value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    mVersion = properties[KEY_VERSION].asString();
    mProcessTypeFlag = properties[KEY_PROCESS_TYPE].asString();
    mDataTypeFlag = properties[KEY_DATA_TYPE].asString();

    if(!checkVersion(mVersion)) {
        LOGE("version is not supported");
        ret = INIT_DATA_PARSE_ERROR_NOT_SUPPORTED;
        goto EXIT;
    }
EXIT:
    return ret;
}

bool CommonHeaderParser::checkVersion(const string version) {
    bool ret = false;
    if (!SUPPORTED_VERSION_1_0.compare(version)) {
        ret = true;
    }
    return ret;
}
