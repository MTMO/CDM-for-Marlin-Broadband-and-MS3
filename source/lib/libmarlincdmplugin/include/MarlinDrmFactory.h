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

#ifndef MARLIN_DRM_FACTORY_H_
#define MARLIN_DRM_FACTORY_H_

#include "media/drm/DrmAPI.h"
#include "media/stagefright/foundation/ABase.h"
#include "utils/Errors.h"

namespace marlindrm {

class MarlinDrmFactory : public android::DrmFactory {
 public:
  MarlinDrmFactory() {};
  virtual ~MarlinDrmFactory() {};

  virtual bool isCryptoSchemeSupported(const uint8_t uuid[16]);

  virtual bool isContentTypeSupported(const android::String8 &mimeType);

  virtual android::status_t createDrmPlugin(const uint8_t uuid[16],
                                            android::DrmPlugin** plugin);

 private:
  DISALLOW_EVIL_CONSTRUCTORS(MarlinDrmFactory);

//  static MarlinGenericCryptoInterface sOemCryptoInterface;
};

} // namespace marlindrm

#endif // MARLIN_DRM_FACTORY_H_
