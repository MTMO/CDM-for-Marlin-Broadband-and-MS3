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

#define LOG_TAG "MarlinRightsManager"
#include "MarlinLog.h"

#include "MarlinAgentHandler.h"
#include "MarlinCommonConfig.h"
#include "MarlinRightsManager.h"
#include "MarlinConstants.h"

using namespace marlincdm;

const string MarlinRightsManager::TABLE_NAME("marlin_table");
const string MarlinRightsManager::DATABASE_NAME("marlin.db");

MarlinRightsManager *MarlinRightsManager::rightsManager = 0;
CMutex MarlinRightsManager::sMutex;

MarlinRightsManager::MarlinRightsManager() :
    mDatabase(NULL)
{
    MARLINLOG_ENTER();
    string database_path(HOME_DIRECTORY);
    database_path.append("/");
    database_path.append(DATABASE_NAME);
    mDatabase = new RightsDb(database_path);
    mDatabase->open(RightsDb::CREATE_IF_NECESSARY);
    MARLINLOG_EXIT();
}

MarlinRightsManager::~MarlinRightsManager() {
    MARLINLOG_ENTER();
    if (NULL != mDatabase) {
        mDatabase->close();
    }
    delete mDatabase;
    mDatabase = NULL;
    MARLINLOG_EXIT();
}

bool MarlinRightsManager::getLicenseData(const string& in_contentId,
                                         string& out_rights)
{
    MARLINLOG_ENTER();

    string licensePath("");
    string rights("");

    string contentId = splitContentId(in_contentId);
//    contentId = contentId + ":0";

    mRightsMutex.lock();
    licensePath = getStringValue(string("_content_id"), contentId, string("_license_path"));
    mRightsMutex.unlock();
    if (licensePath.empty()) {
        LOGE("Could not get licensePath from ContentId");
        return false;
    }

    mRightsMutex.lock();
    rights = readBytes(licensePath);
    mRightsMutex.unlock();
    if (rights.empty()) {
        LOGE("license is empty");
        return false;
    }
    out_rights = rights;

    MARLINLOG_EXIT();
    return true;
}

bool MarlinRightsManager::getLicensePath(const string& in_contentId,
                                         string& out_rightsPath)
{
    MARLINLOG_ENTER();

    string licensePath("");

    string contentId = splitContentId(in_contentId);
//    contentId = contentId + ":0";

    mRightsMutex.lock();
    licensePath = getStringValue(string("_content_id"), contentId, string("_license_path"));
    mRightsMutex.unlock();
    if (licensePath.empty()) {
        LOGE("Could not get licensePath from ContentId");
        return false;
    }
    out_rightsPath = licensePath;

    MARLINLOG_EXIT();
    return true;
}

bool MarlinRightsManager::store(const string& in_contentId,
                                const string& in_accountId,
                                const string& in_subscriptionId,
                                const string& in_rightsPath)
{
    MARLINLOG_ENTER();
    bool ret = true;
    const int numOfEntries = 5;
    string contentId = splitContentId(in_contentId);
    string contentPath("");

    if (!mDatabase->isTableAlreadyExists(TABLE_NAME)) {
        const string names[numOfEntries] = {string("_content_id"),
                                            string("_license_path"),
                                            string("_content_path"),
                                            string("_account_id"),
                                            string("_subscription_id")};

        const string signatures[numOfEntries] = {string("TEXT NON NULL PRIMARY KEY"),
                                                 string("TEXT DEFAULT NULL"),
                                                 string("TEXT DEFAULT NULL"),
                                                 string("TEXT DEFAULT NULL"),
                                                 string("TEXT DEFAULT NULL")};

        if (true != mDatabase->createTable(&TABLE_NAME, numOfEntries, names, signatures)) {
            LOGE("Failed to create Marlin database Table");
            return false;
        }
    }

    const string add_values[numOfEntries] = {toSqlString(contentId),
                                             toSqlString(in_rightsPath),
                                             toSqlString(contentPath),
                                             toSqlString(in_accountId),
                                             toSqlString(in_subscriptionId)};
    LOGV("MarlinRightsManager::insertValues - contentId: %s", add_values[0].c_str());
    mRightsMutex.lock();
    ret = mDatabase->add(&TABLE_NAME, numOfEntries, add_values);
    mRightsMutex.unlock();
    MARLINLOG_EXIT();
    return ret;
}

bool MarlinRightsManager::remove(const string& in_contentId)
{
    MARLINLOG_ENTER();
    bool ret = true;
    string whereString = string("_content_id = ");
    string contentId = splitContentId(in_contentId);
    whereString += toSqlString(contentId);

    const int noOfArgs = 1;
    const string query[noOfArgs] = { whereString };

    mRightsMutex.lock();
    ret = mDatabase->remove(&TABLE_NAME, noOfArgs, query);
    mRightsMutex.unlock();

    MARLINLOG_EXIT();
    return ret;
}

string MarlinRightsManager::splitContentId(const string& in_contentId)
{
    MARLINLOG_ENTER();
    LOGD("content id=%s", in_contentId.c_str());

    int index = 0;
    string spString("");
    index = in_contentId.find(":", 0);
    if (index < 0) {
        spString = in_contentId;
    } else {
        spString = in_contentId.substr(index + 1);
        while(index >= 0) {
            index = spString.find(":", 0);
            if (index >= 0) {
                spString = spString.substr(index + 1);
            }
        }
    }
    LOGD("split content id=%s", spString.c_str());
    MARLINLOG_EXIT();
    return spString;
}

string MarlinRightsManager::getStringValue(const string& selectionName,
                                           const string& selectionValue,
                                           const string& destColumnName) {
    MARLINLOG_ENTER();
    LOGV("selectionName : %s", selectionName.c_str());
    LOGV("selectionValue : %s", selectionValue.c_str());
    LOGV("destColumnName : %s", destColumnName.c_str());
    string resultString("");

    const int numOfColumns = 1;
    const string columnNames[numOfColumns] = { destColumnName };
    string selection("");
    selection += selectionName;
    selection += " = ";
    selection += toSqlString(selectionValue);
    LOGV("selection : %s", selection.c_str());

    SQLiteQueryResult* result = mDatabase->query(&TABLE_NAME,
                                                 numOfColumns,
                                                 columnNames,
                                                 &selection,
                                                 NULL, NULL, NULL, NULL);
    for (int i = 0 ; i < result->getCount(); i++) {
        string name = result->getColumnName(i);
        if (name == destColumnName) {
            vector<string> values = result->getValues(name);
            for (unsigned int j = 0; j < values.size(); j++) {
                resultString = values[j];
                LOGV("result : %s", resultString.c_str());
            }
        }
    }
    MARLINLOG_EXIT();
    return resultString;
}

string MarlinRightsManager::toSqlString(const string& source)
{
    MARLINLOG_ENTER();
    string sqlString("\'");
    sqlString += source.c_str();
    sqlString += "\'";
    LOGV("sqlString : %s", sqlString.c_str());
    MARLINLOG_EXIT();
    return sqlString;
}

string MarlinRightsManager::readBytes(const string& filePath)
{
    MARLINLOG_ENTER();
    LOGD("filePath=%s", filePath.c_str());
    FILE* file = NULL;
    file = fopen(filePath.c_str(), "r");
    string string("");

    if (NULL != file) {
        int fd = fileno(file);
        struct stat sb;

        if (fstat(fd, &sb) == 0 && sb.st_size > 0) {
            off64_t length = sb.st_size;
            char* bytes = new char[length];
            if (length == read(fd, (void*) bytes, length)) {
                string.append(bytes, length);
            }
            delete[] bytes; bytes = NULL;
        }
        fclose(file);
    } else {
        LOGD("file can not open.");
    }
    MARLINLOG_EXIT();
    return string;
}
