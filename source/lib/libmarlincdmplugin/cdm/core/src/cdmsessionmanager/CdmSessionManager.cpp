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

#define LOG_TAG "CdmSessionManager"
#include "MarlinLog.h"

#include <limits.h>
#include "CdmSessionManager.h"

using namespace marlincdm;

// singleton instance and lock
CdmSessionManager *CdmSessionManager::sInstance = 0;
CMutex CdmSessionManager::sMutex;

CdmSessionManager::CdmSessionManager() : mSessionId(3)
{
    MARLINLOG_ENTER();
}

CdmSessionManager::~CdmSessionManager()
{
    MARLINLOG_ENTER();
}

mcdm_status_t CdmSessionManager::getCdmSessionId(mcdm_SessionId_t &sessionId)
{
    MARLINLOG_ENTER();

    mSessionIdMutex.lock();
    if (mSessionId < 0) {
        //error;
        return ERROR_UNKNOWN;
    }
    if (mSessionId >= 0 && mSessionId <= 2) {
        //error, reserve 0..2
        return ERROR_UNKNOWN;
    }

    if (mSessionId >= LONG_LONG_MAX) {
        mSessionId = 3;
    }
    sessionId.assign((char*)&mSessionId);
    LOGV("Session ID: str[%s] [%lld]",sessionId.c_str(), mSessionId);
    mSessionId++;
    mSessionIdMutex.unlock();

    MARLINLOG_EXIT();
    return OK;
}
