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

#define LOG_TAG "MS3License"
#include "MarlinLog.h"

#include <cctype>
#include <algorithm>

#include "MS3License.h"
#include "MarlinCdmUtils.h"
#include "MessageCreator.h"

using namespace marlincdm;

MS3License::MS3License()
{
    mMS3handle.handle = NULL;
    mSasTriggerInfo.contenttype = new MH_string_t;
    mSasTriggerInfo.contenttype->bufstr = NULL;
    mSasTriggerInfo.contenttype->length = 0;
    mSasTriggerInfo.surl = new MH_string_t;
    mSasTriggerInfo.surl->bufstr = NULL;
    mSasTriggerInfo.surl->length = 0;
    mSasinfo = NULL;
    ms3_session_status = MS3_SESSION_STATUS_UNINITIALIZED;
}

MS3License::~MS3License()
{
    if (mMS3handle.handle != NULL) {
        finalizeMS3Session();
    }
    if (mSasTriggerInfo.contenttype != NULL) {
        if (mSasTriggerInfo.contenttype->bufstr != NULL) {
            delete[] mSasTriggerInfo.contenttype->bufstr;
            mSasTriggerInfo.contenttype->bufstr = NULL;
        }
        delete mSasTriggerInfo.contenttype;
        mSasTriggerInfo.contenttype = NULL;
    }
    if (mSasTriggerInfo.surl != NULL) {
        if (mSasTriggerInfo.surl->bufstr != NULL) {
            delete[] mSasTriggerInfo.surl->bufstr;
            mSasTriggerInfo.surl->bufstr = NULL;
        }
        delete mSasTriggerInfo.surl;
        mSasTriggerInfo.surl = NULL;
    }
    deleteSasInfo(mSasinfo);
}

mcdm_status_t MS3License::initializeMS3Session()
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_ms3Handle_t ms3_handle;

    if (mMS3handle.handle != NULL) {
        mHandler->finMS3Handle(&mMS3handle);
        mMS3handle.handle = NULL;
    }

    agentStatus = mHandler->initMS3Handle(&ms3_handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initMS3Handle (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    mMS3handle = ms3_handle;
    ms3_session_status = MS3_SESSION_STATUS_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

void MS3License::finalizeMS3Session()
{
    mHandler->finMS3Handle(&mMS3handle);
    mMS3handle.handle = NULL;
    ms3_session_status = MS3_SESSION_STATUS_UNINITIALIZED;
}

mcdm_status_t MS3License::initializeLicenseSession(const string& content_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    if (license_status == LICENSE_STATUS_INITIALIZED) {
        LOGD("license handle is already initilized.");
        licenseHandleReference++;
        goto EXIT;
    }

    mLicenseHandle.handle = mMS3handle.handle;
    licenseHandleReference++;
    license_status = LICENSE_STATUS_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    return status;

}

void MS3License::finalizeLicenseSession()
{
    licenseHandleReference--;
    if (licenseHandleReference <= 0) {
        if (mLicenseHandle.handle != NULL) {
            mLicenseHandle.handle = NULL;
        }
        license_status = LICENSE_STATUS_UNINITIALIZED;
    }
}

mcdm_status_t MS3License::processServiceToken(string* /*request*/,
                                              string* /*uri*/,
                                              uint32_t& /*remaining_steps*/)
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
    return ERROR_UNKNOWN;
}

mcdm_status_t MS3License::processServiceToken(mcdm_data_t suri,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_string_t in_curi = {0, NULL};
    MH_sasTriggerData_t in_trigger_data;
    MH_string_t in_request = {0, NULL};
    MH_httpRequest_t http_request;
    http_request.request = &in_request;
    http_request.url = NULL;

    in_curi.bufstr = suri.data;
    in_curi.length = suri.len;
    in_trigger_data.curi = &in_curi;
    in_trigger_data.triggertype = getTriggerType(suri);

    if (mSasTriggerInfo.contenttype->bufstr != NULL) {
        delete[] mSasTriggerInfo.contenttype->bufstr;
        mSasTriggerInfo.contenttype->bufstr = NULL;
        mSasTriggerInfo.contenttype->length = 0;
    }
    if (mSasTriggerInfo.surl->bufstr != NULL) {
        delete[] mSasTriggerInfo.surl->bufstr;
        mSasTriggerInfo.surl->bufstr = NULL;
        mSasTriggerInfo.surl->length = 0;
    }

    agentStatus = mHandler->generateSASRequest(&mMS3handle,
                                               &in_trigger_data,
                                               &mSasTriggerInfo,
                                               &http_request);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling generateSASRequest (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (!isEme()) {
        if (!createHttpRequestMessage(http_request, 1, request, url)) {
            LOGE("Could not generate request");
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
    }

EXIT:
    if (isEme()) {
        if (status != OK) {
            createCompleteMessage(SAS_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  0,
                                  request);
        } else {
            createHttpRequestMessageForEME(http_request, 0, request, url);
        }
    }
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3License::processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* content_id,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_httpResponse_t http_response;
    MH_string_t i_response =  {0, NULL};
    MH_sasResultData_t result;
    MH_string_t* contentid = NULL;
    is_license_exist = false;

    if (message == NULL) {
        LOGE("Input parameter is invalid.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (mSasinfo != NULL) {
        deleteSasInfo(mSasinfo);
    }
    initSasInfo(&mSasinfo);
    result.sasinfo = mSasinfo;

    if (response.data == NULL || response.len == 0) {
        LOGE("i_response is illegal.");
    }
    if (response.data != NULL && response.len > 0) {
        i_response.bufstr = new uint8_t[response.len];
        strToBin((char*)response.data, i_response.bufstr, response.len);
        i_response.length = response.len/2;
    }
    http_response.response = &i_response;
    http_response.error_flag = err;

    agentStatus = mHandler->processSASResponse(&mMS3handle,
                                               &mSasTriggerInfo,
                                               &http_response,
                                               &result);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling processSASResponse (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (result.sasinfo->numcontentid == 0
            || result.sasinfo->content_ids == NULL
            || result.sasinfo->content_ids[0] == NULL) {
        LOGE("Could not content id from sasinfo");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    contentid = result.sasinfo->content_ids[0];
    if (contentid->bufstr == NULL || contentid->length == 0) {
        LOGE("Could not get content id from sasinfo");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    if (isEme()) {
        uint32_t result_status = SUCCESS;

        if (status != OK) {
            result_status = FAIL;
        }
        createCompleteMessage(SAS_TYPE_ATKN_PROCESS,
                              status,
                              result_status,
                              0,
                              message);
    }
    if (i_response.bufstr != NULL) {
        delete[] i_response.bufstr;
        i_response.bufstr = NULL;
    }
    return status;
}

mcdm_status_t MS3License::removeLicense(const string contentId)
{
    return OK;
}

uint32_t& MS3License::getTriggerType(const mcdm_data_t data)
{
    const string actiontoken  = "actiontoken";
    const string ms3  = "ms3://";
    const string ms3h  = "ms3h://";
    const string ms3hs  = "ms3hs://";

    uint32_t tiggerType = MH_MS3_COMPOUND_URI;
    string tiggerdata;

    tiggerdata.assign(
            reinterpret_cast<char*>(const_cast<uint8_t*>(data.data)),
            data.len);
    transform(tiggerdata.begin(), tiggerdata.end(), tiggerdata.begin(), ::tolower);

    if (strstr(tiggerdata.c_str(), actiontoken.c_str()) != NULL) {
        tiggerType = MH_MS3_ACTION_TOKEN;
    } else if ((!ms3.compare(0, ms3.length(), tiggerdata))
            || (!ms3h.compare(0, ms3h.length(), tiggerdata))
            || (!ms3hs.compare(0, ms3hs.length(), tiggerdata))) {
        tiggerType = MH_MS3_COMPOUND_URI;
    } else {
        tiggerType = MH_MS3_MANIFEST_FILE;
    }

    return tiggerType;
}
