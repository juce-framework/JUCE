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

 name:             WebBrowserDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays a web browser.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        WebBrowserDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#if JUCE_WEB_BROWSER

#include "../Assets/DemoUtilities.h"

//==============================================================================
/** We'll use a subclass of WebBrowserComponent to demonstrate how to get callbacks
    when the browser changes URL. You don't need to do this, you can just also
    just use the WebBrowserComponent class directly.
*/
class DemoBrowserComponent final : public WebBrowserComponent
{
public:
    //==============================================================================
    DemoBrowserComponent (TextEditor& addressBox)
        : addressTextBox (addressBox)
    {}

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
class WebBrowserDemo final : public Component
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
        webView.reset (new DemoBrowserComponent (addressTextBox));
        addAndMakeVisible (webView.get());

        // add some buttons..
        addAndMakeVisible (goButton);
        goButton.onClick = [this] { webView->goToURL (addressTextBox.getText()); };
        addAndMakeVisible (backButton);
        backButton.onClick = [this] { webView->goBack(); };
        addAndMakeVisible (forwardButton);
        forwardButton.onClick = [this] { webView->goForward(); };

        // send the browser to a start page..
        webView->goToURL ("https://www.juce.com");

        setSize (1000, 1000);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::grey));
    }

    void resized() override
    {
        webView->setBounds       (10, 45, getWidth() - 20, getHeight() - 55);
        goButton      .setBounds (getWidth() - 45, 10, 35, 25);
        addressTextBox.setBounds (100, 10, getWidth() - 155, 25);
        backButton    .setBounds (10, 10, 35, 25);
        forwardButton .setBounds (55, 10, 35, 25);
    }

private:
    std::unique_ptr<DemoBrowserComponent> webView;

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

#endif
