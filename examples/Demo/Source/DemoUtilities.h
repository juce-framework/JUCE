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

#ifndef DEMOUTILITIES_H_INCLUDED
#define DEMOUTILITIES_H_INCLUDED

//==============================================================================
/*
    This file contains a bunch of miscellaneous utilities that are
    used by the various demos.
*/

//==============================================================================
inline Colour getRandomColour (float brightness)
{
    return Colour::fromHSV (Random::getSystemRandom().nextFloat(), 0.5f, brightness, 1.0f);
}

inline Colour getRandomBrightColour()   { return getRandomColour (0.8f); }
inline Colour getRandomDarkColour()     { return getRandomColour (0.3f); }

inline void fillStandardDemoBackground (Graphics& g)
{
    g.setColour (Colour (0xff4d4d4d));
    g.fillAll();
}

//==============================================================================
// This is basically a sawtooth wave generator - maps a value that bounces between
// 0.0 and 1.0 at a random speed
struct BouncingNumber
{
    BouncingNumber()
        : speed (0.0004 + 0.0007 * Random::getSystemRandom().nextDouble()),
          phase (Random::getSystemRandom().nextDouble())
    {
    }

    float getValue() const
    {
        double v = fmod (phase + speed * Time::getMillisecondCounterHiRes(), 2.0);
        return (float) (v >= 1.0 ? (2.0 - v) : v);
    }

protected:
    double speed, phase;
};

struct SlowerBouncingNumber  : public BouncingNumber
{
    SlowerBouncingNumber()
    {
        speed *= 0.3;
    }
};


#endif  // DEMOUTILITIES_H_INCLUDED
