/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "../../Assets/DemoUtilities.h"

#include "UI/MainComponent.h"

//==============================================================================
#if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
 // Just add a simple icon to the Window system tray area or Mac menu bar..
 struct DemoTaskbarComponent final : public SystemTrayIconComponent,
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
class DemoRunnerApplication final : public JUCEApplication
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

      #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
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
    void systemRequestedQuit() override                   { quit(); }
    void anotherInstanceStarted (const String&) override  {}

    ApplicationCommandManager& getGlobalCommandManager()  { return commandManager; }

private:
    class MainAppWindow final : public DocumentWindow
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

            auto& desktop = Desktop::getInstance();

            desktop.setOrientationsEnabled (Desktop::allOrientations);
            desktop.setKioskModeComponent (this);
           #else
            setBounds ((int) (0.1f * (float) getParentWidth()),
                       (int) (0.1f * (float) getParentHeight()),
                       jmax (850, (int) (0.5f * (float) getParentWidth())),
                       jmax (600, (int) (0.7f * (float) getParentHeight())));
           #endif

            setContentOwned (new MainComponent(), false);
            setVisible (true);

           #if JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
            taskbarIcon.reset (new DemoTaskbarComponent());
           #endif
        }

        void closeButtonPressed() override    { JUCEApplication::getInstance()->systemRequestedQuit(); }

       #if JUCE_IOS || JUCE_ANDROID
        void parentSizeChanged() override
        {
            if (auto* comp = getContentComponent())
                comp->resized();
        }
       #endif

        //==============================================================================
        MainComponent& getMainComponent()    { return *dynamic_cast<MainComponent*> (getContentComponent()); }

    private:
        std::unique_ptr<Component> taskbarIcon;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAppWindow)
    };

    std::unique_ptr<MainAppWindow> mainWindow;
    ApplicationCommandManager commandManager;
};

ApplicationCommandManager& getGlobalCommandManager()
{
    return dynamic_cast<DemoRunnerApplication*> (JUCEApplication::getInstance())->getGlobalCommandManager();
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (DemoRunnerApplication)
