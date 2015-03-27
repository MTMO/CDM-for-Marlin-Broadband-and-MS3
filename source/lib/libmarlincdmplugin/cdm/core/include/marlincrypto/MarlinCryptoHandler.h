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

#ifndef __CRYPTO_CRYPTO_HANDLER_H__
#define __CRYPTO_CRYPTO_HANDLER_H__

#include "CMutex.h"
#include "IMarlinMediaFormat.h"
#include "MarlinCommonTypes.h"
#include "MarlinCrypto.h"
#include "ContentDataParser.h"

#include <string>

namespace marlincdm {

class MarlinCryptoHandler {
public:

    typedef enum _MarlinCryptoType {
        CRYPTO_TYPE_NONE = 0,
        CRYPTO_TYPE_MBB,
        CRYPTO_TYPE_MS3
    } MarlinCryptoType;

    static inline MarlinCryptoHandler& getMarlinCryptoHandler() {
        sMutex.lock();
        MarlinCryptoHandler *instance = cryptoHandler;
        if (instance == 0) {
            instance = new MarlinCryptoHandler();
            cryptoHandler = instance;
        }
        sMutex.unlock();
        return *instance;
    }

    mcdm_status_t setupCryptoHandle(const MarlinCryptoType cryptoType,
                                    const MH_licenseHandle_t& i_licenseHandle,
                                    const mcdm_SessionId_t& i_session_id,
                                    IMarlinMediaFormat* i_mediaFormat);

    mcdm_status_t setupCryptoHandle(const MarlinCryptoType cryptoType,
                                    const MH_licenseHandle_t& i_licenseHandle,
                                    const mcdm_SessionId_t& i_session_id,
                                    const MH_sasInfo_t* i_sasInfo,
                                    IMarlinMediaFormat* i_mediaFormat);

    void removeCrypto(const mcdm_SessionId_t& session_id,
                      IMarlinMediaFormat* i_mediaFormat);

    MarlinCrypto* getCrypto(const mcdm_SessionId_t& session_id,
                            IMarlinMediaFormat* i_mediaFormat);

private:
    static MarlinCryptoHandler *cryptoHandler;
    static CMutex sMutex;

    MarlinCryptoHandler();
    virtual ~MarlinCryptoHandler();

    bool isExistCryptoFromCrypto(MarlinCrypto* i_marlinCrypto);
};  //class
};  //namespace

#endif /* __CRYPTO_CRYPTO_HANDLER_H__ */
