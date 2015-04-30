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

#define LOG_TAG "MarlinLicenseManager"
#include "MarlinLog.h"

#include "MarlinLicenseManager.h"
#include "MarlinAgentHandler.h"
#include "MarlinRightsManager.h"
#include "MarlinConstants.h"
#include "MBBLicense.h"
#include "MS3License.h"

using namespace marlincdm;

MarlinLicenseManager *MarlinLicenseManager::licenseManager = 0;
CMutex MarlinLicenseManager::sMutex;

static map<string, IMarlinLicense*> mMarlinLicenseMapByCID;
static map<mcdm_SessionId_t, IMarlinLicense*> mMarlinLicenseMapBySessionId;
static CMutex sMapMutex;

MarlinLicenseManager::MarlinLicenseManager()
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
}

MarlinLicenseManager::~MarlinLicenseManager()
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
}

IMarlinLicense* MarlinLicenseManager::getMarlinLicense(const MarlinLicenseType& i_type,
                                                       mcdm_SessionId_t* i_sessionId,
                                                       IMarlinMediaFormat* i_mediaFormat)
{
    MARLINLOG_ENTER();
    IMarlinLicense* marlinLicense = NULL;

    sMapMutex.lock();
    if (i_sessionId != NULL && i_mediaFormat !=  NULL) {
        string cid;
        if(i_mediaFormat->getCid(NULL, cid) != OK) {
            LOGE("Could not get cid.");
            goto EXIT;
        }
        if (isExistLicenseFromCid(cid) && isExistLicenseFromSessionId(*i_sessionId)) {
            IMarlinLicense* sidMarlinLicense = mMarlinLicenseMapBySessionId[*i_sessionId];
            IMarlinLicense* cidMarlinLicense = mMarlinLicenseMapByCID[cid];

            if (sidMarlinLicense != cidMarlinLicense) {
                delete sidMarlinLicense;
                sidMarlinLicense = NULL;
                marlinLicense = getMarlinLicense(i_type, cidMarlinLicense, *i_mediaFormat);
                mMarlinLicenseMapBySessionId[*i_sessionId] = marlinLicense;
            } else {
                marlinLicense = mMarlinLicenseMapBySessionId[*i_sessionId];
            }
        } else if (!isExistLicenseFromCid(cid) && isExistLicenseFromSessionId(*i_sessionId)) {
            marlinLicense = mMarlinLicenseMapBySessionId[*i_sessionId];
            marlinLicense = getMarlinLicense(i_type, marlinLicense, *i_mediaFormat);
            mMarlinLicenseMapBySessionId[*i_sessionId] = marlinLicense;
        } else if (isExistLicenseFromCid(cid) && !isExistLicenseFromSessionId(*i_sessionId)) {
            marlinLicense = getMarlinLicense(i_type, *i_mediaFormat);
            mMarlinLicenseMapBySessionId[*i_sessionId] = marlinLicense;
        } else if (!isExistLicenseFromCid(cid) && !isExistLicenseFromSessionId(*i_sessionId)) {
            marlinLicense = getMarlinLicense(i_type, *i_mediaFormat);
            mMarlinLicenseMapBySessionId[*i_sessionId] = marlinLicense;
        }
    } else if (i_mediaFormat != NULL) {
        marlinLicense = getMarlinLicense(i_type, *i_mediaFormat);
    } else if (i_sessionId != NULL) {
        marlinLicense = getMarlinLicense(i_type, *i_sessionId);
    } else {
        marlinLicense = createMarlinLicense(i_type);
    }

EXIT:
    MARLINLOG_EXIT();
    sMapMutex.unlock();
    return marlinLicense;
}

IMarlinLicense* MarlinLicenseManager::getMarlinLicense(const MarlinLicenseType& i_type,
                                                       mcdm_SessionId_t& i_sessionId)
{
    MARLINLOG_ENTER();
    IMarlinLicense* marlinLicense = NULL;

    if (isExistLicenseFromSessionId(i_sessionId)) {
        marlinLicense = mMarlinLicenseMapBySessionId[i_sessionId];
        goto EXIT;
    }
    marlinLicense = createMarlinLicense(i_type);
    mMarlinLicenseMapBySessionId[i_sessionId] = marlinLicense;

EXIT:
    MARLINLOG_EXIT();
    return marlinLicense;
}

IMarlinLicense* MarlinLicenseManager::getMarlinLicense(const MarlinLicenseType& i_type,
                                                       IMarlinMediaFormat& i_mediaFormat)
{
    MARLINLOG_ENTER();
    bool ret = false;
    IMarlinLicense* marlinLicense = NULL;
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());
    MarlinAgentHandler agentHandler;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string extract_license;
    MH_string_t setup_license;
    vector<string> cids;
    string current_cid;

    mediaFormat = i_mediaFormat.getMediaFormat();
    if ((mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN)) {
        LOGE("Content Format is illegal.");
        goto EXIT;
    }

    // get current cid
    if(i_mediaFormat.getCid(NULL, current_cid) != OK) {
        LOGE("Could not get cid.");
        goto EXIT;
    }
    if (isExistLicenseFromCid(current_cid)) {
        marlinLicense = mMarlinLicenseMapByCID[current_cid];
        if(marlinLicense != NULL) {
            marlinLicense->initializeLicenseSession(current_cid);
            ret = true;
            goto EXIT;
        }
    }

    // create MarlinLicense and Initialize License Session
    marlinLicense = createMarlinLicense(i_type);
    if (marlinLicense == NULL) {
        goto EXIT;
    }
    if(marlinLicense->initializeLicenseSession(current_cid) != OK) {
        LOGE("Could not Initialize License Session");
        goto EXIT;
    }

    // get license from DB
    if (!rm.getLicenseData(current_cid, extract_license)
            || extract_license.empty()) {
        LOGE("Could not get license from DB");
        goto EXIT;
    }

    // get content id list from license
    cids.clear();
    setup_license.length = extract_license.length();
    setup_license.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(extract_license.data()));
    if (agentHandler.getContentIdsByLicense(&setup_license, &cids) != MH_ERR_OK) {
        LOGE("Could not get cid from license.");
        if (marlinLicense != NULL) {
            delete marlinLicense;
            marlinLicense = NULL;
        }
        goto EXIT;
    }

    // Mapping the license for cids in license file
    for (uint32_t i = 0; i < cids.size(); i++) {
        string cid = cids.at(i);
        mMarlinLicenseMapByCID[cid] = marlinLicense;
    }
    ret = true;
EXIT:
    MARLINLOG_EXIT();
    if (!ret && marlinLicense != NULL) {
        delete marlinLicense;
        marlinLicense = NULL;
    }
    return marlinLicense;
}

IMarlinLicense* MarlinLicenseManager::getMarlinLicense(const MarlinLicenseType& i_type,
                                                       IMarlinLicense* i_marlinLicense,
                                                       IMarlinMediaFormat& i_mediaFormat)
{
    MARLINLOG_ENTER();
    bool ret = false;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string current_cid;
    vector<string> cids;

    if(i_marlinLicense == NULL) {
        goto EXIT;
    }

    mediaFormat = i_mediaFormat.getMediaFormat();
    if ((mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN)) {
        LOGE("Content Format is illegal.");
        goto EXIT;
    }

    // get current cid
    if(i_mediaFormat.getCid(NULL, current_cid) != OK) {
        LOGE("Could not get cid.");
        goto EXIT;
    }

    // Initialize License Session
    if (isExistLicenseFromCid(current_cid)) {
        IMarlinLicense* marlinLicense = mMarlinLicenseMapByCID[current_cid];

        if (i_marlinLicense != marlinLicense) {
            delete i_marlinLicense;
            i_marlinLicense = NULL;
            i_marlinLicense = marlinLicense;
        }

        if(i_marlinLicense != NULL) {
            i_marlinLicense->initializeLicenseSession(current_cid);
            ret = true;
            goto EXIT;
        }
    }

    if(i_marlinLicense != NULL &&
            i_marlinLicense->initializeLicenseSession(current_cid) != OK) {
        LOGE("Could not Initialize License Session");
        goto EXIT;
    }

    if (i_type == LICENSE_TYPE_MBB) {
        MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());
        MarlinAgentHandler agentHandler;
        string extract_license;
        MH_string_t setup_license;

        // get license from DB
        if (!rm.getLicenseData(current_cid, extract_license)
                || extract_license.empty()) {
            LOGE("Could not get license from DB");
            goto EXIT;
        }

        // get content id list from license
        cids.clear();
        setup_license.length = extract_license.length();
        setup_license.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(extract_license.data()));
        if (agentHandler.getContentIdsByLicense(&setup_license, &cids) != MH_ERR_OK) {
            LOGE("Could not get cid from license.");
            if (i_marlinLicense != NULL) {
                delete i_marlinLicense;
                i_marlinLicense = NULL;
            }
            goto EXIT;
        }
    } else if (i_type == LICENSE_TYPE_MS3) {
        const MH_sasInfo_t* sasInfo = ((MS3License*)i_marlinLicense)->getSasinfo();

        if (sasInfo != NULL) {
            LOGE("Could not get sasInfo");
            goto EXIT;
        }
        for (uint32_t i = 0; i < sasInfo->numcontentid; i++) {
            MH_string_t* contentid = sasInfo->content_ids[i];
            if (contentid != NULL) {
                cids.push_back(
                        string(reinterpret_cast<char*>(contentid->bufstr),
                        contentid->length));
            }
        }
    }

    // Mapping the license for cids in license file
    for (uint32_t i = 0; i < cids.size(); i++) {
        string cid = cids.at(i);
        mMarlinLicenseMapByCID[cid] = i_marlinLicense;
    }
    ret = true;
EXIT:
    MARLINLOG_EXIT();
    if (!ret && i_marlinLicense != NULL) {
        delete i_marlinLicense;
        i_marlinLicense = NULL;
    }
    return i_marlinLicense;
}

void MarlinLicenseManager::removeMarlinLicense(mcdm_SessionId_t* i_sessionId,
                                               IMarlinMediaFormat* i_mediaFormat,
                                               bool isEME)
{
    MARLINLOG_ENTER();
    bool ret = false; // marlinLicense is deleted if it is true
    IMarlinLicense* marlinLicense = NULL;

    sMapMutex.lock();
    if (i_sessionId != NULL && i_mediaFormat !=  NULL) {
        if (isExistLicenseFromSessionId(*i_sessionId)) {
            marlinLicense = mMarlinLicenseMapBySessionId[*i_sessionId];
            mMarlinLicenseMapBySessionId.erase(*i_sessionId);

            // check to exist same MarlinLicense in cid map
            if (!isExistLicenseFromLicense(marlinLicense)) {
                ret = true;
                goto EXIT;
            }
            ret = removeMarlinLicense(marlinLicense);

        } else {
            ret = removeMarlinLicense(*i_mediaFormat, &marlinLicense);
        }
    } else if (i_mediaFormat != NULL) {
        ret = removeMarlinLicense(*i_mediaFormat, &marlinLicense);

    } else if (i_sessionId != NULL) {
        if (isExistLicenseFromSessionId(*i_sessionId)) {
            marlinLicense = mMarlinLicenseMapBySessionId[*i_sessionId];
            mMarlinLicenseMapBySessionId.erase(*i_sessionId);
        }
        ret = true;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    sMapMutex.unlock();
    if (ret && marlinLicense != NULL) {
        if (isEME && i_mediaFormat != NULL) {
            string cid;
            if (i_mediaFormat->getCid(NULL, cid) == OK) {
                marlinLicense->removeLicense(cid);
            }
        }
        delete marlinLicense;
        marlinLicense = NULL;
    }
}

bool MarlinLicenseManager::removeMarlinLicense(IMarlinMediaFormat& i_mediaFormat,
                                               IMarlinLicense** io_marlinLicense)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());
    MarlinAgentHandler agentHandler;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    IMarlinLicense* marlinLicense = NULL;
    string extract_license;
    MH_string_t setup_license;
    vector<string> cids;
    string current_cid;

    mediaFormat = i_mediaFormat.getMediaFormat();
    if ((mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN)) {
        LOGD("Content Format is illegal.");
        goto EXIT;
    }
    if(i_mediaFormat.getCid(NULL, current_cid) != OK) {
        LOGD("Could not get cid.");
        goto EXIT;
    }
    if (!isExistLicenseFromCid(current_cid)) {
        goto EXIT;
    }

    marlinLicense = mMarlinLicenseMapByCID[current_cid];
    marlinLicense->finalizeLicenseSession();

    if (marlinLicense->getLicenseStatus() != IMarlinLicense::LICENSE_STATUS_UNINITIALIZED) {
        LOGD("Using license handle yet.");
        goto EXIT;
    }

    if (!rm.getLicenseData(current_cid, extract_license)
            || extract_license.empty()) {
        LOGE("Could not get license from DB");
        goto EXIT;
    }

    setup_license.length = extract_license.length();
    setup_license.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(extract_license.data()));

    if (agentHandler.getContentIdsByLicense(&setup_license, &cids) != MH_ERR_OK) {
        LOGE("Could not get cid from license.");
        goto EXIT;
    }

    for (uint32_t i = 0; i < cids.size(); i++) {
        string cid = cids.at(i);
        mMarlinLicenseMapByCID.erase(cid);
    }
    *io_marlinLicense = marlinLicense;
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool MarlinLicenseManager::removeMarlinLicense(IMarlinLicense* i_marlinLicense)
{
    MARLINLOG_ENTER();
    bool ret = false;
    map<string, IMarlinLicense*>::const_iterator itr;
    vector<string> cids;

    if (i_marlinLicense == NULL) {
        goto EXIT;
    }

    i_marlinLicense->finalizeLicenseSession();
    if (i_marlinLicense->getLicenseStatus() != IMarlinLicense::LICENSE_STATUS_UNINITIALIZED) {
        LOGD("Using license handle yet.");
        goto EXIT;
    }

    itr = mMarlinLicenseMapByCID.begin();
    while(itr != mMarlinLicenseMapByCID.end()) {
        if(i_marlinLicense == itr->second) {
            cids.push_back(itr->first);
        }
        itr++;
    }
    for (uint32_t i = 0; i < cids.size(); i++) {
        string cid = cids.at(i);
        mMarlinLicenseMapByCID.erase(cid);
    }
    ret = true;

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

string MarlinLicenseManager::createMarlinLicenseFromSasInfo(const mcdm_SessionId_t* i_sessionId,
                                                                   const MH_sasInfo_t* i_sasInfo,
                                                                   IMarlinLicense* i_marlinLicense)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    string cid;

    if (i_sasInfo == NULL || i_sasInfo->content_ids == NULL) {
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    sMapMutex.lock();
    for (uint32_t i = 0; i < i_sasInfo->numcontentid; i++) {
        MH_string_t* contentid = i_sasInfo->content_ids[i];
        if (contentid != NULL) {
            cid.assign(reinterpret_cast<char*>(contentid->bufstr), contentid->length);

            if (isExistLicenseFromCid(cid)) {
                IMarlinLicense* marlinLicense = mMarlinLicenseMapByCID[cid];
                delete marlinLicense;
                marlinLicense = i_marlinLicense;
            }
            if (i_marlinLicense->initializeLicenseSession(
                    string(reinterpret_cast<char*>(contentid->bufstr), contentid->length)) != OK ) {
                LOGE("Could not Initialize License Session");
                sMapMutex.unlock();
                goto EXIT;
            }
            mMarlinLicenseMapByCID[cid] = i_marlinLicense;
            break;
        }
    }
    sMapMutex.unlock();

EXIT:
    MARLINLOG_EXIT();
    return cid;
}

IMarlinLicense* MarlinLicenseManager::createMarlinLicense(const MarlinLicenseType& i_type)
{
    IMarlinLicense* marlinLicense = NULL;
    switch(i_type) {
    case LICENSE_TYPE_MBB:
        marlinLicense = new MBBLicense();
        break;
    case LICENSE_TYPE_MS3:
        marlinLicense = new MS3License();
        break;
    case LICENSE_TYPE_NONE:
    default:
        marlinLicense = new MarlinLicense();
        break;
    };
    return marlinLicense;
}

bool MarlinLicenseManager::isExistLicenseFromSessionId(mcdm_SessionId_t& i_sessionId)
{
    bool ret = false;
    if (mMarlinLicenseMapBySessionId.count(i_sessionId) != 0) {
        ret = true;
    }
    return ret;
}

bool MarlinLicenseManager::isExistLicenseFromCid(string& i_cid)
{
    bool ret = false;
    if (mMarlinLicenseMapByCID.count(i_cid) != 0) {
        ret = true;
    }
    return ret;
}

bool MarlinLicenseManager::isExistLicenseFromLicense(IMarlinLicense* i_marlinLicense)
{
    bool ret = false;
    map<string, IMarlinLicense*>::const_iterator itr;
    itr = mMarlinLicenseMapByCID.begin();
    while(itr != mMarlinLicenseMapByCID.end()) {
        if(i_marlinLicense == itr->second) {
            ret = true;
            break;
        }
        itr++;
    }
    return ret;
}
