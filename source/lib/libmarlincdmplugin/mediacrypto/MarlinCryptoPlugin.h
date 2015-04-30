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

#ifndef MARLIN_CRYPTO_PLUGIN_H_
#define MARLIN_CRYPTO_PLUGIN_H_

#include <stdint.h>

#include <MarlinCdmInterface.h>
#include <MarlinCommonTypes.h>
#include "media/hardware/CryptoAPI.h"
#include "media/stagefright/foundation/ABase.h"
#include "media/stagefright/foundation/AString.h"
#include "IonBuf.h"

namespace marlindrm {

using android::AString;
using android::CryptoPlugin;
using marlincdm::MarlinCdmInterface;

using namespace std;
using namespace marlincdm;

class MarlinCryptoPlugin : public android::CryptoPlugin {
 public:
  MarlinCryptoPlugin(const void* data, size_t size,
                 marlincdm::MarlinCdmInterface* cdm);
  virtual ~MarlinCryptoPlugin();

  virtual bool requiresSecureDecoderComponent(const char* mime) const;

  virtual ssize_t decrypt(bool secure, const uint8_t key[16],
                          const uint8_t iv[16], CryptoPlugin::Mode mode, const void* srcPtr,
                          const CryptoPlugin::SubSample* subSamples, size_t numSubSamples,
                          void* dstPtr, AString* errorDetailMsg);

  virtual android::status_t getOutputProtectionFlags(int *data, size_t size);

 private:
  void convertDecryptModeFwToMh(const CryptoPlugin::Mode mode, mcdm_decryptMode &out);

  void convertSubSampleInfoFwToMh(const CryptoPlugin::SubSample *subSamples,
          size_t numSubSamples,
          mcdm_subsample_t **out_subSamples);

  DISALLOW_EVIL_CONSTRUCTORS(MarlinCryptoPlugin);

  auto_ptr<MarlinCdmInterface> const mCDM;

  mcdm_SessionId_t mSessionId;
  mcdm_status_t license_status;

  IonBuf* encIonBuffer;
  IonBuf* decIonBuffer;

  static void incrementIV(uint64_t increaseBy, std::vector<uint8_t>* ivPtr);

 protected:
};

} // namespace marlindrm

#endif // MARLIN_CRYPTO_PLUGIN_H_
