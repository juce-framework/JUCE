/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ChildProcessDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Launches applications as child processes.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make

 type:             Console
 mainClass:        ChildProcessDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
// This is a token that's used at both ends of our parent-child processes, to
// act as a unique token in the command line arguments.
static const char* demoCommandLineUID = "demoUID";

// A few quick utility functions to convert between raw data and ValueTrees
static ValueTree memoryBlockToValueTree (const MemoryBlock& mb)
{
    return ValueTree::readFromData (mb.getData(), mb.getSize());
}

static MemoryBlock valueTreeToMemoryBlock (const ValueTree& v)
{
    MemoryOutputStream mo;
    v.writeToStream (mo);

    return mo.getMemoryBlock();
}

static String valueTreeToString (const ValueTree& v)
{
    std::unique_ptr<XmlElement> xml (v.createXml());

    if (xml.get() != nullptr)
        return xml->createDocument ({}, true, false);

    return {};
}

//==============================================================================
class ChildProcessDemo   : public Component,
                           private MessageListener
{
public:
    ChildProcessDemo()
    {
        setOpaque (true);

        addAndMakeVisible (launchButton);
        launchButton.onClick = [this] { launchChildProcess(); };

        addAndMakeVisible (pingButton);
        pingButton.onClick = [this] { pingChildProcess(); };

        addAndMakeVisible (killButton);
        killButton.onClick = [this] { killChildProcess(); };

        addAndMakeVisible (testResultsBox);
        testResultsBox.setMultiLine (true);
        testResultsBox.setFont ({ Font::getDefaultMonospacedFontName(), 12.0f, Font::plain });

        logMessage (String ("This demo uses the ChildProcessMaster and ChildProcessSlave classes to launch and communicate "
                            "with a child process, sending messages in the form of serialised ValueTree objects.") + newLine);

        setSize (500, 500);
    }

    ~ChildProcessDemo()
    {
        masterProcess.reset();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds();

        auto top = area.removeFromTop (40);
        launchButton.setBounds (top.removeFromLeft (180).reduced (8));
        pingButton  .setBounds (top.removeFromLeft (180).reduced (8));
        killButton  .setBounds (top.removeFromLeft (180).reduced (8));

        testResultsBox.setBounds (area.reduced (8));
    }

    // Appends a message to the textbox that's shown in the demo as the console
    void logMessage (const String& message)
    {
        postMessage (new LogMessage (message));
    }

    // invoked by the 'launch' button.
    void launchChildProcess()
    {
        if (masterProcess.get() == nullptr)
        {
            masterProcess.reset (new DemoMasterProcess (*this));

            if (masterProcess->launchSlaveProcess (File::getSpecialLocation (File::currentExecutableFile), demoCommandLineUID))
                logMessage ("Child process started");
        }
    }

    // invoked by the 'ping' button.
    void pingChildProcess()
    {
        if (masterProcess.get() != nullptr)
            masterProcess->sendPingMessageToSlave();
        else
            logMessage ("Child process is not running!");
    }

    // invoked by the 'kill' button.
    void killChildProcess()
    {
        if (masterProcess.get() != nullptr)
        {
            masterProcess.reset();
            logMessage ("Child process killed");
        }
    }

    //==============================================================================
    // This class is used by the main process, acting as the master and receiving messages
    // from the slave process.
    class DemoMasterProcess  : public ChildProcessMaster,
                               private DeletedAtShutdown
    {
    public:
        DemoMasterProcess (ChildProcessDemo& d) : demo (d) {}

        // This gets called when a message arrives from the slave process..
        void handleMessageFromSlave (const MemoryBlock& mb) override
        {
            auto incomingMessage = memoryBlockToValueTree (mb);

            demo.logMessage ("Received: " + valueTreeToString (incomingMessage));
        }

        // This gets called if the slave process dies.
        void handleConnectionLost() override
        {
            demo.logMessage ("Connection lost to child process!");
            demo.killChildProcess();
        }

        void sendPingMessageToSlave()
        {
            ValueTree message ("MESSAGE");
            message.setProperty ("count", count++, nullptr);

            demo.logMessage ("Sending: " + valueTreeToString (message));

            sendMessageToSlave (valueTreeToMemoryBlock (message));
        }

        ChildProcessDemo& demo;
        int count = 0;
    };

    //==============================================================================
    std::unique_ptr<DemoMasterProcess> masterProcess;

private:
    TextButton launchButton  { "Launch Child Process" };
    TextButton pingButton    { "Send Ping" };
    TextButton killButton    { "Kill Child Process" };

    TextEditor testResultsBox;

    struct LogMessage  : public Message
    {
        LogMessage (const String& m) : message (m) {}

        String message;
    };

    void handleMessage (const Message& message) override
    {
        testResultsBox.moveCaretToEnd();
        testResultsBox.insertTextAtCaret (static_cast<const LogMessage&> (message).message + newLine);
        testResultsBox.moveCaretToEnd();
    }

    void lookAndFeelChanged() override
    {
        testResultsBox.applyFontToAllText (testResultsBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcessDemo)
};

//==============================================================================
/*  This class gets instantiated in the child process, and receives messages from
    the master process.
*/
class DemoSlaveProcess  : public ChildProcessSlave,
                          private DeletedAtShutdown
{
public:
    DemoSlaveProcess() {}

    void handleMessageFromMaster (const MemoryBlock& mb) override
    {
        ValueTree incomingMessage (memoryBlockToValueTree (mb));

        /*  In this demo we're only expecting one type of message, which will contain a 'count' parameter -
            we'll just increment that number and send back a new message containing the new number.

            Obviously in a real app you'll probably want to look at the type of the message, and do
            some more interesting behaviour.
        */

        ValueTree reply ("REPLY");
        reply.setProperty ("countPlusOne", static_cast<int> (incomingMessage["count"]) + 1, nullptr);

        sendMessageToMaster (valueTreeToMemoryBlock (reply));
    }

    void handleConnectionMade() override
    {
        // This method is called when the connection is established, and in response, we'll just
        // send off a message to say hello.
        ValueTree reply ("HelloWorld");
        sendMessageToMaster (valueTreeToMemoryBlock (reply));
    }

    /* If no pings are received from the master process for a number of seconds, then this will get invoked.
       Typically you'll want to use this as a signal to kill the process as quickly as possible, as you
       don't want to leave it hanging around as a zombie..
    */
    void handleConnectionLost() override
    {
        JUCEApplication::quit();
    }
};

//==============================================================================
/*  The JUCEApplication::initialise method calls this function to allow the
    child process to launch when the command line parameters indicate that we're
    being asked to run as a child process..
*/
bool invokeChildProcessDemo (const String& commandLine)
{
    std::unique_ptr<DemoSlaveProcess> slave (new DemoSlaveProcess());

    if (slave->initialiseFromCommandLine (commandLine, demoCommandLineUID))
    {
        slave.release(); // allow the slave object to stay alive - it'll handle its own deletion.
        return true;
    }

    return false;
}

#ifndef JUCE_DEMO_RUNNER
 //==============================================================================
 // As we need to modify the JUCEApplication::initialise method to launch the child process
 // based on the command line parameters, we can't just use the normal auto-generated Main.cpp.
 // Instead, we don't do anything in Main.cpp and create a JUCEApplication subclass here with
 // the necessary modifications.
 class Application    : public JUCEApplication
 {
 public:
     //==============================================================================
     Application() {}

     const String getApplicationName() override              { return "ChildProcessDemo"; }
     const String getApplicationVersion() override           { return "1.0.0"; }

     void initialise (const String& commandLine) override
     {
         // launches the child process if the command line parameters contain the demo UID
         if (invokeChildProcessDemo (commandLine))
             return;

         mainWindow.reset (new MainWindow ("ChildProcessDemo", new ChildProcessDemo()));
     }

     void shutdown() override                                { mainWindow = nullptr; }

 private:
     class MainWindow    : public DocumentWindow
     {
     public:
         MainWindow (const String& name, Component* c)  : DocumentWindow (name,
                                                                          Desktop::getInstance().getDefaultLookAndFeel()
                                                                                                .findColour (ResizableWindow::backgroundColourId),
                                                                          DocumentWindow::allButtons)
         {
             setUsingNativeTitleBar (true);
             setContentOwned (c, true);

             centreWithSize (getWidth(), getHeight());

             setVisible (true);
         }

         void closeButtonPressed() override
         {
             JUCEApplication::getInstance()->systemRequestedQuit();
         }

     private:
         JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
     };

     std::unique_ptr<MainWindow> mainWindow;
 };

 //==============================================================================
 START_JUCE_APPLICATION (Application)
#endif
