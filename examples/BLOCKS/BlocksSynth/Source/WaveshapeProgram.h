/*
  ==============================================================================

    WaveshapeProgram.h
    Created: 12 Nov 2016 9:28:17am
    Author:  Edward Davies

  ==============================================================================
*/

#ifndef WAVESHAPEPROGRAM_H_INCLUDED
#define WAVESHAPEPROGRAM_H_INCLUDED

class WaveshapeProgram : public LEDGrid::Program
{
public:
    WaveshapeProgram (LEDGrid& lg, int waveshape) : Program (lg)
    {
        setWaveshapeType (waveshape);
        generateWaveshapes();
    }

    void generateWaveshapes()
    {
        int sineWaveY[45];
        int squareWaveY[45];
        int sawWaveY[45];
        int triangleWaveY[45];
        
        // Set current phase position to 0 and work out the required phase increment for one cycle
        double currentPhase = 0.0;
        double phaseInc = (1.0 / 30.0) * (2.0 * double_Pi);
        
        for (int x = 0; x < 30; ++x)
        {
            // Scale and offset the sin output to the Lightpad display
            double sineOutput = sin (currentPhase);
            sineWaveY[x] = roundToInt ((sineOutput * 6.5) + 7.0);
            
            // Square wave output, set flags for when vertical line should be drawn
            if (currentPhase < double_Pi)
            {
                if (x == 0)
                    squareWaveY[x] = 20;
                else
                    squareWaveY[x] = 1;
            }
            else
            {
                if (squareWaveY[x - 1] == 1)
                    squareWaveY[x - 1] = 20;
                
                squareWaveY[x] = 13;
            }
            
            // Saw wave output, set flags for when vertical line should be drawn
            sawWaveY[x] = 14 - ((x / 2) % 15);
            
            if (sawWaveY[x] == 0 && sawWaveY[x - 1] != 20)
                sawWaveY[x] = 20;
            
            // Triangle wave output
            triangleWaveY[x] = x < 15 ? x : 14 - (x % 15);
            
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
        
        for (int i = 0; i < 45; ++i)
        {
            ledGrid.setDataBits ((sineWaveOffset * 8)     + (32 * i), 32, sineWaveY[i]);
            jassert (ledGrid.getDataByte (sineWaveOffset + (i * 4)) == sineWaveY[i]);
            
            ledGrid.setDataBits ((squareWaveOffset * 8)   + (32 * i), 32, squareWaveY[i]);
            jassert (ledGrid.getDataByte (squareWaveOffset + (i * 4)) == squareWaveY[i]);
            
            ledGrid.setDataBits ((sawWaveOffset * 8)      + (32 * i), 32, sawWaveY[i]);
            jassert (ledGrid.getDataByte (sawWaveOffset + (i * 4)) == sawWaveY[i]);
            
            ledGrid.setDataBits ((triangleWaveOffset * 8) + (32 * i), 32, triangleWaveY[i]);
            jassert (ledGrid.getDataByte (triangleWaveOffset + (i * 4)) == triangleWaveY[i]);
        }
    }
    
    void setWaveshapeType (int type)
    {
        ledGrid.setDataBits (0, 4, type);
    }

    String getLittleFootProgram() override
    {
        return R"littlefoot(

        int yOffset;
        
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
        
        void drawLEDCircle (int x0, int y0)
        {
            setLED (x0, y0, 0xffffffff);
            
            int minLedIndex = 0;
            int maxLedIndex = 14;

            setLED (min (x0 + 1, maxLedIndex), y0, 0xff660000);
            setLED (max (x0 - 1, minLedIndex), y0, 0xff660000);
            setLED (x0, min (y0 + 1, maxLedIndex), 0xff660000);
            setLED (x0, max (y0 - 1, minLedIndex), 0xff660000);

            setLED (min (x0 + 1, maxLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000);
            setLED (min (x0 + 1, maxLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000);
            setLED (max (x0 - 1, minLedIndex), min (y0 + 1, maxLedIndex), 0xff1a0000);
            setLED (max (x0 - 1, minLedIndex), max (y0 - 1, minLedIndex), 0xff1a0000);
        }
        
        void repaint()
        {
            // Clear LEDs to black
            fillRect (0xff000000, 0, 0, 15, 15);

            // Get the waveshape type
            int type = getHeapInt (0);
            int offset = 4 + (type * 180) + yOffset;
            
            for (int x = 0; x < 15; ++x)
            {
                // Find the corresponding Y co-ordinate for the current waveshape
                int y = getHeapInt (offset + (x * 4));

                // Draw a vertical line if flag is set or draw an LED circle
                if (y == 20)
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

        )littlefoot";
    }

    uint32 getHeapSize() override
    {
        return totalDataSize;
    }

private:
    static constexpr uint32 waveshapeType      = 0;   // 4 bytes
    static constexpr uint32 sineWaveOffset     = 4;   // 4 byte * 45
    static constexpr uint32 squareWaveOffset   = 184; // 4 byte * 45
    static constexpr uint32 sawWaveOffset      = 364; // 4 byte * 45
    static constexpr uint32 triangleWaveOffset = 544; // 4 byte * 45

    static constexpr uint32 totalDataSize = triangleWaveOffset + (4 * 45);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveshapeProgram)
};

#endif  // WAVESHAPEPROGRAM_H_INCLUDED
