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

#ifndef __MS3_LICENSE_H__
#define __MS3_LICENSE_H__

#include "MarlinLicense.h"

namespace marlincdm {

class MS3License : public IMarlinLicense {
public:

    typedef enum _MS3SessionStatus {
        MS3_SESSION_STATUS_UNINITIALIZED = 0,
        MS3_SESSION_STATUS_INITIALIZED
    } ms3SessionStatus;

    MS3License();
    virtual ~MS3License();

    mcdm_status_t initializeMS3Session();

    void finalizeMS3Session();

    virtual mcdm_status_t initializeLicenseSession(const string& content_id);

    virtual void finalizeLicenseSession();

    virtual mcdm_status_t processServiceToken(string* request,
                                              string* uri,
                                              uint32_t& remaining_steps);

    virtual mcdm_status_t processServiceToken(mcdm_data_t suri,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* o_content_id,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist);

    virtual mcdm_status_t removeLicense(const string contentId);

    const MH_sasInfo_t* getSasinfo() const {
        return mSasinfo;
    }

    const ms3SessionStatus& getMs3SessionStatus() const {
        return ms3_session_status;
    }

private:

    uint32_t& getTriggerType(const mcdm_data_t data);

    MH_ms3Handle_t mMS3handle;
    MH_sasTriggerInfo_t mSasTriggerInfo;
    MH_sasInfo_t* mSasinfo;
    ms3SessionStatus ms3_session_status;

};  //class
};  //namespace

#endif /* __MS3_LICENSE_H__ */
