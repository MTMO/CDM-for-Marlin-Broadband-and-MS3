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

#define LOG_TAG "IMarlinLicense"
#include "MarlinLog.h"

#include <cstring>

#include "MarlinConstants.h"
#include "IMarlinLicense.h"
#include "MarlinRightsManager.h"
#include "MarlinCdmUtils.h"
#include "MessageCreator.h"

using namespace marlincdm;

CMutex IMarlinLicense::sLicenseAccessMutex;

IMarlinLicense::IMarlinLicense() :
        licenseHandleReference(0),
        isLicenseConsumed(false),
        mHandler(NULL),
        mHandle(NULL),
        isEME(false)
{
    MH_status_t agentStatus = MH_ERR_OK;
    MH_agentHandle_t handle;
    mHandler = new MarlinAgentHandler();
    mLicenseHandle.handle = NULL;
    mLicenseHandle.licensebuf.bufstr = NULL;
    mLicenseHandle.licensebuf.length = 0;

    agentStatus = mHandler->initAgent(&handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initAgent (%d).", agentStatus);
    }

    m_expiryTime.time1.reset();
    m_expiryTime.time2.reset();

    license_status = LICENSE_STATUS_UNINITIALIZED;
}

IMarlinLicense::~IMarlinLicense()
{
    if (license_status == LICENSE_STATUS_INITIALIZED) {
        finalizeLicenseSession();
    }
    if (mHandler != NULL) {
        mHandler->finAgent(&mHandle);
        delete mHandler;
        mHandler = NULL;
    }

    m_expiryTime.time1.reset();
    m_expiryTime.time2.reset();
}

mcdm_status_t IMarlinLicense::initializeLicenseSession(const string& content_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_licenseHandle_t licenseHandle = {NULL, {0, NULL}};
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());
    string extract_license;
    MH_licenseInfo_t info;

    licenseHandleMutex.lock();
    if (license_status == LICENSE_STATUS_INITIALIZED) {
        LOGD("license handle is already initilized.");
        licenseHandleReference++;
        goto EXIT;
    }

    agentStatus = mHandler->initLicenseHandle(mHandle, &licenseHandle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initLicenseHandle (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!rm.getLicenseData(content_id, extract_license)
            || extract_license.empty()) {
        LOGE("Could not get license from DB");
        status = ERROR_NO_LICENSE;
        goto EXIT;
    }

    licenseHandle.licensebuf.bufstr = new uint8_t[extract_license.length()];
    memcpy(licenseHandle.licensebuf.bufstr,
            reinterpret_cast<uint8_t*>(const_cast<char*>(extract_license.data())),
            extract_license.length());
    licenseHandle.licensebuf.length = extract_license.length();
    agentStatus = mHandler->setLicense(licenseHandle, &info);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling setLicense (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    mLicenseHandle = licenseHandle;
    licenseHandleReference++;
    license_status = LICENSE_STATUS_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    if (status != OK) {
        if (licenseHandle.licensebuf.bufstr != NULL
                && licenseHandle.licensebuf.length > 0) {
            free(licenseHandle.licensebuf.bufstr);
        }
        if (licenseHandle.handle != NULL) {
            mHandler->finLicenseHandle(&licenseHandle);
        }
        licenseHandle.handle = NULL;
        licenseHandle.licensebuf.bufstr = NULL;
        licenseHandle.licensebuf.length = 0;
    }
    licenseHandleMutex.unlock();
    return status;
}

void IMarlinLicense::finalizeLicenseSession()
{
    MARLINLOG_ENTER();
    licenseHandleMutex.lock();
    licenseHandleReference--;
    if (licenseHandleReference <= 0) {
        if (mLicenseHandle.licensebuf.bufstr != NULL
                && mLicenseHandle.licensebuf.length > 0) {
            free(mLicenseHandle.licensebuf.bufstr);
        }
        if (mLicenseHandle.handle != NULL
                && license_status == LICENSE_STATUS_INITIALIZED) {
            mHandler->finLicenseHandle(&mLicenseHandle);
        }
        mLicenseHandle.handle = NULL;
        mLicenseHandle.licensebuf.bufstr = NULL;
        mLicenseHandle.licensebuf.length = 0;
        license_status = LICENSE_STATUS_UNINITIALIZED;
    }
    MARLINLOG_EXIT();
    licenseHandleMutex.unlock();
}

mcdm_status_t IMarlinLicense::removeLicense(const string content_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    string extract_license;
    string extract_contentId;
    string licensePath("");
    MH_string_t i_license = {0, NULL};
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());

    if (!rm.getLicenseData(content_id, extract_license)
            || extract_license.empty()) {
        LOGD("license is not exist in DB");
        goto EXIT;
    }

    i_license.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(extract_license.data()));
    i_license.length = extract_license.size();
    agentStatus = mHandler->getContentIdByLicense(&i_license, &extract_contentId);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling getContentIdByLicense (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!rm.getLicensePath(extract_contentId, licensePath)) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!rm.remove(extract_contentId)) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    sLicenseAccessMutex.lock();
    if (remove(licensePath.c_str()) != 0) {
        LOGD("Error deleting %s", licensePath.c_str());
        status = ERROR_UNKNOWN;
    }
    sLicenseAccessMutex.unlock();
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinLicense::removeLicense(const mcdm_data_t license_data)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    string extract_contentId;
    string licensePath("");
    MH_string_t i_license = {(MH_size_t)license_data.len, license_data.data};
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());

    agentStatus = mHandler->getContentIdByLicense(&i_license, &extract_contentId);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling getContentIdByLicense (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!rm.getLicensePath(extract_contentId, licensePath)) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!rm.remove(extract_contentId)) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    sLicenseAccessMutex.lock();
    if (remove(licensePath.c_str()) != 0) {
        LOGD("Error deleting %s", licensePath.c_str());
        status = ERROR_UNKNOWN;
    }
    sLicenseAccessMutex.unlock();
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinLicense::storeLicenseToDataBase(const mcdm_key_type keyType,
                                                     const string license_Path,
                                                     const mcdm_data_t license_data,
                                                     const mcdm_data_t service_info)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_string_t i_license = {(MH_size_t)license_data.len, license_data.data};
    string licensePath("");
    string accountId("");
    string subscriptionId("");
    MH_cid_t contentId;
    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());

    agentStatus = mHandler->getContentIdByLicense(&i_license, &contentId);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling getContentIdByLicense (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (service_info.len > 0
            && service_info.data != NULL) {
        size_t offset = sizeof(int32_t);

        if (service_info.len > ACCOUNTID_BLOCKTYPE.length() + offset
                && !memcmp(service_info.data + offset, ACCOUNTID_BLOCKTYPE.c_str(), ACCOUNTID_BLOCKTYPE.length())) {
            // account id
            offset += ACCOUNTID_BLOCKTYPE.length();
            binToStr(service_info.data + offset, service_info.len - offset, accountId);
            LOGD("accountId = %s", accountId.c_str());
        }
        if (service_info.len > SUBSCRIPTIONID_BLOCKTYPE.length() + offset
                && !memcmp(service_info.data + offset, SUBSCRIPTIONID_BLOCKTYPE.c_str(), SUBSCRIPTIONID_BLOCKTYPE.length())) {
            // subscription Id
            offset += SUBSCRIPTIONID_BLOCKTYPE.length();
            binToStr(service_info.data + offset, service_info.len - offset, subscriptionId);
            subscriptionId = string(reinterpret_cast<const char*>(service_info.data + offset), service_info.len - offset);
            LOGD("subscriptionId = %s", subscriptionId.c_str());
        }
    }

    if (license_Path.empty() || keyType == KEY_TYPE_STREAMING) {
        licensePath.assign(HOME_DIRECTORY);
        licensePath.append(FILE_PATH_SEPARATOR);
        if (keyType == KEY_TYPE_STREAMING) {
            licensePath.append(STREAMING_LICENSE_NAME);
        } else {
            string fileName = rm.splitContentId(contentId);
            licensePath.append(fileName);
        }
        licensePath.append(LICENSE_EXTENSION);
    } else {
        licensePath.assign(license_Path);
    }

    LOGV("licensePath = %s", licensePath.c_str());
    if (!rm.store(contentId, accountId, subscriptionId, licensePath)) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinLicense::writeLicenseToFile(const mcdm_key_type keyType,
                                                 const string license_Path,
                                                 const mcdm_data_t license_data)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MarlinRightsManager& rm(MarlinRightsManager::getMarlinRightsManager());
    MH_string_t i_license = {(MH_size_t)license_data.len, license_data.data};
    string licensePath("");
    MH_cid_t contentId;
    FILE *fp;

    agentStatus = mHandler->getContentIdByLicense(&i_license, &contentId);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling getContentIdByLicense (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (license_Path.empty() || keyType == KEY_TYPE_STREAMING) {
        licensePath.assign(HOME_DIRECTORY);
        licensePath.append(FILE_PATH_SEPARATOR);
        if (keyType == KEY_TYPE_STREAMING) {
            licensePath.append(STREAMING_LICENSE_NAME);
        } else {
            string fileName = rm.splitContentId(contentId);
            licensePath.append(fileName);
        }
        licensePath.append(LICENSE_EXTENSION);
    } else {
        licensePath.assign(license_Path);
    }

    sLicenseAccessMutex.lock();
    LOGV("licensePath = %s", licensePath.c_str());
    if ((fp = fopen(licensePath.c_str(), "w+")) == NULL ) {
        LOGE("ERROR : create file\n");
        status = ERROR_UNKNOWN;
        sLicenseAccessMutex.unlock();
        goto EXIT;
    }
    if (license_data.len !=  fwrite(license_data.data, sizeof(uint8_t), license_data.len, fp)) {
        LOGD("ERROR : file write error\n");
        fclose(fp);
        status = ERROR_UNKNOWN;
        sLicenseAccessMutex.unlock();
        goto EXIT;
    }
    fclose(fp);
    sLicenseAccessMutex.unlock();
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinLicense::consumeLicense()
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_8pusInfo_t octpusInfo;

    allocate8pusInfo(octpusInfo);
    licenseHandleMutex.lock();
    if (license_status == LICENSE_STATUS_UNINITIALIZED) {
        status = ERROR_RIGHTS_NOT_SET;
        goto EXIT;
    }

    if (!isLicenseConsumed) {
        agentStatus = mHandler->evaluateLicense(mLicenseHandle, &octpusInfo);
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling evaluateLicense (%d).", agentStatus);
            status = ERROR_UNKNOWN;
            goto EXIT;
        }

        if ((MH_8PUS_TEMPORAL_FLAG_NOTAFTER & octpusInfo.temporal_c.flags) != 0) {
            if ((0 < octpusInfo.temporal_c.notafter_count) &&
                    NULL != octpusInfo.temporal_c.notafter[0]) {
                m_expiryTime.time1 = CTime((mcdm_date_t*)octpusInfo.temporal_c.notafter[0]);
            }
            if ((MAX_NOTAFTER_COUNT == octpusInfo.temporal_c.notafter_count) &&
                    (NULL != octpusInfo.temporal_c.notafter[1])) {
                m_expiryTime.time2 = CTime((mcdm_date_t*)octpusInfo.temporal_c.notafter[1]);
            }
        }
        isLicenseConsumed = true;
    }

EXIT:
    MARLINLOG_EXIT();
    free8pusInfo(octpusInfo);
    licenseHandleMutex.unlock();
    return status;
}

mcdm_status_t IMarlinLicense::checkGracePreiod()
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_LICENSE_EXPIRED;
    MH_status_t agentStatus = MH_ERR_OK;
    CTime trustedCurrentTime;
    MH_date_t io_date;
    mcdm_date_t date;

    if (!(m_expiryTime.time1.getUtcAsString().empty()) ||
        !(m_expiryTime.time2.getUtcAsString().empty())) {

        agentStatus = mHandler->getTrustedTime(mHandle, &io_date);
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling getTrustedTime (%d).", agentStatus);
            goto EXIT;
        }
        date.year  = io_date.year;
        date.mon   = io_date.mon;
        date.mday  = io_date.mday;
        date.hour  = io_date.hour;
        date.min   = io_date.min;
        date.sec   = io_date.sec;

        trustedCurrentTime.reset();
        trustedCurrentTime = CTime(&date);
        if ((!m_expiryTime.time1.getUtcAsString().empty()) &&
            (trustedCurrentTime.getUtc() - m_expiryTime.time1.getUtc() >= GRACE_PERIOD_SEC)) {
            LOGE("time1 license has expired.");
            goto EXIT;
        }
        if ((!m_expiryTime.time2.getUtcAsString().empty()) &&
            (trustedCurrentTime.getUtc() - m_expiryTime.time2.getUtc() >= GRACE_PERIOD_SEC)) {
            LOGE("time2 license has expired.");
            goto EXIT;
        }
    }
    status = OK;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinLicense::queryLicenseStatus(map<string, string>& /*licenseStatus*/)
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
    return ERROR_UNKNOWN;
}

mcdm_status_t IMarlinLicense::setProperty(string& /*name*/, string& /*value*/)
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
    return ERROR_UNKNOWN;
}

mcdm_status_t IMarlinLicense::getProperty(const string& name, string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t in_name = {0, NULL};
    MH_string_t io_value = {0, NULL};

    if (mHandler == NULL) {
        LOGE("Agent Handler is not initialize.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (name.length() > 0) {
        in_name.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(name.data()));
        in_name.length = name.length();

        agentStatus = mHandler->getPropertyString(in_name, &io_value);
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling getPropertyString (%d).", agentStatus);
            status = ERROR_UNKNOWN;
            goto EXIT;
        }

        if (io_value.bufstr != NULL && io_value.length > 0) {
            value.assign(reinterpret_cast<const char*>(io_value.bufstr), io_value.length);
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (io_value.bufstr != NULL) {
        delete[] io_value.bufstr;
        io_value.bufstr = NULL;
    }
    return status;
}

mcdm_status_t IMarlinLicense::getPropertyFormatString(const string& name, string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t in_name = {0, NULL};
    MH_string_t io_value = {0, NULL};

    if (mHandler == NULL) {
        LOGE("Agent Handler is not initialize.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (name.length() > 0) {
        in_name.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(name.data()));
        in_name.length = name.length();

        agentStatus = mHandler->getPropertyString(in_name, &io_value);
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling getPropertyString (%d).", agentStatus);
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
        if (!createPropertyMessage(&io_value, &value)) {
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (io_value.bufstr != NULL) {
        delete[] io_value.bufstr;
        io_value.bufstr = NULL;
    }
    return status;
}

mcdm_status_t IMarlinLicense::getDeviceProperty(string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t io_value = {0, NULL};

    if (mHandler == NULL) {
        LOGE("Agent Handler is not initialize.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    agentStatus = mHandler->getDeviceProperty(&io_value);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling getPropertyString (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (io_value.bufstr != NULL && io_value.length > 0) {
        value.assign(reinterpret_cast<char*>(io_value.bufstr), io_value.length);
    }

EXIT:
    MARLINLOG_EXIT();
    if (io_value.bufstr != NULL) {
        delete[] io_value.bufstr;
        io_value.bufstr = NULL;
    }
    return status;
}

mcdm_status_t IMarlinLicense::isTypeSupported(const string& key_system,
                                              const string& init_data_type,
                                              const string& content_type,
                                              const string& capability,
                                              string* isTypeSupportedResult)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t i_keysystem = {0, NULL};
    MH_string_t i_initdatatype = {0, NULL};
    MH_string_t i_contenttype = {0, NULL};
    MH_string_t i_capability = {0, NULL};
    MH_string_t o_istypesupportedresult = {0, NULL};

    if (mHandler == NULL) {
        LOGE("Agent Handler is not initialize.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (key_system.length() > 0) {
        i_keysystem.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(key_system.data()));
        i_keysystem.length = key_system.length();

        if(init_data_type.length() > 0) {
            i_initdatatype.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(init_data_type.data()));
            i_initdatatype.length = init_data_type.length();
        }
        if(content_type.length() > 0) {
            i_contenttype.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(content_type.data()));
            i_contenttype.length = content_type.length();
        }
        if(capability.length() > 0) {
            i_capability.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(capability.data()));
            i_capability.length = capability.length();
        }

        agentStatus = mHandler->isTypeSupported(i_keysystem, i_initdatatype, i_contenttype, i_capability, &o_istypesupportedresult);
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling isTypeSupported (%d).", agentStatus);
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
        if (o_istypesupportedresult.bufstr != NULL && o_istypesupportedresult.length > 0) {
            isTypeSupportedResult->assign(reinterpret_cast<const char*>(o_istypesupportedresult.bufstr), o_istypesupportedresult.length);
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (o_istypesupportedresult.bufstr != NULL) {
        delete[] o_istypesupportedresult.bufstr;
        o_istypesupportedresult.bufstr = NULL;
    }
    return status;
}

bool IMarlinLicense::initActionResult(MH_actionResult_t& o_result)
{
    bool ret = true;
    o_result.licensebuf = new MH_string_t;
    o_result.licensebuf->bufstr = NULL;
    o_result.licensebuf->length = 0;
    o_result.serviceidbuf = new MH_string_t;
    o_result.serviceidbuf->bufstr = NULL;
    o_result.serviceidbuf->length = 0;
    o_result.serviceinfobuf = new MH_string_t;
    o_result.serviceinfobuf->bufstr = NULL;
    o_result.serviceinfobuf->length = 0;
    return ret;
}

bool IMarlinLicense::deleteActionResult(MH_actionResult_t& i_result)
{
    bool ret = true;

    if (i_result.licensebuf != NULL) {
        if (i_result.licensebuf->bufstr != NULL) {
            delete[] i_result.licensebuf->bufstr;
            i_result.licensebuf->bufstr = NULL;
        }
        delete i_result.licensebuf;
        i_result.licensebuf = NULL;
    }
    if (i_result.serviceidbuf != NULL) {
        if (i_result.serviceidbuf->bufstr != NULL) {
            delete[] i_result.serviceidbuf->bufstr;
            i_result.serviceidbuf->bufstr = NULL;
        }
        delete i_result.serviceidbuf;
        i_result.serviceidbuf = NULL;
    }
    if (i_result.serviceinfobuf != NULL) {
        if (i_result.serviceinfobuf->bufstr != NULL) {
            delete[] i_result.serviceinfobuf->bufstr;
            i_result.serviceinfobuf->bufstr = NULL;
        }
        delete i_result.serviceinfobuf;
        i_result.serviceinfobuf = NULL;
    }

    return ret;
}

bool IMarlinLicense::initSasInfo(MH_sasInfo_t** io_sasInfo)
{
    bool ret = true;

    *io_sasInfo = new MH_sasInfo_t;
    (*io_sasInfo)->curl = new MH_string_t;
    (*io_sasInfo)->curl->bufstr = NULL;
    (*io_sasInfo)->curl->length = 0;
    (*io_sasInfo)->retentionflag = false;
    (*io_sasInfo)->numcontentid = 0;
    (*io_sasInfo)->content_ids = NULL;
    initOutPutControl((*io_sasInfo)->outputcontrol);

    return ret;
}

bool IMarlinLicense::deleteSasInfo(MH_sasInfo_t* i_sasInfo)
{
    bool ret = true;

    if (i_sasInfo != NULL) {
        if (i_sasInfo->curl != NULL) {
            if (i_sasInfo->curl->bufstr != NULL) {
                delete[] i_sasInfo->curl->bufstr;
                i_sasInfo->curl->bufstr = NULL;
            }
            delete i_sasInfo->curl;
            i_sasInfo->curl = NULL;
        }

        if (i_sasInfo->numcontentid > 0) {
            if (i_sasInfo->content_ids != NULL) {
                MH_string_t* content_ids = *i_sasInfo->content_ids;
                for (uint32_t i = 0; i > i_sasInfo->numcontentid; i++) {
                    MH_string_t* content_id = i_sasInfo->content_ids[i];
                    if (content_id != NULL) {
                        if (content_id->bufstr != NULL) {
                            delete[] content_id->bufstr;
                            content_id->bufstr = NULL;
                        }
                        delete content_id;
                        content_id = NULL;
                    }
                }
                delete[] content_ids;
                content_ids = NULL;
            }
        }
        delete i_sasInfo;
        i_sasInfo = NULL;
    }

    return ret;
}

void IMarlinLicense::allocate8pusInfo(MH_8pusInfo_t& io_8pusInfo)
{
    initOutPutControl(io_8pusInfo.permission_params);

    io_8pusInfo.counter_c.flags = 0;
    io_8pusInfo.counter_c.repeat_count_max = 0;
    io_8pusInfo.counter_c.repeat_count_remain = 0;

    io_8pusInfo.temporal_c.flags = 0;
    io_8pusInfo.temporal_c.notafter_count = 0;
    io_8pusInfo.temporal_c.notbefore_count = 0;
    io_8pusInfo.temporal_c.notlaterthan = 0;
    io_8pusInfo.temporal_c.notlongerthan = 0;
    io_8pusInfo.temporal_c.notmorethan = 0;

    io_8pusInfo.temporal_c.notbefore[0] = new MH_date_t;
    io_8pusInfo.temporal_c.notbefore[1] = new MH_date_t;

    io_8pusInfo.temporal_c.notafter[0] = new MH_date_t;
    io_8pusInfo.temporal_c.notafter[1] = new MH_date_t;

    io_8pusInfo.temporal_c.notduring[0] = new MH_date_t;
    io_8pusInfo.temporal_c.notduring[1] = new MH_date_t;
}

void IMarlinLicense::free8pusInfo(MH_8pusInfo_t& io_8pusInfo)
{
    if (io_8pusInfo.temporal_c.notbefore[0] != NULL) {
        delete io_8pusInfo.temporal_c.notbefore[0];
        io_8pusInfo.temporal_c.notbefore[0] = NULL;
    }
    if (io_8pusInfo.temporal_c.notbefore[1] != NULL) {
        delete io_8pusInfo.temporal_c.notbefore[1];
        io_8pusInfo.temporal_c.notbefore[1] = NULL;
    }
    if (io_8pusInfo.temporal_c.notafter[0] != NULL) {
        delete io_8pusInfo.temporal_c.notafter[0];
        io_8pusInfo.temporal_c.notafter[0] = NULL;
    }
    if (io_8pusInfo.temporal_c.notafter[1] != NULL) {
        delete io_8pusInfo.temporal_c.notafter[1];
        io_8pusInfo.temporal_c.notafter[1] = NULL;
    }
    if (io_8pusInfo.temporal_c.notduring[0] != NULL) {
        delete io_8pusInfo.temporal_c.notduring[0];
        io_8pusInfo.temporal_c.notduring[0] = NULL;
    }
    if (io_8pusInfo.temporal_c.notduring[1] != NULL) {
        delete io_8pusInfo.temporal_c.notduring[1];
        io_8pusInfo.temporal_c.notduring[1] = NULL;
    }
}

void IMarlinLicense::initOutPutControl(MH_8pus_outputcontrol_t& o_outputcontrol)
{
    o_outputcontrol.flags = 0;
    o_outputcontrol.e.basic.aps = 0;
    o_outputcontrol.e.basic.cci = 0;
    o_outputcontrol.e.basic.dot = 0;
    o_outputcontrol.e.basic.epn = 0;
    o_outputcontrol.e.basic.ict = 0;
    o_outputcontrol.e.dtcp.aps = 0;
    o_outputcontrol.e.dtcp.cci = 0;
    o_outputcontrol.e.dtcp.epn = 0;
    o_outputcontrol.e.dtcp.ict = 0;
    o_outputcontrol.e.dtcp.rmm = 0;
    o_outputcontrol.e.dtcp.rs = 0;
}
