/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

namespace TimeHelpers
{
    static struct tm millisToLocal (const int64 millis) noexcept
    {
        struct tm result;
        const int64 seconds = millis / 1000;

        if (seconds < 86400LL || seconds >= 2145916800LL)
        {
            // use extended maths for dates beyond 1970 to 2037..
            const int timeZoneAdjustment = 31536000 - (int) (Time (1971, 0, 1, 0, 0).toMilliseconds() / 1000);
            const int64 jdm = seconds + timeZoneAdjustment + 210866803200LL;

            const int days = (int) (jdm / 86400LL);
            const int a = 32044 + days;
            const int b = (4 * a + 3) / 146097;
            const int c = a - (b * 146097) / 4;
            const int d = (4 * c + 3) / 1461;
            const int e = c - (d * 1461) / 4;
            const int m = (5 * e + 2) / 153;

            result.tm_mday  = e - (153 * m + 2) / 5 + 1;
            result.tm_mon   = m + 2 - 12 * (m / 10);
            result.tm_year  = b * 100 + d - 6700 + (m / 10);
            result.tm_wday  = (days + 1) % 7;
            result.tm_yday  = -1;

            int t = (int) (jdm % 86400LL);
            result.tm_hour  = t / 3600;
            t %= 3600;
            result.tm_min   = t / 60;
            result.tm_sec   = t % 60;
            result.tm_isdst = -1;
        }
        else
        {
            time_t now = static_cast <time_t> (seconds);

          #if JUCE_WINDOWS
           #ifdef _INC_TIME_INL
            if (now >= 0 && now <= 0x793406fff)
                localtime_s (&result, &now);
            else
                zerostruct (result);
           #else
            result = *localtime (&now);
           #endif
          #else

            localtime_r (&now, &result); // more thread-safe
          #endif
        }

        return result;
    }

    static int extendedModulo (const int64 value, const int modulo) noexcept
    {
        return (int) (value >= 0 ? (value % modulo)
                                 : (value - ((value / modulo) + 1) * modulo));
    }

    static inline String formatString (const String& format, const struct tm* const tm)
    {
       #if JUCE_ANDROID
        typedef CharPointer_UTF8  StringType;
       #elif JUCE_WINDOWS
        typedef CharPointer_UTF16 StringType;
       #else
        typedef CharPointer_UTF32 StringType;
       #endif

        for (size_t bufferSize = 256; ; bufferSize += 256)
        {
            HeapBlock<StringType::CharType> buffer (bufferSize);

           #if JUCE_ANDROID
            const size_t numChars = strftime (buffer, bufferSize - 1, format.toUTF8(), tm);
           #elif JUCE_WINDOWS
            const size_t numChars = wcsftime (buffer, bufferSize - 1, format.toWideCharPointer(), tm);
           #else
            const size_t numChars = wcsftime (buffer, bufferSize - 1, format.toUTF32(), tm);
           #endif

            if (numChars > 0 || format.isEmpty())
                return String (StringType (buffer),
                               StringType (buffer) + (int) numChars);
        }
    }

    static uint32 lastMSCounterValue = 0;
}

//==============================================================================
Time::Time() noexcept
    : millisSinceEpoch (0)
{
}

Time::Time (const Time& other) noexcept
    : millisSinceEpoch (other.millisSinceEpoch)
{
}

Time::Time (const int64 ms) noexcept
    : millisSinceEpoch (ms)
{
}

Time::Time (const int year,
            const int month,
            const int day,
            const int hours,
            const int minutes,
            const int seconds,
            const int milliseconds,
            const bool useLocalTime) noexcept
{
    jassert (year > 100); // year must be a 4-digit version

    if (year < 1971 || year >= 2038 || ! useLocalTime)
    {
        // use extended maths for dates beyond 1970 to 2037..
        const int timeZoneAdjustment = useLocalTime ? (31536000 - (int) (Time (1971, 0, 1, 0, 0).toMilliseconds() / 1000))
                                                    : 0;
        const int a = (13 - month) / 12;
        const int y = year + 4800 - a;
        const int jd = day + (153 * (month + 12 * a - 2) + 2) / 5
                           + (y * 365) + (y /  4) - (y / 100) + (y / 400)
                           - 32045;

        const int64 s = ((int64) jd) * 86400LL - 210866803200LL;

        millisSinceEpoch = 1000 * (s + (hours * 3600 + minutes * 60 + seconds - timeZoneAdjustment))
                             + milliseconds;
    }
    else
    {
        struct tm t;
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
}

Time::~Time() noexcept
{
}

Time& Time::operator= (const Time& other) noexcept
{
    millisSinceEpoch = other.millisSinceEpoch;
    return *this;
}

//==============================================================================
int64 Time::currentTimeMillis() noexcept
{
  #if JUCE_WINDOWS
    struct _timeb t;
   #ifdef _INC_TIME_INL
    _ftime_s (&t);
   #else
    _ftime (&t);
   #endif
    return ((int64) t.time) * 1000 + t.millitm;
  #else
    struct timeval tv;
    gettimeofday (&tv, nullptr);
    return ((int64) tv.tv_sec) * 1000 + tv.tv_usec / 1000;
  #endif
}

Time JUCE_CALLTYPE Time::getCurrentTime() noexcept
{
    return Time (currentTimeMillis());
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept;

uint32 Time::getMillisecondCounter() noexcept
{
    const uint32 now = juce_millisecondsSinceStartup();

    if (now < TimeHelpers::lastMSCounterValue)
    {
        // in multi-threaded apps this might be called concurrently, so
        // make sure that our last counter value only increases and doesn't
        // go backwards..
        if (now < TimeHelpers::lastMSCounterValue - 1000)
            TimeHelpers::lastMSCounterValue = now;
    }
    else
    {
        TimeHelpers::lastMSCounterValue = now;
    }

    return now;
}

uint32 Time::getApproximateMillisecondCounter() noexcept
{
    if (TimeHelpers::lastMSCounterValue == 0)
        getMillisecondCounter();

    return TimeHelpers::lastMSCounterValue;
}

void Time::waitForMillisecondCounter (const uint32 targetTime) noexcept
{
    for (;;)
    {
        const uint32 now = getMillisecondCounter();

        if (now >= targetTime)
            break;

        const int toWait = (int) (targetTime - now);

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
double Time::highResolutionTicksToSeconds (const int64 ticks) noexcept
{
    return ticks / (double) getHighResolutionTicksPerSecond();
}

int64 Time::secondsToHighResolutionTicks (const double seconds) noexcept
{
    return (int64) (seconds * (double) getHighResolutionTicksPerSecond());
}

//==============================================================================
String Time::toString (const bool includeDate,
                       const bool includeTime,
                       const bool includeSeconds,
                       const bool use24HourClock) const noexcept
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
        const int mins = getMinutes();

        result << (use24HourClock ? getHours() : getHoursInAmPmFormat())
               << (mins < 10 ? ":0" : ":") << mins;

        if (includeSeconds)
        {
            const int secs = getSeconds();
            result << (secs < 10 ? ":0" : ":") << secs;
        }

        if (! use24HourClock)
            result << (isAfternoon() ? "pm" : "am");
    }

    return result.trimEnd();
}

String Time::formatted (const String& format) const
{
    struct tm t (TimeHelpers::millisToLocal (millisSinceEpoch));
    return TimeHelpers::formatString (format, &t);
}

//==============================================================================
int Time::getYear() const noexcept          { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_year + 1900; }
int Time::getMonth() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mon; }
int Time::getDayOfYear() const noexcept     { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_yday; }
int Time::getDayOfMonth() const noexcept    { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_mday; }
int Time::getDayOfWeek() const noexcept     { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_wday; }
int Time::getHours() const noexcept         { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_hour; }
int Time::getMinutes() const noexcept       { return TimeHelpers::millisToLocal (millisSinceEpoch).tm_min; }
int Time::getSeconds() const noexcept       { return TimeHelpers::extendedModulo (millisSinceEpoch / 1000, 60); }
int Time::getMilliseconds() const noexcept  { return TimeHelpers::extendedModulo (millisSinceEpoch, 1000); }

int Time::getHoursInAmPmFormat() const noexcept
{
    const int hours = getHours();

    if (hours == 0)  return 12;
    if (hours <= 12) return hours;

    return hours - 12;
}

bool Time::isAfternoon() const noexcept
{
    return getHours() >= 12;
}

bool Time::isDaylightSavingTime() const noexcept
{
    return TimeHelpers::millisToLocal (millisSinceEpoch).tm_isdst != 0;
}

String Time::getTimeZone() const noexcept
{
    String zone[2];

  #if JUCE_WINDOWS
    _tzset();

   #ifdef _INC_TIME_INL
    for (int i = 0; i < 2; ++i)
    {
        char name[128] = { 0 };
        size_t length;
        _get_tzname (&length, name, 127, i);
        zone[i] = name;
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
             && zone[0].containsIgnoreCase ("daylight")
             && zone[0].contains ("GMT"))
            zone[0] = "BST";
    }

    return zone[0].substring (0, 3);
}

String Time::getMonthName (const bool threeLetterVersion) const
{
    return getMonthName (getMonth(), threeLetterVersion);
}

String Time::getWeekdayName (const bool threeLetterVersion) const
{
    return getWeekdayName (getDayOfWeek(), threeLetterVersion);
}

String Time::getMonthName (int monthNumber, const bool threeLetterVersion)
{
    static const char* const shortMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static const char* const longMonthNames[]  = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

    monthNumber %= 12;

    return TRANS (threeLetterVersion ? shortMonthNames [monthNumber]
                                     : longMonthNames [monthNumber]);
}

String Time::getWeekdayName (int day, const bool threeLetterVersion)
{
    static const char* const shortDayNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
    static const char* const longDayNames[]  = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

    day %= 7;

    return TRANS (threeLetterVersion ? shortDayNames [day]
                                     : longDayNames [day]);
}

//==============================================================================
Time& Time::operator+= (RelativeTime delta)           { millisSinceEpoch += delta.inMilliseconds(); return *this; }
Time& Time::operator-= (RelativeTime delta)           { millisSinceEpoch -= delta.inMilliseconds(); return *this; }

Time operator+ (Time time, RelativeTime delta)        { Time t (time); return t += delta; }
Time operator- (Time time, RelativeTime delta)        { Time t (time); return t -= delta; }
Time operator+ (RelativeTime delta, Time time)        { Time t (time); return t += delta; }
const RelativeTime operator- (Time time1, Time time2) { return RelativeTime::milliseconds (time1.toMilliseconds() - time2.toMilliseconds()); }

bool operator== (Time time1, Time time2)      { return time1.toMilliseconds() == time2.toMilliseconds(); }
bool operator!= (Time time1, Time time2)      { return time1.toMilliseconds() != time2.toMilliseconds(); }
bool operator<  (Time time1, Time time2)      { return time1.toMilliseconds() <  time2.toMilliseconds(); }
bool operator>  (Time time1, Time time2)      { return time1.toMilliseconds() >  time2.toMilliseconds(); }
bool operator<= (Time time1, Time time2)      { return time1.toMilliseconds() <= time2.toMilliseconds(); }
bool operator>= (Time time1, Time time2)      { return time1.toMilliseconds() >= time2.toMilliseconds(); }
