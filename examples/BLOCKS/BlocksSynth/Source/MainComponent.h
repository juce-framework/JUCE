
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Audio.h"

/**
    A struct that handles the setup and layout of the DrumPadGridProgram
*/
struct SynthGrid
{
    SynthGrid (int cols, int rows)
        : numColumns (cols),
          numRows (rows)
    {
        constructGridFillArray();
    }

    /** Creates a GridFill object for each pad in the grid and sets its colour
        and fill before adding it to an array of GridFill objects
     */
    void constructGridFillArray()
    {
        gridFillArray.clear();

        for (int i = 0; i < numRows; ++i)
        {
            for (int j = 0; j < numColumns; ++j)
            {
                DrumPadGridProgram::GridFill fill;

                int padNum = (i * 5) + j;

                fill.colour =  notes.contains (padNum) ? baseGridColour
                                                       : tonics.contains (padNum) ? Colours::white
                                                                                  : Colours::black;
                fill.fillType = DrumPadGridProgram::GridFill::FillType::gradient;
                gridFillArray.add (fill);
            }
        }
    }

    int getNoteNumberForPad (int x, int y)
    {
        int xIndex = x / 3;
        int yIndex = y / 3;

        return 60 + ((4 - yIndex) * 5) + xIndex;
    }

    //==============================================================================
    int numColumns, numRows;
    float width, height;

    Array<DrumPadGridProgram::GridFill> gridFillArray;
    Colour baseGridColour = Colours::green;
    Colour touchColour = Colours::cyan;

    Array<int> tonics = { 4, 12, 20 };
    Array<int> notes = { 1, 3, 6, 7, 9, 11, 14, 15, 17, 19, 22, 24 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGrid)
};

/**
    The main component
*/
class MainComponent   : public Component,
                        public TopologySource::Listener,
                        private TouchSurface::Listener,
                        private ControlButton::Listener,
                        private Timer
{
public:
    MainComponent() : layout (5, 5)
    {
        setSize (600, 400);

        // Register MainContentComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

        generateWaveshapes();
    };

    ~MainComponent()
    {
        if (activeBlock != nullptr)
            detachActiveBlock();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);
        g.drawText ("Connect a Lightpad Block to play.", getLocalBounds(), Justification::centred, false);
    }

    void resized() override {}

    /** Overridden from TopologySource::Listener, called when the topology changes */
    void topologyChanged() override
    {
        // Reset the activeBlock object
        if (activeBlock != nullptr)
            detachActiveBlock();

        // Get the array of currently connected Block objects from the PhysicalTopologySource
        Block::Array blocks = topologySource.getCurrentTopology().blocks;

        // Iterate over the array of Block objects
        for (auto b : blocks)
        {
            // Find the first Lightpad
            if (b->getType() == Block::Type::lightPadBlock)
            {
                activeBlock = b;

                // Register MainContentComponent as a listener to the touch surface
                if (auto surface = activeBlock->getTouchSurface())
                    surface->addListener (this);

                // Register MainContentComponent as a listener to any buttons
                for (auto button : activeBlock->getButtons())
                    button->addListener (this);

                // Get the LEDGrid object from the Lightpad and set its program to the program for the current mode
                if (auto grid = activeBlock->getLEDGrid())
                {
                    // Work out scale factors to translate X and Y touches to LED indexes
                    scaleX = static_cast<float> (grid->getNumColumns() - 1) / activeBlock->getWidth();
                    scaleY = static_cast<float> (grid->getNumRows() - 1)    / activeBlock->getHeight();

                    setLEDProgram (grid);
                }

                break;
            }
        }
    }

private:
    /** Overridden from TouchSurface::Listener. Called when a Touch is received on the Lightpad */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& touch) override
    {
        if (currentMode == waveformSelectionMode && touch.isTouchStart)
        {
            // Change the displayed waveshape to the next one
            ++waveshapeMode;

            if (waveshapeMode > 3)
                waveshapeMode = 0;
        }
        else if (currentMode == playMode)
        {
            // Translate X and Y touch events to LED indexes
            int xLed = roundToInt (touch.startX * scaleX);
            int yLed = roundToInt (touch.startY * scaleY);

            // Limit the number of touches per second
            constexpr int maxNumTouchMessagesPerSecond = 100;
            auto now = Time::getCurrentTime();
            clearOldTouchTimes (now);

            int midiChannel = waveshapeMode + 1;

            // Send the touch event to the DrumPadGridProgram and Audio class
            if (touch.isTouchStart)
            {
                gridProgram->startTouch (touch.startX, touch.startY);
                audio.noteOn (midiChannel, layout.getNoteNumberForPad (xLed, yLed), touch.z);
            }
            else if (touch.isTouchEnd)
            {
                gridProgram->endTouch (touch.startX, touch.startY);
                audio.noteOff (midiChannel, layout.getNoteNumberForPad (xLed, yLed), 1.0);
            }
            else
            {
                if (touchMessageTimesInLastSecond.size() > maxNumTouchMessagesPerSecond / 3)
                    return;

                gridProgram->sendTouch (touch.x, touch.y, touch.z, layout.touchColour);

                // Send pitch change and pressure values to the Audio class
                audio.pitchChange (midiChannel, (touch.x - touch.startX) / static_cast<float> (activeBlock->getWidth()));
                audio.pressureChange (midiChannel, touch.z);
            }

            touchMessageTimesInLastSecond.add (now);
        }
    }

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is pressed */
    void buttonPressed (ControlButton&, Block::Timestamp) override {}

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is released */
    void buttonReleased (ControlButton&, Block::Timestamp) override
    {
        // Turn any active synthesiser notes off
        audio.allNotesOff();

        // Switch modes
        if (currentMode == waveformSelectionMode)
            currentMode = playMode;
        else if (currentMode == playMode)
            currentMode = waveformSelectionMode;

        // Set the LEDGrid program to the new mode
        setLEDProgram (activeBlock->getLEDGrid());
    }

    void timerCallback() override
    {
        // Clear all LEDs
        for (uint32 x = 0; x < 15; ++x)
            for (uint32 y = 0; y < 15; ++y)
                bitmapProgram->setLED (x, y, Colours::black);

        // Determine which array to use based on waveshapeMode
        int* waveshapeY = nullptr;
        switch (waveshapeMode)
        {
            case 0:
                waveshapeY = sineWaveY;
                break;
            case 1:
                waveshapeY = squareWaveY;
                break;
            case 2:
                waveshapeY = sawWaveY;
                break;
            case 3:
                waveshapeY = triangleWaveY;
                break;
            default:
                break;
        }

        // For each X co-ordinate
        for (uint32 x = 0; x < 15; ++x)
        {
            // Find the corresponding Y co-ordinate for the current waveshape
            int y = waveshapeY[x + yOffset];

            // Draw a vertical line if flag is set or draw an LED circle
            if (y == -1)
            {
                for (uint32 i = 0; i < 15; ++i)
                    drawLEDCircle (x, i);
            }
            else if (x % 2 == 0)
            {
                drawLEDCircle (x, static_cast<uint32> (y));
            }
        }

        // Increment the offset to draw a 'moving' waveshape
        if (++yOffset == 30)
            yOffset -= 30;
    }

    /** Clears the old touch times */
    void clearOldTouchTimes (const Time now)
    {
        for (int i = touchMessageTimesInLastSecond.size(); --i >= 0;)
            if (touchMessageTimesInLastSecond.getReference(i) < now - juce::RelativeTime::seconds (0.33))
                touchMessageTimesInLastSecond.remove (i);
    }

    /** Removes TouchSurface and ControlButton listeners and sets activeBlock to nullptr */
    void detachActiveBlock()
    {
        if (auto surface = activeBlock->getTouchSurface())
            surface->removeListener (this);

        for (auto button : activeBlock->getButtons())
            button->removeListener (this);

        activeBlock = nullptr;
    }

    /** Sets the LEDGrid Program for the selected mode */
    void setLEDProgram (LEDGrid* grid)
    {
        if (currentMode == waveformSelectionMode)
        {
            // Create a new BitmapLEDProgram for the LEDGrid
            bitmapProgram = new BitmapLEDProgram (*grid);

            // Set the LEDGrid program
            grid->setProgram (bitmapProgram);

            // Redraw at 25Hz
            startTimerHz (25);
        }
        else if (currentMode == playMode)
        {
            // Stop the redraw timer
            stopTimer();

            // Create a new DrumPadGridProgram for the LEDGrid
            gridProgram = new DrumPadGridProgram (*grid);

            // Set the LEDGrid program
            grid->setProgram (gridProgram);

            // Setup the grid layout
            gridProgram->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
        }
    }

    /** Generates the X and Y co-ordiantes for 1.5 cycles of each of the 4 waveshapes and stores them in arrays */
    void generateWaveshapes()
    {
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
                    squareWaveY[x] = -1;
                else
                    squareWaveY[x] = 1;
            }
            else
            {
                if (squareWaveY[x - 1] == 1)
                    squareWaveY[x - 1] = -1;

                squareWaveY[x] = 13;
            }

            // Saw wave output, set flags for when vertical line should be drawn
            sawWaveY[x] = 14 - ((x / 2) % 15);
            if (sawWaveY[x] == 0 && sawWaveY[x - 1] != -1)
                sawWaveY[x] = -1;

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
    }

    /** Draws a 'circle' on the Lightpad around an origin co-ordinate */
    void drawLEDCircle (uint32 x0, uint32 y0)
    {
        bitmapProgram->setLED (x0, y0, waveshapeColour);

        const uint32 minLedIndex = 0;
        const uint32 maxLedIndex = 14;

        bitmapProgram->setLED (jmin (x0 + 1, maxLedIndex), y0, waveshapeColour.withBrightness (0.4f));
        bitmapProgram->setLED (jmax (x0 - 1, minLedIndex), y0, waveshapeColour.withBrightness (0.4f));
        bitmapProgram->setLED (x0, jmin (y0 + 1, maxLedIndex), waveshapeColour.withBrightness (0.4f));
        bitmapProgram->setLED (x0, jmax (y0 - 1, minLedIndex), waveshapeColour.withBrightness (0.4f));

        bitmapProgram->setLED (jmin (x0 + 1, maxLedIndex), jmin (y0 + 1, maxLedIndex), waveshapeColour.withBrightness (0.1f));
        bitmapProgram->setLED (jmin (x0 + 1, maxLedIndex), jmax (y0 - 1, minLedIndex), waveshapeColour.withBrightness (0.1f));
        bitmapProgram->setLED (jmax (x0 - 1, minLedIndex), jmin (y0 + 1, maxLedIndex), waveshapeColour.withBrightness (0.1f));
        bitmapProgram->setLED (jmax (x0 - 1, minLedIndex), jmax (y0 - 1, minLedIndex), waveshapeColour.withBrightness (0.1f));
    }

    /**
     enum for the two modes
     */
    enum BlocksSynthMode
    {
        waveformSelectionMode = 0,
        playMode
    };
    BlocksSynthMode currentMode = playMode;

    //==============================================================================
    Audio audio;

    DrumPadGridProgram* gridProgram;
    BitmapLEDProgram* bitmapProgram;

    SynthGrid layout;
    PhysicalTopologySource topologySource;
    Block::Ptr activeBlock;

    Array<juce::Time> touchMessageTimesInLastSecond;

    Colour waveshapeColour = Colours::red;

    int sineWaveY[45];
    int squareWaveY[45];
    int sawWaveY[45];
    int triangleWaveY[45];

    int waveshapeMode = 0;
    uint32 yOffset = 0;

    float scaleX = 0.0;
    float scaleY = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
