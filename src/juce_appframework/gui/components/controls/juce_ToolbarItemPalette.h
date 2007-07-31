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
                        Toolbar* const toolbar);

    /** Destructor. */
    ~ToolbarItemPalette();

    //==============================================================================
    /** @internal */
    void resized();

    juce_UseDebuggingNewOperator

private:
    ToolbarItemFactory& factory;
    Toolbar* toolbar;
    Viewport* viewport;

    friend class Toolbar;
    void replaceComponent (ToolbarItemComponent* const comp);

    ToolbarItemPalette (const ToolbarItemPalette&);
    const ToolbarItemPalette& operator= (const ToolbarItemPalette&);
};


#endif   // __JUCE_TOOLBARITEMPALETTE_JUCEHEADER__
