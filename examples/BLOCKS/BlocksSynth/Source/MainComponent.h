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

#include "../JuceLibraryCode/JuceHeader.h"
#include "Audio.h"
#include "WaveshapeProgram.h"

//==============================================================================
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

        for (auto i = 0; i < numRows; ++i)
        {
            for (auto j = 0; j < numColumns; ++j)
            {
                DrumPadGridProgram::GridFill fill;

                auto padNum = (i * 5) + j;

                fill.colour =  notes.contains (padNum) ? baseGridColour
                                                       : tonics.contains (padNum) ? Colours::white
                                                                                  : Colours::black;
                fill.fillType = DrumPadGridProgram::GridFill::FillType::gradient;
                gridFillArray.add (fill);
            }
        }
    }

    int getNoteNumberForPad (int x, int y) const
    {
        auto xIndex = x / 3;
        auto yIndex = y / 3;

        return 60 + ((4 - yIndex) * 5) + xIndex;
    }

    //==============================================================================
    int numColumns, numRows;
    float width, height;

    Array<DrumPadGridProgram::GridFill> gridFillArray;
    Colour baseGridColour = Colours::green;
    Colour touchColour    = Colours::red;

    Array<int> tonics = { 4, 12, 20 };
    Array<int> notes  = { 1, 3, 6, 7, 9, 11, 14, 15, 17, 19, 22, 24 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthGrid)
};

//==============================================================================
/**
    The main component
*/
class MainComponent   : public Component,
                        public TopologySource::Listener,
                        private TouchSurface::Listener,
                        private ControlButton::Listener,
                       #if JUCE_IOS
                        private Button::Listener,
                       #endif
                        private Timer
{
public:
    MainComponent()
    {
        setSize (600, 400);

        // Register MainContentComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

       #if JUCE_IOS
        connectButton.setButtonText ("Connect");
        connectButton.addListener (this);
        addAndMakeVisible (connectButton);
       #endif
    };

    ~MainComponent()
    {
        if (activeBlock != nullptr)
            detachActiveBlock();
    }

    void paint (Graphics& g) override
    {
        g.setColour (getLookAndFeel().findColour (Label::textColourId));
        g.drawText ("Connect a Lightpad Block to play.",
                    getLocalBounds(), Justification::centred, false);
    }

    void resized() override
    {
       #if JUCE_IOS
        connectButton.setBounds (getRight() - 100, 20, 80, 30);
       #endif
    }

    /** Overridden from TopologySource::Listener, called when the topology changes */
    void topologyChanged() override
    {
        // Reset the activeBlock object
        if (activeBlock != nullptr)
            detachActiveBlock();

        // Get the array of currently connected Block objects from the PhysicalTopologySource
        auto blocks = topologySource.getCurrentTopology().blocks;

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

                    setLEDProgram (*activeBlock);
                }

                break;
            }
        }
    }

private:
    /** Overridden from TouchSurface::Listener. Called when a Touch is received on the Lightpad */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& touch) override
    {
        if (currentMode == waveformSelectionMode && touch.isTouchStart && allowTouch)
        {
            if (auto* waveshapeProgram = getWaveshapeProgram())
            {
                // Change the displayed waveshape to the next one
                ++waveshapeMode;

                if (waveshapeMode > 3)
                    waveshapeMode = 0;

                waveshapeProgram->setWaveshapeType (static_cast<uint8> (waveshapeMode));

                allowTouch = false;
                startTimer (250);
            }
        }
        else if (currentMode == playMode)
        {
            if (auto* gridProgram = getGridProgram())
            {
                // Translate X and Y touch events to LED indexes
                auto xLed = roundToInt (touch.startX * scaleX);
                auto yLed = roundToInt (touch.startY * scaleY);

                // Limit the number of touches per second
                constexpr auto maxNumTouchMessagesPerSecond = 100;
                auto now = Time::getCurrentTime();
                clearOldTouchTimes (now);

                auto midiChannel = waveshapeMode + 1;

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

                    gridProgram->sendTouch (touch.x, touch.y, touch.z,
                                            layout.touchColour);

                    // Send pitch change and pressure values to the Audio class
                    audio.pitchChange (midiChannel, (touch.x - touch.startX) / activeBlock->getWidth());
                    audio.pressureChange (midiChannel, touch.z);
                }

                touchMessageTimesInLastSecond.add (now);
            }
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
        setLEDProgram (*activeBlock);
    }

   #if JUCE_IOS
    void buttonClicked (Button* b) override
    {
        if (b == &connectButton)
            BluetoothMidiDevicePairingDialogue::open();
    }
   #endif

    /** Clears the old touch times */
    void clearOldTouchTimes (const Time now)
    {
        for (auto i = touchMessageTimesInLastSecond.size(); --i >= 0;)
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
    void setLEDProgram (Block& block)
    {
        if (currentMode == waveformSelectionMode)
        {
            // Set the LEDGrid program
            block.setProgram (new WaveshapeProgram (block));

            // Initialise the program
            if (auto* waveshapeProgram = getWaveshapeProgram())
            {
                waveshapeProgram->setWaveshapeType (static_cast<uint8> (waveshapeMode));
                waveshapeProgram->generateWaveshapes();
            }
        }
        else if (currentMode == playMode)
        {
            // Set the LEDGrid program
            auto error = block.setProgram (new DrumPadGridProgram (block));

            if (error.failed())
            {
                DBG (error.getErrorMessage());
                jassertfalse;
            }

            // Setup the grid layout
            if (auto* gridProgram = getGridProgram())
                gridProgram->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
        }
    }

    /** Stops touch events from triggering multiple waveshape mode changes */
    void timerCallback() override { allowTouch = true; }

    //==============================================================================
    DrumPadGridProgram* getGridProgram()
    {
        if (activeBlock != nullptr)
            return dynamic_cast<DrumPadGridProgram*> (activeBlock->getProgram());

        return nullptr;
    }

    WaveshapeProgram* getWaveshapeProgram()
    {
        if (activeBlock != nullptr)
            return dynamic_cast<WaveshapeProgram*> (activeBlock->getProgram());

        return nullptr;
    }

    //==============================================================================
    enum BlocksSynthMode
    {
        waveformSelectionMode = 0,
        playMode
    };

    BlocksSynthMode currentMode = playMode;

    //==============================================================================
    Audio audio;

    SynthGrid layout { 5, 5 };
    PhysicalTopologySource topologySource;
    Block::Ptr activeBlock;

    Array<juce::Time> touchMessageTimesInLastSecond;

    int waveshapeMode = 0;

    float scaleX = 0.0;
    float scaleY = 0.0;

    bool allowTouch = true;

    //==============================================================================
   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
