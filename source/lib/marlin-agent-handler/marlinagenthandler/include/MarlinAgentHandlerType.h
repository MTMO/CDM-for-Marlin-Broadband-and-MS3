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

#ifndef MARLIN_AGENT_HANDLER_TYPE_H_
#define MARLIN_AGENT_HANDLER_TYPE_H_

#include <string>
#include <map>

using namespace std;

/* flags for 8pus info*/
#define MH_8PUS_TEMPORAL_FLAG_NOTBEFORE  (1<<0)
#define MH_8PUS_TEMPORAL_FLAG_NOTAFTER   (1<<1)
#define MH_8PUS_TEMPORAL_FLAG_NOTDURING  (1<<2)
#define MH_8PUS_TEMPORAL_FLAG_NOTLONGER  (1<<3)
#define MH_8PUS_TEMPORAL_FLAG_NOTMORE    (1<<4)
#define MH_8PUS_TEMPORAL_FLAG_NOTLATER   (1<<5)

/* counter contraint parameters */
#define MH_8PUS_COUNTER_FLAG_REPEATCOUNT          (1<<0)

/* output contraint parameters */
#define MH_8PUS_OUTPUTCONTROL_FLAG_BASIC  (1<<0)
#define MH_8PUS_OUTPUTCONTROL_FLAG_DTCP   (1<<1)

/**
 * @brief Error Type of Marlin Agent Handler
 */
enum MH_status_t {
  MH_ERR_OK = 0, //!< process was success.
  MH_ERR_UNKNOWN, //!< process failed for error of unknown.
  MH_ERR_FAILURE, //!< process failed for error of failure.
  MH_ERR_INVALID_PARAM, //!< process failed for error of invalid parameter.
  MH_ERR_INVALID_LICENSE, //!< process failed for error of invalid license.
  MH_ERR_LICENSE_EXPIRED, //!< process failed for error of expired license.
  MH_ERR_TOO_SMALL_BUFFER, //!< process failed for error of too small buffer.
  MH_ERR_NO_KEY_FOUND, //!< process failed for error which key was not found.
};

typedef uint8_t MH_REQUEST_METHOD_TYPE;
static const MH_REQUEST_METHOD_TYPE MH_GET = 0x00;
static const MH_REQUEST_METHOD_TYPE MH_POST = 0x01;

/**
 * @brief encryption mode
 */
enum MH_decryptMode {
  Unencrypted = 0, //!< content is not encrypted
  AES_CTR     = 1, //!< content is encrypted by AES CTR
  AES_CBC     = 2, //!< content is encrypted by AES CBC
};

/**
 * @brief Result Type for processRightsResponse
 */
enum MH_resultType {
    RESULT_TYPE_UNKNOWN = 0, //!< Result indicates unknown error
    RESULT_TYPE_REGISTER, //!< Result indicates registration and deregistration
    RESULT_TYPE_LICENSE, //!< Result indicates license acquisition
    RESULT_TYPE_SOAPFAULT, //!< Result indicates soapfault
};

/**
 * @brief BB mode Type
 */
enum MH_BB_MODE {
    MH_BB_MODE_TYPE_LOCAL = 0, //!< mode local
    MH_BB_MODE_TYPE_USB //!< mode usb that it is used LTP Process
};

/**
 * @brief MS3 Trigger Type
 */
enum MH_MS3_TRIGGER_TYPE {
    MH_MS3_ACTION_TOKEN  = 0, //!< Action Token
    MH_MS3_COMPOUND_URI, //!< Compound URI
    MH_MS3_MANIFEST_FILE //!< Manifest File
};

/**
 * @brief This parameter show size.
 */
typedef uint32_t MH_size_t;

/**
 * @brief This parameter show agentHandle.
 */
typedef void* MH_agentHandle_t;

/**
 * @brief This structure includes agent handle and required information for marlin bb session.
 */
struct MH_bbHandle_t {
    MH_agentHandle_t handle; //!< agent handle
    void* client; //!< USB or LOCAL Client
    uint32_t mode; //!< mode
};

/**
 * @brief This parameter show license Handle.
 */
typedef MH_agentHandle_t MH_licenseSessionHandle_t;

/**
 * @brief This parameter show track Handle.
 */
typedef int32_t MH_trackSessionHandle_t;

/**
 * @brief This parameter show content id.
 */
typedef string MH_cid_t;

/**
 * @brief This parameter show http request header list.
 */
typedef map<string, string> MH_header_list_t;

/**
 * @brief This structure includes bufstring length and bufstring buffer.
 */
struct MH_string_t {
    MH_size_t length; //!< bufstring length
    uint8_t* bufstr; //!< bufstring buffer
};

/**
 * @brief This structure includes memdata length and memdata buffer.
 */
struct MH_memdata_t {
    MH_size_t length; //!< memdata length
    uint8_t* bufdata; //!< memdata buffer
};

/**
 * @brief This structure includes license handle and required information for ms3 session.
 */
struct MH_ms3Handle_t {
    MH_agentHandle_t handle; //!< ms3 handle
};

/**
 * @brief This structure includes Trigger data for ms3 session.
 */
struct MH_sasTriggerData_t {
    uint32_t triggertype; //!< Trigger Type
    MH_string_t* curi; //!< Compound URI Trigger Data
};

/**
 * @brief This structure includes Trigger information for ms3 session.
 */
struct MH_sasTriggerInfo_t {
    MH_string_t* surl;
    MH_string_t* contenttype;
};

/**
 * @brief This structure includes license handle and license buffer.
 */
struct MH_licenseHandle_t {
    MH_licenseSessionHandle_t handle; //!< license session handle
    MH_string_t licensebuf; //!< license buffer
};

/**
 * @brief This structure includes crypto information.
 */
struct MH_cryptoInfo_t {
    uint32_t type;
    union {
        struct {
            uint8_t selenc;
            uint8_t keyindlen;
            uint8_t ivlen;
            uint8_t resv[1];
            uint8_t salt[8];
        } ismacrypt;
        struct {
            uint8_t selenc;
            uint8_t algo;
            uint8_t padding;
        } kfcbc;
        uint8_t dummy[12];
    } data;
};

/**
 * @brief This structure includes content id and crypto information.
 */
struct MH_trackInfo_t {
    MH_string_t* cid; //!< content id
    MH_cryptoInfo_t* cryptoinfo; //!< crypto information
};

/**
 * @brief This structure includes license handle and track information.
 */
struct MH_trackHandle_t {
    MH_trackSessionHandle_t handle; //!< track session handle
    MH_trackInfo_t trackinfo; //!< track information
};

/**
 * @brief This structure includes date data length and time information.
 */
struct MH_date_t {
    int32_t year;
    int32_t mon;
    int32_t mday;
    int32_t hour;
    int32_t min;
    int32_t sec;
    int32_t msec;
    int32_t offset;
    int32_t isdst;
};

/**
 * @brief This structure includes link information.
 */
struct MH_linkInfo_t {
    MH_string_t* from_id; //!< from id
    MH_string_t* to_id; //!< to id
    MH_date_t* before; //!< before date
    MH_date_t* after; //!< after date
};

/**
 * @brief This structure includes linkList information.
 */
struct MH_linkList_t {
    MH_linkList_t* next_link; //!< next link
    MH_string_t* from_id; //!< from id
    MH_string_t* to_id; //!< to id
};

/**
 * @brief This structure includes node information.
 */
struct MH_nodeInfo_t {
    MH_linkList_t* next_link; //!< next link
    MH_string_t* from_id; //!< from id
    MH_string_t* to_id; //!< to id
};

/**
 * @brief This structure includes nodeList information.
 */
struct MH_nodeList_t {
    MH_nodeList_t* next_node; //!< next node
    MH_string_t* node_id; //!< node id
};

/**
 * @brief This structure includes http request information.
 */
struct MH_httpRequest_t {
    uint8_t method; //!< http method
    char* url; //!< uri
    MH_header_list_t headers; //!< header list
    MH_string_t* request; //!< request buffer
};

/**
 * @brief This structure includes http response information.
 */
struct MH_httpResponse_t {
    bool error_flag; //!< error flag
    MH_string_t* response; //!< response buffer
};

/**
 * @brief This structure includes action result information.
 */
struct MH_actionResult_t {
    uint32_t result_type; //!< action result
    MH_string_t* licensebuf; //!< license buffer
    MH_string_t* serviceidbuf; //!< service id buffer
    MH_string_t* serviceinfobuf; //!< account id buffer
};

/**
 * @brief This structure includes keysystem id.
 */
struct MH_keysystemId_t {
  uint32_t keysystem_id_len; //!< keysystem id length
  uint8_t* keysystem_id; //!< keysystem id
};

/**
 * @brief This structure includes license information.
 */
struct MH_licenseInfo_t {
    MH_nodeList_t* target_node; //!< target node
    MH_date_t* before; //!< before date
    MH_date_t* after; //!< after date
    uint32_t* remaining_time; //!< remaining time
    uint32_t* first_use_remaining_time; //!< use remaining time
};

/**
 * @brief This structure includes Clear data length and Encrypted data length.
 */
struct MH_subSample_t {
    size_t mNumBytesOfClearData; //!< clear data size
    size_t mNumBytesOfEncryptedData; //!< encrypted data size
};

/**
 * @brief This structure includes buf length and data buffer and fd for platform specific buffer.
 */
struct MH_buffer_t {
    size_t len; //!< data size
    uint8_t* data; //!< data buffer
    int fd; //!< File descriptor for platform specific buffer
};

/**
 * @brief This structure includes 8pus output control.
 */
struct MH_8pus_outputcontrol_t {
    u_int32_t   flags;
    struct {
        struct {
            u_int32_t epn;
            u_int32_t cci;
            u_int32_t ict;
            u_int32_t dot;
            u_int32_t aps;
        } basic;
        struct {
            u_int32_t rmm;
            u_int32_t rs;
            u_int32_t epn;
            u_int32_t cci;
            u_int32_t ict;
            u_int32_t aps;
        } dtcp;
    } e;
};

/**
 * @brief This structure includes 8pus information.
 */
struct MH_8pusInfo_t {
  MH_8pus_outputcontrol_t permission_params;
  struct {
      uint32_t flags;
      uint32_t notbefore_count;
      uint32_t notafter_count;
      MH_date_t* notbefore[2];
      MH_date_t* notafter[2];
      MH_date_t* notduring[2];
      int32_t notlongerthan;
      int32_t notmorethan;
      int32_t notlaterthan;
  } temporal_c;
  struct {
      uint32_t flags;
      uint32_t repeat_count_max;
      uint32_t repeat_count_remain;
  } counter_c;
};

/**
 * @brief This structure includes SAS infromation for ms3 session.
 */
struct MH_sasInfo_t {
    MH_8pus_outputcontrol_t outputcontrol;
    MH_string_t* curl;
    bool retentionflag;
    uint32_t numcontentid;
    MH_string_t** content_ids;
};

/**
 * @brief This structure includes SAS Result information for ms3 session.
 */
struct MH_sasResultData_t {
    MH_sasInfo_t* sasinfo;
};

#endif /* __MARLIN_AGENT_HANDLER_TYPE_H__ */
