/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
        fillStandardDemoBackground (g);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkingDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<NetworkingDemo> demo ("40 HTTP");
