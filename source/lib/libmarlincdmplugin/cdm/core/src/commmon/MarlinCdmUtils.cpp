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

#include <stdarg.h>
#include <stdlib.h>
#include <cstring>
#include "MarlinCdmUtils.h"

void marlincdm::read8(const uint8_t *data, uint8_t &out) {
    memcpy(&out, data, sizeof(uint8_t));
}

void marlincdm::read16(const uint8_t *data, uint16_t &out) {
    uint8_t buf;
    for (size_t i = 0; i < sizeof(uint16_t); i++) {
        memcpy(&buf, data + i, 1);
        out = (out << 8) | buf;
    }
}

void marlincdm::read32(const uint8_t *data, uint32_t &out) {
    uint8_t buf;
    for (size_t i = 0; i < sizeof(uint32_t); i++) {
        memcpy(&buf, data + i, 1);
        out = (out << 8) | buf;
    }
}

bool marlincdm::strToBin(const char* pInStr, unsigned char* pOutBin, int idSize)
{
    unsigned char workChar = 0;
    bool switchFlg = true;
    int cnt = 0;

    if (pInStr[idSize] != 0x00) {
        return false;
    }
    memset(pOutBin, 0x00, idSize/2);

    for (cnt = 0; cnt < idSize; ) {
        switch (pInStr[cnt]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                workChar = pInStr[cnt] - 0x30;
                break;
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                workChar = pInStr[cnt] - 0x57;
                break;
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
                workChar = pInStr[cnt] - 0x37;
                break;
            default:
                return false;
                /* ERROR */
                break;
        }
        *pOutBin = *pOutBin | workChar;
        cnt++;
        if (cnt > idSize-1) {
            break;
        }
        if (switchFlg) {
            *pOutBin = *pOutBin << 4;
            switchFlg = false;
        } else {
            pOutBin++;
            switchFlg = true;
        }
    }
    return true;
}

bool marlincdm::binToStr(const uint8_t* pInBuf, unsigned int idsize, std::string& outStr) {
  static const char kHexChars[] = "0123456789ABCDEF";
  std::string out_buffer(idsize * 2, '\0');

  if (pInBuf == NULL || idsize <= 0) {
      return false;
  }

  for (unsigned int i = 0; i < idsize; ++i) {
    char byte = pInBuf[i];
    out_buffer[(i << 1)] = kHexChars[(byte >> 4) & 0xf];
    out_buffer[(i << 1) + 1] = kHexChars[byte & 0xf];
  }
  outStr = out_buffer;
  return true;
}

void marlincdm::SNPRINTF(char* str, uint32_t size , const char* format, ... ) {
    va_list args;
    va_start(args,format);
    vsnprintf(str, size, format, args);
    va_end( args);
}

char* marlincdm::STRTOK(char *pDataToParse, const char* pDelimiter) {
    return strtok(pDataToParse, pDelimiter);
}
