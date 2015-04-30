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

#ifndef __IMARLIN_FUNCTION_H__
#define __IMARLIN_FUNCTION_H__

#include <map>

#include "MarlinCommonTypes.h"
#include "MarlinError.h"

#include "MarlinLicense.h"
#include "MarlinCryptoHandler.h"
#include "IMarlinMediaFormat.h"

namespace marlincdm {

class IMarlinFunction {
protected:
    typedef enum _ProcessStatus {
        STATUS_NONE = 0,
        STATUS_STARTING,
        STATUS_PROCESSING,
        STATUS_DONE,
    } ProcessStatus;

    mcdm_SessionId_t* mSessionId;
    IMarlinMediaFormat* mMarlinFormat;
    ProcessStatus mProcessStatus;
    mcdm_key_type mProcessKeyType;
    string mLicensePath;
    bool isEME;

    IMarlinMediaFormat* getMarlinMediaFormat(const string content_data);

    IMarlinMediaFormat* getIPMPFormat(const string ipmp_data);

    IMarlinMediaFormat* getCENCFormat(const string cenc_data);

public:
    IMarlinFunction();
    IMarlinFunction(const mcdm_SessionId_t& session_id);
    virtual ~IMarlinFunction();

    virtual mcdm_status_t initUnit(const string content_data, string* message);

    virtual mcdm_status_t closeUnit(string* message);

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              map<string, string>* optionalParameters,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processServiceToken(const string* mimeType,
                                              const mcdm_key_type* keyType,
                                              const string atkn_data,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processServiceToken(const mcdm_key_type* keyType,
                                              const string metadata,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processServiceToken(const string atkn,
                                              string* request,
                                              string* url) = 0;

    virtual mcdm_status_t processRightsResponse(const string& response,
                                                string* request) = 0;

    virtual mcdm_status_t removeKeys(const string& content_data,
                                     string* message);

    virtual mcdm_status_t removeKeys(string* message);

    virtual mcdm_status_t restoreKeys(const string& content_data,
                                      string* message);

    virtual mcdm_status_t queryKeyStatus(map<string, string>& infoMap);

    virtual mcdm_status_t queryKeyStatus(const string& name,
                                         string& value);

    virtual mcdm_status_t decrypt(bool& is_encrypted,
                                  bool& is_secure,
                                  const uint8_t key[16],
                                  const uint8_t iv[16],
                                  mcdm_decryptMode& mode,
                                  mcdm_buffer_t* srcPtr,
                                  const mcdm_subsample_t* subSamples,
                                  size_t& numSubSamples,
                                  mcdm_buffer_t* dstPtr,
                                  size_t& decrypt_buffer_offset,
                                  string& errorDetailMsg);

    virtual mcdm_status_t getPropertyString(const string& name, string& value);

    virtual mcdm_status_t getPropertyFormatString(const string& name, string& value);

    virtual mcdm_status_t isTypeSupported(const string& key_system,
                                          const string& init_data_type,
                                          const string& content_type,
                                          const string& capability,
                                          string* isTypeSupportedResult);

    virtual bool isEme() const {
        return isEME;
    }

    virtual void setIsEme(bool isEme) {
        isEME = isEme;
    }
};
//class
};  //namespace

#endif /* __IMARLIN_FUNCTION_H__ */
