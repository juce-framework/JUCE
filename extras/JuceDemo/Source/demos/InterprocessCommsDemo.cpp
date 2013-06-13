/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"


//==============================================================================
class InterprocessCommsDemo  : public Component,
                               public ButtonListener,
                               public ComboBoxListener
{
public:
    //==============================================================================
    InterprocessCommsDemo()
        : sendButton ("send", "Fires off the message"),
          modeLabel (String::empty, "Mode:"),
          pipeLabel (String::empty, "Pipe Name:"),
          numberLabel (String::empty, "Socket Port:"),
          hostLabel (String::empty, "Socket Host:")
    {
        setName ("Interprocess Communication");

        server = new DemoInterprocessConnectionServer (*this);

        // create all our UI bits and pieces..
        addAndMakeVisible (&modeSelector);
        modeSelector.setBounds (100, 25, 200, 24);
        modeLabel.attachToComponent (&modeSelector, true);

        modeSelector.addItem ("(Disconnected)", 8);
        modeSelector.addSeparator();
        modeSelector.addItem ("Named pipe (listening)", 1);
        modeSelector.addItem ("Named pipe (connect to existing pipe)", 5);
        modeSelector.addSeparator();
        modeSelector.addItem ("Socket (listening)", 2);
        modeSelector.addItem ("Socket (connect to existing socket)", 6);

        modeSelector.setSelectedId (8);
        modeSelector.addListener (this);

        addAndMakeVisible (&pipeName);
        pipeName.setBounds (100, 60, 130, 24);
        pipeName.setMultiLine (false);
        pipeName.setText ("juce demo pipe");
        pipeLabel.attachToComponent (&pipeName, true);

        addAndMakeVisible (&socketNumber);
        socketNumber.setBounds (350, 60, 80, 24);
        socketNumber.setMultiLine (false);
        socketNumber.setText ("12345");
        socketNumber.setInputRestrictions (5, "0123456789");
        numberLabel.attachToComponent (&socketNumber, true);

        addAndMakeVisible (&socketHost);
        socketHost.setBounds (530, 60, 130, 24);
        socketHost.setMultiLine (false);
        socketHost.setText ("localhost");
        socketNumber.setInputRestrictions (512);
        hostLabel.attachToComponent (&socketHost, true);

        addChildComponent (&sendText);
        sendText.setBounds (30, 120, 200, 24);
        sendText.setMultiLine (false);
        sendText.setReadOnly (false);
        sendText.setText ("testing 1234");

        addChildComponent (&sendButton);
        sendButton.setBounds (240, 120, 200, 24);
        sendButton.changeWidthToFitText();
        sendButton.addListener (this);

        addChildComponent (&incomingMessages);
        incomingMessages.setReadOnly (true);
        incomingMessages.setMultiLine (true);
        incomingMessages.setBounds (30, 150, 500, 250);

        // call this to set up everything's state correctly.
        comboBoxChanged (0);
    }

    ~InterprocessCommsDemo()
    {
        close();
    }

    void buttonClicked (Button* button)
    {
        if (button == &sendButton)
        {
            // The send button has been pressed, so write out the contents of the
            // text box to the socket or pipe, depending on which is active.
            const String text (sendText.getText());
            MemoryBlock messageData (text.toUTF8(), text.getNumBytesAsUTF8());

            for (int i = activeConnections.size(); --i >= 0;)
            {
                if (! activeConnections[i]->sendMessage (messageData))
                {
                    // the write failed, so indicate that the connection has broken..
                    appendMessage ("send message failed!");
                }
            }
        }
    }

    void comboBoxChanged (ComboBox*)
    {
        // This is called when the user picks a different mode from the drop-down list..
        const int modeId = modeSelector.getSelectedId();

        close();

        if (modeId < 8)
        {
            open ((modeId & 2) != 0,
                  (modeId & 4) != 0);
        }
    }

    //==============================================================================
    // Just closes any connections that are currently open.
    void close()
    {
        server->stop();
        activeConnections.clear();

        // Reset the UI stuff to a disabled state.
        sendText.setVisible (false);
        sendButton.setVisible (false);
        incomingMessages.setText (String::empty, false);
        incomingMessages.setVisible (true);

        appendMessage (
            "To demonstrate named pipes, you'll need to run two instances of the JuceDemo application on this machine. On "
            "one of them, select \"named pipe (listening)\", and then on the other, select \"named pipe (connect to existing pipe)\". Then messages that you "
            "send from the 'sender' app should appear on the listener app. The \"pipe name\" field lets you choose a name for the pipe\n\n"
            "To demonstrate sockets, you can either run two instances of the app on the same machine, or on different "
            "machines on your network. In each one enter a socket number, then on one of the apps, select the "
            "\"Socket (listening)\" mode. On the other, enter the host address of the listening app, and select \"Socket (connect to existing socket)\". "
            "Messages should then be be sent between the apps in the same way as through the named pipes.");
    }

    void open (bool asSocket, bool asSender)
    {
        close();

        // Make the appropriate bits of UI visible..
        sendText.setVisible (true);
        sendButton.setVisible (true);

        incomingMessages.setText (String::empty, false);
        incomingMessages.setVisible (true);

        // and try to open the socket or pipe...
        bool openedOk = false;

        if (asSender)
        {
            // if we're connecting to an existing server, we can just create a connection object
            // directly.
            ScopedPointer<DemoInterprocessConnection> newConnection (new DemoInterprocessConnection (*this));

            if (asSocket)
            {
                openedOk = newConnection->connectToSocket (socketHost.getText(),
                                                           socketNumber.getText().getIntValue(),
                                                           1000);
            }
            else
            {
                openedOk = newConnection->connectToPipe (pipeName.getText(), 5000);
            }

            if (openedOk)
                activeConnections.add (newConnection.release());
        }
        else
        {
            // if we're starting up a server, we need to tell the server to start waiting for
            // clients to connect. It'll then create connection objects for us when clients arrive.
            if (asSocket)
            {
                openedOk = server->beginWaitingForSocket (socketNumber.getText().getIntValue());

                if (openedOk)
                    appendMessage ("Waiting for another app to connect to this socket..");
            }
            else
            {
                ScopedPointer<DemoInterprocessConnection> newConnection (new DemoInterprocessConnection (*this));

                openedOk = newConnection->createPipe (pipeName.getText(), 2000);

                if (openedOk)
                {
                    appendMessage ("Waiting for another app to connect to this pipe..");
                    activeConnections.add (newConnection.release());
                }
            }
        }

        if (! openedOk)
        {
            modeSelector.setSelectedId (8);

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Interprocess Comms Demo",
                                              "Failed to open the socket or pipe...");
        }
    }

    void appendMessage (const String& message)
    {
        incomingMessages.setCaretPosition (INT_MAX);
        incomingMessages.insertTextAtCaret (message + "\n");
        incomingMessages.setCaretPosition (INT_MAX);
    }

    //==============================================================================
    class DemoInterprocessConnection  : public InterprocessConnection
    {
    public:
        DemoInterprocessConnection (InterprocessCommsDemo& owner_)
            : InterprocessConnection (true),
              owner (owner_)
        {
            static int totalConnections = 0;
            ourNumber = ++totalConnections;
        }

        void connectionMade()
        {
            owner.appendMessage ("Connection #" + String (ourNumber) + " - connection started");
        }

        void connectionLost()
        {
            owner.appendMessage ("Connection #" + String (ourNumber) + " - connection lost");
        }

        void messageReceived (const MemoryBlock& message)
        {
            owner.appendMessage ("Connection #" + String (ourNumber) + " - message received: " + message.toString());
        }

    private:
        InterprocessCommsDemo& owner;
        int ourNumber;
    };

    //==============================================================================
    class DemoInterprocessConnectionServer   : public InterprocessConnectionServer
    {
    public:
        DemoInterprocessConnectionServer (InterprocessCommsDemo& owner_)
            : owner (owner_)
        {
        }

        InterprocessConnection* createConnectionObject()
        {
            DemoInterprocessConnection* newConnection = new DemoInterprocessConnection (owner);

            owner.activeConnections.add (newConnection);
            return newConnection;
        }

    private:
        InterprocessCommsDemo& owner;
    };

    OwnedArray <DemoInterprocessConnection, CriticalSection> activeConnections;


private:
    ComboBox modeSelector;
    TextButton sendButton;
    TextEditor sendText, incomingMessages, pipeName, socketNumber, socketHost;
    Label modeLabel, pipeLabel, numberLabel, hostLabel;

    ScopedPointer<DemoInterprocessConnectionServer> server;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InterprocessCommsDemo)
};


//==============================================================================
Component* createInterprocessCommsDemo()
{
    return new InterprocessCommsDemo();
}
