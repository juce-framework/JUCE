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

#ifndef __MAINHOSTWINDOW_JUCEHEADER__
#define __MAINHOSTWINDOW_JUCEHEADER__

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
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    bool tryToQuitApplication();

    void createPlugin (const PluginDescription* desc, int x, int y);

    void addPluginsToMenu (PopupMenu& m) const;
    const PluginDescription* getChosenType (const int menuID) const;

    GraphDocumentComponent* getGraphEditor() const;

    bool isDoublePrecisionProcessing();
    void updatePrecisionMenuItem (ApplicationCommandInfo& info);

private:
    //==============================================================================
    AudioDeviceManager deviceManager;
    AudioPluginFormatManager formatManager;

    OwnedArray <PluginDescription> internalTypes;
    KnownPluginList knownPluginList;
    KnownPluginList::SortMethod pluginSortMethod;

    class PluginListWindow;
    ScopedPointer<PluginListWindow> pluginListWindow;

    void showAudioSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainHostWindow)
};


#endif   // __MAINHOSTWINDOW_JUCEHEADER__
