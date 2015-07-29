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

#ifndef JUCE_TOOLBARITEMFACTORY_H_INCLUDED
#define JUCE_TOOLBARITEMFACTORY_H_INCLUDED


//==============================================================================
/**
    A factory object which can create ToolbarItemComponent objects.

    A subclass of ToolbarItemFactory publishes a set of types of toolbar item
    that it can create.

    Each type of item is identified by a unique ID, and multiple instances of an
    item type can exist at once (even on the same toolbar, e.g. spacers or separator
    bars).

    @see Toolbar, ToolbarItemComponent, ToolbarButton
*/
class JUCE_API  ToolbarItemFactory
{
public:
    //==============================================================================
    ToolbarItemFactory();

    /** Destructor. */
    virtual ~ToolbarItemFactory();

    //==============================================================================
    /** A set of reserved item ID values, used for the built-in item types.
    */
    enum SpecialItemIds
    {
        separatorBarId      = -1,   /**< The item ID for a vertical (or horizontal) separator bar that
                                         can be placed between sets of items to break them into groups. */
        spacerId            = -2,   /**< The item ID for a fixed-width space that can be placed between
                                         items.*/
        flexibleSpacerId    = -3    /**< The item ID for a gap that pushes outwards against the things on
                                         either side of it, filling any available space. */
    };

    //==============================================================================
    /** Must return a list of the IDs for all the item types that this factory can create.

        The ids should be added to the array that is passed-in.

        An item ID can be any integer you choose, except for 0, which is considered a null ID,
        and the predefined IDs in the SpecialItemIds enum.

        You should also add the built-in types (separatorBarId, spacerId and flexibleSpacerId)
        to this list if you want your toolbar to be able to contain those items.

        The list returned here is used by the ToolbarItemPalette class to obtain its list
        of available items, and their order on the palette will reflect the order in which
        they appear on this list.

        @see ToolbarItemPalette
    */
    virtual void getAllToolbarItemIds (Array <int>& ids) = 0;

    /** Must return the set of items that should be added to a toolbar as its default set.

        This method is used by Toolbar::addDefaultItems() to determine which items to
        create.

        The items that your method adds to the array that is passed-in will be added to the
        toolbar in the same order. Items can appear in the list more than once.
    */
    virtual void getDefaultItemSet (Array <int>& ids) = 0;

    /** Must create an instance of one of the items that the factory lists in its
        getAllToolbarItemIds() method.

        The itemId parameter can be any of the values listed by your getAllToolbarItemIds()
        method, except for the built-in item types from the SpecialItemIds enum, which
        are created internally by the toolbar code.

        Try not to keep a pointer to the object that is returned, as it will be deleted
        automatically by the toolbar, and remember that multiple instances of the same
        item type are likely to exist at the same time.
    */
    virtual ToolbarItemComponent* createItem (int itemId) = 0;
};


#endif   // JUCE_TOOLBARITEMFACTORY_H_INCLUDED
