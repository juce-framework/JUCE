/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../../Assets/DemoUtilities.h"

#include "UI/MainComponent.h"

//==============================================================================
#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
 // Just add a simple icon to the Window system tray area or Mac menu bar..
 struct DemoTaskbarComponent  : public SystemTrayIconComponent,
                                private Timer
 {
     DemoTaskbarComponent()
     {
         setIconImage (getImageFromAssets ("juce_icon.png"),
                       getImageFromAssets ("juce_icon_template.png"));
         setIconTooltip ("JUCE demo runner!");
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

     void timerCallback() override
     {
         stopTimer();

         PopupMenu m;
         m.addItem (1, "Quit");

         // It's always better to open menus asynchronously when possible.
         m.showMenuAsync (PopupMenu::Options(),
                          ModalCallbackFunction::forComponent (menuInvocationCallback, this));
     }
 };
#endif

std::unique_ptr<AudioDeviceManager> sharedAudioDeviceManager;

//==============================================================================
class DemoRunnerApplication  : public JUCEApplication
{
public:
    //==============================================================================
    DemoRunnerApplication() {}

    ~DemoRunnerApplication() override
    {
        sharedAudioDeviceManager.reset();
    }

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        registerAllDemos();

      #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX
        // (This function call is for one of the demos, which involves launching a child process)
        if (invokeChildProcessDemo (commandLine))
            return;
      #else
        ignoreUnused (commandLine);
      #endif

        mainWindow.reset (new MainAppWindow (getApplicationName()));
    }

    bool backButtonPressed() override    { mainWindow->getMainComponent().getSidePanel().showOrHide (false); return true; }
    void shutdown() override             { mainWindow = nullptr; }

    //==============================================================================
    void systemRequestedQuit() override                                 { quit(); }
    void anotherInstanceStarted (const String&) override                {}

private:
    class MainAppWindow    : public DocumentWindow
    {
    public:
        MainAppWindow (const String& name)
            : DocumentWindow (name, Desktop::getInstance().getDefaultLookAndFeel()
                                                          .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setResizable (true, false);
            setResizeLimits (400, 400, 10000, 10000);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
            Desktop::getInstance().setOrientationsEnabled (Desktop::rotatedClockwise | Desktop::rotatedAntiClockwise);
           #else
            setBounds ((int) (0.1f * getParentWidth()),
                       (int) (0.1f * getParentHeight()),
                       jmax (850, (int) (0.5f * getParentWidth())),
                       jmax (600, (int) (0.7f * getParentHeight())));
           #endif

            setContentOwned (new MainComponent(), false);
            setVisible (true);

           #if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC
            taskbarIcon.reset (new DemoTaskbarComponent());
           #endif
        }

        void closeButtonPressed() override    { JUCEApplication::getInstance()->systemRequestedQuit(); }

        //==============================================================================
        MainComponent& getMainComponent()    { return *dynamic_cast<MainComponent*> (getContentComponent()); }

    private:
        std::unique_ptr<Component> taskbarIcon;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAppWindow)
    };

    std::unique_ptr<MainAppWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (DemoRunnerApplication)
