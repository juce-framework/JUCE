/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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
