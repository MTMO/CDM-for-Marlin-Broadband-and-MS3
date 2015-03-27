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

#ifndef __COMMON_HEADER_PARSER_H__
#define __COMMON_HEADER_PARSER_H__

#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"

namespace marlincdm {

// COMMON HEADER TABLE
typedef string INIT_DATA_STRING_TYPE;
typedef int32_t INIT_DATA_INT_TYPE;

// KEY FOR COMMON HEADER TABLE
typedef string INIT_DATA_KEY;
static const INIT_DATA_KEY KEY_PROPERTIES = "properties";
static const INIT_DATA_KEY KEY_VERSION = "version";
static const INIT_DATA_KEY KEY_PROCESS_TYPE = "process_type";
static const INIT_DATA_KEY KEY_DATA_TYPE = "data_type";
static const INIT_DATA_KEY KEY_OPTINAL_DATA = "optional_parameters";
static const INIT_DATA_KEY KEY_OPTINAL_DATA_NAME = "name";
static const INIT_DATA_KEY KEY_OPTINAL_DATA_VALUE = "value";
static const INIT_DATA_KEY KEY_CONTENT_ID = "content_id";

// VERSION
typedef INIT_DATA_STRING_TYPE INIT_DATA_VERSION;
static const INIT_DATA_VERSION CURRENT_VERSION = "1.0";
static const INIT_DATA_VERSION SUPPORTED_VERSION_1_0 = "1.0";

// PROCESS TYPE
typedef INIT_DATA_STRING_TYPE PROCESS_TYPE_FLAG;
static const PROCESS_TYPE_FLAG PROCESS_TYPE_ANDROID("android");
static const PROCESS_TYPE_FLAG PROCESS_TYPE_EME("eme");

// DATA TYPE
typedef INIT_DATA_STRING_TYPE DATA_TYPE_FLAG;
static const DATA_TYPE_FLAG DATA_TYPE_CENC("cenc");
static const DATA_TYPE_FLAG DATA_TYPE_IPMP("ipmp");
static const DATA_TYPE_FLAG DATA_TYPE_MAS("mas");
static const DATA_TYPE_FLAG DATA_TYPE_ATKN("atkn");
static const DATA_TYPE_FLAG DATA_TYPE_MS3("ms3");
static const DATA_TYPE_FLAG DATA_TYPE_LTP("ltp");
static const DATA_TYPE_FLAG DATA_TYPE_PROP("prop");
static const DATA_TYPE_FLAG DATA_TYPE_DECE("dece");

class CommonHeaderParser {
    public:
        CommonHeaderParser();
        virtual ~CommonHeaderParser();
        parse_status_t parse(const string& json);
        bool checkVersion(const string version);

        INIT_DATA_VERSION& getVersion();
        PROCESS_TYPE_FLAG& getProcessTypeFlag();
        DATA_TYPE_FLAG& getDataTypeFlag();

    private:

        INIT_DATA_VERSION mVersion;
        PROCESS_TYPE_FLAG mProcessTypeFlag;
        DATA_TYPE_FLAG mDataTypeFlag;

}; // class

}; // namespace marlincdm

#endif /* __COMMON_HEADER_PARSER_H__ */
