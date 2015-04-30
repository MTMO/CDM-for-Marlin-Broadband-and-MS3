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

#ifndef __PROP_INIT_FORMAT_DATA_H__
#define __PROP_INIT_FORMAT_DATA_H__

#include <map>
#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

static const INIT_DATA_KEY KEY_PROP_DATA = "prop";
static const INIT_DATA_KEY KEY_PROP_DATA_NAME = "name";

// PROPERTY TYPE
typedef INIT_DATA_STRING_TYPE QUERY_PROPERTY_NAME_TYPE;
static const QUERY_PROPERTY_NAME_TYPE QUERY_NAME_DUID("duid");
static const QUERY_PROPERTY_NAME_TYPE QUERY_NAME_REGISTERED_STATE("registered_state");

class PropInitFormatData {
public:
    PropInitFormatData();
    virtual ~PropInitFormatData();
    parse_status_t parse(const string& data);
    QUERY_PROPERTY_NAME_TYPE getName();
    INIT_DATA_STRING_TYPE& getServiceId();
    INIT_DATA_STRING_TYPE& getAccountId();
    INIT_DATA_STRING_TYPE& getSubscriptionId();
    map<string, string>& getOptionalParameters();
private:
    QUERY_PROPERTY_NAME_TYPE mPropName;
    INIT_DATA_STRING_TYPE mServiceId;
    INIT_DATA_STRING_TYPE mAccountId;
    INIT_DATA_STRING_TYPE mSubscriptionId;
    map<string, string> mOptionalParameters;
}; // class

}; // namespace

#endif /* __PROP_INIT_FORMAT_DATA_H__ */
