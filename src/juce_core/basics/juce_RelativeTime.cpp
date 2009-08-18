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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_RelativeTime.h"
#include "../text/juce_LocalisedStrings.h"


//==============================================================================
RelativeTime::RelativeTime (const double seconds_) throw()
    : seconds (seconds_)
{
}

RelativeTime::RelativeTime (const RelativeTime& other) throw()
    : seconds (other.seconds)
{
}

RelativeTime::~RelativeTime() throw()
{
}

//==============================================================================
const RelativeTime RelativeTime::milliseconds (const int milliseconds) throw()
{
    return RelativeTime (milliseconds * 0.001);
}

const RelativeTime RelativeTime::milliseconds (const int64 milliseconds) throw()
{
    return RelativeTime (milliseconds * 0.001);
}

const RelativeTime RelativeTime::minutes (const double numberOfMinutes) throw()
{
    return RelativeTime (numberOfMinutes * 60.0);
}

const RelativeTime RelativeTime::hours (const double numberOfHours) throw()
{
    return RelativeTime (numberOfHours * (60.0 * 60.0));
}

const RelativeTime RelativeTime::days (const double numberOfDays) throw()
{
    return RelativeTime (numberOfDays * (60.0 * 60.0 * 24.0));
}

const RelativeTime RelativeTime::weeks (const double numberOfWeeks) throw()
{
    return RelativeTime (numberOfWeeks * (60.0 * 60.0 * 24.0 * 7.0));
}

//==============================================================================
int64 RelativeTime::inMilliseconds() const throw()
{
    return (int64)(seconds * 1000.0);
}

double RelativeTime::inMinutes() const throw()
{
    return seconds / 60.0;
}

double RelativeTime::inHours() const throw()
{
    return seconds / (60.0 * 60.0);
}

double RelativeTime::inDays() const throw()
{
    return seconds / (60.0 * 60.0 * 24.0);
}

double RelativeTime::inWeeks() const throw()
{
    return seconds / (60.0 * 60.0 * 24.0 * 7.0);
}

const String RelativeTime::getDescription (const String& returnValueForZeroTime) const throw()
{
    if (seconds < 0.001 && seconds > -0.001)
        return returnValueForZeroTime;

    String result;

    if (seconds < 0)
        result = T("-");

    int fieldsShown = 0;
    int n = abs ((int) inWeeks());
    if (n > 0)
    {
        result << n << ((n == 1) ? TRANS(" week ")
                                 : TRANS(" weeks "));
        ++fieldsShown;
    }

    n = abs ((int) inDays()) % 7;
    if (n > 0)
    {
        result << n << ((n == 1) ? TRANS(" day ")
                                 : TRANS(" days "));
        ++fieldsShown;
    }

    if (fieldsShown < 2)
    {
        n = abs ((int) inHours()) % 24;
        if (n > 0)
        {
            result << n << ((n == 1) ? TRANS(" hr ")
                                     : TRANS(" hrs "));
            ++fieldsShown;
        }

        if (fieldsShown < 2)
        {
            n = abs ((int) inMinutes()) % 60;
            if (n > 0)
            {
                result << n << ((n == 1) ? TRANS(" min ")
                                         : TRANS(" mins "));
                ++fieldsShown;
            }

            if (fieldsShown < 2)
            {
                n = abs ((int) inSeconds()) % 60;
                if (n > 0)
                {
                    result << n << ((n == 1) ? TRANS(" sec ")
                                             : TRANS(" secs "));
                    ++fieldsShown;
                }

                if (fieldsShown < 1)
                {
                    n = abs ((int) inMilliseconds()) % 1000;
                    if (n > 0)
                    {
                        result << n << TRANS(" ms");
                        ++fieldsShown;
                    }
                }
            }
        }
    }

    return result.trimEnd();
}

//==============================================================================
const RelativeTime& RelativeTime::operator= (const RelativeTime& other) throw()
{
    seconds = other.seconds;
    return *this;
}

bool RelativeTime::operator== (const RelativeTime& other) const throw()
{
    return seconds == other.seconds;
}

bool RelativeTime::operator!= (const RelativeTime& other) const throw()
{
    return seconds != other.seconds;
}

bool RelativeTime::operator>  (const RelativeTime& other) const throw()
{
    return seconds > other.seconds;
}

bool RelativeTime::operator<  (const RelativeTime& other) const throw()
{
    return seconds < other.seconds;
}

bool RelativeTime::operator>= (const RelativeTime& other) const throw()
{
    return seconds >= other.seconds;
}

bool RelativeTime::operator<= (const RelativeTime& other) const throw()
{
    return seconds <= other.seconds;
}

//==============================================================================
const RelativeTime RelativeTime::operator+ (const RelativeTime& timeToAdd) const throw()
{
    return RelativeTime (seconds + timeToAdd.seconds);
}

const RelativeTime RelativeTime::operator- (const RelativeTime& timeToSubtract) const throw()
{
    return RelativeTime (seconds - timeToSubtract.seconds);
}

const RelativeTime RelativeTime::operator+ (const double secondsToAdd) const throw()
{
    return RelativeTime (seconds + secondsToAdd);
}

const RelativeTime RelativeTime::operator- (const double secondsToSubtract) const throw()
{
    return RelativeTime (seconds - secondsToSubtract);
}

//==============================================================================
const RelativeTime& RelativeTime::operator+= (const RelativeTime& timeToAdd) throw()
{
    seconds += timeToAdd.seconds;
    return *this;
}

const RelativeTime& RelativeTime::operator-= (const RelativeTime& timeToSubtract) throw()
{
    seconds -= timeToSubtract.seconds;
    return *this;
}

const RelativeTime& RelativeTime::operator+= (const double secondsToAdd) throw()
{
    seconds += secondsToAdd;
    return *this;
}

const RelativeTime& RelativeTime::operator-= (const double secondsToSubtract) throw()
{
    seconds -= secondsToSubtract;
    return *this;
}

END_JUCE_NAMESPACE
