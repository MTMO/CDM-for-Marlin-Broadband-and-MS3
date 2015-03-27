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

#ifndef __MARLIN_ERROR_H__
#define __MARLIN_ERROR_H__

#include <string>

namespace marlincdm {
/**
 * @brief Marlin CDM Error Type
 */
enum mcdm_status_t {
    OK = 0,  //!< process is success
    ERROR_CANNOT_HANDLE,  //!< Content information cannot be handled
    ERROR_UNKNOWN,  //!< Error by other reasons
    ERROR_ILLEGAL_ARGUMENT,  //!< Invalid parameter
    ERROR_ILLEGAL_STATE,  //!< Invalid state
    ERROR_UNSUPPORTED,  //!< Non support
    ERROR_UNSUPPORTED_SECURE_DECODER,  //!< Non support secure decoder
    ERROR_UNSUPPORTED_CRYPTO_MODE,  //!< Non support crypto mode
    ERROR_ILLEGAL_LICENSE,  //!< Invalid license
    ERROR_RIGHTS_NOT_SET,  //!< license has not been set
    ERROR_RUNNING_ALREADY,  //!< process has already run
    ERROR_ALREADY_RIGHTS_SET,  //!< license has been already set
    ERROR_FAILED_ACQUIRE_RIGHTS,  //!< acquisition license failed
    ERROR_CANNOT_FIND_HANDLE,  //!< Handle id cannot be found
    ERROR_FAILED_DECRYPTION,  //!< Failed to decrypt
    ERROR_NO_LICENSE,  //!< License does not exist
    ERROR_LICENSE_EXPIRED,  //!< License is expired
    ERROR_SESSION_NOT_OPENED,  //!< Session is discarded
    ERROR_NO_CID,  //!< No Key for KID
};

void convertDecryptErrorToMessage(mcdm_status_t status, std::string& str);

} // marlincdm

#endif /* __MARLIN_ERROR_H__ */
