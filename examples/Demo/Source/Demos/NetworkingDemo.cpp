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

#include "../JuceDemoHeader.h"


//==============================================================================
class NetworkingDemo   : public Component,
                         private Button::Listener,
                         private TextEditor::Listener,
                         private Thread
{
public:
    NetworkingDemo()
        : Thread ("Network Demo"),
          resultsBox (resultsDocument, nullptr)
    {
        setOpaque (true);

        addAndMakeVisible (urlBox);
        urlBox.setText ("https://www.google.com");
        urlBox.addListener (this);

        addAndMakeVisible (fetchButton);
        fetchButton.setButtonText ("Download URL Contents");
        fetchButton.addListener (this);

        addAndMakeVisible (resultsBox);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());

        {
            Rectangle<int> topArea (area.removeFromTop (40));
            fetchButton.setBounds (topArea.removeFromRight (180).reduced (8));
            urlBox.setBounds (topArea.reduced (8));
        }

        resultsBox.setBounds (area.reduced (8));
    }

    void run() override
    {
        String result (getResultText (urlBox.getText()));

        MessageManagerLock mml (this);

        if (mml.lockWasGained())
            resultsBox.loadContent (result);
    }

    String getResultText (const URL& url)
    {
        StringPairArray responseHeaders;
        int statusCode = 0;

        ScopedPointer<InputStream> stream (url.createInputStream (false, nullptr, nullptr, String(),
                                                                  10000, // timeout in millisecs
                                                                  &responseHeaders, &statusCode));
        if (stream != nullptr)
            return (statusCode != 0 ? "Status code: " + String (statusCode) + newLine : String())
                    + "Response headers: " + newLine
                    + responseHeaders.getDescription() + newLine
                    + "----------------------------------------------------" + newLine
                    + stream->readEntireStreamAsString();

        if (statusCode != 0)
            return "Failed to connect, status code = " + String (statusCode);

        return "Failed to connect!";
    }

private:
    TextEditor urlBox;
    TextButton fetchButton;

    CodeDocument resultsDocument;
    CodeEditorComponent resultsBox;

    void buttonClicked (Button* button) override
    {
        if (button == &fetchButton)
            startThread();
    }

    void textEditorReturnKeyPressed (TextEditor&) override
    {
        fetchButton.triggerClick();
    }

    void lookAndFeelChanged() override
    {
        urlBox.applyFontToAllText (urlBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkingDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<NetworkingDemo> demo ("40 HTTP");
