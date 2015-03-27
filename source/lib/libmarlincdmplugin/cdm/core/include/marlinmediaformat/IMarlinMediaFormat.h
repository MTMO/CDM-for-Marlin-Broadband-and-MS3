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

#ifndef __IMARLIN_MEDIA_FORMAT_H__
#define __IMARLIN_MEDIA_FORMAT_H__

#define FIRST_INDEX 0

#include <vector>

#include "ContentDataParser.h"
#include "MarlinCommonTypes.h"
#include "MarlinError.h"

namespace marlincdm {

class IMarlinMediaFormat {

    public:
        virtual mcdm_status_t getCid(const uint8_t kid[16], string &cid) = 0;
        virtual mcdm_status_t getCidWithIndex(const uint32_t index, string &cid) = 0;
        virtual mcdm_status_t getCids(vector<string> &cids) = 0;
        virtual mcdm_status_t getIV(string &iv) = 0;
        virtual mcdm_status_t getOctopusBundle(vector<string> &octpusBundle) = 0;
        virtual IMarlinMediaFormat* duplicate() = 0;

        enum MarlinMediaFormat {
            FORMAT_UNKNOWN      = 0,
            FORMAT_CENC         = 1,
            FORMAT_IPMP         = 2,
        };

        inline MarlinMediaFormat getMediaFormat() {
            return mMediaFormat;
        }

        inline mcdm_decryptMode getDecryptMode() {
            return mDecryptMode;
        }

        inline ContentDataParser* getDataParser() {
            return mParser;
        }

        virtual ~IMarlinMediaFormat(){
            if (mParser != NULL) {
                delete mParser;
                mParser = NULL;
            }
        }

    protected:
        IMarlinMediaFormat() : mMediaFormat(FORMAT_UNKNOWN),
                               mDecryptMode(MODE_UNENCRYPTED),
                               mParser(NULL){}

        MarlinMediaFormat mMediaFormat;
        mcdm_decryptMode mDecryptMode;
        ContentDataParser* mParser;
};  //class
};  //namespace

#endif /* __IMARLIN_MEDIA_FORMAT_H__ */

