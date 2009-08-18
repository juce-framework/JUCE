/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"

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
    DemoBrowserComponent (TextEditor* addressTextBox_)
        : addressTextBox (addressTextBox_)
    {
    }

    // This method gets called when the browser is about to go to a new URL..
    bool pageAboutToLoad (const String& newURL)
    {
        // We'll just update our address box to reflect the new location..
        addressTextBox->setText (newURL, false);

        // we could return false here to tell the browser not to go ahead with
        // loading the page.
        return true;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    TextEditor* addressTextBox;

    DemoBrowserComponent (DemoBrowserComponent&);
    const DemoBrowserComponent& operator= (const DemoBrowserComponent&);
};


//==============================================================================
class WebBrowserDemo   : public Component,
                         public TextEditorListener,
                         public ButtonListener
{
public:
    //==============================================================================
    WebBrowserDemo()
    {
        setName ("Web Browser");

        // Create an address box..
        addAndMakeVisible (addressTextBox = new TextEditor());
        addressTextBox->setTextToShowWhenEmpty ("Enter a web address, e.g. http://www.rawmaterialsoftware.com", Colours::grey);
        addressTextBox->addListener (this);

        // create the actual browser component
        addAndMakeVisible (webView = new DemoBrowserComponent (addressTextBox));

        // add some buttons..
        addAndMakeVisible (goButton = new TextButton ("Go", "Go to URL"));
        goButton->addButtonListener (this);
        addAndMakeVisible (backButton = new TextButton ("<<", "Back"));
        backButton->addButtonListener (this);
        addAndMakeVisible (forwardButton = new TextButton (">>", "Forward"));
        forwardButton->addButtonListener (this);

        // send the browser to a start page..
        webView->goToURL ("http://www.google.com");
    }

    ~WebBrowserDemo()
    {
        deleteAllChildren();
    }

    void resized()
    {
        webView->setBounds (10, 45, getWidth() - 20, getHeight() - 55);
        goButton->setBounds (getWidth() - 45, 10, 35, 25);
        addressTextBox->setBounds (100, 10, getWidth() - 155, 25);
        backButton->setBounds (10, 10, 35, 25);
        forwardButton->setBounds (55, 10, 35, 25);
    }

    void textEditorTextChanged (TextEditor& editor)             {}
    void textEditorEscapeKeyPressed (TextEditor& editor)        {}
    void textEditorFocusLost (TextEditor& editor)               {}

    void textEditorReturnKeyPressed (TextEditor&)
    {
        webView->goToURL (addressTextBox->getText());
    }

    void buttonClicked (Button* b)
    {
        if (b == backButton)
            webView->goBack();
        else if (b == forwardButton)
            webView->goForward();
        else if (b == goButton)
            webView->goToURL (addressTextBox->getText());
    }

    juce_UseDebuggingNewOperator

private:
    DemoBrowserComponent* webView;

    TextEditor* addressTextBox;
    TextButton* goButton;
    TextButton* backButton;
    TextButton* forwardButton;
};


//==============================================================================
Component* createWebBrowserDemo()
{
    return new WebBrowserDemo();
}

#endif
