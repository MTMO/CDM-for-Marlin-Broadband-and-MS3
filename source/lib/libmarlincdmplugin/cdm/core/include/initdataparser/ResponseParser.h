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

#ifndef __RESPONSE_PARSER_H__
#define __RESPONSE_PARSER_H__

#include <map>
#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

// KEY FOR RESPONSE TABLE
static const INIT_DATA_KEY KEY_RESPONSE_MSG_TYPE = "message_type";
static const INIT_DATA_KEY KEY_APP_ERROR = "applicationError";
static const INIT_DATA_KEY KEY_APP_ERROR_CODE = "code";
static const INIT_DATA_KEY KEY_HTTP_RESPONSE = "httpResponse";
static const INIT_DATA_KEY KEY_STATUS_CODE = "statusCode";
static const INIT_DATA_KEY KEY_RESPONSE_HEADERS = "headers";
static const INIT_DATA_KEY KEY_RESPONSE_BODY = "body";
static const INIT_DATA_KEY KEY_LICENSE = "license";
static const INIT_DATA_KEY KEY_LICENSE_DATA = "data";
static const INIT_DATA_KEY KEY_LICENSE_PATH = "path";
static const INIT_DATA_KEY KEY_TLS_RECORD = "tls_record";

// MASSAGE TYPE
typedef INIT_DATA_STRING_TYPE RESPONSE_MASSAGE_TYPE_FLAG;
static const RESPONSE_MASSAGE_TYPE_FLAG TYPE_NONE("none");
static const RESPONSE_MASSAGE_TYPE_FLAG TYPE_RESPONSE("httpResponse");
static const RESPONSE_MASSAGE_TYPE_FLAG TYPE_LICENSE("license");
static const RESPONSE_MASSAGE_TYPE_FLAG TYPE_TLS_RECORD("tls_record");
static const RESPONSE_MASSAGE_TYPE_FLAG TYPE_APP_ERROR("applicationError");

// STATUS CODE
typedef INIT_DATA_INT_TYPE STATUS_CODE_TYPE;
static const STATUS_CODE_TYPE STATUS_CODE_OK = 200;

// Application Error Code
static const STATUS_CODE_TYPE APP_NETWORK_ERROR = 1;
static const STATUS_CODE_TYPE APP_TIMEOUT_ERROR = 2;
static const STATUS_CODE_TYPE APP_ABORT_ERROR = 3;
static const STATUS_CODE_TYPE APP_NOT_SUPPORTED_ERROR = 4;
static const STATUS_CODE_TYPE APP_INVALID_STATE_ERROR = 5;

class ResponseParser {
    public:
        ResponseParser();
        virtual ~ResponseParser();
        parse_status_t parse(const string& httpResponse);
        parse_status_t parseForEME(const string& httpResponse);

        INIT_DATA_VERSION& getVersion();
        RESPONSE_MASSAGE_TYPE_FLAG& getMassageTypeFlag();
        map<string, string>& getHttpHeaders();
        INIT_DATA_INT_TYPE& getStatusCode();
        mcdm_data_t& getResponse();
        mcdm_data_t& getLicenseData();
        string& getLicensePath();
        mcdm_data_t& getTLSRecord();

    private:
        INIT_DATA_VERSION mVersion;
        RESPONSE_MASSAGE_TYPE_FLAG mMassageTypeFlag;
        map<string, string> mHttpHeaders;
        INIT_DATA_INT_TYPE mStatusCode;
        mcdm_data_t mResponse;
        mcdm_data_t mLicenseData;
        string mLicensePath;
        mcdm_data_t mTLSRecord;
}; // class

}; // namespace marlincdm

#endif /* __RESPONSE_PARSER_H__ */
