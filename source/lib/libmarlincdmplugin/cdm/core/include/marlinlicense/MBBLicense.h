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

#ifndef __MBB_LICENSE_H__
#define __MBB_LICENSE_H__

#include "MarlinLicense.h"

namespace marlincdm {

class MBBLicense : public IMarlinLicense {
public:

    MBBLicense();
    virtual ~MBBLicense();

    virtual mcdm_status_t initializeBBSession(mcdm_bb_mode mode);

    void finalizeBBSession();

    virtual mcdm_status_t processServiceToken(string* request,
                                              string* uri,
                                              uint32_t& remaining_steps);

    virtual mcdm_status_t processServiceToken(mcdm_data_t atkn,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url);

    virtual mcdm_status_t processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* license,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist);

    mcdm_status_t processLTPMessage(mcdm_data_t& ltpMessage,
                                    string* license,
                                    bool& is_license_exist);

    mcdm_status_t getRegisteredStatus(const string& service_id,
                                      const string& account_id,
                                      const string& subscription_id,
                                      string& registeredstate);

    mcdm_status_t getRegisteredStatus(const string& service_id,
                                      const string& account_id,
                                      string& registeredstate);

    virtual mcdm_status_t queryLicenseStatus(map<string, string>& licenseStatus);

    inline MH_bbHandle_t getBBhandle() const {
        return mBBhandle;
    }

    inline void setBBhandle(const MH_bbHandle_t bBhandle) {
        mBBhandle = bBhandle;
    }

private:

    typedef enum _BBLicenseStatus {
        BB_LICENSE_STATUS_UNINITIALIZED = 0,
        BB_LICENSE_STATUS_INITIALIZED,
        BB_LICENSE_STATUS_DECRYPTION_INITIALIZED
    } BBLicenseStatus;

    enum mcdm_registered_state {
        STATE_UNKNOWN = 0,
        STATE_ACCOUNT_NOT_REGISTERED,
        STATE_ACCOUNT_ALREADY_REGISTERED,
        STATE_ALREADY_REGISTERED_BY_ANOTHER_ACCOUNT
    };

    static const string ALREADY_REGISTERED;
    static const string NOT_REGISTERED;
    static const string ANOTHER_REGISTERED;

    MH_bbHandle_t mBBhandle;
    BBLicenseStatus bb_license_status;

    bool isLinkAccount(MH_string_t i_service_id,
                       MH_string_t i_service_info);

    bool isOtherLink(void);

    void freeLinklist(MH_linkList_t* io_links);

    bool convertAccountID(const string& accountIdStr,
                          unsigned char* accountIdBin);

    bool convertSubscriptionID(const string& subscriptionIdStr,
                               unsigned char* subscriptionIdBin);

    bool convert8pusToTemporalLicenseInfo(const MH_8pusInfo_t& i_8pusInfo,
                                          map<string, string>& o_info);

    bool convert8pusToCounterLicenseInfo(const MH_8pusInfo_t& i_8pusInfo,
                                         map<string, string>& o_info);

    bool convert8pusToOutputControlInfo(const MH_8pusInfo_t& i_8pusInfo,
                                        map<string, string>& o_info);

    bool convertTrustedTimeToLicenseInfo(map<string, string>& o_info);

    bool convertRegisteredStatusToStr(const mcdm_registered_state state, string& o_state);

};  //class
};  //namespace

#endif /* __MBB_LICENSE_H__ */
