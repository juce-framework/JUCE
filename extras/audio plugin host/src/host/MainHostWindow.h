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

#ifndef __JUCE_MAINHOSTWINDOW_JUCEHEADER__
#define __JUCE_MAINHOSTWINDOW_JUCEHEADER__

#include "FilterGraph.h"
#include "GraphEditorPanel.h"


//==============================================================================
namespace CommandIDs
{
    static const int open                   = 0x30000;
    static const int save                   = 0x30001;
    static const int saveAs                 = 0x30002;
    static const int showPluginListEditor   = 0x30100;
    static const int showAudioSettings      = 0x30200;
    static const int aboutBox               = 0x30300;
}

extern ApplicationCommandManager* commandManager;


//==============================================================================
/**
*/
class MainHostWindow    : public DocumentWindow,
                          public MenuBarModel,
                          public ApplicationCommandTarget,
                          public ChangeListener,
                          public FileDragAndDropTarget
{
public:
    //==============================================================================
    MainHostWindow();
    ~MainHostWindow();

    //==============================================================================
    void closeButtonPressed();
    void changeListenerCallback (void*);

    bool isInterestedInFileDrag (const StringArray& files);
    void fileDragEnter (const StringArray& files, int, int);
    void fileDragMove (const StringArray& files, int, int);
    void fileDragExit (const StringArray& files);
    void filesDropped (const StringArray& files, int, int);

    const StringArray getMenuBarNames();
    const PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    bool tryToQuitApplication();

    void createPlugin (const PluginDescription* desc, int x, int y);

    void addPluginsToMenu (PopupMenu& m) const;
    const PluginDescription* getChosenType (const int menuID) const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioDeviceManager deviceManager;

    OwnedArray <PluginDescription> internalTypes;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    void showAudioSettings();
    GraphDocumentComponent* getGraphEditor() const;
};


#endif
