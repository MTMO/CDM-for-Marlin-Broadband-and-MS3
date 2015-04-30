/*
 * Copyright (C) 2015 Marlin Trust Management Organization
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

#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>

#include "prlog.h"
#include "gmp-decryption.h"
#include "gmp-entrypoints.h"

#include <json/json.h>
#include <MarlinConstants.h>
#include <MarlinCdmInterface.h>

#ifdef PR_LOGGING
PRLogModuleInfo*
GetMarlinLog()
{
  static PRLogModuleInfo *sLog;
  if (!sLog)
    sLog = PR_NewLogModule("MARLIN");
  return sLog;
}

#define LOGD(msg) PR_LOG(GetMarlinLog(), PR_LOG_DEBUG, msg)
#define LOG(level, msg) PR_LOG(GetMarlinLog(), (level), msg)
#else
#define LOGD(msg)
#define LOG(leve1, msg)
#endif

#if defined(_MSC_VER)
#define PUBLIC_FUNC __declspec(dllexport)
#else
#define PUBLIC_FUNC
#endif

using namespace marlincdm;
using namespace std;

typedef string eme_key_id;
typedef string eme_key;
typedef map<eme_key_id, eme_key> eme_key_set;

static GMPPlatformAPI* sPlatformAPI = nullptr;

class MarlinDecryptor;

static vector<MarlinDecryptor*> sDecryptors;

class MarlinDecryptor : public GMPDecryptor {
public:
    explicit MarlinDecryptor(GMPDecryptorHost* aHost)
      : mCDM(nullptr)
      , mCallback(nullptr)
      , mHost(aHost)
      , mDecryptNumber(0)
    {
        LOGD(("MarlinDecryptor"));
    }

    /* static */
    MarlinDecryptor*
    Get(const GMPEncryptedBufferMetadata* aCryptoData)
    {
      const string kid((const char*)aCryptoData->KeyId(), aCryptoData->KeyIdSize());

      auto itr = find_if(sDecryptors.rbegin(),
                         sDecryptors.rend(),
                         [&](MarlinDecryptor* d)->bool{ return d->CanDecryptKey(kid); });

      return (itr != sDecryptors.rend()) ? *itr : nullptr;
    }

    bool
    CanDecryptKey(const string& aKeyId)
    {
      return mCallback && mKeySet.find(aKeyId) != mKeySet.end();
    }

    virtual void Init(GMPDecryptorCallback* aCallback)
    {
        LOGD(("MarlinDecryptor::Init"));
        mCDM = new MarlinCdmInterface(MBB_UUID);
        mCallback = aCallback;
        sDecryptors.push_back(this);
    }

    virtual void CreateSession(
                               uint32_t aCreateSessionToken,
                               uint32_t aPromiseId,
                               const char* aInitDataType,
                               uint32_t aInitDataTypeSize,
                               const uint8_t* aInitData,
                               uint32_t aInitDataSize,
                               GMPSessionType aSessionType)
    {
        LOGD(("MarlinDecryptor::CreateSession"));
        mcdm_status_t ret = OK;
        mcdm_SessionId_t sessionId = "";
        string dataType = string(aInitDataType, aInitDataTypeSize);
        string initdata = string((const char*)aInitData, aInitDataSize);
        mcdm_key_type in_keytype = KEY_TYPE_STREAMING;
        string o_request;

        ret = mCDM->OpenSession(sessionId);
        if (ret == OK) {
            if (aSessionType == kGMPPersistentSession) {
                in_keytype = KEY_TYPE_OFFLINE;
            }

            ret = mCDM->GenerateKeyRequest(sessionId,
                                           initdata,
                                           &dataType,
                                           &in_keytype,
                                           nullptr,
                                           &o_request,
                                           nullptr);
        }

        if (ret != OK) {
            string msg = "Failed to CreateSession.";
            mCallback->SessionError(nullptr, 0,
                                    kGMPInvalidStateError,
                                    42,
                                    msg.c_str(),
                                    msg.size());
        } else {
            mCallback->SessionMessage(sessionId.c_str(),
                                      sessionId.size(),
                                      kGMPLicenseRequest,
                                      (uint8_t*)o_request.c_str(),
                                      o_request.size());
        }
    }

    virtual void LoadSession(uint32_t aPromiseId,
                             const char* aSessionId,
                             uint32_t aSessionIdLength)
    {
        LOGD(("MarlinDecryptor::LoadSession"));
    }

    virtual void UpdateSession(uint32_t aPromiseId,
                               const char* aSessionId,
                               uint32_t aSessionIdLength,
                               const uint8_t* aResponse,
                               uint32_t aResponseSize)
    {
        LOGD(("MarlinDecryptor::UpdateSession"));
        mcdm_status_t ret = OK;
        mcdm_SessionId_t sessionId = mcdm_SessionId_t(aSessionId, aSessionIdLength);
        string response = string((const char*)aResponse, aResponseSize);
        string o_request;
        respose_status_t status = ERROR;

        ret = mCDM->AddKey(sessionId,
                           response,
                           &o_request);
        if (ret == OK) {
            status = getResponseStatus(o_request);
        }

        if ((ret != OK) || (status == ERROR)) {
            const string msg = "Failed to UpdateSession.";
            mCallback->SessionError(aSessionId,
                                    aSessionIdLength,
                                    kGMPInvalidStateError,
                                    42,
                                    msg.c_str(),
                                    msg.size());
        } else if (status == HTTP_REQUEST) {
            mCallback->SessionMessage(aSessionId,
                                      aSessionIdLength,
                                      kGMPLicenseRenewal,
                                      (uint8_t*)o_request.c_str(),
                                      o_request.size());
        } else {
            // complete process
            mCallback->KeyStatusChanged(aSessionId,
                                       aSessionIdLength,
                                       (uint8_t*)o_request.c_str(),
                                       o_request.length(),
                                       kGMPUsable);
        }
    }

    virtual void CloseSession(uint32_t aPromiseId,
                              const char* aSessionId,
                              uint32_t aSessionIdLength)
    {
        mcdm_SessionId_t sessionId = mcdm_SessionId_t(aSessionId, aSessionIdLength);
        mCDM->CloseSession(sessionId);
    }

    virtual void RemoveSession(uint32_t aPromiseId,
                               const char* aSessionId,
                               uint32_t aSessionIdLength)
    {
    }

    virtual void SetServerCertificate(uint32_t aPromiseId,
                                      const uint8_t* aServerCert,
                                      uint32_t aServerCertSize)
    {
    }

    virtual void Decrypt(GMPBuffer* aBuffer,
                         GMPEncryptedBufferMetadata* aMetadata)
    {
        mcdm_status_t ret = OK;
        mcdm_SessionId_t sessionId;
        bool isEncrypted = true;
        bool isSecure = true;
        mcdm_decryptMode mode = MODE_AES_CTR;
        mcdm_buffer_t srcPtr = {0, NULL, 0};
        mcdm_buffer_t dstPtr = {0, NULL, 0};
        uint8_t key[16] = {0};
        size_t numSubSamples = 0;
        mcdm_subsample_t *subSamples = NULL;
        size_t decrypt_buffer_length = 0;
        string errorMsg;

        if ((aBuffer == NULL) || (aMetadata == NULL)) {
            ret = ERROR_ILLEGAL_ARGUMENT;
        } else {
            numSubSamples = aMetadata->NumSubsamples();
            subSamples = new mcdm_subsample_t[numSubSamples];
            sessionId = mcdm_SessionId_t((const char*)aMetadata->KeyId(),
                                         aMetadata->KeyIdSize());

            srcPtr.data = aBuffer->Data();
            srcPtr.len = aBuffer->Size();
            dstPtr.data = new uint8_t[aBuffer->Size()];
            dstPtr.len = aBuffer->Size();

            convertSubSampleInfoFwToMh(aMetadata, &subSamples);
            ret = mCDM->Decrypt(sessionId,
                                isEncrypted,
                                isSecure,
                                key,
                                aMetadata->IV(),
                                mode,
                                &srcPtr,
                                subSamples,
                                numSubSamples,
                                &dstPtr,
                                decrypt_buffer_length,
                                errorMsg);
        }

        if (ret != OK) {
            mCallback->Decrypted(aBuffer, GMPCryptoErr);
        } else {
            aBuffer->Resize(decrypt_buffer_length);
            memcpy(aBuffer->Data(), dstPtr.data, decrypt_buffer_length);
            mCallback->Decrypted(aBuffer, GMPNoErr);
        }

        if (dstPtr.data != NULL) {
            delete[] dstPtr.data;
            dstPtr.data = NULL;
        }
        if (subSamples != NULL) {
            delete[] subSamples;
            subSamples = NULL;
        }
    }

    virtual void DecryptingComplete()
    {
    }

private:

    enum respose_status_t {
        ERROR = 0,
        HTTP_REQUEST,
        COMPLETE
    };

    void convertSubSampleInfoFwToMh(GMPEncryptedBufferMetadata* aMetadata,
                                    mcdm_subsample_t **out_subSamples) {

        const uint32_t num_subsamples = aMetadata->NumSubsamples();
        const uint16_t* clear_bytes = aMetadata->ClearBytes();
        const uint32_t* cipher_bytes = aMetadata->CipherBytes();

        for (uint32_t i = 0; i < num_subsamples; i++) {
          (*out_subSamples)[i].mNumBytesOfClearData = clear_bytes[i];
          (*out_subSamples)[i].mNumBytesOfEncryptedData = cipher_bytes[i];
        }
    }

    respose_status_t getResponseStatus(const string request) {
        bool json_ret = false;
        const string prop_key = "properties";
        const string http_request_key = "httpRequest";
        const string complete_key = "completeNotification";
        Json::Reader reader;
        Json::Value root;

        json_ret = reader.parse(request, root);
        if (!json_ret || !root.isObject() || !root.isMember(prop_key)) {
            return ERROR;
        }

        if(root[prop_key].isMember(http_request_key) &&
                root[prop_key][http_request_key].isObject()){
            return HTTP_REQUEST;
        }

        if(root[prop_key].isMember(complete_key) &&
                root[prop_key][complete_key].isObject()){
            return COMPLETE;
        }
        return ERROR;
    }

    MarlinCdmInterface* mCDM;

    GMPDecryptorCallback* mCallback;
    class MessageTask : public GMPTask {
        public:
        MessageTask(GMPDecryptorCallback* aCallback,
                const string& aSessionId,
                const string& aMessage)
        : mCallback(aCallback)
        , mSessionId(aSessionId)
        , mMessage(aMessage)
        {
        }

        virtual void Destroy() override {
            delete this;
        }

        virtual void Run() override {
            mCallback->SessionMessage(mSessionId.c_str(),
                                      (uint32_t)mSessionId.size(),
                                      kGMPLicenseRenewal,
                                      (const uint8_t*)mMessage.c_str(),
                                      (uint32_t)mMessage.size());
        }
        GMPDecryptorCallback* mCallback;
        string mSessionId;
        string mMessage;
    };

    GMPDecryptorHost* mHost;
    uint32_t mDecryptNumber;
    eme_key_set mKeySet;

}; // MarlinDecryptor class

extern "C" {

PUBLIC_FUNC GMPErr
GMPInit(GMPPlatformAPI* aPlatformAPI)
{
    LOGD(("MarlinDecryptor GMPInit"));
    sPlatformAPI = aPlatformAPI;
    return GMPNoErr;
}

PUBLIC_FUNC GMPErr
GMPGetAPI(const char* aApiName, void* aHostAPI, void** aPluginApi)
{
    LOGD(("MarlinDecryptor GMPGetAPI"));
    if (!strcmp (aApiName, "eme-decrypt")) {
        *aPluginApi = new MarlinDecryptor(reinterpret_cast<GMPDecryptorHost*>(aHostAPI));
      return GMPNoErr;
    }
    return GMPGenericErr;
}

PUBLIC_FUNC void
GMPShutdown(void)
{
    LOGD(("MarlinDecryptor GMPShutdown"));
    sPlatformAPI = nullptr;
}

} // extern "C"
