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
