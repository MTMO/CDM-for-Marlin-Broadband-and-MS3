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

#ifndef __INIT_DATA_PARSER_ERRORS_H__
#define __INIT_DATA_PARSER_ERRORS_H__

namespace marlincdm {

enum parse_status_t {
    INIT_DATA_PARSE_OK                      = 0,
    INIT_DATA_PARSE_ERROR_UNKNOWN           = -1000,
    INIT_DATA_PARSE_ERROR_INVALID_ARG       = INIT_DATA_PARSE_ERROR_UNKNOWN - 1,
    INIT_DATA_PARSE_ERROR_ILLEGAL_VERSION   = INIT_DATA_PARSE_ERROR_UNKNOWN - 2,
    INIT_DATA_PARSE_ERROR_HEADER_ILLEGAL    = INIT_DATA_PARSE_ERROR_UNKNOWN - 3,
    INIT_DATA_PARSE_ERROR_PAYLOAD_ILLEGAL   = INIT_DATA_PARSE_ERROR_UNKNOWN - 4,
    INIT_DATA_PARSE_ERROR_NOT_SUPPORTED     = INIT_DATA_PARSE_ERROR_UNKNOWN - 5,
    INIT_DATA_PARSE_ERROR_IDENTIFIED_CID    = INIT_DATA_PARSE_ERROR_UNKNOWN - 6,
};

}; // namespace

#endif /* __INIT_DATA_PARSER_ERRORS_H__ */
