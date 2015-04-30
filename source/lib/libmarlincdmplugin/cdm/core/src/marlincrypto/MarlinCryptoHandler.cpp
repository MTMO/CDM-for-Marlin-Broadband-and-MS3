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

#define LOG_TAG "MarlinCryptoHandler"
#include "MarlinLog.h"

#include "ContentDataParser.h"
#include "MarlinCryptoHandler.h"
#include "MBBCrypto.h"
#include "MS3Crypto.h"

using namespace marlincdm;

MarlinCryptoHandler *MarlinCryptoHandler::cryptoHandler = 0;
CMutex MarlinCryptoHandler::sMutex;

static map<string, MarlinCrypto*> mMarlinCryptoMapByCID;
static map<mcdm_SessionId_t, MarlinCrypto*> mMarlinCryptoMapBySessionId;
static CMutex sMapMutex;

MarlinCryptoHandler::MarlinCryptoHandler() {
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
}

MarlinCryptoHandler::~MarlinCryptoHandler()
{
    MARLINLOG_ENTER();
    MARLINLOG_EXIT();
}

mcdm_status_t MarlinCryptoHandler::setupCryptoHandle(MarlinCryptoType cryptoType,
                                                     const MH_licenseHandle_t& i_licenseHandle,
                                                     const mcdm_SessionId_t& i_session_id,
                                                     IMarlinMediaFormat* i_mediaFormat)
{
    MARLINLOG_ENTER();
    ContentDataParser* parser = NULL;
    u_int32_t format;
    mcdm_decryptMode crypto_type;
    mcdm_algorithmType algo_type;
    mcdm_paddingType pad_type;
    mcdm_status_t status = ERROR_UNKNOWN;
    string current_cid;
    mcdm_data_t setup_cid;

    if (i_mediaFormat == NULL) {
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    status = i_mediaFormat->getCid(NULL, current_cid);
    if (status != OK) {
        LOGE("Couldn't find CID from KID");
        status = ERROR_NO_CID;
        goto EXIT;
    }

    sMapMutex.lock();
    if (mMarlinCryptoMapByCID.count(current_cid) > 0) {
        LOGD("CryptoHandle was created already");
        if (mMarlinCryptoMapBySessionId.count(i_session_id) == 0) {
            mMarlinCryptoMapBySessionId[i_session_id] = mMarlinCryptoMapByCID[current_cid];
        }
        status = OK;
        goto EXIT;
    }

    format = i_mediaFormat->getMediaFormat();
    crypto_type  = i_mediaFormat->getDecryptMode();

    if (crypto_type == MODE_UNENCRYPTED
            || format == IMarlinMediaFormat::FORMAT_UNKNOWN) {
        status = ERROR_UNSUPPORTED;
        goto EXIT;
    }

    if (crypto_type == MODE_AES_CTR) {
        algo_type = CRYPTO_KFCBC_ALGO_AES;
        pad_type = CRYPTO_KFCBC_PAD_NONE;
    } else if (crypto_type == MODE_AES_CBC) {
        algo_type = CRYPTO_KFCBC_ALGO_AES;
        pad_type = CRYPTO_KFCBC_PAD_RFC2630;
    } else {
        LOGE("Invalid Mode");
        status = ERROR_UNSUPPORTED;
        goto EXIT;
    }

    parser = i_mediaFormat->getDataParser();
    if (parser->getData().data == NULL || parser->getData().len == 0) {
        LOGE("Couldn't find content metadata");
        status = ERROR_NO_CID;
        goto EXIT;
    }

    if (parser->getDataMap().size() > 0) {
        // Android case
        for (size_t i = 0; i < parser->getDataMap().size(); i++) {
            MarlinCrypto* crypto;
            string extract_cid;
            bool isLast = false;
            status = i_mediaFormat->getCidWithIndex(i, extract_cid);
            if (status != OK) {
                LOGE("Couldn't find CID from index");
                status = ERROR_NO_CID;
                goto EXIT;
            }

            crypto = new MarlinCrypto(parser->getData(i));
            crypto->clearCryptoInfo();

            setup_cid.len = extract_cid.length();
            setup_cid.data = (uint8_t*)extract_cid.data();
            if ((i + 1) == parser->getDataMap().size()) {
                isLast = true;
            }
            crypto->setLicenseHandle(i_licenseHandle);
            status = crypto->setCryptoInfo(setup_cid,
                                           isLast,
                                           crypto_type,
                                           algo_type,
                                           pad_type);
            if (status != OK) {
                LOGE("Couldn't setup for decryption: %d", status);
                crypto->clearCryptoInfo();
                delete crypto;
                crypto = NULL;
                status = ERROR_UNKNOWN;
                goto EXIT;
            } else {
                if (mMarlinCryptoMapByCID.count(extract_cid) > 0) {
                    MarlinCrypto* tmpCrypto = mMarlinCryptoMapByCID[extract_cid];
                    if (tmpCrypto != NULL) {
                        tmpCrypto->clearCryptoInfo();
                        delete tmpCrypto;
                        tmpCrypto = NULL;
                    }
                    mMarlinCryptoMapByCID.erase(extract_cid);
                }
                mMarlinCryptoMapByCID[extract_cid] = crypto;

                if (!extract_cid.compare(current_cid)) {
                    mMarlinCryptoMapBySessionId[i_session_id] = crypto;
                }
            }
        }
    } else {
        MarlinCrypto* crypto;
        bool isLast = true;

        crypto = new MarlinCrypto(parser->getData());
        crypto->clearCryptoInfo();

        setup_cid.len = current_cid.length();
        setup_cid.data = (uint8_t*)current_cid.data();
        crypto->setLicenseHandle(i_licenseHandle);
        status = crypto->setCryptoInfo(setup_cid,
                                       isLast,
                                       crypto_type,
                                       algo_type,
                                       pad_type);
        if (status != OK) {
            LOGE("Couldn't setup for decryption: %d", status);
            crypto->clearCryptoInfo();
            delete crypto;
            crypto = NULL;
            status = ERROR_UNKNOWN;
            goto EXIT;
        } else {
            if (mMarlinCryptoMapByCID.count(current_cid) > 0) {
                MarlinCrypto* tmpCrypto = mMarlinCryptoMapByCID[current_cid];
                if (tmpCrypto != NULL) {
                    tmpCrypto->clearCryptoInfo();
                    delete tmpCrypto;
                    tmpCrypto = NULL;
                }
                mMarlinCryptoMapByCID.erase(current_cid);
            }
            mMarlinCryptoMapByCID[current_cid] = crypto;
            mMarlinCryptoMapBySessionId[i_session_id] = crypto;
        }
    }
    LOGV("Completed a setup for decryption");
    status = OK;
EXIT:
    MARLINLOG_EXIT();
    sMapMutex.unlock();
    return status;
}

mcdm_status_t MarlinCryptoHandler::setupCryptoHandle(const MarlinCryptoType cryptoType,
                                                     const MH_licenseHandle_t& i_licenseHandle,
                                                     const mcdm_SessionId_t& i_session_id,
                                                     const MH_sasInfo_t* i_sasInfo,
                                                     IMarlinMediaFormat* i_mediaFormat)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;

    u_int32_t format;
    mcdm_decryptMode crypto_type;
    mcdm_algorithmType algo_type;
    mcdm_paddingType pad_type;
    string current_cid;
    mcdm_data_t setup_cid;

    if (i_sasInfo == NULL) {
        LOGE("Couldn't find CID from SAS Info");
        status = ERROR_NO_CID;
        goto EXIT;
    }

    if (i_mediaFormat != NULL) {
        status = i_mediaFormat->getCid(NULL, current_cid);
        if (status != OK) {
            LOGE("Couldn't find CID from KID");
            status = ERROR_NO_CID;
            goto EXIT;
        }

        if (mMarlinCryptoMapByCID.count(current_cid) > 0) {
            LOGD("CryptoHandle was created already");
            status = OK;
            goto EXIT;
        }

        format = i_mediaFormat->getMediaFormat();
        crypto_type  = i_mediaFormat->getDecryptMode();

        if (crypto_type != MODE_AES_CTR
                || format == IMarlinMediaFormat::FORMAT_UNKNOWN) {
            status = ERROR_UNSUPPORTED;
            goto EXIT;
        }
    }

    algo_type = CRYPTO_KFCBC_ALGO_AES;
    pad_type = CRYPTO_KFCBC_PAD_NONE;

    for (uint32_t i = 0; i < i_sasInfo->numcontentid; i++) {
        MarlinCrypto* crypto;
        MH_string_t* contentid = i_sasInfo->content_ids[i];
        string extract_cid = string(reinterpret_cast<char*>(contentid->bufstr), contentid->length);

        if (mMarlinCryptoMapByCID.count(current_cid) == 0) {
            crypto = new MS3Crypto();
            crypto->clearCryptoInfo();

            setup_cid.len = contentid->length;
            setup_cid.data = contentid->bufstr;
            crypto->setLicenseHandle(i_licenseHandle);
            status = crypto->setCryptoInfo(setup_cid,
                                           false,
                                           crypto_type,
                                           algo_type,
                                           pad_type);
            if (status != OK) {
                LOGE("Couldn't setup for decryption: %d", status);
                crypto->clearCryptoInfo();
                delete crypto;
                crypto = NULL;
                status = ERROR_UNKNOWN;
                goto EXIT;
            } else {
                if (mMarlinCryptoMapByCID.count(current_cid) > 0) {
                    MarlinCrypto* tmpCrypto = mMarlinCryptoMapByCID[current_cid];
                    if (tmpCrypto != NULL) {
                        tmpCrypto->clearCryptoInfo();
                        delete tmpCrypto;
                        tmpCrypto = NULL;
                    }
                    mMarlinCryptoMapByCID.erase(current_cid);
                }

                mMarlinCryptoMapByCID[current_cid] = crypto;
                if (!extract_cid.compare(current_cid)) {
                    mMarlinCryptoMapBySessionId[i_session_id] = crypto;
                }
            }
        }
    }

    LOGV("Completed a setup for decryption");
    status = OK;
EXIT:
    MARLINLOG_EXIT();
    return status;
}

void MarlinCryptoHandler::removeCrypto(const mcdm_SessionId_t& i_session_id,
                                       IMarlinMediaFormat* i_mediaFormat)
{
    MARLINLOG_ENTER();
    ContentDataParser* parser = NULL;
    mcdm_status_t status = OK;
    string current_cid;

    if (i_mediaFormat == NULL) {
        return;
    }

    sMapMutex.lock();
    parser = i_mediaFormat->getDataParser();
    for (size_t i = 0; i < parser->getDataMap().size(); i++) {
        string extract_cid;
        status = i_mediaFormat->getCidWithIndex(i, extract_cid);
        if (status != OK) {
            LOGE("Couldn't find CID from index");
            break;
        }

        if (mMarlinCryptoMapByCID.count(extract_cid) > 0) {
            MarlinCrypto* crypto = mMarlinCryptoMapByCID[extract_cid];

            mMarlinCryptoMapBySessionId.erase(i_session_id);
            if (crypto != NULL) {
                if (!isExistCryptoFromCrypto(crypto)) {
                    crypto->clearCryptoInfo();
                    delete crypto;
                    crypto = NULL;
                    mMarlinCryptoMapByCID.erase(extract_cid);
                }
            }
        }
    }
    sMapMutex.unlock();

    MARLINLOG_EXIT();
}

MarlinCrypto* MarlinCryptoHandler::getCrypto(const mcdm_SessionId_t& i_session_id,
                                             IMarlinMediaFormat* i_mediaFormat)
{
    MARLINLOG_ENTER();
    MarlinCrypto* crypto = NULL;
    mcdm_status_t status = OK;
    string current_cid;

    if (i_mediaFormat == NULL) {
        goto EXIT;
    }

    status = i_mediaFormat->getCid(NULL, current_cid);
    if (status != OK) {
        LOGE("Couldn't find CID from KID");
        goto EXIT;
    }

    sMapMutex.lock();
    if (mMarlinCryptoMapByCID.count(current_cid) > 0) {
        MarlinCrypto* cryptoHandle;
        cryptoHandle = mMarlinCryptoMapByCID[current_cid];

        if (mMarlinCryptoMapBySessionId.count(i_session_id) == 0) {
            mMarlinCryptoMapBySessionId[i_session_id] = cryptoHandle;
        } else if (cryptoHandle != mMarlinCryptoMapBySessionId[i_session_id]) {
            MarlinCrypto* tmpCrypto = mMarlinCryptoMapBySessionId[i_session_id];
            if (tmpCrypto != NULL) {
                tmpCrypto->clearCryptoInfo();
                delete tmpCrypto;
                tmpCrypto = NULL;
            }
            mMarlinCryptoMapBySessionId[i_session_id] = cryptoHandle;
        }
        crypto = cryptoHandle;
    }
    sMapMutex.unlock();
EXIT:
    MARLINLOG_EXIT();
    return crypto;
}

bool MarlinCryptoHandler::isExistCryptoFromCrypto(MarlinCrypto* i_marlinCrypto)
{
    bool ret = false;
    map<mcdm_SessionId_t, MarlinCrypto*>::const_iterator itr;
    itr = mMarlinCryptoMapBySessionId.begin();
    while(itr != mMarlinCryptoMapBySessionId.end()) {
        if(i_marlinCrypto == itr->second) {
            ret = true;
            break;
        }
        itr++;
    }
    return ret;
}
