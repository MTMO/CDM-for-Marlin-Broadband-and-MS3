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

#ifndef __SQLITE_QUERY_RESULT_H__
#define __SQLITE_QUERY_RESULT_H__

#include <string>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

namespace marlincdm {

class SQLiteQueryResult {
public:
    /**
     * SQLiteQueryResult
     *
     * @return none
     *     none.
     * @note
     *     none.
     */
    SQLiteQueryResult() {
        mStatus = true;
    }

    /**
     * ~SQLiteQueryResult
     *
     * @return virtual
     *
     * @note
     *     none.
     */
    virtual ~SQLiteQueryResult() { }

public:
    /**
     * clear
     *
     * @return void
     *
     * @note
     *     none.
     */
    void clear();

    /**
     * getCount
     *
     * @return int
     *
     * @note
     *     none.
     */
    int getCount() const;

    /**
     * getStatus
     *
     * @return bool
     *
     * @note
     *     none.
     */
    bool getStatus() const;

    /**
     * getDescription
     *
     * @return string
     *
     * @note
     *     none.
     */
    string getDescription() const;

    /**
     * setStatus
     *
     * @param [in] i_status
     * @return void
     *
     * @note
     *     none.
     */
    void setStatus(bool i_status);

    /**
     * getColumnName
     *
     * @param [in] i_index
     * @return string
     *
     * @note
     *     none.
     */
    string getColumnName(int i_index) const;

    /**
     * setDescription
     *
     * @param [in] i_description
     * @return void
     *
     * @note
     *     none.
     */
    void setDescription(const string& i_description);

    /**
     * getValues
     *
     * @param [in] i_columnName
     * @return vector<string>
     *
     * @note
     *     none.
     */
    vector<string> getValues(const string& i_columnName);

    /**
     * addEntry
     *
     * @param [in] i_columnName
     * @param [in] i_value
     * @return void
     *
     * @note
     *     none.
     */
    void addEntry(const string& i_columnName, const string& i_value);

private:
    /**
     * addEntryImp
     *
     * @param [in] i_columnName
     * @param [in] i_valVector
     * @param [in] i_value
     * @return void
     *
     * @note
     *     none.
     */
    void addEntryImp(const string& i_columnName,
                     vector<string>& i_valVector,
                     const string& i_value);
    bool mStatus;
    string mDescription;
    map<string, vector<string> > mEntriesMap;
};

}; // namespace marlincdm

#endif /* __SQLITE_QUERY_RESULT_H__ */
