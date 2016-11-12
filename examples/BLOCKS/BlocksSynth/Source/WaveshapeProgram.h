/*
  ==============================================================================

    WaveshapeProgram.h
    Created: 12 Nov 2016 9:28:17am
    Author:  Edward Davies

  ==============================================================================
*/

#ifndef WAVESHAPEPROGRAM_H_INCLUDED
#define WAVESHAPEPROGRAM_H_INCLUDED

class WaveshapeProgram : LEDGrid::Program
{
public:
    WaveshapeProgram (LEDGrid& lg) : Program (lg)
    {

    }

    void generateWaveshapes()
    {

    }

    void setWaveshapeType (uint8 type)
    {
        ledGrid.setDataByte (0, type);
    }

    String getLittleFootProgram() override
    {
        return R"littlefoot(

        int yOffset

        void repaint()
        {
            // Clear LEDs to black
            fillRect (0xff000000, 0, 0, 15, 15);

            // Get the waveshape type
            int type = getHeapByte (0);
            int offset = 1 + (type * 180) + yOffset;

            for (int x = 0; x < 15; ++x)
            {
                // Find the corresponding Y co-ordinate for the current waveshape
                int y = getHeapInt (offset);

                // Draw a vertical line if flag is set or draw an LED circle
                if (y == -1)
                {
                    for (int i = 0; i < 15; ++i)
                        drawLEDCircle (x, i);
                }
                else if (x % 2 == 0)
                {
                    drawLEDCircle (x, y);
                }
            }

            // Increment the offset to draw a 'moving' waveshape
            yOffset += 4;
            if (yOffset == (4 * 30))
                yOffset -= (4 * 30);
        }

        void drawLEDCircle (int x0, int y0)
        {
            setLED (x0, y0, 0xffff0000);

            int minLedIndex = 0;
            int maxLedIndex = 14;

            setLED (min (x0 + 1, maxLedIndex), y0, 0xff660000);
            setLED (max (x0 - 1, minLedIndex), y0, 0xff660000);
            setLED (x0, min (y0 + 1, maxLedIndex), 0xff660000);
            setLED (x0, max (y0 - 1, minLedIndex), 0xff660000);

            setLED (min (x0 + 1, maxLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000));
            setLED (min (x0 + 1, maxLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000));
            setLED (max (x0 - 1, minLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000);
            setLED (max (x0 - 1, minLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000);
        }

        int min (int a, int b)
        {
            if (a > b)
                return b;

            return a;
        }

        int max (int a, int b)
        {
            if (a > b)
                return a;

            return b;
        }

        )littlefoot";
    }

    uint32 getHeapSize() override
    {
        return totalDataSize;
    }

private:
    static constexpr uint32 waveshapeType      = 0;   // 1 byte
    static constexpr uint32 sineWaveOffset     = 1;   // 4 byte * 45
    static constexpr uint32 squareWaveOffset   = 181; // 4 byte * 45
    static constexpr uint32 sawWaveOffset      = 361; // 4 byte * 45
    static constexpr uint32 triangleWaveOffset = 541; // 4 byte * 45

    static constexpr uint32 totalDataSize = triangleWaveOffset + (4 * 45);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveshapeProgram)
};

#endif  // WAVESHAPEPROGRAM_H_INCLUDED
