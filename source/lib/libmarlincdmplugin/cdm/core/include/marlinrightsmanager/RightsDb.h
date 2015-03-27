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

#ifndef __RIGHTS_DB_H__
#define __RIGHTS_DB_H__

#include <sqlite3.h>
#include <SQLiteQueryResult.h>
#include <string>

namespace marlincdm {

class RightsDb {
public:
    static const int OPEN_READWRITE = 0x00000001;
    static const int CREATE_IF_NECESSARY = 0x00000002;

private:
    static SQLiteQueryResult* mQueryResult;

public:
    /**
     * RightsDb
     *
     * @param [in] i_uri
     * @return none
     *     none.
     * @note
     *     none.
     */
    RightsDb(const string& i_uri) :
        mUri(i_uri) {
        mSqliteInstance = NULL;
    }

    /**
     * ~RightsDb
     *
     * @return virtual
     *
     * @note
     *     none.
     */
    virtual ~RightsDb() {
        //delete mQueryResult; mQueryResult = NULL;
    }

public:
    /**
     * open
     *
     * @param [in] i_flags
     * @return bool
     *
     * @note
     *     none.
     */
    bool open(int i_flags);

    /**
     * isTableAlreadyExists
     *
     * @param [in] i_tableName
     * @return bool
     *
     * @note
     *     none.
     */
    bool isTableAlreadyExists(const string& i_tableName);

    /**
     * createTable
     *
     * @param [in] i_tableName
     * @param [in] i_numOfEntries
     * @param [in] i_columnNames
     * @param [in] i_signatures
     * @return bool
     *
     * @note
     *     none.
     */
    bool createTable(const string* i_tableName, int i_numOfEntries,
                     const string* i_columnNames, const string* i_signatures) const;

    /**
     * add
     *
     * @param [in] i_tableName
     * @param [in] i_numOfEntries
     * @param [in] i_values
     * @return bool
     *
     * @note
     *     none.
     */
    bool add(const string* i_tableName,
             int i_numOfEntries,
             const string* i_values) const;

    /**
     * query
     *
     * @param [in] i_tableName
     * @param [in] i_distinct
     * @param [in] i_numOfColumns
     * @param [in] i_columns
     * @param [in] i_selection
     * @param [in] i_groupBy
     * @param [in] i_having
     * @param [in] i_orderBy
     * @param [in] i_limit
     * @return bool
     *
     * @note
     *     none.
     */
    bool query(const string* i_tableName,
               bool i_distinct,
               int i_numOfColumns,
               const string* i_columns,
               const string* i_selection,
               const string* i_groupBy,
               const string* i_having,
               const string* i_orderBy,
               const string* i_limit,
               int (*callback_function)(void*, int, char**, char**));

    /**
     * query
     *
     * @param [in] tableName
     * @param [in] numOfColumns
     * @param [in] columns
     * @param [in] selection
     * @param [in] groupBy
     * @param [in] having
     * @param [in] orderBy
     * @param [in] limit
     * @return bool
     *
     * @note
     *     none.
     */
    bool query(const string* i_tableName,
               int i_numOfColumns,
               const string* i_columns,
               const string* i_selection,
               const string* i_groupBy,
               const string* i_having,
               const string* i_orderBy,
               const string* i_limit,
               int (*callback_function)(void*, int, char**, char**));

    /**
     * query
     *
     * @param [in] i_tableName
     * @param [in] i_numOfColumns
     * @param [in] i_columns
     * @param [in] i_selection
     * @param [in] i_groupBy
     * @param [in] i_having
     * @param [in] i_orderBy
     * @param [in] i_limit
     * @return SQLiteQueryResult*
     *
     * @note
     *     none.
     */
    SQLiteQueryResult* query(const string* i_tableName,
                             int i_numOfColumns,
                             const string* i_columns,
                             const string* i_selection,
                             const string* i_groupBy,
                             const string* i_having,
                             const string* i_orderBy,
                             const string* i_limit);
    /**
     * remove
     *
     * @param [in] i_tableName
     * @param [in] i_noOfArgs
     * @param [in] i_whereArgs
     * @return bool
     *
     * @note
     *     none.
     */
    bool remove(const string* i_tableName,
                int i_noOfArgs,
                const string* i_whereArgs) const;

    /**
     * dropTable
     *
     * @param [in] i_tableName
     * @return bool
     *
     * @note
     *     none.
     */
    bool dropTable(const string& i_tableName) const;

    /**
     * close
     *
     * @return bool
     *
     * @note
     *     none.
     */
    bool close();

private:
    /**
     * execSQL
     *
     * @param [in] i_sqlString
     * @return bool
     *
     * @note
     *     none.
     */
    bool execSQL(const string& i_sqlString) const;

    /**
     * callback
     *
     * @param [in] i_queryDescription
     * @param [in] i_numOfEntries
     * @param [in] i_values
     * @param [in] i_columnNames
     * @return static int
     *
     * @note
     *     none.
     */
    static int callback(void* i_queryDescription,
                        int i_numOfEntries,
                        char** i_values,
                        char** i_columnNames);

    /**
     * execSQL
     *
     * @param [in] i_sqlString
     * @param [in] i_description
     * @param [in] i_errMsg
     * @return bool
     *
     * @note
     *     none.
     */
    bool execSQL(const string& i_sqlString,
                 int (*callback_function)(void*, int, char**, char**),
                 void* description, char** i_errMsg) const;

    /**
     * appendClause
     *
     * @param [in] i_query
     * @param [in] i_name
     * @param [in] i_clause
     * @return void
     *
     * @note
     *     none.
     */
    void appendClause(string* i_query,
                      const string& i_name,
                      const string* i_clause);
    /**
     * convertToSqliteFlags
     *
     * @param [in] i_flags
     * @return int
     *
     * @note
     *     none.
     */
    int convertToSqliteFlags(int i_flags) const;

private:
    string mUri;
    sqlite3* mSqliteInstance;
};

};  //namespace

#endif /* __RIGHTS_DB_H__ */
