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

#include "jucer_Headers.h"
#include "ui/jucer_MainWindow.h"

ApplicationCommandManager* commandManager = 0;


//==============================================================================
class JucerApplication : public JUCEApplication
{
public:
    //==============================================================================
    JucerApplication()
    {
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {
        commandManager = new ApplicationCommandManager();

        theMainWindow = new MainWindow();
        theMainWindow->setVisible (true);

        ImageCache::setCacheTimeout (30 * 1000);

        if (commandLine.trim().isNotEmpty()
              && ! commandLine.trim().startsWithChar ('-'))
            anotherInstanceStarted (commandLine);
    }

    void shutdown()
    {
        theMainWindow = nullptr;

        deleteAndZero (commandManager);
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        if (theMainWindow == nullptr || theMainWindow->closeAllDocuments())
        {
            theMainWindow = nullptr;
            StoredSettings::deleteInstance();
            quit();
        }
    }

    //==============================================================================
    const String getApplicationName()
    {
        return "The Jucer";
    }

    const String getApplicationVersion()
    {
        return String (JUCER_MAJOR_VERSION) + "." + String (JUCER_MINOR_VERSION);
    }

    bool moreThanOneInstanceAllowed()
    {
       #ifndef JUCE_LINUX
        return false;
       #else
        return true; //xxx should be false but doesn't work on linux..
       #endif
    }

    void anotherInstanceStarted (const String& commandLine)
    {
        if (theMainWindow != nullptr && commandLine.unquoted().isNotEmpty())
            theMainWindow->openFile (commandLine.unquoted());
    }

private:
    ScopedPointer<MainWindow> theMainWindow;
};


START_JUCE_APPLICATION(JucerApplication)
