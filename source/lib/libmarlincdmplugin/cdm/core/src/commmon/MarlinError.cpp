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

#include "MarlinError.h"

namespace marlincdm {
void convertDecryptErrorToMessage(mcdm_status_t status, std::string& str)
{
    switch (status) {
    case ERROR_NO_LICENSE:
        str.assign("License does not exist");
        break;
    case ERROR_LICENSE_EXPIRED:
        str.assign("License is expired");
        break;
    case ERROR_SESSION_NOT_OPENED:
        str.assign("Session is discarded");
        break;
    case ERROR_UNSUPPORTED_CRYPTO_MODE:
        str.assign("Non support crypto mode");
        break;
    case ERROR_UNSUPPORTED_SECURE_DECODER:
        str.assign("Non support secure decoder ");
        break;
    case ERROR_CANNOT_HANDLE:
        str.assign("Content information cannot be handled");
        break;
    case ERROR_NO_CID:
        str.assign("No Key for KID");
        break;
    case ERROR_ILLEGAL_ARGUMENT:
        str.assign("Invalid parameter");
        break;
    case ERROR_FAILED_DECRYPTION:
        str.assign("Failed to decrypt");
        break;
    case ERROR_UNKNOWN:
    default:
        str.assign("Error by other reasons");
        break;
    }
}
} // namespace marlincdm
