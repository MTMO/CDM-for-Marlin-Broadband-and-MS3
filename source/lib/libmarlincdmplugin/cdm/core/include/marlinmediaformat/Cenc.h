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

#ifndef __CENC_H__
#define __CENC_H__

#include <map>

#include "IMarlinMediaFormat.h"

namespace marlincdm {

class Cenc : public IMarlinMediaFormat {

    public:
        Cenc();
        Cenc(const Cenc &o);
        Cenc(const string cid);
        Cenc(const uint8_t *psshData, size_t psshSize, const string cid);
        Cenc(const uint8_t *psshData, size_t psshSize, const vector<uint8_t*> kidList, const string cid);

        virtual ~Cenc();

        virtual mcdm_status_t getCid(const uint8_t kid[16], string &cid);
        virtual mcdm_status_t getCidWithIndex(const uint32_t index, string &cid);
        virtual mcdm_status_t getCids(vector<string> &cids);
        virtual mcdm_status_t getIV(string &iv);
        virtual mcdm_status_t getOctopusBundle(vector<string> &octpusBundle);
        virtual Cenc* duplicate();

    private:
        void init(const uint8_t *psshData, size_t psshSize, const string cid);
        void init(const uint8_t *psshData, size_t psshSize, vector<uint8_t*> kidList, const string cid);
        vector<uint8_t*> mKidList;
        uint8_t *mPsshData;
        u_int32_t mPsshSize;
        string mContentId;
};  //class
};  //namespace

#endif /* __CENC_H__ */
