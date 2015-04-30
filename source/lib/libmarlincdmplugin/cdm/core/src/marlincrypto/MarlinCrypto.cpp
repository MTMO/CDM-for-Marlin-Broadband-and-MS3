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

#define LOG_TAG "MarlinCrypto"
#include "MarlinLog.h"

#include "MarlinCrypto.h"

using namespace marlincdm;

MarlinCrypto::MarlinCrypto(void)
    : crypto_status(CRYPTO_STATUS_UNINITIALIZED)
{
    MARLINLOG_ENTER();
    MH_status_t agentStatus = MH_ERR_OK;
    MH_agentHandle_t handle;
    mHandler = new MarlinAgentHandler();

    agentStatus = mHandler->initAgent(&handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initAgent (%d).", agentStatus);
    }
    mHandle = handle;
    mTrackHandle.handle = -1;
    mTrackHandle.trackinfo.cid = NULL;
    mTrackHandle.trackinfo.cryptoinfo = NULL;
    mLicenseHandle.handle = NULL;
    mLicenseHandle.licensebuf.length = 0;
    mLicenseHandle.licensebuf.bufstr = NULL;
    mTrackData.len = 0;
    mTrackData.data = NULL;
    MARLINLOG_EXIT();
}

MarlinCrypto::MarlinCrypto(mcdm_data_t data)
    : crypto_status(CRYPTO_STATUS_UNINITIALIZED)
{
    MARLINLOG_ENTER();
    MH_status_t agentStatus = MH_ERR_OK;
    mLicenseHandle.handle = NULL;
    mLicenseHandle.licensebuf.length = 0;
    mLicenseHandle.licensebuf.bufstr = NULL;
    mTrackData.len = 0;
    mTrackData.data = NULL;
    MH_agentHandle_t handle;
    mHandler = new MarlinAgentHandler();

    agentStatus = mHandler->initAgent(&handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initAgent (%d).", agentStatus);
    }
    mHandle = handle;
    mTrackHandle.handle = -1;
    mTrackHandle.trackinfo.cid = NULL;
    mTrackHandle.trackinfo.cryptoinfo = NULL;

    if (data.data != NULL && data.len > 0) {
        mTrackData.data = new uint8_t[data.len];
        memcpy(mTrackData.data, data.data, data.len);
        mTrackData.len = data.len;
    }
    MARLINLOG_EXIT();
}

MarlinCrypto::MarlinCrypto(const MarlinCrypto &other)
    : crypto_status(CRYPTO_STATUS_UNINITIALIZED)
{
    MARLINLOG_ENTER();
    MH_status_t agentStatus = MH_ERR_OK;
    MH_agentHandle_t handle;
    mHandler = new MarlinAgentHandler();

    agentStatus = mHandler->initAgent(&handle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initAgent (%d).", agentStatus);
    }
    mHandle = handle;
    this->mLicenseHandle = other.mLicenseHandle;
    mTrackHandle.handle = -1;
    mTrackHandle.trackinfo.cid = NULL;
    mTrackHandle.trackinfo.cryptoinfo = NULL;

    if (other.mTrackData.data != NULL && other.mTrackData.len > 0) {
        this->mTrackData.data = new uint8_t[other.mTrackData.len];
        memcpy(this->mTrackData.data, other.mTrackData.data, other.mTrackData.len);
        this->mTrackData.len = other.mTrackData.len;
    } else {
        this->mTrackData.len = 0;
        this->mTrackData.data = NULL;
    }

    MARLINLOG_EXIT();
}

MarlinCrypto::~MarlinCrypto(void)
{
  MARLINLOG_ENTER();
  if (mTrackHandle.trackinfo.cid != NULL) {
      if (mTrackHandle.trackinfo.cid->bufstr != NULL &&
              mTrackHandle.trackinfo.cid->length > 0) {
          free(mTrackHandle.trackinfo.cid->bufstr);
          mTrackHandle.trackinfo.cid->bufstr = NULL;
      }
      delete mTrackHandle.trackinfo.cid;
      mTrackHandle.trackinfo.cid = NULL;
  }
  if (mTrackHandle.trackinfo.cryptoinfo != NULL) {
      delete mTrackHandle.trackinfo.cryptoinfo;
      mTrackHandle.trackinfo.cryptoinfo = NULL;
  }
  if (mHandler != NULL) {
      mHandler->finAgent(&mHandle);
      delete mHandler;
      mHandler = NULL;
  }
  if (mTrackData.data != NULL) {
      delete mTrackData.data;
      mTrackData.data = NULL;
  }
  crypto_status = CRYPTO_STATUS_UNINITIALIZED;
  MARLINLOG_EXIT();
}

MarlinCrypto* MarlinCrypto::duplicate(void)
{
    return new MarlinCrypto(*this);
}

mcdm_status_t MarlinCrypto::setCryptoInfo(const mcdm_data_t& in_cid,
                                          bool isLastTrack,
                                          mcdm_decryptMode in_cryptoType,
                                          mcdm_algorithmType in_algoType,
                                          mcdm_paddingType in_padType)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_trackHandle_t trackHandle;
    MH_memdata_t dataHeader = {0, NULL};
    MH_memdata_t contentKey = {0, NULL};
    MH_string_t* content_id = NULL;
    MH_trackInfo_t trackInfo = {NULL, NULL};
    MH_cryptoInfo_t* cryptoInfo = NULL;

    cryptoInfo = new MH_cryptoInfo_t;
    if (!generateCryptoInfo(in_cryptoType, in_algoType,
                            in_padType, &cryptoInfo)) {
        LOGE("input parameter is invalid.");
        delete cryptoInfo;
        cryptoInfo = NULL;
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    content_id = new MH_string_t;
    if (in_cid.len > 0 && in_cid.data != NULL) {
        content_id->bufstr = (uint8_t*)malloc(in_cid.len);
        memcpy(content_id->bufstr, in_cid.data, in_cid.len);
        content_id->length = in_cid.len;
    }
    trackInfo.cid = content_id;
    trackInfo.cryptoinfo = cryptoInfo;

    agentStatus = mHandler->initTrack(mHandle, &mLicenseHandle, &trackInfo, &trackHandle);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling initTrack (%d).", agentStatus);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    dataHeader.bufdata = mTrackData.data;
    dataHeader.length = mTrackData.len;
    agentStatus = mHandler->retrieveContentKey(mLicenseHandle,
                                               &trackHandle,
                                               &dataHeader,
                                               &isLastTrack,
                                               &contentKey);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling retrieveContentKey (%d).", agentStatus);
        mHandler->finTrack(mHandle, &mLicenseHandle, &trackHandle);
        if (trackInfo.cid != NULL) {
            if (trackInfo.cid->bufstr != NULL) {
                free(trackInfo.cid->bufstr);
                trackInfo.cid->bufstr = NULL;
            }
            delete trackInfo.cid;
            trackInfo.cid = NULL;
        }
        if (trackInfo.cryptoinfo != NULL) {
            delete trackInfo.cryptoinfo;
            trackInfo.cryptoinfo = NULL;
        }
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    mTrackHandle.handle = trackHandle.handle;
    mTrackHandle.trackinfo.cid = trackInfo.cid;
    mTrackHandle.trackinfo.cryptoinfo = trackInfo.cryptoinfo;
    crypto_status = CRYPTO_STATUS_DECRYPTION_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCrypto::clearCryptoInfo(void) {
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    if (mTrackHandle.handle != -1) {
        agentStatus = mHandler->finTrack(mHandle, &mLicenseHandle, &mTrackHandle);
        mTrackHandle.handle = -1;
        if (agentStatus != MH_ERR_OK) {
            LOGE("calling initTrack (%d).", agentStatus);
            status = ERROR_UNKNOWN;
        }
    }

    if (mTrackHandle.trackinfo.cid != NULL) {
        if (mTrackHandle.trackinfo.cid->bufstr != NULL &&
                mTrackHandle.trackinfo.cid->length > 0) {
            free(mTrackHandle.trackinfo.cid->bufstr);
            mTrackHandle.trackinfo.cid->bufstr = NULL;
        }
        delete mTrackHandle.trackinfo.cid;
        mTrackHandle.trackinfo.cid = NULL;
    }
    if (mTrackHandle.trackinfo.cryptoinfo != NULL) {
        delete mTrackHandle.trackinfo.cryptoinfo;
        mTrackHandle.trackinfo.cryptoinfo = NULL;
    }

    crypto_status = CRYPTO_STATUS_UNINITIALIZED;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MarlinCrypto::decrypt(bool& /*is_encrypted*/,
                                    const uint8_t iv[16],
                                    mcdm_buffer_t* srcPtr,
                                    const mcdm_subsample_t* subSamples,
                                    size_t& numSubSamples,
                                    mcdm_buffer_t* dstPtr,
                                    size_t& decrypt_buffer_length,
                                    string& errorDetailMsg)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    uint8_t key[16];
    MH_subSample_t* in_subsample = NULL;
    MH_buffer_t enc_buffer = {0, NULL, 0};
    MH_buffer_t dec_buffer = {0, NULL, 0};
    MH_memdata_t in_iv = {0, NULL};

    if (srcPtr == NULL || srcPtr->len <= 0 || srcPtr->data == NULL) {
        LOGE("Enc Buffer is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (dstPtr == NULL || dstPtr->len <= 0 || dstPtr->data == NULL) {
        LOGE("Dec Buffer is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (!convertSubSampleToAgent(subSamples, numSubSamples, &in_subsample)) {
        LOGE("sub sample is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    enc_buffer.len = srcPtr->len;
    enc_buffer.data = srcPtr->data;
    enc_buffer.fd = srcPtr->fd;
    dec_buffer.len = dstPtr->len;
    dec_buffer.data = dstPtr->data;
    dec_buffer.fd = dstPtr->fd;

    if (iv != NULL) {
        in_iv.bufdata = const_cast<uint8_t*>(iv);
        in_iv.length = 16;
    }

    agentStatus = mHandler->decrypt(&mLicenseHandle,
                                    &mTrackHandle,
                                    true,
                                    key,
                                    &in_iv,
                                    &enc_buffer,
                                    in_subsample,
                                    numSubSamples,
                                    &dec_buffer,
                                    decrypt_buffer_length,
                                    errorDetailMsg);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling decrypt (%d).", agentStatus);
        decrypt_buffer_length = 0;
        status = ERROR_FAILED_DECRYPTION;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    if (in_subsample != NULL) {
        delete[] in_subsample;
        in_subsample = NULL;
    }
    return status;
}

mcdm_status_t MarlinCrypto::decryptWithHW(bool& /*is_encrypted*/,
                                          const uint8_t iv[16],
                                          mcdm_buffer_t* srcPtr,
                                          const mcdm_subsample_t* subSamples,
                                          size_t& numSubSamples,
                                          mcdm_buffer_t* dstPtr,
                                          size_t& decrypt_buffer_length,
                                          string& errorDetailMsg)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;
    uint8_t key[16];
    MH_subSample_t* in_subsample = NULL;
    MH_buffer_t enc_buffer = {0, NULL, 0};
    MH_buffer_t dec_buffer = {0, NULL, 0};
    MH_memdata_t in_iv = {0, NULL};

    if (srcPtr == NULL || srcPtr->len == 0 || srcPtr->fd < 0) {
        LOGE("Enc Buffer is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (dstPtr == NULL || dstPtr->len == 0 || dstPtr->fd < 0) {
        LOGE("Dec Buffer is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (!convertSubSampleToAgent(subSamples, numSubSamples, &in_subsample)) {
        LOGE("sub sample is invalid parameter.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    enc_buffer.len = srcPtr->len;
    enc_buffer.data = srcPtr->data;
    enc_buffer.fd = srcPtr->fd;
    dec_buffer.len = dstPtr->len;
    dec_buffer.data = dstPtr->data;
    dec_buffer.fd = dstPtr->fd;

    if (iv != NULL) {
        in_iv.bufdata = const_cast<uint8_t*>(iv);
        in_iv.length = 16;
    }

    agentStatus = mHandler->decryptWithHW(&mLicenseHandle,
                                          &mTrackHandle,
                                          true,
                                          key,
                                          &in_iv,
                                          &enc_buffer,
                                          in_subsample,
                                          numSubSamples,
                                          &dec_buffer,
                                          decrypt_buffer_length,
                                          errorDetailMsg);
    if (agentStatus != MH_ERR_OK) {
        LOGE("calling decryptWithHW (%d).", agentStatus);
        decrypt_buffer_length = 0;
        status = ERROR_FAILED_DECRYPTION;
        goto EXIT;
    }
EXIT:
    MARLINLOG_EXIT();
    if (in_subsample != NULL) {
        delete[] in_subsample;
        in_subsample = NULL;
    }
    return status;
}

bool MarlinCrypto::generateCryptoInfo(mcdm_decryptMode in_cryptoType,
                                      mcdm_algorithmType in_algoType,
                                      mcdm_paddingType in_padType,
                                      MH_cryptoInfo_t** cryptoInfo)
{
    MARLINLOG_ENTER();
    bool ret = true;

    if (cryptoInfo == NULL || (*cryptoInfo) == NULL) {
        return false;
    }

    switch (in_cryptoType) {
    case MODE_AES_CTR:
        (*cryptoInfo)->type = in_cryptoType;
        break;
    case MODE_AES_CBC:
        (*cryptoInfo)->type = in_cryptoType;
        (*cryptoInfo)->data.kfcbc.selenc = 0;
        (*cryptoInfo)->data.kfcbc.algo = in_algoType;
        (*cryptoInfo)->data.kfcbc.padding = in_padType;
        break;
    default:
        LOGV("Crypto Type is unsupported.");
        ret = false;
        goto EXIT;
        break;
    }

EXIT:
    MARLINLOG_EXIT();
    return ret;
}

bool MarlinCrypto::convertSubSampleToAgent(const mcdm_subsample_t* in_subsamples,
                                           size_t numSubSamples,
                                           MH_subSample_t** out_subsamples)
{
    if (in_subsamples == NULL || numSubSamples == 0) {
        return false;
    }
    *out_subsamples = new MH_subSample_t[numSubSamples];
    for (size_t i = 0; i < numSubSamples; i++) {
        (*out_subsamples)[i].mNumBytesOfClearData = in_subsamples[i].mNumBytesOfClearData;
        (*out_subsamples)[i].mNumBytesOfEncryptedData = in_subsamples[i].mNumBytesOfEncryptedData;
        LOGV("subSamples[%d].mNumBytesOfClearData : %d",i ,in_subsamples[i].mNumBytesOfClearData);
        LOGV("subSamples[%d].mNumBytesOfEncryptedData : %d",i ,in_subsamples[i].mNumBytesOfEncryptedData);
    }
    return true;
}

