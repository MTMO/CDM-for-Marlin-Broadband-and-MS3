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

#define LOG_TAG "ResponseParser"
#include "MarlinLog.h"

#include <cstdlib>
#include <cstring>
#include "json/json.h"

#include "ResponseParser.h"
#include "MarlinCdmUtils.h"

using namespace marlincdm;

ResponseParser::ResponseParser() {
    MARLINLOG_ENTER();
    mVersion.clear();
    mStatusCode = -1;
    mResponse.len = 0;
    mResponse.data = NULL;
    mLicenseData.len = 0;
    mLicenseData.data = NULL;
    mLicensePath.clear();
    mTLSRecord.len = 0;
    mTLSRecord.data = NULL;
    mHttpHeaders.clear();
}

ResponseParser::~ResponseParser() {
    MARLINLOG_ENTER();
    if (mResponse.data != NULL) {
        delete mResponse.data;
        mResponse.data = NULL;
    }
    if (mLicenseData.data != NULL) {
        delete mLicenseData.data;
        mLicenseData.data = NULL;
    }
    if (mTLSRecord.data != NULL) {
        delete mTLSRecord.data;
        mTLSRecord.data = NULL;
    }
}

INIT_DATA_VERSION& ResponseParser::getVersion() {
    return mVersion;
}

RESPONSE_MASSAGE_TYPE_FLAG& ResponseParser::getMassageTypeFlag() {
    return mMassageTypeFlag;
}

INIT_DATA_INT_TYPE& ResponseParser::getStatusCode() {
    return mStatusCode;
}

mcdm_data_t& ResponseParser::getResponse() {
    return mResponse;
}

mcdm_data_t& ResponseParser::getLicenseData() {
    return mLicenseData;
}

string& ResponseParser::getLicensePath() {
    return mLicensePath;
}

mcdm_data_t& ResponseParser::getTLSRecord() {
    return mTLSRecord;
}

map<string, string>& ResponseParser::getHttpHeaders() {
    return mHttpHeaders;
}

parse_status_t ResponseParser::parse(const string& httpResponse) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    bool json_ret = false;
    CommonHeaderParser common_header_parser;
    Json::Reader reader;
    Json::Value root, properties;

    if (httpResponse.length() <= 0) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
    }

    json_ret = reader.parse(httpResponse, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("response data type is illegal");
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
            || !properties.isMember(KEY_RESPONSE_MSG_TYPE)){
        LOGE("nothing required value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    if(!properties[KEY_VERSION].isString()
            || !properties[KEY_RESPONSE_MSG_TYPE].isString()){
        LOGE("required value type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    mVersion = properties[KEY_VERSION].asString();
    mMassageTypeFlag = properties[KEY_RESPONSE_MSG_TYPE].asString();

    if (!common_header_parser.checkVersion(mVersion)) {
        LOGE("version is not supported");
        ret = INIT_DATA_PARSE_ERROR_NOT_SUPPORTED;
        goto EXIT;
    }

    if (!TYPE_RESPONSE.compare(mMassageTypeFlag)
            && properties.isMember(KEY_HTTP_RESPONSE)
            && properties[KEY_HTTP_RESPONSE].isObject()) {
        Json::Value response;
        string responseData("");
        response = properties[KEY_HTTP_RESPONSE];
        if(!response.isMember(KEY_STATUS_CODE)
                || !response.isMember(KEY_RESPONSE_BODY)){
            LOGE("nothing status code or response value");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }
        if(response[KEY_STATUS_CODE].type() != Json::intValue
                || response[KEY_RESPONSE_BODY].type() != Json::stringValue){
            LOGE("required value type is illegal");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }

        mStatusCode = response[KEY_STATUS_CODE].asInt();
        responseData = response[KEY_RESPONSE_BODY].asString();
        if (responseData.length() > 0) {
            mResponse.data = new uint8_t[responseData.length()];
            memcpy(mResponse.data, reinterpret_cast<uint8_t*>(const_cast<char*>(responseData.data())), responseData.length());
            mResponse.len = responseData.length();
        }

        if (response.isMember(KEY_RESPONSE_HEADERS)
                && response[KEY_RESPONSE_HEADERS].isArray()) {
            Json::Value optinal;
            string name;
            string value;
            for (size_t i = 0; i < response[KEY_RESPONSE_HEADERS].size(); i++) {
                optinal = response[KEY_RESPONSE_HEADERS][static_cast<unsigned int>(i)];
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
                    mHttpHeaders[name] = value;
                }
            }
        }
    }

    if (!TYPE_LICENSE.compare(mMassageTypeFlag)
            && properties.isMember(KEY_LICENSE)
            && !properties[KEY_LICENSE].isNull()
            && properties[KEY_LICENSE].isObject()) {
        Json::Value license;
        string data("");

        license = properties[KEY_LICENSE];
        if(!license.isMember(KEY_LICENSE_DATA)
                || !license[KEY_LICENSE_DATA].isString()){
            LOGE("license value type is illegal");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }

        data = license[KEY_LICENSE_DATA].asString();
        if (data.length() > 0) {
            mLicenseData.data = new uint8_t[data.length()];
            memcpy(mLicenseData.data, reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.length());
            mLicenseData.len = data.length();
        }

        if(license.isMember(KEY_LICENSE_PATH)
                && license[KEY_LICENSE_PATH].isString()){
            mLicensePath = license[KEY_LICENSE_PATH].asString();
        }
    }

    if (!TYPE_TLS_RECORD.compare(mMassageTypeFlag)
            && properties.isMember(KEY_TLS_RECORD)
            && !properties[KEY_TLS_RECORD].isNull()
            && properties[KEY_TLS_RECORD].isObject()) {
        string tls_record("");

        if(!properties[KEY_TLS_RECORD].isString()){
            LOGE("tls record value type is illegal");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }

        tls_record = properties[KEY_TLS_RECORD].asString();
        if (tls_record.length() > 0) {
            mTLSRecord.data = new uint8_t[tls_record.length()];
            memcpy(mTLSRecord.data, reinterpret_cast<uint8_t*>(const_cast<char*>(tls_record.data())), tls_record.length());
            mTLSRecord.len = tls_record.length();
        }
    }
EXIT:
    MARLINLOG_EXIT();
    return ret;
}

parse_status_t ResponseParser::parseForEME(const string& httpResponse) {
    MARLINLOG_ENTER();
    parse_status_t ret = INIT_DATA_PARSE_OK;
    bool json_ret = false;
    Json::Reader reader;
    Json::Value root, properties;

    if (httpResponse.length() <= 0) {
        LOGE("init data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
    }

    json_ret = reader.parse(httpResponse, root);
    if (!json_ret || root.type() != Json::objectValue) {
        LOGE("response data type is illegal");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }

    if(!root.isMember(KEY_PROPERTIES)){
        LOGE("nothing properties value");
        ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
        goto EXIT;
    }
    properties = root[KEY_PROPERTIES];

    if (properties.isMember(KEY_HTTP_RESPONSE)
            && properties[KEY_HTTP_RESPONSE].isObject()) {
        Json::Value response;
        string responseData("");

        mMassageTypeFlag = TYPE_RESPONSE;
        response = properties[KEY_HTTP_RESPONSE];
        if(!response.isMember(KEY_STATUS_CODE)
                || !response.isMember(KEY_RESPONSE_BODY)){
            LOGE("nothing status code or response value");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }
        if(response[KEY_STATUS_CODE].type() != Json::intValue
                || response[KEY_RESPONSE_BODY].type() != Json::stringValue){
            LOGE("required value type is illegal");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }

        mStatusCode = response[KEY_STATUS_CODE].asInt();
        responseData = response[KEY_RESPONSE_BODY].asString();
        if (responseData.length() > 0) {
            mResponse.data = new uint8_t[responseData.length()];
            memcpy(mResponse.data, reinterpret_cast<uint8_t*>(const_cast<char*>(responseData.data())), responseData.length());
            mResponse.len = responseData.length();
        }

        if (response.isMember(KEY_RESPONSE_HEADERS)
                && response[KEY_RESPONSE_HEADERS].isArray()) {
            Json::Value optinal;
            string name;
            string value;
            for (size_t i = 0; i < response[KEY_RESPONSE_HEADERS].size(); i++) {
                optinal = response[KEY_RESPONSE_HEADERS][static_cast<unsigned int>(i)];
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
                    mHttpHeaders[name] = value;
                }
            }
        }
    }

    if (properties.isMember(KEY_APP_ERROR)
            && properties[KEY_APP_ERROR].isObject()) {
        Json::Value appError;
        int appErrorCode = 0;

        mMassageTypeFlag = TYPE_APP_ERROR;
        appError = properties[KEY_APP_ERROR];
        if(!appError.isMember(KEY_APP_ERROR_CODE)){
            LOGE("nothing error code");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }
        if(appError[KEY_APP_ERROR_CODE].type() != Json::intValue){
            LOGE("required value type is illegal");
            ret = INIT_DATA_PARSE_ERROR_INVALID_ARG;
            goto EXIT;
        }
        mStatusCode = appError[KEY_APP_ERROR_CODE].asInt();
    }
EXIT:
    MARLINLOG_EXIT();
    return ret;
}
