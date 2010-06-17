/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_APPLICATION_H_6595C2A8__
#define __JUCER_APPLICATION_H_6595C2A8__

#include "jucer_Headers.h"
#include "ui/jucer_MainWindow.h"

ApplicationCommandManager* commandManager = 0;


//==============================================================================
class JucerApplication : public JUCEApplication
{
public:
    //==============================================================================
    JucerApplication()  {}
    ~JucerApplication() {}

    //==============================================================================
    void initialise (const String& commandLine)
    {
        /* Running a command-line of the form "Jucer --resave foobar.jucer" will try to load that
           jucer file and re-export all of its projects.
        */
        if (commandLine.startsWithIgnoreCase ("-resave ") || commandLine.startsWithIgnoreCase ("--resave "))
        {
            Project::resaveJucerFile (File::getCurrentWorkingDirectory()
                                        .getChildFile (commandLine.fromFirstOccurrenceOf (" ", false, false).unquoted()));
            quit();
            return;
        }

        commandManager = new ApplicationCommandManager();
        commandManager->registerAllCommandsForTarget (this);

        theMainWindow = new MainWindow();
        doExtraInitialisation();

        ImageCache::setCacheTimeout (30 * 1000);

        if (commandLine.trim().isNotEmpty() && ! commandLine.trim().startsWithChar ('-'))
            anotherInstanceStarted (commandLine);

        theMainWindow->reloadLastProject();

        theMainWindow->getLookAndFeel().setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
    }

    void shutdown()
    {
        theMainWindow = 0;

        OpenDocumentManager::deleteInstance();
        deleteAndZero (commandManager);
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        if (theMainWindow == 0 || theMainWindow->closeCurrentProject())
        {
            theMainWindow = 0;
            StoredSettings::deleteInstance();

            quit();
        }
    }

    //==============================================================================
    const String getApplicationName()
    {
        return "The Jucer V" + getApplicationVersion();
    }

    const String getApplicationVersion()
    {
        return ProjectInfo::versionString;
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
        if (theMainWindow != 0)
            theMainWindow->openFile (commandLine.unquoted());
    }

    virtual void doExtraInitialisation() {}

private:
    ScopedPointer <MainWindow> theMainWindow;
};


#endif  // __JUCER_APPLICATION_H_6595C2A8__
