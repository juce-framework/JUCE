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

#include "jucedemo_headers.h"
#include "MainDemoWindow.h"


//==============================================================================
class ContentComp  : public Component,
                     public MenuBarModel,
                     public ApplicationCommandTarget
{
public:
    //==============================================================================
    ContentComp (MainDemoWindow& mainWindow_)
        : mainWindow (mainWindow_),
          currentDemoId (0)
    {
        setOpaque (true);
        invokeDirectly (showRendering, true);
    }

    ~ContentComp()
    {
       #if JUCE_OPENGL
        openGLContext.detach();
       #endif
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
    }

    void resized()
    {
        if (currentDemo != nullptr)
            currentDemo->setBounds (getLocalBounds());
    }

    //==============================================================================
    void showDemo (Component* demoComp)
    {
        currentDemo = demoComp;
        addAndMakeVisible (currentDemo);
        resized();
    }

    //==============================================================================
    StringArray getMenuBarNames()
    {
        const char* const names[] = { "Demo", "Look-and-feel", nullptr };

        return StringArray (names);
    }

    PopupMenu getMenuForIndex (int menuIndex, const String& /*menuName*/)
    {
        ApplicationCommandManager* commandManager = &(mainWindow.commandManager);

        PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (commandManager, showRendering);
            menu.addCommandItem (commandManager, showFontsAndText);
            menu.addCommandItem (commandManager, showWidgets);
            menu.addCommandItem (commandManager, showThreading);
            menu.addCommandItem (commandManager, showTreeView);
            menu.addCommandItem (commandManager, showTable);
            menu.addCommandItem (commandManager, showAudio);
            menu.addCommandItem (commandManager, showDragAndDrop);
            menu.addCommandItem (commandManager, showOpenGL);
            menu.addCommandItem (commandManager, showQuicktime);
            menu.addCommandItem (commandManager, showDirectShow);
            menu.addCommandItem (commandManager, showInterprocessComms);
            menu.addCommandItem (commandManager, showCamera);
            menu.addCommandItem (commandManager, showWebBrowser);
            menu.addCommandItem (commandManager, showCodeEditor);

            menu.addSeparator();
            menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (commandManager, setDefaultLookAndFeel);
            menu.addCommandItem (commandManager, setOldSchoolLookAndFeel);
            menu.addSeparator();
            menu.addCommandItem (commandManager, useNativeTitleBar);

           #if JUCE_MAC
            menu.addCommandItem (commandManager, useNativeMenus);
           #endif

           #if ! JUCE_LINUX
            menu.addCommandItem (commandManager, goToKioskMode);
           #endif

            StringArray engines (getRenderingEngines());

            if (engines.size() > 1)
            {
                menu.addSeparator();

                int currentEngine = getPeer()->getCurrentRenderingEngine();

               #if JUCE_OPENGL
                if (openGLContext.isAttached())
                    currentEngine = engines.size() - 1;
               #endif

                for (int i = 0; i < engines.size(); ++i)
                    menu.addItem (5001 + i, "Use " + engines[i], true, i == currentEngine);
            }
        }

        return menu;
    }

    void menuItemSelected (int menuItemID, int /*topLevelMenuIndex*/)
    {
        // most of our menu items are invoked automatically as commands, but we can handle the
        // other special cases here..

        if (menuItemID >= 5001 && menuItemID < 5010)
        {
            const int engineIndex = menuItemID - 5001;

           #if JUCE_OPENGL
            if (engineIndex >= getPeer()->getAvailableRenderingEngines().size())
            {
                setUsingOpenGLRenderer (true);
                return;
            }
           #endif

            setUsingOpenGLRenderer (false);
            getPeer()->setCurrentRenderingEngine (engineIndex);
        }
    }

    void setUsingOpenGLRenderer (bool shouldUseOpenGL)
    {
       #if JUCE_OPENGL
        if (shouldUseOpenGL && currentDemoId != showOpenGL)
            openGLContext.attachTo (*getTopLevelComponent());
        else
            openGLContext.detach();
       #endif
    }

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    ApplicationCommandTarget* getNextCommandTarget()
    {
        // this will return the next parent component that is an ApplicationCommandTarget (in this
        // case, there probably isn't one, but it's best to use this method in your own apps).
        return findFirstTargetParentComponent();
    }

    void getAllCommands (Array <CommandID>& commands)
    {
        // this returns the set of all commands that this target can perform..
        const CommandID ids[] = { showRendering,
                                  showFontsAndText,
                                  showWidgets,
                                  showThreading,
                                  showTreeView,
                                  showTable,
                                  showAudio,
                                  showDragAndDrop,
                                  showOpenGL,
                                  showQuicktime,
                                  showDirectShow,
                                  showCamera,
                                  showWebBrowser,
                                  showCodeEditor,
                                  showInterprocessComms,
                                  setDefaultLookAndFeel,
                                  setOldSchoolLookAndFeel,
                                  useNativeTitleBar
                                 #if JUCE_MAC
                                  , useNativeMenus
                                 #endif

                                 #if ! JUCE_LINUX
                                  , goToKioskMode
                                 #endif
        };

        commands.addArray (ids, numElementsInArray (ids));
    }

    // This method is used when something needs to find out the details about one of the commands
    // that this object can perform..
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
    {
        const String generalCategory ("General");
        const String demosCategory ("Demos");

        switch (commandID)
        {
        case showRendering:
            result.setInfo ("Graphics Rendering", "Shows the graphics demo", demosCategory, 0);
            result.setTicked (currentDemoId == showRendering);
            result.addDefaultKeypress ('1', ModifierKeys::commandModifier);
            break;

        case showFontsAndText:
            result.setInfo ("Fonts and Text", "Shows the fonts & text demo", demosCategory, 0);
            result.setTicked (currentDemoId == showFontsAndText);
            result.addDefaultKeypress ('2', ModifierKeys::commandModifier);
            break;

        case showWidgets:
            result.setInfo ("Widgets", "Shows the widgets demo", demosCategory, 0);
            result.setTicked (currentDemoId == showWidgets);
            result.addDefaultKeypress ('3', ModifierKeys::commandModifier);
            break;

        case showThreading:
            result.setInfo ("Multithreading", "Shows the threading demo", demosCategory, 0);
            result.setTicked (currentDemoId == showThreading);
            result.addDefaultKeypress ('4', ModifierKeys::commandModifier);
            break;

        case showTreeView:
            result.setInfo ("Treeviews", "Shows the treeviews demo", demosCategory, 0);
            result.setTicked (currentDemoId == showTreeView);
            result.addDefaultKeypress ('5', ModifierKeys::commandModifier);
            break;

        case showTable:
            result.setInfo ("Table Components", "Shows the table component demo", demosCategory, 0);
            result.setTicked (currentDemoId == showTable);
            result.addDefaultKeypress ('6', ModifierKeys::commandModifier);
            break;

        case showAudio:
            result.setInfo ("Audio", "Shows the audio demo", demosCategory, 0);
            result.setTicked (currentDemoId == showAudio);
            result.addDefaultKeypress ('7', ModifierKeys::commandModifier);
            break;

        case showDragAndDrop:
            result.setInfo ("Drag-and-drop", "Shows the drag & drop demo", demosCategory, 0);
            result.setTicked (currentDemoId == showDragAndDrop);
            result.addDefaultKeypress ('8', ModifierKeys::commandModifier);
            break;

        case showOpenGL:
            result.setInfo ("OpenGL", "Shows the OpenGL demo", demosCategory, 0);
            result.addDefaultKeypress ('9', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showOpenGL);
           #if ! JUCE_OPENGL
            result.setActive (false);
           #endif
            break;

        case showQuicktime:
            result.setInfo ("Quicktime", "Shows the Quicktime demo", demosCategory, 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showQuicktime);
           #if ! (JUCE_QUICKTIME && ! JUCE_LINUX)
            result.setActive (false);
           #endif
            break;

        case showDirectShow:
            result.setInfo ("DirectShow", "Shows the DirectShow demo", demosCategory, 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showDirectShow);
           #if ! JUCE_DIRECTSHOW
            result.setActive (false);
           #endif
            break;

        case showCamera:
            result.setInfo ("Camera Capture", "Shows the camera demo", demosCategory, 0);
            result.addDefaultKeypress ('c', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showCamera);
           #if ! JUCE_USE_CAMERA
            result.setActive (false);
           #endif
            break;

        case showWebBrowser:
            result.setInfo ("Web Browser", "Shows the web browser demo", demosCategory, 0);
            result.addDefaultKeypress ('i', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showWebBrowser);
           #if (! JUCE_WEB_BROWSER) || JUCE_LINUX
            result.setActive (false);
           #endif
            break;

        case showCodeEditor:
            result.setInfo ("Code Editor", "Shows the code editor demo", demosCategory, 0);
            result.addDefaultKeypress ('e', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showCodeEditor);
            break;

        case showInterprocessComms:
            result.setInfo ("Interprocess Comms", "Shows the interprocess communications demo", demosCategory, 0);
            result.addDefaultKeypress ('0', ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showInterprocessComms);
            break;

        case setDefaultLookAndFeel:
            result.setInfo ("Use default look-and-feel", String::empty, generalCategory, 0);
            result.setTicked (dynamic_cast <OldSchoolLookAndFeel*> (&getLookAndFeel()) == 0);
            break;

        case setOldSchoolLookAndFeel:
            result.setInfo ("Use the old, original juce look-and-feel", String::empty, generalCategory, 0);
            result.setTicked (dynamic_cast <OldSchoolLookAndFeel*> (&getLookAndFeel()) != 0);
            break;

        case useNativeTitleBar:
            result.setInfo ("Use native window title bar", String::empty, generalCategory, 0);
            result.setTicked (mainWindow.isUsingNativeTitleBar());
            break;

       #if JUCE_MAC
        case useNativeMenus:
            result.setInfo ("Use the native OSX menu bar", String::empty, generalCategory, 0);
            result.setTicked (MenuBarModel::getMacMainMenu() != 0);
            break;
       #endif

       #if ! JUCE_LINUX
        case goToKioskMode:
            result.setInfo ("Show full-screen kiosk mode", String::empty, generalCategory, 0);
            result.setTicked (Desktop::getInstance().getKioskModeComponent() != 0);
            break;
       #endif

        default:
            break;
        };
    }

    // this is the ApplicationCommandTarget method that is used to actually perform one of our commands..
    bool perform (const InvocationInfo& info)
    {
        switch (info.commandID)
        {
        case showRendering:
            showDemo (createRenderingDemo());
            currentDemoId = showRendering;
            break;

        case showFontsAndText:
            showDemo (createFontsAndTextDemo());
            currentDemoId = showFontsAndText;
            break;

        case showWidgets:
            showDemo (createWidgetsDemo());
            currentDemoId = showWidgets;
            break;

        case showThreading:
            showDemo (createThreadingDemo());
            currentDemoId = showThreading;
            break;

        case showTreeView:
            showDemo (createTreeViewDemo());
            currentDemoId = showTreeView;
            break;

        case showTable:
            showDemo (createTableDemo());
            currentDemoId = showTable;
            break;

        case showAudio:
            showDemo (createAudioDemo());
            currentDemoId = showAudio;
            break;

        case showDragAndDrop:
            showDemo (createDragAndDropDemo());
            currentDemoId = showDragAndDrop;
            break;

        case showOpenGL:
           #if JUCE_OPENGL
            setUsingOpenGLRenderer (false);
            showDemo (createOpenGLDemo());
            currentDemoId = showOpenGL;
           #endif
            break;

        case showQuicktime:
           #if JUCE_QUICKTIME && ! JUCE_LINUX
            setUsingOpenGLRenderer (false);
            showDemo (createQuickTimeDemo());
            currentDemoId = showQuicktime;
           #endif
            break;

        case showDirectShow:
           #if JUCE_DIRECTSHOW
            setUsingOpenGLRenderer (false);
            showDemo (createDirectShowDemo());
            currentDemoId = showDirectShow;
           #endif
            break;

        case showCamera:
           #if JUCE_USE_CAMERA
            setUsingOpenGLRenderer (false);
            showDemo (createCameraDemo());
            currentDemoId = showCamera;
           #endif
            break;

        case showWebBrowser:
           #if JUCE_WEB_BROWSER
            setUsingOpenGLRenderer (false);
            showDemo (createWebBrowserDemo());
            currentDemoId = showWebBrowser;
           #endif
            break;

        case showCodeEditor:
            showDemo (createCodeEditorDemo());
            currentDemoId = showCodeEditor;
            break;

        case showInterprocessComms:
            showDemo (createInterprocessCommsDemo());
            currentDemoId = showInterprocessComms;
            break;

        case setDefaultLookAndFeel:
            LookAndFeel::setDefaultLookAndFeel (nullptr);
            break;

        case setOldSchoolLookAndFeel:
            LookAndFeel::setDefaultLookAndFeel (&oldLookAndFeel);
            break;

        case useNativeTitleBar:
            mainWindow.setUsingNativeTitleBar (! mainWindow.isUsingNativeTitleBar());
            break;

       #if JUCE_MAC
        case useNativeMenus:
            if (MenuBarModel::getMacMainMenu() != 0)
            {
                MenuBarModel::setMacMainMenu (0);
                mainWindow.setMenuBar ((ContentComp*) mainWindow.getContentComponent());
            }
            else
            {
                MenuBarModel::setMacMainMenu ((ContentComp*) mainWindow.getContentComponent());
                mainWindow.setMenuBar (0);
            }

            break;
       #endif

       #if ! JUCE_LINUX
        case goToKioskMode:
            {
                Desktop& desktop = Desktop::getInstance();

                if (desktop.getKioskModeComponent() == nullptr)
                    desktop.setKioskModeComponent (getTopLevelComponent());
                else
                    desktop.setKioskModeComponent (nullptr);

                break;
            }
       #endif

        default:
            return false;
        };

        return true;
    }

private:
    //==============================================================================
    MainDemoWindow& mainWindow;
    OldSchoolLookAndFeel oldLookAndFeel;
    ScopedPointer<Component> currentDemo;
    int currentDemoId;

   #if JUCE_OPENGL
    OpenGLContext openGLContext;
   #endif

    TooltipWindow tooltipWindow; // to add tooltips to an application, you
                                 // just need to create one of these and leave it
                                 // there to do its work..

    //==============================================================================
    StringArray getRenderingEngines()
    {
        StringArray renderingEngines (getPeer()->getAvailableRenderingEngines());

       #if JUCE_OPENGL
        renderingEngines.add ("Use OpenGL Renderer");
       #endif

        return renderingEngines;
    }

    //==============================================================================
    enum CommandIDs
    {
        showRendering              = 0x2000,
        showFontsAndText           = 0x2001,
        showWidgets                = 0x2002,
        showThreading              = 0x2003,
        showTreeView               = 0x2004,
        showAudio                  = 0x2005,
        showDragAndDrop            = 0x2006,
        showOpenGL                 = 0x2007,
        showQuicktime              = 0x2008,
        showInterprocessComms      = 0x2009,
        showTable                  = 0x2010,
        showCamera                 = 0x2011,
        showWebBrowser             = 0x2012,
        showCodeEditor             = 0x2013,
        showDirectShow             = 0x2014,

        setDefaultLookAndFeel      = 0x200b,
        setOldSchoolLookAndFeel    = 0x200c,
        useNativeTitleBar          = 0x200d,
        useNativeMenus             = 0x200e,
        goToKioskMode              = 0x200f
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentComp)
};

//==============================================================================
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC

// Just add a simple icon to the Window system tray area..
class DemoTaskbarComponent  : public SystemTrayIconComponent,
                              private Timer
{
public:
    DemoTaskbarComponent()
    {
        setIconImage (createImageForIcon());
        setIconTooltip ("Juce Demo App!");
    }

    Image createImageForIcon()
    {
        Image icon (Image::RGB, 32, 32, true);

        Graphics g (icon);

        // This draws an icon which is just a square with a "j" in it..
        g.fillAll (Colours::lightblue);
        g.setColour (Colours::black);
        g.setFont (Font ((float) icon.getHeight(), Font::bold));
        g.drawText ("j", 0, 0, icon.getWidth(), icon.getHeight(), Justification::centred, false);

        return icon;
    }

    void mouseDown (const MouseEvent&) override
    {
        // On OSX, there can be problems launching a menu when we're not the foreground
        // process, so just in case, we'll first make our process active, and then use a
        // timer to wait a moment before opening our menu, which gives the OS some time to
        // get its act together and bring our windows to the front.

        Process::makeForegroundProcess();
        startTimer (50);
    }

    void timerCallback() override
    {
        stopTimer();

        PopupMenu m;
        m.addItem (1, "Quit the Juce demo");

        // It's always better to open menus asynchronously when possible.
        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuInvocationCallback, this));
    }

    // This is invoked when the menu is clicked or dismissed
    static void menuInvocationCallback (int chosenItemID, DemoTaskbarComponent*)
    {
        if (chosenItemID == 1)
            JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

#endif

//==============================================================================
MainDemoWindow::MainDemoWindow()
    : DocumentWindow ("JUCE Demo!",
                      Colours::azure,
                      DocumentWindow::allButtons,
                      true)
{
    setResizable (true, false); // resizability is a property of ResizableWindow
    setResizeLimits (400, 300, 8192, 8192);

    ContentComp* contentComp = new ContentComp (*this);

    commandManager.registerAllCommandsForTarget (contentComp);
    commandManager.registerAllCommandsForTarget (JUCEApplication::getInstance());

    // this lets the command manager use keypresses that arrive in our window to send
    // out commands
    addKeyListener (commandManager.getKeyMappings());

    // sets the main content component for the window to be this tabbed
    // panel. This will be deleted when the window is deleted.
    setContentOwned (contentComp, false);

    // this tells the DocumentWindow to automatically create and manage a MenuBarComponent
    // which uses our contentComp as its MenuBarModel
    setMenuBar (contentComp);

    // tells our menu bar model that it should watch this command manager for
    // changes, and send change messages accordingly.
    contentComp->setApplicationCommandManagerToWatch (&commandManager);

    setVisible (true);

   #if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    taskbarIcon = new DemoTaskbarComponent();
   #endif
}

MainDemoWindow::~MainDemoWindow()
{
    // because we've set the content comp to be used as our menu bar model, we
    // have to switch this off before deleting the content comp..
    setMenuBar (nullptr);

   #if JUCE_MAC  // ..and also the main bar if we're using that on a Mac...
    MenuBarModel::setMacMainMenu (nullptr);
   #endif

    // clearing the content component will delete the current one, and
    // that will in turn delete all its child components. You don't always
    // have to do this explicitly, because the base class's destructor will
    // also delete the content component, but in this case we need to
    // make sure our content comp has gone away before deleting our command
    // manager.
    clearContentComponent();
}

void MainDemoWindow::closeButtonPressed()
{
    // The correct thing to do when you want the app to quit is to call the
    // JUCEApplication::systemRequestedQuit() method.

    // That means that requests to quit that come from your own UI, or from other
    // OS-specific sources (e.g. the dock menu on the mac) all get handled in the
    // same way.

    JUCEApplication::getInstance()->systemRequestedQuit();
}
