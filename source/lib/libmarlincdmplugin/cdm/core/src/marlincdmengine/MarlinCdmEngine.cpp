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

#define LOG_TAG "MarlinCdmEngine"
#include "MarlinLog.h"

#include <sys/stat.h>
#include <cstring>
#include <cerrno>

#include "MarlinConstants.h"
#include "MarlinCdmEngine.h"
#include "CdmSessionManager.h"
#include "MarlinFunction.h"
#include "MBBFunction.h"
#include "MS3Function.h"
#include "PropInitFormatData.h"

#define VIDEO_TYPE "video/"
#define UUID_LENGTH 16

using namespace marlincdm;

static map<mcdm_SessionId_t, IMarlinFunction*> mCdmSessionMap;
static CMutex sCdmSessionMapMutex;

MarlinCdmEngine::MarlinCdmEngine(const uint8_t uuid[16]){
    MARLINLOG_ENTER();
    int res = mkdir(HOME_DIRECTORY.c_str(), 0700);
    if (0 != res && EEXIST != errno) {
        LOGE("Could not create marlind directory");
    }
    if (uuid == NULL) {
        LOGE("uuid is NULL");
        return;
    }
    memcpy(mUUID, uuid, UUID_LENGTH);
    MARLINLOG_EXIT();
};

MarlinCdmEngine::~MarlinCdmEngine(){};

mcdm_status_t MarlinCdmEngine::OpenSession(mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;
    CdmSessionManager& sm(CdmSessionManager::getCdmSessionManager());

    if (sm.getCdmSessionId(session_id) != OK) {
        LOGE("Could not create session id");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    sCdmSessionMapMutex.lock();
    if (mCdmSessionMap.count(session_id) != 0) {
        LOGE("invalid session id");
        session_id = "";
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT1;
    }

    marlinFunction = createMarlinFunction(session_id, mUUID);
    if (marlinFunction == NULL) {
        LOGE("Could not create function");
        status = ERROR_UNKNOWN;
        goto EXIT1;
    }
    mCdmSessionMap[session_id] = marlinFunction;

EXIT1:
    sCdmSessionMapMutex.unlock();
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::CloseSession(const mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    sCdmSessionMapMutex.lock();
    if (mCdmSessionMap.count(session_id) == 0) {
        sCdmSessionMapMutex.unlock();
        LOGE("invalid session id");
        status = ERROR_SESSION_NOT_OPENED;
    } else {
        sCdmSessionMapMutex.unlock();
        IMarlinFunction* marlinFunction = getMarlinFunction(session_id);
        delete marlinFunction;

        sCdmSessionMapMutex.lock();
        mCdmSessionMap.erase(session_id);
        sCdmSessionMapMutex.unlock();
    }
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::CreateDecryptUnit(const mcdm_SessionId_t& session_id, const string& init_data)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    IMarlinFunction* marlinFunction = NULL;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGD("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }

    if (!init_data.empty()) {
        CommonHeaderParser common_header_parser;
        parse_status_t parse_result = INIT_DATA_PARSE_OK;

        parse_result = common_header_parser.parse(init_data);
        if (parse_result != INIT_DATA_PARSE_OK) {
            LOGE("Could not parse Init Data %d", parse_result);
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }

        if (common_header_parser.getProcessTypeFlag().compare(PROCESS_TYPE_ANDROID) != 0) {
            LOGE("Process Type is illegal.");
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }

        if (!common_header_parser.getDataTypeFlag().compare(DATA_TYPE_IPMP)
                || !common_header_parser.getDataTypeFlag().compare(DATA_TYPE_CENC)) {
            status = marlinFunction->initUnit(init_data, NULL);

        } else {
            LOGE("Could not parse data type");
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }
    } else {
        status = marlinFunction->initUnit(init_data, NULL);
    }

EXIT:
    MARLINLOG_EXIT();
    if (status == ERROR_ALREADY_RIGHTS_SET) {
        status = OK;
    }
    return status;
}

mcdm_status_t MarlinCdmEngine::CloseDecryptUnit(const mcdm_SessionId_t& /*session_id*/)
{
    return OK;
}

mcdm_status_t MarlinCdmEngine::GenerateKeyRequest(const mcdm_SessionId_t& session_id,
                                                  const string& init_data,
                                                  const string* mimeType,
                                                  const mcdm_key_type* keyType,
                                                  map<string, string>* optionalParameters,
                                                  string* request,
                                                  string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    DATA_TYPE_FLAG dataType("");

    if (keyType == NULL || mimeType == NULL) {
        LOGE("Key Type or Mime Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if ((0 == memcmp(mimeType->c_str(), MIME_TYPE_REGISTRATION.c_str(), MIME_TYPE_REGISTRATION.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_DEREGISTRATION.c_str(), MIME_TYPE_DEREGISTRATION.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_LICENSE.c_str(), MIME_TYPE_LICENSE.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_CENC.c_str(), MIME_TYPE_CENC.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_IPMP.c_str(), MIME_TYPE_IPMP.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_SAS.c_str(), MIME_TYPE_SAS.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_LTP.c_str(), MIME_TYPE_LTP.length()))
            || (0 == memcmp(mimeType->c_str(), MIME_TYPE_PROPERTY.c_str(), MIME_TYPE_PROPERTY.length()))) {
        // Android Case
        CommonHeaderParser common_header_parser;
        parse_status_t parse_result = INIT_DATA_PARSE_OK;

        parse_result = common_header_parser.parse(init_data);
        if (parse_result != INIT_DATA_PARSE_OK) {
            LOGE("Could not parse Init Data %d", parse_result);
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }
        dataType = common_header_parser.getDataTypeFlag();

        status = generateKeyRequestForAndroid(session_id,
                                              init_data,
                                              mimeType,
                                              keyType,
                                              dataType,
                                              optionalParameters,
                                              request,
                                              url);

    } else if ((0 == memcmp(mimeType->c_str(), MIME_TYPE_EME_CENC.c_str(), MIME_TYPE_EME_CENC.length()))
                || (0 == memcmp(mimeType->c_str(), MIME_TYPE_EME_KEYIDS.c_str(), MIME_TYPE_EME_KEYIDS.length()))) {
        // EME Case
        status = generateKeyRequestForEME(session_id,
                                          init_data,
                                          keyType,
                                          request,
                                          url);
    } else {
        LOGE("Mime Type is illegal");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::AddKey(const mcdm_SessionId_t& session_id,
                                      const string& key_data,
                                      string* request)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    if (request == NULL) {
        LOGE("request is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }

    status = marlinFunction->processRightsResponse(key_data, request);
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::CancelKeyRequest(const mcdm_SessionId_t& /*session_id*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::GetProvisionRequest(string& /*request*/,
                                                   string& /*default_url*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::ProvideProvisionResponse(string& /*response_data*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::useSecureBuffer(const uint8_t* mime,
                                               bool& is_secure)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    if (mime == NULL) {
        is_secure = false;
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (!memcmp(mime, VIDEO_TYPE, sizeof(VIDEO_TYPE) - 1)) {
        is_secure = true;
    } else {
        is_secure = false;
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::isTypeSupported(const string& key_system,
                                               const string& init_data_type,
                                               const string& content_type,
                                               const string& capability,
                                               string* isTypeSupportedResult)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinFunction *function = NULL;
    function = new MarlinFunction();

    if (function == NULL) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    status = function->isTypeSupported(key_system, init_data_type, content_type, capability, isTypeSupportedResult);

EXIT:
    MARLINLOG_EXIT();
    if (function != NULL) {
        delete function;
        function = NULL;
    }
    return status;
}


mcdm_status_t MarlinCdmEngine::RestoreKey(const mcdm_SessionId_t& session_id,
                                          const string& init_data)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;
    CommonHeaderParser common_header_parser;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }

    parse_result = common_header_parser.parse(init_data);
    if (parse_result  != INIT_DATA_PARSE_OK) {
        LOGE("Could not parse Init Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (!DATA_TYPE_IPMP.compare(common_header_parser.getDataTypeFlag())
            || !DATA_TYPE_CENC.compare(common_header_parser.getDataTypeFlag())) {

        status = marlinFunction->restoreKeys(init_data, NULL);
    } else {
        LOGV("Format Type is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::RemoveKey(const mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }
    status = marlinFunction->removeKeys(NULL);

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::QueryKeyStatus(const mcdm_SessionId_t &session_id,
                                              map<string, string> &infoMap)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }
    status = marlinFunction->queryKeyStatus(infoMap);
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::GetPropertyString(const string& name,
                                                 string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction *function = NULL;
    function = new MarlinFunction();

    if (function == NULL) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    status = function->getPropertyString(name, value);

EXIT:
    MARLINLOG_EXIT();
    if (function != NULL) {
        delete function;
        function = NULL;
    }
    return status;
}

mcdm_status_t MarlinCdmEngine::GetPropertyByteArray(const string& /*name*/,
                                           uint8_t& /*value*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::SetPropertyString(const string& /*name*/,
                                                 string& /*value*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::SetPropertyByteArray(const string& /*name*/,
                                                    uint8_t& /*value*/)
{
    return ERROR_UNKNOWN;
}

mcdm_status_t MarlinCdmEngine::Decrypt(const mcdm_SessionId_t& session_id,
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
                                       string& errorDetailMsg)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_SESSION_NOT_OPENED;
        string err_message;
        convertDecryptErrorToMessage(status, err_message);
        errorDetailMsg.assign(err_message);
        goto EXIT;
    }

    status = marlinFunction->decrypt(is_encrypted,
                                     is_secure,
                                     key,
                                     iv,
                                     mode,
                                     srcPtr,
                                     subSamples,
                                     numSubSamples,
                                     dstPtr,
                                     decrypt_buffer_length,
                                     errorDetailMsg);
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::generateKeyRequestForAndroid(const mcdm_SessionId_t& session_id,
                                                            const string& init_data,
                                                            const string* mimeType,
                                                            const mcdm_key_type* keyType,
                                                            const DATA_TYPE_FLAG& dataType,
                                                            map<string, string>* optionalParameters,
                                                            string* request,
                                                            string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    if (((!DATA_TYPE_CENC.compare(dataType) && !MIME_TYPE_IPMP.compare(*mimeType))
            || (!DATA_TYPE_IPMP.compare(dataType) && !MIME_TYPE_CENC.compare(*mimeType)))
            && (*keyType == KEY_TYPE_RELEASE)) {
        // remove key
        marlinFunction = getMarlinFunction(session_id);
        if (marlinFunction == NULL) {
            LOGE("not found session");
            status = ERROR_SESSION_NOT_OPENED;
            goto EXIT;
        }

        status = marlinFunction->removeKeys(init_data, request);

    } else if ((!DATA_TYPE_ATKN.compare(dataType))
            || (!DATA_TYPE_MS3.compare(dataType) && !MIME_TYPE_SAS.compare(*mimeType))) {

        marlinFunction = getMarlinFunction(session_id);
        if (marlinFunction == NULL) {
            LOGE("not found session");
            status = ERROR_SESSION_NOT_OPENED;
            goto EXIT;
        }

        status = marlinFunction->processServiceToken(mimeType,
                                                     keyType,
                                                     init_data,
                                                     optionalParameters,
                                                     request,
                                                     url);
    } else if (!DATA_TYPE_LTP.compare(dataType)
            && !MIME_TYPE_LTP.compare(*mimeType)
            && *keyType == KEY_TYPE_OFFLINE) {
        // LTP Process
        marlinFunction = getMarlinFunction(session_id);
        if (marlinFunction == NULL) {
            LOGE("not found session");
            status = ERROR_SESSION_NOT_OPENED;
            goto EXIT;
        }
        if (!memcmp(mUUID, MBB_UUID, sizeof(mUUID))) {
            status = ((MBBFunction*)marlinFunction)->processLTPMessage(mimeType,
                                                                       keyType,
                                                                       init_data,
                                                                       request,
                                                                       url);
        } else {
            LOGE("session id is illegal.");
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }
    } else if (!DATA_TYPE_PROP.compare(dataType)
            && !MIME_TYPE_PROPERTY.compare(*mimeType)) {
        // get property
        string value;
        if (request == NULL) {
            LOGD("out put value is NULL");
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }
        status = getPropertyStringFormatString(session_id, init_data, value);
        request->assign(value);
    } else {
        LOGE("Could not parse data type");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::generateKeyRequestForEME(const mcdm_SessionId_t& session_id,
                                                        const string& init_data,
                                                        const mcdm_key_type* keyType,
                                                        string* request,
                                                        string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinFunction* marlinFunction = NULL;

    marlinFunction = getMarlinFunction(session_id);
    if (marlinFunction == NULL) {
        LOGE("not found session");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    marlinFunction->setIsEme(true);

    status = marlinFunction->processServiceToken(keyType,
                                                 init_data,
                                                 request,
                                                 url);

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCdmEngine::getPropertyStringFormatString(const mcdm_SessionId_t& session_id,
                                                             const string& name,
                                                             string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    PropInitFormatData prop_format_parser;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    parse_result = prop_format_parser.parse(name);
    if (parse_result != INIT_DATA_PARSE_OK) {
        LOGE("Could not parse Init Data %d", parse_result);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (!QUERY_NAME_REGISTERED_STATE.compare(prop_format_parser.getName())) {
        // REGISTERED STATUS
        MBBFunction *function = new MBBFunction();
        status = function->getRegisteredStatus(prop_format_parser.getServiceId(),
                                               prop_format_parser.getAccountId(),
                                               prop_format_parser.getSubscriptionId(),
                                               value);
        delete function;
        function = NULL;
    } else {
        IMarlinFunction *function = new MarlinFunction();
        status = function->getPropertyFormatString(prop_format_parser.getName(), value);
        delete function;
        function = NULL;
    }

    if (status != OK) {
        // query key status
        IMarlinFunction *function = NULL;
        function = getMarlinFunction(session_id);
        if (function == NULL) {
            LOGE("not found session");
            status = ERROR_SESSION_NOT_OPENED;
            goto EXIT;
        }
        status = function->queryKeyStatus(prop_format_parser.getName(), value);
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

IMarlinFunction* MarlinCdmEngine::createMarlinFunction(const mcdm_SessionId_t& session_id,
                                                       const u_int8_t uuid[16])
{
    MARLINLOG_ENTER();
    IMarlinFunction* marlinFunction = NULL;
    if (!memcmp(uuid, MBB_UUID, UUID_LENGTH)) {
        marlinFunction = new MBBFunction(session_id);
    } else if (!memcmp(uuid, MS3_UUID, UUID_LENGTH)) {
        marlinFunction = new MS3Function(session_id);
    } else {
        LOGD("UUID not supported.");
    }
    MARLINLOG_EXIT();
    return marlinFunction;
}

IMarlinFunction* MarlinCdmEngine::getMarlinFunction(const mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    IMarlinFunction* marlinFunction = NULL;
    sCdmSessionMapMutex.lock();
    if (mCdmSessionMap.count(session_id) == 0) {
        LOGD("invalid session id.");
    } else {
        marlinFunction = mCdmSessionMap[session_id];
    }
    sCdmSessionMapMutex.unlock();
    MARLINLOG_EXIT();
    return marlinFunction;
}
