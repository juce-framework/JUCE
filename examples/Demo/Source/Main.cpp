/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
