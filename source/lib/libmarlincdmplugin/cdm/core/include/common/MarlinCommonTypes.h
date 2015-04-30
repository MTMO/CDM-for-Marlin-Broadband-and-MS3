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

#ifndef MARLIN_COMMON_TYPE_H_
#define MARLIN_COMMON_TYPE_H_

#include <string>
#include <cstdint>

#define DOMAINID_ACCFIELD_SIZE 8 /**< Account ID size in domain ID */

using namespace std;

namespace marlincdm {

/**
 * @brief Unique string to identify Marlin CDM object
 */
typedef string mcdm_SessionId_t;

typedef int mcdm_Unit_Handle_t;

/**
 * @brief encryption mode
 */
enum mcdm_decryptMode {
    MODE_UNENCRYPTED = 0, //!< content is not encrypted
    MODE_AES_CTR     = 1, //!< content is encrypted by AES CTR
    MODE_AES_CBC      = 2, //!< content is encrypted by AES CBC
};

/**
 * @brief algorithm type
 */
enum mcdm_algorithmType{
    CRYPTO_KFCBC_ALGO_NONE = 0, // NONE
    CRYPTO_KFCBC_ALGO_AES = 0,  // AES
    CRYPTO_KFCBC_ALGO_DES,      // DES
};

/**
 * @brief padding type
 */
enum mcdm_paddingType{
    CRYPTO_KFCBC_PAD_NONE = 0, // NONE
    CRYPTO_KFCBC_PAD_RFC2630, // RFC2630
    CRYPTO_KFCBC_PAD_LENGTH,
};

/**
 * @brief This structure includes Clear data length and Encrypted data length
 */
struct mcdm_subsample_t {
    size_t mNumBytesOfClearData; //!<Clear data size
    size_t mNumBytesOfEncryptedData; //!< Encrypted data size
};

/**
 * @brief Marlin CDM Key Type
 */
enum mcdm_key_type {
    KEY_TYPE_OFFLINE,
    KEY_TYPE_STREAMING,
    KEY_TYPE_RELEASE
};

/**
 * @brief BB mode Type
 */
enum mcdm_bb_mode {
    BB_MODE_TYPE_LOCAL = 0, //!< mode local
    BB_MODE_TYPE_USB //!< mode usb that it is used LTP Process
};

/**
 * @brief This structure includes data length and data buffer and fd
 */
struct mcdm_buffer_t {
    size_t len; //!< data size
    uint8_t *data; //!< data buffer
    int fd; //!< File descriptor
};

/**
 * @brief This structure includes data length and data buffer
 */
struct mcdm_data_t {
    size_t len; //!< data size
    uint8_t *data; //!< data buffer
};

/**
 * @brief This structure includes data length and ION fd
 */
struct mcdm_iondata_t {
    size_t len; //!< data size
    int fd; //!< File descriptor for ION
};

/**
 * @brief This structure includes date data length and date data.
 */
struct mcdm_date_t {
    int32_t year;
    int32_t mon;
    int32_t mday;
    int32_t hour;
    int32_t min;
    int32_t sec;
    int32_t msec;
    int32_t offset;
    int32_t isdst;
};

} // namespace marlincdm

#endif  // MARLIN_COMMON_TYPE_H_
