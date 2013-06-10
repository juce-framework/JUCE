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

#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
#define __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__

#include "juce_KnownPluginList.h"
#include "../format/juce_AudioPluginFormat.h"


//==============================================================================
/**
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.
*/
class JUCE_API  PluginListComponent   : public Component,
                                        public FileDragAndDropTarget,
                                        private ListBoxModel,
                                        private ChangeListener,
                                        private ButtonListener  // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    //==============================================================================
    /**
        Creates the list component.

        For info about the deadMansPedalFile, see the PluginDirectoryScanner constructor.

        The properties file, if supplied, is used to store the user's last search paths.
    */
    PluginListComponent (AudioPluginFormatManager& formatManager,
                         KnownPluginList& listToRepresent,
                         const File& deadMansPedalFile,
                         PropertiesFile* propertiesToUse);

    /** Destructor. */
    ~PluginListComponent();

    /** Changes the text in the panel's button. */
    void setOptionsButtonText (const String& newText);

    /** Sets how many threads to simultaneously scan for plugins.
        If this is 0, then all scanning happens on the message thread (this is the default)
    */
    void setNumberOfThreadsForScanning (int numThreads);

    /** Returns the last search path stored in a given properties file for the specified format. */
    static FileSearchPath getLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format);

    /** Stores a search path in a properties file for the given format. */
    static void setLastSearchPath (PropertiesFile& properties, AudioPluginFormat& format,
                                   const FileSearchPath& newPath);

    /** Triggers a scan for the given format. */
    void scanFor (AudioPluginFormat& format);

    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    bool isInterestedInFileDrag (const StringArray&);
    /** @internal */
    void filesDropped (const StringArray&, int, int);
    /** @internal */
    int getNumRows();
    /** @internal */
    void paintListBoxItem (int row, Graphics&, int width, int height, bool rowIsSelected);
    /** @internal */
    void deleteKeyPressed (int lastRowSelected);

private:
    //==============================================================================
    AudioPluginFormatManager& formatManager;
    KnownPluginList& list;
    File deadMansPedalFile;
    ListBox listBox;
    TextButton optionsButton;
    PropertiesFile* propertiesToUse;
    int numThreads;

    class Scanner;
    friend class Scanner;
    friend class ScopedPointer<Scanner>;
    ScopedPointer<Scanner> currentScanner;

    void scanFinished (const StringArray&);

    static void optionsMenuStaticCallback (int, PluginListComponent*);
    void optionsMenuCallback (int);
    void updateList();
    void removeSelected();
    void showSelectedFolder();
    bool canShowSelectedFolder() const;
    void removeMissingPlugins();

    void buttonClicked (Button*);
    void changeListenerCallback (ChangeBroadcaster*);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginListComponent)
};


#endif   // __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
