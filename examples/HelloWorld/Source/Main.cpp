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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainComponent.h"

//==============================================================================
/**
    This is the top-level window that we'll pop up. Inside it, we'll create and
    show a component from the MainComponent.cpp file (you can open this file using
    the Jucer to edit it).
*/
class HelloWorldWindow  : public DocumentWindow
{
public:
    //==============================================================================
    HelloWorldWindow()
        : DocumentWindow ("JUCE Hello World!",
                          LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                          DocumentWindow::allButtons,
                          true)
    {
        // Create an instance of our main content component, and add it to our window..
        setContentOwned (new MainComponent(), true);

        // Centre the window on the screen
        centreWithSize (getWidth(), getHeight());

        // And show it!
        setVisible (true);
    }

    ~HelloWorldWindow()
    {
        // (the content component will be deleted automatically, so no need to do it here)
    }

    //==============================================================================
    void closeButtonPressed() override
    {
        // When the user presses the close button, we'll tell the app to quit. This
        // HelloWorldWindow object will be deleted by the JUCEHelloWorldApplication class.
        JUCEApplication::quit();
    }
};

//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class JUCEHelloWorldApplication : public JUCEApplication
{
public:
    //==============================================================================
    JUCEHelloWorldApplication() {}

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // For this demo, we'll just create the main window...
        helloWorldWindow = new HelloWorldWindow();

        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown() override
    {
        // This method is where you should clear-up your app's resources..

        // The helloWorldWindow variable is a ScopedPointer, so setting it to a null
        // pointer will delete the window.
        helloWorldWindow = nullptr;
    }

    //==============================================================================
    const String getApplicationName() override
    {
        return "Hello World for JUCE";
    }

    const String getApplicationVersion() override
    {
        // The ProjectInfo::versionString value is automatically updated by the Jucer, and
        // can be found in the JuceHeader.h file that it generates for our project.
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
    }

private:
    ScopedPointer<HelloWorldWindow> helloWorldWindow;
};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (JUCEHelloWorldApplication)
