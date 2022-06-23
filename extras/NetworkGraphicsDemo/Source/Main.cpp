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

#include <JuceHeader.h>

namespace
{
    String getBroadcastIPAddress()
    {
        return IPAddress::getLocalAddress().toString().upToLastOccurrenceOf (".", false, false) + ".255";
    }

    static const int masterPortNumber = 9001;  // the UDP port the master sends on / the clients receive.
    static const int clientPortNumber = 9002;  // the UDP port the clients send on / the master receives.

    static const String canvasStateOSCAddress = "/juce/nfd/canvasState";
    static const String newClientOSCAddress   = "/juce/nfd/newClient";
    static const String userInputOSCAddress   = "/juce/nfd/userInput";
}

#include "SharedCanvas.h"
#include "ClientComponent.h"
#include "Demos.h"
#include "MasterComponent.h"


//==============================================================================
class NetworkGraphicsDemoApplication  : public JUCEApplication
{
public:
    NetworkGraphicsDemoApplication()  : properties (getPropertyFileOptions())
    {}

    const String getApplicationName() override           { return ProjectInfo::projectName; }
    const String getApplicationVersion() override        { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override           { return true; }
    void anotherInstanceStarted (const String&) override {}

    //==============================================================================
    void initialise (const String& commandLine) override
    {
       #if ! JUCE_IOS && ! JUCE_ANDROID
        // Run as the master if we have a command-line flag "master" or if the exe itself
        // has been renamed to include the word "master"..
        bool isMaster = commandLine.containsIgnoreCase ("master")
                          || File::getSpecialLocation (File::currentApplicationFile)
                                .getFileName().containsIgnoreCase ("master");

        if (isMaster)
            mainWindows.add (new MainWindow (properties));
       #endif

        mainWindows.add (new MainWindow (properties, 0));

        Desktop::getInstance().setScreenSaverEnabled (false);
    }

    void shutdown() override
    {
        mainWindows.clear();
        properties.saveIfNeeded();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    //==============================================================================
    struct MainWindow    : public DocumentWindow
    {
        explicit MainWindow (PropertiesFile& props)
            : DocumentWindow ("JUCE Networked Graphics Demo - Master", Colours::white, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MasterContentComponent (props), true);
            setBounds (100, 50, getWidth(), getHeight());
            setResizable (true, false);
            setVisible (true);

            glContext.attachTo (*this);
        }

        MainWindow (PropertiesFile& props, int windowIndex)
            : DocumentWindow ("JUCE Networked Graphics Demo", Colours::black, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new ClientCanvasComponent (props, windowIndex), true);
            setBounds (500, 100, getWidth(), getHeight());
            setResizable (true, false);
            setVisible (true);

           #if ! JUCE_IOS
            glContext.attachTo (*this);
           #endif

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #endif
        }

        ~MainWindow() override
        {
            glContext.detach();
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        OpenGLContext glContext;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    static PropertiesFile::Options getPropertyFileOptions()
    {
        PropertiesFile::Options o;
        o.applicationName = "JUCE Network Graphics Demo";
        o.filenameSuffix = ".settings";
        o.folderName = "JUCE Network Graphics Demo";
        o.osxLibrarySubFolder = "Application Support/JUCE Network Graphics Demo";
        o.millisecondsBeforeSaving = 2000;
        return o;
    }

    PropertiesFile properties;
    OwnedArray<MainWindow> mainWindows;
};


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (NetworkGraphicsDemoApplication)
