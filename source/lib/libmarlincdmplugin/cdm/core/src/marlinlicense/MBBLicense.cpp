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

#define LOG_TAG "MBBLicense"
#include "MarlinLog.h"

#include "MBBLicense.h"
#include "MessageCreator.h"
#include "MarlinConstants.h"
#include "MarlinCdmUtils.h"
#include "CTime.h"

using namespace marlincdm;

const string MBBLicense::ALREADY_REGISTERED("Already Registered");
const string MBBLicense::NOT_REGISTERED("Not Registered");
const string MBBLicense::ANOTHER_REGISTERED("Registered by others");

MBBLicense::MBBLicense()
{
    mBBhandle.handle = NULL;
    mBBhandle.client = NULL;
    mBBhandle.mode = MH_BB_MODE_TYPE_LOCAL;
    bb_license_status = BB_LICENSE_STATUS_UNINITIALIZED;
}

MBBLicense::~MBBLicense()
{
    if (bb_license_status == BB_LICENSE_STATUS_INITIALIZED) {
        finalizeBBSession();
    }
}

mcdm_status_t MBBLicense::initializeBBSession(mcdm_bb_mode mode)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_bbHandle_t bb_handle;

    if (mode == BB_MODE_TYPE_USB) {
        bb_handle.mode = MH_BB_MODE_TYPE_USB;
    } else {
        bb_handle.mode = MH_BB_MODE_TYPE_LOCAL;
    }

    agentStatus = mHandler->initBBHandle(&bb_handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initBBHandle (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    mBBhandle = bb_handle;
    bb_license_status = BB_LICENSE_STATUS_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

void MBBLicense::finalizeBBSession()
{
    if (mHandler != NULL && bb_license_status == BB_LICENSE_STATUS_INITIALIZED) {
        mHandler->finBBHandle(&mBBhandle);
        mBBhandle.handle = NULL;
        mBBhandle.client = NULL;
    }
    bb_license_status = BB_LICENSE_STATUS_UNINITIALIZED;
}


mcdm_status_t MBBLicense::processServiceToken(string* request,
                                              string* url,
                                              uint32_t& remaining_steps)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t request_buf = {0, NULL};
    MH_httpRequest_t http_request;
    http_request.request = &request_buf;
    http_request.method = MH_GET;
    http_request.url = NULL;

    if (bb_license_status == BB_LICENSE_STATUS_UNINITIALIZED) {
        LOGE("BB Handle is not initialized.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (request == NULL || url == NULL) {
        LOGE("Input parameter is invalid.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (remaining_steps == 0) {
        *url = string("");
        if(!createDefaultMessage(remaining_steps, request)) {
            LOGE("Could not genarate request");
            status = ERROR_UNKNOWN;
        }
        goto EXIT;
    }

    agentStatus = mHandler->processServiceToken(&mBBhandle, &http_request);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling processServiceToken (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!isEme()) {
        if (!createHttpRequestMessage(http_request, remaining_steps, request, url)) {
            LOGE("Could not generate request");
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (isEme()) {
        if (status != OK) {
            createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  remaining_steps,
                                  request);
        } else {
            createHttpRequestMessageForEME(http_request, remaining_steps, request, url);
        }
    }
    if (http_request.request->bufstr != NULL) {
        delete[] http_request.request->bufstr;
        http_request.request->bufstr = NULL;
    }
    if (http_request.url != NULL) {
        delete[] http_request.url;
        http_request.url = NULL;
    }
    return status;
}

mcdm_status_t MBBLicense::processServiceToken(mcdm_data_t atkn,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    string service_id;
    string account_id;
    string subscription_id;
    MH_string_t in_request = {0, NULL};
    MH_string_t service_token = {0, NULL};
    MH_string_t in_service_id = {0, NULL};
    MH_string_t in_service_info = {0, NULL};
    MH_httpRequest_t http_request;
    uint32_t remaining_steps = 0;
    http_request.request = &in_request;
    http_request.url = NULL;

    size_t lengthSize = 0, accountBoxSize = 0, subscriptionBoxSize = 0;
    uint32_t index = 0, index2 = 0;
    uint8_t boxSize[sizeof(uint32_t)] = {0x00};
    uint8_t* serviceInfo = NULL;
    string serviceInfoString("");
    unsigned char* convertAID = NULL;
    unsigned char* convertSID = NULL;

    if (bb_license_status == BB_LICENSE_STATUS_UNINITIALIZED) {
        LOGE("BB Handle is not initialized.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (request == NULL || url == NULL) {
        LOGE("Input parameter is invalid.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (optionalParameters != NULL && !optionalParameters->empty()) {
        service_id = (*optionalParameters)[KEY_OPTIONAL_SERVICE_ID];
        account_id = (*optionalParameters)[KEY_OPTIONAL_ACCOUNT_ID];
        subscription_id = (*optionalParameters)[KEY_OPTIONAL_SUBSCRIPTION_ID];

        if (service_id.length() <= 0
                || (account_id.length() <= 0 && subscription_id.length() <= 0)) {
            LOGE("optionalParameters is illegal.");
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }

        if (account_id.length() > 0) {
            accountBoxSize = sizeof(uint32_t) + ACCOUNTID_BLOCKTYPE.length() + DOMAINID_ACCFIELD_SIZE;
        }
        if (subscription_id.length() > 0) {
            subscriptionBoxSize = sizeof(uint32_t) + SUBSCRIPTIONID_BLOCKTYPE.length() + subscription_id.length();
        }

        in_service_info.length = accountBoxSize + subscriptionBoxSize;
        serviceInfo = new uint8_t[in_service_info.length];
        lengthSize = sizeof(uint32_t);
        memset(serviceInfo, 0x00, in_service_info.length);

        if (account_id.length() > 0) {
            convertAID = new unsigned char[DOMAINID_ACCFIELD_SIZE];
            if (convertAccountID(account_id, convertAID)) {
                memset(&boxSize, 0x00, sizeof(lengthSize));
                memcpy(&boxSize, &accountBoxSize, sizeof(uint32_t));
                for (index = 0; index < lengthSize; index++) {
                    memcpy(serviceInfo + index, &boxSize[lengthSize - index - 1], 1);
                }
                serviceInfoString = ACCOUNTID_BLOCKTYPE;
                memcpy(serviceInfo + index, serviceInfoString.c_str(), serviceInfoString.length());
                index += serviceInfoString.length();
                memcpy(serviceInfo + index, convertAID, DOMAINID_ACCFIELD_SIZE);
            }
        }

        if (subscription_id.length() > 0) {
            convertSID = new unsigned char[DOMAINID_SUBSFIELD_SIZE];
            if (convertSubscriptionID(subscription_id, convertSID)) {
                index = accountBoxSize;
                serviceInfoString = string("");
                memset(&boxSize, 0x00, sizeof(boxSize));
                memcpy(&boxSize, &subscriptionBoxSize, sizeof(uint32_t));
                for (index2 = 0; index2 < lengthSize; index2++) {
                    memcpy(serviceInfo + index, &boxSize[lengthSize - index2 - 1], 1);
                    index++;
                }
                serviceInfoString = SUBSCRIPTIONID_BLOCKTYPE;
                memcpy(serviceInfo + index, serviceInfoString.c_str(), serviceInfoString.length());

                index += serviceInfoString.length();
                memcpy(serviceInfo + index, subscription_id.c_str(), subscription_id.length());
            }
        }

        in_service_info.bufstr = (uint8_t*)serviceInfo;
        in_service_id.length = service_id.length();
        in_service_id.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(service_id.data()));
    }

    service_token.length = atkn.len;
    service_token.bufstr = atkn.data;
    agentStatus = mHandler->setServiceToken(&mBBhandle,
                                            &service_token,
                                            &in_service_id,
                                            &in_service_info,
                                            &remaining_steps);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling setServiceToken (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    agentStatus = mHandler->processServiceToken(&mBBhandle, &http_request);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling processServiceToken (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!isEme()) {
        if (!createHttpRequestMessage(http_request, remaining_steps, request, url)) {
            LOGE("Could not generate request");
            status = ERROR_UNKNOWN;
            goto EXIT;
        }
    }
EXIT:
    MARLINLOG_EXIT();
    if (isEme()) {
        if (status != OK) {
            createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  remaining_steps,
                                  request);
        } else {
            createHttpRequestMessageForEME(http_request, remaining_steps, request, url);
        }
    }

    if (serviceInfo != NULL) {
        delete[] serviceInfo;
        serviceInfo = NULL;
    }
    if (convertAID != NULL) {
        delete[] convertAID;
        convertAID = NULL;
    }
    if (convertSID != NULL) {
        delete[] convertSID;
        convertSID = NULL;
    }
    if (in_request.bufstr != NULL) {
        delete[] in_request.bufstr;
        in_request.bufstr = NULL;
    }
    if (http_request.url != NULL) {
        delete[] http_request.url;
        http_request.url = NULL;
    }

    return status;
}

mcdm_status_t MBBLicense::processRightsResponse(mcdm_data_t& response,
                                                bool err,
                                                uint32_t& remaining_steps,
                                                string* message,
                                                mcdm_data_t* license,
                                                mcdm_data_t* service_info,
                                                bool& is_license_exist)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_httpResponse_t http_response;
    MH_string_t i_response =  {0, NULL};
    MH_actionResult_t result;
    is_license_exist = false;

    initActionResult(result);
    if (bb_license_status == BB_LICENSE_STATUS_UNINITIALIZED) {
        LOGE("BB Handle is not initialized.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (message == NULL || license == NULL || service_info == NULL) {
        LOGE("Input parameter is invalid.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (response.data != NULL && response.len > 0) {
        i_response.length = response.len;
        i_response.bufstr = response.data;
    }
    http_response.response = &i_response;
    http_response.error_flag = err;

    agentStatus = mHandler->processRightsResponse(&mBBhandle,
                                                  &http_response,
                                                  &remaining_steps,
                                                  &result);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling processServiceToken (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (result.result_type == RESULT_TYPE_LICENSE) {
        if (result.licensebuf == NULL
                || result.licensebuf->length <= 0
                || result.licensebuf->bufstr == NULL) {
            LOGE("Response is illegal.");
            status = ERROR_UNKNOWN;
            goto EXIT;
        }

        if (!isEme()) {
            if (!createLicenseMessage(result.licensebuf, remaining_steps, message)) {
                LOGE("Could not get rights.");
                status = ERROR_UNKNOWN;
                goto EXIT;
            }
        }

        license->data = new uint8_t[result.licensebuf->length];
        memcpy(license->data, result.licensebuf->bufstr, result.licensebuf->length);
        license->len = result.licensebuf->length;

        if (result.serviceinfobuf != NULL
                && result.serviceinfobuf->length > 0
                && result.serviceinfobuf->bufstr != NULL) {
            service_info->data = new uint8_t[result.serviceinfobuf->length];
            memcpy(service_info->data, result.serviceinfobuf->bufstr, result.serviceinfobuf->length);
            service_info->len = result.serviceinfobuf->length;
        }
        is_license_exist = true;
    }
EXIT:
    MARLINLOG_EXIT();
    if (isEme()) {
        uint32_t result_status = SUCCESS;

        if (status != OK) {
            result_status = FAIL;
        }
        createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                              status,
                              result_status,
                              remaining_steps,
                              message);
    }
    deleteActionResult(result);
    return status;
}

mcdm_status_t MBBLicense::processLTPMessage(mcdm_data_t& ltpMessage,
                                            string* license,
                                            bool& is_license_exist)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    MH_string_t in_ltp_message = {0, NULL};
    MH_string_t o_response = {0, NULL};
    MH_actionResult_t result;
    char* rHeader = new char[MTP_HEADER_SIZE + 1];
    MH_string_t o_ltp_message = {0, NULL};

    initActionResult(result);
    if (ltpMessage.len <= 0) {
        LOGE("account id is illegal.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    in_ltp_message.bufstr = ltpMessage.data;
    in_ltp_message.length = ltpMessage.len;

    agentStatus = mHandler->processLTPMessage(&mBBhandle,
                                              &in_ltp_message,
                                              &result,
                                              &o_response);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling processLTPMessage (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (result.result_type == RESULT_TYPE_LICENSE) {
        // store license
        mcdm_data_t store_license = {0, NULL};
        mcdm_data_t store_service_info = {0, NULL};
        if (result.licensebuf == NULL
                || result.licensebuf->length <= 0
                || result.licensebuf->bufstr == NULL) {
            LOGE("Response is illegal.");
            status = ERROR_UNKNOWN;
            goto EXIT;
        }

        store_license.data = result.licensebuf->bufstr;
        store_license.len = result.licensebuf->length;

        if (result.serviceinfobuf != NULL
                && result.serviceinfobuf->length > 0
                && result.serviceinfobuf->bufstr != NULL) {
            store_service_info.data = result.serviceinfobuf->bufstr;
            store_service_info.len = result.serviceinfobuf->length;
        }

        status = storeLicenseToDataBase(KEY_TYPE_OFFLINE,
                                        "",
                                        store_license,
                                        store_service_info);
        if (status != OK) {
            goto EXIT;
        }
        is_license_exist = true;
    }

    // create response header
    if (result.result_type == RESULT_TYPE_LICENSE) {
        MH_string_t *resultLicense = NULL;
        resultLicense = result.licensebuf;
        o_ltp_message.length = MTP_HEADER_SIZE*2 + o_response.length + resultLicense->length;
        o_ltp_message.bufstr = new uint8_t[o_ltp_message.length];
        memset(rHeader, 0x00, MTP_HEADER_SIZE + 1);
        SNPRINTF(rHeader, MTP_HEADER_SIZE + 1, "%06d",  o_response.length);
        memcpy(o_ltp_message.bufstr, (const char*)rHeader, MTP_HEADER_SIZE);

        memset(rHeader, 0x00, MTP_HEADER_SIZE + 1);
        SNPRINTF(rHeader, MTP_HEADER_SIZE + 1, "%06d",  o_response.length);
        memcpy(o_ltp_message.bufstr + MTP_HEADER_SIZE, (const char*)rHeader, MTP_HEADER_SIZE);

        // get License Infomation
        memcpy(o_ltp_message.bufstr + MTP_HEADER_SIZE*2, (const char*)o_response.bufstr, o_response.length);
        memcpy(o_ltp_message.bufstr + MTP_HEADER_SIZE*2 + o_response.length, (const char*)resultLicense->bufstr, resultLicense->length);
    } else {
        LOGV("OUTPUT DATA LENGTH:%d \n", o_response.length);
        string reserve("000000");
        o_ltp_message.length = MTP_HEADER_SIZE + o_response.length + reserve.length();
        o_ltp_message.bufstr = new uint8_t[o_ltp_message.length];
        memset(rHeader, 0x00, MTP_HEADER_SIZE + 1);
        SNPRINTF(rHeader, MTP_HEADER_SIZE + 1, "%06d",  o_response.length);
        memcpy(o_ltp_message.bufstr, (const char*)rHeader, MTP_HEADER_SIZE);
        memcpy(o_ltp_message.bufstr + MTP_HEADER_SIZE, (const char*)reserve.data(), reserve.length());
        memcpy(o_ltp_message.bufstr + MTP_HEADER_SIZE + reserve.length(),(const char*)o_response.bufstr, o_response.length);
    }
    if (!createLTPMessage(&o_ltp_message, license)) {
        LOGE("Could not get ltp message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    deleteActionResult(result);
    if (o_ltp_message.bufstr != NULL) {
        delete[] o_ltp_message.bufstr;
        o_ltp_message.bufstr = NULL;
    }
    if (o_response.bufstr != NULL) {
        delete[] o_response.bufstr;
        o_response.bufstr = NULL;
    }
    delete[] rHeader;
    rHeader = NULL;

    return status;
}

mcdm_status_t MBBLicense::getRegisteredStatus(const string& service_id,
                                              const string& account_id,
                                              string& registeredstate)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    mcdm_registered_state state = STATE_ACCOUNT_NOT_REGISTERED;
    string strState("");

    bool isGetLinkAccount = false;
    bool isGetLinkOtherLink = false;
    MH_string_t in_service_id = {0, NULL};
    MH_string_t in_service_info = {0, NULL};
    string service_info;

    size_t lengthSize = 0, accountBoxSize = 0;
    uint8_t boxSize[sizeof(uint32_t)] = {0x00};
    uint32_t index = 0;

    if (bb_license_status == BB_LICENSE_STATUS_UNINITIALIZED) {
        LOGE("BB Handle is not initialized.");
        state = STATE_UNKNOWN;
        goto EXIT;
    }
    if (account_id.length() <= 0) {
        LOGE("account id is illegal.");
        state = STATE_UNKNOWN;
        goto EXIT;
    }

    accountBoxSize = sizeof(uint32_t) + ACCOUNTID_BLOCKTYPE.length() + DOMAINID_ACCFIELD_SIZE;
    in_service_info.length = accountBoxSize;
    uint8_t serviceInfo[in_service_info.length];
    lengthSize = sizeof(uint32_t);
    memset(&serviceInfo, 0x00, in_service_info.length);

    if (account_id.length() > 0) {
        unsigned char convertID[DOMAINID_ACCFIELD_SIZE] = {0};
        if (convertAccountID(account_id, convertID)) {
        memset(&boxSize, 0x00, sizeof(lengthSize));
        memcpy(&boxSize, &accountBoxSize, sizeof(uint32_t));
        for (index=0; index < lengthSize; index++) {
            memcpy(serviceInfo+index, &boxSize[lengthSize-index-1], 1);
        }
        service_info = ACCOUNTID_BLOCKTYPE;
        memcpy(serviceInfo+index, service_info.c_str(), service_info.length());
        index += service_info.length();
        memcpy(serviceInfo+index, convertID, DOMAINID_ACCFIELD_SIZE);
        }
    }

    in_service_info.bufstr = (uint8_t*)serviceInfo;
    in_service_id.length = service_id.length();
    in_service_id.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(service_id.data()));

    isGetLinkAccount = isLinkAccount(in_service_id, in_service_info);
    if (isGetLinkAccount) {
        state = STATE_ACCOUNT_ALREADY_REGISTERED;
        LOGV("calling isLinkAccount. The account ID already registered.");
    } else {
        isGetLinkOtherLink = isOtherLink();
        if (isGetLinkOtherLink) {
            state = STATE_ALREADY_REGISTERED_BY_ANOTHER_ACCOUNT;
            LOGE("calling isOtherLink. Already registered by another account.");
            goto EXIT;
        } else {
            state = STATE_ACCOUNT_NOT_REGISTERED;
            LOGV("Not yet registered.");
            goto EXIT;
        }
    }

    if (!convertRegisteredStatusToStr(state, strState)) {
        LOGE("Could not get state message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (!createPropertyMessage(strState, &registeredstate)) {
        LOGE("Could not get state message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBLicense::getRegisteredStatus(const string& service_id,
                                              const string& account_id,
                                              const string& subscription_id,
                                              string& registeredstate)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    mcdm_registered_state state = STATE_ACCOUNT_NOT_REGISTERED;
    string strState("");

    bool isGetLinkAccount = false;
    bool isGetLinkOtherLink = false;
    MH_string_t in_service_id = {0, NULL};
    MH_string_t in_service_info = {0, NULL};
    string serviceInfostring;
    size_t lengthSize = 0, accountBoxSize = 0, subscriptionBoxSize = 0;
    uint8_t boxSize[sizeof(uint32_t)] = {0x00};
    uint32_t index = 0, index2 = 0;


    if (bb_license_status == BB_LICENSE_STATUS_UNINITIALIZED) {
        LOGE("BB Handle is not initialized.");
        state = STATE_UNKNOWN;
        goto EXIT;
    }

    if (account_id.length() > 0) {
        accountBoxSize = sizeof(uint32_t) + ACCOUNTID_BLOCKTYPE.length() + DOMAINID_ACCFIELD_SIZE;
    }
    if (subscription_id.length() > 0) {
        subscriptionBoxSize = sizeof(uint32_t) + SUBSCRIPTIONID_BLOCKTYPE.length() + DOMAINID_SUBSFIELD_SIZE;
    }

    in_service_info.length = accountBoxSize + subscriptionBoxSize;
    uint8_t serviceInfo[in_service_info.length];
    lengthSize = sizeof(uint32_t);
    memset(&serviceInfo, 0x00, in_service_info.length);

    if (account_id.length() > 0) {
        unsigned char convertID[DOMAINID_ACCFIELD_SIZE] = {0};
        if (convertAccountID(account_id, convertID)) {
            memset(&boxSize, 0x00, sizeof(lengthSize));
            memcpy(&boxSize, &accountBoxSize, sizeof(uint32_t));
            for (index=0; index < lengthSize; index++) {
                memcpy(serviceInfo+index, &boxSize[lengthSize-index-1], 1);
            }
            serviceInfostring = ACCOUNTID_BLOCKTYPE;
            memcpy(serviceInfo+index, serviceInfostring.c_str(), serviceInfostring.length());
            index += serviceInfostring.length();
            memcpy(serviceInfo+index, convertID, DOMAINID_ACCFIELD_SIZE);
        }
    }

    if (subscription_id.length() > 0) {
        unsigned char convertID[DOMAINID_ACCFIELD_SIZE] = {0};
        if (convertSubscriptionID(subscription_id, convertID)) {
            index = accountBoxSize;
            serviceInfostring = string("");
            memset(&boxSize, 0x00, sizeof(boxSize));
            memcpy(&boxSize, &subscriptionBoxSize, sizeof(uint32_t));
            for (index2 = 0; index2 < lengthSize; index2++) {
                memcpy(serviceInfo + index, &boxSize[lengthSize - index2 - 1], 1);
                index++;
            }
            serviceInfostring = SUBSCRIPTIONID_BLOCKTYPE;
            memcpy(serviceInfo + index, serviceInfostring.c_str(), serviceInfostring.length());

            index += serviceInfostring.length();
            memcpy(serviceInfo + index, subscription_id.c_str(), subscription_id.length());
        }
    }

    in_service_info.bufstr = (uint8_t*)serviceInfo;
    in_service_id.length = service_id.length();
    in_service_id.bufstr = reinterpret_cast<uint8_t*>(const_cast<char*>(service_id.data()));

    isGetLinkAccount = isLinkAccount(in_service_id, in_service_info);
    if (isGetLinkAccount) {
        state = STATE_ACCOUNT_ALREADY_REGISTERED;
        LOGV("calling isLinkAccount. The account ID already registered.");
    } else {
        isGetLinkOtherLink = isOtherLink();
        if (isGetLinkOtherLink) {
            state = STATE_ALREADY_REGISTERED_BY_ANOTHER_ACCOUNT;
            LOGV("calling isOtherLink. Already registered by another account.");
        } else {
            state = STATE_ACCOUNT_NOT_REGISTERED;
            LOGV("Not yet registered.");
        }
    }

    if (!convertRegisteredStatusToStr(state, strState)) {
        LOGE("Could not get state message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    if (!createPropertyMessage(strState, &registeredstate)) {
        LOGE("Could not get state message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBLicense::queryLicenseStatus(map<string, string>& licenseStatus)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    bool isValid = false;
    MH_8pusInfo_t octpusInfo;
    MH_licenseInfo_t licenseInfo;
    map<string, string> temporalLicenseInfo, counterLicenseInfo, trustedTimeInfo;

    allocate8pusInfo(octpusInfo);
    if (license_status == LICENSE_STATUS_UNINITIALIZED) {
        LOGD("license handle is not initilized.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    agentStatus = mHandler->isLicenseValid(&mLicenseHandle, isValid);
    switch (agentStatus) {
    case MH_ERR_OK:
        licenseStatus[LICENSE_METADATA_KEY_SATUS] = LICENSE_METADATA_VALUE_SATUS_VALID;
        break;
    case MH_ERR_LICENSE_EXPIRED:
        licenseStatus[LICENSE_METADATA_KEY_SATUS] = LICENSE_METADATA_VALUE_SATUS_EXPIRED;
        LOGD("calling checkLicense. RIGHTS_EXPIRED");
        break;
    case MH_ERR_INVALID_LICENSE:
    default:
        licenseStatus[LICENSE_METADATA_KEY_SATUS] = LICENSE_METADATA_VALUE_SATUS_INVALID;
        LOGD("calling checkLicense. RIGHTS_INVALID");
        break;
    }
    if (agentStatus != MH_ERR_OK) {
        goto EXIT;
    }

    agentStatus = mHandler->queryLicenseStatus(mLicenseHandle, &octpusInfo, &licenseInfo);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling queryLicenseStatus (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (convert8pusToTemporalLicenseInfo(octpusInfo, temporalLicenseInfo)) {
        licenseStatus.insert(temporalLicenseInfo.begin(), temporalLicenseInfo.end());
    }
    if (convert8pusToCounterLicenseInfo(octpusInfo, counterLicenseInfo)) {
        licenseStatus.insert(counterLicenseInfo.begin(), counterLicenseInfo.end());
    }
    if (convertTrustedTimeToLicenseInfo(trustedTimeInfo)) {
        licenseStatus.insert(trustedTimeInfo.begin(), trustedTimeInfo.end());
    }
EXIT:
    MARLINLOG_EXIT();
    free8pusInfo(octpusInfo);
    return status;
}

bool MBBLicense::isLinkAccount(MH_string_t i_service_id,
                               MH_string_t i_service_info)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MH_status_t retCode = MH_ERR_UNKNOWN;
    MH_linkList_t *outLinks = NULL;

    retCode = mHandler->getLinksForServiceID(&mBBhandle,
                                             i_service_id,
                                             i_service_info,
                                             &outLinks);
    if (MH_ERR_OK != retCode) {
        LOGE("calling getLinksForServiceID. return code(%d)", retCode);
        goto EXIT;
    }
    if (NULL == outLinks) {
        LOGV("calling getLinksForServiceID. outLinks object pointer is NULL.");
        goto EXIT;
    }

    ret = true;
EXIT:
    MARLINLOG_EXIT();
    if (NULL != outLinks) {
        freeLinklist(outLinks);
        outLinks = NULL;
    }
    return ret;
}

bool MBBLicense::isOtherLink(void)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MH_status_t retCode = MH_ERR_UNKNOWN;
    MH_linkList_t *outLinks = NULL;

    retCode = mHandler->getAvailableLinks(&mBBhandle, &outLinks);
    if (MH_ERR_OK != retCode) {
        LOGE("calling getAvailableLinks. return code(%d)", retCode);
        goto EXIT;
    }
    if (NULL == outLinks) {
        LOGV("calling getAvailableLinks. outLinks object pointer is NULL.");
        goto EXIT;
    }

    ret = true;
EXIT:
    MARLINLOG_EXIT();
    if (NULL != outLinks) {
        freeLinklist(outLinks);
        outLinks = NULL;
    }
    return ret;
}

void MBBLicense::freeLinklist(MH_linkList_t* io_links)
{
    MH_linkList_t  *link = io_links;
    MH_linkList_t  *next = NULL;

    while (NULL != link) {
        next = link->next_link;
        if (link->from_id != NULL) {
            if(link->from_id->bufstr != NULL) {
                free(link->from_id->bufstr);
            }
        }
        if (link->to_id != NULL) {
            if(link->to_id->bufstr != NULL) {
                free(link->to_id->bufstr);
            }
        }
        free(link);
        link = next;
    }
}

bool MBBLicense::convertAccountID(const string& accountIdStr,
                                  unsigned char* accountIdBin)
{
    return strToBin(accountIdStr.c_str(), accountIdBin, DOMAINID_ACCFIELD_SIZE*2);
}

bool MBBLicense::convertSubscriptionID(const string& subscriptionIdStr,
                                       unsigned char* subscriptionIdBin)
{
    return strToBin(subscriptionIdStr.c_str(), subscriptionIdBin, DOMAINID_SUBSFIELD_SIZE*2);
}

bool MBBLicense::convert8pusToTemporalLicenseInfo(const MH_8pusInfo_t& i_8pusInfo,
                                                  map<string, string>& o_info)
{
    MARLINLOG_ENTER();
    bool ret = false;
    CTime time1, time2;
    CTime time1end, time2end;
    string constraintValue("");
    char temporalData[MAX_WIDTH_OF_INT + 1] = {0x00};

    LOGD("*****flags:[%x]*****\n", i_8pusInfo.temporal_c.flags);
    // The below block is for notbefore[]
    if ((MH_8PUS_TEMPORAL_FLAG_NOTBEFORE & i_8pusInfo.temporal_c.flags) != 0) {
        LOGD("MH_8PUS_TEMPORAL_FLAG_NOTBEFORE[%x]\n", MH_8PUS_TEMPORAL_FLAG_NOTBEFORE);
        time1.reset();
        time2.reset();
        if ((0 < i_8pusInfo.temporal_c.notbefore_count) &&
            (NULL != i_8pusInfo.temporal_c.notbefore[0])) {
            // before first use or after first use
            time1 = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notbefore[0]);
        }
        if ((2 == i_8pusInfo.temporal_c.notbefore_count) &&
            (NULL != (MH_date_t*)i_8pusInfo.temporal_c.notbefore[1])) {
            // after first use
            time2 = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notbefore[1]);
        }
        LOGD("*****time1 tm_year:[%d]*****\n", time1.getGmTime()->tm_year);
        if (0 == time1.getGmTime()->tm_year) {
            LOGE("time1(start time) is NULL.");
            goto EXIT;
        }
        LOGD("*****time2 tm_year:[%d]*****\n", time2.getGmTime()->tm_year);
        if (0 == time2.getGmTime()->tm_year) {
            // before first use
            o_info[MBB_LICENSE_METADATA_LICENSE_START_TIME] = time1.getUtcAsString();
        } else {
            // after first use
            if (time1 <= time2) {
                // order is
                // 0 : purchase date
                // 1 : date of first use
                o_info[MBB_LICENSE_METADATA_LICENSE_START_TIME] = time1.getUtcAsString();
                o_info[MBB_LICENSE_METADATA_PLAY_TIME_1ST_USE] = time2.getUtcAsString();
            } else {
                // order is
                // 0 : date of first use
                // 1 : purchase date
                o_info[MBB_LICENSE_METADATA_PLAY_TIME_1ST_USE] = time1.getUtcAsString();
                o_info[MBB_LICENSE_METADATA_LICENSE_START_TIME] = time2.getUtcAsString();
            }
        }
    }

    // The below block is for notafter[]
    if ((MH_8PUS_TEMPORAL_FLAG_NOTAFTER & i_8pusInfo.temporal_c.flags) != 0) {
        time1end.reset();
        time2end.reset();
        LOGD("MH_8PUS_TEMPORAL_FLAG_NOTAFTER[%x]\n", MH_8PUS_TEMPORAL_FLAG_NOTAFTER);
        if ((0 < i_8pusInfo.temporal_c.notafter_count) &&
            (NULL != i_8pusInfo.temporal_c.notafter[0])) {
            time1end = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notafter[0]);
        }
        if ((2 == i_8pusInfo.temporal_c.notafter_count) &&
            (NULL != i_8pusInfo.temporal_c.notafter[1])) {
            time2end = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notafter[1]);
        }
        LOGD("*****time1end tm_year:[%d]*****\n", time1end.getGmTime()->tm_year);
        if (0 == time1end.getGmTime()->tm_year) {
            LOGE("time1(end time) is NULL.");
            goto EXIT;
        }
        LOGD("*****time2end tm_year:[%d]*****\n", time2end.getGmTime()->tm_year);
        if (0 == time2end.getGmTime()->tm_year) {
            o_info[MBB_LICENSE_METADATA_LICENSE_END_TIME] = time1end.getUtcAsString();
        } else {
            LOGD("notafter time1end= %s", time1end.getUtcAsString().c_str());
            LOGD("notafter time2end= %s", time2end.getUtcAsString().c_str());

            if (time1end <= time2end) {
                o_info[MBB_LICENSE_METADATA_PLAY_END_TIME_1ST_USE] = time1end.getUtcAsString();
                o_info[MBB_LICENSE_METADATA_LICENSE_END_TIME] = time2end.getUtcAsString();
            } else {
                o_info[MBB_LICENSE_METADATA_LICENSE_END_TIME] = time1end.getUtcAsString();
                o_info[MBB_LICENSE_METADATA_PLAY_END_TIME_1ST_USE] = time2end.getUtcAsString();
            }
        }
    }

    // The below block is for notduring[0]
    if ((MH_8PUS_TEMPORAL_FLAG_NOTDURING & i_8pusInfo.temporal_c.flags) != 0) {
        LOGD("MH_8PUS_TEMPORAL_FLAG_NOTDURING[%x]\n", MH_8PUS_TEMPORAL_FLAG_NOTDURING);
        time1.reset();
        if (NULL != i_8pusInfo.temporal_c.notduring[0]) {
            time1 = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notduring[0]);
        }
        if (0 == time1.getGmTime()->tm_year) {
            goto EXIT;
        }
        o_info[MBB_LICENSE_METADATA_PLAY_START_TIME_PERIOD] = time1.getUtcAsString();
        time1.reset();
        if (NULL != i_8pusInfo.temporal_c.notduring[1]) {
            time1 = CTime((mcdm_date_t*)i_8pusInfo.temporal_c.notduring[1]);
        }
        if (0 == time1.getGmTime()->tm_year) {
            goto EXIT;
        }
        o_info[MBB_LICENSE_METADATA_PLAY_END_TIME_PERIOD] = time1.getUtcAsString();
    }

    // The below block is for notlongerthan
    if ((MH_8PUS_TEMPORAL_FLAG_NOTLONGER & i_8pusInfo.temporal_c.flags) != 0) {
        LOGD("MH_8PUS_TEMPORAL_FLAG_NOTLONGER[%x]\n", MH_8PUS_TEMPORAL_FLAG_NOTLONGER);
        SNPRINTF(temporalData, MAX_WIDTH_OF_INT + 1, "%d", i_8pusInfo.temporal_c.notlongerthan);
        constraintValue = temporalData;
        o_info[MBB_LICENSE_METADATA_PLAY_AVAILABLE_TIME] = constraintValue;
    }
    // The below block is for notmorethan
    if ((MH_8PUS_TEMPORAL_FLAG_NOTMORE & i_8pusInfo.temporal_c.flags) != 0) {
        LOGD("MH_8PUS_TEMPORAL_FLAG_NOTMORE[%x]\n", MH_8PUS_TEMPORAL_FLAG_NOTMORE);
        SNPRINTF(temporalData, MAX_WIDTH_OF_INT + 1, "%d", i_8pusInfo.temporal_c.notmorethan);
        constraintValue = temporalData;
        o_info[MBB_LICENSE_METADATA_LICENSE_AVAILABLE_TIME] = constraintValue;
    }
    ret = true;
EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool MBBLicense::convert8pusToCounterLicenseInfo(const MH_8pusInfo_t& i_8pusInfo,
                                                 map<string, string>& o_info)
{
    MARLINLOG_ENTER();
    char valueBuffer[MAX_WIDTH_OF_INT] = {0x00};
    string constraintValue("");

    if ((MH_8PUS_COUNTER_FLAG_REPEATCOUNT & i_8pusInfo.counter_c.flags) != 0) {
        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.counter_c.repeat_count_max);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_MAX_REPEAT_CNT] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.counter_c.repeat_count_remain);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_REMAINING_REPEAT_CNT] = constraintValue;
    }
    MARLINLOG_EXIT();
    return true;
}

bool MBBLicense::convert8pusToOutputControlInfo(const MH_8pusInfo_t& i_8pusInfo,
                                                map<string, string>& o_info)
{
    MARLINLOG_ENTER();
    char valueBuffer[MAX_WIDTH_OF_INT] = {0x00};
    string constraintValue("");

    LOGD("*****flags:[%x]*****\n", i_8pusInfo.permission_params.flags);
    if ((MH_8PUS_OUTPUTCONTROL_FLAG_BASIC & i_8pusInfo.permission_params.flags) != 0) {
        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.basic.aps);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_BASIC_EPN] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.basic.cci);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_BASIC_CCI] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.basic.dot);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_BASIC_DOT] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.basic.epn);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_BASIC_EPN] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.basic.ict);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_BASIC_ICT] =  constraintValue;
    }

    if ((MH_8PUS_OUTPUTCONTROL_FLAG_DTCP & i_8pusInfo.permission_params.flags) != 0) {
        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.aps);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_EPN] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.cci);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_CCI] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.epn);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_EPN] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.ict);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_ICT] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.rmm);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_RMM] =  constraintValue;

        SNPRINTF(valueBuffer, MAX_WIDTH_OF_INT, "%d", i_8pusInfo.permission_params.e.dtcp.rs);
        constraintValue = valueBuffer;
        o_info[MBB_LICENSE_METADATA_OCI_DTCP_RS] =  constraintValue;
    }
    MARLINLOG_EXIT();
    return true;
}

bool MBBLicense::convertTrustedTimeToLicenseInfo(map<string, string>& o_info)
{
    MARLINLOG_ENTER();
    bool ret = false;
    MH_status_t status = MH_ERR_OK;
    MH_date_t io_date;
    mcdm_date_t date;
    string ttstring;
    CTime trustedCurrentTime;

    status = mHandler->getTrustedTime(mHandle, &io_date);
    if ( status != MH_ERR_OK ) {
        LOGD("calling getTrustedTime. return code %d ", status);
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
    if (0 == trustedCurrentTime.getGmTime()->tm_year) {
        LOGE("trustedCurrentTime is NULL.");
        goto EXIT;
    }
    ttstring = trustedCurrentTime.getUtcAsString();
    LOGD("trustedCurrentTime - %s", ttstring.c_str());
    o_info[MBB_LICENSE_METADATA_PLAY_TRUSTED_TIME] =  trustedCurrentTime.getUtcAsString();

    ret = true;
EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool MBBLicense::convertRegisteredStatusToStr(const mcdm_registered_state state, string& o_state)
{
    MARLINLOG_ENTER();
    bool ret = false;

    switch(state) {
    case STATE_ACCOUNT_NOT_REGISTERED:
        o_state.assign(NOT_REGISTERED);
        break;
    case STATE_ACCOUNT_ALREADY_REGISTERED:
        o_state.assign(ALREADY_REGISTERED);
        break;
    case STATE_ALREADY_REGISTERED_BY_ANOTHER_ACCOUNT:
        o_state.assign(ANOTHER_REGISTERED);
        break;
    default:
        LOGE("Registered Status Unknown");
        goto EXIT;
    }
    ret = true;
EXIT:
    MARLINLOG_EXIT();
    return ret;
}
