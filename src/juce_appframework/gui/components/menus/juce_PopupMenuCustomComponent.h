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

#ifndef __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__
#define __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"


//==============================================================================
/** A user-defined copmonent that can appear inside one of the rows of a popup menu.

    @see PopupMenu::addCustomItem
*/
class JUCE_API  PopupMenuCustomComponent  : public Component
{
public:
    /** Destructor. */
    ~PopupMenuCustomComponent();

    /** Chooses the size that this component would like to have.

        Note that the size which this method returns isn't necessarily the one that
        the menu will give it, as it will be stretched to fit the other items in
        the menu.
    */
    virtual void getIdealSize (int& idealWidth,
                               int& idealHeight) = 0;

    /** Dismisses the menu indicating that this item has been chosen.

        This will cause the menu to exit from its modal state, returning
        this item's id as the result.
    */
    void triggerMenuItem();

    /** Returns true if this item should be highlighted because the mouse is
        over it.

        You can call this method in your paint() method to find out whether
        to draw a highlight.
    */
    bool isItemHighlighted() const throw()                   { return isHighlighted; }

protected:
    /** Constructor.

        If isTriggeredAutomatically is true, then the menu will automatically detect
        a click on this component and use that to trigger it. If it's false, then it's
        up to your class to manually trigger the item if it wants to.
    */
    PopupMenuCustomComponent (const bool isTriggeredAutomatically = true);


private:
    friend class MenuItemInfo;
    friend class MenuItemComponent;
    friend class PopupMenuWindow;
    int refCount_;
    bool isHighlighted, isTriggeredAutomatically;

    PopupMenuCustomComponent (const PopupMenuCustomComponent&);
    const PopupMenuCustomComponent& operator= (const PopupMenuCustomComponent&);
};



#endif   // __JUCE_POPUPMENUCUSTOMCOMPONENT_JUCEHEADER__
