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

#ifndef __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__
#define __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Project/jucer_Project.h"


//==============================================================================
class GroupInformationComponent  : public Component,
                                   private ListBoxModel,
                                   private ValueTree::Listener
{
public:
    //==============================================================================
    GroupInformationComponent (const Project::Item& item_);
    ~GroupInformationComponent();

    //==============================================================================
    void resized();

    int getNumRows();
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property);
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded);
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved);
    void valueTreeChildOrderChanged (ValueTree& parentTree);
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

private:
    Project::Item item;
    ListBox list;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GroupInformationComponent);
};


#endif   // __JUCER_GROUPINFORMATIONCOMPONENT_JUCEHEADER__
