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

#include "jucedemo_headers.h"
#include "MainDemoWindow.h"


//==============================================================================
class SourceCodeWindow;
static SourceCodeWindow* sourceWindow = 0;


//==============================================================================
class SourceCodeWindow  : public DialogWindow
{
    TextEditor* textBox;

public:
    SourceCodeWindow()
        : DialogWindow (T("JUCE Demo Source Code!"),
                        Colours::floralwhite,
                        false)
    {
        setContentComponent (textBox = new TextEditor());

        textBox->setColour (TextEditor::backgroundColourId, Colours::white);
        textBox->setMultiLine (true, false);
        textBox->setReturnKeyStartsNewLine (true);

        setResizable (true, true); // we'll choose a corner-resizer component for this window,
                                   // as a contrast to the resizable border on the main window
    }

    ~SourceCodeWindow()
    {
        // the text editor gets deleted automatically because it's the
        // window's content component.

        sourceWindow = 0;
    }

    void closeButtonPressed()
    {
        delete this;
    }

    void updateSourceCode (const String& text)
    {
        Font font (14.0f);
        font.setTypefaceName (Font::getDefaultMonospacedFontName());
        textBox->setFont (font);

        textBox->setText (text);

        toFront (true);
    }
};

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
    const char* demoSourceCodeText;

    TooltipWindow tooltipWindow; // to add tooltips to an application, you
                                 // just need to create one of these and leave it
                                 // there to do its work..

    enum CommandIDs
    {
        showPathsAndTransforms     = 0x2000,
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

        showSourceCode             = 0x200a,

        setDefaultLookAndFeel      = 0x200b,
        setOldSchoolLookAndFeel    = 0x200c,
        useNativeTitleBar          = 0x200d,
        useNativeMenus             = 0x200e
    };

public:
    //==============================================================================
    ContentComp (MainDemoWindow* mainWindow_)
        : mainWindow (mainWindow_),
          currentDemo (0),
          currentDemoId (0),
          demoSourceCodeText (0)
    {
        invokeDirectly (showWidgets, true);
    }

    ~ContentComp()
    {
        // (need to do this because the old school look-and-feel object is one of our members,
        // so will be deleted with us, and would leave a dangling pointer if it's selected)
        LookAndFeel::setDefaultLookAndFeel (0);

        deleteAllChildren();

        deleteAndZero (sourceWindow);
    }

    //==============================================================================
    void resized()
    {
        if (currentDemo != 0)
            currentDemo->setBounds (0, 0, getWidth(), getHeight());
    }

    //==============================================================================
    void showDemo (Component* demoComp, const char* sourceCodeText)
    {
        delete currentDemo;
        currentDemo = demoComp;

        addAndMakeVisible (currentDemo);
        resized();

        demoSourceCodeText = sourceCodeText;
    }

    void showSource()
    {
        if (sourceWindow == 0)
        {
            sourceWindow = new SourceCodeWindow();
            sourceWindow->centreAroundComponent (this, 750, 600);
            sourceWindow->setVisible (true);
        }

        sourceWindow->updateSourceCode (demoSourceCodeText);
    }

    //==============================================================================
    const StringArray getMenuBarNames()
    {
        const tchar* const names[] = { T("Demo"), T("Source Code"), T("Look-and-feel"), 0 };

        return StringArray ((const tchar**) names);
    }

    const PopupMenu getMenuForIndex (int menuIndex,
                                     const String& menuName)
    {
        ApplicationCommandManager* const commandManager = mainWindow->commandManager;

        PopupMenu menu;

        if (menuIndex == 0)
        {
            menu.addCommandItem (commandManager, showPathsAndTransforms);
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

            menu.addSeparator();
            menu.addCommandItem (commandManager, StandardApplicationCommandIDs::quit);
        }
        else if (menuIndex == 1)
        {
            menu.addCommandItem (commandManager, showSourceCode);
        }
        else if (menuIndex == 2)
        {
            menu.addCommandItem (commandManager, setDefaultLookAndFeel);
            menu.addCommandItem (commandManager, setOldSchoolLookAndFeel);
            menu.addSeparator();
            menu.addCommandItem (commandManager, useNativeTitleBar);

#if JUCE_MAC
            menu.addCommandItem (commandManager, useNativeMenus);
#endif
        }

        return menu;
    }

    void menuItemSelected (int menuItemID,
                           int topLevelMenuIndex)
    {
        // all our menu items are invoked automatically as commands, so no need to do
        // anything in this callback
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
        const CommandID ids[] = { showPathsAndTransforms,
                                  showFontsAndText,
                                  showWidgets,
                                  showThreading,
                                  showTreeView,
                                  showTable,
                                  showAudio,
                                  showDragAndDrop,
                                  showOpenGL,
                                  showQuicktime,
                                  showInterprocessComms,
                                  showSourceCode,
                                  setDefaultLookAndFeel,
                                  setOldSchoolLookAndFeel,
                                  useNativeTitleBar
#if JUCE_MAC
                                , useNativeMenus
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
        case showPathsAndTransforms:
            result.setInfo (T("Paths and Transforms"), T("Shows the paths & transforms demo"), demosCategory, 0);
            result.setTicked (currentDemoId == showPathsAndTransforms);
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
#ifndef JUCE_OPENGL
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

        case showInterprocessComms:
            result.setInfo (T("Interprocess Comms"), T("Shows the interprocess communications demo"), demosCategory, 0);
            result.addDefaultKeypress (T('0'), ModifierKeys::commandModifier);
            result.setTicked (currentDemoId == showInterprocessComms);
            break;

        case showSourceCode:
            result.setInfo (T("Show the source code for this demo"), T("Opens a window containing this demo's source code"), generalCategory, 0);
            result.addDefaultKeypress (T('s'), ModifierKeys::commandModifier);
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

        default:
            break;
        };
    }

    // this is the ApplicationCommandTarget method that is used to actually perform one of our commands..
    bool perform (const InvocationInfo& info)
    {
        switch (info.commandID)
        {
        case showPathsAndTransforms:
            showDemo (createPathsAndTransformsDemo(), BinaryData::pathsandtransformsdemo_cpp);
            currentDemoId = showPathsAndTransforms;
            break;

        case showFontsAndText:
            showDemo (createFontsAndTextDemo(), BinaryData::fontsandtextdemo_cpp);
            currentDemoId = showFontsAndText;
            break;

        case showWidgets:
            showDemo (createWidgetsDemo (mainWindow->commandManager), BinaryData::widgetsdemo_cpp);
            currentDemoId = showWidgets;
            break;

        case showThreading:
            showDemo (createThreadingDemo(), BinaryData::threadingdemo_cpp);
            currentDemoId = showThreading;
            break;

        case showTreeView:
            showDemo (createTreeViewDemo(), BinaryData::treeviewdemo_cpp);
            currentDemoId = showTreeView;
            break;

        case showTable:
            showDemo (createTableDemo(), BinaryData::tabledemo_cpp);
            currentDemoId = showTable;
            break;

        case showAudio:
            showDemo (createAudioDemo(), BinaryData::audiodemo_cpp);
            currentDemoId = showAudio;
            break;

        case showDragAndDrop:
            showDemo (createDragAndDropDemo(), BinaryData::draganddropdemo_cpp);
            currentDemoId = showDragAndDrop;
            break;

        case showOpenGL:
#if JUCE_OPENGL
            showDemo (createOpenGLDemo(), BinaryData::opengldemo_cpp);
            currentDemoId = showOpenGL;
#endif
            break;

        case showQuicktime:
#if JUCE_QUICKTIME && ! JUCE_LINUX
            showDemo (createQuickTimeDemo(), BinaryData::quicktimedemo_cpp);
            currentDemoId = showQuicktime;
#endif
            break;

        case showInterprocessComms:
            showDemo (createInterprocessCommsDemo(), BinaryData::interprocesscommsdemo_cpp);
            currentDemoId = showInterprocessComms;
            break;

        case showSourceCode:
            showSource();
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
