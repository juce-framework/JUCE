/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

/**
    A Program to draw moving waveshapes onto the LEDGrid
*/
class WaveshapeProgram : public Block::Program
{
public:
    WaveshapeProgram (Block& b) : Program (b) {}

    /** Sets the waveshape type to display on the grid */
    void setWaveshapeType (uint8 type)
    {
        block.setDataByte (0, type);
    }

    /** Generates the Y coordinates for 1.5 cycles of each of the four waveshapes and stores them
        at the correct offsets in the shared data heap. */
    void generateWaveshapes()
    {
        uint8 sineWaveY[45];
        uint8 squareWaveY[45];
        uint8 sawWaveY[45];
        uint8 triangleWaveY[45];

        // Set current phase position to 0 and work out the required phase increment for one cycle
        auto currentPhase = 0.0;
        auto phaseInc = (1.0 / 30.0) * (2.0 * double_Pi);

        for (auto x = 0; x < 30; ++x)
        {
            // Scale and offset the sin output to the Lightpad display
            auto sineOutput = std::sin (currentPhase);
            sineWaveY[x] = static_cast<uint8> (roundToInt ((sineOutput * 6.5) + 7.0));

            // Square wave output, set flags for when vertical line should be drawn
            if (currentPhase < double_Pi)
            {
                if (x == 0)
                    squareWaveY[x] = 255;
                else
                    squareWaveY[x] = 1;
            }
            else
            {
                if (squareWaveY[x - 1] == 1)
                    squareWaveY[x - 1] = 255;

                squareWaveY[x] = 13;
            }

            // Saw wave output, set flags for when vertical line should be drawn
            sawWaveY[x] = 14 - ((x / 2) % 15);

            if (sawWaveY[x] == 0 && sawWaveY[x - 1] != 255)
                sawWaveY[x] = 255;

            // Triangle wave output
            triangleWaveY[x] = x < 15 ? static_cast<uint8> (x) : static_cast<uint8> (14 - (x % 15));

            // Add half cycle to end of array so it loops correctly
            if (x < 15)
            {
                sineWaveY[x + 30] = sineWaveY[x];
                squareWaveY[x + 30] = squareWaveY[x];
                sawWaveY[x + 30] = sawWaveY[x];
                triangleWaveY[x + 30] = triangleWaveY[x];
            }

            // Increment the current phase
            currentPhase += phaseInc;
        }

        // Store the values for each of the waveshapes at the correct offsets in the shared data heap
        for (uint8 i = 0; i < 45; ++i)
        {
            block.setDataByte (sineWaveOffset     + i, sineWaveY[i]);
            block.setDataByte (squareWaveOffset   + i, squareWaveY[i]);
            block.setDataByte (sawWaveOffset      + i, sawWaveY[i]);
            block.setDataByte (triangleWaveOffset + i, triangleWaveY[i]);
        }
    }

    String getLittleFootProgram() override
    {
        return R"littlefoot(

        #heapsize: 256

        int yOffset;

        void drawLEDCircle (int x0, int y0)
        {
            blendPixel (0xffff0000, x0, y0);

            int minLedIndex = 0;
            int maxLedIndex = 14;

            blendPixel (0xff660000, min (x0 + 1, maxLedIndex), y0);
            blendPixel (0xff660000, max (x0 - 1, minLedIndex), y0);
            blendPixel (0xff660000, x0, min (y0 + 1, maxLedIndex));
            blendPixel (0xff660000, x0, max (y0 - 1, minLedIndex));

            blendPixel (0xff1a0000, min (x0 + 1, maxLedIndex), min (y0 + 1, maxLedIndex));
            blendPixel (0xff1a0000, min (x0 + 1, maxLedIndex), max (y0 - 1, minLedIndex));
            blendPixel (0xff1a0000, max (x0 - 1, minLedIndex), min (y0 + 1, maxLedIndex));
            blendPixel (0xff1a0000, max (x0 - 1, minLedIndex), max (y0 - 1, minLedIndex));
        }

        void repaint()
        {
            // Clear LEDs to black
            fillRect (0xff000000, 0, 0, 15, 15);

            // Get the waveshape type
            int type = getHeapByte (0);

            // Calculate the heap offset
            int offset = 1 + (type * 45) + yOffset;

            for (int x = 0; x < 15; ++x)
            {
                // Get the corresponding Y coordinate for each X coordinate
                int y = getHeapByte (offset + x);

                // Draw a vertical line if flag is set or draw an LED circle
                if (y == 255)
                {
                    for (int i = 0; i < 15; ++i)
                        drawLEDCircle (x, i);
                }
                else if (x % 2 == 0)
                {
                    drawLEDCircle (x, y);
                }
            }

            // Increment and wrap the Y offset to draw a 'moving' waveshape
            if (++yOffset == 30)
                yOffset = 0;
        }

        )littlefoot";
    }

private:
    //==============================================================================
    /** Shared data heap is laid out as below. There is room for the waveshape type and
        the Y coordinates for 1.5 cycles of each of the four waveshapes. */

    static constexpr uint32 waveshapeType      = 0;   // 1 byte
    static constexpr uint32 sineWaveOffset     = 1;   // 1 byte * 45
    static constexpr uint32 squareWaveOffset   = 46;  // 1 byte * 45
    static constexpr uint32 sawWaveOffset      = 91;  // 1 byte * 45
    static constexpr uint32 triangleWaveOffset = 136; // 1 byte * 45

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveshapeProgram)
};
