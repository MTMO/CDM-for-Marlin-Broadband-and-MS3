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

#ifndef __IPMP_INIT_FORMAT_DATA_H__
#define __IPMP_INIT_FORMAT_DATA_H__

#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

static const INIT_DATA_KEY KEY_IPMP_DATA = "ipmp";
static const INIT_DATA_KEY KEY_SINF_DATA = "sinf";

class IpmpInitFormatData {
public:
    IpmpInitFormatData();
    virtual ~IpmpInitFormatData();
    parse_status_t parse(const string& data);
    mcdm_data_t& getSinf();
    string& getContentId();
private:
    mcdm_data_t mSinf;
    string mCID;
}; // class

}; // namespace

#endif /* __IPMP_INIT_FORMAT_DATA_H__ */
