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

#ifndef __MARLIN_CRYPTO_H__
#define __MARLIN_CRYPTO_H__

#include "MarlinAgentHandler.h"
#include "MarlinCommonTypes.h"
#include "MarlinCommonConfig.h"
#include "MarlinError.h"
#include "CTime.h"

#define MAX_NOTAFTER_COUNT 2

namespace marlincdm {

class MarlinCrypto {
public:

    typedef enum _CryptoStatus {
        CRYPTO_STATUS_UNINITIALIZED = 0,
        CRYPTO_STATUS_DECRYPTION_INITIALIZED
    } CryptoStatus;

    MarlinCrypto();
    MarlinCrypto(mcdm_data_t data);
    virtual ~MarlinCrypto();
    MarlinCrypto* duplicate(void);

    /**
     * Copy data.
     *
     * @param [in] in_data Copy source data
     *
     */
    void copyData(const MarlinCrypto* in_data);

    /**
     * Set crypto info.
     *
     * @param [in] in_cid Content id.
     * @param [in] in_cryptoType Crypto type.
     * @param [in] in_algoType Crypto algorithm type. for KF CBC mode only
     * @param [in] in_padType Padding algorithm type. for KF CBC mode only
     *
     * @return Returns the error code for this API.
     * @return ERROR_UNKNOWN Cannot set crypto info.
     */
    virtual mcdm_status_t setCryptoInfo(const mcdm_data_t& in_cid,
                                        bool isLastTrack,
                                        mcdm_decryptMode in_cryptoType,
                                        mcdm_algorithmType in_algoType,
                                        mcdm_paddingType in_padType);

    /**
     * Clear Crypto info.
     *
     * @return Returns the error code for this API.
     * @return ERROR_UNKNOWN Cannot clear cryoto info.
     */
    virtual mcdm_status_t clearCryptoInfo(void);


    virtual mcdm_status_t decrypt(bool& is_encrypted,
                                  const uint8_t iv[16],
                                  mcdm_buffer_t* srcPtr,
                                  const mcdm_subsample_t* subSamples,
                                  size_t& numSubSamples,
                                  mcdm_buffer_t* dstPtr,
                                  size_t& decrypt_buffer_length,
                                  string& errorDetailMsg);

    virtual mcdm_status_t decryptWithHW(bool& is_encrypted,
                                        const uint8_t iv[16],
                                        mcdm_buffer_t* srcPtr,
                                        const mcdm_subsample_t* subSamples,
                                        size_t& numSubSamples,
                                        mcdm_buffer_t* dstPtr,
                                        size_t& decrypt_buffer_length,
                                        string& errorDetailMsg);

    void setLicenseHandle(MH_licenseHandle_t licenseHandle) {
        mLicenseHandle = licenseHandle;
    }

    CryptoStatus getCryptoStatus() const {
        return crypto_status;
    }

protected:

    MarlinCrypto(const MarlinCrypto &obj);

    CryptoStatus crypto_status;
    MarlinAgentHandler* mHandler;
    MH_agentHandle_t mHandle;
    MH_licenseHandle_t mLicenseHandle;
    MH_trackHandle_t mTrackHandle;
    mcdm_data_t mTrackData;

    bool generateCryptoInfo(mcdm_decryptMode in_cryptoType,
                            mcdm_algorithmType in_algoType,
                            mcdm_paddingType in_padType,
                            MH_cryptoInfo_t** cryptoInfo);

    bool convertSubSampleToAgent(const mcdm_subsample_t* in_subsamples,
                                 size_t numSubSamples,
                                 MH_subSample_t** out_subsamples);

};
}
; //namespace

#endif /* __MARLIN_CRYPTO_H__ */
