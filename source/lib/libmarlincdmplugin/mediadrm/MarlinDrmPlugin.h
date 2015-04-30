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

#ifndef MARLIN_DRM_PLUGIN_H_
#define MARLIN_DRM_PLUGIN_H_

#include <stdint.h>
#include <map>

#include "media/drm/DrmAPI.h"
#include "media/stagefright/foundation/ABase.h"
#include "media/stagefright/foundation/AString.h"
#include "utils/Errors.h"
#include "utils/KeyedVector.h"
#include "utils/List.h"
#include "utils/String8.h"
#include "utils/Vector.h"
#include "MarlinCommonTypes.h"
#include "MarlinCdmInterface.h"
#include "IonBuf.h"

using namespace android;
using namespace marlincdm;
using std::map;

namespace marlindrm {

class MarlinDrmPlugin : public android::DrmPlugin {
 public:
  MarlinDrmPlugin(MarlinCdmInterface* cdm);

  virtual ~MarlinDrmPlugin();

  virtual status_t openSession(Vector<uint8_t>& sessionId);

  virtual status_t closeSession(const Vector<uint8_t>& sessionId);

  virtual status_t getKeyRequest(const Vector<uint8_t>& scope,
                                 const Vector<uint8_t>& initData,
                                 const String8& mimeType,
                                 KeyType keyType,
                                 const KeyedVector<String8, String8>& optionalParameters,
                                 Vector<uint8_t>& request,
                                 String8& defaultUrl);

  virtual status_t provideKeyResponse(const Vector<uint8_t>& scope,
                                      const Vector<uint8_t>& response,
                                      Vector<uint8_t>& keySetId);

  virtual status_t removeKeys(const Vector<uint8_t>& sessionId);

  virtual status_t restoreKeys(const Vector<uint8_t>& sessionId,
                               const Vector<uint8_t>& keySetId);

  virtual status_t queryKeyStatus(const Vector<uint8_t>& sessionId,
                                  KeyedVector<String8, String8>& infoMap) const;

  virtual status_t getProvisionRequest(Vector<uint8_t>& request,
                                       String8& defaultUrl);

  virtual status_t getProvisionRequest(String8 const &cert_type,
                                             String8 const &cert_authority,
                                             Vector<uint8_t> &request,
                                             String8 &defaultUrl);

  virtual status_t provideProvisionResponse(const Vector<uint8_t>& response);

  virtual status_t provideProvisionResponse(Vector<uint8_t> const &response,
                                                  Vector<uint8_t> &certificate,
                                                  Vector<uint8_t> &wrapped_key);
  virtual status_t unprovisionDevice();

  virtual status_t getSecureStops(List<Vector<uint8_t> >& secureStops);

  virtual status_t releaseSecureStops(const Vector<uint8_t>& ssRelease);

  virtual status_t getPropertyString(const String8& name, String8& value) const;

  virtual status_t getPropertyByteArray(const String8& name,
                                        Vector<uint8_t>& value) const;

  virtual status_t setPropertyString(const String8& name, const String8& value);

  virtual status_t setPropertyByteArray(const String8& name,
                                        const Vector<uint8_t>& value);

  virtual status_t setCipherAlgorithm(const Vector<uint8_t>& sessionId,
                                      const String8& algorithm);

  virtual status_t setMacAlgorithm(const Vector<uint8_t>& sessionId,
                                   const String8& algorithm);

  virtual status_t encrypt(const Vector<uint8_t>& sessionId,
                           const Vector<uint8_t>& keyId,
                           const Vector<uint8_t>& input,
                           const Vector<uint8_t>& iv,
                           Vector<uint8_t>& output);

  virtual status_t decrypt(const Vector<uint8_t>& sessionId,
                           const Vector<uint8_t>& keyId,
                           const Vector<uint8_t>& input,
                           const Vector<uint8_t>& iv,
                           Vector<uint8_t>& output);

  virtual status_t sign(const Vector<uint8_t>& sessionId,
                        const Vector<uint8_t>& keyId,
                        const Vector<uint8_t>& message,
                        Vector<uint8_t>& signature);

  virtual status_t verify(const Vector<uint8_t>& sessionId,
                          const Vector<uint8_t>& keyId,
                          const Vector<uint8_t>& message,
                          const Vector<uint8_t>& signature,
                          bool& match);

  virtual status_t signRSA(Vector<uint8_t> const &sessionId,
                           String8 const &algorithm,
                           Vector<uint8_t> const &message,
                           Vector<uint8_t> const &wrapped_key,
                           Vector<uint8_t> &signature);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MarlinDrmPlugin);

 struct CryptoSession {
  public:
    CryptoSession()
     : mCdmSessionId(""),
       mCipherAlgorithm(MODE_UNENCRYPTED),
       mMacAlgorithm(MODE_UNENCRYPTED){}

   CryptoSession(mcdm_SessionId_t sessionId)
     : mCdmSessionId(sessionId),
       mCipherAlgorithm(MODE_UNENCRYPTED),
       mMacAlgorithm(MODE_UNENCRYPTED){}

   mcdm_SessionId_t cdmSessionId() const { return mCdmSessionId; }

   mcdm_decryptMode cipherAlgorithm() const { return mCipherAlgorithm; }

   void setCipherAlgorithm(mcdm_decryptMode newAlgorithm) {
     mCipherAlgorithm = newAlgorithm;
   }

   mcdm_decryptMode macAlgorithm() const { return mMacAlgorithm; }

   void setMacAlgorithm(mcdm_decryptMode newAlgorithm) {
     mMacAlgorithm = newAlgorithm;
   }

  private:
   mcdm_SessionId_t mCdmSessionId;
   mcdm_decryptMode mCipherAlgorithm;
   mcdm_decryptMode mMacAlgorithm;
 };

  auto_ptr<MarlinCdmInterface> mCDM;

  mcdm_SessionId_t mSessionId;

  IonBuf* encIonBuffer;
  IonBuf* decIonBuffer;

  CryptoSession cryptoSession;

  status_t convertSessionIdFwToCDM(const Vector<uint8_t> &i_fwSessionId,
                                   mcdm_SessionId_t &o_cdmSessionId);

  status_t convertKeyTypeFwToCDM(KeyType &i_fwKeyType,
                                 mcdm_key_type &o_cdmKeyType);

  string vectorToString(Vector<uint8_t> const &i_vector);

  Vector<uint8_t> stringToVector(string &i_str);
};

} // namespace marlindrm

#endif // MARLIN_DRM_PLUGIN_H_
