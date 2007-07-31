/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifdef _MSC_VER
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif

#include "juce_StandardHeader.h"

#ifndef JUCE_WIN32
  #include <sys/time.h>
#else
  #include <ctime>
#endif

#include <sys/timeb.h>

BEGIN_JUCE_NAMESPACE


#include "juce_Time.h"
#include "../threads/juce_Thread.h"
#include "../containers/juce_MemoryBlock.h"
#include "../text/juce_LocalisedStrings.h"

#ifdef _MSC_VER
  #pragma warning (pop)

  #ifdef _INC_TIME_INL
    #define USE_NEW_SECURE_TIME_FNS
  #endif
#endif

//==============================================================================
static void millisToLocal (const int64 millis, struct tm& result) throw()
{
    time_t now = (time_t) (millis / 1000);

#if JUCE_WIN32
  #ifdef USE_NEW_SECURE_TIME_FNS
    if (now >= 0 && now <= 0x793406fff)
        localtime_s (&result, &now);
    else
        zeromem (&result, sizeof (result));
  #else
    result = *localtime (&now);
  #endif
#else
    // more thread-safe
    localtime_r (&now, &result);
#endif
}


//==============================================================================
Time::Time() throw()
    : millisSinceEpoch (0)
{
}

Time::Time (const Time& other) throw()
    : millisSinceEpoch (other.millisSinceEpoch)
{
}

Time::Time (const int64 ms) throw()
    : millisSinceEpoch (ms)
{
}

Time::Time (const int year,
            const int month,
            const int day,
            const int hours,
            const int minutes,
            const int seconds,
            const int milliseconds) throw()
{
    struct tm t;

    jassert (year > 100); // year must be a 4-digit version

    t.tm_year   = year - 1900;
    t.tm_mon    = month;
    t.tm_mday   = day;
    t.tm_hour   = hours;
    t.tm_min    = minutes;
    t.tm_sec    = seconds;
    t.tm_isdst  = -1;

    millisSinceEpoch = 1000 * (int64) mktime (&t);

    if (millisSinceEpoch < 0)
        millisSinceEpoch = 0;
    else
        millisSinceEpoch += milliseconds;
}

Time::~Time() throw()
{
}

const Time& Time::operator= (const Time& other) throw()
{
    millisSinceEpoch = other.millisSinceEpoch;
    return *this;
}

//==============================================================================
int64 Time::currentTimeMillis() throw()
{
    static uint32 lastCounterResult = 0xffffffff;
    static int64 correction = 0;

    const uint32 now = getMillisecondCounter();

    // check the counter hasn't wrapped (also triggered the first time this function is called)
    if (now < lastCounterResult)
    {
        // double-check it's actually wrapped, in case multi-cpu machines have timers that drift a bit.
        if (lastCounterResult == 0xffffffff || now < lastCounterResult - 10)
        {
            // get the time once using normal library calls, and store the difference needed to
            // turn the millisecond counter into a real time.
#if JUCE_WIN32
            struct _timeb t;
  #ifdef USE_NEW_SECURE_TIME_FNS
            _ftime_s (&t);
  #else
            _ftime (&t);
  #endif
            correction = (((int64) t.time) * 1000 + t.millitm) - now;
#else
            struct timeval tv;
            struct timezone tz;
            gettimeofday (&tv, &tz);
            correction = (((int64) tv.tv_sec) * 1000 + tv.tv_usec / 1000) - now;
#endif
        }
    }

    lastCounterResult = now;

    return correction + now;
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() throw();
static uint32 lastMSCounterValue = 0;

uint32 Time::getMillisecondCounter() throw()
{
    const uint32 now = juce_millisecondsSinceStartup();

    if (now < lastMSCounterValue)
    {
        // in multi-threaded apps this might be called concurrently, so
        // make sure that our last counter value only increases and doesn't
        // go backwards..
        if (now < lastMSCounterValue - 1000)
            lastMSCounterValue = now;
    }
    else
    {
        lastMSCounterValue = now;
    }

    return now;
}

uint32 Time::getApproximateMillisecondCounter() throw()
{
    jassert (lastMSCounterValue != 0);
    return lastMSCounterValue;
}

void Time::waitForMillisecondCounter (const uint32 targetTime) throw()
{
    for (;;)
    {
        const uint32 now = getMillisecondCounter();

        if (now >= targetTime)
            break;

        const int toWait = targetTime - now;

        if (toWait > 2)
        {
            Thread::sleep (jmin (20, toWait >> 1));
        }
        else
        {
            // xxx should consider using mutex_pause on the mac as it apparently
            // makes it seem less like a spinlock and avoids lowering the thread pri.
            for (int i = 10; --i >= 0;)
                Thread::yield();
        }
    }
}

//==============================================================================
double Time::highResolutionTicksToSeconds (const int64 ticks) throw()
{
    return ticks / (double) getHighResolutionTicksPerSecond();
}

int64 Time::secondsToHighResolutionTicks (const double seconds) throw()
{
    return (int64) (seconds * (double) getHighResolutionTicksPerSecond());
}


//==============================================================================
const Time JUCE_CALLTYPE Time::getCurrentTime() throw()
{
    return Time (currentTimeMillis());
}

//==============================================================================
const String Time::toString (const bool includeDate,
                             const bool includeTime,
                             const bool includeSeconds,
                             const bool use24HourClock) const throw()
{
    String result;

    if (includeDate)
    {
        result << getDayOfMonth() << ' '
               << getMonthName (true) << ' '
               << getYear();

        if (includeTime)
            result << ' ';
    }

    if (includeTime)
    {
        if (includeSeconds)
        {
            result += String::formatted (T("%d:%02d:%02d "),
                                         (use24HourClock) ? getHours()
                                                          : getHoursInAmPmFormat(),
                                         getMinutes(),
                                         getSeconds());
        }
        else
        {
            result += String::formatted (T("%d.%02d"),
                                         (use24HourClock) ? getHours()
                                                          : getHoursInAmPmFormat(),
                                         getMinutes());
        }

        if (! use24HourClock)
            result << (isAfternoon() ? "pm" : "am");
    }

    return result.trimEnd();
}

const String Time::formatted (const tchar* const format) const throw()
{
    tchar buffer[80];

    struct tm t;
    millisToLocal (millisSinceEpoch, t);

    if (CharacterFunctions::ftime (buffer, 79, format, &t) <= 0)
    {
        int bufferSize = 128;

        for (;;)
        {
            MemoryBlock mb (bufferSize * sizeof (tchar));
            tchar* const b = (tchar*) mb.getData();

            if (CharacterFunctions::ftime (b, bufferSize, format, &t) > 0)
                return String (b);

            bufferSize += 128;
        }
    }

    return String (buffer);
}

//==============================================================================
int Time::getYear() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_year + 1900;
}

int Time::getMonth() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_mon;
}

int Time::getDayOfMonth() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_mday;
}

int Time::getDayOfWeek() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_wday;
}

int Time::getHours() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_hour;
}

int Time::getHoursInAmPmFormat() const throw()
{
    const int hours = getHours();

    if (hours == 0)
        return 12;
    else if (hours <= 12)
        return hours;
    else
        return hours - 12;
}

bool Time::isAfternoon() const throw()
{
    return getHours() >= 12;
}

int Time::getMinutes() const throw()
{
    return (int) ((millisSinceEpoch / 60000) % 60);
}

int Time::getSeconds() const throw()
{
    return (int) ((millisSinceEpoch / 1000) % 60);
}

int Time::getMilliseconds() const throw()
{
    return (int) (millisSinceEpoch % 1000);
}

bool Time::isDaylightSavingTime() const throw()
{
    struct tm t;
    millisToLocal (millisSinceEpoch, t);
    return t.tm_isdst != 0;
}

const String Time::getTimeZone() const throw()
{
    String zone[2];

#if JUCE_WIN32
    _tzset();

  #ifdef USE_NEW_SECURE_TIME_FNS
    {
        char name [128];
        size_t length;

        for (int i = 0; i < 2; ++i)
        {
            zeromem (name, sizeof (name));
            _get_tzname (&length, name, 127, i);
            zone[i] = name;
        }
    }
  #else
    const char** const zonePtr = (const char**) _tzname;
    zone[0] = zonePtr[0];
    zone[1] = zonePtr[1];
  #endif
#else
    tzset();
    const char** const zonePtr = (const char**) tzname;
    zone[0] = zonePtr[0];
    zone[1] = zonePtr[1];
#endif

    if (isDaylightSavingTime())
    {
        zone[0] = zone[1];

        if (zone[0].length() > 3
             && zone[0].containsIgnoreCase (T("daylight"))
             && zone[0].contains (T("GMT")))
            zone[0] = "BST";
    }

    return zone[0].substring (0, 3);
}

const String Time::getMonthName (const bool threeLetterVersion) const throw()
{
    return getMonthName (getMonth(), threeLetterVersion);
}

const String Time::getWeekdayName (const bool threeLetterVersion) const throw()
{
    return getWeekdayName (getDayOfWeek(), threeLetterVersion);
}

const String Time::getMonthName (int monthNumber,
                                 const bool threeLetterVersion) throw()
{
    const char* const shortMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    const char* const longMonthNames[]  = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

    monthNumber %= 12;

    return TRANS (threeLetterVersion ? shortMonthNames [monthNumber]
                                     : longMonthNames [monthNumber]);
}

const String Time::getWeekdayName (int day,
                                   const bool threeLetterVersion) throw()
{
    const char* const shortDayNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    const char* const longDayNames[]  = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    day %= 7;

    return TRANS (threeLetterVersion ? shortDayNames [day]
                                     : longDayNames [day]);
}

END_JUCE_NAMESPACE
