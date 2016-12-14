/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/


BitmapLEDProgram::BitmapLEDProgram (LEDGrid& lg)  : Program (lg) {}

/*
    The heap format for this program is just an array of 15x15 5:6:5 colours,
    and the program just copies them onto the screen each frame.
*/

void BitmapLEDProgram::setLED (uint32 x, uint32 y, LEDColour colour)
{
    auto w = (uint32) ledGrid.getNumColumns();
    auto h = (uint32) ledGrid.getNumRows();

    if (x < w && y < h)
    {
        auto bit = (x + y * w) * 16;

        ledGrid.setDataBits (bit,      5, colour.getRed()   >> 3);
        ledGrid.setDataBits (bit + 5,  6, colour.getGreen() >> 2);
        ledGrid.setDataBits (bit + 11, 5, colour.getBlue()  >> 3);
    }
}

uint32 BitmapLEDProgram::getHeapSize()
{
    return 15 * 15 * 16;
}

juce::String BitmapLEDProgram::getLittleFootProgram()
{
    auto program = R"littlefoot(

    void repaint()
    {
        for (int y = 0; y < NUM_ROWS; ++y)
        {
            for (int x = 0; x < NUM_COLUMNS; ++x)
            {
                int bit = (x + y * NUM_COLUMNS) * 16;

                setLED (x, y, makeARGB (255,
                                        getHeapBits (bit,      5) << 3,
                                        getHeapBits (bit + 5,  6) << 2,
                                        getHeapBits (bit + 11, 5) << 3));
            }
        }
    }

    )littlefoot";

    return juce::String (program)
             .replace ("NUM_COLUMNS", juce::String (ledGrid.getNumColumns()))
             .replace ("NUM_ROWS",    juce::String (ledGrid.getNumRows()));
}
