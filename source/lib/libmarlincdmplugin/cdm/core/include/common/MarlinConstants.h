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

#ifndef __MARLIN_CONSTANTS_H__
#define __MARLIN_CONSTANTS_H__

#include <MarlinCommonTypes.h>

namespace marlincdm {

/*! ID shows Marlin Broadband DRM scheme. It is used to create DrmPlugin that can be handled to Marlin Broadband transactions.\n
 *  Created DrmPlugin can handle MBB Registration/Deregistration, License Acquisition/Deletion, metadata acquisition and decryption preparation. \n
 *  Value of ID is 4FC955B3-B934-4111-9DE9-8FF7DAD07170.
 */
const uint8_t MBB_UUID[16] = {0x4f, 0xc9, 0x55, 0xb3, 0xb9, 0x34, 0x41, 0x11, 0x9d, 0xe9, 0x8f, 0xf7, 0xda, 0xd0, 0x71, 0x70};

/*! ID shows MS3 DRM scheme. It is used to create DrmPlugin that can be handled to MS3 transactions and SAS.\n
 *  Created DrmPlugin can handle SAS Acquisition, metadata acquisition. \n
 *  Value of ID is 91193106-5F36-40FC-A78B-6EDB62FC7FE5.
 */
const uint8_t MS3_UUID[16] = {0x91, 0x19, 0x31, 0x06, 0x5f, 0x36, 0x40, 0xfc, 0xa7, 0x8b, 0x6e, 0xdb, 0x62, 0xfc, 0x7f, 0xe5};

/*!  Start process with Action Token for User Regsitration.
*/
const string MIME_TYPE_REGISTRATION("application/vnd.marlin.drm.actiontoken+xml+registration");

/*!  Start process with Action Token for User Deregsitration.
*/
const string MIME_TYPE_DEREGISTRATION("application/vnd.marlin.drm.actiontoken+xml+deregistration");

/*!  Start process with Action Token for License Acquisition.
*/
const string MIME_TYPE_LICENSE("application/vnd.marlin.drm.actiontoken+xml+license");

/*!  Start process with MPD metadata for License Deletion.
*/
const string MIME_TYPE_MPD("application/vnd.marlin.drm.metadata.mpd");

/*!  Start process with CENC metadata for License Deletion.
*/
const string MIME_TYPE_CENC("application/vnd.marlin.drm.metadata.cenc");

/*!  Start process with IPMP metadata for License Deletion.
*/
const string MIME_TYPE_IPMP("application/vnd.marlin.drm.metadata.ipmp");

/*!  Start process to acquire SAS.
*/
const string MIME_TYPE_SAS("application/vnd.marlin.drm.StreamAccessStatement");

/*!  Start process to acquire LTP.
*/
const string MIME_TYPE_LTP("application/vnd.marlin.drm.metadata.ltp");

/*!  Start process to acquire Property.
*/
const string MIME_TYPE_PROPERTY("application/vnd.marlin.drm.metadata.property");

/*!  Start process with CENC metadata for EME.
*/
const string MIME_TYPE_EME_CENC("cenc");

/*!  Start process without metadata for EME.
*/
const string MIME_TYPE_EME_KEYIDS("keyids");

/*!  This is a key to get service id from optionalParameters.
*/
const string KEY_OPTIONAL_SERVICE_ID("service.id");

/*!  This is a key to get account id from optionalParameters.
*/
const string KEY_OPTIONAL_ACCOUNT_ID("account.id");

/*!  This is a key to get subscription id from optionalParameters.
*/
const string KEY_OPTIONAL_SUBSCRIPTION_ID("subscription.id");

/*!  This is a key to get license path from optionalParameters.
*/
const string KEY_OPTIONAL_LICENSE_PATH("license.path");

const string LICENSE_METADATA_KEY_SATUS("license.status");
const string LICENSE_METADATA_VALUE_SATUS_VALID("valid");
const string LICENSE_METADATA_VALUE_SATUS_INVALID("invalid");
const string LICENSE_METADATA_VALUE_SATUS_EXPIRED("expired");

const string MS3_SAS_METADATA_KEY_SATUS("sas.acquisition.status");
const string MS3_SAS_METADATA_KEY_FGLAG("sas.retention_flag");
const string MS3_SAS_METADATA_KEY_CURL("sas.curl");
const string MS3_SAS_METADATA_KEY_CID_NUM("sas.cid.num");
const string MS3_SAS_METADATA_KEY_CID_N("sas.cid.N");
const string MS3_SAS_METADATA_KEY_EXTENSION_NUM("sas.extension.num");
const string MS3_SAS_METADATA_KEY_EXTENSION_TYPE("sas.extension.N.type");
const string MS3_SAS_METADATA_KEY_EXTENSION_IS_CRITICAL("sas.extension.N.is_critical");
const string MS3_SAS_METADATA_KEY_EXTENSION_PAYLOAD("sas.extension.N.payload");
const string MS3_SAS_METADATA_KEY_OCI_BASIC_EPN("sas.oci.basic.epn");
const string MS3_SAS_METADATA_KEY_OCI_BASIC_CCI("sas.oci.basic.cci");
const string MS3_SAS_METADATA_KEY_OCI_BASIC_DOT("sas.oci.basic.dot");
const string MS3_SAS_METADATA_KEY_OCI_BASIC_ICT("sas.oci.basic.ict");
const string MS3_SAS_METADATA_KEY_OCI_BASIC_APS("sas.oci.basic.aps");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_EPN("sas.oci.dtcp.epn");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_CCI("sas.oci.dtcp.cci");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_RMM("sas.oci.dtcp.rmm");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_RS("sas.oci.dtcp.rs");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_ICT("sas.oci.dtcp.ict");
const string MS3_SAS_METADATA_KEY_OCI_DTCP_APS("sas.oci.dtcp.aps");

const string MBB_LICENSE_METADATA_MAX_REPEAT_CNT("mbb.m.repeat.cnt");
const string MBB_LICENSE_METADATA_REMAINING_REPEAT_CNT("mbb.r.repeat.cnt");
const string MBB_LICENSE_METADATA_LICENSE_START_TIME("mbb.stime");
const string MBB_LICENSE_METADATA_LICENSE_END_TIME("mbb.etime");
const string MBB_LICENSE_METADATA_LICENSE_AVAILABLE_TIME("mbb.atime");
const string MBB_LICENSE_METADATA_PLAY_TIME_1ST_USE("mbb.p.time.1st.use");
const string MBB_LICENSE_METADATA_PLAY_END_TIME_1ST_USE("mbb.p.etime.1st.use");
const string MBB_LICENSE_METADATA_PLAY_START_TIME_PERIOD("mbb.p.stime.exclusive");
const string MBB_LICENSE_METADATA_PLAY_END_TIME_PERIOD("mbb.p.etime.exclusive");
const string MBB_LICENSE_METADATA_PLAY_AVAILABLE_TIME("mbb.p.atime");
const string MBB_LICENSE_METADATA_PLAY_TRUSTED_TIME("mbb.p.trusted.time");
const string MBB_LICENSE_METADATA_OCI_BASIC_EPN("mbb.oci.basic.epn");
const string MBB_LICENSE_METADATA_OCI_BASIC_CCI("mbb.oci.basic.cci");
const string MBB_LICENSE_METADATA_OCI_BASIC_DOT("mbb.oci.basic.dot");
const string MBB_LICENSE_METADATA_OCI_BASIC_ICT("mbb.oci.basic.ict");
const string MBB_LICENSE_METADATA_OCI_BASIC_APS("mbb.oci.basic.aps");
const string MBB_LICENSE_METADATA_OCI_DTCP_EPN("mbb.oci.dtcp.epn");
const string MBB_LICENSE_METADATA_OCI_DTCP_CCI("mbb.oci.dtcp.cci");
const string MBB_LICENSE_METADATA_OCI_DTCP_RMM("mbb.oci.dtcp.rmm");
const string MBB_LICENSE_METADATA_OCI_DTCP_RS("mbb.oci.dtcp.rs");
const string MBB_LICENSE_METADATA_OCI_DTCP_ICT("mbb.oci.dtcp.ict");
const string MBB_LICENSE_METADATA_OCI_DTCP_APS("mbb.oci.dtcp.aps");

} // namespace marlincdm

#endif /* __MARLIN_CONSTANTS_H__ */
