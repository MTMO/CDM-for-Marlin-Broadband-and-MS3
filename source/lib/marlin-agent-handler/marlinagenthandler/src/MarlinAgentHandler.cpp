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

#define LOG_TAG "MarlinAgentHandler"
#include "MarlinAgentLog.h"

#include <stdarg.h>
#include "MarlinAgentHandler.h"
#include "MarlinCdmUtils.h"
#include "CMutex.h"

using namespace marlincdm;

static CMutex mutexLicenseHandle;
static CMutex mutexMS3Handle;

MarlinAgentHandler::MarlinAgentHandler()
{
}

MarlinAgentHandler::~MarlinAgentHandler()
{
}

MH_status_t MarlinAgentHandler::initAgent(MH_agentHandle_t* o_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    mutexLicenseHandle.lock();
    /* Add marlin agent specific call if needed */
    mutexLicenseHandle.unlock();
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::finAgent(MH_agentHandle_t* io_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    mutexLicenseHandle.lock();
    /* Add marlin agent specific call if needed */
    mutexLicenseHandle.unlock();
    LOG_EXIT();
    return retCode;
}

uint32_t MarlinAgentHandler::getRefCount(void)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::increaseRefCount(void)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::decreaseRefCount(void)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::initBBHandle(MH_bbHandle_t* o_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

void MarlinAgentHandler::finBBHandle(MH_bbHandle_t* io_handle)
{
    /* Add marlin agent specific call if needed */
}

MH_status_t MarlinAgentHandler::initLicenseHandle(MH_agentHandle_t i_handle, MH_licenseHandle_t* o_license_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    mutexLicenseHandle.lock();
    /* Add marlin agent specific call if needed */
    mutexLicenseHandle.unlock();
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::initMS3Handle(MH_ms3Handle_t* o_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    mutexMS3Handle.lock();
    /* Add marlin agent specific call if needed */
    mutexMS3Handle.unlock();
    LOG_EXIT();
    return retCode;
}

void MarlinAgentHandler::finMS3Handle(MH_ms3Handle_t* io_handle)
{
    LOG_ENTER();
    mutexMS3Handle.lock();
    /* Add marlin agent specific call if needed */
    mutexMS3Handle.unlock();
    LOG_EXIT();
}

MH_status_t MarlinAgentHandler::finLicenseHandle(MH_licenseHandle_t* io_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    mutexLicenseHandle.lock();
    /* Add marlin agent specific call if needed */
    mutexLicenseHandle.unlock();
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::initTrack(MH_agentHandle_t i_handle,
                                          MH_licenseHandle_t* i_licenseHandle,
                                          MH_trackInfo_t* io_track,
                                          MH_trackHandle_t* o_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::finTrack(MH_agentHandle_t i_handle, MH_licenseHandle_t* i_licenseHandle, MH_trackHandle_t* io_handle)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::isLicenseValid(MH_licenseHandle_t* i_handle, bool& ret)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getTrustedTime(MH_agentHandle_t i_handle, MH_date_t* o_date)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::updateTrustedTime(MH_agentHandle_t i_handle, MH_date_t* o_date)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::getLinksForServiceID(MH_bbHandle_t* i_handle, MH_string_t i_service_id, MH_string_t i_service_info, MH_linkList_t** io_links)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getAvailableLinks(MH_bbHandle_t* i_handle, MH_linkList_t** io_links)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getReachableNodes(MH_licenseHandle_t i_handle, MH_linkInfo_t i_link, MH_nodeInfo_t** io_node_info)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::getLinkPeriod(MH_licenseHandle_t i_handle, MH_linkInfo_t* i_from_id, MH_linkInfo_t* i_to_id, MH_date_t* o_date_before, MH_date_t* o_date_after)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::setServiceToken(MH_bbHandle_t* i_handle,
                                                MH_string_t* i_service_token,
                                                MH_string_t* i_service_id,
                                                MH_string_t* i_account_id,
                                                uint32_t* o_steps)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::processServiceToken(MH_bbHandle_t* i_handle, MH_httpRequest_t* o_request)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::processRightsResponse(MH_bbHandle_t* i_handle,
                                                      MH_httpResponse_t* i_response,
                                                      uint32_t* o_steps,
                                                      MH_actionResult_t* o_result)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::generateSASRequest(MH_ms3Handle_t* i_handle,
                                                   MH_sasTriggerData_t* i_trigger_data,
                                                   MH_sasTriggerInfo_t* o_trigger_info,
                                                   MH_httpRequest_t* o_request)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::processSASResponse(MH_ms3Handle_t* i_handle,
                                                   MH_sasTriggerInfo_t* i_trigger_info,
                                                   MH_httpResponse_t* i_response,
                                                   MH_sasResultData_t* o_result)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::processLTPMessage(MH_bbHandle_t* i_handle,
                                                  MH_string_t* i_ltpMassage,
                                                  MH_actionResult_t* o_result,
                                                  MH_string_t* o_response)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getLicense(MH_licenseHandle_t i_handle, MH_string_t* licensebuf)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::queryLicenseStatus(MH_licenseHandle_t i_handle, MH_8pusInfo_t* o_8pusInfo, MH_licenseInfo_t* o_info)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::setLicense(MH_licenseHandle_t i_handle, MH_licenseInfo_t* o_info)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::clearLicense(MH_licenseHandle_t i_handle)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::evaluateLicense(MH_licenseHandle_t i_handle, MH_8pusInfo_t* o_8pusInfo)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::retrieveContentKey(MH_licenseHandle_t i_handle,
                                                   MH_trackHandle_t* i_track,
                                                   MH_memdata_t* i_data,
                                                   bool* islastflg,
                                                   MH_memdata_t* o_data)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::isKeySystemSupported(MH_keysystemId_t& i_id, bool* out_is_suported)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::decrypt(MH_licenseHandle_t* i_licenseHandle,
                    MH_trackHandle_t* i_trackHandle, bool is_encrypted,
                    const uint8_t key[16], MH_memdata_t* i_iv,
                    MH_buffer_t* srcBuf, MH_subSample_t* SubSamples,
                    size_t& numSubSamples, MH_buffer_t* dstPtr,
                    size_t& decrypt_buffer_length, string &errorDetailMsg)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::decryptWithHW(MH_licenseHandle_t* i_licenseHandle,
                                              MH_trackHandle_t* i_trackHandle, bool is_encrypted,
                                              const uint8_t key[16], MH_memdata_t* i_iv,
                                              MH_buffer_t* srcBuf, MH_subSample_t* SubSamples,
                                              size_t& numSubSamples, MH_buffer_t* dstPtr,
                                              size_t& decrypt_buffer_length, string &errorDetailMsg)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::checkPersonalizationStatus(MH_agentHandle_t i_handle)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::processOfflinePersonalization(MH_agentHandle_t i_handle, MH_string_t* i_personalities)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::setPersonalizeToken(MH_agentHandle_t i_handle, MH_string_t* i_personalities)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::getPersonalizeRequest(MH_agentHandle_t i_handle, MH_string_t* i_personalities, MH_string_t* o_request)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::processPersonalizeResponse(MH_agentHandle_t i_handle, MH_string_t* i_response)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::getContentIdByLicense(MH_string_t* i_license, MH_cid_t* o_cid)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getContentIdsByLicense(MH_string_t* i_license, vector<string> *o_cids)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getContentIdByIpmp(MH_memdata_t i_sinfheader, MH_cid_t* o_cid)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getContentIdByPssh(MH_memdata_t i_psshbox, const uint8_t* i_kid, MH_cid_t* o_cid)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getContentIdByMPD(MH_string_t i_mpd, MH_cid_t* o_cid)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::isTypeSupported(MH_string_t i_keysystem,  MH_string_t i_initdatatype, MH_string_t i_contenttype, MH_string_t i_capability, MH_string_t* o_istypesupportedresult)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}


MH_status_t MarlinAgentHandler::getPropertyString(MH_string_t i_name,  MH_string_t* io_value)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::getDeviceProperty(MH_string_t* io_value)
{
    LOG_ENTER();
    MH_status_t retCode = MH_ERR_OK;
    /* Add marlin agent specific call if needed */
    LOG_EXIT();
    return retCode;
}

MH_status_t MarlinAgentHandler::setPropertyString(MH_string_t* io_name, MH_string_t* io_value)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::getPropertyByteArray(MH_string_t i_name,  MH_memdata_t* io_value)
{
    return MH_ERR_OK;
}

MH_status_t MarlinAgentHandler::setPropertyByteArray(MH_string_t* io_name,  MH_memdata_t* io_value)
{
    return MH_ERR_OK;
}


