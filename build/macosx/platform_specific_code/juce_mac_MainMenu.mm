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

// (This file gets included by juce_mac_NativeCode.mm, rather than being 
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

//==============================================================================
class JuceMainMenuHandler;

END_JUCE_NAMESPACE
using namespace JUCE_NAMESPACE;

@interface JuceMenuCallback  : NSObject
{
    JuceMainMenuHandler* owner;
}

- (JuceMenuCallback*) initWithOwner: (JuceMainMenuHandler*) owner_;
- (void) dealloc;
- (void) menuItemInvoked: (id) menu;

@end
BEGIN_JUCE_NAMESPACE


//==============================================================================
class JuceMainMenuHandler   : private MenuBarModelListener,
                              private DeletedAtShutdown
{
public:
    static JuceMainMenuHandler* instance;

    //==============================================================================
    JuceMainMenuHandler() throw()
        : currentModel (0)
    {
        callback = [[JuceMenuCallback alloc] initWithOwner: this];
    }

    ~JuceMainMenuHandler() throw()
    {
        setMenu (0);
        
        jassert (instance == this);
        instance = 0;
        
        [callback release];
    }

    void setMenu (MenuBarModel* const newMenuBarModel) throw()
    {
        if (currentModel != newMenuBarModel)
        {
            if (currentModel != 0)
                currentModel->removeListener (this);

            currentModel = newMenuBarModel;

            if (currentModel != 0)
                currentModel->addListener (this);
            
            menuBarItemsChanged (0);
        }
    }

    void addSubMenu (NSMenu* parent, const PopupMenu& child, 
                     const String& name, int& menuId, int& tag)
    {
        NSMenuItem* item = [parent addItemWithTitle: juceStringToNS (name)
                                             action: nil
                                      keyEquivalent: @""];
        [item setTag: tag];

        NSMenu* sub = createMenu (child, name, menuId, tag);
        
        [parent setSubmenu: sub forItem: item];
        [sub setAutoenablesItems: false];
        [sub release];
    }

    void menuBarItemsChanged (MenuBarModel*)
    {
        NSMenu* menuBar = [NSApp mainMenu];
        while ([menuBar numberOfItems] > 1)
            [menuBar removeItemAtIndex: 1];

        if (currentModel != 0)
        {
            const StringArray menuNames (currentModel->getMenuBarNames());
            int menuId = 1;

            for (int i = 0; i < menuNames.size(); ++i)
            {
                const PopupMenu menu (currentModel->getMenuForIndex (i, menuNames [i]));
                addSubMenu (menuBar, menu, menuNames [i], menuId, i);
            }
        }
    }

    static void flashMenuBar (NSMenu* menu)
    {
        const unichar f35Key = NSF35FunctionKey;
        NSString* f35String = [NSString stringWithCharacters: &f35Key length: 1];

        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle: @"x"
                                                      action: nil
                                               keyEquivalent: f35String];
        [item setTarget: nil];
        [menu insertItem: item atIndex: [menu numberOfItems]];
        [item release];

        NSEvent* f35Event = [NSEvent keyEventWithType: NSKeyDown
                                             location: NSZeroPoint
                                        modifierFlags: NSCommandKeyMask
                                            timestamp: 0
                                         windowNumber: 0
                                              context: [NSGraphicsContext currentContext]
                                           characters: f35String
                          charactersIgnoringModifiers: f35String
                                            isARepeat: NO
                                              keyCode: 0];
        
        [menu performKeyEquivalent: f35Event];
        [menu removeItem: item];
    }

    void menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo& info)
    {
        NSMenuItem* item = [[NSApp mainMenu] itemWithTag: info.commandID];
        
        if (item != 0)
            flashMenuBar ([item menu]);
    }

    void invoke (const int commandId, ApplicationCommandManager* const commandManager, const int topLevelIndex) const
    {
        if (currentModel != 0)
        {
            if (commandManager != 0)
            {
                ApplicationCommandTarget::InvocationInfo info (commandId);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;
                
                commandManager->invoke (info, true);
            }

            currentModel->menuItemSelected (commandId, topLevelIndex);
        }
    }
    
    MenuBarModel* currentModel;

private:
    JuceMenuCallback* callback;

    NSMenu* createMenu (const PopupMenu menu,
                        const String& menuName,
                        int& id,
                        const int topLevelIndex)
    {
        NSMenu* m = [[NSMenu alloc] initWithTitle: juceStringToNS (menuName)];

        [m setAutoenablesItems: false];

        PopupMenu::MenuItemIterator iter (menu);

        while (iter.next())
        {
            NSString* text = juceStringToNS (iter.itemName.upToFirstOccurrenceOf (T("<end>"), false, true));

            if (iter.isSeparator)
            {
                [m addItem: [NSMenuItem separatorItem]];
            }
            else if (iter.isSectionHeader)
            {
                NSMenuItem* item = [m addItemWithTitle: text
                                                action: nil
                                         keyEquivalent: @""];

                [item setEnabled: iter.isEnabled];
            }
            else if (iter.subMenu != 0)
            {
                NSMenuItem* item = [m addItemWithTitle: text
                                                action: nil
                                         keyEquivalent: @""];

                [item setTag: iter.itemId];
                [item setEnabled: iter.isEnabled];
                
                NSMenu* sub = createMenu (*iter.subMenu, iter.itemName, id, topLevelIndex);
                [m setSubmenu: sub forItem: item];
                [sub release];
            }
            else
            {
                NSMenuItem* item = [m addItemWithTitle: text
                                                action: @selector (menuItemInvoked:)
                                         keyEquivalent: @""];

                [item setTag: iter.itemId];
                [item setEnabled: iter.isEnabled];
                [item setState: iter.isTicked ? NSOnState : NSOffState];
                [item setTarget: (id) callback];

                NSMutableArray* info = [NSMutableArray arrayWithObject: [NSNumber numberWithUnsignedLongLong: (pointer_sized_int) (void*) iter.commandManager]];
                [info addObject: [NSNumber numberWithInt: topLevelIndex]];
                [item setRepresentedObject: info];

                if (iter.commandManager != 0)
                {
                    const Array <KeyPress> keyPresses (iter.commandManager->getKeyMappings()
                                                       ->getKeyPressesAssignedToCommand (iter.itemId));
                    
                    if (keyPresses.size() > 0)
                    {
                        const KeyPress& kp = keyPresses.getReference(0);

                        juce_wchar key = kp.getTextCharacter();

                        if (kp.getKeyCode() == KeyPress::backspaceKey)
                            key = NSBackspaceCharacter;
                        else if (kp.getKeyCode() == KeyPress::deleteKey)
                            key = NSDeleteCharacter;
                        else if (key == 0)
                            key = (juce_wchar) kp.getKeyCode();

                        unsigned int mods = 0;
                        if (kp.getModifiers().isShiftDown())
                            mods |= NSShiftKeyMask;
                        if (kp.getModifiers().isCtrlDown())
                            mods |= NSControlKeyMask;
                        if (kp.getModifiers().isAltDown())
                            mods |= NSAlternateKeyMask;
                        if (kp.getModifiers().isCommandDown())
                            mods |= NSCommandKeyMask;

                        [item setKeyEquivalent: juceStringToNS (String::charToString (key))];
                        [item setKeyEquivalentModifierMask: mods];
                    }
                }
            }
        }
        
        [m update];
        return m;
    }
};

JuceMainMenuHandler* JuceMainMenuHandler::instance = 0;

END_JUCE_NAMESPACE
@implementation JuceMenuCallback

- (JuceMenuCallback*) initWithOwner: (JuceMainMenuHandler*) owner_
{
    [super init];
    owner = owner_;
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

- (void) menuItemInvoked: (id) menu
{
    NSMenuItem* item = (NSMenuItem*) menu;

    if ([[item representedObject] isKindOfClass: [NSArray class]])
    {
        NSArray* info = (NSArray*) [item representedObject];

        owner->invoke ([item tag],
                       (ApplicationCommandManager*) (pointer_sized_int) 
                            [((NSNumber*) [info objectAtIndex: 0]) unsignedLongLongValue],
                       (int) [((NSNumber*) [info objectAtIndex: 1]) intValue]);
    }
}

@end

BEGIN_JUCE_NAMESPACE

void MenuBarModel::setMacMainMenu (MenuBarModel* newMenuBarModel) throw()
{
    if (getMacMainMenu() != newMenuBarModel)
    {
        if (newMenuBarModel == 0)
        {
            delete JuceMainMenuHandler::instance;
            jassert (JuceMainMenuHandler::instance == 0); // should be zeroed in the destructor
        }
        else
        {
            if (JuceMainMenuHandler::instance == 0)
                JuceMainMenuHandler::instance = new JuceMainMenuHandler();
            
            JuceMainMenuHandler::instance->setMenu (newMenuBarModel);
        }
    }
}

MenuBarModel* MenuBarModel::getMacMainMenu() throw()
{
    return JuceMainMenuHandler::instance != 0 
            ? JuceMainMenuHandler::instance->currentModel : 0;
}


//==============================================================================
static NSMenu* createStandardAppMenu (const String& appName)
{
	NSMenu* menu = [[NSMenu alloc] initWithTitle: @"Apple"];
	NSMenuItem* item;

    // xxx should allow the 'about' and 'preferences' items to be turned on programatically...
/*    item = [menu addItemWithTitle: juceStringToNS ("About " + appName)]
						   action: @selector(orderFrontStandardAboutPanel:)  keyEquivalent: @""];
	[item setTarget: NSApp];
	[menu addItem: [NSMenuItem separatorItem]];
*/

/*    item = [menu addItemWithTitle: NSLocalizedString (@"Preferences...", nil)
						   action: nil  keyEquivalent: @","];
	[menu addItem: [NSMenuItem separatorItem]];
*/

    // Services...
	item = [menu addItemWithTitle: NSLocalizedString (@"Services", nil)
                           action: nil keyEquivalent: @""];
	NSMenu* servicesMenu = [[NSMenu alloc] initWithTitle: @"Services"];
	[menu setSubmenu: servicesMenu forItem: item];
	[NSApp setServicesMenu: servicesMenu];
	[servicesMenu release];
	[menu addItem: [NSMenuItem separatorItem]];

    // Hide + Show stuff...
	item = [menu addItemWithTitle: juceStringToNS ("Hide " + appName)
						   action: @selector (hide:)  keyEquivalent: @"h"];
	[item setTarget: NSApp];

	item = [menu addItemWithTitle: NSLocalizedString (@"Hide Others", nil)
						   action: @selector (hideOtherApplications:)  keyEquivalent: @"h"];
	[item setKeyEquivalentModifierMask: NSCommandKeyMask | NSAlternateKeyMask];
	[item setTarget: NSApp];

	item = [menu addItemWithTitle: NSLocalizedString (@"Show All", nil)
						   action: @selector (unhideAllApplications:)  keyEquivalent: @""];
	[item setTarget: NSApp];
	[menu addItem: [NSMenuItem separatorItem]];

    // Quit item....
	item = [menu addItemWithTitle: juceStringToNS ("Quit " + appName)
						   action: @selector (terminate:)  keyEquivalent: @"q"];
	[item setTarget: NSApp];

    return menu;
}

// Since our app has no NIB, this initialises a standard app menu...
void initialiseMainMenu()
{
    if (JUCEApplication::getInstance() != 0) // only needed in an app
    {
        const ScopedAutoReleasePool pool;

        NSMenu* mainMenu = [[NSMenu alloc] initWithTitle: @"MainMenu"];
        NSMenuItem* item = [mainMenu addItemWithTitle: @"Apple" action: nil keyEquivalent: @""];

        NSMenu* appMenu = createStandardAppMenu (JUCEApplication::getInstance()->getApplicationName());

        [NSApp performSelector: @selector (setAppleMenu:) withObject: appMenu];
        [mainMenu setSubmenu: appMenu forItem: item];

        [NSApp setMainMenu: mainMenu];
        [appMenu release];
        [mainMenu release];
    }
}


#endif
