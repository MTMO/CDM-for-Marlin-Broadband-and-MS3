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

#define LOG_TAG "MBBFunction"
#include "MarlinLog.h"

#include <string.h>
#include <vector>

#include "MBBFunction.h"
#include "MarlinConstants.h"
#include "MessageCreator.h"
#include "MarlinLicenseManager.h"
#include "MBBLicense.h"
#include "ResponseParser.h"
#include "AtknInitFormatData.h"
#include "LTPInitFormatData.h"

using namespace marlincdm;

const uint32_t USB_MESSAGE_HELLO_REQUEST = 0x00000106;

MBBFunction::MBBFunction()
{
    MARLINLOG_ENTER();
    mProcessStatus = STATUS_NONE;
}

MBBFunction::MBBFunction(const mcdm_SessionId_t& session_id)
{
    MARLINLOG_ENTER();
    mProcessStatus = STATUS_NONE;
    mSessionId = new string(session_id);
}

MBBFunction::~MBBFunction()
{
    MARLINLOG_ENTER();
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    if (mSessionId != NULL) {
        if (ch.getCrypto(*mSessionId, mMarlinFormat) != NULL) {
            ch.removeCrypto(*mSessionId, mMarlinFormat);
        }
    }
}

mcdm_status_t MBBFunction::initUnit(const string content_data, string* message)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    IMarlinMediaFormat* content = NULL;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string extract_cid;
    MarlinCryptoHandler& ch(MarlinCryptoHandler::getMarlinCryptoHandler());
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = NULL;

    if (mMarlinFormat != NULL) {
        mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                      mSessionId,
                                                      mMarlinFormat);
        if (mbbLicense != NULL && ch.getCrypto(*mSessionId, mMarlinFormat) != NULL) {
            if (mbbLicense->getLicenseStatus() == IMarlinLicense::LICENSE_STATUS_INITIALIZED
                    && ch.getCrypto(*mSessionId, mMarlinFormat)->getCryptoStatus() == MarlinCrypto::CRYPTO_STATUS_DECRYPTION_INITIALIZED) {
                LOGV("Already rights set");
                status = ERROR_ALREADY_RIGHTS_SET;
                goto EXIT;
            }
        }
        content = mMarlinFormat;
    } else {
        content = getMarlinMediaFormat(content_data);
    }

    if (content == NULL) {
        LOGE("Content Data is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mediaFormat = content->getMediaFormat();
    if ((mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN)
            || (mediaFormat != IMarlinMediaFormat::FORMAT_CENC
                    && mediaFormat != IMarlinMediaFormat::FORMAT_IPMP)) {
        LOGE("Content Format is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (mbbLicense == NULL) {
        mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                      mSessionId,
                                                      mMarlinFormat);
        if (mbbLicense == NULL) {
            LOGE("Could not initialize license");
            status = ERROR_NO_LICENSE;
            goto EXIT;
        }
    }

    status = mbbLicense->consumeLicense();
    if (status != OK) {
        LOGE("Could not evaluate license");
        goto EXIT;
    }

    status = ch.setupCryptoHandle(MarlinCryptoHandler::CRYPTO_TYPE_MBB,
                                  mbbLicense->getLicenseHandle(),
                                  *mSessionId,
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

mcdm_status_t MBBFunction::closeUnit(string* /*message*/)
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

mcdm_status_t MBBFunction::processServiceToken(const string* mimeType,
                                               const mcdm_key_type* keyType,
                                               const string atkn_data,
                                               map<string, string>* optionalParameters,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;
    mLicensePath.clear();

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);
    mcdm_data_t matd = {0, NULL};
    AtknInitFormatData atkn_init_data;

    if (keyType == NULL || mimeType == NULL) {
        LOGE("Key Type or Mime Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    parse_result = atkn_init_data.parse(atkn_data);
    if (parse_result < 0) {
        LOGE("Could not parse ATKN Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    matd = atkn_init_data.getMatd();
    if (matd.len <= 0 || matd.data == NULL) {
        LOGE("Service Token is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    status = mbbLicense->initializeBBSession(BB_MODE_TYPE_LOCAL);
    if (status != OK) {
        goto EXIT;
    }

    if (!MIME_TYPE_LICENSE.compare(*mimeType)
            && optionalParameters->count(KEY_OPTIONAL_LICENSE_PATH) > 0) {
        mLicensePath = (*optionalParameters)[KEY_OPTIONAL_LICENSE_PATH];
    }

    if ((mProcessKeyType == KEY_TYPE_OFFLINE
                    && !MIME_TYPE_REGISTRATION.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_RELEASE
                    && !MIME_TYPE_DEREGISTRATION.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_OFFLINE
                    && !MIME_TYPE_LICENSE.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_STREAMING
                    && !MIME_TYPE_LICENSE.compare(*mimeType))) {
            status = mbbLicense->processServiceToken(matd,
                                                     optionalParameters,
                                                     request,
                                                     url);
            if (status != OK) {
                goto EXIT;
            }
            mProcessStatus = STATUS_PROCESSING;
    } else {
        LOGE("Key Type or Mime Type is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBFunction::processServiceToken(const mcdm_key_type* keyType,
                                               const string atkn_data,
                                               map<string, string>* optionalParameters,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;
    mLicensePath.clear();

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);
    mcdm_data_t matd = {0, NULL};
    AtknInitFormatData atkn_init_data;

    if (keyType == NULL) {
        LOGE("Key Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    parse_result = atkn_init_data.parse(atkn_data);
    if (parse_result < 0) {
        LOGE("Could not parse ATKN Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    matd = atkn_init_data.getMatd();
    if (matd.len <= 0 || matd.data == NULL) {
        LOGE("Service Token is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    status = mbbLicense->initializeBBSession(BB_MODE_TYPE_LOCAL);
    if (status != OK) {
        goto EXIT;
    }

    if (optionalParameters->count(KEY_OPTIONAL_LICENSE_PATH) > 0) {
        mLicensePath = (*optionalParameters)[KEY_OPTIONAL_LICENSE_PATH];
    }

    if (mProcessKeyType == KEY_TYPE_OFFLINE
            || mProcessKeyType == KEY_TYPE_RELEASE
            || mProcessKeyType == KEY_TYPE_OFFLINE
            || mProcessKeyType == KEY_TYPE_STREAMING) {
            status = mbbLicense->processServiceToken(matd,
                                                     optionalParameters,
                                                     request,
                                                     url);
            if (status != OK) {
                goto EXIT;
            }
            mProcessStatus = STATUS_PROCESSING;
    } else {
        LOGE("Key Type or Mime Type is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBFunction::processServiceToken(const string* mimeType,
                                               const mcdm_key_type* keyType,
                                               const string atkn_data,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                 mSessionId,
                                                 NULL);
    mcdm_data_t matd = {0, NULL};
    AtknInitFormatData atkn_init_data;
    map<string, string> optionalParameters;

    if (keyType == NULL || mimeType == NULL) {
        LOGE("Key Type or Mime Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    parse_result = atkn_init_data.parse(atkn_data);
    if (parse_result < 0) {
        LOGE("Could not parse ATKN Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    matd = atkn_init_data.getMatd();
    if (matd.len <= 0 || matd.data == NULL) {
        LOGE("Service Token is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    optionalParameters = atkn_init_data.getOptionalParameters();

    status = mbbLicense->initializeBBSession(BB_MODE_TYPE_LOCAL);
    if (status != OK) {
        goto EXIT;
    }

    if (!MIME_TYPE_LICENSE.compare(*mimeType)
            && optionalParameters.count(KEY_OPTIONAL_LICENSE_PATH) > 0) {
        mLicensePath = optionalParameters[KEY_OPTIONAL_LICENSE_PATH];
    }

    if ((mProcessKeyType == KEY_TYPE_OFFLINE
                    && !MIME_TYPE_REGISTRATION.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_RELEASE
                    && !MIME_TYPE_DEREGISTRATION.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_OFFLINE
                    && !MIME_TYPE_LICENSE.compare(*mimeType))
            || (mProcessKeyType == KEY_TYPE_STREAMING
                    && !MIME_TYPE_LICENSE.compare(*mimeType))) {
            status = mbbLicense->processServiceToken(matd,
                                                     &optionalParameters,
                                                     request,
                                                     url);
            if (status != OK) {
                goto EXIT;
            }
            mProcessStatus = STATUS_PROCESSING;
    } else {
        LOGE("Key Type or Mime Type is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBFunction::processServiceToken(const mcdm_key_type* keyType,
                                               const string pssh,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    IMarlinMediaFormat* mediaFormat = NULL;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = NULL;
    string property;

    if (keyType == NULL) {
        LOGE("Key Type is NULL");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    mProcessKeyType = *keyType;

    mediaFormat = getMarlinMediaFormat(pssh);
    mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                  mSessionId,
                                                  NULL);
    if (mbbLicense == NULL) {
        if (mediaFormat == NULL) {
            LOGE("pssh is invalid");
        }
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    mbbLicense->setIsEme(true);
    status = mbbLicense->initializeBBSession(BB_MODE_TYPE_LOCAL);
    if (status != OK) {
        goto EXIT;
    }
    status = mbbLicense->getDeviceProperty(property);
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
            createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  0,
                                  request);
        }
    }
    return status;
}

mcdm_status_t MBBFunction::processServiceToken(const string atkn,
                                               string* request,
                                               string* url)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    mcdm_data_t matd = {0, NULL};
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);

    if (atkn.c_str() == NULL || atkn.length() == 0) {
        LOGE("Service Token is NULL");
        if (isEme()) {
            createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  0,
                                  request);
        }
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    matd.len = atkn.length();
    matd.data = reinterpret_cast<uint8_t*>(const_cast<char*>(atkn.c_str()));

    status = mbbLicense->processServiceToken(matd,
                                             NULL,
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

mcdm_status_t MBBFunction::processRightsResponse(const string& response,
                                                 string* request)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    mcdm_data_t o_license_data = {0, NULL};
    mcdm_data_t o_service_info = {0, NULL};
    MBBLicense* mbbLicense = NULL;

    if (mProcessStatus == STATUS_STARTING) {
        // setup service token
        string url;
        status = processServiceToken(response, request, &url);
        if (status != OK) {
            goto EXIT;
        }
    } else {
        parse_status_t parse_result = INIT_DATA_PARSE_OK;
        ResponseParser response_parser;
        MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
        mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                      mSessionId,
                                                      NULL);

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

        if (!TYPE_APP_ERROR.compare(response_parser.getMassageTypeFlag()) ||
                !TYPE_RESPONSE.compare(response_parser.getMassageTypeFlag())) {
            // HTTP response process
            string o_message("");
            string url("");
            uint32_t remaining_steps = 0;
            bool is_err = false;
            bool is_license_exist = false;

            if ((mProcessStatus != STATUS_STARTING) &&
                    (mProcessStatus != STATUS_PROCESSING)) {
                LOGE("Rights process is not start");
                status = ERROR_ILLEGAL_STATE;
                goto EXIT;
            }
            if (request == NULL) {
                LOGE("io request is NULL");
                status = ERROR_ILLEGAL_ARGUMENT;
                goto EXIT;
            }

            if (response_parser.getStatusCode() != STATUS_CODE_OK) {
                is_err = true;
            }
            status = mbbLicense->processRightsResponse(response_parser.getResponse(),
                                                       is_err,
                                                       remaining_steps,
                                                       &o_message,
                                                       &o_license_data,
                                                       &o_service_info,
                                                       is_license_exist);
            if (status != OK) {
                goto EXIT;
            }

            if (remaining_steps > 0) {
                // generate http request
                status = mbbLicense->processServiceToken(request,
                                                         &url,
                                                         remaining_steps);
            } else if (remaining_steps == 0 && is_license_exist) {

                // store license to DB
                status = mbbLicense->storeLicenseToDataBase(mProcessKeyType,
                                                            mLicensePath,
                                                            o_license_data,
                                                            o_service_info);
                if (status != OK) {
                    mbbLicense->removeLicense(o_license_data);
                    o_message.assign("");
                } else {
                    // write license to file
                    status = mbbLicense->writeLicenseToFile(mProcessKeyType,
                                                            mLicensePath,
                                                            o_license_data);
                    if (status != OK) {
                        mbbLicense->removeLicense(o_license_data);
                        o_message.assign("");
                    }
                }
                mLicensePath.clear();

                // acquire a license done
                mbbLicense->finalizeBBSession();

                if (isEme()) {
                    // setup to decrypt
                    status = initUnit("", NULL);
                    if (status != OK) {
                        if (status != ERROR_ALREADY_RIGHTS_SET) {
                            mbbLicense->removeLicense(o_license_data);
                            o_message.assign("");
                        } else {
                            status = OK;
                        }
                    }
                }

                if (o_message.empty()) {
                    LOGE("Could not acquire license.");
                    status = ERROR_UNKNOWN;
                    goto EXIT;
                }
                *request = string(o_message.data(), o_message.length());
            } else if (remaining_steps == 0) {
                // http process is done
                if (!isEme()) {
                    status = mbbLicense->processServiceToken(request,
                                                             &url,
                                                             remaining_steps);
                    if (status != OK) {
                        request->assign("");
                        goto EXIT;
                    }
                    url.assign("");
                } else {
                    *request = string(o_message.data(), o_message.length());
                }
                mbbLicense->finalizeBBSession();
                mProcessStatus = STATUS_DONE;
            }
        } else if (!TYPE_LICENSE.compare(response_parser.getMassageTypeFlag())) {
            // store license
            mcdm_data_t license = {0, NULL};
            mcdm_data_t accountId = {0, NULL};

            license = response_parser.getLicenseData();
            if (license.data == NULL || license.len == 0) {
                LOGE("License is empty");
                status = ERROR_ILLEGAL_ARGUMENT;
                goto EXIT;
            }
            if (!response_parser.getLicensePath().empty()) {
                mLicensePath = response_parser.getLicensePath();
            }
            status = mbbLicense->storeLicenseToDataBase(mProcessKeyType,
                                                        mLicensePath,
                                                        license,
                                                        accountId);
            if (status != OK) {
                goto EXIT;
            }
            // write license to file
            status = mbbLicense->writeLicenseToFile(mProcessKeyType,
                                                    mLicensePath,
                                                    license);
            if (status != OK) {
                mbbLicense->removeLicense(license);
                goto EXIT;
            }
            mLicensePath.clear();
        }
    }

EXIT:
    MARLINLOG_EXIT();
    if (status != OK) {
        if (isEme()) {
            createCompleteMessage(ACTION_TYPE_ATKN_PROCESS,
                                  status,
                                  FAIL,
                                  0,
                                  request);
        }
        if (mbbLicense != NULL) {
            mbbLicense->finalizeBBSession();
        }
        mProcessStatus = STATUS_NONE;
    }
    if (o_license_data.data != NULL) {
        delete o_license_data.data;
        o_license_data.data = NULL;
    }
    if (o_service_info.data != NULL) {
        delete o_service_info.data;
        o_service_info.data = NULL;
    }
    return status;
}

mcdm_status_t MBBFunction::removeKeys(const string& content_data, string* message)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = NULL;

    IMarlinMediaFormat* content = NULL;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string extract_cid;

    content = getMarlinMediaFormat(content_data);
    if (content == NULL) {
        LOGE("Content Data is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    mediaFormat = content->getMediaFormat();
    if (mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN) {
        LOGE("Content Format is unknown type");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    if (content->getCid(NULL, extract_cid) != OK) {
        LOGE("Could not get CID");
        status = ERROR_NO_CID;
        goto EXIT;
    }

    mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                  mSessionId,
                                                  mMarlinFormat);
    if (mbbLicense == NULL) {
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    status  = mbbLicense->removeLicense(extract_cid);

EXIT:
    MARLINLOG_EXIT();
    if (message != NULL) {
        if (!createDefaultMessage(0, message)) {
            LOGE("Could not create message.");
            goto EXIT;
        }
    }
    return status;
}

mcdm_status_t MBBFunction::removeKeys(string* message)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              mMarlinFormat);
    IMarlinMediaFormat* content = NULL;
    uint32_t mediaFormat = IMarlinMediaFormat::FORMAT_UNKNOWN;
    string extract_cid;

    if (mMarlinFormat == NULL || mbbLicense == NULL
            || mbbLicense->getLicenseStatus() != IMarlinLicense::LICENSE_STATUS_INITIALIZED) {
        LOGE("Did not load license");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    content = mMarlinFormat;

    mediaFormat = content->getMediaFormat();
    if (mediaFormat == IMarlinMediaFormat::FORMAT_UNKNOWN) {
        LOGE("Content Format is unknown type");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }
    if (content->getCid(NULL, extract_cid) != OK) {
        LOGE("Could not get CID");
        status = ERROR_NO_CID;
        goto EXIT;
    }

    status  = mbbLicense->removeLicense(extract_cid);

EXIT:
    MARLINLOG_EXIT();
    if (message != NULL) {
        if (!createDefaultMessage(0, message)) {
            LOGE("Could not create message.");
            goto EXIT;
        }
    }
    return status;
}

mcdm_status_t MBBFunction::restoreKeys(const string& content_data, string* message)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = NULL;
    IMarlinMediaFormat* content = NULL;
    string extract_cid;

    content = getMarlinMediaFormat(content_data);
    if (content == NULL) {
        LOGE("Content Data is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                  mSessionId,
                                                  mMarlinFormat);
    if (mbbLicense == NULL) {
        LOGE("Could not get Marlin License object");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (message != NULL) {
        if (!createDefaultMessage(0, message)) {
            LOGE("Could not create message.");
            goto EXIT;
        }
    }
EXIT:
    MARLINLOG_EXIT();
    return status;
}

mcdm_status_t MBBFunction::queryKeyStatus(map<string, string>& infoMap)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);

    status = mbbLicense->queryLicenseStatus(infoMap);
    if (status != OK) {
        LOGE("Calling queryLicenseStatus. return code %d", status);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    MARLINLOG_EXIT();
EXIT:
    return status;
}

mcdm_status_t MBBFunction::queryKeyStatus(const string& name, string& value)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);
    map<string, string> infoMap;
    string keyStatus("");
    string message("");

    status = mbbLicense->queryLicenseStatus(infoMap);
    if (status != OK) {
        LOGE("Calling queryLicenseStatus. return code %d", status);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    if (infoMap.count(name) != 0) {
        keyStatus = infoMap[name];
    }
    if (!createPropertyMessage(keyStatus, &message)) {
        LOGE("Could not get prop message.");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }
    value.assign(message);
    MARLINLOG_EXIT();
EXIT:
    return status;
}

mcdm_status_t MBBFunction::processLTPMessage(const string* mimeType,
                                             const mcdm_key_type* keyType,
                                             const string ltp_data,
                                             string* request,
                                             string* /*url*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);
    LTPInitFormatData ltp_format_data;
    mcdm_data_t ltp_mesasge = {0, NULL};
    uint32_t cmd;
    bool is_license_exist = false;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    if (mimeType == NULL
            || MIME_TYPE_LTP.compare(*mimeType) != 0
            || keyType == NULL
            || *keyType != KEY_TYPE_OFFLINE) {
        LOGE("input parameter is illegal");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    parse_result = ltp_format_data.parse(ltp_data);
    if (parse_result < 0) {
        LOGE("Could not parse LTP Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    ltp_mesasge = ltp_format_data.getMessage();
    if (ltp_mesasge.len <= 0 || ltp_mesasge.data == NULL) {
        LOGE("ltp message is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    memcpy(&cmd, ltp_mesasge.data + 4, sizeof(uint32_t));
    if (cmd == USB_MESSAGE_HELLO_REQUEST) {
        LOGD("USB_MESSAGE_HELLO_REQUEST");
        if ((mProcessStatus == STATUS_STARTING) ||
                (mProcessStatus == STATUS_PROCESSING)) {
            mbbLicense->finalizeBBSession();
            mProcessStatus = STATUS_NONE;
        }
        status = mbbLicense->initializeBBSession(BB_MODE_TYPE_USB);
    }

    if (status != OK) {
        goto EXIT;
    }

    mProcessStatus = STATUS_PROCESSING;
    status = mbbLicense->processLTPMessage(ltp_mesasge,
                                           request,
                                           is_license_exist);
EXIT:
    MARLINLOG_EXIT();
    // mbbLicense->finalizeBBSession();
    // process_status = STATUS_DONE;
    return status;
}

mcdm_status_t MBBFunction::processLTPMessage(const mcdm_key_type* keyType,
                                             const string ltp_data,
                                             string* request,
                                             string* /*url*/)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;

    MarlinLicenseManager& lm(MarlinLicenseManager::getMarlinLicenseManager());
    MBBLicense* mbbLicense = (MBBLicense*)lm.getMarlinLicense(MarlinLicenseManager::LICENSE_TYPE_MBB,
                                                              mSessionId,
                                                              NULL);
    LTPInitFormatData ltp_format_data;
    mcdm_data_t ltp_mesasge = {0, NULL};
    uint32_t cmd;
    bool is_license_exist = false;
    parse_status_t parse_result = INIT_DATA_PARSE_OK;

    if (keyType == NULL
            || *keyType != KEY_TYPE_OFFLINE) {
        LOGE("input parameter is illegal");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    parse_result = ltp_format_data.parse(ltp_data);
    if (parse_result < 0) {
        LOGE("Could not parse LTP Data %d", parse_result);
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    ltp_mesasge = ltp_format_data.getMessage();
    if (ltp_mesasge.len <= 0 || ltp_mesasge.data == NULL) {
        LOGE("ltp message is illegal.");
        status = ERROR_ILLEGAL_ARGUMENT;
        goto EXIT;
    }

    memcpy(&cmd, ltp_mesasge.data + 4, sizeof(uint32_t));
    if (cmd == USB_MESSAGE_HELLO_REQUEST) {
        LOGD("USB_MESSAGE_HELLO_REQUEST");
        if ((mProcessStatus == STATUS_STARTING) ||
                (mProcessStatus == STATUS_PROCESSING)) {
            mbbLicense->finalizeBBSession();
            mProcessStatus = STATUS_NONE;
        }
        status = mbbLicense->initializeBBSession(BB_MODE_TYPE_USB);
    }
    if (status != OK) {
        goto EXIT;
    }

    mProcessStatus = STATUS_PROCESSING;
    status = mbbLicense->processLTPMessage(ltp_mesasge,
                                           request,
                                           is_license_exist);
EXIT:
    MARLINLOG_EXIT();
    // mbbLicense->finalizeBBSession();
    // process_status = STATUS_DONE;
    return status;
}

mcdm_status_t MBBFunction::getRegisteredStatus(string& service_id,
                                               string& account_id,
                                               string& subscription_id,
                                               string& state)
{
    MARLINLOG_ENTER();
    mcdm_status_t status = OK;
    MBBLicense* license = NULL;

    license = new MBBLicense();
    if (license == NULL) {
        LOGE("Could not create license session");
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    status = license->initializeBBSession(BB_MODE_TYPE_LOCAL);
    if (status != OK) {
        LOGE("Calling initializeBBSession. return code %d", status);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

    status = license->getRegisteredStatus(service_id,
                                          account_id,
                                          subscription_id,
                                          state);
    if (status != OK) {
        LOGE("Calling getRegisteredStatus. return code %d", status);
        status = ERROR_UNKNOWN;
        goto EXIT;
    }

EXIT:
    MARLINLOG_EXIT();
    if (license != NULL) {
        license->finalizeBBSession();
        delete license;
        license = NULL;
    }
    return status;
}
