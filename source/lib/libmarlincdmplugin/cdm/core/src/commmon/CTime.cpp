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

#define LOG_TAG "CTime"
#include "MarlinLog.h"

#include <CTime.h>
#include "MarlinCommonTypes.h"
#include "MarlinCdmUtils.h"

using namespace marlincdm;

const int32_t MAX_LENGTH_OF_FROMATTED_DATE = 14;

CTime::CTime(const CTime& i_rCTime) {
    m_utcTime = i_rCTime.m_utcTime;

    m_time.tm_year  = i_rCTime.m_time.tm_year;
    m_time.tm_mon   = i_rCTime.m_time.tm_mon;
    m_time.tm_mday  = i_rCTime.m_time.tm_mday;
    m_time.tm_hour  = i_rCTime.m_time.tm_hour;
    m_time.tm_min   = i_rCTime.m_time.tm_min;
    m_time.tm_sec   = i_rCTime.m_time.tm_sec;
    m_time.tm_isdst = i_rCTime.m_time.tm_isdst;

    m_time.tm_gmtoff = i_rCTime.m_time.tm_gmtoff;   /* not used.  */
    m_time.tm_wday   = i_rCTime.m_time.tm_wday;     /* not used.  */
    m_time.tm_yday   = i_rCTime.m_time.tm_yday;     /* not used.  */
    m_time.tm_zone   = i_rCTime.m_time.tm_zone;     /* not used.  */

}

CTime::CTime(const time_t i_time) {
    m_utcTime = i_time;
    gmtime_r(&m_utcTime, &m_time);
}

CTime::CTime(const mcdm_date_t* i_pDate) {
    // Year is calculated since 1900
    m_time.tm_year = ((i_pDate->year) - 1900);

    // struct tm consider month range from 0(Jan) - 11(Dec)
    m_time.tm_mon  = ((i_pDate->mon) - 1);
    m_time.tm_mday = i_pDate->mday;
    m_time.tm_hour = i_pDate->hour;
    m_time.tm_min  = i_pDate->min;
    m_time.tm_sec  = i_pDate->sec;

    m_time.tm_gmtoff = 0;     /* not used.  */
    m_time.tm_zone   = NULL;  /* not used.  */

    // Daylight saving i_time is not in effect.
    m_time.tm_isdst = 0;

    // TODO: mki_time() must be replaced with i_timegm()
    // Currently i_timegm() is not supported with the normal android build.
    char *tz;
    tz = getenv("TZ");
    setenv("TZ", "UTC", 1);
    tzset();
    m_utcTime = mktime(&m_time);
    if (tz) {
        setenv("TZ", tz, 1);
    } else {
        unsetenv("TZ");
    }
    tzset();
}

CTime& CTime::operator=(const CTime& i_rCTime) {
    m_utcTime = i_rCTime.m_utcTime;

    m_time.tm_year  = i_rCTime.m_time.tm_year;
    m_time.tm_mon   = i_rCTime.m_time.tm_mon;
    m_time.tm_mday  = i_rCTime.m_time.tm_mday;
    m_time.tm_hour  = i_rCTime.m_time.tm_hour;
    m_time.tm_min   = i_rCTime.m_time.tm_min;
    m_time.tm_sec   = i_rCTime.m_time.tm_sec;
    m_time.tm_isdst = i_rCTime.m_time.tm_isdst;

    return (*this);
}

const string CTime::getUtcAsString() const {
    string dateTime("");
    char formatedDate[MAX_LENGTH_OF_FROMATTED_DATE] = "";
    if (0 != m_utcTime) {
        // Converting seconds into milli-seconds [1 sec = 1000 msec].
        SNPRINTF(formatedDate, MAX_LENGTH_OF_FROMATTED_DATE, "%10ld000", m_utcTime);
        dateTime = string(formatedDate);
    }
    return dateTime;
}

void CTime::reset() {
    m_utcTime = 0;

    m_time.tm_year  = 0;
    m_time.tm_mon   = 0;
    m_time.tm_mday  = 0;
    m_time.tm_hour  = 0;
    m_time.tm_min   = 0;
    m_time.tm_sec   = 0;
    m_time.tm_isdst = 0;
}

const struct tm* CTime::getGmTime() const {
    return (&m_time);
}
