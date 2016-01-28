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

#include "../JuceLibraryCode/JuceHeader.h"

namespace
{
    String getIPAddress()
    {
        Array<IPAddress> addresses;
        IPAddress::findAllAddresses (addresses);
        return addresses[1].toString();
    }

    String getBroadcastIPAddress()
    {
        return getIPAddress().upToLastOccurrenceOf (".", false, false) + ".255";
    }

    static const int masterPortNumber = 9001;  // the UDP port the master sends on / the clients receive.
    static const int clientPortNumber = 9002;  // the UDP port the clients send on / the master receives.

    static const String canvasStateOSCAddress = "/juce/nfd/canvasState";
    static const String newClientOSCAddress   = "/juce/nfd/newClient";
    static const String userInputOSCAddress   = "/juce/nfd/userInput";
};

#include "SharedCanvas.h"
#include "SlaveComponent.h"
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
        MainWindow (PropertiesFile& props)
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
            setContentOwned (new SlaveCanvasComponent (props, windowIndex), true);
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

        ~MainWindow()
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
