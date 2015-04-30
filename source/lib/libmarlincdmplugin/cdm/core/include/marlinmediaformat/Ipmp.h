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

#ifndef __IPMP_H__
#define __IPMP_H__

#include <map>

#include "IMarlinMediaFormat.h"

namespace marlincdm {

class Ipmp : public IMarlinMediaFormat {

    public:
        Ipmp();
        Ipmp(const Ipmp &o);
        Ipmp(const uint8_t *sinfData, size_t sinfSize, string cid);

        virtual ~Ipmp();

        virtual mcdm_status_t getCid(const uint8_t kid[16], string &cid);
        virtual mcdm_status_t getCidWithIndex(const uint32_t index, string &cid);
        virtual mcdm_status_t getCids(vector<string> &cids);
        virtual mcdm_status_t getIV(string &iv);
        virtual mcdm_status_t getOctopusBundle(vector<string> &octpusBundle);
        virtual Ipmp* duplicate();

    private:
        void init(const uint8_t *sinfData, size_t sinfSize, string cid);
        uint8_t *mSinfData;
        u_int32_t mSinfSize;
        string mContentId;
};  //class
};  //namespace

#endif /* __IPMP_H__ */
