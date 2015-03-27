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

#ifndef __MARLIN_AGENT_HANDLER_H__
#define __MARLIN_AGENT_HANDLER_H__

#include "MarlinAgentHandlerType.h"

#include <vector>

/**
 * @brief
 * Marlin Content Decryption Module(CDM) is the Module which is created for adapting to
 * Android Framework and HTML Encrypted Media Extensions(EME).
 * @image html marlincdm_overview.jpg
 *
 * Marlin CDM includes below part of implementation.
 * - Marlin Drm Plugin : It is glue code to absorb API difference between Marlin CDM Interface and Android Drm Framework.
 * - Marlin CDM Adapter : IIt is glue code to absorb API difference between Marlin CDM Interface and each browsers EME.
 * - Marlin CDM Interface : It is interface layer between Android or Web browser framework and Marlin CDM.
 * - Marlin CDM Engine : It is implementation to delegate HTTP transaction responsibility with License server to Application. It also has session and key management for each application call
 * - Marlin Agent Handler : It is a Porting layer to support Marlin DRM solution in conjunction with specialized SDK. With this layer customization, implementer can use any Marlin DRM Agent.
 *
 * @version 1.0
 */

namespace marlincdm {

/**
 * @brief
 * This is adaptation class in Marlin CDM, which links to Marlin DRM Agent.
 *
 */
class MarlinAgentHandler {

public:

  /**
   * @brief Construct MarlinAgentHandler.
   *
   * @param[in] void
   *
   */
  MarlinAgentHandler();
  virtual ~MarlinAgentHandler();

  /**
   * @brief Load Marlin Agent.
   *
   * @param [in] o_handle Agent handle
   *
   * @retval MH_ERR_OK Decryption is success
   * @retval MH_ERR_FAILURE Cannot initialize Agent
   */
  MH_status_t initAgent(MH_agentHandle_t* o_handle);

  /**
   * @brief Unload Marlin Agent. \n It is necessary for you to call initAgent() function before calling this function.
   *
   * @param [in] io_handle Agent handle
   *
   * @retval MH_ERR_OK Decryption is success
   * @retval MH_ERR_FAILURE Cannot finalize Agent
   */
  MH_status_t finAgent(MH_agentHandle_t* io_handle);

  /**
   * @brief Get reference count.
   * @param[in] void
   *
   * @retval current reference count
   */
  uint32_t getRefCount(void);

  /**
   * @brief Increase reference count.
   * @param[in] void
   *
   * @retval MH_ERR_OK Increasing RefCount is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t increaseRefCount(void);

  /**
   * @brief Decrease reference count.
   * @param[in] void
   *
   * @retval MH_ERR_OK Decreasing RefCount is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t decreaseRefCount(void);

  /**
   * @brief Initialize Marlin BB session.
   *
   * @param [out] o_handle MBB license handle
   *
   * @retval MH_ERR_OK Intialize BBHandle is success
   * @retval MH_ERR_FAILURE Cannot initialize bbhandle
   */
  MH_status_t initBBHandle(MH_bbHandle_t* o_handle);

  /**
   * @brief Finalize marlin BB session. \n
   *
   * @param [in] io_handle MBB license handle
   *
   * @retval MH_ERR_OK Intialize BBHandle is success
   * @retval MH_ERR_FAILURE Cannot finalize bbhandle
   */
  void finBBHandle(MH_bbHandle_t* io_handle);

  /**
   * @brief Initialize Marlin MS3 session.
   *
   * @param [out] o_handle MS3 license handle
   *
   * @retval MH_ERR_OK Intialize MS3Handle is success
   * @retval MH_ERR_FAILURE Cannot initialize bbhandle
   */
  MH_status_t initMS3Handle(MH_ms3Handle_t* o_handle);

  /**
   * @brief Finalize marlin MS3 session. \n
   *
   * @param [in] io_handle MS3 license handle
   *
   * @retval MH_ERR_OK Intialize MS3Handle is success
   * @retval MH_ERR_FAILURE Cannot finalize bbhandle
   */
  void finMS3Handle(MH_ms3Handle_t* io_handle);

  /**
   * @brief Initialize license session.
   *
   * @param[in] i_handle Agent handle
   * @param[out] o_license_handle MBB license handle
   *
   * @retval MH_ERR_OK initialize LicenseHandle is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t initLicenseHandle(MH_agentHandle_t i_handle, MH_licenseHandle_t* o_license_handle);

  /**
   * @brief Finalize license session.
   *
   * @param[in] io_handle licensehandle
   *
   * @retval MH_ERR_OK Finalize LicenseHandle is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t finLicenseHandle(MH_licenseHandle_t* io_handle);

  /**
   * @brief Initialize track session.
   *
   * @param [in] i_handle Agant handle
   * @param [in] i_licenseHandle MBB license handle
   * @param [in] io_track Track info
   * @param [in] o_handle Track handle
   *
   * @retval MH_ERR_OK Processing RightsResponse is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t initTrack(MH_agentHandle_t i_handle, MH_licenseHandle_t* i_licenseHandle, MH_trackInfo_t* io_track, MH_trackHandle_t* o_handle);

  /**
   * @brief Finalize track session.
   *
   * @param [in] i_handle agent handle
   * @param [in] i_licenseHandle license handle
   * @param [in] io_handle Track handle
   *
   * @retval MH_ERR_OK Finalize Track is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t finTrack(MH_agentHandle_t i_handle, MH_licenseHandle_t* i_licenseHandle, MH_trackHandle_t* io_handle);

  /**
   * @brief Return the validity of license.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] ret varid or non varid
   *
   * @retval MH_ERR_OK License is varid or non varid
   * @retval MH_ERR_FAILURE Cannot transaction
   */
  MH_status_t isLicenseValid(MH_licenseHandle_t* i_handle, bool& ret);

  /**
   * @brief Return current trusted time.
   *
   * @param [in] i_handle Agent handle
   * @param [out] o_date MBB date status
   *
   * @retval MH_ERR_OK Getting TrustedTime is success
   * @retval MH_ERR_FAILURE Cannot get trusted time
   */
  MH_status_t getTrustedTime(MH_agentHandle_t i_handle, MH_date_t* o_date);

  /**
   * @brief Update trusted time.
   *
   * @param [in] i_handle Agent handle
   * @param [out] o_date MBB date status
   *
   * @retval MH_ERR_OK Updating TrustedTime is success
   * @retval MH_ERR_FAILURE Cannot update trusted time
   */
  MH_status_t updateTrustedTime(MH_agentHandle_t i_handle, MH_date_t* o_date);

  /**
   * @brief Return if the specified node has registation link.
   *
   * @param [in] i_handle MBB license handle
   * @param [in] i_service_id Service id
   * @param [in] i_service_info Service infomation
   * @param [out] io_link_info Link information
   *
   * @retval MH_ERR_OK Getting AvailableLinks is success
   * @retval MH_ERR_FAILURE Cannot get AvailableLinks
   */
  MH_status_t getLinksForServiceID(MH_bbHandle_t* i_handle, MH_string_t i_service_id, MH_string_t i_service_info, MH_linkList_t** io_link_info);

  /**
   * @brief Return if the specified node has registation link.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] io_link_info Link information
   *
   * @retval MH_ERR_OK Getting AvailableLinks is success
   * @retval MH_ERR_FAILURE Cannot get AvailableLinks
   */
  MH_status_t getAvailableLinks(MH_bbHandle_t* i_handle, MH_linkList_t** io_link_info);

  /**
   * @brief Return if the specified node has subscription link.
   *
   * @param [in] i_handle MBB license handle
   * @param [in] i_link Link information
   * @param [out] io_node_info Node information
   *
   * @retval MH_ERR_OK Getting ReachableNodes is success
   * @retval MH_ERR_FAILURE Cannot get ReachableNodes
   */
  MH_status_t getReachableNodes(MH_licenseHandle_t i_handle, MH_linkInfo_t i_link, MH_nodeInfo_t** io_node_info);

  /**
   * @brief Return subscriptionvalidity.
   *
   * @param [in] i_handle MBB license handle
   * @param [in] i_from_id From id
   * @param [in] i_to_id To id
   * @param [out] o_date_before Before date
   * @param [out] o_date_after After date
   *
   * @retval MH_ERR_OK Getting LinkPeriod is success
   * @retval MH_ERR_FAILURE Cannot get LinkPeriod
   */
  MH_status_t getLinkPeriod(MH_licenseHandle_t i_handle, MH_linkInfo_t* i_from_id, MH_linkInfo_t* i_to_id, MH_date_t* o_date_before, MH_date_t* o_date_after);

  /**
   * @brief Set action token to Marlin Agent.
   *
   * @param[in] i_handle MBB license handle
   * @param[in] i_service_token actiontoken data
   * @param [in] i_service_id Service id
   * @param [in] i_account_id Service id
   * @param[out] o_steps remaining steps
   *
   * @retval MH_ERR_OK Setting ServiceToken is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t setServiceToken(MH_bbHandle_t* i_handle,
                              MH_string_t* i_service_token,
                              MH_string_t* i_service_id,
                              MH_string_t* i_account_id,
                              uint32_t* o_steps);

  /**
   * @brief Process action token and generate http request.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] o_request request information.
   *
   * @retval MH_ERR_OK Processing ServiceToken is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t processServiceToken(MH_bbHandle_t* i_handle, MH_httpRequest_t* o_request);

  /**
   * @brief Process http response.
   *
   * @param [in] i_handle MBB license handle
   * @param [in] i_response http response
   * @param [out] o_steps remaining steps
   * @param [out] o_result result
   *
   * @retval MH_ERR_OK Processing RightsResponse is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t processRightsResponse(MH_bbHandle_t* i_handle,
                                    MH_httpResponse_t* i_response,
                                    uint32_t* o_steps,
                                    MH_actionResult_t* o_result);
  /**
   * @brief Generate SAS request.
   *
   * @param [in] i_handle MS3 license handle
   * @param [in] i_triggerData trigger data
   * @param [out] o_triggerInfo trigger info
   * @param [out] o_request request information 
   *
   * @retval MH_ERR_OK Generating SAS request is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t generateSASRequest(MH_ms3Handle_t* i_handle,
                                 MH_sasTriggerData_t* i_trigger_data,
                                 MH_sasTriggerInfo_t* o_trigger_info,
                                 MH_httpRequest_t* o_request);
  /**
   * @brief Process SAS response.
   *
   * @param [in] i_handle MS3 license handle
   * @param [in] i_triggerData trigger data
   * @param [in] i_response trigger info
   * @param [out] o_result result 
   *
   * @retval MH_ERR_OK Processing SAS response is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t processSASResponse(MH_ms3Handle_t* i_handle,
                                 MH_sasTriggerInfo_t* i_trigger_info,
                                 MH_httpResponse_t* i_response,
                                 MH_sasResultData_t* o_result);

  /**
   * @brief Process LTP message.
   *
   * @param [in] i_handle MBB license handle
   * @param [in] i_ltpMassage LTP Message
   * @param [out] o_result result
   * @param [out] o_response response information
   *
   * @retval OK Processing LTP Message is success
   * @retval MH_ERR_UNKNOWN Error by other reasons
   */
  MH_status_t processLTPMessage(MH_bbHandle_t* i_handle,
                                MH_string_t* i_ltpMassage,
                                MH_actionResult_t* o_result,
                                MH_string_t* o_response);

  /**
   * @brief Retrieve loaded license from Marlin Agent.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] licensebuf MBB license data
   *
   * @retval MH_ERR_OK Getting License is success
   * @retval MH_ERR_FAILURE Cannot get license
   */
  MH_status_t getLicense(MH_licenseHandle_t i_handle, MH_string_t* licensebuf);

  /**
   * @brief Retireve metadata from loaded license.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] o_8pusInfo 8pusinfo
   * @param [out] o_info MBB license info
   *
   * @retval MH_ERR_OK Querying LicenseStatus is success
   * @retval MH_ERR_FAILURE Cannot query license
   */
  MH_status_t queryLicenseStatus(MH_licenseHandle_t i_handle, MH_8pusInfo_t* o_8pusInfo, MH_licenseInfo_t* o_info);

  /**
   * @brief Pass license to Marlin Agent and make Marlin Agent retireve content keys.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] o_info MBB license info
   *
   * @retval MH_ERR_OK Setting License is success
   * @retval MH_ERR_FAILURE Cannot set license
   */
  MH_status_t setLicense(MH_licenseHandle_t i_handle, MH_licenseInfo_t* o_info);

  /**
   * @brief Clear loaded license and cached content keys.
   *
   * @param [in] i_handle MBB license handle
   *
   * @retval MH_ERR_OK Clearing License is success
   * @retval MH_ERR_FAILURE Cannot clear license
   */
  MH_status_t clearLicense(MH_licenseHandle_t i_handle);

  /**
   * @brief Evaluate loaded license.
   *
   * @param [in] i_handle MBB license handle
   * @param [out] o_8pusInfo MBB 8pusInfo status
   *
   * @retval MH_ERR_OK Evaluating License is success
   * @retval MH_ERR_FAILURE Cannot clear license
   */
  MH_status_t evaluateLicense(MH_licenseHandle_t i_handle, MH_8pusInfo_t* o_8pusInfo);

  /**
   * @brief Retrieve a DRM content key.
   *
   * @param [in] i_handle License handle
   * @param [in] i_track Track handle
   * @param [in] i_data A protection data header of track
   * @param [in] i_lastflg Track is last or not
   * @param [out] o_data Retrieved DRM content key
   *
   * @retval MH_ERR_OK Retrieving a DRM content key is success
   * @retval MH_ERR_FAILURE Cannot Retrieving a DRM content key
   */
  MH_status_t retrieveContentKey(MH_licenseHandle_t i_handle,
                                 MH_trackHandle_t* i_track,
                                 MH_memdata_t* i_data,
                                 bool* islastflg,
                                 MH_memdata_t* o_data);

  /**
   * @brief Check if KeySystem is suppoted in Marlin Agent.
   *
   * @param [in] i_id KeySystem Id
   * @param [out] out_is_suported KeySystem supported or not supported
   * 
   * @retval MH_ERR_OK Checking if KeySystem is suppoted is success
   * @retval MH_ERR_FAILURE Cannot checking if KeySystem is suppoted
   */
  MH_status_t isKeySystemSupported(MH_keysystemId_t& i_id, bool* out_is_suported);

  /**
   * @brief Decrypt with Software. \n Decide if we use CTR/CBC when initialized. \n
   *
   * @param [in] i_licenseHandle License Handle
   * @param [in] i_trackHandle Track Handle
   * @param [in] is_encrypted Encrypted or not encrypted
   * @param [in] key Key data for decryption of media content \n Key data is KID value related to media content decryption
   * @param [in] i_iv iv data for decryption of media content
   * @param [in] srcBuf Input buffer of encrypted data
   * @param [in] SubSamples Sub sample structural informations
   * @param [in] numSubSamples Number of sub sample structure
   * @param [out] dstPtr Output buffer of decrypted data
   * @param [out] decrypt_buffer_length Output buffer of length
   * @param [out] errorDetailMsg Output buffer of error message
   *
   * @retval MH_ERR_OK Decryption is success
   * @retval MH_ERR_TOO_SMALL_BUFFER Out buffer is too small
   * @retval MH_ERR_FAILURE Cannot decrypt content
   */
  MH_status_t decrypt(MH_licenseHandle_t* i_licenseHandle,
                      MH_trackHandle_t* i_trackHandle, bool is_encrypted,
                      const uint8_t key[16], MH_memdata_t* i_iv,
                      MH_buffer_t* srcBuf, MH_subSample_t* SubSamples,
                      size_t& numSubSamples, MH_buffer_t* dstPtr,
                      size_t& decrypt_buffer_length, string& errorDetailMsg);

  /**
   * @brief Decrypt with hardware. \n Decide if we use CTR/CBC when initialized. \n
   *
   * @param [in] i_licenseHandle License Handle
   * @param [in] i_trackHandle Track Handle
   * @param [in] is_encrypted Encrypted or not encrypted
   * @param [in] key Key data for decryption of media content \n Key data is KID value related to media content decryption
   * @param [in] i_iv iv data for decryption of media content
   * @param [in] srcBuf Input buffer of encrypted data
   * @param [in] SubSamples Sub sample structural informations
   * @param [in] numSubSamples Number of sub sample structure
   * @param [out] dstPtr Output buffer of decrypted data
   * @param [out] decrypt_buffer_length Output buffer of length
   * @param [out] errorDetailMsg Output buffer of error message
   *
   * @retval MH_ERR_OK Decryption is success
   * @retval MH_ERR_TOO_SMALL_BUFFER Out buffer is too small
   * @retval MH_ERR_FAILURE Cannot decrypt content
   */
  MH_status_t decryptWithHW(MH_licenseHandle_t* i_licenseHandle,
                            MH_trackHandle_t* i_trackHandle, bool is_encrypted,
                            const uint8_t key[16], MH_memdata_t* i_iv,
                            MH_buffer_t* srcBuf, MH_subSample_t* SubSamples,
                            size_t& numSubSamples, MH_buffer_t* dstPtr,
                            size_t& decrypt_buffer_length, string& errorDetailMsg);

  /**
   * @brief checkPersonalizationStatus.
   *
   * @param [in] i_handle Marlin Agent handle
   *
   * @retval MH_ERR_OK Checking PersonalizationStatus is success
   * @retval MH_ERR_FAILURE Cannot check PersonalizationStatus
   */
  MH_status_t checkPersonalizationStatus(MH_agentHandle_t i_handle);

  /**
   * @brief processOfflinePersonalization.
   *
   * @param [in] i_handle Marlin Agent handle
   * @param [in] i_personalities Personalities information
   *
   * @retval MH_ERR_OK Checking PersonalizationStatus is success
   * @retval MH_ERR_FAILURE Cannot process OfflinePersonalization
   */
  MH_status_t processOfflinePersonalization(MH_agentHandle_t i_handle, MH_string_t* i_personalities);

  /**
   * @brief setPersonalizeToken.
   *
   * @param [in] i_handle Marlin Agent handle
   * @param [in] i_personalities Personalities information
   *
   * @retval MH_ERR_OK Setting PersonalizeToken is success
   * @retval MH_ERR_FAILURE Cannot set PersonalizeToken
   */
  MH_status_t setPersonalizeToken(MH_agentHandle_t i_handle, MH_string_t* i_personalities);

  /**
   * @brief getPersonalizeRequest.
   *
   * @param [in] i_handle Marlin Agent handle
   * @param [in] i_personalities Personalities information
   * @param [out] o_request Request information
   *
   * @retval MH_ERR_OK Getting PersonalizeRequest is success
   * @retval MH_ERR_FAILURE Cannot get PersonalizeRequest
   */
  MH_status_t getPersonalizeRequest(MH_agentHandle_t i_handle, MH_string_t* i_personalities, MH_string_t* o_request);

  /**
   * @brief processPersonalizeResponse.
   *
   * @param [in] i_handle Marlin Agent handle
   * @param [in] i_response Response data
   *
   * @retval MH_ERR_OK Processing PersonalizeResponse is success
   * @retval MH_ERR_FAILURE Cannot process PersonalizeResponse
   */
  MH_status_t processPersonalizeResponse(MH_agentHandle_t i_handle, MH_string_t* i_response);

  /**
   * @brief getContentId from License.
   *
   * @param [in] i_license MBB license handle
   * @param [out] o_cid Content Id
   *
   * @retval MH_ERR_OK Getting ContentIdByLicense is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByLicense
   */
  MH_status_t getContentIdByLicense(MH_string_t* i_license, MH_cid_t* o_cid);

  /**
   * @brief getContentId from License.
   *
   * @param [in] i_license MBB license handle
   * @param [out] o_cids Content Id list
   *
   * @retval MH_ERR_OK Getting ContentIdByLicense is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByLicense
   */
  MH_status_t getContentIdsByLicense(MH_string_t* i_license, vector<string>* o_cids);

  /**
   * @brief getContentId from Ipmp box.
   *
   * @param [in] i_sinfheader sinf header
   * @param [out] o_cid Content Id
   *
   * @retval MH_ERR_OK Getting ContentIdByIpmp is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByIpmp
   */
  MH_status_t getContentIdByIpmp(MH_memdata_t i_sinfheader, MH_cid_t* o_cid);

  /**
   * @brief getContentId from Pssh box.
   *
   * @param [in] i_psshbox Pssh box
   * @param [in] i_kid  KID
   * @param [out] o_cid Content Id
   *
   * @retval MH_ERR_OK Getting ContentIdByPssh is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByPssh
   */
  MH_status_t getContentIdByPssh(MH_memdata_t i_psshbox, const uint8_t* i_kid, MH_cid_t* o_cid);

  /**
   * @brief getContentId from MPD.
   *
   * @param [in] i_mpd MPD
   * @param [out] o_cid Content Id
   *
   * @retval MH_ERR_OK Getting ContentIdByMPD is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByMPD
   */
  MH_status_t getContentIdByMPD(MH_string_t i_mpd, MH_cid_t* o_cid);

  /**
   * @brief Check if the specified key system ID is supported.
   *
   * @param [in] i_keysystem keysystem name 
   * @param [in] i_initdatatype init data type 
   * @param [in] i_contenttype content type 
   * @param [in] i_capability capability 
   * @param [out] o_istypesupportedresult supported result of specified key system
   *
   * @retval MH_ERR_OK Getting ContentIdByMPD is success
   * @retval MH_ERR_FAILURE Cannot get ContentIdByMPD
   */
  MH_status_t isTypeSupported(MH_string_t i_keysystem,
                              MH_string_t i_initdatatype,
                              MH_string_t i_contenttype,
                              MH_string_t i_capability,
                              MH_string_t* o_istypesupportedresult);

  /**
   * @brief Get specified property from Marlin Agent.
   *
   * @param [in] i_name Property name
   * @param [out] io_value Property value associated with name
   *
   * @retval MH_ERR_OK Getting PropertyString is success
   * @retval MH_ERR_FAILURE Cannot get PropertyString
   */
  MH_status_t getPropertyString(MH_string_t i_name,  MH_string_t* io_value);

  /**
   * @brief Get device property to acquire Service Token.
   *
   * @param [out] io_value Property value associated
   *
   * @retval MH_ERR_OK Getting PropertyString is success
   * @retval MH_ERR_FAILURE Cannot get PropertyString
   */
  MH_status_t getDeviceProperty(MH_string_t* io_value);

  /**
   * @brief Set specific property to Marlin Agent
   *
   * @param [in] io_name Property name
   * @param [out] io_value Property value associated with name
   *
   * @retval MH_ERR_OK Setting PropertyString is success
   * @retval MH_ERR_FAILURE Cannot set PropertyString
   */
  MH_status_t setPropertyString(MH_string_t* io_name, MH_string_t* io_value);

  /**
   * @brief Get specified property from Marlin Agent
   *
   * @param [in] i_name Property name
   * @param [out] io_value Property value associated with name
   *
   * @retval MH_ERR_OK Getting PropertyByteArray is success
   * @retval MH_ERR_FAILURE Cannot get PropertyByteArray
   */
  MH_status_t getPropertyByteArray(MH_string_t i_name,  MH_memdata_t* io_value);

  /**
   * @brief Set specific property to Marlin Agent
   *
   * @param [in] io_name Property name
   * @param [out] io_value Property value associated with name
   *
   * @retval MH_ERR_OK Setting PropertyByteArray is success
   * @retval MH_ERR_FAILURE Cannot set PropertyByteArray
   */
  MH_status_t setPropertyByteArray(MH_string_t* io_name,  MH_memdata_t* io_value);


protected:

private:

};
}; //namespace

#endif /* __MARLIN_AGENT_HANDLER_H__ */
