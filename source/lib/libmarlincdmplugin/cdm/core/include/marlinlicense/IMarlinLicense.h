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

#ifndef __IMARLIN_LICENSE_H__
#define __IMARLIN_LICENSE_H__

#include "MarlinAgentHandler.h"
#include "MarlinCommonTypes.h"
#include "MarlinCommonConfig.h"
#include "MarlinError.h"
#include "CTime.h"
#include "CMutex.h"

#define DOMAINID_SUBSFIELD_SIZE    10
#define DOMAINID_PADDING_SIZE       2
#define DOMAINID_ACCFIELD_SIZE      8

#define MAX_WIDTH_OF_INT 11
#define MAX_NOTAFTER_COUNT 2

const string FILE_PATH_SEPARATOR("/");
const string STREAMING_LICENSE_NAME("stream");
const string ACCOUNTID_BLOCKTYPE("ACID");
const string SUBSCRIPTIONID_BLOCKTYPE("ETID");

namespace marlincdm {

class IMarlinLicense {
public:

    typedef enum _LicenseStatus {
        LICENSE_STATUS_UNINITIALIZED = 0,
        LICENSE_STATUS_INITIALIZED
    } LicenseStatus;

    IMarlinLicense();
    virtual ~IMarlinLicense();

    virtual mcdm_status_t initializeLicenseSession(const string& content_id);

    virtual void finalizeLicenseSession();

    virtual mcdm_status_t processServiceToken(string* request,
                                              string* uri,
                                              uint32_t& remaining_steps) = 0;

    virtual mcdm_status_t processServiceToken(mcdm_data_t atkn,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* license,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist) = 0;

    virtual mcdm_status_t removeLicense(const string contentId);

    virtual mcdm_status_t removeLicense(const mcdm_data_t license_data);

    virtual mcdm_status_t storeLicenseToDataBase(const mcdm_key_type keyType,
                                                 const string license_Path,
                                                 const mcdm_data_t license_data,
                                                 const mcdm_data_t service_info);

    virtual mcdm_status_t writeLicenseToFile(const mcdm_key_type keyType,
                                             const string license_Path,
                                             const mcdm_data_t license_data);

    virtual mcdm_status_t consumeLicense();

    virtual mcdm_status_t checkGracePreiod();

    virtual mcdm_status_t queryLicenseStatus(map<string, string>& licenseStatus);

    virtual mcdm_status_t setProperty(string& name, string& value);

    virtual mcdm_status_t getProperty(const string& name, string& value);

    virtual mcdm_status_t getPropertyFormatString(const string& name,
                                                  string& value);

    virtual mcdm_status_t getDeviceProperty(string& value);

    virtual mcdm_status_t isTypeSupported(const string& key_system,
                                          const string& init_data_type,
                                          const string& content_type,
                                          const string& capability,
                                          string* isTypeSupportedResult);

    inline MH_agentHandle_t getHandle() const {
        return mHandle;
    }

    inline void setHandle(const MH_agentHandle_t handle) {
        mHandle = handle;
    }

    inline const MarlinAgentHandler* getHandler() const {
        return mHandler;
    }

    inline const MH_licenseHandle_t& getLicenseHandle() const {
        return mLicenseHandle;
    }

    inline void setLicenseHandle(const MH_licenseHandle_t& licenseHandle) {
        mLicenseHandle = licenseHandle;
    }

    LicenseStatus getLicenseStatus() const {
        return license_status;
    }

    virtual bool isEme() const {
        return isEME;
    }

    virtual void setIsEme(bool isEme) {
        isEME = isEme;
    }

protected:

    typedef struct {
        CTime time1;
        CTime time2;
    }ExpiryTime;

    static CMutex sLicenseAccessMutex;
    CMutex licenseHandleMutex;
    uint32_t licenseHandleReference;
    LicenseStatus license_status;
    MH_licenseHandle_t mLicenseHandle;
    bool isLicenseConsumed;
    MarlinAgentHandler* mHandler;
    MH_agentHandle_t mHandle;
    ExpiryTime m_expiryTime;
    bool isEME;

    bool initActionResult(MH_actionResult_t& o_result);

    bool deleteActionResult(MH_actionResult_t& i_result);

    bool initSasInfo(MH_sasInfo_t** io_sasInfo);

    bool deleteSasInfo(MH_sasInfo_t* i_sasInfo);

    void allocate8pusInfo(MH_8pusInfo_t& io_8pusInfo);

    void free8pusInfo(MH_8pusInfo_t& io_8pusInfo);

    void initOutPutControl(MH_8pus_outputcontrol_t& o_outputcontrol);

};  //class
};  //namespace

#endif /* __IMARLIN_LICENSE_H__ */
