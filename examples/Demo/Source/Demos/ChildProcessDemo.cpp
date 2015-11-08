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

#include "../JuceDemoHeader.h"

#if JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX

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
    const ScopedPointer<XmlElement> xml (v.createXml());
    return xml != nullptr ? xml->createDocument ("", true, false) : String();
}

//==============================================================================
class ChildProcessDemo   : public Component,
                           private Button::Listener,
                           private MessageListener
{
public:
    ChildProcessDemo()
    {
        setOpaque (true);

        addAndMakeVisible (launchButton);
        launchButton.setButtonText ("Launch Child Process");
        launchButton.addListener (this);

        addAndMakeVisible (pingButton);
        pingButton.setButtonText ("Send Ping");
        pingButton.addListener (this);

        addAndMakeVisible (killButton);
        killButton.setButtonText ("Kill Child Process");
        killButton.addListener (this);

        addAndMakeVisible (testResultsBox);
        testResultsBox.setMultiLine (true);
        testResultsBox.setFont (Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));

        logMessage (String ("This demo uses the ChildProcessMaster and ChildProcessSlave classes to launch and communicate "
                            "with a child process, sending messages in the form of serialised ValueTree objects.") + newLine);
    }

    ~ChildProcessDemo()
    {
        masterProcess = nullptr;
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        Rectangle<int> top (area.removeFromTop (40));
        launchButton.setBounds (top.removeFromLeft (180).reduced (8));
        pingButton.setBounds (top.removeFromLeft (180).reduced (8));
        killButton.setBounds (top.removeFromLeft (180).reduced (8));
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
        if (masterProcess == nullptr)
        {
            masterProcess = new DemoMasterProcess (*this);

            if (masterProcess->launchSlaveProcess (File::getSpecialLocation (File::currentExecutableFile), demoCommandLineUID))
                logMessage ("Child process started");
        }
    }

    // invoked by the 'ping' button.
    void pingChildProcess()
    {
        if (masterProcess != nullptr)
            masterProcess->sendPingMessageToSlave();
        else
            logMessage ("Child process is not running!");
    }

    // invoked by the 'kill' button.
    void killChildProcess()
    {
        if (masterProcess != nullptr)
        {
            masterProcess = nullptr;
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
        DemoMasterProcess (ChildProcessDemo& d) : demo (d), count (0) {}

        // This gets called when a message arrives from the slave process..
        void handleMessageFromSlave (const MemoryBlock& mb) override
        {
            ValueTree incomingMessage (memoryBlockToValueTree (mb));

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
        int count;
    };

    //==============================================================================
    ScopedPointer<DemoMasterProcess> masterProcess;

private:
    TextButton launchButton, pingButton, killButton;
    TextEditor testResultsBox;

    void buttonClicked (Button* button) override
    {
        if (button == &launchButton)  launchChildProcess();
        if (button == &pingButton)    pingChildProcess();
        if (button == &killButton)    killChildProcess();
    }

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

        /*  In the demo we're only expecting one type of message, which will contain a 'count' parameter -
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
/*  The JuceDemoApplication::initialise method calls this function to allow the
    child process to launch when the command line parameters indicate that we're
    being asked to run as a child process..
*/
bool invokeChildProcessDemo (const String& commandLine)
{
    ScopedPointer<DemoSlaveProcess> slave (new DemoSlaveProcess());

    if (slave->initialiseFromCommandLine (commandLine, demoCommandLineUID))
    {
        slave.release(); // allow the slave object to stay alive - it'll handle its own deletion.
        return true;
    }

    return false;
}

// This static object will register this demo type in a global list of demos..
static JuceDemoType<ChildProcessDemo> childProcessDemo ("40 Child Process Comms");

#else

// (Dummy stub for platforms that don't support this demo)
bool invokeChildProcessDemo (const String&)    { return false; }

#endif
