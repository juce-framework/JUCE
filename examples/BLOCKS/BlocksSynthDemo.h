/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             BlocksSynthDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Blocks synthesiser application.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_blocks_basics,
                   juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        BlocksSynthDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    Base class for oscillators
*/
class OscillatorBase   : public SynthesiserVoice
{
public:
    OscillatorBase()
    {
        amplitude.reset (getSampleRate(), 0.1);
        phaseIncrement.reset (getSampleRate(), 0.1);
    }

    void startNote (int midiNoteNumber, float velocity, SynthesiserSound*, int) override
    {
        frequency = MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        phaseIncrement.setValue (((MathConstants<double>::twoPi) * frequency) / sampleRate);
        amplitude.setValue (velocity);

        // Store the initial note and work out the maximum frequency deviations for pitch bend
        initialNote = midiNoteNumber;
        maxFreq = MidiMessage::getMidiNoteInHertz (initialNote + 4) - frequency;
        minFreq = frequency - MidiMessage::getMidiNoteInHertz (initialNote - 4);
    }

    void stopNote (float, bool) override
    {
        clearCurrentNote();
        amplitude.setValue (0.0);
    }

    void pitchWheelMoved (int newValue) override
    {
        // Change the phase increment based on pitch bend amount
        auto frequencyOffset = ((newValue > 0 ? maxFreq : minFreq) * (newValue / 127.0));
        phaseIncrement.setValue (((MathConstants<double>::twoPi) * (frequency + frequencyOffset)) / sampleRate);
    }

    void controllerMoved (int, int) override {}

    void channelPressureChanged (int newChannelPressureValue) override
    {
        // Set the amplitude based on pressure value
        amplitude.setValue (newChannelPressureValue / 127.0);
    }

    void renderNextBlock (AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override
    {
        while (--numSamples >= 0)
        {
            auto output = getSample() * amplitude.getNextValue();

            for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                outputBuffer.addSample (i, startSample, static_cast<float> (output));

            ++startSample;
        }
    }

    /** Returns the next sample */
    double getSample()
    {
        auto output = renderWaveShape (phasePos);

        phasePos += phaseIncrement.getNextValue();

        if (phasePos > MathConstants<double>::twoPi)
            phasePos -= MathConstants<double>::twoPi;

        return output;
    }

    /** Subclasses should override this to say whether they can play the given sound */
    virtual bool canPlaySound (SynthesiserSound*) override = 0;

    /** Subclasses should override this to render a waveshape */
    virtual double renderWaveShape (const double currentPhase) = 0;

private:
    LinearSmoothedValue<double> amplitude, phaseIncrement;

    double frequency = 0.0;
    double phasePos = 0.0;
    double sampleRate = 44100.0;

    int initialNote = 0;
    double maxFreq = 0.0, minFreq = 0.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscillatorBase)
};

//==============================================================================
/**
    Sine sound struct - applies to MIDI channel 1
*/
struct SineSound : public SynthesiserSound
{
    SineSound () {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 1); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineSound)
};

/**
    Sine voice struct that renders a sin waveshape
*/
struct SineVoice : public OscillatorBase
{
    SineVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SineSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return sin (currentPhase); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SineVoice)
};

//==============================================================================
/**
    Square sound struct - applies to MIDI channel 2
*/
struct SquareSound : public SynthesiserSound
{
    SquareSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 2); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareSound)
};

/**
    Square voice struct that renders a square waveshape
*/
struct SquareVoice : public OscillatorBase
{
    SquareVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SquareSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return (currentPhase < MathConstants<double>::pi ? 0.0 : 1.0); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SquareVoice)
};

//==============================================================================
/**
    Sawtooth sound - applies to MIDI channel 3
*/
struct SawSound : public SynthesiserSound
{
    SawSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 3); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawSound)
};

/**
    Sawtooth voice that renders a sawtooth waveshape
*/
struct SawVoice : public OscillatorBase
{
    SawVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<SawSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override { return (1.0 / MathConstants<double>::pi) * currentPhase - 1.0; }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SawVoice)
};

//==============================================================================
/**
    Triangle sound - applies to MIDI channel 4
*/
struct TriangleSound : public SynthesiserSound
{
    TriangleSound() {}

    bool appliesToNote (int) override { return true; }

    bool appliesToChannel (int midiChannel) override { return (midiChannel == 4); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangleSound)
};

/**
    Triangle voice that renders a triangle waveshape
*/
struct TriangleVoice : public OscillatorBase
{
    TriangleVoice() {}

    bool canPlaySound (SynthesiserSound* sound) override { return dynamic_cast<TriangleSound*> (sound) != nullptr; }

    double renderWaveShape (const double currentPhase) override
    {
        return currentPhase < MathConstants<double>::pi ? -1.0 + (2.0 / MathConstants<double>::pi) * currentPhase
                                                        :  3.0 - (2.0 / MathConstants<double>::pi) * currentPhase;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangleVoice)
};

//==============================================================================
/**
    Class to handle the Audio functionality
*/
class Audio : public AudioIODeviceCallback
{
public:
    Audio()
    {
        // Set up the audio device manager
       #ifndef JUCE_DEMO_RUNNER
        audioDeviceManager.initialiseWithDefaultDevices (0, 2);
       #endif

        audioDeviceManager.addAudioCallback (this);

        // Set up the synthesiser and add each of the waveshapes
        synthesiser.clearVoices();
        synthesiser.clearSounds();

        synthesiser.addVoice (new SineVoice());
        synthesiser.addVoice (new SquareVoice());
        synthesiser.addVoice (new SawVoice());
        synthesiser.addVoice (new TriangleVoice());

        synthesiser.addSound (new SineSound());
        synthesiser.addSound (new SquareSound());
        synthesiser.addSound (new SawSound());
        synthesiser.addSound (new TriangleSound());
    }

    ~Audio()
    {
        audioDeviceManager.removeAudioCallback (this);
    }

    /** Audio callback */
    void audioDeviceIOCallback (const float** /*inputChannelData*/, int /*numInputChannels*/,
                                float** outputChannelData, int numOutputChannels, int numSamples) override
    {
        AudioBuffer<float> sampleBuffer (outputChannelData, numOutputChannels, numSamples);
        sampleBuffer.clear();

        synthesiser.renderNextBlock (sampleBuffer, MidiBuffer(), 0, numSamples);
    }

    void audioDeviceAboutToStart (AudioIODevice* device) override
    {
        synthesiser.setCurrentPlaybackSampleRate (device->getCurrentSampleRate());
    }

    void audioDeviceStopped() override {}

    /** Called to turn a synthesiser note on */
    void noteOn (int channel, int noteNum, float velocity)
    {
        synthesiser.noteOn (channel, noteNum, velocity);
    }

    /** Called to turn a synthesiser note off */
    void noteOff (int channel, int noteNum, float velocity)
    {
        synthesiser.noteOff (channel, noteNum, velocity, false);
    }

    /** Called to turn all synthesiser notes off */
    void allNotesOff()
    {
        for (auto i = 1; i < 5; ++i)
            synthesiser.allNotesOff (i, false);
    }

    /** Send pressure change message to synthesiser */
    void pressureChange (int channel, float newPressure)
    {
        synthesiser.handleChannelPressure (channel, static_cast<int> (newPressure * 127));
    }

    /** Send pitch change message to synthesiser */
    void pitchChange (int channel, float pitchChange)
    {
        synthesiser.handlePitchWheel (channel, static_cast<int> (pitchChange * 127));
    }

private:
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif
    Synthesiser synthesiser;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Audio)
};

//==============================================================================
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
        auto phaseInc = (1.0 / 30.0) * MathConstants<double>::twoPi;

        for (auto x = 0; x < 30; ++x)
        {
            // Scale and offset the sin output to the Lightpad display
            auto sineOutput = std::sin (currentPhase);
            sineWaveY[x] = static_cast<uint8> (roundToInt ((sineOutput * 6.5) + 7.0));

            // Square wave output, set flags for when vertical line should be drawn
            if (currentPhase < MathConstants<double>::pi)
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
class BlocksSynthDemo   : public Component,
                          public TopologySource::Listener,
                          private TouchSurface::Listener,
                          private ControlButton::Listener,
                          private Timer
{
public:
    BlocksSynthDemo()
    {
        // Register BlocksSynthDemo as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

       #if JUCE_IOS
        connectButton.setButtonText ("Connect");
        connectButton.onClick = [] { BluetoothMidiDevicePairingDialogue::open(); };
        addAndMakeVisible (connectButton);
       #endif

        setSize (600, 400);

        topologyChanged();
    }

    ~BlocksSynthDemo()
    {
        if (activeBlock != nullptr)
            detachActiveBlock();

        topologySource.removeListener (this);
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

                // Register BlocksSynthDemo as a listener to the touch surface
                if (auto surface = activeBlock->getTouchSurface())
                    surface->addListener (this);

                // Register BlocksSynthDemo as a listener to any buttons
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

    /** Clears the old touch times */
    void clearOldTouchTimes (const Time now)
    {
        for (auto i = touchMessageTimesInLastSecond.size(); --i >= 0;)
            if (touchMessageTimesInLastSecond.getReference(i) < now - RelativeTime::seconds (0.33))
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

    Array<Time> touchMessageTimesInLastSecond;

    int waveshapeMode = 0;

    float scaleX = 0.0f;
    float scaleY = 0.0f;

    bool allowTouch = true;

    //==============================================================================
   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlocksSynthDemo)
};
