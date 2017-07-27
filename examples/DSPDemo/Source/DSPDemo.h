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

using namespace dsp;

//==============================================================================
struct DSPDemoParameterBase    : public ChangeBroadcaster
{
    DSPDemoParameterBase (const String& labelName) : name (labelName) {}
    virtual ~DSPDemoParameterBase() {}

    virtual Component* getComponent() = 0;

    virtual int getPreferredHeight()  = 0;
    virtual int getPreferredWidth()   = 0;

    String name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DSPDemoParameterBase)
};

//==============================================================================
struct SliderParameter   : public DSPDemoParameterBase,
                           private Slider::Listener
{
    SliderParameter (Range<double> range, double skew, double initialValue,
                     const String& labelName, const String& suffix = {})
        : DSPDemoParameterBase (labelName)
    {
        slider.setRange (range.getStart(), range.getEnd(), 0.01);
        slider.setSkewFactor (skew);
        slider.setValue (initialValue);

        if (suffix.isNotEmpty())
            slider.setTextValueSuffix (suffix);

        slider.addListener (this);
    }

    Component* getComponent() override    { return &slider; }

    int getPreferredHeight() override     { return 40; }
    int getPreferredWidth()  override     { return 500; }

    double getCurrentValue() const        { return slider.getValue(); }

private:
    Slider slider;

    void sliderValueChanged (Slider*) override    { sendChangeMessage(); }
};

//==============================================================================
struct ChoiceParameter    : public DSPDemoParameterBase,
                            private ComboBox::Listener
{
    ChoiceParameter (const StringArray& options, int initialId, const String& labelName)
        : DSPDemoParameterBase (labelName)
    {
        parameterBox.addItemList (options, 1);
        parameterBox.addListener (this);

        parameterBox.setSelectedId (initialId);
    }

    Component* getComponent() override    { return &parameterBox; }

    int getPreferredHeight() override     { return 25; }
    int getPreferredWidth()  override     { return 250; }

    int getCurrentSelectedID() const      { return parameterBox.getSelectedId(); }

private:
    ComboBox parameterBox;

    void comboBoxChanged (ComboBox*) override    { sendChangeMessage(); }
};

//==============================================================================
// This is just a base class for the demos which exposes them as an AudioSource with
// an array of parameters
struct DSPDemoBase   : public AudioSource
{
    virtual const std::vector<DSPDemoParameterBase*>& getParameters() = 0;

    AudioSource* inputSource = nullptr;
};

//==============================================================================
template <typename DemoType>
struct DSPDemo  : public DSPDemoBase,
                  public ProcessorWrapper<DemoType>,
                  private ChangeListener
{
    DSPDemo()
    {
        for (auto* p : getParameters())
            p->addChangeListener (this);
    }

    void prepareToPlay (int blockSize, double sampleRate) override
    {
        inputSource->prepareToPlay (blockSize, sampleRate);
        this->prepare ({ sampleRate, (uint32) blockSize, 2 });
    }

    void releaseResources() override
    {
        inputSource->releaseResources();
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        jassert (bufferToFill.buffer != nullptr);

        inputSource->getNextAudioBlock (bufferToFill);

        dsp::AudioBlock<float> block (*bufferToFill.buffer,
                                      (size_t) bufferToFill.startSample);

        ScopedLock audioLock (audioCallbackLock);
        this->process (ProcessContextReplacing<float> (block));
    }

    const std::vector<DSPDemoParameterBase*>& getParameters() override
    {
        return this->processor.parameters;
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        ScopedLock audioLock (audioCallbackLock);
        static_cast<DemoType&> (this->processor).updateParameters();
    }

    CriticalSection audioCallbackLock;
};


//==============================================================================
struct Demo
{
    using CreateDemoFn = std::function<DSPDemoBase*(AudioSource&)>;

    String name, code;
    CreateDemoFn createDemo;

    Demo (const char* nameToUse, const char* codeToUse, CreateDemoFn create)
        : name (nameToUse), code (codeToUse), createDemo (create)
    {
        code = code.fromFirstOccurrenceOf ("// @@ START_DEMO", false, false)
                   .upToLastOccurrenceOf  ("// @@ END_DEMO",   false, false)
                   .trim();

        getList().add (this);
    }

    static Array<const Demo*>& getList()
    {
        static Array<const Demo*> demos;
        return demos;
    }
};

template <typename DemoType>
struct RegisterDSPDemo  : public Demo
{
    RegisterDSPDemo (const char* nameToUse, const char* codeToUse)
        : Demo (nameToUse, codeToUse, [](AudioSource& input) { auto* demo = new DSPDemo<DemoType>(); demo->inputSource = &input; return demo; })
    {}
};
