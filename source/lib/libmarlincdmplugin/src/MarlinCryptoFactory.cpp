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

//#define LOG_NDEBUG 0
#define LOG_TAG "MarlinCryptoFactory"
#include <utils/Log.h>

#include <MarlinConstants.h>
#include "MarlinCryptoFactory.h"
#include "MarlinCDMSingleton.h"
#include "MarlinCryptoPlugin.h"
#include "utils/Errors.h"

namespace marlindrm {

MarlinCryptoFactory::MarlinCryptoFactory() {
}

MarlinCryptoFactory::~MarlinCryptoFactory() {
}

bool MarlinCryptoFactory::isCryptoSchemeSupported(const uint8_t uuid[16]) const {

    bool ret = true;
    if (memcmp(uuid, marlincdm::MBB_UUID, 16)
            && memcmp(uuid, marlincdm::MS3_UUID, 16)){
        ALOGD("Do not supported UUID: %s", uuid);
        ret = false;
    }
    return ret;
}

android::status_t MarlinCryptoFactory::createPlugin(const uint8_t uuid[16], const void* data,
                                       size_t size, android::CryptoPlugin** plugin) {
    if (!isCryptoSchemeSupported(uuid)) {
      *plugin = NULL;
      return android::BAD_VALUE;
    }

    if (size == 0) {
        return -EINVAL;
    }

    *plugin = new MarlinCryptoPlugin(data, size, getCDM(uuid, data, size));

    return android::OK;
}

} // namespace marlindrm
