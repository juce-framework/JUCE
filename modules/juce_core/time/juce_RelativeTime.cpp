/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

RelativeTime::RelativeTime (const double secs) noexcept           : seconds (secs) {}
RelativeTime::RelativeTime (const RelativeTime& other) noexcept   : seconds (other.seconds) {}
RelativeTime::~RelativeTime() noexcept {}

//==============================================================================
RelativeTime RelativeTime::milliseconds (const int milliseconds) noexcept   { return RelativeTime (milliseconds * 0.001); }
RelativeTime RelativeTime::milliseconds (const int64 milliseconds) noexcept { return RelativeTime (milliseconds * 0.001); }
RelativeTime RelativeTime::minutes (const double numberOfMinutes) noexcept  { return RelativeTime (numberOfMinutes * 60.0); }
RelativeTime RelativeTime::hours (const double numberOfHours) noexcept      { return RelativeTime (numberOfHours * (60.0 * 60.0)); }
RelativeTime RelativeTime::days (const double numberOfDays) noexcept        { return RelativeTime (numberOfDays  * (60.0 * 60.0 * 24.0)); }
RelativeTime RelativeTime::weeks (const double numberOfWeeks) noexcept      { return RelativeTime (numberOfWeeks * (60.0 * 60.0 * 24.0 * 7.0)); }

//==============================================================================
int64 RelativeTime::inMilliseconds() const noexcept { return (int64) (seconds * 1000.0); }
double RelativeTime::inMinutes() const noexcept     { return seconds / 60.0; }
double RelativeTime::inHours() const noexcept       { return seconds / (60.0 * 60.0); }
double RelativeTime::inDays() const noexcept        { return seconds / (60.0 * 60.0 * 24.0); }
double RelativeTime::inWeeks() const noexcept       { return seconds / (60.0 * 60.0 * 24.0 * 7.0); }

//==============================================================================
RelativeTime& RelativeTime::operator= (const RelativeTime& other) noexcept      { seconds = other.seconds; return *this; }

const RelativeTime& RelativeTime::operator+= (const RelativeTime& t) noexcept   { seconds += t.seconds; return *this; }
const RelativeTime& RelativeTime::operator-= (const RelativeTime& t) noexcept   { seconds -= t.seconds; return *this; }
const RelativeTime& RelativeTime::operator+= (const double secs) noexcept       { seconds += secs; return *this; }
const RelativeTime& RelativeTime::operator-= (const double secs) noexcept       { seconds -= secs; return *this; }

RelativeTime operator+ (const RelativeTime& t1, const RelativeTime& t2) noexcept   { RelativeTime t (t1); return t += t2; }
RelativeTime operator- (const RelativeTime& t1, const RelativeTime& t2) noexcept   { RelativeTime t (t1); return t -= t2; }

bool operator== (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() == t2.inSeconds(); }
bool operator!= (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() != t2.inSeconds(); }
bool operator>  (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() >  t2.inSeconds(); }
bool operator<  (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() <  t2.inSeconds(); }
bool operator>= (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() >= t2.inSeconds(); }
bool operator<= (const RelativeTime& t1, const RelativeTime& t2) noexcept       { return t1.inSeconds() <= t2.inSeconds(); }

//==============================================================================
String RelativeTime::getDescription (const String& returnValueForZeroTime) const
{
    if (seconds < 0.001 && seconds > -0.001)
        return returnValueForZeroTime;

    String result;
    result.preallocateBytes (32);

    if (seconds < 0)
        result << '-';

    int fieldsShown = 0;
    int n = std::abs ((int) inWeeks());
    if (n > 0)
    {
        result << n << TRANS (n == 1 ? " week "
                                     : " weeks ");
        ++fieldsShown;
    }

    n = std::abs ((int) inDays()) % 7;
    if (n > 0)
    {
        result << n << TRANS (n == 1 ? " day "
                                     : " days ");
        ++fieldsShown;
    }

    if (fieldsShown < 2)
    {
        n = std::abs ((int) inHours()) % 24;
        if (n > 0)
        {
            result << n << TRANS (n == 1 ? " hr "
                                         : " hrs ");
            ++fieldsShown;
        }

        if (fieldsShown < 2)
        {
            n = std::abs ((int) inMinutes()) % 60;
            if (n > 0)
            {
                result << n << TRANS (n == 1 ? " min "
                                             : " mins ");
                ++fieldsShown;
            }

            if (fieldsShown < 2)
            {
                n = std::abs ((int) inSeconds()) % 60;
                if (n > 0)
                {
                    result << n << TRANS (n == 1 ? " sec "
                                                 : " secs ");
                    ++fieldsShown;
                }

                if (fieldsShown == 0)
                {
                    n = std::abs ((int) inMilliseconds()) % 1000;
                    if (n > 0)
                        result << n << TRANS (" ms");
                }
            }
        }
    }

    return result.trimEnd();
}
