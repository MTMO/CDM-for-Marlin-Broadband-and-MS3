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

#ifndef _DRM_ION_BUF_H_
#define _DRM_ION_BUF_H_

#include <utils/String8.h>
#include <linux/ion.h>
#include <linux/ioctl.h>

namespace marlindrm {

class IonBuf {
    public:
        IonBuf();
        ~IonBuf();

    public:
        void* getBuffer(size_t size);
        int getFd();
        size_t getSize();

    private:
        void free();

    private:
        int devFd;
        int ionFd;
        size_t size;
        void *mem;
        struct ion_handle_data ion_handle;
};

}

#endif
