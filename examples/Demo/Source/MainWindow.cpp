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

#include <typeinfo>
#include "JuceDemoHeader.h"

//==============================================================================
struct AlphabeticDemoSorter
{
    static int compareElements (const JuceDemoTypeBase* first, const JuceDemoTypeBase* second)
    {
        return first->name.compare (second->name);
    }
};

JuceDemoTypeBase::JuceDemoTypeBase (const String& demoName)  : name (demoName)
{
    AlphabeticDemoSorter sorter;
    getDemoTypeList().addSorted (sorter, this);
}

JuceDemoTypeBase::~JuceDemoTypeBase()
{
    getDemoTypeList().removeFirstMatchingValue (this);
}

Array<JuceDemoTypeBase*>& JuceDemoTypeBase::getDemoTypeList()
{
    static Array<JuceDemoTypeBase*> list;
    return list;
}

//==============================================================================
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC

// Just add a simple icon to the Window system tray area or Mac menu bar..
class DemoTaskbarComponent  : public SystemTrayIconComponent,
                              private Timer
{
public:
    DemoTaskbarComponent()
    {
        setIconImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        setIconTooltip ("Juce Demo App!");
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

    // This is invoked when the menu is clicked or dismissed
    static void menuInvocationCallback (int chosenItemID, DemoTaskbarComponent*)
    {
        if (chosenItemID == 1)
            JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    void timerCallback() override
    {
        stopTimer();

        PopupMenu m;
        m.addItem (1, "Quit the Juce demo");

        // It's always better to open menus asynchronously when possible.
        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (menuInvocationCallback, this));
    }
};

#endif

bool juceDemoRepaintDebuggingActive = false;

//==============================================================================
class ContentComponent   : public Component,
                           public ListBoxModel,
                           public ApplicationCommandTarget
{
public:
    ContentComponent()
    {
        // set lookAndFeel colour properties
        lookAndFeelV3.setColour (Label::textColourId, Colours::white);
        lookAndFeelV3.setColour (Label::textColourId, Colours::white);
        lookAndFeelV3.setColour (ToggleButton::textColourId, Colours::white);
        LookAndFeel::setDefaultLookAndFeel (&lookAndFeelV3);

        demoList.setModel (this);
        demoList.setColour (ListBox::backgroundColourId, Colour::greyLevel (0.2f));
        demoList.selectRow (0);
        addAndMakeVisible (demoList);

    }

    void clearCurrentDemo()
    {
        currentDemo = nullptr;
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds());

        if (r.getWidth() > 600)
        {
            demoList.setBounds (r.removeFromLeft (210));
            demoList.setRowHeight (20);
        }
        else
        {
            demoList.setBounds (r.removeFromLeft (130));
            demoList.setRowHeight (30);
        }

        if (currentDemo != nullptr)
            currentDemo->setBounds (r);
    }

    int getNumRows() override
    {
        return JuceDemoTypeBase::getDemoTypeList().size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::deepskyblue);

        if (JuceDemoTypeBase* type = JuceDemoTypeBase::getDemoTypeList() [rowNumber])
        {
            String name (type->name.trimCharactersAtStart ("0123456789").trimStart());

            AttributedString a;
            a.setJustification (Justification::centredLeft);

            String category;

            if (name.containsChar (':'))
            {
                category = name.upToFirstOccurrenceOf (":", true, false);
                name = name.fromFirstOccurrenceOf (":", false, false).trim();

                if (height > 20)
                    category << "\n";
                else
                    category << " ";
            }

            if (category.isNotEmpty())
                a.append (category, Font (10.0f), Colour::greyLevel (0.5f));

            a.append (name, Font (13.0f), Colours::white.withAlpha (0.9f));

            a.draw (g, Rectangle<int> (width + 10, height).reduced (6, 0).toFloat());
        }
    }

    void selectedRowsChanged (int lastRowSelected) override
    {
        if (JuceDemoTypeBase* selectedDemoType = JuceDemoTypeBase::getDemoTypeList() [lastRowSelected])
        {
            currentDemo = nullptr;
            addAndMakeVisible (currentDemo = selectedDemoType->createComponent());
            currentDemo->setName (selectedDemoType->name);
            resized();
        }
    }

    MouseCursor getMouseCursorForRow (int /*row*/) override
    {
        return MouseCursor::PointingHandCursor;
    }

    int getCurrentPageIndex() const noexcept
    {
        if (currentDemo == nullptr)
            return -1;

        Array<JuceDemoTypeBase*>& demos (JuceDemoTypeBase::getDemoTypeList());

        for (int i = demos.size(); --i >= 0;)
            if (demos.getUnchecked (i)->name == currentDemo->getName())
                return i;

        return -1;
    }

    void moveDemoPages (int numPagesToMove)
    {
        const int newIndex = negativeAwareModulo (getCurrentPageIndex() + numPagesToMove,
                                                  JuceDemoTypeBase::getDemoTypeList().size());
        demoList.selectRow (newIndex);
        // we have to go through our demo list here or it won't be updated to reflect the current demo
    }

    bool isShowingOpenGLDemo() const
    {
        return currentDemo != nullptr
                && currentDemo->getName().contains ("OpenGL")
                && ! isShowingOpenGL2DDemo();
    }

    bool isShowingOpenGL2DDemo() const
    {
        return currentDemo != nullptr && currentDemo->getName().contains ("OpenGL 2D");
    }

private:
    ListBox demoList;
    ScopedPointer<Component> currentDemo;

    LookAndFeel_V1 lookAndFeelV1;
    LookAndFeel_V2 lookAndFeelV2;
    LookAndFeel_V3 lookAndFeelV3;

    //==============================================================================
    // The following methods implement the ApplicationCommandTarget interface, allowing
    // this window to publish a set of actions it can perform, and which can be mapped
    // onto menus, keypresses, etc.

    ApplicationCommandTarget* getNextCommandTarget() override
    {
        // this will return the next parent component that is an ApplicationCommandTarget (in this
        // case, there probably isn't one, but it's best to use this method in your own apps).
        return findFirstTargetParentComponent();
    }

    void getAllCommands (Array<CommandID>& commands) override
    {
        // this returns the set of all commands that this target can perform..
        const CommandID ids[] = { MainAppWindow::showPreviousDemo,
                                  MainAppWindow::showNextDemo,
                                  MainAppWindow::welcome,
                                  MainAppWindow::componentsAnimation,
                                  MainAppWindow::componentsDialogBoxes,
                                  MainAppWindow::componentsKeyMappings,
                                  MainAppWindow::componentsMDI,
                                  MainAppWindow::componentsPropertyEditors,
                                  MainAppWindow::componentsTransforms,
                                  MainAppWindow::componentsWebBrowsers,
                                  MainAppWindow::componentsWidgets,
                                  MainAppWindow::useLookAndFeelV1,
                                  MainAppWindow::useLookAndFeelV2,
                                  MainAppWindow::useLookAndFeelV3,
                                  MainAppWindow::toggleRepaintDebugging,
                                 #if ! JUCE_LINUX
                                  MainAppWindow::goToKioskMode,
                                 #endif
                                  MainAppWindow::useNativeTitleBar
                                };

        commands.addArray (ids, numElementsInArray (ids));

        const CommandID engineIDs[] = { MainAppWindow::renderingEngineOne,
                                        MainAppWindow::renderingEngineTwo,
                                        MainAppWindow::renderingEngineThree };

        StringArray renderingEngines (MainAppWindow::getMainAppWindow()->getRenderingEngines());
        commands.addArray (engineIDs, renderingEngines.size());
    }

    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override
    {
        const String generalCategory ("General");
        const String demosCategory ("Demos");

        switch (commandID)
        {
            case MainAppWindow::showPreviousDemo:
                result.setInfo ("Previous Demo", "Shows the previous demo in the list", demosCategory, 0);
                result.addDefaultKeypress ('-', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::showNextDemo:
                result.setInfo ("Next Demo", "Shows the next demo in the list", demosCategory, 0);
                result.addDefaultKeypress ('=', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::welcome:
                result.setInfo ("Welcome Demo", "Shows the 'Welcome' demo", demosCategory, 0);
                result.addDefaultKeypress ('1', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsAnimation:
                result.setInfo ("Animation Demo", "Shows the 'Animation' demo", demosCategory, 0);
                result.addDefaultKeypress ('2', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsDialogBoxes:
                result.setInfo ("Dialog Boxes Demo", "Shows the 'Dialog Boxes' demo", demosCategory, 0);
                result.addDefaultKeypress ('3', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsKeyMappings:
                result.setInfo ("Key Mappings Demo", "Shows the 'Key Mappings' demo", demosCategory, 0);
                result.addDefaultKeypress ('4', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsMDI:
                result.setInfo ("Multi-Document Demo", "Shows the 'Multi-Document' demo", demosCategory, 0);
                result.addDefaultKeypress ('5', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsPropertyEditors:
                result.setInfo ("Property Editor Demo", "Shows the 'Property Editor' demo", demosCategory, 0);
                result.addDefaultKeypress ('6', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsTransforms:
                result.setInfo ("Component Transforms Demo", "Shows the 'Transforms' demo", demosCategory, 0);
                result.addDefaultKeypress ('7', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsWebBrowsers:
                result.setInfo ("Web Browser Demo", "Shows the 'Web Browser' demo", demosCategory, 0);
                result.addDefaultKeypress ('8', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::componentsWidgets:
                result.setInfo ("Widgets Demo", "Shows the 'Widgets' demo", demosCategory, 0);
                result.addDefaultKeypress ('9', ModifierKeys::commandModifier);
                break;

            case MainAppWindow::renderingEngineOne:
            case MainAppWindow::renderingEngineTwo:
            case MainAppWindow::renderingEngineThree:
            {
                MainAppWindow& mainWindow = *MainAppWindow::getMainAppWindow();
                const StringArray engines (mainWindow.getRenderingEngines());
                const int index = commandID - MainAppWindow::renderingEngineOne;

                result.setInfo ("Use " + engines[index], "Uses the " + engines[index] + " engine to render the UI", generalCategory, 0);
                result.setTicked (mainWindow.getActiveRenderingEngine() == index);

                result.addDefaultKeypress ('1' + index, ModifierKeys::noModifiers);
                break;
            }

            case MainAppWindow::useLookAndFeelV1:
                result.setInfo ("Use LookAndFeel_V1", String(), generalCategory, 0);
                result.addDefaultKeypress ('i', ModifierKeys::commandModifier);
                result.setTicked (isLookAndFeelSelected<LookAndFeel_V1>());
                break;

            case MainAppWindow::useLookAndFeelV2:
                result.setInfo ("Use LookAndFeel_V2", String(), generalCategory, 0);
                result.addDefaultKeypress ('o', ModifierKeys::commandModifier);
                result.setTicked (isLookAndFeelSelected<LookAndFeel_V2>());
                break;

            case MainAppWindow::useLookAndFeelV3:
                result.setInfo ("Use LookAndFeel_V3", String(), generalCategory, 0);
                result.addDefaultKeypress ('p', ModifierKeys::commandModifier);
                result.setTicked (isLookAndFeelSelected<LookAndFeel_V3>());
                break;

            case MainAppWindow::toggleRepaintDebugging:
                result.setInfo ("Toggle repaint display", String(), generalCategory, 0);
                result.addDefaultKeypress ('r', ModifierKeys());
                result.setTicked (juceDemoRepaintDebuggingActive);
                break;

            case MainAppWindow::useNativeTitleBar:
            {
                result.setInfo ("Use native window title bar", String(), generalCategory, 0);
                result.addDefaultKeypress ('n', ModifierKeys::commandModifier);
                bool nativeTitlebar = false;

                if (MainAppWindow* map = MainAppWindow::getMainAppWindow())
                    nativeTitlebar = map->isUsingNativeTitleBar();

                result.setTicked (nativeTitlebar);
                break;
            }

           #if ! JUCE_LINUX
            case MainAppWindow::goToKioskMode:
                result.setInfo ("Show full-screen kiosk mode", String(), generalCategory, 0);
                result.addDefaultKeypress ('f', ModifierKeys::commandModifier);
                result.setTicked (Desktop::getInstance().getKioskModeComponent() != 0);
                break;
           #endif

            default:
                break;
        }
    }

    bool perform (const InvocationInfo& info) override
    {
        MainAppWindow* mainWindow = MainAppWindow::getMainAppWindow();

        if (mainWindow == nullptr)
            return true;

        switch (info.commandID)
        {
            case MainAppWindow::showPreviousDemo:   moveDemoPages (-1); break;
            case MainAppWindow::showNextDemo:       moveDemoPages ( 1); break;

            case MainAppWindow::welcome:
            case MainAppWindow::componentsAnimation:
            case MainAppWindow::componentsDialogBoxes:
            case MainAppWindow::componentsKeyMappings:
            case MainAppWindow::componentsMDI:
            case MainAppWindow::componentsPropertyEditors:
            case MainAppWindow::componentsTransforms:
            case MainAppWindow::componentsWebBrowsers:
            case MainAppWindow::componentsWidgets:
                demoList.selectRow (info.commandID - MainAppWindow::welcome);
                break;

            case MainAppWindow::renderingEngineOne:
            case MainAppWindow::renderingEngineTwo:
            case MainAppWindow::renderingEngineThree:
                mainWindow->setRenderingEngine (info.commandID - MainAppWindow::renderingEngineOne);
                break;

            case MainAppWindow::useLookAndFeelV1:  LookAndFeel::setDefaultLookAndFeel (&lookAndFeelV1); break;
            case MainAppWindow::useLookAndFeelV2:  LookAndFeel::setDefaultLookAndFeel (&lookAndFeelV2); break;
            case MainAppWindow::useLookAndFeelV3:  LookAndFeel::setDefaultLookAndFeel (&lookAndFeelV3); break;

            case MainAppWindow::toggleRepaintDebugging:
                juceDemoRepaintDebuggingActive = ! juceDemoRepaintDebuggingActive;
                mainWindow->repaint();
                break;

            case MainAppWindow::useNativeTitleBar:
                mainWindow->setUsingNativeTitleBar (! mainWindow->isUsingNativeTitleBar());
                break;

           #if ! JUCE_LINUX
            case MainAppWindow::goToKioskMode:
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
        }

        return true;
    }

    template <typename LookAndFeelType>
    bool isLookAndFeelSelected()
    {
        LookAndFeel& lf = getLookAndFeel();
        return typeid (LookAndFeelType) == typeid (lf);
    }
};

//==============================================================================
static ScopedPointer<ApplicationCommandManager> applicationCommandManager;
static ScopedPointer<AudioDeviceManager> sharedAudioDeviceManager;

MainAppWindow::MainAppWindow()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colours::lightgrey,
                      DocumentWindow::allButtons)
{
    setUsingNativeTitleBar (true);
    setResizable (true, false);
    setResizeLimits (400, 400, 10000, 10000);

   #if JUCE_IOS || JUCE_ANDROID
    setFullScreen (true);
   #else
    setBounds ((int) (0.1f * getParentWidth()),
               (int) (0.1f * getParentHeight()),
               jmax (850, (int) (0.5f * getParentWidth())),
               jmax (600, (int) (0.7f * getParentHeight())));
   #endif

    contentComponent = new ContentComponent();
    setContentNonOwned (contentComponent, false);
    setVisible (true);

    // this lets the command manager use keypresses that arrive in our window to send out commands
    addKeyListener (getApplicationCommandManager().getKeyMappings());

   #if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
    taskbarIcon = new DemoTaskbarComponent();
   #endif

   #if JUCE_ANDROID
    setOpenGLRenderingEngine();
   #endif

    triggerAsyncUpdate();
}

MainAppWindow::~MainAppWindow()
{
    contentComponent->clearCurrentDemo();
    clearContentComponent();
    contentComponent = nullptr;
    applicationCommandManager = nullptr;
    sharedAudioDeviceManager = nullptr;

   #if JUCE_OPENGL
    openGLContext.detach();
   #endif
}

void MainAppWindow::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}

ApplicationCommandManager& MainAppWindow::getApplicationCommandManager()
{
    if (applicationCommandManager == nullptr)
        applicationCommandManager = new ApplicationCommandManager();

    return *applicationCommandManager;
}

AudioDeviceManager& MainAppWindow::getSharedAudioDeviceManager()
{
    if (sharedAudioDeviceManager == nullptr)
    {
        sharedAudioDeviceManager = new AudioDeviceManager();
        RuntimePermissions::request (RuntimePermissions::recordAudio, runtimPermissionsCallback);
    }

    return *sharedAudioDeviceManager;
}

void MainAppWindow::runtimPermissionsCallback (bool wasGranted)
{
    int numInputChannels = wasGranted ? 2 : 0;
    sharedAudioDeviceManager->initialise (numInputChannels, 2, nullptr, true, String(), nullptr);
}

MainAppWindow* MainAppWindow::getMainAppWindow()
{
    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
        if (MainAppWindow* maw = dynamic_cast<MainAppWindow*> (TopLevelWindow::getTopLevelWindow (i)))
            return maw;

    return nullptr;
}

void MainAppWindow::handleAsyncUpdate()
{
    // This registers all of our commands with the command manager but has to be done after the window has
    // been created so we can find the number of rendering engines available
    ApplicationCommandManager& commandManager = MainAppWindow::getApplicationCommandManager();
    commandManager.registerAllCommandsForTarget (contentComponent);
    commandManager.registerAllCommandsForTarget (JUCEApplication::getInstance());
}

void MainAppWindow::showMessageBubble (const String& text)
{
    currentBubbleMessage = new BubbleMessageComponent (500);
    getContentComponent()->addChildComponent (currentBubbleMessage);

    AttributedString attString;
    attString.append (text, Font (15.0f));

    currentBubbleMessage->showAt (Rectangle<int> (getLocalBounds().getCentreX(), 10, 1, 1),
                                  attString,
                                  500,  // numMillisecondsBeforeRemoving
                                  true,  // removeWhenMouseClicked
                                  false); // deleteSelfAfterUse
}

static const char* openGLRendererName = "OpenGL Renderer";

StringArray MainAppWindow::getRenderingEngines() const
{
    StringArray renderingEngines;

    if (ComponentPeer* peer = getPeer())
        renderingEngines = peer->getAvailableRenderingEngines();

   #if JUCE_OPENGL
    renderingEngines.add (openGLRendererName);
   #endif

    return renderingEngines;
}

void MainAppWindow::setRenderingEngine (int index)
{
    showMessageBubble (getRenderingEngines()[index]);

   #if JUCE_OPENGL
    if (getRenderingEngines()[index] == openGLRendererName
          && contentComponent != nullptr
          && ! contentComponent->isShowingOpenGLDemo())
    {
        openGLContext.attachTo (*getTopLevelComponent());
        return;
    }

    openGLContext.detach();
   #endif

    if (ComponentPeer* peer = getPeer())
        peer->setCurrentRenderingEngine (index);
}

void MainAppWindow::setOpenGLRenderingEngine()
{
    setRenderingEngine (getRenderingEngines().indexOf (openGLRendererName));
}

int MainAppWindow::getActiveRenderingEngine() const
{
   #if JUCE_OPENGL
    if (openGLContext.isAttached())
        return getRenderingEngines().indexOf (openGLRendererName);
   #endif

    if (ComponentPeer* peer = getPeer())
        return peer->getCurrentRenderingEngine();

    return 0;
}

Path MainAppWindow::getJUCELogoPath()
{
    return Drawable::parseSVGPath (
        "M250,301.3c-37.2,0-67.5-30.3-67.5-67.5s30.3-67.5,67.5-67.5s67.5,30.3,67.5,67.5S287.2,301.3,250,301.3zM250,170.8c-34.7,0-63,28.3-63,63s28.3,63,63,63s63-28.3,63-63S284.7,170.8,250,170.8z"
        "M247.8,180.4c0-2.3-1.8-4.1-4.1-4.1c-0.2,0-0.3,0-0.5,0c-10.6,1.2-20.6,5.4-29,12c-1,0.8-1.5,1.8-1.6,2.9c-0.1,1.2,0.4,2.3,1.3,3.2l32.5,32.5c0.5,0.5,1.4,0.1,1.4-0.6V180.4z"
        "M303.2,231.6c1.2,0,2.3-0.4,3.1-1.2c0.9-0.9,1.3-2.1,1.1-3.3c-1.2-10.6-5.4-20.6-12-29c-0.8-1-1.9-1.6-3.2-1.6c-1.1,0-2.1,0.5-3,1.3l-32.5,32.5c-0.5,0.5-0.1,1.4,0.6,1.4L303.2,231.6z"
        "M287.4,191.3c-0.1-1.1-0.6-2.2-1.6-2.9c-8.4-6.6-18.4-10.8-29-12c-0.2,0-0.3,0-0.5,0c-2.3,0-4.1,1.9-4.1,4.1v46c0,0.7,0.9,1.1,1.4,0.6l32.5-32.5C287,193.6,287.5,192.5,287.4,191.3z"
        "M252.2,287.2c0,2.3,1.8,4.1,4.1,4.1c0.2,0,0.3,0,0.5,0c10.6-1.2,20.6-5.4,29-12c1-0.8,1.5-1.8,1.6-2.9c0.1-1.2-0.4-2.3-1.3-3.2l-32.5-32.5c-0.5-0.5-1.4-0.1-1.4,0.6V287.2z"
        "M292.3,271.2L292.3,271.2c1.2,0,2.4-0.6,3.2-1.6c6.6-8.4,10.8-18.4,12-29c0.1-1.2-0.3-2.4-1.1-3.3c-0.8-0.8-1.9-1.2-3.1-1.2l-45.9,0c-0.7,0-1.1,0.9-0.6,1.4l32.5,32.5C290.2,270.8,291.2,271.2,292.3,271.2z"
        "M207.7,196.4c-1.2,0-2.4,0.6-3.2,1.6c-6.6,8.4-10.8,18.4-12,29c-0.1,1.2,0.3,2.4,1.1,3.3c0.8,0.8,1.9,1.2,3.1,1.2l45.9,0c0.7,0,1.1-0.9,0.6-1.4l-32.5-32.5C209.8,196.8,208.8,196.4,207.7,196.4z"
        "M242.6,236.1l-45.9,0c-1.2,0-2.3,0.4-3.1,1.2c-0.9,0.9-1.3,2.1-1.1,3.3c1.2,10.6,5.4,20.6,12,29c0.8,1,1.9,1.6,3.2,1.6c1.1,0,2.1-0.5,3-1.3c0,0,0,0,0,0l32.5-32.5C243.7,236.9,243.4,236.1,242.6,236.1z"
        "M213.8,273.1L213.8,273.1c-0.9,0.9-1.3,2-1.3,3.2c0.1,1.1,0.6,2.2,1.6,2.9c8.4,6.6,18.4,10.8,29,12c0.2,0,0.3,0,0.5,0h0c1.2,0,2.3-0.5,3.1-1.4c0.7-0.8,1-1.8,1-2.9v-45.9c0-0.7-0.9-1.1-1.4-0.6l-13.9,13.9L213.8,273.1z"
        "M197.2,353c-4.1,0-7.4-1.5-10.4-5.4l4-3.5c2,2.6,3.9,3.6,6.4,3.6c4.4,0,7.4-3.3,7.4-8.3v-24.7h5.6v24.7C210.2,347.5,204.8,353,197.2,353z"
        "M232.4,353c-8.1,0-15-6-15-15.8v-22.5h5.6v22.2c0,6.6,3.9,10.8,9.5,10.8c5.6,0,9.5-4.3,9.5-10.8v-22.2h5.6v22.5C247.5,347,240.5,353,232.4,353z"
        "M272,353c-10.8,0-19.5-8.6-19.5-19.3c0-10.8,8.8-19.3,19.5-19.3c4.8,0,9,1.6,12.3,4.4l-3.3,4.1c-3.4-2.4-5.7-3.2-8.9-3.2c-7.7,0-13.8,6.2-13.8,14.1c0,7.9,6.1,14.1,13.8,14.1c3.1,0,5.6-1,8.8-3.2l3.3,4.1C280.1,351.9,276.4,353,272,353z"
        "M290.4,352.5v-37.8h22.7v5H296v11.2h16.5v5H296v11.6h17.2v5H290.4z");
};
