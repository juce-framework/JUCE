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

//==============================================================================
class AudioThumbnailComponent    : public Component,
                                   public FileDragAndDropTarget,
                                   public ChangeBroadcaster,
                                   private ChangeListener,
                                   private Timer
{
public:
    AudioThumbnailComponent (AudioDeviceManager& adm, AudioFormatManager& afm)
        : audioDeviceManager (adm),
          thumbnailCache (5),
          thumbnail (128, afm, thumbnailCache)
    {
        thumbnail.addChangeListener (this);
    }

    ~AudioThumbnailComponent()
    {
        thumbnail.removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xff495358));

        g.setColour (Colours::white);

        if (thumbnail.getTotalLength() > 0.0)
        {
            thumbnail.drawChannels (g, getLocalBounds().reduced (2),
                                    0.0, thumbnail.getTotalLength(), 1.0f);

            g.setColour (Colours::black);
            g.fillRect (static_cast<float> (currentPosition * getWidth()), 0.0f,
                        1.0f, static_cast<float> (getHeight()));
        }
        else
        {
            g.drawFittedText ("No audio file loaded.\nDrop a file here or click the \"Load File...\" button.", getLocalBounds(),
                              Justification::centred, 2);
        }
    }

    bool isInterestedInFileDrag (const StringArray&) override          { return true; }
    void filesDropped (const StringArray& files, int, int) override    { loadFile (File (files[0]), true); }

    void setCurrentFile (const File& f)
    {
        if (currentFile == f)
            return;

        loadFile (f);
    }

    File getCurrentFile()    { return currentFile; }

    void setTransportSource (AudioTransportSource* newSource)
    {
        transportSource = newSource;

        struct ResetCallback  : public CallbackMessage
        {
            ResetCallback (AudioThumbnailComponent& o) : owner (o) {}
            void messageCallback() override    { owner.reset(); }

            AudioThumbnailComponent& owner;
        };

        (new ResetCallback (*this))->post();
    }

private:
    AudioDeviceManager& audioDeviceManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    AudioTransportSource* transportSource = nullptr;

    File currentFile;
    double currentPosition = 0.0;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster*) override    { repaint(); }

    void reset()
    {
        currentPosition = 0.0;
        repaint();

        if (transportSource == nullptr)
            stopTimer();
        else
            startTimerHz (25);
    }

    void loadFile (const File& f, bool notify = false)
    {
        if (currentFile == f || ! f.existsAsFile())
            return;

        currentFile = f;
        thumbnail.setSource (new FileInputSource (f));

        if (notify)
            sendChangeMessage();
    }

    void timerCallback() override
    {
        if (transportSource != nullptr)
        {
            currentPosition = transportSource->getCurrentPosition() / thumbnail.getTotalLength();
            repaint();
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (transportSource != nullptr)
        {
            const ScopedLock sl (audioDeviceManager.getAudioCallbackLock());

            transportSource->setPosition ((jmax (static_cast<double> (e.x), 0.0) / getWidth())
                                            * thumbnail.getTotalLength());
        }
    }
};

//==============================================================================
class AudioPlayerHeader     : public Component,
                              private Button::Listener,
                              private ChangeListener,
                              private Value::Listener
{
public:
    AudioPlayerHeader();
    ~AudioPlayerHeader();

    void paint (Graphics&) override;
    void resized() override;

    AudioThumbnailComponent thumbnailComp;

private:
    //==============================================================================
    void buttonClicked (Button*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void valueChanged (Value& value) override;

    //==============================================================================
    TextButton loadButton { "Load File..." }, playButton { "Play" };
    ToggleButton loopButton { "Loop File" };
};

//==============================================================================
class DemoParametersComponent    : public Component
{
public:
    DemoParametersComponent (const std::vector<DSPDemoParameterBase*>& demoParams)
    {
        parameters = demoParams;

        for (auto demoParameter : parameters)
        {
            addAndMakeVisible (demoParameter->getComponent());

            auto* paramLabel = new Label ({}, demoParameter->name);

            paramLabel->attachToComponent (demoParameter->getComponent(), true);
            paramLabel->setJustificationType (Justification::centredLeft);
            addAndMakeVisible (paramLabel);
            labels.add (paramLabel);
        }
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        bounds.removeFromLeft (100);

        for (auto* p : parameters)
        {
            auto* comp = p->getComponent();

            comp->setSize (jmin (bounds.getWidth(), p->getPreferredWidth()), p->getPreferredHeight());

            auto compBounds = bounds.removeFromTop (p->getPreferredHeight());
            comp->setCentrePosition (compBounds.getCentre());
        }
    }

    int getHeightNeeded()
    {
        auto height = 0;

        for (auto* p : parameters)
            height += p->getPreferredHeight();

        return height + 10;
    }

private:
    std::vector<DSPDemoParameterBase*> parameters;
    OwnedArray<Label> labels;
};

//==============================================================================
class MainContentComponent     : public Component,
                                 private ListBoxModel
{
public:
    MainContentComponent();

    void paint (Graphics&) override;
    void resized() override;

    AudioThumbnailComponent& getThumbnailComponent()    { return header.thumbnailComp; }
    void initParameters();

private:
    //==============================================================================
    void paintListBoxItem (int rowNumber, Graphics&, int width, int height, bool rowIsSelected) override;
    int getNumRows() override;
    void selectedRowsChanged (int lastRowSelected) override;

    void setupDemoColours();

    //==============================================================================
    AudioPlayerHeader header;
    ListBox demoList { "Demo List" };

    CPlusPlusCodeTokeniser cppTokeniser;
    CodeDocument codeDocument;
    CodeEditorComponent codeEditor { codeDocument, &cppTokeniser };

    ScopedPointer<DemoParametersComponent> parametersComponent;
};
