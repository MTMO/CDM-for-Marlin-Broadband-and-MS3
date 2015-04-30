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

#ifndef __MARLIN_CMUTEX_H__
#define __MARLIN_CMUTEX_H__

#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

namespace marlincdm {

class CMutex {
public:
  CMutex();
  ~CMutex();

  /**
   * Get Lock.
   *
   * @return 0        successfully
   * @return -EINVAL  mutex has not been instantiated yet
   * @return -EDEADLK mutex has been locked by other thread
   */
  int32_t lock();

  /**
   * Release Lock.
   */
  void unlock();

  /**
   * Try to get Lock if possible.
   *
   * @return -EINVAL  mutex has not been instantiated yet
   * @return -EBUSY   could not get mutex because mutex is busy
   */
  int32_t tryLock();

private:
  CMutex(const CMutex&);
  CMutex& operator =(const CMutex&);
  pthread_mutex_t mMutex;
};

inline CMutex::CMutex() {
  pthread_mutex_init(&mMutex, NULL);
}
inline CMutex::~CMutex() {
  pthread_mutex_destroy(&mMutex);
}
inline int32_t CMutex::lock() {
  return -pthread_mutex_lock(&mMutex);
}
inline void CMutex::unlock() {
  pthread_mutex_unlock(&mMutex);
}
inline int32_t CMutex::tryLock() {
  return -pthread_mutex_trylock(&mMutex);
}

} // namespace marlincdm

#endif /* __MARLIN_CMUTEX_H__ */
