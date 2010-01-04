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

#ifndef __JUCE_POPUPMENU_JUCEHEADER__
#define __JUCE_POPUPMENU_JUCEHEADER__

#include "juce_PopupMenuCustomComponent.h"
#include "../../../application/juce_ApplicationCommandManager.h"


//==============================================================================
/** Creates and displays a popup-menu.

    To show a popup-menu, you create one of these, add some items to it, then
    call its show() method, which returns the id of the item the user selects.

    E.g. @code
    void MyWidget::mouseDown (const MouseEvent& e)
    {
        PopupMenu m;
        m.addItem (1, "item 1");
        m.addItem (2, "item 2");

        const int result = m.show();

        if (result == 0)
        {
            // user dismissed the menu without picking anything
        }
        else if (result == 1)
        {
            // user picked item 1
        }
        else if (result == 2)
        {
            // user picked item 2
        }
    }
    @endcode

    Submenus are easy too: @code

    void MyWidget::mouseDown (const MouseEvent& e)
    {
        PopupMenu subMenu;
        subMenu.addItem (1, "item 1");
        subMenu.addItem (2, "item 2");

        PopupMenu mainMenu;
        mainMenu.addItem (3, "item 3");
        mainMenu.addSubMenu ("other choices", subMenu);

        const int result = m.show();

        ...etc
    }
    @endcode
*/
class JUCE_API  PopupMenu
{
public:
    //==============================================================================
    /** Creates an empty popup menu. */
    PopupMenu();

    /** Creates a copy of another menu. */
    PopupMenu (const PopupMenu& other);

    /** Destructor. */
    ~PopupMenu();

    /** Copies this menu from another one. */
    const PopupMenu& operator= (const PopupMenu& other);

    //==============================================================================
    /** Resets the menu, removing all its items. */
    void clear();

    /** Appends a new text item for this menu to show.

        @param itemResultId     the number that will be returned from the show() method
                                if the user picks this item. The value should never be
                                zero, because that's used to indicate that the user didn't
                                select anything.
        @param itemText         the text to show.
        @param isActive         if false, the item will be shown 'greyed-out' and can't be
                                picked
        @param isTicked         if true, the item will be shown with a tick next to it
        @param iconToUse        if this is non-zero, it should be an image that will be
                                displayed to the left of the item. This method will take its
                                own copy of the image passed-in, so there's no need to keep
                                it hanging around.

        @see addSeparator, addColouredItem, addCustomItem, addSubMenu
    */
    void addItem (const int itemResultId,
                  const String& itemText,
                  const bool isActive = true,
                  const bool isTicked = false,
                  const Image* const iconToUse = 0);

    /** Adds an item that represents one of the commands in a command manager object.

        @param commandManager       the manager to use to trigger the command and get information
                                    about it
        @param commandID            the ID of the command
        @param displayName          if this is non-empty, then this string will be used instead of
                                    the command's registered name
    */
    void addCommandItem (ApplicationCommandManager* commandManager,
                         const int commandID,
                         const String& displayName = String::empty);


    /** Appends a text item with a special colour.

        This is the same as addItem(), but specifies a colour to use for the
        text, which will override the default colours that are used by the
        current look-and-feel. See addItem() for a description of the parameters.
    */
    void addColouredItem (const int itemResultId,
                          const String& itemText,
                          const Colour& itemTextColour,
                          const bool isActive = true,
                          const bool isTicked = false,
                          const Image* const iconToUse = 0);

    /** Appends a custom menu item.

        This will add a user-defined component to use as a menu item. The component
        passed in will be deleted by this menu when it's no longer needed.

        @see PopupMenuCustomComponent
    */
    void addCustomItem (const int itemResultId,
                        PopupMenuCustomComponent* const customComponent);

    /** Appends a custom menu item that can't be used to trigger a result.

        This will add a user-defined component to use as a menu item. Unlike the
        addCustomItem() method that takes a PopupMenuCustomComponent, this version
        can't trigger a result from it, so doesn't take a menu ID. It also doesn't
        delete the component when it's finished, so it's the caller's responsibility
        to manage the component that is passed-in.

        if triggerMenuItemAutomaticallyWhenClicked is true, the menu itself will handle
        detection of a mouse-click on your component, and use that to trigger the
        menu ID specified in itemResultId. If this is false, the menu item can't
        be triggered, so itemResultId is not used.

        @see PopupMenuCustomComponent
    */
    void addCustomItem (const int itemResultId,
                        Component* customComponent,
                        int idealWidth, int idealHeight,
                        const bool triggerMenuItemAutomaticallyWhenClicked);

    /** Appends a sub-menu.

        If the menu that's passed in is empty, it will appear as an inactive item.
    */
    void addSubMenu (const String& subMenuName,
                     const PopupMenu& subMenu,
                     const bool isActive = true,
                     Image* const iconToUse = 0,
                     const bool isTicked = false);

    /** Appends a separator to the menu, to help break it up into sections.

        The menu class is smart enough not to display separators at the top or bottom
        of the menu, and it will replace mutliple adjacent separators with a single
        one, so your code can be quite free and easy about adding these, and it'll
        always look ok.
    */
    void addSeparator();

    /** Adds a non-clickable text item to the menu.

        This is a bold-font items which can be used as a header to separate the items
        into named groups.
    */
    void addSectionHeader (const String& title);

    /** Returns the number of items that the menu currently contains.

        (This doesn't count separators).
    */
    int getNumItems() const;

    /** Returns true if the menu contains a command item that triggers the given command. */
    bool containsCommandItem (const int commandID) const;

    /** Returns true if the menu contains any items that can be used. */
    bool containsAnyActiveItems() const;

    //==============================================================================
    /** Displays the menu and waits for the user to pick something.

        This will display the menu modally, and return the ID of the item that the
        user picks. If they click somewhere off the menu to get rid of it without
        choosing anything, this will return 0.

        The current location of the mouse will be used as the position to show the
        menu - to explicitly set the menu's position, use showAt() instead. Depending
        on where this point is on the screen, the menu will appear above, below or
        to the side of the point.

        @param itemIdThatMustBeVisible  if you set this to the ID of one of the menu items,
                                        then when the menu first appears, it will make sure
                                        that this item is visible. So if the menu has too many
                                        items to fit on the screen, it will be scrolled to a
                                        position where this item is visible.
        @param minimumWidth             a minimum width for the menu, in pixels. It may be wider
                                        than this if some items are too long to fit.
        @param maximumNumColumns        if there are too many items to fit on-screen in a single
                                        vertical column, the menu may be laid out as a series of
                                        columns - this is the maximum number allowed. To use the
                                        default value for this (probably about 7), you can pass
                                        in zero.
        @param standardItemHeight       if this is non-zero, it will be used as the standard
                                        height for menu items (apart from custom items)
        @see showAt
    */
    int show (const int itemIdThatMustBeVisible = 0,
              const int minimumWidth = 0,
              const int maximumNumColumns = 0,
              const int standardItemHeight = 0);


    /** Displays the menu at a specific location.

        This is the same as show(), but uses a specific location (in global screen
        co-ordinates) rather than the current mouse position.

        Note that the co-ordinates don't specify the top-left of the menu - they
        indicate a point of interest, and the menu will position itself nearby to
        this point, trying to keep it fully on-screen.

        @see show()
    */
    int showAt (const int screenX,
                const int screenY,
                const int itemIdThatMustBeVisible = 0,
                const int minimumWidth = 0,
                const int maximumNumColumns = 0,
                const int standardItemHeight = 0);

    /** Displays the menu as if it's attached to a component such as a button.

        This is similar to showAt(), but will position it next to the given component, e.g.
        so that the menu's edge is aligned with that of the component. This is intended for
        things like buttons that trigger a pop-up menu.
    */
    int showAt (Component* componentToAttachTo,
                const int itemIdThatMustBeVisible = 0,
                const int minimumWidth = 0,
                const int maximumNumColumns = 0,
                const int standardItemHeight = 0);

    //==============================================================================
    /** Closes any menus that are currently open.

        This might be useful if you have a situation where your window is being closed
        by some means other than a user action, and you'd like to make sure that menus
        aren't left hanging around.
    */
    static void JUCE_CALLTYPE dismissAllActiveMenus();


    //==============================================================================
    /** Specifies a look-and-feel for the menu and any sub-menus that it has.

        This can be called before show() if you need a customised menu. Be careful
        not to delete the LookAndFeel object before the menu has been deleted.
    */
    void setLookAndFeel (LookAndFeel* const newLookAndFeel);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the menu.

        These constants can be used either via the LookAndFeel::setColour()
        method for the look and feel that is set for this menu with setLookAndFeel()

        @see setLookAndFeel, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId             = 0x1000700,  /**< The colour to fill the menu's background with. */
        textColourId                   = 0x1000600,  /**< The colour for normal menu item text, (unless the
                                                          colour is specified when the item is added). */
        headerTextColourId             = 0x1000601,  /**< The colour for section header item text (see the
                                                          addSectionHeader() method). */
        highlightedBackgroundColourId  = 0x1000900,  /**< The colour to fill the background of the currently
                                                          highlighted menu item. */
        highlightedTextColourId        = 0x1000800,  /**< The colour to use for the text of the currently
                                                          highlighted item. */
    };

    //==============================================================================
    /**
        Allows you to iterate through the items in a pop-up menu, and examine
        their properties.

        To use this, just create one and repeatedly call its next() method. When this
        returns true, all the member variables of the iterator are filled-out with
        information describing the menu item. When it returns false, the end of the
        list has been reached.
    */
    class JUCE_API  MenuItemIterator
    {
    public:
        //==============================================================================
        /** Creates an iterator that will scan through the items in the specified
            menu.

            Be careful not to add any items to a menu while it is being iterated,
            or things could get out of step.
        */
        MenuItemIterator (const PopupMenu& menu);

        /** Destructor. */
        ~MenuItemIterator();

        /** Returns true if there is another item, and sets up all this object's
            member variables to reflect that item's properties.
        */
        bool next();

        //==============================================================================
        String itemName;
        const PopupMenu* subMenu;
        int itemId;
        bool isSeparator;
        bool isTicked;
        bool isEnabled;
        bool isCustomComponent;
        bool isSectionHeader;
        const Colour* customColour;
        const Image* customImage;
        ApplicationCommandManager* commandManager;

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        const PopupMenu& menu;
        int index;

        MenuItemIterator (const MenuItemIterator&);
        const MenuItemIterator& operator= (const MenuItemIterator&);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class PopupMenuWindow;
    friend class MenuItemIterator;
    VoidArray items;
    LookAndFeel* lookAndFeel;
    bool separatorPending;

    void addSeparatorIfPending();

    int showMenu (const int x, const int y, const int w, const int h,
                  const int itemIdThatMustBeVisible,
                  const int minimumWidth,
                  const int maximumNumColumns,
                  const int standardItemHeight,
                  const bool alignToRectangle,
                  Component* const componentAttachedTo);

    friend class MenuBarComponent;
    Component* createMenuComponent (const int x, const int y, const int w, const int h,
                                    const int itemIdThatMustBeVisible,
                                    const int minimumWidth,
                                    const int maximumNumColumns,
                                    const int standardItemHeight,
                                    const bool alignToRectangle,
                                    Component* menuBarComponent,
                                    ApplicationCommandManager** managerOfChosenCommand,
                                    Component* const componentAttachedTo);
};

#endif   // __JUCE_POPUPMENU_JUCEHEADER__
