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

#define LOG_TAG "IMarlinFunction"
#include "MarlinLog.h"

#include <string.h>
#include <vector>

#include "IMarlinFunction.h"
#include "MarlinConstants.h"
#include "MarlinLicenseManager.h"
#include "PropInitFormatData.h"
#include "IpmpInitFormatData.h"
#include "CencInitFormatData.h"
#include "Ipmp.h"
#include "Cenc.h"

using namespace marlincdm;

IMarlinFunction::IMarlinFunction()
    : mSessionId(NULL),
      mMarlinFormat(NULL),
      mProcessStatus(STATUS_NONE),
      mProcessKeyType(KEY_TYPE_OFFLINE),
      isEME(false)
{
    MARLINLOG_ENTER();
    mLicensePath.clear();
}

IMarlinFunction::IMarlinFunction(const mcdm_SessionId_t& session_id)
    : mSessionId(NULL),
      mMarlinFormat(NULL),
      mProcessStatus(STATUS_NONE),
      mProcessKeyType(KEY_TYPE_OFFLINE),
      isEME(false)
{
    MARLINLOG_ENTER();
    mSessionId = new string(session_id);
    mLicensePath.clear();
}

IMarlinFunction::~IMarlinFunction()
{
    MARLINLOG_ENTER();
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    lm.removeMarlinLicense(mSessionId,
                           mMarlinFormat,
                           isEme());
    if (mSessionId != NULL) {
        delete mSessionId;
        mSessionId = NULL;
    }
    if (mMarlinFormat != NULL) {
        delete mMarlinFormat;
        mMarlinFormat = NULL;
    }
}

mcdm_status_t IMarlinFunction::initUnit(const string /*content_data*/, string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::closeUnit(string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::removeKeys(const string& /*content_data*/, string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::removeKeys(string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::restoreKeys(const string& /*content_data*/, string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::queryKeyStatus(map<string, string>& /*infoMap*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::queryKeyStatus(const string& /*name*/, string& /*value*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t IMarlinFunction::decrypt(bool& is_encrypted,
                                       bool& is_secure,
                                       const uint8_t []/*key[16]*/,
                                       const uint8_t iv[16],
                                       mcdm_decryptMode& /*mode*/,
                                       mcdm_buffer_t* srcPtr,
                                       const mcdm_subsample_t* subSamples,
                                       size_t& numSubSamples,
                                       mcdm_buffer_t* dstPtr,
                                       size_t& decrypt_buffer_length,
                                       string& errorDetailMsg)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    MarlinCrypto* crypto = NULL;
    size_t numWholeEncryptedData = 0;
    size_t numWholeClearData = 0;
    uint8_t *wholeEncryptedData = NULL;
    uint8_t *wholeDecryptedData = NULL;
    mcdm_data_t in_iv = {0, NULL};
    size_t out_len = 0;

    if (mSessionId == NULL) {
        LOGE("session not opened");
        status = ERROR_SESSION_NOT_OPENED;
        goto EXIT;
    }
    if (srcPtr == NULL || dstPtr == NULL) {
        LOGE("input buffer is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (subSamples == NULL || numSubSamples == 0) {
        LOGE("SubSamples is required, but no SubSamples was set.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    crypto = (MarlinCrypto*)ch.getCrypto(*mSessionId,
                                         mMarlinFormat);
    if (crypto == NULL) {
        LOGE("Can not find handle");
        status = ERROR_CANNOT_FIND_HANDLE;
        goto EXIT;
    }
    if (crypto->getCryptoStatus() == MarlinCrypto::CRYPTO_STATUS_UNINITIALIZED) {
        LOGE("Marlin Crypto not initialized");
        status = ERROR_ILLEGAL_STATE;
        goto EXIT;
    }

    for (size_t i = 0; i < numSubSamples; i++) {
        numWholeClearData += subSamples[i].mNumBytesOfClearData;
        numWholeEncryptedData += subSamples[i].mNumBytesOfEncryptedData;
    }
    if (numWholeClearData == 0 && numWholeEncryptedData == 0) {
        // EOF
        status = OK;
        goto EXIT;
    }

    if (is_secure) {
        mcdm_buffer_t in_buff = {0, NULL, 0};
        mcdm_buffer_t out_buff = {0, NULL, 0};
        in_buff.fd = srcPtr->fd;
        in_buff.len = numWholeEncryptedData + numWholeClearData;
        in_buff.data = srcPtr->data;
        in_iv.data = (uint8_t*)iv;
        in_iv.len = 16;
        out_buff.fd = dstPtr->fd;
        out_buff.data = dstPtr->data;
        out_buff.len = numWholeEncryptedData + numWholeClearData;
        out_len = 0;

        status = crypto->decryptWithHW(is_encrypted,
                                       iv,
                                       &in_buff,
                                       subSamples,
                                       numSubSamples,
                                       &out_buff,
                                       out_len,
                                       errorDetailMsg);
        if (status != OK) {
            LOGE("decryption failed: %d", status);
            goto EXIT;
        }

        if (out_len == 0) {
            LOGV("decryption successfully, but the length of result is zero");
            goto EXIT;
        }

        LOGV("decryption successfully: out_len=%zd", (ssize_t) out_len);
        decrypt_buffer_length = out_len;
        dstPtr->len = out_len;

    } else {
        if (numWholeClearData > 0 && numWholeEncryptedData == 0) {
            // only clear data
            memcpy(dstPtr->data, (const uint8_t *) srcPtr->data, numWholeClearData);
            dstPtr->len = (uint32_t)numWholeClearData;
            decrypt_buffer_length = numWholeClearData;
            status = OK;
            goto EXIT;
        }

        mcdm_buffer_t in_buff = {0, NULL, 0};
        mcdm_buffer_t out_buff = {0, NULL, 0};

        wholeEncryptedData = new uint8_t[numWholeEncryptedData];
        wholeDecryptedData = new uint8_t[numWholeEncryptedData];

        if (numWholeClearData > 0) {
            size_t target_offset = 0;
            size_t read_offset = 0;
            for (size_t i = 0; i < numSubSamples; i++) {
                const mcdm_subsample_t &subsample = subSamples[i];
                memcpy(wholeEncryptedData + target_offset,
                       (const uint8_t *) srcPtr->data + read_offset + subsample.mNumBytesOfClearData,
                       subsample.mNumBytesOfEncryptedData);
                target_offset += subsample.mNumBytesOfEncryptedData;
                read_offset += subsample.mNumBytesOfClearData
                               + subsample.mNumBytesOfEncryptedData;
            }
        } else {
            memcpy(wholeEncryptedData, (const uint8_t *) srcPtr->data, numWholeEncryptedData);
        }

        in_buff.data = (uint8_t*)wholeEncryptedData;
        in_buff.len = numWholeEncryptedData;
        in_buff.fd = srcPtr->fd;
        in_iv.data = (uint8_t*)iv;
        in_iv.len = 16;
        out_buff.data = (uint8_t*)wholeDecryptedData;
        out_buff.len = (uint32_t)numWholeEncryptedData;
        out_buff.fd = dstPtr->fd;
        out_len = 0;
        LOGV("decrypt param: in_buff.len=%u in_iv.len=%u out_buff.len=%u",
                in_buff.len, in_iv.len, out_buff.len);

        status = crypto->decrypt(is_encrypted,
                                 iv,
                                 &in_buff,
                                 subSamples,
                                 numSubSamples,
                                 &out_buff,
                                 out_len,
                                 errorDetailMsg);

        if (status != OK) {
            LOGE("decryption failed: %d", status);
            goto EXIT;
        }

        if (out_len == 0) {
            LOGV("decryption successfully, but the length of result is zero");
            goto EXIT;
        }

        LOGV("decryption successfully: out_len=%zd", (ssize_t) out_len);
        if (numWholeClearData > 0) {
            size_t read_offset = 0;
            for (size_t i = 0; i < numSubSamples; i++) {
                const mcdm_subsample_t &subsample = subSamples[i];
                memcpy((uint8_t *) dstPtr->data + decrypt_buffer_length,
                       (const uint8_t *) srcPtr->data + decrypt_buffer_length,
                       subsample.mNumBytesOfClearData);
                decrypt_buffer_length += subsample.mNumBytesOfClearData;

                memcpy((uint8_t *) dstPtr->data + decrypt_buffer_length,
                       wholeDecryptedData + read_offset,
                       subsample.mNumBytesOfEncryptedData);
                decrypt_buffer_length += subsample.mNumBytesOfEncryptedData;
                read_offset += subsample.mNumBytesOfEncryptedData;
            }
            dstPtr->len = decrypt_buffer_length;
        } else {
            memcpy((uint8_t *) dstPtr->data, wholeDecryptedData, out_len);
            decrypt_buffer_length = out_len;
            dstPtr->len = out_len;
        }
    }
EXIT:
    MARLINLOG_EXIT();
    if (wholeEncryptedData != NULL) {
        delete[] wholeEncryptedData;
        wholeEncryptedData = NULL;
    }
    if (wholeDecryptedData != NULL) {
        delete[] wholeDecryptedData;
        wholeDecryptedData = NULL;
    }
    if (status != OK) {
        string err_message;
        convertDecryptErrorToMessage(status, err_message);
        errorDetailMsg.assign(err_message);
    }
    return status;
}

mcdm_status_t IMarlinFunction::getPropertyString(const string& name, string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinLicense *marlinlicense = NULL;
    marlinlicense = new MarlinLicense();

    if (marlinlicense == NULL) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    status = marlinlicense->getProperty(name, value);

EXIT:
    MARLINLOG_EXIT();
    if (marlinlicense != NULL) {
        delete marlinlicense;
        marlinlicense = NULL;
    }
    return status;
}

mcdm_status_t IMarlinFunction::getPropertyFormatString(const string& name, string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinLicense *marlinlicense = NULL;
    marlinlicense = new MarlinLicense();

    if (marlinlicense == NULL) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    status = marlinlicense->getPropertyFormatString(name, value);

EXIT:
    MARLINLOG_EXIT();
    if (marlinlicense != NULL) {
        delete marlinlicense;
        marlinlicense = NULL;
    }
    return status;
}

mcdm_status_t IMarlinFunction::isTypeSupported(const string& key_system,
                                               const string& init_data_type,
                                               const string& content_type,
                                               const string& capability,
                                               string* isTypeSupportedResult)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicense *marlinlicense = NULL;
    marlinlicense = new MarlinLicense();
    if (marlinlicense == NULL) {
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    status = marlinlicense->isTypeSupported(key_system, init_data_type, content_type, capability, isTypeSupportedResult);
EXIT:
    MARLINLOG_EXIT();
    if (marlinlicense != NULL) {
        delete marlinlicense;
        marlinlicense = NULL;
    }
    return status;
}

IMarlinMediaFormat* IMarlinFunction::getMarlinMediaFormat(const string content_data)
{
    MARLINLOG_ENTER();
    IMarlinMediaFormat* content = NULL;

    if (mMarlinFormat != NULL) {
        delete mMarlinFormat;
        mMarlinFormat = NULL;
    }

    if (!isEme()) {
        CommonHeaderParser common_header_parser;
        parse_status_t parse_result = INIT_DATA_PARSE_OK;
        parse_result = common_header_parser.parse(content_data);
        if (parse_result != INIT_DATA_PARSE_OK) {
            LOGE("Could not parse Init Data %d", parse_result);
            goto EXIT;
        }
        if (!common_header_parser.getDataTypeFlag().compare(DATA_TYPE_IPMP)) {
            // ipmp content
            content = getIPMPFormat(content_data);
        } else if (!common_header_parser.getDataTypeFlag().compare(DATA_TYPE_CENC)) {
            // cenc content
            content = getCENCFormat(content_data);
        }
    } else {
        content = getCENCFormat(content_data);
    }

    if (content != NULL) {
        mMarlinFormat = content;
    }
EXIT:
    MARLINLOG_EXIT();
    return content;
}

IMarlinMediaFormat* IMarlinFunction::getIPMPFormat(const string ipmp_data)
{
    MARLINLOG_ENTER();
    IpmpInitFormatData ipmp_format_data;
    Ipmp* content = NULL;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    parse_result = ipmp_format_data.parse(ipmp_data);
    if (parse_result != INIT_DATA_PARSE_OK) {
        LOGE("Could not parse Init Data: %d", parse_result);
        goto EXIT;
    }
    content = new Ipmp(ipmp_format_data.getSinf().data,
                       ipmp_format_data.getSinf().len,
                       ipmp_format_data.getContentId());

EXIT:
    MARLINLOG_EXIT();
    return content;
}

IMarlinMediaFormat* IMarlinFunction::getCENCFormat(const string cenc_data)
{
    MARLINLOG_ENTER();
    Cenc* content = NULL;

    if (!isEme()) {
        CencInitFormatData cenc_format_data;
        parse_status_t parse_result = INIT_DATA_PARSE_OK;
        parse_result = cenc_format_data.parse(cenc_data);
        if (parse_result != INIT_DATA_PARSE_OK) {
            LOGE("Could not parse Init Data: %d", parse_result);
            goto EXIT;
        }

        if (!cenc_format_data.getKidList().empty()) {
            content = new Cenc(cenc_format_data.getPssh().data,
                               cenc_format_data.getPssh().len,
                               cenc_format_data.getKidList(),
                               cenc_format_data.getContentId());
        } else {
            content = new Cenc(cenc_format_data.getPssh().data,
                               cenc_format_data.getPssh().len,
                               cenc_format_data.getContentId());
        }
    } else {
        CencInitFormatData cenc_format_data;
        uint8_t* pssh = NULL;

        if (cenc_data.c_str() == NULL || cenc_data.length() == 0) {
            LOGD("pssh is NULL");
            goto EXIT;
        }

        if (cenc_format_data.parseKeyIds(cenc_data) == INIT_DATA_PARSE_OK) {
            content = new Cenc(cenc_format_data.getContentId());
        } else {
            pssh = reinterpret_cast<uint8_t*>(const_cast<char*>(cenc_data.c_str()));
            content = new Cenc(pssh,
                               cenc_data.length(),
                               "");
        }
    }

EXIT:
    MARLINLOG_EXIT();
    return content;
}
