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

#include "MarlinCDMSingleton.h"

#include "utils/Mutex.h"

namespace marlindrm {

using marlincdm::MarlinCdmInterface;
using android::Mutex;

Mutex cdmLock;
MarlinCdmInterface* cdm = NULL;

MarlinCdmInterface* getCDM(const uint8_t uuid[16]) {
  Mutex::Autolock lock(cdmLock);
  return new MarlinCdmInterface(uuid);
}

MarlinCdmInterface* getCDM(const uint8_t uuid[16], const void* /*data*/, size_t /*size*/) {
  Mutex::Autolock lock(cdmLock);
  return new MarlinCdmInterface(uuid);
}

} // namespace marlindrm
