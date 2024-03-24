/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             SurroundPlugin
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Surround audio plugin.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_plugin_client, juce_audio_processors,
                   juce_audio_utils, juce_core, juce_data_structures,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             AudioProcessor
 mainClass:        SurroundProcessor

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class ProcessorWithLevels : public AudioProcessor,
                            private AsyncUpdater,
                            private Timer
{
public:
    ProcessorWithLevels()
        : AudioProcessor (BusesProperties().withInput  ("Input", AudioChannelSet::stereo())
                                           .withInput  ("Aux", AudioChannelSet::stereo(), false)
                                           .withOutput ("Output", AudioChannelSet::stereo())
                                           .withOutput ("Aux", AudioChannelSet::stereo(), false))
    {
        startTimerHz (60);
        applyBusLayouts (getBusesLayout());
    }

    ~ProcessorWithLevels() override
    {
        stopTimer();
        cancelPendingUpdate();
    }

    void prepareToPlay (double, int) override
    {
        samplesToPlay = (int) getSampleRate();
        reset();
    }

    void processBlock (AudioBuffer<float>&  audio, MidiBuffer&) override { processAudio (audio); }
    void processBlock (AudioBuffer<double>& audio, MidiBuffer&) override { processAudio (audio); }

    void releaseResources() override { reset(); }

    float getLevel (int bus, int channel) const
    {
        return readableLevels[(size_t) getChannelIndexInProcessBlockBuffer (true, bus, channel)];
    }

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto isSetValid = [] (const AudioChannelSet& set)
        {
            return ! set.isDisabled()
                   && ! (set.isDiscreteLayout() && set.getChannelIndexForType (AudioChannelSet::discreteChannel0) == -1);
        };

        return isSetValid (layouts.getMainOutputChannelSet())
               && isSetValid (layouts.getMainInputChannelSet());
    }

    void reset() override
    {
        channelClicked = 0;
        samplesPlayed = samplesToPlay;
    }

    bool applyBusLayouts (const BusesLayout& layouts) final
    {
        // Some very badly-behaved hosts will call this during processing!
        const SpinLock::ScopedLockType lock (levelMutex);

        const auto result = AudioProcessor::applyBusLayouts (layouts);

        size_t numInputChannels = 0;

        for (auto i = 0; i < getBusCount (true); ++i)
            numInputChannels += (size_t) getBus (true, i)->getLastEnabledLayout().size();

        incomingLevels = readableLevels = std::vector<float> (numInputChannels, 0.0f);

        triggerAsyncUpdate();
        return result;
    }

    //==============================================================================
    const String getName() const override                  { return "Surround PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return "None"; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock&) override       {}
    void setStateInformation (const void*, int) override   {}

    void channelButtonClicked (int bus, int channelIndex)
    {
        channelClicked = getChannelIndexInProcessBlockBuffer (false, bus, channelIndex);
        samplesPlayed = 0;
    }

    std::function<void()> updateEditor;

private:
    void handleAsyncUpdate() override
    {
        NullCheckedInvocation::invoke (updateEditor);
    }

    template <typename Float>
    void processAudio (AudioBuffer<Float>& audio)
    {
        {
            SpinLock::ScopedTryLockType lock (levelMutex);

            if (lock.isLocked())
            {
                const auto numInputChannels = (size_t) getTotalNumInputChannels();

                for (size_t i = 0; i < numInputChannels; ++i)
                {
                    const auto minMax = audio.findMinMax ((int) i, 0, audio.getNumSamples());
                    const auto newMax = (float) std::max (std::abs (minMax.getStart()), std::abs (minMax.getEnd()));

                    auto& toUpdate = incomingLevels[i];
                    toUpdate = jmax (toUpdate, newMax);
                }
            }
        }

        audio.clear (0, audio.getNumSamples());

        auto fillSamples = jmin (samplesToPlay - samplesPlayed, audio.getNumSamples());

        if (isPositiveAndBelow (channelClicked, audio.getNumChannels()))
        {
            auto* channelBuffer = audio.getWritePointer (channelClicked);
            auto freq = (float) (440.0 / getSampleRate());

            for (auto i = 0; i < fillSamples; ++i)
                channelBuffer[i] += std::sin (MathConstants<float>::twoPi * freq * (float) samplesPlayed++);
        }
    }

    void timerCallback() override
    {
        const SpinLock::ScopedLockType lock (levelMutex);

        for (size_t i = 0; i < readableLevels.size(); ++i)
            readableLevels[i] = std::max (readableLevels[i] * 0.95f, std::exchange (incomingLevels[i], 0.0f));
    }

    SpinLock levelMutex;
    std::vector<float> incomingLevels;
    std::vector<float> readableLevels;

    int channelClicked;
    int samplesPlayed;
    int samplesToPlay;
};

//==============================================================================
const Colour textColour = Colours::white.withAlpha (0.8f);

inline void drawBackground (Component& comp, Graphics& g)
{
    g.setColour (comp.getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker (0.8f));
    g.fillRoundedRectangle (comp.getLocalBounds().toFloat(), 4.0f);
}

inline void configureLabel (Label& label, const AudioProcessor::Bus* layout)
{
    const auto text = layout != nullptr
                          ? (layout->getName() + ": " + layout->getCurrentLayout().getDescription())
                          : "";
    label.setText (text, dontSendNotification);
    label.setJustificationType (Justification::centred);
    label.setColour (Label::textColourId, textColour);
}

class InputBusViewer final : public Component,
                             private Timer
{
public:
    InputBusViewer (ProcessorWithLevels& proc, int busNumber)
        : processor (proc),
          bus (busNumber)
    {
        configureLabel (layoutName, processor.getBus (true, bus));
        addAndMakeVisible (layoutName);

        startTimerHz (60);
    }

    ~InputBusViewer() override
    {
        stopTimer();
    }

    void paint (Graphics& g) override
    {
        drawBackground (*this, g);

        auto* layout = processor.getBus (true, bus);

        if (layout == nullptr)
            return;

        const auto channelSet = layout->getCurrentLayout();
        const auto numChannels = channelSet.size();

        Grid grid;

        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo (Grid::Fr (1));
        grid.items.insertMultiple (0, GridItem(), numChannels);
        grid.performLayout (getLocalBounds());

        const auto minDb = -50.0f;
        const auto maxDb = 6.0f;

        for (auto i = 0; i < numChannels; ++i)
        {
            g.setColour (Colours::orange.darker());

            const auto levelInDb = Decibels::gainToDecibels (processor.getLevel (bus, i), minDb);
            const auto fractionOfHeight = jmap (levelInDb, minDb, maxDb, 0.0f, 1.0f);
            const auto bounds = grid.items[i].currentBounds;
            const auto trackBounds = bounds.withSizeKeepingCentre (16, bounds.getHeight() - 10).toFloat();
            g.fillRect (trackBounds.withHeight (trackBounds.proportionOfHeight (fractionOfHeight)).withBottomY (trackBounds.getBottom()));

            g.setColour (textColour);

            g.drawText (channelSet.getAbbreviatedChannelTypeName (channelSet.getTypeOfChannel (i)),
                        bounds,
                        Justification::centredBottom);
        }
    }

    void resized() override
    {
        layoutName.setBounds (getLocalBounds().removeFromTop (20));
    }

    int getNumChannels() const
    {
        if (auto* b = processor.getBus (true, bus))
            return b->getCurrentLayout().size();

        return 0;
    }

private:
    void timerCallback() override { repaint(); }

    ProcessorWithLevels& processor;
    int bus = 0;
    Label layoutName;
};

//==============================================================================
class OutputBusViewer final : public Component
{
public:
    OutputBusViewer (ProcessorWithLevels& proc, int busNumber)
        : processor (proc),
          bus (busNumber)
    {
        auto* layout = processor.getBus (false, bus);

        configureLabel (layoutName, layout);
        addAndMakeVisible (layoutName);

        if (layout == nullptr)
            return;

        const auto& channelSet = layout->getCurrentLayout();

        const auto numChannels = channelSet.size();

        for (auto i = 0; i < numChannels; ++i)
        {
            const auto channelName = channelSet.getAbbreviatedChannelTypeName (channelSet.getTypeOfChannel (i));

            channelButtons.emplace_back (channelName, channelName);

            auto& newButton = channelButtons.back();
            newButton.onClick = [&p = processor, b = bus, i] { p.channelButtonClicked (b, i); };
            addAndMakeVisible (newButton);
        }

        resized();
    }

    void paint (Graphics& g) override
    {
        drawBackground (*this, g);
    }

    void resized() override
    {
        auto b = getLocalBounds();

        layoutName.setBounds (b.removeFromBottom (20));

        Grid grid;
        grid.autoFlow = Grid::AutoFlow::column;
        grid.autoColumns = grid.autoRows = Grid::TrackInfo (Grid::Fr (1));

        for (auto& channelButton : channelButtons)
            grid.items.add (GridItem (channelButton));

        grid.performLayout (b.reduced (2));
    }

    int getNumChannels() const
    {
        if (auto* b = processor.getBus (false, bus))
            return b->getCurrentLayout().size();

        return 0;
    }

private:
    ProcessorWithLevels& processor;
    int bus = 0;
    Label layoutName;
    std::list<TextButton> channelButtons;
};

//==============================================================================
class SurroundEditor final : public AudioProcessorEditor
{
public:
    explicit SurroundEditor (ProcessorWithLevels& parent)
        : AudioProcessorEditor (parent),
          customProcessor (parent),
          scopedUpdateEditor (customProcessor.updateEditor, [this] { updateGUI(); })
    {
        updateGUI();
        setResizable (true, true);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        doLayout (inputViewers, r.removeFromTop (proportionOfHeight (0.5f)));
        doLayout (outputViewers, r);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    }

private:
    template <typename Range>
    void doLayout (Range& range, Rectangle<int> bounds) const
    {
        FlexBox fb;

        for (auto& viewer : range)
        {
            if (viewer.getNumChannels() != 0)
            {
                fb.items.add (FlexItem (viewer)
                                  .withFlex ((float) viewer.getNumChannels())
                                  .withMargin (4.0f));
            }
        }

        fb.performLayout (bounds);
    }

    void updateGUI()
    {
        inputViewers.clear();
        outputViewers.clear();

        const auto inputBuses = getAudioProcessor()->getBusCount (true);

        for (auto i = 0; i < inputBuses; ++i)
        {
            inputViewers.emplace_back (customProcessor, i);
            addAndMakeVisible (inputViewers.back());
        }

        const auto outputBuses = getAudioProcessor()->getBusCount (false);

        for (auto i = 0; i < outputBuses; ++i)
        {
            outputViewers.emplace_back (customProcessor, i);
            addAndMakeVisible (outputViewers.back());
        }

        const auto channels = jmax (processor.getTotalNumInputChannels(),
                                    processor.getTotalNumOutputChannels());
        setSize (jmax (150, channels * 40), 200);

        resized();
    }

    ProcessorWithLevels& customProcessor;
    ScopedValueSetter<std::function<void()>> scopedUpdateEditor;
    std::list<InputBusViewer> inputViewers;
    std::list<OutputBusViewer> outputViewers;
};

//==============================================================================
struct SurroundProcessor final : public ProcessorWithLevels
{
    AudioProcessorEditor* createEditor() override { return new SurroundEditor (*this); }
    bool hasEditor() const override               { return true; }
};
