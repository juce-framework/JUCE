/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_DECIBELS_H_INCLUDED
#define JUCE_DECIBELS_H_INCLUDED


//==============================================================================
/**
    This class contains some helpful static methods for dealing with decibel values.
*/
class Decibels
{
public:
    //==============================================================================
    /** Converts a dBFS value to its equivalent gain level.

        A gain of 1.0 = 0 dB, and lower gains map onto negative decibel values. Any
        decibel value lower than minusInfinityDb will return a gain of 0.
    */
    template <typename Type>
    static Type decibelsToGain (const Type decibels,
                                const Type minusInfinityDb = (Type) defaultMinusInfinitydB)
    {
        return decibels > minusInfinityDb ? std::pow ((Type) 10.0, decibels * (Type) 0.05)
                                          : Type();
    }

    /** Converts a gain level into a dBFS value.

        A gain of 1.0 = 0 dB, and lower gains map onto negative decibel values.
        If the gain is 0 (or negative), then the method will return the value
        provided as minusInfinityDb.
    */
    template <typename Type>
    static Type gainToDecibels (const Type gain,
                                const Type minusInfinityDb = (Type) defaultMinusInfinitydB)
    {
        return gain > Type() ? jmax (minusInfinityDb, (Type) std::log10 (gain) * (Type) 20.0)
                             : minusInfinityDb;
    }

    //==============================================================================
    /** Converts a decibel reading to a string, with the 'dB' suffix.
        If the decibel value is lower than minusInfinityDb, the return value will
        be "-INF dB".
    */
    template <typename Type>
    static String toString (const Type decibels,
                            const int decimalPlaces = 2,
                            const Type minusInfinityDb = (Type) defaultMinusInfinitydB)
    {
        String s;

        if (decibels <= minusInfinityDb)
        {
            s = "-INF dB";
        }
        else
        {
            if (decibels >= Type())
                s << '+';

            s << String (decibels, decimalPlaces) << " dB";
        }

        return s;
    }


private:
    //==============================================================================
    enum
    {
        defaultMinusInfinitydB = -100
    };

    Decibels(); // This class can't be instantiated, it's just a holder for static methods..
    JUCE_DECLARE_NON_COPYABLE (Decibels)
};


#endif   // JUCE_DECIBELS_H_INCLUDED
