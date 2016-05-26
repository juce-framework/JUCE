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

#ifndef __MAINWINDOW_H_7DB41986__
#define __MAINWINDOW_H_7DB41986__

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

        useLookAndFeelV1            = 0x200b,
        useLookAndFeelV2            = 0x200c,
        useLookAndFeelV3            = 0x200d,

        toggleRepaintDebugging      = 0x200e,

        useNativeTitleBar           = 0x201d,
        goToKioskMode               = 0x200f
    };

private:

    static void runtimPermissionsCallback (bool wasGranted);

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


#endif  // __MAINWINDOW_H_7DB41986__
