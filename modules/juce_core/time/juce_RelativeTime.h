/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_RELATIVETIME_H_INCLUDED
#define JUCE_RELATIVETIME_H_INCLUDED


//==============================================================================
/** A relative measure of time.

    The time is stored as a number of seconds, at double-precision floating
    point accuracy, and may be positive or negative.

    If you need an absolute time, (i.e. a date + time), see the Time class.
*/
class JUCE_API  RelativeTime
{
public:
    //==============================================================================
    /** Creates a RelativeTime.

        @param seconds  the number of seconds, which may be +ve or -ve.
        @see milliseconds, minutes, hours, days, weeks
    */
    explicit RelativeTime (double seconds = 0.0) noexcept;

    /** Copies another relative time. */
    RelativeTime (const RelativeTime& other) noexcept;

    /** Copies another relative time. */
    RelativeTime& operator= (const RelativeTime& other) noexcept;

    /** Destructor. */
    ~RelativeTime() noexcept;

    //==============================================================================
    /** Creates a new RelativeTime object representing a number of milliseconds.
        @see seconds, minutes, hours, days, weeks
    */
    static RelativeTime milliseconds (int milliseconds) noexcept;

    /** Creates a new RelativeTime object representing a number of milliseconds.
        @see seconds, minutes, hours, days, weeks
    */
    static RelativeTime milliseconds (int64 milliseconds) noexcept;

    /** Creates a new RelativeTime object representing a number of seconds.
        @see milliseconds, minutes, hours, days, weeks
    */
    static RelativeTime seconds (double seconds) noexcept;

    /** Creates a new RelativeTime object representing a number of minutes.
        @see milliseconds, hours, days, weeks
    */
    static RelativeTime minutes (double numberOfMinutes) noexcept;

    /** Creates a new RelativeTime object representing a number of hours.
        @see milliseconds, minutes, days, weeks
    */
    static RelativeTime hours (double numberOfHours) noexcept;

    /** Creates a new RelativeTime object representing a number of days.
        @see milliseconds, minutes, hours, weeks
    */
    static RelativeTime days (double numberOfDays) noexcept;

    /** Creates a new RelativeTime object representing a number of weeks.
        @see milliseconds, minutes, hours, days
    */
    static RelativeTime weeks (double numberOfWeeks) noexcept;

    //==============================================================================
    /** Returns the number of milliseconds this time represents.
        @see milliseconds, inSeconds, inMinutes, inHours, inDays, inWeeks
    */
    int64 inMilliseconds() const noexcept;

    /** Returns the number of seconds this time represents.
        @see inMilliseconds, inMinutes, inHours, inDays, inWeeks
    */
    double inSeconds() const noexcept       { return numSeconds; }

    /** Returns the number of minutes this time represents.
        @see inMilliseconds, inSeconds, inHours, inDays, inWeeks
    */
    double inMinutes() const noexcept;

    /** Returns the number of hours this time represents.
        @see inMilliseconds, inSeconds, inMinutes, inDays, inWeeks
    */
    double inHours() const noexcept;

    /** Returns the number of days this time represents.
        @see inMilliseconds, inSeconds, inMinutes, inHours, inWeeks
    */
    double inDays() const noexcept;

    /** Returns the number of weeks this time represents.
        @see inMilliseconds, inSeconds, inMinutes, inHours, inDays
    */
    double inWeeks() const noexcept;

    /** Returns a readable textual description of the time.

        The exact format of the string returned will depend on
        the magnitude of the time - e.g.

        "1 min 4 secs", "1 hr 45 mins", "2 weeks 5 days", "140 ms"

        so that only the two most significant units are printed.

        The returnValueForZeroTime value is the result that is returned if the
        length is zero. Depending on your application you might want to use this
        to return something more relevant like "empty" or "0 secs", etc.

        @see inMilliseconds, inSeconds, inMinutes, inHours, inDays, inWeeks
    */
    String getDescription (const String& returnValueForZeroTime = "0") const;


    //==============================================================================
    /** Adds another RelativeTime to this one. */
    RelativeTime operator+= (RelativeTime timeToAdd) noexcept;
    /** Subtracts another RelativeTime from this one. */
    RelativeTime operator-= (RelativeTime timeToSubtract) noexcept;

    /** Adds a number of seconds to this time. */
    RelativeTime operator+= (double secondsToAdd) noexcept;
    /** Subtracts a number of seconds from this time. */
    RelativeTime operator-= (double secondsToSubtract) noexcept;

private:
    //==============================================================================
    double numSeconds;
};

//==============================================================================
/** Compares two RelativeTimes. */
bool operator== (RelativeTime t1, RelativeTime t2) noexcept;
/** Compares two RelativeTimes. */
bool operator!= (RelativeTime t1, RelativeTime t2) noexcept;
/** Compares two RelativeTimes. */
bool operator>  (RelativeTime t1, RelativeTime t2) noexcept;
/** Compares two RelativeTimes. */
bool operator<  (RelativeTime t1, RelativeTime t2) noexcept;
/** Compares two RelativeTimes. */
bool operator>= (RelativeTime t1, RelativeTime t2) noexcept;
/** Compares two RelativeTimes. */
bool operator<= (RelativeTime t1, RelativeTime t2) noexcept;

//==============================================================================
/** Adds two RelativeTimes together. */
RelativeTime  operator+  (RelativeTime t1, RelativeTime t2) noexcept;
/** Subtracts two RelativeTimes. */
RelativeTime  operator-  (RelativeTime t1, RelativeTime t2) noexcept;



#endif   // JUCE_RELATIVETIME_H_INCLUDED
