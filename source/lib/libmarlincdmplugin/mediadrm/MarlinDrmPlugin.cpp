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

//#define LOG_NDEBUG 0
#define LOG_TAG "MarlinDrmPlugin"
#include <utils/Log.h>

#include <endian.h>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include "MarlinDrmPlugin.h"
#include "MarlinPluginCommon.h"
#include "media/stagefright/MediaErrors.h"
#include "utils/Errors.h"

namespace marlindrm {

MarlinDrmPlugin::MarlinDrmPlugin(MarlinCdmInterface* cdm)
  : mCDM(cdm),
    encIonBuffer(NULL),
    decIonBuffer(NULL),
    cryptoSession("")
{
    mSessionId.clear();
}

MarlinDrmPlugin::~MarlinDrmPlugin() {
    if (!mSessionId.empty()) {
        mCDM->CloseSession(mSessionId);
        mSessionId.clear();
    }
}

status_t MarlinDrmPlugin::openSession(Vector<uint8_t>& sessionId) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id("");

    cdm_status = mCDM->OpenSession(session_id);
    if(cdm_status == marlincdm::OK) {
        sessionId.appendArray(reinterpret_cast<const uint8_t*>(session_id.data()), session_id.length());
        ALOGV("MarlinDrmPlugin::openSession() -> %s", session_id.c_str());

        if (!mSessionId.empty()) {
            mCDM->CloseSession(mSessionId);
            mSessionId.clear();
        }
        mSessionId.assign(session_id);
    }

EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::closeSession(const Vector<uint8_t>& sessionId) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;

    if(convertSessionIdFwToCDM(sessionId, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    cdm_status = mCDM->CloseSession(session_id);
    mSessionId.clear();
EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::getKeyRequest(const Vector<uint8_t>& scope,
                                        const Vector<uint8_t>& initData,
                                        const String8& mimeType,
                                        KeyType keyType,
                                        const KeyedVector<String8, String8>& optionalParameters,
                                        Vector<uint8_t>& request,
                                        String8& defaultUrl) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    mcdm_key_type in_keytype;
    string in_data;
    string in_mimetype(mimeType.string(), mimeType.length());
    string service_id;
    string account_id;
    string subscription_id;
    string o_url;
    string o_request;
    map<string, string> in_optionalParameters;

    in_data = vectorToString(initData);
    if(convertSessionIdFwToCDM(scope, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if(convertKeyTypeFwToCDM(keyType, in_keytype) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    for (size_t i = 0; i < optionalParameters.size(); i++) {
        const String8 name = optionalParameters.keyAt(i);
        String8 value = optionalParameters.valueFor(name);

        in_optionalParameters[string(name.string())] = string(value.string());
    }

    cdm_status = mCDM->GenerateKeyRequest(session_id,
                                          in_data,
                                          &in_mimetype,
                                          &in_keytype,
                                          &in_optionalParameters,
                                          &o_request,
                                          &o_url);
    if (cdm_status != marlincdm::OK) {
        goto EXIT;
    }

    request = stringToVector(o_request);
    defaultUrl.setTo(o_url.data(), o_url.size());

EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::provideKeyResponse(const Vector<uint8_t>& scope,
                                             const Vector<uint8_t>& response,
                                             Vector<uint8_t>& keySetId) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    string in_response;
    string request;
    keySetId.clear();

    if(convertSessionIdFwToCDM(scope, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    in_response = vectorToString(response);
    if(in_response.length() == 0) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    cdm_status = mCDM->AddKey(session_id,
                              in_response,
                              &request);
    if (cdm_status != marlincdm::OK) {
        goto EXIT;
    }
    keySetId = stringToVector(request);
EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::removeKeys(const Vector<uint8_t>& sessionId) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;

    if(convertSessionIdFwToCDM(sessionId, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    cdm_status = mCDM->RemoveKey(session_id);
EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::restoreKeys(const Vector<uint8_t>& sessionId,
                                      const Vector<uint8_t>& init_data) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    string content_data;

    if(convertSessionIdFwToCDM(sessionId, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    content_data = vectorToString(init_data);
    if(content_data.empty()) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    cdm_status = mCDM->RestoreKey(session_id, content_data);
EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::queryKeyStatus(const Vector<uint8_t>& sessionId,
                                         KeyedVector<String8, String8>& infoMap) const {

    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    map<string, string> licenseInfo;
    map<string, string>::const_iterator itr;
    string init_data("");
    String8 name;
    String8 value;

    if (sessionId.size() == 0) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    session_id = mcdm_SessionId_t(reinterpret_cast<const char*>(sessionId.array()), sessionId.size());

    cdm_status = mCDM->QueryKeyStatus(session_id, licenseInfo);
    if (cdm_status != marlincdm::OK) {
        goto EXIT;
    }

    if (licenseInfo.size() > 0) {
        for (itr = licenseInfo.begin(); itr != licenseInfo.end(); itr++) {
            name.setTo(itr->first.c_str(), itr->first.size());
            value.setTo(itr->second.c_str(), itr->second.size());
            infoMap.add(name, value);
        }
    }
EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::getProvisionRequest(Vector<uint8_t>& /*request*/,
                                          String8& /*defaultUrl*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::getProvisionRequest(String8 const& /*cert_type*/,
                                             String8 const& /*cert_authority*/,
                                             Vector<uint8_t>& /*request*/,
                                             String8& /*defaultUrl*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::provideProvisionResponse(const Vector<uint8_t>& /*response*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::provideProvisionResponse(Vector<uint8_t> const& /*response*/,
                                                  Vector<uint8_t>& /*certificate*/,
                                                  Vector<uint8_t>& /*wrapped_key*/){
    return android::OK;
}

status_t MarlinDrmPlugin::unprovisionDevice(){
    return android::OK;
}

status_t MarlinDrmPlugin::getSecureStops(List<Vector<uint8_t> >& /*secureStops*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::releaseSecureStops(const Vector<uint8_t>& /*ssRelease*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::getPropertyString(const String8& name,
                                            String8& value) const {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;

    string in_name = string(name.string());
    string io_value("");

    cdm_status = mCDM->GetPropertyString(in_name, io_value);
    if (cdm_status != marlincdm::OK) {
        goto EXIT;
    }
    value.setTo(io_value.c_str(), io_value.length());

EXIT:
    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::getPropertyByteArray(const String8& /*name*/,
                                               Vector<uint8_t>& /*value*/) const {
    return android::OK;
}

status_t MarlinDrmPlugin::setPropertyString(const String8& /*name*/,
                                            const String8& /*value*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::setPropertyByteArray(const String8& /*name*/,
                                               const Vector<uint8_t>& /*value*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::setCipherAlgorithm(const Vector<uint8_t>& sessionId,
                                         const String8& algorithm) {
    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    if(convertSessionIdFwToCDM(sessionId, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    cryptoSession = CryptoSession(session_id);

    if (algorithm == "AES/CBC/PKCS5Padding") {
        cryptoSession.setCipherAlgorithm(MODE_AES_CBC);
    } else {
        cdm_status = marlincdm::ERROR_UNSUPPORTED_CRYPTO_MODE;
        goto EXIT;
    }

EXIT:
    return android::OK;
}

status_t MarlinDrmPlugin::setMacAlgorithm(const Vector<uint8_t>& /*sessionId*/,
                                      const String8& /*algorithm*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::encrypt(const Vector<uint8_t>& /*sessionId*/,
                              const Vector<uint8_t>& /*keyId*/,
                              const Vector<uint8_t>& /*input*/,
                              const Vector<uint8_t>& /*iv*/,
                              Vector<uint8_t>& /*output*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::decrypt(const Vector<uint8_t>& sessionId,
                              const Vector<uint8_t>& /*keyId*/,
                              const Vector<uint8_t>& input,
                              const Vector<uint8_t>& /*iv*/,
                              Vector<uint8_t>& output) {

    mcdm_status_t cdm_status = marlincdm::OK;
    status_t status = android::OK;
    mcdm_SessionId_t session_id;
    string init_data("");
    bool is_encrypted = true;
    bool is_secure = true;
    mcdm_decryptMode in_mode = MODE_UNENCRYPTED;
    mcdm_subsample_t *in_subSamples = new mcdm_subsample_t[1];
    size_t numSubSamples = 1;

    char* encMem;
    char* decMem;
    string in;
    const uint8_t* source;
    mcdm_buffer_t in_srcPtr = {0, NULL, 0};
    mcdm_buffer_t in_dstPtr = {0, NULL, 0};
    size_t decrypt_buffer_length = 0;
    string errorMsg;

    if(convertSessionIdFwToCDM(sessionId, session_id) != android::OK) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    cdm_status = mCDM->CreateDecryptUnit(session_id, init_data);
    if (cdm_status != marlincdm::OK) {
        ALOGE("call CreateDecryptUnit cdm_status : %d", cdm_status);
        goto EXIT;
    }

    if (session_id != cryptoSession.cdmSessionId()) {
        cdm_status = marlincdm::ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    in_mode = cryptoSession.cipherAlgorithm();

    if (in_mode == MODE_UNENCRYPTED) {
        cdm_status = marlincdm::ERROR_UNSUPPORTED_CRYPTO_MODE;
        goto EXIT;
    }

    in_subSamples[0].mNumBytesOfClearData = 0;
    in_subSamples[0].mNumBytesOfEncryptedData = input.size();

    encIonBuffer = new IonBuf();
    decIonBuffer = new IonBuf();

    if (NULL == encIonBuffer || NULL == decIonBuffer) {
        ALOGE("Ion buffer is NULL");
        cdm_status = marlincdm::ERROR_FAILED_DECRYPTION;
        goto EXIT;
    }

    encMem = (char*)encIonBuffer->getBuffer(input.size());
    decMem = (char*)decIonBuffer->getBuffer(input.size());

    if (NULL == encMem || NULL == decMem) {
        ALOGE("Fail to alloc ion buffer");
        cdm_status = marlincdm::ERROR_FAILED_DECRYPTION;
        goto EXIT;
    }

    in = vectorToString(input);
    source = reinterpret_cast<const uint8_t*>(in.c_str());
    memcpy(encMem, source, input.size());

    in_srcPtr.data = (uint8_t*)encMem;
    in_srcPtr.fd = encIonBuffer->getFd();
    in_srcPtr.len = encIonBuffer->getSize();
    in_dstPtr.data = (uint8_t*)decMem;
    in_dstPtr.fd = decIonBuffer->getFd();
    in_dstPtr.len = decIonBuffer->getSize();

    uint8_t key_data[16];
    uint8_t iv_data[16];
    memset(key_data, 0x00, 16);
    memset(iv_data, 0x00, 16);

    cdm_status = mCDM->Decrypt(session_id,
                               is_encrypted,
                               is_secure,
                               key_data,
                               iv_data,
                               in_mode,
                               &in_srcPtr,
                               in_subSamples,
                               numSubSamples,
                               &in_dstPtr,
                               decrypt_buffer_length,
                               errorMsg);

    if (cdm_status != marlincdm::OK) {
        ALOGE("Decrypt error result in session %s during unencrypted block: %d",
            mSessionId.c_str(), cdm_status);
        goto EXIT;
    }

    if (decrypt_buffer_length > 0) {
        output.appendArray(reinterpret_cast<const uint8_t*>(decMem), decrypt_buffer_length);
    }

EXIT:
    if (encIonBuffer != NULL) {
        delete encIonBuffer;
        encIonBuffer = NULL;
    }
    if (decIonBuffer != NULL) {
        delete decIonBuffer;
        decIonBuffer = NULL;
    }
    if (in_subSamples != NULL) {
        delete[] in_subSamples;
        in_subSamples = NULL;
    }

    return convertMarlinHandlerError(cdm_status);
}

status_t MarlinDrmPlugin::sign(const Vector<uint8_t>& /*sessionId*/,
                           const Vector<uint8_t>& /*keyId*/,
                           const Vector<uint8_t>& /*message*/,
                           Vector<uint8_t>& /*signature*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::verify(const Vector<uint8_t>& /*sessionId*/,
                             const Vector<uint8_t>& /*keyId*/,
                             const Vector<uint8_t>& /*message*/,
                             const Vector<uint8_t>& /*signature*/,
                             bool& /*match*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::signRSA(Vector<uint8_t> const& /*sessionId*/,
                                  String8 const& /*algorithm*/,
                                  Vector<uint8_t> const& /*message*/,
                                  Vector<uint8_t> const& /*wrapped_key*/,
                                  Vector<uint8_t>& /*signature*/) {
    return android::OK;
}

status_t MarlinDrmPlugin::convertKeyTypeFwToCDM(KeyType &i_fwKeyType,
                                                mcdm_key_type &o_cdmKeyType) {
    switch(i_fwKeyType) {
    case kKeyType_Offline:
        o_cdmKeyType = KEY_TYPE_OFFLINE;
        break;
    case kKeyType_Streaming:
        o_cdmKeyType = KEY_TYPE_STREAMING;
        break;
    case kKeyType_Release:
        o_cdmKeyType = KEY_TYPE_RELEASE;
        break;
    default:
        return android::UNKNOWN_ERROR;
        break;
    }
    return android::OK;
}

status_t MarlinDrmPlugin::convertSessionIdFwToCDM(const Vector<uint8_t> &i_fwSessionId,
                                                  mcdm_SessionId_t &o_cdmSessionId) {
    if (&i_fwSessionId == NULL) {
        return android::BAD_VALUE;
    }
    if (i_fwSessionId.size() == 0) {
        return android::BAD_VALUE;
    }
    o_cdmSessionId = mcdm_SessionId_t(reinterpret_cast<const char*>(i_fwSessionId.array()), i_fwSessionId.size());
    return android::OK;
}

string MarlinDrmPlugin::vectorToString(Vector<uint8_t> const &i_vector) {
    string out("");
    if (&i_vector == NULL) {
        return out;
    }
    if (i_vector.size() == 0) {
        return out;
    }
    out.assign(reinterpret_cast<const char*>(i_vector.array()), i_vector.size());
    return out;
}

Vector<uint8_t> MarlinDrmPlugin::stringToVector(string &i_str) {
    Vector<uint8_t> out;
    if (i_str.size() == 0) {
        out.clear();
        return out;
    }
    out.clear();
    out.appendArray(reinterpret_cast<const uint8_t*>(i_str.data()), i_str.length());
    return out;
}
}  // namespace marlindrm
