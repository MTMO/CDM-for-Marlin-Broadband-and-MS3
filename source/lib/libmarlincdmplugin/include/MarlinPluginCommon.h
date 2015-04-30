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

#ifndef __MARLIN_PLUGIN_COMMON__
#define __MARLIN_PLUGIN_COMMON__

#include "MarlinCommonTypes.h"
#include "MarlinConstants.h"
#include "MarlinError.h"
#include <media/stagefright/MediaErrors.h>
#include <utils/String8.h>

namespace marlindrm {

const size_t KEY_IV_SIZE = 16;
const size_t KEY_ID_SIZE = 16;

static android::status_t convertMarlinHandlerError(marlincdm::mcdm_status_t &mcdmStatus) {
    switch (mcdmStatus) {
        case marlincdm::OK:
            return android::OK;
        case marlincdm::ERROR_SESSION_NOT_OPENED:
        case marlincdm::ERROR_ILLEGAL_ARGUMENT:
            return android::BAD_VALUE;
        case marlincdm::ERROR_ILLEGAL_STATE:
        case marlincdm::ERROR_UNSUPPORTED_CRYPTO_MODE:
        case marlincdm::ERROR_UNSUPPORTED_SECURE_DECODER:
            return android::ERROR_DRM_CANNOT_HANDLE;
        case marlincdm::ERROR_LICENSE_EXPIRED:
            return android::ERROR_DRM_LICENSE_EXPIRED;
        case marlincdm::ERROR_ILLEGAL_LICENSE:
        case marlincdm::ERROR_RIGHTS_NOT_SET:
        case marlincdm::ERROR_NO_LICENSE:
            return android::ERROR_DRM_NO_LICENSE;
        case marlincdm::ERROR_RUNNING_ALREADY:
            return android::ERROR_DRM_CANNOT_HANDLE;
        case marlincdm::ERROR_CANNOT_FIND_HANDLE:
            return android::ERROR_DRM_DECRYPT_UNIT_NOT_INITIALIZED;
        case marlincdm::ERROR_FAILED_DECRYPTION:
            return android::ERROR_DRM_DECRYPT;
        case marlincdm::ERROR_UNKNOWN:
        default:
            return android::ERROR_DRM_UNKNOWN;
    }
}

};  //namespace

#endif /* __MARLIN_PLUGIN_COMMON__ */
