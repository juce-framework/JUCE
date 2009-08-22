/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_RELATIVETIME_JUCEHEADER__
#define __JUCE_RELATIVETIME_JUCEHEADER__

#include "../text/juce_String.h"


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
    explicit RelativeTime (const double seconds = 0.0) throw();

    /** Copies another relative time. */
    RelativeTime (const RelativeTime& other) throw();

    /** Copies another relative time. */
    const RelativeTime& operator= (const RelativeTime& other) throw();

    /** Destructor. */
    ~RelativeTime() throw();

    //==============================================================================
    /** Creates a new RelativeTime object representing a number of milliseconds.

        @see minutes, hours, days, weeks
    */
    static const RelativeTime milliseconds (const int milliseconds) throw();

    /** Creates a new RelativeTime object representing a number of milliseconds.

        @see minutes, hours, days, weeks
    */
    static const RelativeTime milliseconds (const int64 milliseconds) throw();

    /** Creates a new RelativeTime object representing a number of minutes.

        @see milliseconds, hours, days, weeks
    */
    static const RelativeTime minutes (const double numberOfMinutes) throw();

    /** Creates a new RelativeTime object representing a number of hours.

        @see milliseconds, minutes, days, weeks
    */
    static const RelativeTime hours (const double numberOfHours) throw();

    /** Creates a new RelativeTime object representing a number of days.

        @see milliseconds, minutes, hours, weeks
    */
    static const RelativeTime days (const double numberOfDays) throw();

    /** Creates a new RelativeTime object representing a number of weeks.

        @see milliseconds, minutes, hours, days
    */
    static const RelativeTime weeks (const double numberOfWeeks) throw();

    //==============================================================================
    /** Returns the number of milliseconds this time represents.

        @see milliseconds, inSeconds, inMinutes, inHours, inDays, inWeeks
    */
    int64 inMilliseconds() const throw();

    /** Returns the number of seconds this time represents.

        @see inMilliseconds, inMinutes, inHours, inDays, inWeeks
    */
    double inSeconds() const throw()        { return seconds; }

    /** Returns the number of minutes this time represents.

        @see inMilliseconds, inSeconds, inHours, inDays, inWeeks
    */
    double inMinutes() const throw();

    /** Returns the number of hours this time represents.

        @see inMilliseconds, inSeconds, inMinutes, inDays, inWeeks
    */
    double inHours() const throw();

    /** Returns the number of days this time represents.

        @see inMilliseconds, inSeconds, inMinutes, inHours, inWeeks
    */
    double inDays() const throw();

    /** Returns the number of weeks this time represents.

        @see inMilliseconds, inSeconds, inMinutes, inHours, inDays
    */
    double inWeeks() const throw();

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
    const String getDescription (const String& returnValueForZeroTime = JUCE_T("0")) const throw();

    //==============================================================================

    /** Compares two RelativeTimes. */
    bool operator== (const RelativeTime& other) const throw();
    /** Compares two RelativeTimes. */
    bool operator!= (const RelativeTime& other) const throw();

    /** Compares two RelativeTimes. */
    bool operator>  (const RelativeTime& other) const throw();
    /** Compares two RelativeTimes. */
    bool operator<  (const RelativeTime& other) const throw();
    /** Compares two RelativeTimes. */
    bool operator>= (const RelativeTime& other) const throw();
    /** Compares two RelativeTimes. */
    bool operator<= (const RelativeTime& other) const throw();

    //==============================================================================
    /** Adds another RelativeTime to this one and returns the result. */
    const RelativeTime  operator+  (const RelativeTime& timeToAdd) const throw();
    /** Subtracts another RelativeTime from this one and returns the result. */
    const RelativeTime  operator-  (const RelativeTime& timeToSubtract) const throw();

    /** Adds a number of seconds to this RelativeTime and returns the result. */
    const RelativeTime  operator+  (const double secondsToAdd) const throw();
    /** Subtracts a number of seconds from this RelativeTime and returns the result. */
    const RelativeTime  operator-  (const double secondsToSubtract) const throw();

    /** Adds another RelativeTime to this one. */
    const RelativeTime& operator+= (const RelativeTime& timeToAdd) throw();
    /** Subtracts another RelativeTime from this one. */
    const RelativeTime& operator-= (const RelativeTime& timeToSubtract) throw();

    /** Adds a number of seconds to this time. */
    const RelativeTime& operator+= (const double secondsToAdd) throw();

    /** Subtracts a number of seconds from this time. */
    const RelativeTime& operator-= (const double secondsToSubtract) throw();


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    double seconds;
};


#endif   // __JUCE_RELATIVETIME_JUCEHEADER__
