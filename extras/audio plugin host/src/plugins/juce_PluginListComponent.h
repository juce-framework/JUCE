/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
#define __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__

#include "juce_KnownPluginList.h"
#include "juce_AudioPluginFormat.h"


//==============================================================================
/**
    A component displaying a list of plugins, with options to scan for them,
    add, remove and sort them.
*/
class PluginListComponent   : public Component,
                              public ListBoxModel,
                              public ChangeListener,
                              public ButtonListener
{
public:
    //==============================================================================
    /**
        Creates the list component.

        For info about the deadMansPedalFile, see the PluginDirectoryScanner constructor.

        The properties file, if supplied is used to store the user's last search paths.
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

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    KnownPluginList& list;
    File deadMansPedalFile;
    ListBox* listBox;
    TextButton* optionsButton;
    PropertiesFile* propertiesToUse;

    void scanFor (AudioPluginFormat* format);
};


#endif   // __JUCE_PLUGINLISTCOMPONENT_JUCEHEADER__
