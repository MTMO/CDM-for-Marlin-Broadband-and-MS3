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

#define LOG_TAG "SQLiteQueryResult"
#include "MarlinLog.h"

#include "SQLiteQueryResult.h"

using namespace marlincdm;

int SQLiteQueryResult::getCount() const {
    return mEntriesMap.size();
}

void SQLiteQueryResult::addEntry(const string& i_columnName, const string& i_value) {

    if (mEntriesMap.find(i_columnName) == mEntriesMap.end()) {
        vector<string> i_valVector = mEntriesMap[i_columnName];
        addEntryImp(i_columnName, i_valVector, i_value);
    } else {
        vector<string> i_valVector;
        addEntryImp(i_columnName, i_valVector, i_value);
    }
}

void SQLiteQueryResult::addEntryImp(
        const string& i_columnName, vector<string>& i_valVector, const string& i_value) {
    i_valVector.push_back(i_value);
    mEntriesMap[i_columnName] = i_valVector;
}

string SQLiteQueryResult::getColumnName(int i_index) const {
    string i_columnName("");

    if (0 <= i_index && (size_t)i_index < mEntriesMap.size()) {
        map<string, vector<string> >::const_iterator itr;
        int i = 0;
        for (itr = mEntriesMap.begin(); itr != mEntriesMap.end(); itr++) {
            if(i == i_index) {
                i_columnName = itr->first;
                break;
            }
            i++;
        }
    }
    return i_columnName;
}

vector<string> SQLiteQueryResult::getValues(const string& i_columnName) {
    if (i_columnName != string("")) {
        return mEntriesMap[i_columnName];
    }
    return vector<string>();
}

void SQLiteQueryResult::setStatus(bool i_status) {
    mStatus = i_status;
}

bool SQLiteQueryResult::getStatus() const {
    return mStatus;
}

void SQLiteQueryResult::setDescription(const string& i_description) {
    mDescription = i_description;
}

string SQLiteQueryResult::getDescription() const {
    return mDescription;
}

void SQLiteQueryResult::clear() {
    mStatus = true;
    mDescription = string("");
    mEntriesMap.clear();
}
