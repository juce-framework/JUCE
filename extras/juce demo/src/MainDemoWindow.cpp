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

#include "jucedemo_headers.h"
#include "MainDemoWindow.h"


//==============================================================================
class ContentComp  : public Component,
                     public MenuBarModel,
                     public ApplicationCommandTarget
{
    //==============================================================================
    MainDemoWindow* mainWindow;

    OldSchoolLookAndFeel oldLookAndFeel;

    Component* currentDemo;
    int currentDemoId;

    TooltipWindow tooltipWindow; // to add tooltips to an application, you
                                 // just need to create one of these and leave it
                                 // there to do its work..

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

        setDefaultLookAndFeel      = 0x200b,
        setOldSchoolLookAndFeel    = 0x200c,
        useNativeTitleBar          = 0x200d,
        useNativeMenus             = 0x200e,
        goToKioskMode              = 0x200f
    };

public:
    //==============================================================================
    ContentComp (MainDemoWindow* mainWindow_)
        : mainWindow (mainWindow_),
          currentDemo (0),
          currentDemoId (0)
    {
        invokeDirectly (showRendering, true);
    }

    ~ContentComp()
    {
        // (need to do this because the old school look-and-feel object is one of our members,
        // so will be deleted with us, and would leave a dangling pointer if it's selected)
        LookAndFeel::setDefaultLookAndFeel (0);

        deleteAllChildren();
    }

    //==============================================================================
    void resized()
    {
        if (currentDemo != 0)
            currentDemo->setBounds (0, 0, getWidth(), getHeight());
    }

    //==============================================================================
    void showDemo (Component* demoComp)
    {
        delete currentDemo;
        currentDemo = demoComp;

        addAndMakeVisible (currentDemo);
        resized();
    }

    //==============================================================================
    const StringArray getMenuBarNames()
    {
        const tchar* const names[] = { T("Demo"), T("Look-and-feel"), 0 };

        return StringArray ((const tchar**) names);
    }

    const PopupMenu getMenuForIndex (int menuIndex,
                                     const String& menuName)
    {
        ApplicationCommandManager* const commandManager = mainWindow->commandManager;

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

            StringArray renderingEngines (getPeer()->getAvailableRenderingEngines());
            if (renderingEngines.size() > 1)
            {
                menu.addSeparator();

                for (int i = 0; i < renderingEngines.size(); ++i)
                    menu.addItem (5001 + i, "Use " + renderingEngines[i], true,
                                  i == getPeer()->getCurrentRenderingEngine());
            }
        }

        return menu;
    }

    void menuItemSelected (int menuItemID,
                           int topLevelMenuIndex)
    {
        // most of our menu items are invoked automatically as commands, but we can handle the
        // other special cases here..

        if (menuItemID >= 5001 && menuItemID < 5010)
            getPeer()->setCurrentRenderingEngine (menuItemID - 5001);
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
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
    {
        const String generalCategory (T("General"));
        const String demosCategory (T("Demos"));

        switch (commandID)
        {
        case showRendering:
            result.setInfo (T("Graphics Rendering"), T("Shows the graphics demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showRendering);
            result.addDefaultKeypress (T('1'), ModifierKeys::commandModifier);
            break;

        case showFontsAndText:
            result.setInfo (T("Fonts and Text"), T("Shows the fonts & text demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showFontsAndText);
            result.addDefaultKeypress (T('2'), ModifierKeys::commandModifier);
            break;

        case showWidgets:
            result.setInfo (T("Widgets"), T("Shows the widgets demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showWidgets);
            result.addDefaultKeypress (T('3'), ModifierKeys::commandModifier);
            break;

        case showThreading:
            result.setInfo (T("Multithreading"), T("Shows the threading demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showThreading);
            result.addDefaultKeypress (T('4'), ModifierKeys::commandModifier);
            break;

        case showTreeView:
            result.setInfo (T("Treeviews"), T("Shows the treeviews demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showTreeView);
            result.addDefaultKeypress (T('5'), ModifierKeys::commandModifier);
            break;

        case showTable:
            result.setInfo (T("Table Components"), T("Shows the table component demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showTable);
            result.addDefaultKeypress (T('6'), ModifierKeys::commandModifier);
            break;

        case showAudio:
            result.setInfo (T("Audio"), T("Shows the audio demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showAudio);
            result.addDefaultKeypress (T('7'), ModifierKeys::commandModifier);
            break;

        case showDragAndDrop:
            result.setInfo (T("Drag-and-drop"), T("Shows the drag & drop demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showDragAndDrop);
            result.addDefaultKeypress (T('8'), ModifierKeys::commandModifier);
            break;

        case showOpenGL:
            result.setInfo (T("OpenGL"), T("Shows the OpenGL demo"), demosCategory, 0);
            result.addDefaultKeypress (T('9'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showOpenGL);
#if ! JUCE_OPENGL
            result.setActive (false);
#endif
            break;

        case showQuicktime:
            result.setInfo (T("Quicktime"), T("Shows the Quicktime demo"), demosCategory, 0);
            result.addDefaultKeypress (T('b'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showQuicktime);
#if ! (JUCE_QUICKTIME && ! JUCE_LINUX)
            result.setActive (false);
#endif
            break;

        case showCamera:
            result.setInfo (T("Camera Capture"), T("Shows the camera demo"), demosCategory, 0);
            result.addDefaultKeypress (T('c'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showCamera);
#if ! JUCE_USE_CAMERA
            result.setActive (false);
#endif
            break;

        case showWebBrowser:
            result.setInfo (T("Web Browser"), T("Shows the web browser demo"), demosCategory, 0);
            result.addDefaultKeypress (T('i'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showWebBrowser);
#if (! JUCE_WEB_BROWSER) || JUCE_LINUX
            result.setActive (false);
#endif
            break;

        case showCodeEditor:
            result.setInfo (T("Code Editor"), T("Shows the code editor demo"), demosCategory, 0);
            result.addDefaultKeypress (T('e'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showCodeEditor);
            break;

        case showInterprocessComms:
            result.setInfo (T("Interprocess Comms"), T("Shows the interprocess communications demo"), demosCategory, 0);
            result.addDefaultKeypress (T('0'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showInterprocessComms);
            break;

        case setDefaultLookAndFeel:
            result.setInfo (T("Use default look-and-feel"), String::empty, generalCategory, 0);
            result.setTicked ((typeid (LookAndFeel) == typeid (getLookAndFeel())) != 0);
            break;

        case setOldSchoolLookAndFeel:
            result.setInfo (T("Use the old, original juce look-and-feel"), String::empty, generalCategory, 0);
            result.setTicked ((typeid (OldSchoolLookAndFeel) == typeid (getLookAndFeel())) != 0);
            break;

        case useNativeTitleBar:
            result.setInfo (T("Use native window title bar"), String::empty, generalCategory, 0);
            result.setTicked (mainWindow->isUsingNativeTitleBar());
            break;

#if JUCE_MAC
        case useNativeMenus:
            result.setInfo (T("Use the native OSX menu bar"), String::empty, generalCategory, 0);
            result.setTicked (MenuBarModel::getMacMainMenu() != 0);
            break;
#endif

#if ! JUCE_LINUX
        case goToKioskMode:
            result.setInfo (T("Show full-screen kiosk mode"), String::empty, generalCategory, 0);
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
            showDemo (createWidgetsDemo (mainWindow->commandManager));
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
            showDemo (createOpenGLDemo());
            currentDemoId = showOpenGL;
#endif
            break;

        case showQuicktime:
#if JUCE_QUICKTIME && ! JUCE_LINUX
            showDemo (createQuickTimeDemo());
            currentDemoId = showQuicktime;
#endif
            break;

        case showCamera:
#if JUCE_USE_CAMERA
            showDemo (createCameraDemo());
            currentDemoId = showCamera;
#endif
            break;

        case showWebBrowser:
#if JUCE_WEB_BROWSER
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
            LookAndFeel::setDefaultLookAndFeel (0);
            break;

        case setOldSchoolLookAndFeel:
            LookAndFeel::setDefaultLookAndFeel (&oldLookAndFeel);
            break;

        case useNativeTitleBar:
            mainWindow->setUsingNativeTitleBar (! mainWindow->isUsingNativeTitleBar());
            break;

#if JUCE_MAC
        case useNativeMenus:
            if (MenuBarModel::getMacMainMenu() != 0)
            {
                MenuBarModel::setMacMainMenu (0);
                mainWindow->setMenuBar ((ContentComp*) mainWindow->getContentComponent());
            }
            else
            {
                MenuBarModel::setMacMainMenu ((ContentComp*) mainWindow->getContentComponent());
                mainWindow->setMenuBar (0);
            }

            break;
#endif

#if ! JUCE_LINUX
        case goToKioskMode:
            if (Desktop::getInstance().getKioskModeComponent() == 0)
            {
                Desktop::getInstance().setKioskModeComponent (getTopLevelComponent());
            }
            else
            {
                Desktop::getInstance().setKioskModeComponent (0);
            }

            break;
#endif

        default:
            return false;
        };

        return true;
    }

    juce_UseDebuggingNewOperator
};

//==============================================================================
#if JUCE_WIN32 || JUCE_LINUX

// Just add a simple icon to the Window system tray area..
class DemoTaskbarComponent  : public SystemTrayIconComponent
{
public:
    DemoTaskbarComponent()
    {
        // Create an icon which is just a square with a "j" in it..
        Image icon (Image::RGB, 32, 32, true);
        Graphics g (icon);
        g.fillAll (Colours::lightblue);
        g.setColour (Colours::black);
        g.setFont ((float) icon.getHeight(), Font::bold);
        g.drawText (T("j"), 0, 0, icon.getWidth(), icon.getHeight(), Justification::centred, false);

        setIconImage (icon);

        setIconTooltip (T("Juce Demo App!"));
    }

    ~DemoTaskbarComponent()
    {
    }

    void mouseDown (const MouseEvent& e)
    {
        PopupMenu m;
        m.addItem (1, T("Quit the Juce demo"));

        const int result = m.show();

        if (result == 1)
            JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

#endif

//==============================================================================
MainDemoWindow::MainDemoWindow()
    : DocumentWindow (T("JUCE Demo!"),
                      Colours::azure,
                      DocumentWindow::allButtons,
                      true)
{
    commandManager = new ApplicationCommandManager();

    setResizable (true, false); // resizability is a property of ResizableWindow
    setResizeLimits (400, 300, 8192, 8192);

    ContentComp* contentComp = new ContentComp (this);

    commandManager->registerAllCommandsForTarget (contentComp);
    commandManager->registerAllCommandsForTarget (JUCEApplication::getInstance());

    // this lets the command manager use keypresses that arrive in our window to send
    // out commands
    addKeyListener (commandManager->getKeyMappings());

    // sets the main content component for the window to be this tabbed
    // panel. This will be deleted when the window is deleted.
    setContentComponent (contentComp);

    // this tells the DocumentWindow to automatically create and manage a MenuBarComponent
    // which uses our contentComp as its MenuBarModel
    setMenuBar (contentComp);

    // tells our menu bar model that it should watch this command manager for
    // changes, and send change messages accordingly.
    contentComp->setApplicationCommandManagerToWatch (commandManager);

    setVisible (true);

#if JUCE_WIN32 || JUCE_LINUX
    taskbarIcon = new DemoTaskbarComponent();
#endif
}

MainDemoWindow::~MainDemoWindow()
{
#if JUCE_WIN32 || JUCE_LINUX
    deleteAndZero (taskbarIcon);
#endif

    // because we've set the content comp to be used as our menu bar model, we
    // have to switch this off before deleting the content comp..
    setMenuBar (0);

#if JUCE_MAC  // ..and also the main bar if we're using that on a Mac...
    MenuBarModel::setMacMainMenu (0);
#endif

    // setting our content component to 0 will delete the current one, and
    // that will in turn delete all its child components. You don't always
    // have to do this explicitly, because the base class's destructor will
    // also delete the content component, but in this case we need to
    // make sure our content comp has gone away before deleting our command
    // manager.
    setContentComponent (0, true);

    delete commandManager;
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
