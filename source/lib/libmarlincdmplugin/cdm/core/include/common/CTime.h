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

#ifndef _CTIME_H_
#define _CTIME_H_

#include <time.h>
#include <MarlinCommonTypes.h>

namespace marlincdm {

class CTime {
public:
    /**
     * CTime
     *
     * @return none
     *     none.
     * @note
     *     none.
     */
    CTime() : m_utcTime(0) {
        m_time.tm_sec = 0;         /* seconds */
        m_time.tm_min = 0;         /* minutes */
        m_time.tm_hour = 0;        /* hours */
        m_time.tm_mday = 0;        /* day of the month */
        m_time.tm_mon = 0;         /* month */
        m_time.tm_year = 0;        /* year */
        m_time.tm_wday = 0;        /* day of the week */
        m_time.tm_yday = 0;        /* day in the year */
        m_time.tm_isdst = 0;       /* daylight saving time */

        m_time.tm_gmtoff = 0;      /* Seconds east of UTC.  */
        m_time.tm_zone = NULL;     /* Timezone abbreviation.  */
    }

    /**
     * Copy constructor
     * CTime
     *
     * @param [in] i_rCTime
     * @return none
     *     none.
     * @note
     *     none.
     */
    CTime(const CTime& i_rCTime);

    /**
     * Parameterized constructor for time_t
     * CTime
     *
     * @param [in] i_time
     * @return none
     *     none.
     * @note
     *     none.
     */
    CTime(const time_t i_time);

    /**
     * Parameterized constructor for mCDM_date_t
     *
     * @param [in] i_pDate
     * @return none
     *     none.
     * @note
     *     none.
     */
    CTime(const mcdm_date_t* i_pDate);

    /**
     * operator =
     *
     * @param[in] i_rCTime
     * @return none
     *     none.
     * @note
     *     none.
     */
    CTime& operator = (const CTime& i_rCTime);

    /**
     * ~CTime
     *
     * @return none
     *     none.
     * @note
     *     none.
     */
    ~CTime(){}

     // Overloaded '<' operator
    bool operator < (const CTime& rCTime) const {
        return (m_utcTime < rCTime.m_utcTime) ? true : false;
    }


     // Overloaded '<' operator
    bool operator > (const CTime& rCTime) const {
        return (m_utcTime > rCTime.m_utcTime) ? true : false;
    }


     // Overloaded '<' operator
    bool operator <= (const CTime& rCTime) const {
        return (m_utcTime <= rCTime.m_utcTime) ? true : false;
    }

     // Overloaded '<' operator
    bool operator >= (const CTime& rCTime) const {
        return (m_utcTime >= rCTime.m_utcTime) ? true : false;
    }

    /**
     * Return the year
     * getYear
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getYear() const {
        return m_time.tm_year;
    }

    /**
     * Return the month
     * getMonth
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getMonth() const {
        return m_time.tm_mon;
    }

    /**
     * Return the date
     * getDate
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getDate() const {
        return m_time.tm_mday;
    }

    /**
     * Return the hour
     * getHour
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getHour() const {
        return m_time.tm_hour;
    }

    /**
     * Return the minutes
     * getMinutes
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getMinutes() const {
        return m_time.tm_min;
    }

    /**
     * Return the seconds
     * getSeconds
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getSeconds() const {
        return m_time.tm_sec;
    }

    /**
     * Returns time represented in seconds(Time elapsed since 1970 Jan 1 00:00:00)
     * getUtc
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getUtc() const {
        return m_utcTime;
    }

    /**
     * Return the current system time.
     * getCurrentTime
     *
     * @return int32_t
     *
     * @note
     *     none.
     */
    int32_t getCurrentTime() {
        return time(NULL);
    }

    /**
     * Return time as string where time is represented in milli-seconds.
     * getUtcAsString
     *
     * @return const string
     *
     * @note
     *     none.
     */
    const string getUtcAsString() const;

    /**
     * Reset the CTime class.
     * reset
     *
     * @return reset
     *
     * @note
     *     none.
     */
    void reset();

    /**
     * Return time in GMT format (struct tm)
     * getGmTime
     *
     * @return const struct tm*
     *
     * @note
     *     none.
     */
    const struct tm* getGmTime() const;

private:
    time_t m_utcTime;
    struct tm m_time;
};

} // End namespace marlincdm

#endif // _CTIME_H_
