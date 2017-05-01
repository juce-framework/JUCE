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

#pragma once

class ContentComponent;

//==============================================================================
class MainAppWindow   : public DocumentWindow,
                        private AsyncUpdater
{
public:
    //==============================================================================
    MainAppWindow();
    ~MainAppWindow();

    static MainAppWindow* getMainAppWindow(); // returns the MainWindow if it exists.

    // called by the OS when the window's close button is pressed.
    void closeButtonPressed() override;

    // (return the command manager object used to dispatch command events)
    static ApplicationCommandManager& getApplicationCommandManager();

    // (returns a shared AudioDeviceManager object that all the demos can use)
    static AudioDeviceManager& getSharedAudioDeviceManager();

    StringArray getRenderingEngines() const;
    int getActiveRenderingEngine() const;
    void setRenderingEngine (int index);
    void setOpenGLRenderingEngine();

    // (returns the exploding JUCE logo path)
    static Path getJUCELogoPath();

    /* Note: Be careful when overriding DocumentWindow methods - the base class
       uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in you content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       implementation calls the superclass's method.
    */

    //==============================================================================
    enum CommandIDs
    {
        showPreviousDemo            = 0x2100,
        showNextDemo                = 0x2101,

        welcome                     = 0x2000,
        componentsAnimation         = 0x2001,
        componentsDialogBoxes       = 0x2002,
        componentsKeyMappings       = 0x2003,
        componentsMDI               = 0x2004,
        componentsPropertyEditors   = 0x2005,
        componentsTransforms        = 0x2006,
        componentsWebBrowsers       = 0x2007,
        componentsWidgets           = 0x2008,

        renderingEngineOne          = 0x2300,
        renderingEngineTwo          = 0x2301,
        renderingEngineThree        = 0x2302, // these must be contiguous!

        useLookAndFeelV1            = 0x300a,
        useLookAndFeelV2            = 0x300b,
        useLookAndFeelV3            = 0x300c,
        useLookAndFeelV4Dark        = 0x300d,
        useLookAndFeelV4Midnight    = 0x300e,
        useLookAndFeelV4Grey        = 0x300f,
        useLookAndFeelV4Light       = 0x3010,

        toggleRepaintDebugging      = 0x201a,

        useNativeTitleBar           = 0x201d,
        goToKioskMode               = 0x200f
    };

private:
    static void runtimePermissionsCallback (bool wasGranted);

    ScopedPointer<ContentComponent> contentComponent;
    ScopedPointer<Component> taskbarIcon;
    ScopedPointer<BubbleMessageComponent> currentBubbleMessage;

    TooltipWindow tooltipWindow; // to add tooltips to an application, you
                                 // just need to create one of these and leave it
                                 // there to do its work..

   #if JUCE_OPENGL
    OpenGLContext openGLContext;
   #endif

    void handleAsyncUpdate() override;
    void showMessageBubble (const String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainAppWindow)
};
