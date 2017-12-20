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

#include "Main.h"
#include "MainComponent.h"

//==============================================================================
AudioPlayerHeader::AudioPlayerHeader()
    : thumbnailComp (DSPSamplesApplication::getApp().getDeviceManager(),
                     DSPSamplesApplication::getApp().getFormatManager())
{
    setOpaque (true);

    addAndMakeVisible (loadButton);
    addAndMakeVisible (playButton);
    addAndMakeVisible (loopButton);

    loadButton.addListener (this);
    playButton.addListener (this);

    addAndMakeVisible (thumbnailComp);
    thumbnailComp.addChangeListener (this);

    DSPSamplesApplication::getApp().getPlayState().addListener (this);
    loopButton.getToggleStateValue().referTo (DSPSamplesApplication::getApp().getLoopState());
}

AudioPlayerHeader::~AudioPlayerHeader()
{
    playButton.removeListener (this);
    loadButton.removeListener (this);
    loopButton.removeListener (this);

    DSPSamplesApplication::getApp().getPlayState().removeListener (this);
}

void AudioPlayerHeader::paint (Graphics& g)
{
    g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
    g.fillRect (getLocalBounds());
}

void AudioPlayerHeader::resized()
{
    auto bounds = getLocalBounds();

    auto buttonBounds = bounds.removeFromLeft (jmin (250, bounds.getWidth() / 4));
    auto top = buttonBounds.removeFromTop (40);

    loadButton.setBounds (top.removeFromLeft (buttonBounds.getWidth() / 2).reduced (10, 10));
    playButton.setBounds (top.reduced (10, 10));

    loopButton.setSize (0, 25);
    loopButton.changeWidthToFitText();
    loopButton.setCentrePosition (buttonBounds.getCentre());

    thumbnailComp.setBounds (bounds);
}

void AudioPlayerHeader::buttonClicked (Button* button)
{
    auto& app = DSPSamplesApplication::getApp();

    if (button == &loadButton)
    {
        app.stop();

        FileChooser fc ("Select an audio file...", File(), "*.wav;*.mp3;*.aif;");

        if (fc.browseForFileToOpen())
        {
            auto f = fc.getResult();

            if (! app.loadFile (f))
                NativeMessageBox::showOkCancelBox (AlertWindow::WarningIcon, "Error loading file", "Unable to load audio file", nullptr, nullptr);
            else
                thumbnailComp.setCurrentFile (f);
        }
    }
    else if (button == &playButton)
    {
        app.togglePlay();
    }
}

void AudioPlayerHeader::changeListenerCallback (ChangeBroadcaster*)
{
    auto& app = DSPSamplesApplication::getApp();

    if (app.getPlayState().getValue())
        app.stop();

    app.loadFile (thumbnailComp.getCurrentFile());
}

void AudioPlayerHeader::valueChanged (Value& v)
{
    playButton.setButtonText (v.getValue() ? "Stop" : "Play");
}

//==============================================================================
MainContentComponent::MainContentComponent()
{
    setSize (1000, 800);
    setOpaque (true);

    codeEditor.setEnabled (false);

    auto currentDemoIndex = DSPSamplesApplication::getApp().getCurrentDemoIndex();
    demoList.setModel (this);
    demoList.updateContent();
    demoList.selectRow (currentDemoIndex);

    addAndMakeVisible (header);
    addAndMakeVisible (demoList);
    addAndMakeVisible (codeEditor);

    setupDemoColours();
}

void MainContentComponent::paint (Graphics& g)
{
    g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.fillRect (getLocalBounds());
}

void MainContentComponent::resized()
{
    auto r = getLocalBounds();
    auto listWidth = jmin (250, r.getWidth() / 4);

    header.setBounds (r.removeFromTop (80));

    demoList.setBounds (r.removeFromLeft (listWidth));

    r.removeFromTop (5);

    if (parametersComponent != nullptr)
        parametersComponent->setBounds (r.removeFromTop (parametersComponent->getHeightNeeded()).reduced (20, 0));

    r.removeFromBottom (10);

    codeEditor.setBounds (r);
}

void MainContentComponent::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    Rectangle<int> r { 0, 0, width, height };
    auto& lf = getLookAndFeel();

    g.setColour (lf.findColour (rowIsSelected ? static_cast<int> (TextEditor::highlightColourId) : static_cast<int> (ListBox::backgroundColourId)));
    g.fillRect (r);

    if (auto demo = Demo::getList()[rowNumber])
    {
        g.setColour (lf.findColour (rowIsSelected ? static_cast<int> (TextEditor::highlightedTextColourId) : static_cast<int> (ListBox::textColourId)));
        g.drawFittedText (demo->name, r.reduced (10, 2), Justification::centredLeft, 1);
    }
}

int MainContentComponent::getNumRows()
{
    return Demo::getList().size();
}

void MainContentComponent::selectedRowsChanged (int lastRowSelected)
{
    if (lastRowSelected >= 0)
    {
        DSPSamplesApplication::getApp().setCurrentDemo (lastRowSelected);

        if (auto demo = Demo::getList()[DSPSamplesApplication::getApp().getCurrentDemoIndex()])
        {
            if (demo->code.isNotEmpty())
                codeDocument.replaceAllContent (demo->code);

            codeEditor.scrollToLine (0);
            initParameters();
        }
    }
}

void MainContentComponent::setupDemoColours()
{
    auto& lf = getLookAndFeel();

    lf.setColour (CodeEditorComponent::backgroundColourId,     Colour (0xff263238));
    lf.setColour (CodeEditorComponent::lineNumberTextId,       Colour (0xffaaaaaa));
    lf.setColour (CodeEditorComponent::lineNumberBackgroundId, Colour (0xff323e44));
    lf.setColour (CodeEditorComponent::highlightColourId,      Colour (0xffe0ec65).withAlpha (0.5f));
    lf.setColour (ScrollBar::ColourIds::thumbColourId,         Colour (0xffd0d8e0));

    lf.setColour (TextEditor::highlightColourId, Colour (0xffe0ec65).withAlpha (0.75f));
    lf.setColour (TextEditor::highlightedTextColourId, Colours::black);

    ScopedPointer<XmlElement> xml (XmlDocument::parse (BinaryData::EditorColourScheme_xml));

    if (xml != nullptr)
    {
        auto colourSchemeTree = ValueTree::fromXml (*xml);
        CodeEditorComponent::ColourScheme scheme (codeEditor.getColourScheme());

        for (auto& type : scheme.types)
        {
            auto colour = colourSchemeTree.getChildWithProperty ("name", type.name);

            if (colour.isValid())
                type.colour = Colour::fromString (colour ["colour"].toString());
        }

        codeEditor.setColourScheme (scheme);
    }

    codeEditor.setScrollbarThickness (6);
}

void MainContentComponent::initParameters()
{
    auto& parameters = DSPSamplesApplication::getApp().getCurrentDemoParameters();

    parametersComponent.reset();

    if (parameters.size() > 0)
        addAndMakeVisible (parametersComponent = new DemoParametersComponent (parameters));

    resized();
}
