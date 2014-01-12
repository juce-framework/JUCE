/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"


//==============================================================================
class NetworkingDemo   : public Component,
                         private Button::Listener
{
public:
    NetworkingDemo()
        : resultsBox (resultsDocument, nullptr)
    {
        setOpaque (true);

        addAndMakeVisible (urlBox);
        urlBox.setText ("http://www.google.com");

        addAndMakeVisible (fetchButton);
        fetchButton.setButtonText ("Download URL Contents");
        fetchButton.addListener (this);

        addAndMakeVisible (resultsBox);
    }

    ~NetworkingDemo()
    {
        fetchButton.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillBrushedAluminiumBackground (g);
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

private:
    TextEditor urlBox;
    TextButton fetchButton;

    CodeDocument resultsDocument;
    CodeEditorComponent resultsBox;

    void downloadUrl()
    {
        URL url (urlBox.getText());
        resultsBox.loadContent (url.readEntireTextStream());
    }

    void buttonClicked (Button* button) override
    {
        if (button == &fetchButton)
            downloadUrl();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetworkingDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<NetworkingDemo> demo ("40 HTTP");
