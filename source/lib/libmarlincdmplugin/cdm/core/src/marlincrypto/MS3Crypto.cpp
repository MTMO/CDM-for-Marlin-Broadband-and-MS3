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

#define LOG_TAG "MS3Crypto"
#include "MarlinLog.h"

#include "MS3Crypto.h"

using namespace marlincdm;

MS3Crypto::MS3Crypto():
        mSasinfo(NULL)
{
}

MS3Crypto::~MS3Crypto()
{
}

mcdm_status_t MS3Crypto::setCryptoInfo(const mcdm_data_t& in_cid,
                                       bool isLastTrack,
                                       mcdm_decryptMode in_cryptoType,
                                       mcdm_algorithmType in_algoType,
                                       mcdm_paddingType in_padType)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MH_status_t agentStatus = MH_ERR_OK;

    MH_trackHandle_t trackHandle;
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

    mTrackHandle.handle = trackHandle.handle;
    mTrackHandle.trackinfo.cid = trackInfo.cid;
    mTrackHandle.trackinfo.cryptoinfo = trackInfo.cryptoinfo;
    crypto_status = CRYPTO_STATUS_DECRYPTION_INITIALIZED;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Crypto::clearCryptoInfo(void) {
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
