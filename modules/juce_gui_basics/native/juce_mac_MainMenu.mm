/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
const auto menuItemInvokedSelector = @selector (menuItemInvoked:);
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

//==============================================================================
struct JuceMainMenuBarHolder : private DeletedAtShutdown
{
    JuceMainMenuBarHolder()
        : mainMenuBar ([[NSMenu alloc] initWithTitle: nsStringLiteral ("MainMenu")])
    {
        auto item = [mainMenuBar addItemWithTitle: nsStringLiteral ("Apple")
                                           action: nil
                                     keyEquivalent: nsEmptyString()];

        auto appMenu = [[NSMenu alloc] initWithTitle: nsStringLiteral ("Apple")];

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [NSApp performSelector: @selector (setAppleMenu:) withObject: appMenu];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        [mainMenuBar setSubmenu: appMenu forItem: item];
        [appMenu release];

        [NSApp setMainMenu: mainMenuBar];
    }

    ~JuceMainMenuBarHolder()
    {
        clearSingletonInstance();

        [NSApp setMainMenu: nil];
        [mainMenuBar release];
    }

    NSMenu* mainMenuBar = nil;

    JUCE_DECLARE_SINGLETON_SINGLETHREADED (JuceMainMenuBarHolder, true)
};

JUCE_IMPLEMENT_SINGLETON (JuceMainMenuBarHolder)

//==============================================================================
class JuceMainMenuHandler   : private MenuBarModel::Listener,
                              private DeletedAtShutdown
{
public:
    JuceMainMenuHandler()
    {
        static JuceMenuCallbackClass cls;
        callback = [cls.createInstance() init];
        JuceMenuCallbackClass::setOwner (callback, this);
    }

    ~JuceMainMenuHandler() override
    {
        setMenu (nullptr, nullptr, String());

        jassert (instance == this);
        instance = nullptr;

        [callback release];
    }

    void setMenu (MenuBarModel* const newMenuBarModel,
                  const PopupMenu* newExtraAppleMenuItems,
                  const String& recentItemsName)
    {
        recentItemsMenuName = recentItemsName;

        if (currentModel != newMenuBarModel)
        {
            if (currentModel != nullptr)
                currentModel->removeListener (this);

            currentModel = newMenuBarModel;

            if (currentModel != nullptr)
                currentModel->addListener (this);

            menuBarItemsChanged (nullptr);
        }

        extraAppleMenuItems.reset (createCopyIfNotNull (newExtraAppleMenuItems));
    }

    void addTopLevelMenu (NSMenu* parent, const PopupMenu& child, const String& name, int menuId, int topLevelIndex)
    {
        NSMenuItem* item = [parent addItemWithTitle: juceStringToNS (name)
                                             action: nil
                                      keyEquivalent: nsEmptyString()];

        NSMenu* sub = createMenu (child, name, menuId, topLevelIndex, true);

        [parent setSubmenu: sub forItem: item];
        [sub release];
    }

    void updateTopLevelMenu (NSMenuItem* parentItem, const PopupMenu& menuToCopy, const String& name, int menuId, int topLevelIndex)
    {
        // Note: This method used to update the contents of the existing menu in-place, but that caused
        // weird side-effects which messed-up keyboard focus when switching between windows. By creating
        // a new menu and replacing the old one with it, that problem seems to be avoided..
        NSMenu* menu = [[NSMenu alloc] initWithTitle: juceStringToNS (name)];

        for (PopupMenu::MenuItemIterator iter (menuToCopy); iter.next();)
            addMenuItem (iter, menu, menuId, topLevelIndex);

        [menu update];

        removeItemRecursive ([parentItem submenu]);
        [parentItem setSubmenu: menu];

        [menu release];
    }

    void updateTopLevelMenu (NSMenu* menu)
    {
        NSMenu* superMenu = [menu supermenu];
        auto menuNames = currentModel->getMenuBarNames();
        auto indexOfMenu = (int) [superMenu indexOfItemWithSubmenu: menu] - 1;

        if (indexOfMenu >= 0)
        {
            removeItemRecursive (menu);

            auto updatedPopup = currentModel->getMenuForIndex (indexOfMenu, menuNames[indexOfMenu]);

            for (PopupMenu::MenuItemIterator iter (updatedPopup); iter.next();)
                addMenuItem (iter, menu, 1, indexOfMenu);

            [menu update];
        }
    }

    void menuBarItemsChanged (MenuBarModel*) override
    {
        if (isOpen)
        {
            defferedUpdateRequested = true;
            return;
        }

        lastUpdateTime = Time::getMillisecondCounter();

        StringArray menuNames;
        if (currentModel != nullptr)
            menuNames = currentModel->getMenuBarNames();

        auto* menuBar = getMainMenuBar();

        while ([menuBar numberOfItems] > 1 + menuNames.size())
            removeItemRecursive (menuBar, static_cast<int> ([menuBar numberOfItems] - 1));

        int menuId = 1;

        for (int i = 0; i < menuNames.size(); ++i)
        {
            const PopupMenu menu (currentModel->getMenuForIndex (i, menuNames[i]));

            if (i >= [menuBar numberOfItems] - 1)
                addTopLevelMenu (menuBar, menu, menuNames[i], menuId, i);
            else
                updateTopLevelMenu ([menuBar itemAtIndex: 1 + i], menu, menuNames[i], menuId, i);
        }
    }

    void menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo& info) override
    {
        if ((info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) == 0
              && info.invocationMethod != ApplicationCommandTarget::InvocationInfo::fromKeyPress)
            if (auto* item = findMenuItemWithCommandID (getMainMenuBar(), info.commandID))
                flashMenuBar ([item menu]);
    }

    void invoke (const PopupMenu::Item& item, int topLevelIndex) const
    {
        if (currentModel != nullptr)
        {
            if (item.action != nullptr)
            {
                MessageManager::callAsync (item.action);
                return;
            }

            if (item.customCallback != nullptr)
                if (! item.customCallback->menuItemTriggered())
                    return;

            if (item.commandManager != nullptr)
            {
                ApplicationCommandTarget::InvocationInfo info (item.itemID);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

                item.commandManager->invoke (info, true);
            }

            MessageManager::callAsync ([=]
            {
                if (instance != nullptr)
                    instance->invokeDirectly (item.itemID, topLevelIndex);
            });
        }
    }

    void invokeDirectly (int commandId, int topLevelIndex)
    {
        if (currentModel != nullptr)
            currentModel->menuItemSelected (commandId, topLevelIndex);
    }

    void addMenuItem (PopupMenu::MenuItemIterator& iter, NSMenu* menuToAddTo,
                      const int topLevelMenuId, const int topLevelIndex)
    {
        const PopupMenu::Item& i = iter.getItem();
        NSString* text = juceStringToNS (i.text);

        if (text == nil)
            text = nsEmptyString();

        if (i.isSeparator)
        {
            [menuToAddTo addItem: [NSMenuItem separatorItem]];
        }
        else if (i.isSectionHeader)
        {
            NSMenuItem* item = [menuToAddTo addItemWithTitle: text
                                                      action: nil
                                               keyEquivalent: nsEmptyString()];

            [item setEnabled: false];
        }
        else if (i.subMenu != nullptr)
        {
            if (recentItemsMenuName.isNotEmpty() && i.text == recentItemsMenuName)
            {
                if (recent == nullptr)
                    recent = std::make_unique<RecentFilesMenuItem>();

                if (recent->recentItem != nil)
                {
                    if (NSMenu* parent = [recent->recentItem menu])
                        [parent removeItem: recent->recentItem];

                    [menuToAddTo addItem: recent->recentItem];
                    return;
                }
            }

            NSMenuItem* item = [menuToAddTo addItemWithTitle: text
                                                      action: nil
                                               keyEquivalent: nsEmptyString()];

            [item setTag: i.itemID];
            [item setEnabled: i.isEnabled];

            NSMenu* sub = createMenu (*i.subMenu, i.text, topLevelMenuId, topLevelIndex, false);
            [menuToAddTo setSubmenu: sub forItem: item];
            [sub release];
        }
        else
        {
            auto item = [[NSMenuItem alloc] initWithTitle: text
                                                   action: menuItemInvokedSelector
                                            keyEquivalent: nsEmptyString()];

            [item setTag: topLevelIndex];
            [item setEnabled: i.isEnabled];
            [item setState: i.isTicked ? NSControlStateValueOn : NSControlStateValueOff];
            [item setTarget: (id) callback];

            auto* juceItem = new PopupMenu::Item (i);
            juceItem->customComponent = nullptr;

            [item setRepresentedObject: [createNSObjectFromJuceClass (juceItem) autorelease]];

            if (i.commandManager != nullptr)
            {
                for (auto& kp : i.commandManager->getKeyMappings()->getKeyPressesAssignedToCommand (i.itemID))
                {
                    if (kp != KeyPress::backspaceKey   // (adding these is annoying because it flashes the menu bar
                         && kp != KeyPress::deleteKey) // every time you press the key while editing text)
                    {
                        juce_wchar key = kp.getTextCharacter();

                        if (key == 0)
                            key = (juce_wchar) kp.getKeyCode();

                        [item setKeyEquivalent: juceStringToNS (String::charToString (key).toLowerCase())];
                        [item setKeyEquivalentModifierMask: juceModsToNSMods (kp.getModifiers())];
                    }

                    break;
                }
            }

            [menuToAddTo addItem: item];
            [item release];
        }
    }

    NSMenu* createMenu (const PopupMenu menu,
                        const String& menuName,
                        const int topLevelMenuId,
                        const int topLevelIndex,
                        const bool addDelegate)
    {
        NSMenu* m = [[NSMenu alloc] initWithTitle: juceStringToNS (menuName)];

        if (addDelegate)
            [m setDelegate: (id<NSMenuDelegate>) callback];

        for (PopupMenu::MenuItemIterator iter (menu); iter.next();)
            addMenuItem (iter, m, topLevelMenuId, topLevelIndex);

        [m update];
        return m;
    }

    static JuceMainMenuHandler* instance;

    MenuBarModel* currentModel = nullptr;
    std::unique_ptr<PopupMenu> extraAppleMenuItems;
    uint32 lastUpdateTime = 0;
    NSObject* callback = nil;
    String recentItemsMenuName;
    bool isOpen = false, defferedUpdateRequested = false;

private:
    struct RecentFilesMenuItem
    {
        RecentFilesMenuItem() : recentItem (nil)
        {
            if (NSNib* menuNib = [[[NSNib alloc] initWithNibNamed: @"RecentFilesMenuTemplate" bundle: nil] autorelease])
            {
                NSArray* array = nil;

                if (@available (macOS 10.11, *))
                {
                    [menuNib instantiateWithOwner: NSApp
                                  topLevelObjects: &array];
                }
                else
                {
                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
                    [menuNib instantiateNibWithOwner: NSApp
                                     topLevelObjects: &array];
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
                }

                for (id object in array)
                {
                    if ([object isKindOfClass: [NSMenu class]])
                    {
                        if (NSArray* items = [object itemArray])
                        {
                            if (NSMenuItem* item = findRecentFilesItem (items))
                            {
                                recentItem = [item retain];
                                break;
                            }
                        }
                    }
                }
            }
        }

        ~RecentFilesMenuItem()
        {
            [recentItem release];
        }

        static NSMenuItem* findRecentFilesItem (NSArray* const items)
        {
            for (id object in items)
                if (NSArray* subMenuItems = [[object submenu] itemArray])
                    for (id subObject in subMenuItems)
                        if ([subObject isKindOfClass: [NSMenuItem class]])
                            return subObject;
            return nil;
        }

        NSMenuItem* recentItem;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecentFilesMenuItem)
    };

    std::unique_ptr<RecentFilesMenuItem> recent;

    //==============================================================================
    static NSMenuItem* findMenuItemWithCommandID (NSMenu* const menu, int commandID)
    {
        for (NSInteger i = [menu numberOfItems]; --i >= 0;)
        {
            NSMenuItem* m = [menu itemAtIndex: i];
            if (auto* menuItem = getJuceClassFromNSObject<PopupMenu::Item> ([m representedObject]))
                if (menuItem->itemID == commandID)
                    return m;

            if (NSMenu* sub = [m submenu])
                if (NSMenuItem* found = findMenuItemWithCommandID (sub, commandID))
                    return found;
        }

        return nil;
    }

    static void flashMenuBar (NSMenu* menu)
    {
        if ([[menu title] isEqualToString: nsStringLiteral ("Apple")])
            return;

        [menu retain];

        const unichar f35Key = NSF35FunctionKey;
        NSString* f35String = [NSString stringWithCharacters: &f35Key length: 1];

        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle: nsStringLiteral ("x")
                                                      action: menuItemInvokedSelector
                                               keyEquivalent: f35String];

        // When the f35Event is invoked, the item's enablement is checked and a
        // NSBeep is triggered if the item appears to be disabled.
        // This ValidatorClass exists solely to return YES from validateMenuItem.
        struct ValidatorClass   : public ObjCClass<NSObject>
        {
            ValidatorClass()  : ObjCClass ("JUCEMenuValidator_")
            {
                addMethod (menuItemInvokedSelector,       menuItemInvoked);
                addMethod (@selector (validateMenuItem:), validateMenuItem);
                addProtocol (@protocol (NSMenuItemValidation));

                registerClass();
            }

        private:
            static BOOL validateMenuItem (id, SEL, NSMenuItem*)      { return YES; }
            static void menuItemInvoked  (id, SEL, NSMenuItem*)      {}
        };

        static ValidatorClass validatorClass;
        static auto* instance = validatorClass.createInstance();

        [item setTarget: instance];
        [menu insertItem: item atIndex: [menu numberOfItems]];
        [item release];

        if ([menu indexOfItem: item] >= 0)
        {
            NSEvent* f35Event = [NSEvent keyEventWithType: NSEventTypeKeyDown
                                                 location: NSZeroPoint
                                            modifierFlags: NSEventModifierFlagCommand
                                                timestamp: 0
                                             windowNumber: 0
                                                  context: [NSGraphicsContext currentContext]
                                               characters: f35String
                              charactersIgnoringModifiers: f35String
                                                isARepeat: NO
                                                  keyCode: 0];

            [menu performKeyEquivalent: f35Event];

            if ([menu indexOfItem: item] >= 0)
                [menu removeItem: item]; // (this throws if the item isn't actually in the menu)
        }

        [menu release];
    }

    static unsigned int juceModsToNSMods (const ModifierKeys mods)
    {
        unsigned int m = 0;
        if (mods.isShiftDown())    m |= NSEventModifierFlagShift;
        if (mods.isCtrlDown())     m |= NSEventModifierFlagControl;
        if (mods.isAltDown())      m |= NSEventModifierFlagOption;
        if (mods.isCommandDown())  m |= NSEventModifierFlagCommand;
        return m;
    }

    // Apple Bug: For some reason [NSMenu removeAllItems] seems to leak it's objects
    // on shutdown, so we need this method to release the items one-by-one manually
    static void removeItemRecursive (NSMenu* parentMenu, int menuItemIndex)
    {
        if (isPositiveAndBelow (menuItemIndex, (int) [parentMenu numberOfItems]))
        {
            auto menuItem = [parentMenu itemAtIndex:menuItemIndex];

            if (auto submenu = [menuItem submenu])
                removeItemRecursive (submenu);

            [parentMenu removeItem:menuItem];
        }
        else
            jassertfalse;
    }

    static void removeItemRecursive (NSMenu* menu)
    {
        if (menu != nullptr)
        {
            auto n = static_cast<int> ([menu numberOfItems]);

            for (auto i = n; --i >= 0;)
                removeItemRecursive (menu, i);
        }
    }

    static NSMenu* getMainMenuBar()
    {
        return JuceMainMenuBarHolder::getInstance()->mainMenuBar;
    }

    //==============================================================================
    struct JuceMenuCallbackClass   : public ObjCClass<NSObject>
    {
        JuceMenuCallbackClass()  : ObjCClass ("JUCEMainMenu_")
        {
            addIvar<JuceMainMenuHandler*> ("owner");

            addMethod (menuItemInvokedSelector,       menuItemInvoked);
            addMethod (@selector (menuNeedsUpdate:),  menuNeedsUpdate);
            addMethod (@selector (validateMenuItem:), validateMenuItem);

            addProtocol (@protocol (NSMenuDelegate));
            addProtocol (@protocol (NSMenuItemValidation));

            registerClass();
        }

        static void setOwner (id self, JuceMainMenuHandler* owner)
        {
            object_setInstanceVariable (self, "owner", owner);
        }

    private:
        static auto* getPopupMenuItem (NSMenuItem* item)
        {
            return getJuceClassFromNSObject<PopupMenu::Item> ([item representedObject]);
        }

        static auto* getOwner (id self)
        {
            return getIvar<JuceMainMenuHandler*> (self, "owner");
        }

        static void menuItemInvoked (id self, SEL, NSMenuItem* item)
        {
            if (auto* juceItem = getPopupMenuItem (item))
                getOwner (self)->invoke (*juceItem, static_cast<int> ([item tag]));
        }

        static void menuNeedsUpdate (id self, SEL, NSMenu* menu)
        {
            getOwner (self)->updateTopLevelMenu (menu);
        }

        static BOOL validateMenuItem (id, SEL, NSMenuItem* item)
        {
            if (auto* juceItem = getPopupMenuItem (item))
                return juceItem->isEnabled;

            return YES;
        }
    };
};

JuceMainMenuHandler* JuceMainMenuHandler::instance = nullptr;

//==============================================================================
class TemporaryMainMenuWithStandardCommands
{
public:
    explicit TemporaryMainMenuWithStandardCommands (FilePreviewComponent* filePreviewComponent)
        : oldMenu (MenuBarModel::getMacMainMenu()), dummyModalComponent (filePreviewComponent)
    {
        if (auto* appleMenu = MenuBarModel::getMacExtraAppleItemsMenu())
            oldAppleMenu = std::make_unique<PopupMenu> (*appleMenu);

        if (auto* handler = JuceMainMenuHandler::instance)
            oldRecentItems = handler->recentItemsMenuName;

        MenuBarModel::setMacMainMenu (nullptr);

        if (auto* mainMenu = JuceMainMenuBarHolder::getInstance()->mainMenuBar)
        {
            NSMenu* menu = [[NSMenu alloc] initWithTitle: nsStringLiteral ("Edit")];
            NSMenuItem* item;

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Cut"), nil)
                                              action: @selector (cut:)  keyEquivalent: nsStringLiteral ("x")];
            [menu addItem: item];
            [item release];

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Copy"), nil)
                                              action: @selector (copy:)  keyEquivalent: nsStringLiteral ("c")];
            [menu addItem: item];
            [item release];

            item = [[NSMenuItem alloc] initWithTitle: NSLocalizedString (nsStringLiteral ("Paste"), nil)
                                              action: @selector (paste:)  keyEquivalent: nsStringLiteral ("v")];
            [menu addItem: item];
            [item release];

            editMenuIndex = [mainMenu numberOfItems];

            item = [mainMenu addItemWithTitle: NSLocalizedString (nsStringLiteral ("Edit"), nil)
                                       action: nil  keyEquivalent: nsEmptyString()];
            [mainMenu setSubmenu: menu forItem: item];
            [menu release];
        }

        // use a dummy modal component so that apps can tell that something is currently modal.
        dummyModalComponent.enterModalState (false);
    }

    ~TemporaryMainMenuWithStandardCommands()
    {
        if (auto* mainMenu = JuceMainMenuBarHolder::getInstance()->mainMenuBar)
            [mainMenu removeItemAtIndex:editMenuIndex];

        MenuBarModel::setMacMainMenu (oldMenu, oldAppleMenu.get(), oldRecentItems);
    }

    static bool checkModalEvent (FilePreviewComponent* preview, const Component* targetComponent)
    {
        if (targetComponent == nullptr)
            return false;

        return (targetComponent == preview
               || targetComponent->findParentComponentOfClass<FilePreviewComponent>() != nullptr);
    }

private:
    MenuBarModel* const oldMenu = nullptr;
    std::unique_ptr<PopupMenu> oldAppleMenu;
    String oldRecentItems;
    NSInteger editMenuIndex;

    // The OS view already plays an alert when clicking outside
    // the modal comp, so this override avoids adding extra
    // inappropriate noises when the cancel button is pressed.
    // This override is also important because it stops the base class
    // calling ModalComponentManager::bringToFront, which can get
    // recursive when file dialogs are involved
    struct SilentDummyModalComp  : public Component
    {
        explicit SilentDummyModalComp (FilePreviewComponent* p)
            : preview (p) {}

        void inputAttemptWhenModal() override {}

        bool canModalEventBeSentToComponent (const Component* targetComponent) override
        {
            return checkModalEvent (preview, targetComponent);
        }

        FilePreviewComponent* preview = nullptr;
    };

    SilentDummyModalComp dummyModalComponent;
};

//==============================================================================
namespace MainMenuHelpers
{
    static NSString* translateMenuName (const String& name)
    {
        return NSLocalizedString (juceStringToNS (TRANS (name)), nil);
    }

    static NSMenuItem* createMenuItem (NSMenu* menu, const String& name, SEL sel, NSString* key)
    {
        NSMenuItem* item = [[[NSMenuItem alloc] initWithTitle: translateMenuName (name)
                                                       action: sel
                                                keyEquivalent: key] autorelease];
        [item setTarget: NSApp];
        [menu addItem: item];
        return item;
    }

    static void createStandardAppMenu (NSMenu* menu, const String& appName, const PopupMenu* extraItems)
    {
        if (extraItems != nullptr && JuceMainMenuHandler::instance != nullptr && extraItems->getNumItems() > 0)
        {
            for (PopupMenu::MenuItemIterator iter (*extraItems); iter.next();)
                JuceMainMenuHandler::instance->addMenuItem (iter, menu, 0, -1);

            [menu addItem: [NSMenuItem separatorItem]];
        }

        // Services...
        NSMenuItem* services = [[[NSMenuItem alloc] initWithTitle: translateMenuName ("Services")
                                                           action: nil  keyEquivalent: nsEmptyString()] autorelease];
        [menu addItem: services];

        NSMenu* servicesMenu = [[[NSMenu alloc] initWithTitle: translateMenuName ("Services")] autorelease];
        [menu setSubmenu: servicesMenu forItem: services];
        [NSApp setServicesMenu: servicesMenu];
        [menu addItem: [NSMenuItem separatorItem]];

        createMenuItem (menu, TRANS("Hide") + String (" ") + appName, @selector (hide:), nsStringLiteral ("h"));

        [createMenuItem (menu, TRANS("Hide Others"), @selector (hideOtherApplications:), nsStringLiteral ("h"))
            setKeyEquivalentModifierMask: NSEventModifierFlagCommand | NSEventModifierFlagOption];

        createMenuItem (menu, TRANS("Show All"), @selector (unhideAllApplications:), nsEmptyString());

        [menu addItem: [NSMenuItem separatorItem]];

        createMenuItem (menu, TRANS("Quit") + String (" ") + appName, @selector (terminate:), nsStringLiteral ("q"));
    }

    // Since our app has no NIB, this initialises a standard app menu...
    static void rebuildMainMenu (const PopupMenu* extraItems)
    {
        // this can't be used in a plugin!
        jassert (JUCEApplicationBase::isStandaloneApp());

        if (auto* app = JUCEApplicationBase::getInstance())
        {
            if (auto* mainMenu = JuceMainMenuBarHolder::getInstance()->mainMenuBar)
            {
                if ([mainMenu numberOfItems] > 0)
                {
                    if (auto appMenu = [[mainMenu itemAtIndex: 0] submenu])
                    {
                        [appMenu removeAllItems];
                        MainMenuHelpers::createStandardAppMenu (appMenu, app->getApplicationName(), extraItems);
                    }
                }
            }
        }
    }
}

void MenuBarModel::setMacMainMenu (MenuBarModel* newMenuBarModel,
                                   const PopupMenu* extraAppleMenuItems,
                                   const String& recentItemsMenuName)
{
    if (getMacMainMenu() != newMenuBarModel)
    {
        JUCE_AUTORELEASEPOOL
        {
            if (newMenuBarModel == nullptr)
            {
                delete JuceMainMenuHandler::instance;
                jassert (JuceMainMenuHandler::instance == nullptr); // should be zeroed in the destructor
                jassert (extraAppleMenuItems == nullptr); // you can't specify some extra items without also supplying a model

                extraAppleMenuItems = nullptr;
            }
            else
            {
                if (JuceMainMenuHandler::instance == nullptr)
                    JuceMainMenuHandler::instance = new JuceMainMenuHandler();

                JuceMainMenuHandler::instance->setMenu (newMenuBarModel, extraAppleMenuItems, recentItemsMenuName);
            }
        }
    }

    MainMenuHelpers::rebuildMainMenu (extraAppleMenuItems);

    if (newMenuBarModel != nullptr)
        newMenuBarModel->menuItemsChanged();
}

MenuBarModel* MenuBarModel::getMacMainMenu()
{
    if (auto* mm = JuceMainMenuHandler::instance)
        return mm->currentModel;

    return nullptr;
}

const PopupMenu* MenuBarModel::getMacExtraAppleItemsMenu()
{
    if (auto* mm = JuceMainMenuHandler::instance)
        return mm->extraAppleMenuItems.get();

    return nullptr;
}

using MenuTrackingChangedCallback = void (*)(bool);
extern MenuTrackingChangedCallback menuTrackingChangedCallback;

static void mainMenuTrackingChanged (bool isTracking)
{
    PopupMenu::dismissAllActiveMenus();

    if (auto* menuHandler = JuceMainMenuHandler::instance)
    {
        menuHandler->isOpen = isTracking;

        if (auto* model = menuHandler->currentModel)
            model->handleMenuBarActivate (isTracking);

        if (menuHandler->defferedUpdateRequested && ! isTracking)
        {
            menuHandler->defferedUpdateRequested = false;
            menuHandler->menuBarItemsChanged (menuHandler->currentModel);
        }
    }
}

void juce_initialiseMacMainMenu()
{
    menuTrackingChangedCallback = mainMenuTrackingChanged;

    if (JuceMainMenuHandler::instance == nullptr)
        MainMenuHelpers::rebuildMainMenu (nullptr);
}

// (used from other modules that need to create an NSMenu)
NSMenu* createNSMenu (const PopupMenu&, const String&, int, int, bool);
NSMenu* createNSMenu (const PopupMenu& menu, const String& name, int topLevelMenuId, int topLevelIndex, bool addDelegate)
{
    juce_initialiseMacMainMenu();

    if (auto* mm = JuceMainMenuHandler::instance)
        return mm->createMenu (menu, name, topLevelMenuId, topLevelIndex, addDelegate);

    jassertfalse; // calling this before making sure the OSX main menu stuff was initialised?
    return nil;
}

} // namespace juce
