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

#ifndef MARLIN_CDM_ENGINE_H_
#define MARLIN_CDM_ENGINE_H_

#include <stdint.h>
#include <string>
#include <map>

#include "MarlinError.h"
#include "MarlinCommonTypes.h"
#include "MarlinCommonConfig.h"
#include "IMarlinFunction.h"
#include "CommonHeaderParser.h"

namespace marlincdm {

using namespace std;

class MarlinCdmEngine {
 public:

  MarlinCdmEngine(const uint8_t uuid[16]);
  virtual ~MarlinCdmEngine();

  mcdm_status_t OpenSession(mcdm_SessionId_t& session_id);

  mcdm_status_t CloseSession(const mcdm_SessionId_t& session_id);

  mcdm_status_t CreateDecryptUnit(const mcdm_SessionId_t& session_id, const string& init_data);

  mcdm_status_t CloseDecryptUnit(const mcdm_SessionId_t& session_id);

  mcdm_status_t GenerateKeyRequest(const mcdm_SessionId_t& session_id,
                                   const string& init_data,
                                   const string* mimeType,
                                   const mcdm_key_type* keyType,
                                   map<string, string>* optionalParameters,
                                   string* request,
                                   string* url);

  mcdm_status_t AddKey(const mcdm_SessionId_t& session_id,
                               const string& key_data,
                               string* request);

  mcdm_status_t CancelKeyRequest(const mcdm_SessionId_t& session_id);

  mcdm_status_t GetProvisionRequest(string& request, string& default_url);

  mcdm_status_t ProvideProvisionResponse(string& response_data);

  mcdm_status_t useSecureBuffer(const uint8_t* mime, bool& is_secure);

  mcdm_status_t isTypeSupported(const string& key_system,
                                const string& init_data_type,
                                const string& content_type,
                                const string& capability,
                                string* isTypeSupportedResult);

  mcdm_status_t RestoreKey(const mcdm_SessionId_t& session_id,
                                 const string& init_data);

  mcdm_status_t RemoveKey(const mcdm_SessionId_t& session_id);

  mcdm_status_t QueryKeyStatus(const mcdm_SessionId_t& session_id,
                               map<string, string>& infoMap);

  mcdm_status_t GetPropertyString(const string& name, string& value);

  mcdm_status_t GetPropertyByteArray(const string& name, uint8_t& value);

  mcdm_status_t SetPropertyString(const string& name, string& value);

  mcdm_status_t SetPropertyByteArray(const string& name, uint8_t& value);

  mcdm_status_t Decrypt(const mcdm_SessionId_t& session_id,
                        bool& is_encrypted,
                        bool& is_secure,
                        const uint8_t key[16],
                        const uint8_t iv[16],
                        mcdm_decryptMode& mode,
                        mcdm_buffer_t* srcPtr,
                        const mcdm_subsample_t* subSamples,
                        size_t& numSubSamples,
                        mcdm_buffer_t* dstPtr,
                        size_t& decrypt_buffer_length,
                        string& errorDetailMsg);


 private:

  static const std::string MARLIN_DIRECTORY;

  uint8_t mUUID[16];

  mcdm_status_t generateKeyRequestForAndroid(const mcdm_SessionId_t& session_id,
                                             const string& init_data,
                                             const string* mimeType,
                                             const mcdm_key_type* keyType,
                                             const DATA_TYPE_FLAG& dataType,
                                             map<string, string>* optionalParameters,
                                             string* request,
                                             string* url);

  mcdm_status_t generateKeyRequestForEME(const mcdm_SessionId_t& session_id,
                                         const string& init_data,
                                         const mcdm_key_type* keyType,
                                         string* request,
                                         string* url);

  mcdm_status_t getPropertyStringFormatString(const mcdm_SessionId_t& session_id,
                                              const string& name,
                                              string& value);

  IMarlinFunction* createMarlinFunction(const mcdm_SessionId_t& session_id,
                                        const uint8_t uuid[16]);

  IMarlinFunction* getMarlinFunction(const mcdm_SessionId_t& session_id);

};

} // namespace marlincdm

#endif  // MARLIN_CDM_ENGINE_H_
