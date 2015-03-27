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

#ifndef MARLIN_COMMON_CONFIG_H_
#define MARLIN_COMMON_CONFIG_H_

#include <string>
#include <map>

using namespace std;

namespace marlincdm {

/*! ID shows home directory for Marlin CDM. It is used to create license Database and store license.\n
    Please modify HOME_DIRECTORY as needed.\n
 */
const string HOME_DIRECTORY("/data/mediadrm/marlin");

/*! ID shows license extension.\n
    Please modify LICENSE_EXTENSION as needed.\n
 */
const string LICENSE_EXTENSION(".dat");

/*! grace period (sec).\n
    Please modify GRACE_PERIOD_SEC as needed.\n
 */
const int GRACE_PERIOD_SEC = 0;

/*! MTP Header Size.\n
    Please modify MTP_HEADER_SIZE as needed.\n
 */
const int MTP_HEADER_SIZE = 6;

} // namespace marlincdm

#endif  // MARLIN_COMMON_CONFIG_H_
