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

#ifndef MARLIN_CRYPTO_FACTORY_H_
#define MARLIN_CRYPTO_FACTORY_H_

#include "media/hardware/CryptoAPI.h"
#include "media/stagefright/foundation/ABase.h"
#include "utils/Errors.h"

namespace marlindrm {

class MarlinCryptoFactory : public android::CryptoFactory {
 public:
  MarlinCryptoFactory();
  virtual ~MarlinCryptoFactory();

  virtual bool isCryptoSchemeSupported(const uint8_t uuid[16]) const;

  virtual android::status_t createPlugin(const uint8_t uuid[16],
                                         const void* data, size_t size,
                                         android::CryptoPlugin** plugin);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MarlinCryptoFactory);
};

} // namespace marlindrm

#endif // MARLIN_CRYPTO_FACTORY_H_
