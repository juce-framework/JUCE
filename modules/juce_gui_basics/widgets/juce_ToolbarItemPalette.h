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

#ifndef __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
#define __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__

#include "juce_Toolbar.h"
#include "../layout/juce_Viewport.h"


//==============================================================================
/**
    A component containing a list of toolbar items, which the user can drag onto
    a toolbar to add them.

    You can use this class directly, but it's a lot easier to call Toolbar::showCustomisationDialog(),
    which automatically shows one of these in a dialog box with lots of extra controls.

    @see Toolbar
*/
class JUCE_API  ToolbarItemPalette    : public Component,
                                        public DragAndDropContainer
{
public:
    //==============================================================================
    /** Creates a palette of items for a given factory, with the aim of adding them
        to the specified toolbar.

        The ToolbarItemFactory::getAllToolbarItemIds() method is used to create the
        set of items that are shown in this palette.

        The toolbar and factory must not be deleted while this object exists.
    */
    ToolbarItemPalette (ToolbarItemFactory& factory,
                        Toolbar* toolbar);

    /** Destructor. */
    ~ToolbarItemPalette();

    //==============================================================================
    /** @internal */
    void resized();

private:
    ToolbarItemFactory& factory;
    Toolbar* toolbar;
    Viewport viewport;
    OwnedArray <ToolbarItemComponent> items;

    friend class Toolbar;
    void replaceComponent (ToolbarItemComponent* comp);
    void addComponent (int itemId, int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToolbarItemPalette)
};


#endif   // __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
