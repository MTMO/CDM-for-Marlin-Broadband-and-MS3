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

#ifndef __MARLIN_UTILS_H__
#define __MARLIN_UTILS_H__

#include <string>
#include <cstdint>

namespace marlincdm {

void read8(const uint8_t *data, uint8_t &out);

void read16(const uint8_t *data, uint16_t &out);

void read32(const uint8_t *data, uint32_t &out);

bool strToBin(const char* pInStr, unsigned char* pOutBin, int idSize);

bool binToStr(const uint8_t* pInBuf, unsigned int idsize, std::string& outStr);

void SNPRINTF(char* str, uint32_t size , const char* format, ... );

char* STRTOK(char *pDataToParse, const char* pDelimiter);

} // marlincdm

#endif /* __MARLIN_UTILS_H__ */
