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
/**
    A class for controlling MenuBar components.

    This class is used to tell a MenuBar what menus to show, and to respond
    to a menu being selected.

    @see MenuBarModel::Listener, MenuBarComponent, PopupMenu

    @tags{GUI}
*/
class JUCE_API  MenuBarModel      : private AsyncUpdater,
                                    private ApplicationCommandManagerListener
{
public:
    //==============================================================================
    MenuBarModel() noexcept;

    /** Destructor. */
    ~MenuBarModel() override;

    //==============================================================================
    /** Call this when some of your menu items have changed.

        This method will cause a callback to any MenuBarListener objects that
        are registered with this model.

        If this model is displaying items from an ApplicationCommandManager, you
        can use the setApplicationCommandManagerToWatch() method to cause
        change messages to be sent automatically when the ApplicationCommandManager
        is changed.

        @see addListener, removeListener, MenuBarListener
    */
    void menuItemsChanged();

    /** Tells the menu bar to listen to the specified command manager, and to update
        itself when the commands change.

        This will also allow it to flash a menu name when a command from that menu
        is invoked using a keystroke.
    */
    void setApplicationCommandManagerToWatch (ApplicationCommandManager* manager);

    //==============================================================================
    /** A class to receive callbacks when a MenuBarModel changes.

        @see MenuBarModel::addListener, MenuBarModel::removeListener, MenuBarModel::menuItemsChanged
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        //==============================================================================
        /** This callback is made when items are changed in the menu bar model. */
        virtual void menuBarItemsChanged (MenuBarModel* menuBarModel) = 0;

        /** This callback is made when an application command is invoked that
            is represented by one of the items in the menu bar model.
        */
        virtual void menuCommandInvoked (MenuBarModel* menuBarModel,
                                         const ApplicationCommandTarget::InvocationInfo& info) = 0;

        /** Called when the menu bar is first activated or when the user finished interacting
            with the menu bar. */
        virtual void menuBarActivated (MenuBarModel* menuBarModel, bool isActive);
    };

    /** Registers a listener for callbacks when the menu items in this model change.

        The listener object will get callbacks when this object's menuItemsChanged()
        method is called.

        @see removeListener
    */
    void addListener (Listener* listenerToAdd);

    /** Removes a listener.
        @see addListener
    */
    void removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** This method must return a list of the names of the menus. */
    virtual StringArray getMenuBarNames() = 0;

    /** This should return the popup menu to display for a given top-level menu.

        @param topLevelMenuIndex    the index of the top-level menu to show
        @param menuName             the name of the top-level menu item to show
    */
    virtual PopupMenu getMenuForIndex (int topLevelMenuIndex,
                                       const String& menuName) = 0;

    /** This is called when a menu item has been clicked on.

        @param menuItemID           the item ID of the PopupMenu item that was selected
        @param topLevelMenuIndex    the index of the top-level menu from which the item was
                                    chosen (just in case you've used duplicate ID numbers
                                    on more than one of the popup menus)
    */
    virtual void menuItemSelected (int menuItemID,
                                   int topLevelMenuIndex) = 0;

    /** This is called when the user starts/stops navigating the menu bar.

        @param isActive              true when the user starts navigating the menu bar
    */
    virtual void menuBarActivated (bool isActive);

    //==============================================================================
   #if JUCE_MAC || DOXYGEN
    /** OSX ONLY - Sets the model that is currently being shown as the main
        menu bar at the top of the screen on the Mac.

        You can pass nullptr to stop the current model being displayed. Be careful
        not to delete a model while it is being used.

        An optional extra menu can be specified, containing items to add to the top of
        the apple menu. (Confusingly, the 'apple' menu isn't the one with a picture of
        an apple, it's the one next to it, with your application's name at the top
        and the services menu etc on it). When one of these items is selected, the
        menu bar model will be used to invoke it, and in the menuItemSelected() callback
        the topLevelMenuIndex parameter will be -1. If you pass in an extraAppleMenuItems
        object then newMenuBarModel must be non-null.

        If the recentItemsMenuName parameter is non-empty, then any sub-menus with this
        name will be replaced by OSX's special recent-files menu.
    */
    static void setMacMainMenu (MenuBarModel* newMenuBarModel,
                                const PopupMenu* extraAppleMenuItems = nullptr,
                                const String& recentItemsMenuName = String());

    /** OSX ONLY - Returns the menu model that is currently being shown as
        the main menu bar.
    */
    static MenuBarModel* getMacMainMenu();

    /** OSX ONLY - Returns the menu that was last passed as the extraAppleMenuItems
        argument to setMacMainMenu(), or nullptr if none was specified.
    */
    static const PopupMenu* getMacExtraAppleItemsMenu();
   #endif

    //==============================================================================
    /** @internal */
    void applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo&) override;
    /** @internal */
    void applicationCommandListChanged() override;
    /** @internal */
    void handleAsyncUpdate() override;
    /** @internal */
    void handleMenuBarActivate (bool isActive);
private:
    ApplicationCommandManager* manager;
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuBarModel)
};


} // namespace juce
