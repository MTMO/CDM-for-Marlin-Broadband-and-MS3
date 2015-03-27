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

#ifndef MARLIN_CDM_INTERFACE_H_
#define MARLIN_CDM_INTERFACE_H_

#include <stdint.h>
#include <string>
#include <map>
#include <memory>

#include <MarlinError.h>
#include <MarlinCommonTypes.h>
#include <MarlinCdmEngine.h>

/**
 * @brief
 * It describes Marlin CDM(Content Decryption Module) interface for Android and HTML5(EME). Marlin CDM can be used to operate the key acquisition of the marlin protected content and to decrypt the marlin protected content on web browser adnd web browser platform.
 * @image html marlincdm_overview.jpg
 *
 * Marlin CDM includes below parts.
 * - Marlin Drm Plugin : It is glue code to absorb API difference between Marlin CDM Interface and Android Modular Drm Framework.
 * - Marlin CDM Adapter : It is glue code to absorb API difference between Marlin CDM Interface and each browsers EME.
 * - Marlin CDM Interface : It is just aan interface between Android or Web browser framework and Marlin CDM implementation. It is included in Marlin CDM Engine in actual when it is built.
 * - Marlin CDM Engine : It is cdm implementation delegating HTTP transaction responsibility with License server to Application and manages the decrypt session and license management, and others required for as a CDM responsibility.
 * - Marlin Agent Handler : It is a Porting layer to support Marlin DRM solution in conjunction with specialized SDK. With this layer customization, implementer can use any Marlin DRM Agent.
 *
 * @if ANDROID
 * @brief
 * The following diagram shows the sequence while acquiring marlin bb license for Android case.
 * @image html marlincdm_seq_license.jpg
 * @endif
 *
 * @if ANDROID
 * - First, Application constructs MediaDrm object and call openSession() function to get sessionId.
 * - Application needs to create Marlin CDM specific initialization data to let CDM know the operation what application want to process. initData must be json object which includes action token for license acquisition. See API information of Marlin CDM Interface to know detail format of initData.
 * - Application then can call getKeyRequest function to send initData to generate HTTP request message in CDM and pass result to Application. The Request message is the json object which includes HTTP request message. See API information of Marlin CDM Interface to know detail format of Request data.
 * - Request message should be parsed by Application and retrieved HTTP request message should be sent to Marlin License server to proceed marlin bb license acquisition process, and then Application can get HTTP response message from server.
 * - ProvideKeyResponse function need to be called by Application to send response message to CDM. response message is also json object which includes HTTP response message. See API information of Marlin CDM Interface class to know detail format.
 * - During ProvideKeyResponse function,  If the CDM notices that more HTTP transaction with server is needed for completing License acquisition process, Application will get HTTP request message through returned Request data. It is also json object and similar format of returned value of getKeyRequest function. Application can know if more message sending is needed or not from remaining steps value in Request data.
 * - After completing and application get 0 value in remaining step, Marlin license was stored in Marlin CDM and can be used later for content playback process. Application can call closeSession function to close MediaDrm and CDM session.
 * @endif
 *
 * @cond ANDROID
 * @brief
 * The following diagram shows the sequence while playing back marlin protected content for Android case.
 * @image html marlincdm_seq_playback.jpg
 * @endcond
 *
 * @if ANDROID
 * - For Playback of the marlin protected content, Application needs to construct MediaExtractor and MediaCodec objects. These Extractor and Codec and lower level component in MultiMedia framework may need to be modified by Integrator to support to render marlin media format such as MP4 IPMP format and MP4 Common Encryption format.
 * - After the creation of such objects, Application can call setDataSource and getPsshInfo function to get metadata of media file, especially content protection box such as pssh, sinf box.
 * - Application needs to create initData, which is defined for sending the required information to CDM to do initialization of content decryption, such as loading marlin bb license and extract CEKs in Marlin DRM Agent. initData must have content protection box which was retrieved from getPsshInfo function. See API information of Marlin CDM Interface class to know detail format of initData.
 * - Application will construct MediaCrypto object, with passing initData for preparing decrypt session as described above.
 * - Next Application will call configure function of MediaCodec. The MediaCrypto object is registered with this method to enable the codec to decrypt content.
 * - And then it is ready to get sample and crypto info such as IV, kid, subsample structre from MediaExtractor, via readSampleData method and getSampleCryptoInfo method.
 * - When these sample metadata is acquired, queueSecureInputBuffer method can be used to oeprate the decryption of protected sample by CDM or lower level module.
 * - According to the integrators Marlin CDM implementation, decryption can be done on Platform HW and then decrypted data can only be supplied to Decoder without passing decrypted data to Application.
 * @endif
 *
 * @if EME
 * @brief
 * The following diagram shows the reference sequence flow while playing marlin protected media on EME case.
 * @image html marlincdm_seq_playback_eme.jpg
 * @endif
 *
 * @if EME
 * - For streaming the marlin protected content with EME, Javascript Application needs to register the onEncrypted event to HTMLMediaElement <video> tag.
 * - After the regitering the event, onEncrypted event will be triggered from HTMLMediaElement and it is time to create MediaKeys. During onEncrypted event, initialization data will be send to 
 * - By using promise, after creating MediaKeys, MediaKeys object is set to HTMLMediaElement.
 * - createSession is called to create keysession and register it to CDMproxy, which will manage sessions for requesting key usage operation. By this call, pssh box which was extracted in Browser media stack is provided.
 * - Once the session is created and registered to proxy, it will trigger generateKeyRequest, to supply pssh box to MarlinCDM. MarlinCDM will check pssh box if the medt box is added. if there is no medt, MarlinCDM will return json message which has vendor specific data in queryKeyRequest property.
 * - After application get json message, Action Token request will be constructed and sent to server. this HTTP request will be service specific protocol.
 * - Then application will get action token to continue license acquisition, update method will be called to provide action token to MarlinCDM.
 * - ActionToken process is done by Marlin CDM and Marlin Agent so that App will get HTTP request message.
 * - Until the license will be acquired, HTTP request message will be sent to server and provided response is supplied to MarlinCDM by update method again and again.
 * - When the license file is acquired, application will get json message which will have CompleteNotification property.
 * - When application start rendering video, media sample will be sent to CDMProxy in subsamples structure by decrypt call, and then the samples will be decrypted by Marlin CDM and Marlin Agent with the content key.
 *
 * @version 1.0
 */

namespace marlincdm {

using namespace std;

#ifndef EME_ENABLED
/**
 * @brief
 * Listener class listens to events from Marlin CDM.
 *
 */
class MarlinCdmEventListener{
 public:
  MarlinCdmEventListener() {}
  virtual ~MarlinCdmEventListener() {}

  /**
   * @brief This function notifies listener registered in AttachEventListener() of CDM event.
   *
   * @param[in] session_id Session ID which is opened by OpenSession()
   * @param[in] cdm_event CDM event value
   *
   * @retval void
   */
  virtual void onEvent(const mcdm_SessionId_t& session_id,
                       int cdm_event) = 0;
}; // marlincdmeventListener
#endif //EME_ENABLED

/**
 * @brief
 * Marlin CDM(Content Decryption Module) interface for Android and HTML5(EME).
 *
 */
class MarlinCdmInterface {
    public:

    /**
     * @brief
     * Construct MarlinCdmInterface for DRM scheme specified by UUID.
     *
     * @param[in] uuid Universally Unique Identifier of marlin drm scheme
     * - [MBB_UUID](@ref MBB_UUID)
     * - [MS3_UUID](@ref MS3_UUID)
     */
    MarlinCdmInterface(const uint8_t uuid[16]);
    virtual ~MarlinCdmInterface();


    /**
     * @brief This function is opening new session associated with Marlin CDM object.
     *
     * @param[out] session_id Session ID that can be used by application to identify Marlin CDM objects.
     * @retval OK Succeed to open session
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t OpenSession(mcdm_SessionId_t& session_id);

    /**
     * @brief This function closes session opened by OpenSession().
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @retval OK Succeed to close session
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t CloseSession(const mcdm_SessionId_t& session_id);

    #ifndef EME_ENABLED
    /**
     * @brief This function creates decryption unit associated with each track. \n function internally load associated keys according to initialization data. \n
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] init_data Initialization data that are different by Data Format type. Refer <a class="el" href="marlincdm_json_initdata_createdecryptunit_android.json">marlincdm_json_initdata_createdecryptunit_android.json</a>.
     * @retval OK Creating DecryptUnit succeeds
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t CreateDecryptUnit(const mcdm_SessionId_t& session_id, const string& init_data);

    /**
     * @brief This function closes decryption unit created by CreateDecryptUnit().
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @retval OK Closing DecryptUnit succeeds
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t CloseDecryptUnit(const mcdm_SessionId_t& session_id);
    #endif //EME_ENABLED

    /**
     * @brief This function generates key request and acquires license.
     *
     * GenerateKeyRequest function provides following processes in conjunction with [AddKey()](@ref AddKey) function :
     * @if ANDROID
     * - User registration for Marlin BB
     * - User deregistration for Marlin BB
	 * @endif
     * - License Acquisition for Marlin BB
     * @if ANDROID
     * - License Deletion for Marlin BB
	 * @endif
     * - SAS Acquisition for MS3
     * @if ANDROID
     * - LTP message processing for Marlin BB
	 * @endif
     * After calling this function, caller should call [AddKey()](@ref AddKey) function for each processes. \n
     *
     * This function supports following Marlin Protected Media Format:
     * - MP4 DASH-CENC content with Marlin protection \n
     *    Marlin CDM can handle MP4 CENC media format that uses implicit mapping and explicit mapping.
     * @if ANDROID
     * - MP4 IPMP extension content with Marlin protection \n
     *    Marlin CDM can handle MP4 IPMP content that included SINF box according to Marlin specification.
	 * @endif
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] init_data Initialization data that is different for each initDataType. \n
     * Please refer following table to understand which initialization data will be included for each initDataType.
     * initDataType                                               | init_Data
     * ---------------------------------------------------------- | -------------------
     * @if ANDROID
     * "application/vnd.marlin.drm.*" or "json"                   | Json format data defined in <a class="el" href="marlincdm_json_initdata_android.json">marlincdm_json_initdata_android.json</a>
	 * @elseif EME
     * "cenc" or "keyids"                                         | PSSH box
	 * @endif
     *
     * @if ANDROID
     * In init_data, following "data_type" property is need to be specified for each purpose. \n
     * Parameter in init_data is different according to its purpose,\n
     * so caller should carefully select initDataType, keyType and init_data combination for each usage.\n
     * Purpose                        | data_type in json init data
     * ------------------------------ | -------------------
     * User Registration              | "atkn"
     * User Deregistration            | "atkn"
     * Marlin BB License Acquisition  | "atkn"
     * MS3 SAS Acquisition by SAS URL | "ms3"
     * LTP message processing         | "ltp"
     * Get device property            | "prop"
	 * @endif
     *
     * @param[in] initDataType initialization data type information which indicates the data structure of init_data
     * Purpose                                                   | initDataType
     * --------------------------------------------------------- | -------------------
     * @if ANDROID
     * User Registration                                         | application/vnd.marlin.drm.actiontoken+xml+registration 
     * User Deregistration                                       | application/vnd.marlin.drm.actiontoken+xml+deregistration
     * License Acquisition                                       | application/vnd.marlin.drm.actiontoken+xml+license
     * LTP messaging                                             | application/vnd.marlin.drm.metadata.ltp
     * Acquire device property                                   | application/vnd.marlin.drm.metadata.property
     * Other operation                                           | "json"
	 * @elseif EME
     * License acquisition with cenc initialization data         | "cenc"
     * License acquisition with cenc initialization data         | "keyids"
	 * @endif
     *
     * @param[in] keyType Key type which indicates process
     * Purpose                           | Key Type
     * --------------------------------- | ----------
     * @if ANDROID
     * User Registration                 | KEY_TYPE_OFFLINE
     * User Deregistration               | KEY_TYPE_RELEASE
     * License Acquisition for offline   | KEY_TYPE_OFFLINE
     * License Acquisition for streaming | KEY_TYPE_STREAMING
     * SAS Acquisition                   | KEY_TYPE_STREAMING
     * Get Device Property               | KEY_TYPE_OFFLINE
     * LTP messaging                     | KEY_TYPE_OFFLINE
	 * @elseif EME
     * License Acquisition               | KEY_TYPE_STREAMING
	 * @endif
     *
     * @param[in] optionalParameters Optional information for each opration. This parameter should be always null. it is reserved for future use.
     *
     * @param[out] request Key request message
     * or Device property data or ltp message.
     * @if ANDROID
     * Purpose                           | message_type in json request message
     * --------------------------------- | ----------
     * User Registration                 | "httpRequest"
     * User Deregistration               | "httpRequest"
     * License Acquisition               | "httpRequest" or "license"
     * License Deletion                  | "none"
     * License Load with CENC            | "none"
     * License Load with IPMP            | "none"
     * SAS Acquisition                   | "none"
     * Get Device Property               | "prop"
     * LTP messaging                     | "ltp"
	 * @elseif EME
     * Purpose                           | properties in json request message
     * --------------------------------- | ----------
     * License Acquisition               | "httpRequest" or "completeNotification"
	 * @endif
     * About detail of request message structure, \n
	 * @if EME
     * please refer <a class="el" href="marlincdm_json_request_message_eme.json">marlincdm_json_request_message_eme.json</a>. \n
     * @elseif ANDROID
     * please refer <a class="el" href="marlincdm_json_request_message_android.json">marlincdm_json_request_message_android.json</a>. \n
     * About detail of json request message in case of message_type is "prop" to get device property, please refer to following table. \n
     * queried name in <a class="el" href="marlincdm_json_initdata_android.json">marlincdm_json_initdata_android.json</a>   | expected return value in "prop" property     | detail
     * ------------------------------------------------------ | -------------------------------------------- | ---------------
     * "duid"                                                 | <Device Unique Id value>                     | byte order of DeviceUniqueId
     * "registered_state"                                     | "Already Registered"                         | device is registered by AccountId
     * "registered_state"                                     | "Not Registered"                             | device is not registered by AccountId
     * "registered_state"                                     | "Registerd by others"                        | device is registered by another AccountId
	 * @endif
     *
     * @param[out] url server url
     * @retval OK Generating key request succeeds
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID has been closed already or Session ID does not exist.
     * @retval ERROR_NO_LICENSE Valid license does not exist
     * @retval ERROR_CANNOT_HANDLE
     *  - Expected data and data type is not included in current data.
     *  - License acquisition with designated session ID has been done already and Session is not closed yet.
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t GenerateKeyRequest(const mcdm_SessionId_t& session_id,
                                             const string& init_data,
                                             const string* initDataType,
                                             const mcdm_key_type* keyType,
                                             map<string, string>* optionalParameters,
                                             string* request,
                                             string* url);

    /**
     * @brief This function is adding key to Marlin CDM to be associated with Session ID.
     * \n After calling [GenerateKeyRequest()](@ref GenerateKeyRequest) function, caller should call this function to provide key response message.
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] key_data Response data that should be sent to Marlin CDM. \n
     * @if ANDROID
     * About detail of response data structure, please refer <a class="el" href="marlincdm_json_response_message_android.json">marlincdm_json_response_message_android.json</a> 
     * @elseif EME
     * About detail of response data structure, please refer <a class="el" href="marlincdm_json_response_message_eme.json">marlincdm_json_response_message_eme.json</a> 
     * @endif
     * and following table. \n
     * @if ANDROID
     * Purpose                           | message_type in json request message
     * --------------------------------- | ----------
     * User Registration                 | "httpResponse"
     * User Deregistration               | "httpResponse"
     * License Acquisition               | "httpResponse" or "license"
     * SAS Acquisition                   | "httpResponse"
     * @elseif EME
     * Purpose                           | properties in json request message
     * --------------------------------- | ----------
     * License Acquisition               | "httpResponse" or "applicationError"
     * @endif
     *
     * @param[out] request Request data. \n
     * @if ANDROID
     * About detail of request data structure, please refer <a class="el" href="marlincdm_json_request_message_android.json">marlincdm_json_request_message_android.json</a> 
     * @elseif EME
     * About detail of request data structure, please refer <a class="el" href="marlincdm_json_request_message_eme.json">marlincdm_json_request_message_eme.json</a> 
     * @endif
     * and following table. \n
     * @if ANDROID
     * Purpose                           | message_type in json request message
     * --------------------------------- | ----------
     * User Registration                 | "httpRequest"
     * User Deregistration               | "httpRequest"
     * License Acquisition               | "httpRequest" or "license"
     * SAS Acquisition                   | "httpRequest"
     * @elseif EME
     * Purpose                           | properties in json request message
     * --------------------------------- | ----------
     * License Acquisition               | "httpRequest" or "completeNotification"
     * @endif
     *
     * @retval OK Adding key succeeds
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_NO_LICENSE Valid license does not exist
     * @retval ERROR_CANNOT_HANDLE Input data type is not included in current data
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t AddKey(const mcdm_SessionId_t& session_id,
                                 const string& key_data,
                                 string* request);

    #ifndef EME_ENABLED
    /**
     * @brief This function is canceling session linked request information which is generated by GenerateKeyRequest().
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @retval OK Succeed to cancel key request
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t CancelKeyRequest(const mcdm_SessionId_t& session_id);

    /**
     * @brief This function is getting request data for personalization
     *
     * @param[in] request
     * @param[in] default_url
     * @retval OK Succeed to get ProvisionResponse
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t GetProvisionRequest(
        string& request,
        string& default_url);

    /**
     * @brief This function is provisioning key data for personalization.
     *
     * @param[in] response_data container-specific data
     * @retval OK Succeed to provide ProvisionResponse
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t ProvideProvisionResponse(
        string& response_data);

    /**
     * @brief This function is querying if crypto scheme requires use of secure decoder to decode data of given mime type.
     *
     * Secure decoder implementation depends on platform.
     * If platform has secure decoder, modification in Marlin CDM may be needed to use secure decoder.
     *
     * @param[in] mime Mime type of media content, such as "video/avc"
     * @param[out] is_secure \n
     *    - true Secure decoder is supported with given mime type.
     *    - false Secure decoder isn't supported with given mime type.
     * @retval OK Succeed to get is_secure flag
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t useSecureBuffer(
        const uint8_t* mime,
        bool& is_secure);
    #endif //EME_ENABLED

	#ifndef ANDROID_ENABLED
    /**
     * @brief This function is called to check if the specified key system ID is supported.
     *
     * @param[in] key_system key_system name
     * @param[in] init_data_type init data type
     * @param[in] content_type content type
     * @param[in] capability capability of specific usage
     * @param[out] isTypeSupportedResult specified key_system, type and capability is supported or not
     * @retval OK Succeed to get isTypeSupported information
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t isTypeSupported(
        const string& key_system,
        const string& init_data_type,
        const string& content_type,
        const string& capability,
        string* isTypeSupportedResult);
    #endif //ANDROID_ENABLED

	#ifndef EME_ENABLED
    /**
     * @brief This function setups keys which are retrieved in earlier key request, for offline usage or querying key information.
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] init_data Initialization data of json format are chosen by Data Format type. Refer <a class="el" href="marlincdm_json_initdata_restorekey_android.json">marlincdm_json_initdata_restorekey_android.json</a>.
     * @retval OK Restoring key is success
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t RestoreKey(const mcdm_SessionId_t& session_id,
                                   const string& init_data);

    /**
     * @brief This function removes offline keys.
     * \n Caller should call RestoreKey() before calling this function.
     *
     * @param[in] session_id Session ID which is opened by OpenSession() and used by RestoreKey()
     *
     * @retval OK Succeed to remove key
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t RemoveKey(const mcdm_SessionId_t& session_id);

    /**
     * @brief This function provides metadata of Marlin BB license, SAS.
     * \n Caller should call RestoreKey() before calling this function.
     *
     * Provided metadata are different by each purpose.
     * Purpose                         | metadata
     * -----------------------------   | ----------------------
     * Marlin BB License information   | MBB_LICENSE_METADATA_TABLE
     * SAS Information                 | MS3_SAS_METADATA_TABLE
     *
     * Session ID is linked to related data by following transactions
     * When user uses this function, provided metadata.
     * - Marlin BB License acquistion transaction is executed by GenerateKeyRequest() on Android.
     * - Marlin BB Registration transaction is executed by GenerateKeyRequest() on Android.
     * - Marlin MS3 SAS acquisition transaction is executed by GenerateKeyRequest() on Android.
     *
     * @param[in] session_id Session ID used for RestoreKey() should be used for this function
     * @param[out] infoMap This data includes metadata that are different by each related data\n Providing metadata is defined in following tables \n
     * [@ref MS3_SAS_METADATA_TABLE] \n
     * [@ref MBB_LICENSE_METADATA_TABLE] \n
     *
     * - @anchor MS3_SAS_METADATA_TABLE [MS3_SAS_METADATA_TABLE]
     *   Name                        | Detail                               | Value
     *   --------------------------- | ------------------------------------ | -----
     *   sas.acquisition.status      | Status of SAS                        | success: SAS is available \n failure: SAS is not available
     *   sas.retention_flag          | Retention flag                       | 0: client could retain streamed content \n 1: client SHALL NOT retain streamed content
     *   sas.curl                    | Content URL if CURI-T is expanded    | String of content url
     *   sas.cid.num                 | Number of Content ID                 | Number of Content IDs
     *   sas.cid.N                   |                                      | Content ID. \n It is expressed by character string data by hex digit to 0-F. \n User must set number to 0-(sas.cid.num - 1) in N. \n For example sas.cid.0 \n
     *   sas.extension.num           | Number of extensions                 | Number of extentions
     *   sas.extension.N.type        |                                      | String that shows value of hexadecimal integer \n User must set number to 0-(sas.extension.num - 1) in N. \n For example sas.extension.0.type
     *   sas.extension.N.is_critical |                                      | 0: not critical data \n 1: critical data \n User must set number to 0-sas.extension.num in N. \n For example sas.extension.0.is_critical
     *   sas.extension.N.payload     |                                      | This data are binary data. \n And it is expressed by character string data expressed by hex digit to 0-F. \n User must set number to 0-sas.extension.num in N. \n For example sas.extension.0.payload
     *   sas.oci.basic.epn           | BasicCCI EPN                         | 0: EPN-asserted \n 1: EPN-unasserted
     *   sas.oci.basic.cci           | BasicCCI Copy control information    | 00: Copy Control Not Asserted \n 01: No More Copy \n 10: Copy One Generation \n 11: Never Copy
     *   sas.oci.basic.dot           | BasicCCI Digital only token          | 0: Output of decrypted content is allowed for Analog/Digital Outputs \n 1: Output of decrypted content is allowed only for Digital Outputs
     *   sas.oci.basic.ict           | BasicCCI Image constraint token      | 0: High Definition Analog Output in form of Constrained Image \n 1: High Definition Analog Output in High Definition Analog Form
     *   sas.oci.basic.aps           | BasicCCI APS                         | 00: APS off \n 01: APS on-type1(AGC) \n 10: APS on-type2(AGC+2L colorstripe) \n 11: APS on-type2(AGC+4L colorstripe)
     *   sas.oci.dtcp.epn            | DTCPCCI EPN                          | 0: EPN-asserted \n 1: EPN-unasserted
     *   sas.oci.dtcp.cci            | DTCPCCI Copy control information     | 00: Copy free \n 01: No More Copy \n 10: Copy One Generation \n 11: Never Copy
     *   sas.oci.dtcp.rmm            | DTCPCCI Retention move mode          | 0: Retention mode or Move mode \n 1: Non retention mode or Non move mode
     *   sas.oci.dtcp.rs             | DTCPCCI Retention State              | 000: Forever \n 001: 1 week \n 010: 2 days \n 011: 1 day \n 100: 12 hours \n 101: 6 hours \n 110: 3 hours \n 111: 90 minutes
     *   sas.oci.dtcp.ict            | DTCPCCI Image constraint token       | 0: High Definition Analog Output in form of Constrained Image \n 1: High Definition Analog Output in High Definition Analog Form
     *   sas.oci.dtcp.aps            | DTCPCCI APS                          | 00: Copy free \n 01: APS on-type1(AGC) \n 10: APS on-type2(AGC+2L colorstripe) \n 11: APS on-type2(AGC+4L colorstripe)
     *
     * - @anchor MBB_LICENSE_METADATA_TABLE [MBB_LICENSE_METADATA_TABLE]
     *   Name                  | Detail                                         | Value
     *   ----------------------| ---------------------------------------------- | -----
     *   mbb.m.repeat.cnt      | Max repeat count                               | Number of max repeat count
     *   mbb.r.repeat.cnt      | Remaining repeat count                         | Number of remaining repeat count
     *   mbb.stime             | License start time(UTC, msec)                  | For example 1309766100000
     *   mbb.etime             | License expiry time(UTC, msec)                 | For example 1312358100000
     *   mbb.atime             | License available time(sec)                    | For example 86400
     *   mbb.p.time.1st.use    | Play:Time of first use(UTC, msec)              | For example 1309852500000
     *   mbb.p.etime.1st.use   | Play:Expiry time after first use(UTC, msec)    | For example 1310111700000
     *   mbb.p.stime.exclusive | Play:Start time of exclusive period(UTC, msec) | For example 1309766000000
     *   mbb.p.etime.exclusive | Play:Emd time of exclusive period(UTC, msec)   | For example 1312358600000
     *   mbb.p.atime           | Play:Availabe time(sec                         | For example 86400
     *   mbb.p.trusted.time    | Play:Trusted urrent time(UTC, msec)            | For example 1312357100000
     *   mbb.oci.basic.epn     | BasicCCI EPN                                   | 0: EPN-asserted \n 1: EPN-unasserted
     *   mbb.oci.basic.cci     | BasicCCI Copy control information              | 00: Copy Control Not Asserted \n 01: No More Copy \n 10: Copy One Generation \n 11: Never Copy
     *   mbb.oci.basic.dot     | BasicCCI Digital only token                    | 0: Output of decrypted content is allowed for Analog/Digital Outputs \n 1: Output of decrypted content is allowed only for Digital Outputs
     *   mbb.oci.basic.ict     | BasicCCI Image constraint token                | 0: High Definition Analog Output in form of Constrained Image \n 1: High Definition Analog Output in High Definition Analog Form
     *   mbb.oci.basic.aps     | BasicCCI APS                                   | 00: APS off \n 01:APS on-type1(AGC) \n 10: APS on-type2(AGC+2L colorstripe) \n 11: APS on-type2(AGC+4L colorstripe)
     *   mbb.oci.dtcp.epn      | DTCPCCI EPN                                    | 0: EPN-asserted \n 1: EPN-unasserted
     *   mbb.oci.dtcp.cci      | DTCPCCI Copy control information               | 00: Copy free \n 01: No More Copy \n 10: Copy One Generation \n 11: Never Copy
     *   mbb.oci.dtcp.rmm      | DTCPCCI Retention move mode                    | 0: Retention mode or Move mode \n 1: Non retention mode or Non move mode
     *   mbb.oci.dtcp.rs       | DTCPCCI Retention State                        | 000: Forever \n 001: 1 week \n 010: 2 days \n 011: 1 day \n 100: 12 hours \n 101: 6 hours \n 110: 3 hours \n 111: 90 minutes
     *   mbb.oci.dtcp.ict      | DTCPCCI Image constraint token                 | 0: High Definition Analog Output in form of Constrained Image \n 1: High Definition Analog Output in High Definition Analog Form
     *   mbb.oci.dtcp.aps      | DTCPCCI APS                                    | 00: Copy free \n 01: APS on-type1(AGC) \n 10: APS on-type2(AGC+2L colorstripe) \n 11: APS on-type2(AGC+4L colorstripe)
     *
     * @retval OK Succeed to get metadata
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_SESSION_NOT_OPENED Session ID does not exist
     * @retval ERROR_NO_LICENSE Valid license does not exist
     * @retval ERROR_CANNOT_HANDLE Input data type is not included in current data
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t QueryKeyStatus(const mcdm_SessionId_t& session_id,
                                         map<string, string>& infoMap);

    /**
     * @brief This function retrieves property value string from its name.
     *
     * @param[in] name Property name.
     * \n Following names are available
     * name          | expected value     | detail
     * ------------- | ------------------ | ---------------
     * "duid"        | string             | byte order of DeviceUniqueId
     *
     * @param[out] value Property value associated with name.
     *
     * @retval OK Succeed to get property
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t GetPropertyString(const string& name,
                                          string& value);

    /**
     * @brief GetPropertyByteArray This function retrieves property value byte array from its name.
     *
     * @param[in] name Property name
     * @param[out] value Property value from its name
     *
     * @retval OK Succeed to get property
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t GetPropertyByteArray(const string& name,
                                             uint8_t& value);

    /**
     * @brief Write property value for given Device property name.
     *
     * @param[in] name Property name
     * @param[in] value Property value
     *
     * @retval OK Succeed to set property
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_DRM_UNKNOWN Error by other reasons
     */

    virtual mcdm_status_t SetPropertyString(const string& name,
                                          string& value);

    /**
     * @brief Write property value for given Device property name.
     *
     * @param[in] name Property name
     * @param[in] value Property value
     *
     * @retval OK Suceed to set property
     * @retval ERROR_ILLEGAL_ARGUMENT Input parameter is invalid
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    virtual mcdm_status_t SetPropertyByteArray(const string& name,
                                             uint8_t& value);
    #endif //EME_ENABLED

    /**
     * @brief This function provides decryption of media content.
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] is_encrypted Encrypted or not encrypted
     * @param[in] is_secure Secure flag which is provided by useSecureBuffer()
     * Value | detail
     * ----- | ------
     * true  | Secure decoder is used for decrypted data
     * false | Secure decoder is not used for decrypted data
     *
     * @param[in] key Key data for decryption of media content \n Key data is KID value related to media content decryption
     * @param[in] iv iv data for decryption of media content
     *
     * @param[in] mode Encryption mode for media content
     * Crypto format | mode
     * ------------- | ------
     * CENC format   | Mode_AES_CTR
     * @if ANDROID
     * IPMP format   | Mode_AES_CBC
     * @endif
     *
     * @param[in] srcPtr Input buffer of encrypted data
     * @param[in] subSamples Sub sample structural information
     * @param[in] numSubSamples Number of sub sample structure
     * @param[out] dstPtr Output buffer of decrypted data
     * @param[out] decrypt_buffer_length Output buffer of length
     * @param[out] errorDetailMsg Output buffer of error message
     * Retval                           | Error string                          | detail
     * -------------------------------- | ------------------------------------- | ------
     * ERROR_NO_LICENSE                 | License does not exist                | License to decode it does not exist
     * ERROR_LICENSE_EXPIRED            | License is expired                    | License is expired
     * ERROR_SESSION_NOT_OPENED         | Session is discarded                  | Session is discarded
     * ERROR_UNSUPPORTED_SECURE_DECODER | Non support secure decoder            | Secure decoder is not supported
     * ERROR_UNSUPPORTED_CRYPTO_MODE    | Non support crypto mode               | Crypto mode is not supported
     * ERROR_CANNOT_HANDLE              | Content information cannot be handled | Content information cannot be handled
     * ERROR_NO_CID                     | No Key for KID                        | Marlin CDM could not find any CID from provided KID.
     * ERROR_ILLEGAL_ARGUMENT           | Invalid parameter                     | Input parameter is invalid
     * ERROR_FAILED_DECRYPTION          | Failed to decrypt                     | Failed to decrypt (unknown reason)
     * ERROR_UNKNOWN                    | Error by other reasons                | Error by other reasons
     *
     * @retval OK Succeed to decrypt
     * @retval ERROR_NO_LICENSE License to decode it does not exist
     * @retval ERROR_LICENSE_EXPIRED License was expired
     * @retval ERROR_CANNOT_HANDLE Content information cannot be handled
     * @retval ERROR_SESSION_NOT_OPENED Session was discarded
     * @retval ERROR_UNSUPPORTED_SECURE_DECODER Non support secure decoder
     * @retval ERROR_UNSUPPORTED_CRYPTO_MODE Non support crypto mode
     * @retval ERROR_NO_CID No Key for KID
     * @retval ERROR_ILLEGAL_ARGUMENT Invalid parameter
     * @retval ERROR_FAILED_DECRYPTION Failed to decrypt
     * @retval ERROR_UNKNOWN Error by other reasons
     */
    // Accept encrypted buffer and return decrypted data.
    virtual mcdm_status_t Decrypt(const mcdm_SessionId_t& session_id,
                                  bool& is_encrypted,
                                  bool& is_secure,
                                  const uint8_t key[16],
                                  const uint8_t iv[16],
                                  mcdm_decryptMode& mode,
                                  mcdm_buffer_t* srcPtr,
                                  const mcdm_subsample_t* subSamples,
                                  size_t& numSubSamples,
                                  mcdm_buffer_t* dstPtr,
                                  size_t& decrypt_buffer_length,
                                  string& errorDetailMsg);

    #ifndef EME_ENABLED
    /**
     * @brief This function is attaching event listener to Marlin CDM.
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] listener Event listener
     *
     * @retval TRUE Attaching of event listener is success
     */
    virtual bool AttachEventListener(const mcdm_SessionId_t& session_id,
                                     MarlinCdmEventListener* listener);

    /**
     * @brief This function is detaching event listener to Marlin CDM.
     *
     * @param[in] session_id Session ID which is opened by OpenSession()
     * @param[in] listener Event listener
     *
     * @retval TRUE Detaching of event listener is success
     */
    virtual bool DetachEventListener(const mcdm_SessionId_t& session_id,
                                     MarlinCdmEventListener* listener);
    #endif //EME_ENABLED

   private:
    // instance variables
    auto_ptr<MarlinCdmEngine> marlin_cdm_engine_;


}; // marlincdminterface

}  // namespace marlincdm

#endif  // MARLIN_CDM_INTERFACE_H_
