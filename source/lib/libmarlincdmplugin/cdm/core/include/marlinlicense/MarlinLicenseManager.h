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

#ifndef __MARLIN_LICENSE_MANAGER_H__
#define __MARLIN_LICENSE_MANAGER_H__

#include "MarlinLicense.h"
#include "MS3License.h"
#include "IMarlinMediaFormat.h"
#include "CMutex.h"

namespace marlincdm {

class MarlinLicenseManager {
public:
    typedef enum _MarlinLicenseType {
        LICENSE_TYPE_NONE = 0,
        LICENSE_TYPE_MBB,
        LICENSE_TYPE_MS3
    } MarlinLicenseType;

    static inline MarlinLicenseManager& getMarlinLicenseManager() {
        sMutex.lock();
        MarlinLicenseManager *instance = licenseManager;
        if (instance == 0) {
            instance = new MarlinLicenseManager();
            licenseManager = instance;
        }
        sMutex.unlock();
        return *instance;
    }

    IMarlinLicense* getMarlinLicense(const MarlinLicenseType& i_type,
                                     mcdm_SessionId_t* i_sessionId,
                                     IMarlinMediaFormat* i_mediaFormat);

    string createMarlinLicenseFromSasInfo(const mcdm_SessionId_t* i_sessionId,
                                                 const MH_sasInfo_t* i_sasInfo,
                                                 IMarlinLicense* i_marlinLicense);

    void removeMarlinLicense(mcdm_SessionId_t* i_sessionId,
                             IMarlinMediaFormat* i_mediaFormat,
                             bool isEME);

private:
    MarlinLicenseManager();
    virtual ~MarlinLicenseManager();

    static MarlinLicenseManager *licenseManager;
    static CMutex sMutex;

    IMarlinLicense* getMarlinLicense(const MarlinLicenseType& i_type,
                                     mcdm_SessionId_t& i_sessionId);

    IMarlinLicense* getMarlinLicense(const MarlinLicenseType& i_type,
                                     IMarlinMediaFormat& i_mediaFormat);

    IMarlinLicense* getMarlinLicense(const MarlinLicenseType& i_type,
                                     IMarlinLicense* i_marlinLicense,
                                     IMarlinMediaFormat& i_mediaFormat);

    bool removeMarlinLicense(IMarlinMediaFormat& i_mediaFormat,
                             IMarlinLicense** io_marlinLicense);

    bool removeMarlinLicense(IMarlinLicense* i_marlinLicense);

    IMarlinLicense* createMarlinLicense(const MarlinLicenseType& i_type);

    bool isExistLicenseFromSessionId(mcdm_SessionId_t& i_sessionId);

    bool isExistLicenseFromCid(string& i_cid);

    bool isExistLicenseFromLicense(IMarlinLicense* i_marlinLicense);

}; // class

}; // namespace marlincdm

#endif /* __MARLIN_LICENSE_MANAGER_H__ */
