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

#ifndef __ATKN_INIT_FORMAT_DATA_H__
#define __ATKN_INIT_FORMAT_DATA_H__

#include <map>
#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

static const INIT_DATA_KEY KEY_ATKN_DATA = "atkn";
static const INIT_DATA_KEY KEY_MATD_DATA = "matd";
static const INIT_DATA_KEY KEY_MATU_DATA = "matu";

class AtknInitFormatData {
public:
    AtknInitFormatData();
    virtual ~AtknInitFormatData();
    parse_status_t parse(const string& data);
    mcdm_data_t& getMatd();
    mcdm_data_t& getMatu();
    INIT_DATA_STRING_TYPE& getServiceId();
    INIT_DATA_STRING_TYPE& getAccountId();
    INIT_DATA_STRING_TYPE& getSubscriptionId();
    map<string, string>& getOptionalParameters();
private:
    mcdm_data_t mMatd;
    mcdm_data_t mMatu;
    INIT_DATA_STRING_TYPE mServiceId;
    INIT_DATA_STRING_TYPE mAccountId;
    INIT_DATA_STRING_TYPE mSubscriptionId;
    map<string, string> mOptionalParameters;
}; // class

}; // namespace

#endif /* __ATKN_INIT_FORMAT_DATA_H__ */
