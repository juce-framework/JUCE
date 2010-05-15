/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_TICKITERATOR_H_8D744D8A__
#define __JUCER_TICKITERATOR_H_8D744D8A__


//==============================================================================
class TickIterator
{
public:
    TickIterator (const double startValue_, const double endValue_, const double valuePerPixel_,
                  int minPixelsPerTick, int minWidthForLabels)
        : startValue (startValue_),
          endValue (endValue_),
          valuePerPixel (valuePerPixel_)
    {
        tickLevelIndex  = findLevelIndexForValue (valuePerPixel * minPixelsPerTick);
        labelLevelIndex = findLevelIndexForValue (valuePerPixel * minWidthForLabels);

        tickPosition = pixelsToValue (-minWidthForLabels);
        tickPosition = snapValueDown (tickPosition, tickLevelIndex);
    }

    bool getNextTick (float& pixelX, float& tickLength, String& label)
    {
        const double tickUnits = getTickSizes() [tickLevelIndex];
        tickPosition += tickUnits;

        const int totalLevels = getNumTickSizes();
        int highestIndex = tickLevelIndex;

        while (++highestIndex < totalLevels)
        {
            const double ticksAtThisLevel = tickPosition / getTickSizes() [highestIndex];

            if (fabs (ticksAtThisLevel - floor (ticksAtThisLevel + 0.5)) > 0.000001)
                break;
        }

        --highestIndex;

        if (highestIndex >= labelLevelIndex)
            label = getDescriptionOfValue (tickPosition, labelLevelIndex);
        else
            label = String::empty;

        tickLength = (highestIndex + 1 - tickLevelIndex) / (float) (totalLevels + 1 - tickLevelIndex);
        pixelX = valueToPixels (tickPosition);

        return tickPosition < endValue;
    }

private:
    double tickPosition;
    int tickLevelIndex, labelLevelIndex;
    const double startValue, endValue, valuePerPixel;

    static int getNumTickSizes()
    {
        return 10;
    }

    static const double* getTickSizes()
    {
        static const double tickSizes[] = { 1.0, 2.0, 5.0,
                                            10.0, 20.0, 50.0,
                                            100.0, 200.0, 500.0, 1000.0 };
        return tickSizes;
    }

    int findLevelIndexForValue (const double value) const
    {
        int i;
        for (i = 0; i < getNumTickSizes(); ++i)
            if (getTickSizes() [i] >= value)
                break;

        return i;
    }

    double pixelsToValue (int pixels) const
    {
        return startValue + pixels * valuePerPixel;
    }

    float valueToPixels (double value) const
    {
        return (float) ((value - startValue) / valuePerPixel);
    }

    static double snapValueToNearest (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = getTickSizes() [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval + 0.5);
    }

    static double snapValueDown (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = getTickSizes() [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval);
    }

    static inline int roundDoubleToInt (const double value)
    {
        union { int asInt[2]; double asDouble; } n;
        n.asDouble = value + 6755399441055744.0;

    #if TARGET_RT_BIG_ENDIAN
        return n.asInt [1];
    #else
        return n.asInt [0];
    #endif
    }

    static const String getDescriptionOfValue (const double value, const int valueLevelIndex)
    {
        return String (roundToInt (value));
    }

    TickIterator (const TickIterator&);
    TickIterator& operator= (const TickIterator&);
};


#endif  // __JUCER_TICKITERATOR_H_8D744D8A__
