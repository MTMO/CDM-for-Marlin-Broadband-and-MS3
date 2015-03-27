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

#ifndef __MARLIN_RIGHTS_MANAGER_H__
#define __MARLIN_RIGHTS_MANAGER_H__

#include <string>
#include <cstring>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "RightsDb.h"
#include "SQLiteQueryResult.h"
#include "CMutex.h"

namespace marlincdm {

class MarlinRightsManager {
public:
    static MarlinRightsManager *rightsManager;
    static CMutex sMutex;

    CMutex mRightsMutex;

    static inline MarlinRightsManager& getMarlinRightsManager() {
        sMutex.lock();
        MarlinRightsManager *instance = rightsManager;
        if (instance == 0) {
            instance = new MarlinRightsManager();
            rightsManager = instance;
        }
        sMutex.unlock();
        return *instance;
    }

    bool getLicenseData(const string& in_contentId,
                        string& out_rights);

    bool getLicensePath(const string& in_contentId,
                        string& out_rightsPath);

    bool store(const string& in_contentId,
               const string& in_accountId,
               const string& in_subscriptionId,
               const string& in_rightsPath);

    bool remove(const string& in_contentId);

    string splitContentId(const string& in_contentId);

private:
    MarlinRightsManager();
    virtual ~MarlinRightsManager();
    RightsDb *mDatabase;

    static const string DATABASE_NAME;
    static const string TABLE_NAME;

    string getStringValue(const string& selectionName,
                               const string& selectionValue,
                               const string& destColumnName);

    string toSqlString(const string& source);

    string readBytes(const string& filePath);

}; // class

}; // namespace marlincdm

#endif /* __MARLIN_RIGHTS_MANAGER_H__ */
