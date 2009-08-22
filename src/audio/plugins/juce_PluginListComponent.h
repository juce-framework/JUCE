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

#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
#define __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__

#include "juce_KnownPluginList.h"
#include "juce_AudioPluginFormat.h"
#include "../../gui/components/controls/juce_ListBox.h"
#include "../../gui/components/buttons/juce_TextButton.h"
#include "../../utilities/juce_PropertiesFile.h"


//==============================================================================
/**
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.
*/
class JUCE_API  PluginListComponent   : public Component,
                                        public ListBoxModel,
                                        public ChangeListener,
                                        public ButtonListener,
                                        public Timer
{
public:
    //==============================================================================
    /**
        Creates the list component.

        For info about the deadMansPedalFile, see the PluginDirectoryScanner constructor.

        The properties file, if supplied, is used to store the user's last search paths.
    */
    PluginListComponent (KnownPluginList& listToRepresent,
                         const File& deadMansPedalFile,
                         PropertiesFile* const propertiesToUse);

    /** Destructor. */
    ~PluginListComponent();

    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    bool isInterestedInFileDrag (const StringArray& files);
    /** @internal */
    void filesDropped (const StringArray& files, int, int);
    /** @internal */
    int getNumRows();
    /** @internal */
    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected);
    /** @internal */
    void deleteKeyPressed (int lastRowSelected);
    /** @internal */
    void buttonClicked (Button* b);
    /** @internal */
    void changeListenerCallback (void*);
    /** @internal */
    void timerCallback();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    KnownPluginList& list;
    File deadMansPedalFile;
    ListBox* listBox;
    TextButton* optionsButton;
    PropertiesFile* propertiesToUse;
    int typeToScan;

    void scanFor (AudioPluginFormat* format);

    PluginListComponent (const PluginListComponent&);
    const PluginListComponent& operator= (const PluginListComponent&);
};


#endif   // __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
