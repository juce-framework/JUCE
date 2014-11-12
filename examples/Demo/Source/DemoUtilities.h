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

inline void fillTiledBackground (Graphics& g)
{
    g.setFillType (FillType (ImageCache::getFromMemory (BinaryData::tile_background_png,
                                                        BinaryData::tile_background_pngSize),
                             AffineTransform::identity));
    //g.setColour (Colour::greyLevel (0.2f));
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
