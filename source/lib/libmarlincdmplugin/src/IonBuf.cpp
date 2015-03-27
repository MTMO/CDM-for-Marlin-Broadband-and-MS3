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

#define LOG_TAG "IonBuf"
#include <utils/Log.h>

#include "IonBuf.h"
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>

using namespace marlindrm;

#define ALIGN_SIZE 4096
#define ALIGNED_DATA_LENGTH(a) (((a) + (ALIGN_SIZE - 1)) & (~(ALIGN_SIZE - 1)))

#ifndef ION_HEAP
#define ION_HEAP(bit) (1 << (bit))
#endif

#ifndef ION_IOMMU_HEAP_ID
#define ION_IOMMU_HEAP_ID 25
#endif

IonBuf::IonBuf() :
    devFd(-1),
    ionFd(-1),
    size(0),
    mem(NULL) {
    ion_handle.handle = 0;
}

IonBuf::~IonBuf() {

    if (devFd >= 0) {
        close(devFd);
    }
}

void* IonBuf::getBuffer(size_t size) {
    if (devFd < 0) {
        devFd = open("/dev/ion", O_RDWR);
    }

    if (devFd >= 0) {
       if (size > this->size) {
           free();

           struct ion_handle_data handle_data;
           struct ion_allocation_data alloc;
           alloc.len = ALIGNED_DATA_LENGTH(size);
           alloc.align = ALIGN_SIZE;
           alloc.heap_mask = ION_HEAP(ION_IOMMU_HEAP_ID);
           alloc.flags = ION_FLAG_CACHED;
           int ret = ioctl(devFd, ION_IOC_ALLOC, &alloc);

           if (ret < 0) {
               ALOGE("ioctl ION_IOC_ALLOC (%dbytes) failed with code %d: %s\n", size, ret,
                       strerror(errno));
               return NULL;
           }

           struct ion_fd_data ion_info_fd;
           ion_info_fd.handle = alloc.handle;
           ret = ioctl(devFd, ION_IOC_SHARE, &ion_info_fd);
           if (ret < 0) {
               handle_data.handle = ion_info_fd.handle;
               ioctl(devFd, ION_IOC_FREE, &handle_data);
               ALOGE("ioctl ION_IOC_SHARE failed with code %d: %s\n", ret, strerror(errno));
               return NULL;
           }

           void *ptr = (unsigned char *)mmap(NULL, alloc.len , PROT_READ | PROT_WRITE, MAP_SHARED,
                   ion_info_fd.fd, 0);
           if (ptr == MAP_FAILED) {
               close(ion_info_fd.fd);
               handle_data.handle = ion_info_fd.handle;
               ioctl(devFd, ION_IOC_FREE, &handle_data);
               ALOGE("mmap failed: %s\n", strerror(errno));
               return NULL;
           }

           ionFd = ion_info_fd.fd;
           mem = ptr;
           this->size = alloc.len;
           ion_handle.handle = ion_info_fd.handle;
           return ptr;
       } else {
           return mem;
       }
    } else {
        ALOGE("open /dev/ion failed!\n");
    }

    return NULL;
}

int IonBuf::getFd() {
    return ionFd;
}

size_t IonBuf::getSize() {
    return size;
}

void IonBuf::free() {
    if (mem != NULL) {
        munmap(mem, size);
        mem = NULL;
    }

    if (devFd >= 0 &&  ion_handle.handle != 0) {
        struct ion_handle_data data;
        data.handle = ion_handle.handle,
        ioctl(devFd, ION_IOC_FREE, &data);
        ion_handle.handle = 0;
    }
    size = 0;
}
