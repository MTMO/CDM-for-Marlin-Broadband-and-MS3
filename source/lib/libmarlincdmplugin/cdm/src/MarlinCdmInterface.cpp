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

#include "MarlinCdmInterface.h"

using namespace marlincdm;

MarlinCdmInterface::MarlinCdmInterface(const uint8_t uuid[16]) :
            marlin_cdm_engine_(new MarlinCdmEngine(uuid))
{
}

MarlinCdmInterface::~MarlinCdmInterface()
{
}

mcdm_status_t MarlinCdmInterface::OpenSession(mcdm_SessionId_t& session_id)
{
    return marlin_cdm_engine_->OpenSession(session_id);
}

mcdm_status_t MarlinCdmInterface::CloseSession(const mcdm_SessionId_t& session_id)
{
    return marlin_cdm_engine_->CloseSession(session_id);
}

mcdm_status_t MarlinCdmInterface::CreateDecryptUnit(const mcdm_SessionId_t& session_id, const string& init_data)
{
    return marlin_cdm_engine_->CreateDecryptUnit(session_id, init_data);
}

mcdm_status_t MarlinCdmInterface::CloseDecryptUnit(const mcdm_SessionId_t& session_id)
{
    return marlin_cdm_engine_->CloseDecryptUnit(session_id);
}

mcdm_status_t MarlinCdmInterface::GenerateKeyRequest(const mcdm_SessionId_t& session_id,
                                                     const string& init_data,
                                                     const string* iniDataType,
                                                     const mcdm_key_type* keyType,
                                                     map<string, string>* optionalParameters,
                                                     string* request,
                                                     string* url)
{
    return marlin_cdm_engine_->GenerateKeyRequest(session_id,
                                                  init_data,
                                                  iniDataType,
                                                  keyType,
                                                  optionalParameters,
                                                  request,
                                                  url);
}

mcdm_status_t MarlinCdmInterface::AddKey(const mcdm_SessionId_t& session_id,
                                         const string& key_data,
                                         string* request)
{
    return marlin_cdm_engine_->AddKey(session_id, key_data, request);
}

mcdm_status_t MarlinCdmInterface::CancelKeyRequest(const mcdm_SessionId_t& session_id)
{
    return marlin_cdm_engine_->CancelKeyRequest(session_id);
}

mcdm_status_t MarlinCdmInterface::GetProvisionRequest(string& request,
                                                      string& default_url)
{
    return marlin_cdm_engine_->GetProvisionRequest(request, default_url);
}

mcdm_status_t MarlinCdmInterface::ProvideProvisionResponse(string& response_data)
{
    return marlin_cdm_engine_->ProvideProvisionResponse(response_data);
}

mcdm_status_t MarlinCdmInterface::useSecureBuffer(const uint8_t* mime,
                                                  bool& is_secure)
{
    return marlin_cdm_engine_->useSecureBuffer(mime, is_secure);
}

mcdm_status_t MarlinCdmInterface::isTypeSupported(const string& key_system,
                                                  const string& init_data_type,
                                                  const string& content_type,
                                                  const string& capability,
                                                  string* isTypeSupportedResult)
{
    return marlin_cdm_engine_->isTypeSupported(key_system, init_data_type, content_type, capability, isTypeSupportedResult);
}

mcdm_status_t MarlinCdmInterface::RestoreKey(const mcdm_SessionId_t& session_id,
                                             const string& init_data)
{
    return marlin_cdm_engine_->RestoreKey(session_id, init_data);
}

mcdm_status_t MarlinCdmInterface::RemoveKey(const mcdm_SessionId_t& session_id)
{
    return marlin_cdm_engine_->RemoveKey(session_id);
}

mcdm_status_t MarlinCdmInterface::QueryKeyStatus(const mcdm_SessionId_t &session_id,
                                                 map<string, string>& infoMap)
{
    return marlin_cdm_engine_->QueryKeyStatus(session_id, infoMap);
}

mcdm_status_t MarlinCdmInterface::GetPropertyString(const string& name,
                                                    string& value)
{
    return marlin_cdm_engine_->GetPropertyString(name, value);
}

mcdm_status_t MarlinCdmInterface::GetPropertyByteArray(const string& name,
                                                       uint8_t& value)
{
    return marlin_cdm_engine_->GetPropertyByteArray(name, value);
}

mcdm_status_t MarlinCdmInterface::SetPropertyString(const string& name,
                                                    string& value)
{
    return marlin_cdm_engine_->SetPropertyString(name, value);
}

mcdm_status_t MarlinCdmInterface::SetPropertyByteArray(const string& name,
                                                       uint8_t& value)
{
    return marlin_cdm_engine_->SetPropertyByteArray(name, value);
}

mcdm_status_t MarlinCdmInterface::Decrypt(const mcdm_SessionId_t& session_id,
                                          bool& is_encrypted,
                                          bool& is_secure,
                                          const uint8_t key[16],
                                          const uint8_t iv[16],
                                          mcdm_decryptMode& mode,
                                          mcdm_buffer_t* srcPtr,
                                          const mcdm_subsample_t* subSamples,
                                          size_t& numSubSamples,
                                          mcdm_buffer_t* dstPtr,
                                          size_t& decrypt_buffer_offset,
                                          string& errorDetailMsg)
{
    return marlin_cdm_engine_->Decrypt(session_id,
                                       is_encrypted,
                                       is_secure,
                                       key,
                                       iv,
                                       mode,
                                       srcPtr,
                                       subSamples,
                                       numSubSamples,
                                       dstPtr,
                                       decrypt_buffer_offset,
                                       errorDetailMsg);
}

bool MarlinCdmInterface::AttachEventListener(const mcdm_SessionId_t& /*session_id*/,
                                             MarlinCdmEventListener* /*listener*/)
{
    return false;
}

bool MarlinCdmInterface::DetachEventListener(const mcdm_SessionId_t& /*session_id*/,
                                             MarlinCdmEventListener* /*listener*/)
{
    return false;
}
