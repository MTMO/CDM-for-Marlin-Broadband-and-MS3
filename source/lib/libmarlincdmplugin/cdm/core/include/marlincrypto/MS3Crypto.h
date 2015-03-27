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

#ifndef __MS3_CRYPTO_H__
#define __MS3_CRYPTO_H__

#include "MarlinCrypto.h"

namespace marlincdm {

class MS3Crypto : public MarlinCrypto {
public:

    MS3Crypto();
    virtual ~MS3Crypto();

    virtual mcdm_status_t setCryptoInfo(const mcdm_data_t& in_cid,
                                        bool isLastTrack,
                                        mcdm_decryptMode in_cryptoType,
                                        mcdm_algorithmType in_algoType,
                                        mcdm_paddingType in_padType);

    virtual mcdm_status_t clearCryptoInfo(void);

    void setSasinfo(MH_sasInfo_t* sasinfo) {
        mSasinfo = sasinfo;
    }

private:

    MH_sasInfo_t* mSasinfo;

};  //class
};  //namespace

#endif /* __MS3_CRYPTO_H__ */
