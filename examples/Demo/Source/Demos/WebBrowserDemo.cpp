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


#if JUCE_WEB_BROWSER

//==============================================================================
/** We'll use a subclass of WebBrowserComponent to demonstrate how to get callbacks
    when the browser changes URL. You don't need to do this, you can just also
    just use the WebBrowserComponent class directly.
*/
class DemoBrowserComponent  : public WebBrowserComponent
{
public:
    //==============================================================================
    DemoBrowserComponent (TextEditor& addressBox)  : addressTextBox (addressBox)
    {
    }

    // This method gets called when the browser is about to go to a new URL..
    bool pageAboutToLoad (const String& newURL) override
    {
        // We'll just update our address box to reflect the new location..
        addressTextBox.setText (newURL, false);

        // we could return false here to tell the browser not to go ahead with
        // loading the page.
        return true;
    }

    // This method gets called when the browser is requested to launch a new window
    void newWindowAttemptingToLoad (const String& newURL) override
    {
        // We'll just load the URL into the main window
        goToURL (newURL);
    }

private:
    TextEditor& addressTextBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoBrowserComponent)
};


//==============================================================================
class WebBrowserDemo    : public Component
{
public:
    WebBrowserDemo()
    {
        setOpaque (true);

        // Create an address box..
        addAndMakeVisible (addressTextBox);
        addressTextBox.setTextToShowWhenEmpty ("Enter a web address, e.g. https://www.juce.com", Colours::grey);
        addressTextBox.onReturnKey = [this] { webView->goToURL (addressTextBox.getText()); };

        // create the actual browser component
        addAndMakeVisible (webView = new DemoBrowserComponent (addressTextBox));

        // add some buttons..
        addAndMakeVisible (goButton);
        goButton.onClick = [this] { webView->goToURL (addressTextBox.getText()); };
        addAndMakeVisible (backButton);
        backButton.onClick = [this] { webView->goBack(); };
        addAndMakeVisible (forwardButton);
        forwardButton.onClick = [this] { webView->goForward(); };

        // send the browser to a start page..
        webView->goToURL ("https://www.juce.com");
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::grey));
    }

    void resized() override
    {
        webView->setBounds (10, 45, getWidth() - 20, getHeight() - 55);
        goButton.setBounds (getWidth() - 45, 10, 35, 25);
        addressTextBox.setBounds (100, 10, getWidth() - 155, 25);
        backButton.setBounds (10, 10, 35, 25);
        forwardButton.setBounds (55, 10, 35, 25);
    }

private:
    ScopedPointer<DemoBrowserComponent> webView;

    TextEditor addressTextBox;

    TextButton goButton      { "Go", "Go to URL" },
               backButton    { "<<", "Back" },
               forwardButton { ">>", "Forward" };

    void lookAndFeelChanged() override
    {
        addressTextBox.applyFontToAllText (addressTextBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowserDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<WebBrowserDemo> demo ("10 Components: Web Browser");

#endif // JUCE_WEB_BROWSER
