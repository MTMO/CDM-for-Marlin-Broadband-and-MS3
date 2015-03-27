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
#define LOG_TAG "MarlinDrmFactory"
#include <utils/Log.h>

#include <MarlinConstants.h>
#include "MarlinDrmFactory.h"
#include "MarlinCDMSingleton.h"
#include "MarlinDrmPlugin.h"
#include "utils/Errors.h"

namespace marlindrm {

bool MarlinDrmFactory::isCryptoSchemeSupported(const uint8_t uuid[16]) {
    bool ret = true;
    if (memcmp(uuid, marlincdm::MBB_UUID, 16)
            && memcmp(uuid, marlincdm::MS3_UUID, 16)){
        ALOGD("Do not supported UUID: %s", uuid);
        ret = false;
    }
    return ret;
}

bool MarlinDrmFactory::isContentTypeSupported(const String8 &mimeType) {
    return mimeType == "video/mp4";
}

status_t MarlinDrmFactory::createDrmPlugin(const uint8_t uuid[16],
                                           android::DrmPlugin** plugin) {
    if (!isCryptoSchemeSupported(uuid)) {
        ALOGD("Do not supported UUID: %s", uuid);
        *plugin = NULL;
        return android::BAD_VALUE;
    }

    *plugin = new MarlinDrmPlugin(getCDM(uuid));
    return android::OK;
}

} // namespace marlindrm
