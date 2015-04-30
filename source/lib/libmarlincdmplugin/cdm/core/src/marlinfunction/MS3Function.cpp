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

#define LOG_TAG "MS3Function"
#include "MarlinLog.h"

#include "MS3Function.h"
#include "MS3InitFormatData.h"
#include "MessageCreator.h"
#include "MarlinLicenseManager.h"
#include "MS3License.h"
#include "ResponseParser.h"
#include "MS3InitFormatData.h"
#include "Cenc.h"

using namespace marlincdm;

MS3Function::MS3Function()
{
    MARLINLOG_ENTER();
    mProcessStatus = STATUS_NONE;
}

MS3Function::MS3Function(const mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    mProcessStatus = STATUS_NONE;
    mSessionId = new string(session_id);
}

MS3Function::~MS3Function()
{
    MARLINLOG_ENTER();
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    if (mSessionId != NULL) {
        if (ch.getCrypto(*mSessionId, mMarlinFormat) != NULL) {
            ch.removeCrypto(*mSessionId, mMarlinFormat);
        }
    }
}

mcdm_status_t MS3Function::initUnit(const string content_data, string* message)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string extract_cid;
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MS3License* ms3License = NULL;
    MarlinCrypto* crypto = NULL;

    ms3License = (MS3License*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MS3,
                                                  mSessionId,
                                                  NULL);
    if (ms3License == NULL) {
        LOGE("could not get sas data");
        status = ERROR_ILLEGAL_STATE;
        goto EXIT;
    }
    if (ms3License->getMs3SessionStatus() != MS3License::MS3_SESSION_STATUS_INITIALIZED) {
        LOGE("MS3 Session is not initialized");
        status = ERROR_ILLEGAL_STATE;
        goto EXIT;
    }

    if (ms3License->getLicenseStatus() != IMarlinLicense::LICENSE_STATUS_INITIALIZED) {
        extract_cid = lm.createMarlinLicenseFromSasInfo(mSessionId,
                                                        ms3License->getSasinfo(),
                                                        ms3License);
        if (extract_cid.empty()) {
            LOGE("Could not initilaize the license session");
            goto EXIT;
        }
    }

    crypto = ch.getCrypto(*mSessionId, mMarlinFormat);
    if (crypto != NULL) {
        if (crypto->getCryptoStatus() == MarlinCrypto::CRYPTO_STATUS_DECRYPTION_INITIALIZED) {
            status = ERROR_ALREADY_RIGHTS_SET;
            goto EXIT;
        }
    }

    if (mMarlinFormat != NULL) {
        delete mMarlinFormat;
    }
    mMarlinFormat = new Cenc(extract_cid);
    status = ch.setupCryptoHandle(MarlinCryptoHandler::CRYPTO_TYPE_MS3,
                                  ms3License->getLicenseHandle(),
                                  *mSessionId,
                                  ms3License->getSasinfo(),
                                  mMarlinFormat);
    if (status != OK) {
        LOGE("Could not setup crypto handle");
        goto EXIT;
    }

EXIT:
    if (message != NULL) {
        if (!createDefaultMessage(0, message)) {
            LOGE("Could not create message.");
            goto EXIT;
        }
    }
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Function::closeUnit(string* /*message*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    if (mSessionId != NULL) {
        if (ch.getCrypto(*mSessionId, mMarlinFormat) != NULL) {
            ch.removeCrypto(*mSessionId, mMarlinFormat);
        } else {
            LOGD("Crypto cannot be found");
            status = ERROR_CANNOT_FIND_HANDLE;
        }
    } else {
        LOGD("Sesssion is not opened");
        status = ERROR_SESSION_NOT_OPENED;
    }
    MARLINLOG_ENTER();
    return status;
}

mcdm_status_t MS3Function::processServiceToken(const string* mimeType,
                                               const mcdm_key_type* keyType,
                                               const string sas_data,
                                               map<string, string>* optionalParameters,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MS3License* ms3License = (MS3License*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MS3,
                                                              mSessionId,
                                                              NULL);
    mcdm_data_t suri = {0, NULL};
    MS3InitFormatData ms3_init_data;

    if (keyType == NULL || mimeType == NULL) {
        LOGE("Key Type or Mime Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    parse_result = ms3_init_data.parse(sas_data);
    if (parse_result < 0) {
        LOGE("Could not parse MS3 Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    suri = ms3_init_data.getSuri();
    if (suri.len <= 0 || suri.data == NULL) {
        LOGE("SURI is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    status = ms3License->initializeMS3Session();
    if (status != OK) {
        goto EXIT;
    }

    status = ms3License->processServiceToken(suri,
                                             optionalParameters,
                                             request,
                                             url);
    if (status != OK) {
        goto EXIT;
    }
    mProcessStatus = STATUS_PROCESSING;

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Function::processServiceToken(const mcdm_key_type* /*keyType*/,
                                               const string /*atkn_data*/,
                                               map<string, string>* /*optionalParameters*/,
                                               string* /*request*/,
                                               string* /*url*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Function::processServiceToken(const string* /*mimeType*/,
                                               const mcdm_key_type* /*keyType*/,
                                               const string /*atkn_data*/,
                                               string* /*request*/,
                                               string* /*url*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Function::processServiceToken(const mcdm_key_type* keyType,
                                               const string pssh,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MS3License* ms3License = NULL;
    string property;

    if (keyType == NULL) {
        LOGE("Key Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    ms3License = (MS3License*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MS3,
                                                  mSessionId,
                                                  NULL);
    if (ms3License == NULL) {
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    ms3License->setIsEme(true);
    status = ms3License->initializeMS3Session();
    if (status != OK) {
        goto EXIT;
    }
    status = ms3License->getDeviceProperty(property);
    if (status != OK) {
        goto EXIT;
    }
    if (!createPropertyResponse(property, request)) {
        status = ERROR_ILLEGAL_STATE;
        goto EXIT;
    }
    mProcessStatus = STATUS_STARTING;

EXIT:
    MARLINLOG_EXIT();
    if (isEme()) {
        if (status != OK) {
            createCompleteMessage(SAS_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  0,
                                  request);
        }
    }
    return status;
}

mcdm_status_t MS3Function::processServiceToken(const string /*atkn*/,
                                               string* /*request*/,
                                               string* /*url*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MS3Function::processRightsResponse(const string& response,
                                                 string* request)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = ERROR_UNKNOWN;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MS3License* ms3License = (MS3License*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MS3,
                                                              mSessionId,
                                                              NULL);

    if (mProcessStatus == STATUS_STARTING) {
        // get suri
        string url;
        mcdm_data_t suri = {0, NULL};

        suri.data = reinterpret_cast<uint8_t*>(const_cast<char*>(response.data()));
        suri.len = response.length();

        status = ms3License->processServiceToken(suri,
                                                 NULL,
                                                 request,
                                                 &url);
        if (status != OK) {
            goto EXIT;
        }
        mProcessStatus = STATUS_PROCESSING;

    } else if (mProcessStatus == STATUS_PROCESSING) {
        ResponseParser response_parser;
        string content_id = "";
        IMarlinMediaFormat* content = NULL;

        string o_message("");
        uint32_t remaining_steps = 0;
        bool is_err = false;
        bool is_license_exist = false;
        request->clear();

        if (!isEme()) {
            parse_result = response_parser.parse(response);
        } else {
            parse_result = response_parser.parseForEME(response);
        }
        if (parse_result < 0) {
            LOGE("Could not parse response Data %d", parse_result);
            status = ERROR_ILLEGAL_ARGUMENT;
            goto EXIT;
        }

        if (response_parser.getStatusCode() != STATUS_CODE_OK) {
            is_err = true;
        }
        if (response_parser.getResponse().data == NULL || response_parser.getResponse().len == 0) {
            LOGE("response is nothing.");
        }
        status = ms3License->processRightsResponse(response_parser.getResponse(),
                                                   is_err,
                                                   remaining_steps,
                                                   &o_message,
                                                   NULL,
                                                   NULL,
                                                   is_license_exist);
        if (status != OK) {
            goto EXIT;
        }

        status = initUnit("", &o_message);
        if (status != OK) {
            LOGE("Could not create decrypt unit");
            goto EXIT;
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (isEme() && request->empty()) {
        uint32_t result_status = SUCCESS;

        if (status != OK) {
            result_status = FAIL;
        }
        createCompleteMessage(SAS_TYPE_ATKN_PROCESS,
                              status,
                              result_status,
                              0,
                              request);
    }
    return status;
}

