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

#ifndef __CENC_INIT_FORMAT_DATA_H__
#define __CENC_INIT_FORMAT_DATA_H__

#include <vector>
#include "MarlinCommonTypes.h"
#include "InitDataParserErrors.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

static const INIT_DATA_KEY KEY_CENC_DATA = "cenc";
static const INIT_DATA_KEY KEY_LICENSE_EMBEDDED = "license_embedded";
static const INIT_DATA_KEY KEY_PSSH_DATA = "pssh";
static const INIT_DATA_KEY KEY_KIDS_DATA = "kids";
static const INIT_DATA_KEY KEY_KEYIDS_DATA = "keyids";
static const INIT_DATA_KEY KEY_SESSION_ID_DATA = "sessionid";

class CencInitFormatData {
    public:
        CencInitFormatData();
        virtual ~CencInitFormatData();
        parse_status_t parse(const string& data);
        parse_status_t parseKeyIds(const string& data);
        bool& getLicenseEmbedded();
        mcdm_data_t& getPssh();
        vector<uint8_t*>& getKidList();
        string& getContentId();
    private:
        bool mLicenseEmbedded;
        mcdm_SessionId_t mSessionId;
        mcdm_data_t mPssh;
        vector<uint8_t*> mKidList;
        string mCID;
}; // class

}; // namespace

#endif /* __CENC_INIT_FORMAT_DATA_H__ */
