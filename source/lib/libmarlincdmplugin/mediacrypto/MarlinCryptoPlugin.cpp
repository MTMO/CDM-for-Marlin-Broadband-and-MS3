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
#define LOG_TAG "MarlinCryptoPlugin"
#include <utils/Log.h>

#include <string>
#include <vector>

#include <MarlinConstants.h>
#include "MarlinCryptoPlugin.h"
#include "MarlinPluginCommon.h"
#include "media/stagefright/MediaErrors.h"
#include "utils/Errors.h"
#include "utils/String8.h"

namespace marlindrm {

MarlinCryptoPlugin::MarlinCryptoPlugin(const void* data, size_t size,
                                       MarlinCdmInterface* cdm)
  : mCDM(cdm),
    encIonBuffer(NULL),
    decIonBuffer(NULL)
{
    mcdm_status_t status = OK;
    string init_data("");
    mcdm_SessionId_t sessionId = string(static_cast<const char*>(data), size);

    encIonBuffer = new IonBuf();
    decIonBuffer = new IonBuf();

    license_status = OK;

    status = mCDM->CreateDecryptUnit(sessionId, init_data);

    if (status == ERROR_SESSION_NOT_OPENED) {
        init_data = string(static_cast<const char*>(data), size);
        status = mCDM->OpenSession(sessionId);
        if(status != OK) {
            ALOGE("call OpenSession status : %d", status);
            return;
        }

        status = mCDM->CreateDecryptUnit(sessionId, init_data);
        if(status != OK) {
            ALOGE("call CreateDecryptUnit status : %d", status);
            license_status = status;
            return;
        }
        if (!mSessionId.empty()) {
            mCDM->CloseSession(mSessionId);
            mSessionId.clear();
        }
    }
    mSessionId.assign(sessionId);
}

MarlinCryptoPlugin::~MarlinCryptoPlugin()
{
    if (!mSessionId.empty()) {
        mCDM->CloseSession(mSessionId);
        mSessionId.clear();
    }
    if (encIonBuffer != NULL) {
        delete encIonBuffer;
        encIonBuffer = NULL;
    }
    if (decIonBuffer != NULL) {
        delete decIonBuffer;
        decIonBuffer = NULL;
    }
}

bool MarlinCryptoPlugin::requiresSecureDecoderComponent(const char* mime) const {
    bool ret = false;
    MarlinCdmInterface* cdm = new MarlinCdmInterface(marlincdm::MBB_UUID);
    cdm->useSecureBuffer(reinterpret_cast<const uint8_t*>(mime), ret);

    delete cdm;
    cdm = NULL;

    return ret;
}

// Returns negative values for error code and
// positive values for the size of decrypted data.  In theory, the output size
// can be larger than the input size, but in practice this should never happen
// for AES-CTR.
ssize_t MarlinCryptoPlugin::decrypt(bool secure, const uint8_t key[16],
                                    const uint8_t iv[16], CryptoPlugin::Mode /*mode*/,
                                    const void* srcPtr, const CryptoPlugin::SubSample* subSamples,
                                    size_t numSubSamples, void* dstPtr,
                                    AString* errorDetailMsg) {
    android::status_t status = android::OK;
    mcdm_status_t cdm_status = OK;

    // Convert parameters to the form the CDM wishes to consume them in.
    const uint8_t* const source = static_cast<const uint8_t*>(srcPtr);
    uint8_t* const dest = static_cast<uint8_t*>(dstPtr);
    size_t len = 0;

    bool is_encrypted = true;
    bool is_secure = secure;
    mcdm_decryptMode in_mode = MODE_AES_CBC;
    mcdm_buffer_t in_srcPtr = {0, NULL, 0};
    mcdm_subsample_t *in_subSamples = NULL;
    mcdm_buffer_t in_dstPtr = {0, NULL, 0};
    char* encMem = NULL;
    char* decMem = NULL;
    size_t decrypt_buffer_length = 0;
    string errorMsg;

    //convertDecryptModeFwToMh(mode, in_mode);
    convertSubSampleInfoFwToMh(subSamples, numSubSamples, &in_subSamples);

    for (size_t i = 0; i < numSubSamples; i++) {
        len += subSamples[i].mNumBytesOfClearData;
        len += subSamples[i].mNumBytesOfEncryptedData;
    }

    if (NULL == encIonBuffer || NULL == decIonBuffer) {
        ALOGE("Ion buffer is NULL");
        status = android::NO_MEMORY;
        decrypt_buffer_length = status;
        goto EXIT;
    }

    encMem = (char*)encIonBuffer->getBuffer(len);
    decMem = (char*)decIonBuffer->getBuffer(len);

    if (NULL == encMem || NULL == decMem) {
        ALOGE("Fail to alloc ion buffer");
        status = android::NO_MEMORY;
        decrypt_buffer_length = status;
        goto EXIT;
    }
    memcpy(encMem, source, len);

    in_srcPtr.data = (uint8_t*)encMem;
    in_srcPtr.fd = encIonBuffer->getFd();
    in_srcPtr.len = encIonBuffer->getSize();
    in_dstPtr.data = (uint8_t*)decMem;
    in_dstPtr.fd = secure ? (unsigned int)dstPtr : decIonBuffer->getFd();
    in_dstPtr.len = decIonBuffer->getSize();

    cdm_status = mCDM->Decrypt(mSessionId,
                               is_encrypted,
                               is_secure,
                               key,
                               iv,
                               in_mode,
                               &in_srcPtr,
                               in_subSamples,
                               numSubSamples,
                               &in_dstPtr,
                               decrypt_buffer_length,
                               errorMsg);
    if (cdm_status != OK) {
        ALOGE("Decrypt error result in session %s during unencrypted block: %d",
              mSessionId.c_str(), cdm_status);
        errorDetailMsg->setTo("Error decrypting data.");
        if (license_status != OK) {
            cdm_status = license_status;
            goto EXIT;
        }
        goto EXIT;
    }

    if (!secure && decrypt_buffer_length > 0) {
        memcpy(dstPtr, decMem, decrypt_buffer_length);
    }
    ALOGV("MarlinCryptoPlugin::decrypt decrypt_buffer_length = %d", decrypt_buffer_length);

EXIT:
    if (in_subSamples != NULL) {
        delete[] in_subSamples;
        in_subSamples = NULL;
    }
    if (cdm_status != OK) {
        status = convertMarlinHandlerError(cdm_status);
        return static_cast<ssize_t>(status);
    } else {
        return static_cast<ssize_t>(decrypt_buffer_length);
    }
}

android::status_t MarlinCryptoPlugin::getOutputProtectionFlags(int *data, size_t size) {
    return OK;
}

void MarlinCryptoPlugin::convertDecryptModeFwToMh(const CryptoPlugin::Mode mode, mcdm_decryptMode &out) {
    ALOGV("mode = %d", mode);
    if (mode == CryptoPlugin::kMode_Unencrypted) {
        out = MODE_UNENCRYPTED;
    } else if (mode == CryptoPlugin::kMode_AES_CTR) {
        out = MODE_AES_CTR;
    } else if (mode == CryptoPlugin::kMode_AES_WV) {
        out = MODE_AES_CBC;
    } else {
        out = MODE_UNENCRYPTED;
    }
    ALOGV("converted mode = %d", out);
}

void MarlinCryptoPlugin::convertSubSampleInfoFwToMh(const CryptoPlugin::SubSample *subSamples,
        size_t numSubSamples,
        mcdm_subsample_t **out_subSamples) {
    if (subSamples == NULL || numSubSamples == 0) {
        return;
    }

    *out_subSamples = new mcdm_subsample_t[numSubSamples];
    for (size_t i = 0; i < numSubSamples; i++) {
        (*out_subSamples)[i].mNumBytesOfClearData = subSamples[i].mNumBytesOfClearData;
        (*out_subSamples)[i].mNumBytesOfEncryptedData = subSamples[i].mNumBytesOfEncryptedData;
        ALOGV("### subSamples[%d].mNumBytesOfClearData : %d",i ,subSamples[i].mNumBytesOfClearData);
        ALOGV("### subSamples[%d].mNumBytesOfEncryptedData : %d",i ,subSamples[i].mNumBytesOfEncryptedData);
    }
}
}  // namespace marlindrm
