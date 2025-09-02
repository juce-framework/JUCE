/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

        m.showMenuAsync (PopupMenu::Options(),
                         [] (int result)
                         {
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
                         });
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

        m.showMenuAsync (...);
    }
    @endcode

    @tags{GUI}
*/
class JUCE_API  PopupMenu
{
public:
    //==============================================================================
    /** Creates an empty popup menu. */
    PopupMenu() = default;

    /** Creates a copy of another menu. */
    PopupMenu (const PopupMenu&);

    /** Destructor. */
    ~PopupMenu();

    /** Copies this menu from another one. */
    PopupMenu& operator= (const PopupMenu&);

    /** Move constructor */
    PopupMenu (PopupMenu&&) noexcept;

    /** Move assignment operator */
    PopupMenu& operator= (PopupMenu&&) noexcept;

    //==============================================================================
    class CustomComponent;
    class CustomCallback;

    //==============================================================================
    /** Resets the menu, removing all its items. */
    void clear();

    /** Describes a popup menu item. */
    struct JUCE_API  Item
    {
        /** Creates a null item.
            You'll need to set some fields after creating an Item before you
            can add it to a PopupMenu
        */
        Item();

        /** Creates an item with the given text.
            This constructor also initialises the itemID to -1, which makes it suitable for
            creating lambda-based item actions.
        */
        Item (String text);

        Item (const Item&);
        Item& operator= (const Item&);
        Item (Item&&);
        Item& operator= (Item&&);

        /** The menu item's name. */
        String text;

        /** The menu item's ID.
            This must not be 0 if you want the item to be triggerable, but if you're attaching
            an action callback to the item, you can set the itemID to -1 to indicate that it
            isn't actively needed.
        */
        int itemID = 0;

        /** An optional function which should be invoked when this menu item is triggered. */
        std::function<void()> action;

        /** A sub-menu, or nullptr if there isn't one. */
        std::unique_ptr<PopupMenu> subMenu;

        /** A drawable to use as an icon, or nullptr if there isn't one. */
        std::unique_ptr<Drawable> image;

        /** A custom component for the item to display, or nullptr if there isn't one. */
        ReferenceCountedObjectPtr<CustomComponent> customComponent;

        /** A custom callback for the item to use, or nullptr if there isn't one. */
        ReferenceCountedObjectPtr<CustomCallback> customCallback;

        /** A command manager to use to automatically invoke the command, or nullptr if none is specified. */
        ApplicationCommandManager* commandManager = nullptr;

        /** An optional string describing the shortcut key for this item.
            This is only used for displaying at the right-hand edge of a menu item - the
            menu won't attempt to actually catch or process the key. If you supply a
            commandManager parameter then the menu will attempt to fill-in this field
            automatically.
        */
        String shortcutKeyDescription;

        /** A colour to use to draw the menu text.
            By default this is transparent black, which means that the LookAndFeel should choose the colour.
        */
        Colour colour;

        /** True if this menu item is enabled. */
        bool isEnabled = true;

        /** True if this menu item should have a tick mark next to it. */
        bool isTicked = false;

        /** True if this menu item is a separator line. */
        bool isSeparator = false;

        /** True if this menu item is a section header. */
        bool isSectionHeader = false;

        /** True if this is the final item in the current column. */
        bool shouldBreakAfter = false;

        /** Sets the isTicked flag (and returns a reference to this item to allow chaining). */
        Item& setTicked (bool shouldBeTicked = true) & noexcept;
        /** Sets the isEnabled flag (and returns a reference to this item to allow chaining). */
        Item& setEnabled (bool shouldBeEnabled) & noexcept;
        /** Sets the action property (and returns a reference to this item to allow chaining). */
        Item& setAction (std::function<void()> action) & noexcept;
        /** Sets the itemID property (and returns a reference to this item to allow chaining). */
        Item& setID (int newID) & noexcept;
        /** Sets the colour property (and returns a reference to this item to allow chaining). */
        Item& setColour (Colour) & noexcept;
        /** Sets the customComponent property (and returns a reference to this item to allow chaining). */
        Item& setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> customComponent) & noexcept;
        /** Sets the image property (and returns a reference to this item to allow chaining). */
        Item& setImage (std::unique_ptr<Drawable>) & noexcept;

        /** Sets the isTicked flag (and returns a reference to this item to allow chaining). */
        Item&& setTicked (bool shouldBeTicked = true) && noexcept;
        /** Sets the isEnabled flag (and returns a reference to this item to allow chaining). */
        Item&& setEnabled (bool shouldBeEnabled) && noexcept;
        /** Sets the action property (and returns a reference to this item to allow chaining). */
        Item&& setAction (std::function<void()> action) && noexcept;
        /** Sets the itemID property (and returns a reference to this item to allow chaining). */
        Item&& setID (int newID) && noexcept;
        /** Sets the colour property (and returns a reference to this item to allow chaining). */
        Item&& setColour (Colour) && noexcept;
        /** Sets the customComponent property (and returns a reference to this item to allow chaining). */
        Item&& setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> customComponent) && noexcept;
        /** Sets the image property (and returns a reference to this item to allow chaining). */
        Item&& setImage (std::unique_ptr<Drawable>) && noexcept;
    };

    /** Adds an item to the menu.
        You can call this method for full control over the item that is added, or use the other
        addItem helper methods if you want to pass arguments rather than creating an Item object.
    */
    void addItem (Item newItem);

    /** Adds an item to the menu with an action callback. */
    void addItem (String itemText,
                  std::function<void()> action);

    /** Adds an item to the menu with an action callback. */
    void addItem (String itemText,
                  bool isEnabled,
                  bool isTicked,
                  std::function<void()> action);

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
                  String itemText,
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
                  String itemText,
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
                  String itemText,
                  bool isEnabled,
                  bool isTicked,
                  std::unique_ptr<Drawable> iconToUse);

    /** Adds an item that represents one of the commands in a command manager object.

        @param commandManager       the manager to use to trigger the command and get information
                                    about it
        @param commandID            the ID of the command
        @param displayName          if this is non-empty, then this string will be used instead of
                                    the command's registered name
        @param iconToUse            an optional Drawable object to use as the icon to the left of the item.
                                    The menu will take ownership of this drawable object and will
                                    delete it later when no longer needed
    */
    void addCommandItem (ApplicationCommandManager* commandManager,
                         CommandID commandID,
                         String displayName = {},
                         std::unique_ptr<Drawable> iconToUse = {});

    /** Appends a text item with a special colour.

        This is the same as addItem(), but specifies a colour to use for the
        text, which will override the default colours that are used by the
        current look-and-feel. See addItem() for a description of the parameters.
    */
    void addColouredItem (int itemResultID,
                          String itemText,
                          Colour itemTextColour,
                          bool isEnabled = true,
                          bool isTicked = false,
                          const Image& iconToUse = {});

    /** Appends a text item with a special colour.

        This is the same as addItem(), but specifies a colour to use for the
        text, which will override the default colours that are used by the
        current look-and-feel. See addItem() for a description of the parameters.
    */
    void addColouredItem (int itemResultID,
                          String itemText,
                          Colour itemTextColour,
                          bool isEnabled,
                          bool isTicked,
                          std::unique_ptr<Drawable> iconToUse);

    /** Appends a custom menu item.

        This will add a user-defined component to use as a menu item.

        Note that native macOS menus do not support custom components.

        itemTitle will be used as the fallback text for this item, and will
        be exposed to screen reader clients.

        @see CustomComponent
    */
    void addCustomItem (int itemResultID,
                        std::unique_ptr<CustomComponent> customComponent,
                        std::unique_ptr<const PopupMenu> optionalSubMenu = nullptr,
                        const String& itemTitle = {});

    /** Appends a custom menu item that can't be used to trigger a result.

        This will add a user-defined component to use as a menu item.
        The caller must ensure that the passed-in component stays alive
        until after the menu has been hidden.

        If triggerMenuItemAutomaticallyWhenClicked is true, the menu itself will handle
        detection of a mouse-click on your component, and use that to trigger the
        menu ID specified in itemResultID. If this is false, the menu item can't
        be triggered, so itemResultID is not used.

        itemTitle will be used as the fallback text for this item, and will
        be exposed to screen reader clients.

        Note that native macOS menus do not support custom components.
    */
    void addCustomItem (int itemResultID,
                        Component& customComponent,
                        int idealWidth,
                        int idealHeight,
                        bool triggerMenuItemAutomaticallyWhenClicked,
                        std::unique_ptr<const PopupMenu> optionalSubMenu = nullptr,
                        const String& itemTitle = {});

    /** Appends a sub-menu.

        If the menu that's passed in is empty, it will appear as an inactive item.
        If the itemResultID argument is non-zero, then the sub-menu item itself can be
        clicked to trigger it as a command.
    */
    void addSubMenu (String subMenuName,
                     PopupMenu subMenu,
                     bool isEnabled = true);

    /** Appends a sub-menu with an icon.

        If the menu that's passed in is empty, it will appear as an inactive item.
        If the itemResultID argument is non-zero, then the sub-menu item itself can be
        clicked to trigger it as a command.
    */
    void addSubMenu (String subMenuName,
                     PopupMenu subMenu,
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
    void addSubMenu (String subMenuName,
                     PopupMenu subMenu,
                     bool isEnabled,
                     std::unique_ptr<Drawable> iconToUse,
                     bool isTicked = false,
                     int itemResultID = 0);

    /** Appends a separator to the menu, to help break it up into sections.
        The menu class is smart enough not to display separators at the top or bottom
        of the menu, and it will replace multiple adjacent separators with a single
        one, so your code can be quite free and easy about adding these, and it'll
        always look ok.
    */
    void addSeparator();

    /** Adds a non-clickable text item to the menu.
        This is a bold-font items which can be used as a header to separate the items
        into named groups.
    */
    void addSectionHeader (String title);

    /** Adds a column break to the menu, to help break it up into sections.
        Subsequent items will be placed in a new column, rather than being appended
        to the current column.

        If a menu contains explicit column breaks, the menu will never add additional
        breaks.
    */
    void addColumnBreak();

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
        /** By default, the target screen area will be the current mouse position. */
        Options();

        Options (const Options&) = default;
        Options& operator= (const Options&) = default;

        enum class PopupDirection
        {
            upwards,
            downwards
        };

        //==============================================================================
        /** Sets the target component to use when displaying the menu.

            This is normally the button or other control that triggered the menu.

            The target component is primarily used to control the scale of the menu, so
            it's important to supply a target component if you'll be using your program
            on hi-DPI displays.

            This function will also set the target screen area, so that the menu displays
            next to the target component. If you need to display the menu at a specific
            location, you should call withTargetScreenArea() after withTargetComponent.

            @see withTargetComponent, withTargetScreenArea
        */
        [[nodiscard]] Options withTargetComponent (Component* targetComponent) const;
        [[nodiscard]] Options withTargetComponent (Component& targetComponent) const;

        /** Sets the region of the screen next to which the menu should be displayed.

            To display the menu next to the mouse cursor use withMousePosition(),
            which is equivalent to passing the following to this function:
            @code
            Rectangle<int>{}.withPosition (Desktop::getMousePosition())
            @endcode

            withTargetComponent() will also set the target screen area. If you need
            a target component and a target screen area, make sure to call
            withTargetScreenArea() after withTargetComponent().

            @see withMousePosition
        */
        [[nodiscard]] Options withTargetScreenArea (Rectangle<int> targetArea) const;

        /** Sets the target screen area to match the current mouse position.

            Make sure to call this after withTargetComponent().

            @see withTargetScreenArea
        */
        [[nodiscard]] Options withMousePosition() const;

        /** If the passed component has been deleted when the popup menu exits,
            the selected item's action will not be called.

            This is useful for avoiding dangling references inside the action
            callback, in the case that the callback needs to access a component that
            may be deleted.
        */
        [[nodiscard]] Options withDeletionCheck (Component& componentToWatchForDeletion) const;

        /** Sets the minimum width of the popup window. */
        [[nodiscard]] Options withMinimumWidth (int minWidth) const;

        /** Sets the minimum number of columns in the popup window. */
        [[nodiscard]] Options withMinimumNumColumns (int minNumColumns) const;

        /** Sets the maximum number of columns in the popup window. */
        [[nodiscard]] Options withMaximumNumColumns (int maxNumColumns) const;

        /** Sets the default height of each item in the popup menu. */
        [[nodiscard]] Options withStandardItemHeight (int standardHeight) const;

        /** Sets an item which must be visible when the menu is initially drawn.

            This is useful to ensure that a particular item is shown when the menu
            contains too many items to display on a single screen.
        */
        [[nodiscard]] Options withItemThatMustBeVisible (int idOfItemToBeVisible) const;

        /** Sets a component that the popup menu will be drawn into.

            Some plugin formats, such as AUv3, dislike it when the plugin editor
            spawns additional windows. Some AUv3 hosts display pink backgrounds
            underneath transparent popup windows, which is confusing and can appear
            as though the plugin is malfunctioning. Setting a parent component will
            avoid this unwanted behaviour, but with the downside that the menu size
            will be constrained by the size of the parent component.
        */
        [[nodiscard]] Options withParentComponent (Component* parentComponent) const;

        /** Sets the direction of the popup menu relative to the target screen area. */
        [[nodiscard]] Options withPreferredPopupDirection (PopupDirection direction) const;

        /** Sets an item to select in the menu.

            This is useful for controls such as combo boxes, where opening the combo box
            with the keyboard should ideally highlight the currently-selected item, allowing
            the next/previous item to be selected by pressing up/down on the keyboard, rather
            than needing to move the highlighted row down from the top of the menu each time
            it is opened.
        */
        [[nodiscard]] Options withInitiallySelectedItem (int idOfItemToBeSelected) const;

        /** Returns a copy of these options with the target component set to null. The value of the
            top-level target component will not be changed.

            @see getTargetComponent(), getTopLevelTargetComponent()
        */
        [[nodiscard]] Options forSubmenu() const;

        //==============================================================================
        /** Gets the parent component. This may be nullptr if the Component has been deleted.

            @see withParentComponent
        */
        Component* getParentComponent() const noexcept               { return parentComponent; }

        /** Gets the target component. This may be nullptr if the Component has been deleted.

            @see withTargetComponent
        */
        Component* getTargetComponent() const noexcept               { return targetComponent; }

        /** Gets the target component that was set for the top-level menu.

            When querying the options of a submenu, getTargetComponent() will always return
            nullptr, while getTopLevelTargetComponent() will return the target passed to
            withTargetComponent() when creating the top-level menu.
        */
        Component* getTopLevelTargetComponent() const noexcept       { return topLevelTarget; }

        /** Returns true if the menu was watching a component, and that component has been deleted, and false otherwise.

            @see withDeletionCheck
        */
        bool hasWatchedComponentBeenDeleted() const noexcept         { return isWatchingForDeletion && componentToWatchForDeletion == nullptr; }

        /** Gets the target screen area.

            @see withTargetScreenArea
        */
        Rectangle<int> getTargetScreenArea() const noexcept          { return targetArea; }

        /** Gets the minimum width.

            @see withMinimumWidth
        */
        int getMinimumWidth() const noexcept                         { return minWidth; }

        /** Gets the maximum number of columns.

            @see withMaximumNumColumns
        */
        int getMaximumNumColumns() const noexcept                    { return maxColumns; }

        /** Gets the minimum number of columns.

            @see withMinimumNumColumns
        */
        int getMinimumNumColumns() const noexcept                    { return minColumns; }

        /** Gets the default height of items in the menu.

            @see withStandardItemHeight
        */
        int getStandardItemHeight() const noexcept                   { return standardHeight; }

        /** Gets the ID of the item that must be visible when the menu is initially shown.

            @see withItemThatMustBeVisible
        */
        int getItemThatMustBeVisible() const noexcept                { return visibleItemID; }

        /** Gets the preferred popup menu direction.

            @see withPreferredPopupDirection
        */
        PopupDirection getPreferredPopupDirection() const noexcept   { return preferredPopupDirection; }

        /** Gets the ID of the item that must be selected when the menu is initially shown.

            @see withItemThatMustBeVisible
        */
        int getInitiallySelectedItemId() const noexcept              { return initiallySelectedItemId; }

    private:
        //==============================================================================
        Rectangle<int> targetArea;
        WeakReference<Component> targetComponent, parentComponent, componentToWatchForDeletion, topLevelTarget;
        int visibleItemID = 0, minWidth = 0, minColumns = 1, maxColumns = 0, standardHeight = 0, initiallySelectedItemId = 0;
        bool isWatchingForDeletion = false;
        PopupDirection preferredPopupDirection = PopupDirection::downwards;
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
        @param callback                 if this is not a nullptr, the menu will be launched
                                        asynchronously, returning immediately, and the callback
                                        will receive a call when the menu is either dismissed or
                                        has an item selected. This object will be owned and
                                        deleted by the system, so make sure that it works safely
                                        and that any pointers that it uses are safely within scope.
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
    int showAt (Rectangle<int> screenAreaToAttachTo,
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

    /** Runs the menu asynchronously. */
    void showMenuAsync (const Options& options);

    /** Runs the menu asynchronously, with a user-provided callback that will receive the result. */
    void showMenuAsync (const Options& options,
                        ModalComponentManager::Callback* callback);

    /** Runs the menu asynchronously, with a user-provided callback that will receive the result. */
    void showMenuAsync (const Options& options,
                        std::function<void (int)> callback);

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

            @param menu                 the menu that needs to be scanned

            @param searchRecursively    if true, all submenus will be recursed into to
                                        do an exhaustive search
        */
        MenuItemIterator (const PopupMenu& menu, bool searchRecursively = false);

        /** Destructor. */
        ~MenuItemIterator();

        /** Returns true if there is another item, and sets up all this object's
            member variables to reflect that item's properties.
        */
        bool next();

        /** Returns a reference to the description of the current item.
            It is only valid to call this after next() has returned true!
        */
        Item& getItem() const;

    private:
        //==============================================================================
        bool searchRecursively;

        Array<int> index;
        Array<const PopupMenu*> menus;
        PopupMenu::Item* currentItem = nullptr;

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
        /** Creates a custom item that is triggered automatically. */
        CustomComponent();

        /** Creates a custom item.

            If isTriggeredAutomatically is true, then the menu will automatically detect
            a mouse-click on this component and use that to invoke the menu item. If it's
            false, then it's up to your class to manually trigger the item when it wants to.

            If isTriggeredAutomatically is true, then an accessibility handler 'wrapper'
            will be created for the item that allows pressing, focusing, and toggling.
            If isTriggeredAutomatically is false, and the item has no submenu, then
            no accessibility wrapper will be created and your component must be
            independently accessible.
        */
        explicit CustomComponent (bool isTriggeredAutomatically);

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

        /** Returns a pointer to the Item that holds this custom component, if this
            component is currently held by an Item.
            You can query the Item for information that you might want to use
            in your paint() method, such as the item's enabled and ticked states.
        */
        const PopupMenu::Item* getItem() const noexcept         { return item; }

        /** @internal */
        bool isTriggeredAutomatically() const noexcept          { return triggeredAutomatically; }
        /** @internal */
        void setHighlighted (bool shouldBeHighlighted);

    private:
        //==============================================================================
        bool isHighlighted = false, triggeredAutomatically;
        const PopupMenu::Item* item = nullptr;

        friend PopupMenu;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomComponent)
    };

    //==============================================================================
    /** A user-defined callback that can be used for specific items in a popup menu.
        @see PopupMenu::Item::customCallback
    */
    class JUCE_API  CustomCallback  : public SingleThreadedReferenceCountedObject
    {
    public:
        CustomCallback();
        ~CustomCallback() override;

        /** Callback to indicate this item has been triggered.
            @returns true if the itemID should be sent to the exitModalState method, or
                     false if it should send 0, indicating no further action should be taken
        */
        virtual bool menuItemTriggered() = 0;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomCallback)
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        menu drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        /** Fills the background of a popup menu component. */
        virtual void drawPopupMenuBackground (Graphics&, int width, int height);

        /** Fills the background of a popup menu component. */
        virtual void drawPopupMenuBackgroundWithOptions (Graphics&,
                                                         int width,
                                                         int height,
                                                         const Options&) = 0;

        /** Draws one of the items in a popup menu. */
        virtual void drawPopupMenuItem (Graphics&, const Rectangle<int>& area,
                                        bool isSeparator, bool isActive, bool isHighlighted,
                                        bool isTicked, bool hasSubMenu,
                                        const String& text,
                                        const String& shortcutKeyText,
                                        const Drawable* icon,
                                        const Colour* textColour);

        /** Draws one of the items in a popup menu. */
        virtual void drawPopupMenuItemWithOptions (Graphics&, const Rectangle<int>& area,
                                                   bool isHighlighted,
                                                   const Item& item,
                                                   const Options&) = 0;

        virtual void drawPopupMenuSectionHeader (Graphics&, const Rectangle<int>&,
                                                 const String&);

        virtual void drawPopupMenuSectionHeaderWithOptions (Graphics&, const Rectangle<int>& area,
                                                            const String& sectionName,
                                                            const Options&) = 0;

        /** Returns the size and style of font to use in popup menus. */
        virtual Font getPopupMenuFont() = 0;

        virtual void drawPopupMenuUpDownArrow (Graphics&,
                                               int width, int height,
                                               bool isScrollUpArrow);

        virtual void drawPopupMenuUpDownArrowWithOptions (Graphics&,
                                                          int width, int height,
                                                          bool isScrollUpArrow,
                                                          const Options&) = 0;

        /** Finds the best size for an item in a popup menu. */
        virtual void getIdealPopupMenuItemSize (const String& text,
                                                bool isSeparator,
                                                int standardMenuItemHeight,
                                                int& idealWidth,
                                                int& idealHeight);

        /** Finds the best size for an item in a popup menu. */
        virtual void getIdealPopupMenuItemSizeWithOptions (const String& text,
                                                           bool isSeparator,
                                                           int standardMenuItemHeight,
                                                           int& idealWidth,
                                                           int& idealHeight,
                                                           const Options&) = 0;

        virtual void getIdealPopupMenuSectionHeaderSizeWithOptions (const String& text,
                                                                    int standardMenuItemHeight,
                                                                    int& idealWidth,
                                                                    int& idealHeight,
                                                                    const Options&) = 0;

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

        virtual Component* getParentComponentForMenuOptions (const Options& options) = 0;

        virtual void preparePopupMenuWindow (Component& newWindow) = 0;

        /** Return true if you want your popup menus to scale with the target component's AffineTransform
            or scale factor
        */
        virtual bool shouldPopupMenuScaleWithTargetComponent (const Options& options) = 0;

        virtual int getPopupMenuBorderSize();

        virtual int getPopupMenuBorderSizeWithOptions (const Options&) = 0;

        /** Implement this to draw some custom decoration between the columns of the popup menu.

            `getPopupMenuColumnSeparatorWidthWithOptions` must return a positive value in order
            to display the separator.
        */
        virtual void drawPopupMenuColumnSeparatorWithOptions (Graphics& g,
                                                              const Rectangle<int>& bounds,
                                                              const Options&) = 0;

        /** Return the amount of space that should be left between popup menu columns. */
        virtual int getPopupMenuColumnSeparatorWidthWithOptions (const Options&) = 0;
    };

    //==============================================================================
    /** @cond */
    [[deprecated ("Use the new method.")]]
    int drawPopupMenuItem (Graphics&, int, int, bool, bool, bool, bool, bool, const String&, const String&, Image*, const Colour*) { return 0; }
    /** @endcond */

private:
    //==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (struct HelperClasses)
    class Window;
    friend struct HelperClasses;
    friend class MenuBarComponent;

    Array<Item> items;
    WeakReference<LookAndFeel> lookAndFeel;

    Component* createWindow (const Options&, ApplicationCommandManager**) const;
    int showWithOptionalCallback (const Options&, ModalComponentManager::Callback*, bool);

    static void setItem (CustomComponent&, const Item*);

    JUCE_LEAK_DETECTOR (PopupMenu)
};

} // namespace juce
