/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

BitmapLEDProgram::BitmapLEDProgram (Block& b)  : Program (b) {}

/*
    The heap format for this program is just an array of 15x15 5:6:5 colours,
    and the program just copies them onto the screen each frame.
*/

void BitmapLEDProgram::setLED (uint32 x, uint32 y, LEDColour colour)
{
    if (auto ledGrid = block.getLEDGrid())
    {
        auto w = (uint32) ledGrid->getNumColumns();
        auto h = (uint32) ledGrid->getNumRows();

        if (x < w && y < h)
        {
            auto bit = (x + y * w) * 16;

            block.setDataBits (bit,      5, colour.getRed()   >> 3);
            block.setDataBits (bit + 5,  6, colour.getGreen() >> 2);
            block.setDataBits (bit + 11, 5, colour.getBlue()  >> 3);
        }
    }
    else
    {
        jassertfalse;
    }
}

juce::String BitmapLEDProgram::getLittleFootProgram()
{
    String program (R"littlefoot(

    #heapsize: 15 * 15 * 2

    void repaint()
    {
        for (int y = 0; y < NUM_ROWS; ++y)
        {
            for (int x = 0; x < NUM_COLUMNS; ++x)
            {
                int bit = (x + y * NUM_COLUMNS) * 16;

                fillPixel (makeARGB (255,
                                     getHeapBits (bit,      5) << 3,
                                     getHeapBits (bit + 5,  6) << 2,
                                     getHeapBits (bit + 11, 5) << 3), x, y);
            }
        }
    }

    )littlefoot");

    if (auto ledGrid = block.getLEDGrid())
        return program.replace ("NUM_COLUMNS", juce::String (ledGrid->getNumColumns()))
                      .replace ("NUM_ROWS",    juce::String (ledGrid->getNumRows()));

    jassertfalse;
    return {};
}

} // namespace juce
