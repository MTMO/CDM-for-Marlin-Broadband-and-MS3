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

#ifndef __MBB_CRYPTO_H__
#define __MBB_CRYPTO_H__

#include "MarlinCrypto.h"

namespace marlincdm {

class MBBCrypto : public MarlinCrypto {
    public:

    MBBCrypto();
    virtual ~MBBCrypto();

    private:

};  //class
};  //namespace

#endif /* __MBB_CRYPTO_H__ */
