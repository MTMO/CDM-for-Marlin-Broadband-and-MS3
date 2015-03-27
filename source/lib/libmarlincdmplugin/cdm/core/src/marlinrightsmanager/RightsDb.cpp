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

#define LOG_TAG "RightsDb"
#include "MarlinLog.h"

#include "RightsDb.h"

using namespace marlincdm;

SQLiteQueryResult* RightsDb::mQueryResult = new SQLiteQueryResult();

bool RightsDb::open(int i_flags) {
    LOGV("Creating or opening database : %s ", mUri.c_str());

    int result = sqlite3_open_v2(
            mUri.c_str(), &mSqliteInstance, convertToSqliteFlags(i_flags), NULL);

    if (SQLITE_OK == result) {
        sqlite3_soft_heap_limit(4 * 1024 * 1024);

        // Set the default busy handler to retry for 1000ms and then return SQLITE_BUSY
        result = sqlite3_busy_timeout(mSqliteInstance, 1000 /* ms */);

        if (SQLITE_OK == result) {
            return true;
        }
    }
    return false;
}

bool RightsDb::isTableAlreadyExists(const string& i_tableName) {
    string i_query("SELECT * FROM ");
    i_query += i_tableName.c_str();
    i_query += " ;";

    bool result = false;

    if (true == execSQL(i_query, 0, 0, 0)) {
        LOGV(" Table: %s  already exists", i_tableName.c_str());
        result = true;
    }
    return result;
}

bool RightsDb::createTable(
        const string* i_tableName, int i_numOfEntries,
        const string* i_columnNames, const string* i_signatures) const {
    string i_query("CREATE TABLE ");

    i_query += i_tableName->c_str();
    i_query += "( ";

    for (int i = 0; i < i_numOfEntries; i++) {
        i_query += i_columnNames[i];
        i_query += " ";
        i_query += i_signatures[i];
        if (i != (i_numOfEntries -1)) {
            i_query += " , ";
        }
    }
    i_query += " );";

    return execSQL(i_query, 0, 0, 0);
}

bool RightsDb::add(
        const string* i_tableName, int i_numOfEntries, const string* values) const {
    LOGV("Enter insert()");

    string i_query("REPLACE INTO ");

    i_query += i_tableName->c_str();
    i_query += " VALUES( ";

    for (int i = 0; i < i_numOfEntries; i++) {
        i_query += values[i];
        if (i != (i_numOfEntries -1)) {
            i_query += " , ";
        }
    }
    i_query += " );";

    return execSQL(i_query, 0, 0, 0);
}

bool RightsDb::query(
        const string* i_tableName, bool i_distinct, int i_numOfColumns,
        const string* i_columns, const string* i_selection, const string* i_groupBy,
        const string* i_having, const string* i_orderBy, const string* i_limit,
        int (*callback_function)(void*, int, char**, char**)) {
    string i_query("SELECT ");

    if (i_distinct) {
        i_query += "DISTINCT ";
    }

    if (NULL != i_columns) {
        // append i_columns required
        for (int i = 0; i < i_numOfColumns; i++) {
            if (i > 0) {
                i_query += ", ";
            }
            i_query += i_columns[i];
        }
    }

    if (0 < i_numOfColumns) {
        i_query += " ";
    } else {
        i_query += "* ";
    }

    i_query += "FROM ";
    i_query += i_tableName->c_str();

    appendClause(&i_query, string(" WHERE "), i_selection);
    appendClause(&i_query, string(" GROUP BY "), i_groupBy);
    appendClause(&i_query, string(" HAVING "), i_having);
    appendClause(&i_query, string(" ORDER BY "), i_orderBy);
    appendClause(&i_query, string(" LIMIT "), i_limit);

    i_query += " ;";

    return execSQL(i_query, callback_function, 0, 0);
}

bool RightsDb::query(
        const string* i_tableName, int i_numOfColumns, const string* i_columns,
        const string* i_selection, const string* i_groupBy, const string* i_having,
        const string* i_orderBy, const string* i_limit,
        int (*callback_function)(void*, int, char**, char**)) {
    if (0 < i_having->size() && 0 >= i_groupBy->size()) {
        // i_having caluse allowed only when we have groupby
    }
    return query(i_tableName, false, i_numOfColumns, i_columns, i_selection,
            i_groupBy, i_having, i_orderBy, i_limit, callback_function);
}

SQLiteQueryResult* RightsDb::query(
        const string* i_tableName, int i_numOfColumns, const string* i_columns,
        const string* i_selection, const string* i_groupBy, const string* i_having,
        const string* i_orderBy, const string* i_limit) {
//    if (0 < i_having->size() && 0 >= i_groupBy->size()) {
        // i_having caluse allowed only when we have groupby
//    }

    if(mQueryResult != NULL) {
        mQueryResult->clear();
    }

    if (true == query(i_tableName, true, i_numOfColumns, i_columns, i_selection,
            i_groupBy, i_having, i_orderBy, i_limit, RightsDb::callback)) {
        mQueryResult->setStatus(true);
    } else {
        mQueryResult->setStatus(false);
    }

    return mQueryResult;
}

bool RightsDb::dropTable(const string& i_tableName) const {
    string i_query("DROP TABLE IF EXISTS ");
    i_query += i_tableName;
    i_query += " ;";

    return execSQL(i_query, 0, 0, 0);
}

bool RightsDb::execSQL(const string& i_sqlString) const {
    return execSQL(i_sqlString, RightsDb::callback, 0, 0);
}

bool RightsDb::remove(
        const string* i_tableName, int i_numOfArgs, const string* i_whereArgs) const {
    string i_query("DELETE FROM ");
    i_query += i_tableName->c_str();

    if (0 < i_numOfArgs) {
        i_query += " WHERE ";
    }

    for (int i = 0; i < i_numOfArgs; i++) {
        i_query += i_whereArgs[i];
    }

    i_query += " ;";

    return execSQL(i_query, 0, 0, 0);
}

bool RightsDb::close() {
    if (NULL != mSqliteInstance) {
        int result = sqlite3_close(mSqliteInstance);

        if (SQLITE_OK == result) {
            LOGV("Closed database: handle = %p\n", mSqliteInstance);
        }
    }
    return true;
}

// private functions

bool RightsDb::execSQL(
        const string& i_sqlString, int (*callback_function)(void*, int, char**, char**),
        void* i_queryDescription, char** i_errMsg) const {
    LOGV("Executing sql");

    int result = sqlite3_exec(mSqliteInstance,      /* handle */
                              i_sqlString.c_str(),   /* sql string */
                              callback_function,    /* callback fucntion */
                              i_queryDescription,     /* type or first argument to callback */
                              i_errMsg);              /* Error message */

    if (SQLITE_OK == result) {
        LOGV("Executed successfully");
        return true;
    } else {
        LOGV("Query Failed : %d", result);
    }
    return false;
}

int RightsDb::callback(void* /*i_queryDescription*/,
                       int i_numOfEntries,
                       char** values,
                       char** i_columnNames) {
    for (int i = 0; i < i_numOfEntries; i++){
        LOGV("%s = %s\n", i_columnNames[i], values[i] ? values[i] : "NULL");
        mQueryResult->addEntry(string(i_columnNames[i]), string(values[i]));
    }
    return SQLITE_OK;
}

void RightsDb::appendClause(
        string* i_query, const string& i_name, const string* i_clause) {
    if (NULL != i_clause) {
        *i_query += i_name.c_str();
        *i_query += i_clause->c_str();
    }
}

int RightsDb::convertToSqliteFlags(int i_flags) const {
    int sqliteFlags;

    // convert our i_flags into the sqlite i_flags
    if (i_flags & CREATE_IF_NECESSARY) {
        sqliteFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    } else {
        sqliteFlags = SQLITE_OPEN_READWRITE;
    }
    return sqliteFlags;
}
