/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             NetworkingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases networking features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        NetworkingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class NetworkingDemo   : public Component,
                         private Thread
{
public:
    NetworkingDemo()
        : Thread ("Network Demo")
    {
        setOpaque (true);

        addAndMakeVisible (urlBox);
        urlBox.setText ("https://www.google.com");
        urlBox.onReturnKey = [this] { fetchButton.triggerClick(); };

        addAndMakeVisible (fetchButton);
        fetchButton.onClick = [this] { startThread(); };

        addAndMakeVisible (resultsBox);

        setSize (500, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds();

        {
            auto topArea = area.removeFromTop (40);
            fetchButton.setBounds (topArea.removeFromRight (180).reduced (8));
            urlBox     .setBounds (topArea.reduced (8));
        }

        resultsBox.setBounds (area.reduced (8));
    }

    void run() override
    {
        auto result = getResultText (urlBox.getText());

        MessageManagerLock mml (this);

        if (mml.lockWasGained())
            resultsBox.loadContent (result);
    }

    String getResultText (const URL& url)
    {
        StringPairArray responseHeaders;
        int statusCode = 0;

        if (auto stream = url.createInputStream (URL::InputStreamOptions (URL::ParameterHandling::inAddress)
                                                                                 .withConnectionTimeoutMs(10000)
                                                                                 .withResponseHeaders (&responseHeaders)
                                                                                 .withStatusCode (&statusCode)))
        {
            return (statusCode != 0 ? "Status code: " + String (statusCode) + newLine : String())
                    + "Response headers: " + newLine
                    + responseHeaders.getDescription() + newLine
                    + "----------------------------------------------------" + newLine
                    + stream->readEntireStreamAsString();
        }

        if (statusCode != 0)
            return "Failed to connect, status code = " + String (statusCode);

        return "Failed to connect!";
    }

private:
    TextEditor urlBox;
    TextButton fetchButton { "Download URL Contents" };

    CodeDocument resultsDocument;
    CodeEditorComponent resultsBox  { resultsDocument, nullptr };

    void lookAndFeelChanged() override
    {
        urlBox.applyFontToAllText (urlBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkingDemo)
};
