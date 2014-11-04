/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_POPUPMENU_H_INCLUDED
#define JUCE_POPUPMENU_H_INCLUDED


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
private:
    class Window;

public:
    //==============================================================================
    /** Creates an empty popup menu. */
    PopupMenu();

    /** Creates a copy of another menu. */
    PopupMenu (const PopupMenu& other);

    /** Destructor. */
    ~PopupMenu();

    /** Copies this menu from another one. */
    PopupMenu& operator= (const PopupMenu& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    PopupMenu (PopupMenu&& other) noexcept;
    PopupMenu& operator= (PopupMenu&& other) noexcept;
   #endif

    //==============================================================================
    /** Resets the menu, removing all its items. */
    void clear();

    /** Appends a new text item for this menu to show.

        @param itemResultID     the number that will be returned from the show() method
                                if the user picks this item. The value should never be
                                zero, because that's used to indicate that the user didn't
                                select anything.
        @param itemText         the text to show.
        @param isEnabled        if false, the item will be shown 'greyed-out' and can't be picked
        @param isTicked         if true, the item will be shown with a tick next to it

        @see addSeparator, addColouredItem, addCustomItem, addSubMenu
    */
    void addItem (int itemResultID,
                  const String& itemText,
                  bool isEnabled = true,
                  bool isTicked = false);

    /** Appends a new item with an icon.

        @param itemResultID     the number that will be returned from the show() method
                                if the user picks this item. The value should never be
                                zero, because that's used to indicate that the user didn't
                                select anything.
        @param itemText         the text to show.
        @param isEnabled        if false, the item will be shown 'greyed-out' and can't be picked
        @param isTicked         if true, the item will be shown with a tick next to it
        @param iconToUse        if this is a valid image, it will be displayed to the left of the item.

        @see addSeparator, addColouredItem, addCustomItem, addSubMenu
    */
    void addItem (int itemResultID,
                  const String& itemText,
                  bool isEnabled,
                  bool isTicked,
                  const Image& iconToUse);

    /** Appends a new item with an icon.

        @param itemResultID     the number that will be returned from the show() method
                                if the user picks this item. The value should never be
                                zero, because that's used to indicate that the user didn't
                                select anything.
        @param itemText         the text to show.
        @param isEnabled        if false, the item will be shown 'greyed-out' and can't be picked
        @param isTicked         if true, the item will be shown with a tick next to it
        @param iconToUse        a Drawable object to use as the icon to the left of the item.
                                The menu will take ownership of this drawable object and will
                                delete it later when no longer needed
        @see addSeparator, addColouredItem, addCustomItem, addSubMenu
    */
    void addItem (int itemResultID,
                  const String& itemText,
                  bool isEnabled,
                  bool isTicked,
                  Drawable* iconToUse);

    /** Adds an item that represents one of the commands in a command manager object.

        @param commandManager       the manager to use to trigger the command and get information
                                    about it
        @param commandID            the ID of the command
        @param displayName          if this is non-empty, then this string will be used instead of
                                    the command's registered name
    */
    void addCommandItem (ApplicationCommandManager* commandManager,
                         CommandID commandID,
                         const String& displayName = String::empty);


    /** Appends a text item with a special colour.

        This is the same as addItem(), but specifies a colour to use for the
        text, which will override the default colours that are used by the
        current look-and-feel. See addItem() for a description of the parameters.
    */
    void addColouredItem (int itemResultID,
                          const String& itemText,
                          Colour itemTextColour,
                          bool isEnabled = true,
                          bool isTicked = false,
                          const Image& iconToUse = Image::null);

    /** Appends a custom menu item that can't be used to trigger a result.

        This will add a user-defined component to use as a menu item.
        It's the caller's responsibility to delete the component that is passed-in
        when it's no longer needed after the menu has been hidden.

        If triggerMenuItemAutomaticallyWhenClicked is true, the menu itself will handle
        detection of a mouse-click on your component, and use that to trigger the
        menu ID specified in itemResultID. If this is false, the menu item can't
        be triggered, so itemResultID is not used.

        @see CustomComponent
    */
    void addCustomItem (int itemResultID,
                        Component* customComponent,
                        int idealWidth, int idealHeight,
                        bool triggerMenuItemAutomaticallyWhenClicked,
                        const PopupMenu* optionalSubMenu = nullptr);

    /** Appends a sub-menu.

        If the menu that's passed in is empty, it will appear as an inactive item.
        If the itemResultID argument is non-zero, then the sub-menu item itself can be
        clicked to trigger it as a command.
    */
    void addSubMenu (const String& subMenuName,
                     const PopupMenu& subMenu,
                     bool isEnabled = true);

    /** Appends a sub-menu with an icon.

        If the menu that's passed in is empty, it will appear as an inactive item.
        If the itemResultID argument is non-zero, then the sub-menu item itself can be
        clicked to trigger it as a command.
    */
    void addSubMenu (const String& subMenuName,
                     const PopupMenu& subMenu,
                     bool isEnabled,
                     const Image& iconToUse,
                     bool isTicked = false,
                     int itemResultID = 0);

    /** Appends a sub-menu with an icon.

        If the menu that's passed in is empty, it will appear as an inactive item.
        If the itemResultID argument is non-zero, then the sub-menu item itself can be
        clicked to trigger it as a command.

        The iconToUse parameter is a Drawable object to use as the icon to the left of
        the item. The menu will take ownership of this drawable object and will delete it
        later when no longer needed
    */
    void addSubMenu (const String& subMenuName,
                     const PopupMenu& subMenu,
                     bool isEnabled,
                     Drawable* iconToUse,
                     bool isTicked = false,
                     int itemResultID = 0);

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
    int getNumItems() const noexcept;

    /** Returns true if the menu contains a command item that triggers the given command. */
    bool containsCommandItem (int commandID) const;

    /** Returns true if the menu contains any items that can be used. */
    bool containsAnyActiveItems() const noexcept;

    //==============================================================================
    /** Class used to create a set of options to pass to the show() method.
        You can chain together a series of calls to this class's methods to create
        a set of whatever options you want to specify.
        E.g. @code
        PopupMenu menu;
        ...
        menu.showMenu (PopupMenu::Options().withMinimumWidth (100)
                                           .withMaximumNumColumns (3)
                                           .withTargetComponent (myComp));
        @endcode
    */
    class JUCE_API  Options
    {
    public:
        Options();

        Options withTargetComponent (Component* targetComponent) const noexcept;
        Options withTargetScreenArea (const Rectangle<int>& targetArea) const noexcept;
        Options withMinimumWidth (int minWidth) const noexcept;
        Options withMaximumNumColumns (int maxNumColumns) const noexcept;
        Options withStandardItemHeight (int standardHeight) const noexcept;
        Options withItemThatMustBeVisible (int idOfItemToBeVisible) const noexcept;

    private:
        friend class PopupMenu;
        friend class PopupMenu::Window;
        Rectangle<int> targetArea;
        Component* targetComponent;
        int visibleItemID, minWidth, maxColumns, standardHeight;
    };

    //==============================================================================
   #if JUCE_MODAL_LOOPS_PERMITTED
    /** Displays the menu and waits for the user to pick something.

        This will display the menu modally, and return the ID of the item that the
        user picks. If they click somewhere off the menu to get rid of it without
        choosing anything, this will return 0.

        The current location of the mouse will be used as the position to show the
        menu - to explicitly set the menu's position, use showAt() instead. Depending
        on where this point is on the screen, the menu will appear above, below or
        to the side of the point.

        @param itemIDThatMustBeVisible  if you set this to the ID of one of the menu items,
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
        @param callback                 if this is non-zero, the menu will be launched asynchronously,
                                        returning immediately, and the callback will receive a
                                        call when the menu is either dismissed or has an item
                                        selected. This object will be owned and deleted by the
                                        system, so make sure that it works safely and that any
                                        pointers that it uses are safely within scope.
        @see showAt
    */
    int show (int itemIDThatMustBeVisible = 0,
              int minimumWidth = 0,
              int maximumNumColumns = 0,
              int standardItemHeight = 0,
              ModalComponentManager::Callback* callback = nullptr);


    /** Displays the menu at a specific location.

        This is the same as show(), but uses a specific location (in global screen
        coordinates) rather than the current mouse position.

        The screenAreaToAttachTo parameter indicates a screen area to which the menu
        will be adjacent. Depending on where this is, the menu will decide which edge to
        attach itself to, in order to fit itself fully on-screen. If you just want to
        trigger a menu at a specific point, you can pass in a rectangle of size (0, 0)
        with the position that you want.

        @see show()
    */
    int showAt (const Rectangle<int>& screenAreaToAttachTo,
                int itemIDThatMustBeVisible = 0,
                int minimumWidth = 0,
                int maximumNumColumns = 0,
                int standardItemHeight = 0,
                ModalComponentManager::Callback* callback = nullptr);

    /** Displays the menu as if it's attached to a component such as a button.

        This is similar to showAt(), but will position it next to the given component, e.g.
        so that the menu's edge is aligned with that of the component. This is intended for
        things like buttons that trigger a pop-up menu.
    */
    int showAt (Component* componentToAttachTo,
                int itemIDThatMustBeVisible = 0,
                int minimumWidth = 0,
                int maximumNumColumns = 0,
                int standardItemHeight = 0,
                ModalComponentManager::Callback* callback = nullptr);

    /** Displays and runs the menu modally, with a set of options.
    */
    int showMenu (const Options& options);
   #endif

    /** Runs the menu asynchronously, with a user-provided callback that will receive the result. */
    void showMenuAsync (const Options& options,
                        ModalComponentManager::Callback* callback);

    //==============================================================================
    /** Closes any menus that are currently open.

        This might be useful if you have a situation where your window is being closed
        by some means other than a user action, and you'd like to make sure that menus
        aren't left hanging around.
    */
    static bool JUCE_CALLTYPE dismissAllActiveMenus();


    //==============================================================================
    /** Specifies a look-and-feel for the menu and any sub-menus that it has.

        This can be called before show() if you need a customised menu. Be careful
        not to delete the LookAndFeel object before the menu has been deleted.
    */
    void setLookAndFeel (LookAndFeel* newLookAndFeel);

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

        /** Adds an item to the target menu which has all the properties of this item. */
        void addItemTo (PopupMenu& targetMenu);

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
        const Drawable* icon;
        ApplicationCommandManager* commandManager;

    private:
        //==============================================================================
        const PopupMenu& menu;
        int index;

        MenuItemIterator& operator= (const MenuItemIterator&);
        JUCE_LEAK_DETECTOR (MenuItemIterator)
    };

    //==============================================================================
    /** A user-defined component that can be used as an item in a popup menu.
        @see PopupMenu::addCustomItem
    */
    class JUCE_API  CustomComponent  : public Component,
                                       public SingleThreadedReferenceCountedObject
    {
    public:
        /** Creates a custom item.
            If isTriggeredAutomatically is true, then the menu will automatically detect
            a mouse-click on this component and use that to invoke the menu item. If it's
            false, then it's up to your class to manually trigger the item when it wants to.
        */
        CustomComponent (bool isTriggeredAutomatically = true);

        /** Destructor. */
        ~CustomComponent();

        /** Returns a rectangle with the size that this component would like to have.

            Note that the size which this method returns isn't necessarily the one that
            the menu will give it, as the items will be stretched to have a uniform width.
        */
        virtual void getIdealSize (int& idealWidth, int& idealHeight) = 0;

        /** Dismisses the menu, indicating that this item has been chosen.

            This will cause the menu to exit from its modal state, returning
            this item's id as the result.
        */
        void triggerMenuItem();

        /** Returns true if this item should be highlighted because the mouse is over it.
            You can call this method in your paint() method to find out whether
            to draw a highlight.
        */
        bool isItemHighlighted() const noexcept                 { return isHighlighted; }

        /** @internal */
        bool isTriggeredAutomatically() const noexcept          { return triggeredAutomatically; }
        /** @internal */
        void setHighlighted (bool shouldBeHighlighted);

    private:
        //==============================================================================
        bool isHighlighted, triggeredAutomatically;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomComponent)
    };

    /** Appends a custom menu item.

        This will add a user-defined component to use as a menu item. The component
        passed in will be deleted by this menu when it's no longer needed.

        @see CustomComponent
    */
    void addCustomItem (int itemResultID, CustomComponent* customComponent,
                        const PopupMenu* optionalSubMenu = nullptr);


    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        menu drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

        /** Fills the background of a popup menu component. */
        virtual void drawPopupMenuBackground (Graphics&, int width, int height) = 0;

        /** Draws one of the items in a popup menu. */
        virtual void drawPopupMenuItem (Graphics&, const Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted,
                                        bool isTicked, bool hasSubMenu,
                                        const String& text,
                                        const String& shortcutKeyText,
                                        const Drawable* icon,
                                        const Colour* textColour) = 0;

        virtual void drawPopupMenuSectionHeader (Graphics&, const Rectangle<int>& area,
                                                 const String& sectionName) = 0;

        /** Returns the size and style of font to use in popup menus. */
        virtual Font getPopupMenuFont() = 0;

        virtual void drawPopupMenuUpDownArrow (Graphics&,
                                               int width, int height,
                                               bool isScrollUpArrow) = 0;

        /** Finds the best size for an item in a popup menu. */
        virtual void getIdealPopupMenuItemSize (const String& text,
                                                bool isSeparator,
                                                int standardMenuItemHeight,
                                                int& idealWidth,
                                                int& idealHeight) = 0;

        virtual int getMenuWindowFlags() = 0;

        virtual void drawMenuBarBackground (Graphics&, int width, int height,
                                            bool isMouseOverBar,
                                            MenuBarComponent&) = 0;

        virtual int getDefaultMenuBarHeight() = 0;

        virtual int getMenuBarItemWidth (MenuBarComponent&, int itemIndex, const String& itemText) = 0;

        virtual Font getMenuBarFont (MenuBarComponent&, int itemIndex, const String& itemText) = 0;

        virtual void drawMenuBarItem (Graphics&, int width, int height,
                                      int itemIndex,
                                      const String& itemText,
                                      bool isMouseOverItem,
                                      bool isMenuOpen,
                                      bool isMouseOverBar,
                                      MenuBarComponent&) = 0;
    };

private:
    //==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (class Item)
    JUCE_PUBLIC_IN_DLL_BUILD (struct HelperClasses)
    friend struct HelperClasses;
    friend class MenuBarComponent;

    OwnedArray<Item> items;
    LookAndFeel* lookAndFeel;

    Component* createWindow (const Options&, ApplicationCommandManager**) const;
    int showWithOptionalCallback (const Options&, ModalComponentManager::Callback*, bool);

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // These methods have new implementations now - see its new definition
    int drawPopupMenuItem (Graphics&, int, int, bool, bool, bool, bool, bool, const String&, const String&, Image*, const Colour*) { return 0; }
   #endif

    JUCE_LEAK_DETECTOR (PopupMenu)
};

#endif   // JUCE_POPUPMENU_H_INCLUDED
