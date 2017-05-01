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

#include "JuceDemoHeader.h"
#include "MainWindow.h"

bool invokeChildProcessDemo (const String& commandLine);

//==============================================================================
class JuceDemoApplication  : public JUCEApplication
{
public:
    JuceDemoApplication() {}

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // (This function call is for one of the demos, which involves launching a child process)
        if (invokeChildProcessDemo (commandLine))
            return;

        Desktop::getInstance().setOrientationsEnabled (Desktop::allOrientations);

        // Do your application's initialisation code here..
        mainWindow = new MainAppWindow();
    }

    void shutdown() override
    {
        // Do your application's shutdown code here..
        mainWindow = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This gets called when the OS wants our app to quit. You may want to
        // ask the user to save documents, close windows, etc here, but in this
        // case we'll just call quit(), which tells the message loop to stop and
        // allows the app to (asynchronously) exit.
        quit();
    }

    //==============================================================================
    const String getApplicationName() override
    {
        return "JuceDemo";
    }

    const String getApplicationVersion() override
    {
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void anotherInstanceStarted (const String& /*commandLine*/) override
    {
    }

private:
    ScopedPointer<MainAppWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that starts the app.
START_JUCE_APPLICATION(JuceDemoApplication)
