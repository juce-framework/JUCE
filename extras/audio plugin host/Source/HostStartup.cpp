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

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainHostWindow.h"
#include "InternalFilters.h"

#if ! (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_AU)
 #error "If you're building the audio plugin host, you probably want to enable VST and/or AU support"
#endif


ApplicationCommandManager* commandManager = nullptr;
ApplicationProperties* appProperties = nullptr;


//==============================================================================
class PluginHostApp  : public JUCEApplication
{
public:
    //==============================================================================
    PluginHostApp()
    {
    }

    void initialise (const String& commandLine)
    {
        // initialise our settings file..

        PropertiesFile::Options options;
        options.applicationName     = "Juce Audio Plugin Host";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Preferences";

        appProperties = new ApplicationProperties();
        appProperties->setStorageParameters (options);

        commandManager = new ApplicationCommandManager();

        AudioPluginFormatManager::getInstance()->addDefaultFormats();
        AudioPluginFormatManager::getInstance()->addFormat (new InternalPluginFormat());

        mainWindow = new MainHostWindow();
        //mainWindow->setUsingNativeTitleBar (true);

        commandManager->registerAllCommandsForTarget (this);
        commandManager->registerAllCommandsForTarget (mainWindow);

        mainWindow->menuItemsChanged();

        if (commandLine.isNotEmpty() && mainWindow->getGraphEditor() != 0)
        {
           #if JUCE_MAC
            if (! commandLine.trimStart().startsWith ("-"))
           #endif
                mainWindow->getGraphEditor()->graph.loadFrom (File::getCurrentWorkingDirectory()
                                                                .getChildFile (commandLine), true);
        }
    }

    void shutdown()
    {
        mainWindow = 0;
        appProperties->closeFiles();

        deleteAndZero (commandManager);
        deleteAndZero (appProperties);
    }

    void systemRequestedQuit()
    {
        if (mainWindow != 0)
            mainWindow->tryToQuitApplication();
        else
            JUCEApplication::quit();
    }

    const String getApplicationName()       { return "Juce Plug-In Host"; }
    const String getApplicationVersion()    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed()       { return true; }

private:
    ScopedPointer <MainHostWindow> mainWindow;
};


// This kicks the whole thing off..
START_JUCE_APPLICATION (PluginHostApp)
