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

#pragma once

#include "FilterGraph.h"
#include "GraphEditorPanel.h"


//==============================================================================
namespace CommandIDs
{
    static const int open                   = 0x30000;
    static const int save                   = 0x30001;
    static const int saveAs                 = 0x30002;
    static const int newFile                = 0x30003;
    static const int showPluginListEditor   = 0x30100;
    static const int showAudioSettings      = 0x30200;
    static const int aboutBox               = 0x30300;
    static const int allWindowsForward      = 0x30400;
    static const int toggleDoublePrecision  = 0x30500;
}

ApplicationCommandManager& getCommandManager();
ApplicationProperties& getAppProperties();

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
    void changeListenerCallback (ChangeBroadcaster*);

    bool isInterestedInFileDrag (const StringArray& files);
    void fileDragEnter (const StringArray& files, int, int);
    void fileDragMove (const StringArray& files, int, int);
    void fileDragExit (const StringArray& files);
    void filesDropped (const StringArray& files, int, int);

    void menuBarActivated (bool isActive);

    StringArray getMenuBarNames();
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array<CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    void tryToQuitApplication();

    void createPlugin (const PluginDescription&, Point<int> pos);

    void addPluginsToMenu (PopupMenu&) const;
    const PluginDescription* getChosenType (int menuID) const;

    GraphDocumentComponent* getGraphEditor() const;

    bool isDoublePrecisionProcessing();
    void updatePrecisionMenuItem (ApplicationCommandInfo& info);

private:
    //==============================================================================
    AudioDeviceManager deviceManager;
    AudioPluginFormatManager formatManager;

    OwnedArray<PluginDescription> internalTypes;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    class PluginListWindow;
    ScopedPointer<PluginListWindow> pluginListWindow;

    void showAudioSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainHostWindow)
};
