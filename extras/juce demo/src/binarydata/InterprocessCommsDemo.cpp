/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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
    {
        server = new DemoInterprocessConnectionServer (*this);

        setName (T("Interprocess Communication"));

        // create all our UI bits and pieces..
        addAndMakeVisible (modeSelector = new ComboBox (T("mode:")));
        modeSelector->setBounds (100, 25, 200, 24);
        (new Label (modeSelector->getName(), modeSelector->getName()))->attachToComponent (modeSelector, true);

        modeSelector->addItem (T("(Disconnected)"), 8);
        modeSelector->addSeparator();
        modeSelector->addItem (T("Named pipe (listening)"), 1);
        modeSelector->addItem (T("Named pipe (connect to existing pipe)"), 5);
        modeSelector->addSeparator();
        modeSelector->addItem (T("Socket (listening)"), 2);
        modeSelector->addItem (T("Socket (connect to existing socket)"), 6);

        modeSelector->setSelectedId (8);
        modeSelector->addListener (this);

        addAndMakeVisible (pipeName = new TextEditor (T("pipe name:")));
        pipeName->setBounds (100, 60, 130, 24);
        pipeName->setMultiLine (false);
        pipeName->setText (T("juce demo pipe"));
        (new Label (pipeName->getName(), pipeName->getName()))->attachToComponent (pipeName, true);

        addAndMakeVisible (socketNumber = new TextEditor (T("socket port:")));
        socketNumber->setBounds (350, 60, 80, 24);
        socketNumber->setMultiLine (false);
        socketNumber->setText (T("12345"));
        socketNumber->setInputRestrictions (5, T("0123456789"));
        (new Label (socketNumber->getName(), socketNumber->getName()))->attachToComponent (socketNumber, true);

        addAndMakeVisible (socketHost = new TextEditor (T("socket host:")));
        socketHost->setBounds (530, 60, 130, 24);
        socketHost->setMultiLine (false);
        socketHost->setText (T("localhost"));
        socketNumber->setInputRestrictions (512);
        (new Label (socketHost->getName(), socketHost->getName()))->attachToComponent (socketHost, true);

        addChildComponent (sendText = new TextEditor (T("sendtext")));
        sendText->setBounds (30, 120, 200, 24);
        sendText->setMultiLine (false);
        sendText->setReadOnly (false);
        sendText->setText (T("testing 1234"));

        addChildComponent (sendButton = new TextButton (T("send"), T("Fires off the message")));
        sendButton->setBounds (240, 120, 200, 24);
        sendButton->changeWidthToFitText();
        sendButton->addButtonListener (this);

        addChildComponent (incomingMessages = new TextEditor (T("messages")));
        incomingMessages->setReadOnly (true);
        incomingMessages->setMultiLine (true);
        incomingMessages->setBounds (30, 150, 500, 250);

        // call this to set up everything's state correctly.
        comboBoxChanged (0);
    }

    ~InterprocessCommsDemo()
    {
        close();
        delete server;

        deleteAllChildren();
    }

    void buttonClicked (Button* button)
    {
        if (button == sendButton)
        {
            // The send button has been pressed, so write out the contents of the
            // text box to the socket or pipe, depending on which is active.
            const String text (sendText->getText());
            MemoryBlock messageData ((const char*) text, text.length());

            for (int i = activeConnections.size(); --i >= 0;)
            {
                if (! activeConnections[i]->sendMessage (messageData))
                {
                    // the write failed, so indicate that the connection has broken..
                    appendMessage (T("send message failed!"));
                }
            }
        }
    }

    void comboBoxChanged (ComboBox*)
    {
        // This is called when the user picks a different mode from the drop-down list..
        const int modeId = modeSelector->getSelectedId();

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
        sendText->setVisible (false);
        sendButton->setVisible (false);
        incomingMessages->setText (String::empty, false);
        incomingMessages->setVisible (true);

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
        sendText->setVisible (true);
        sendButton->setVisible (true);

        incomingMessages->setText (String::empty, false);
        incomingMessages->setVisible (true);

        // and try to open the socket or pipe...
        bool openedOk = false;

        if (asSender)
        {
            // if we're connecting to an existing server, we can just create a connection object
            // directly.
            DemoInterprocessConnection* newConnection = new DemoInterprocessConnection (*this);

            if (asSocket)
            {
                openedOk = newConnection->connectToSocket (socketHost->getText(),
                                                           socketNumber->getText().getIntValue(),
                                                           1000);
            }
            else
            {
                openedOk = newConnection->connectToPipe (pipeName->getText());
            }

            if (openedOk)
                activeConnections.add (newConnection);
            else
                delete newConnection;
        }
        else
        {
            // if we're starting up a server, we need to tell the server to start waiting for
            // clients to connect. It'll then create connection objects for us when clients arrive.
            if (asSocket)
            {
                openedOk = server->beginWaitingForSocket (socketNumber->getText().getIntValue());

                if (openedOk)
                    appendMessage (T("Waiting for another app to connect to this socket.."));
            }
            else
            {
                DemoInterprocessConnection* newConnection = new DemoInterprocessConnection (*this);

                openedOk = newConnection->createPipe (pipeName->getText());

                if (openedOk)
                {
                    appendMessage (T("Waiting for another app to connect to this pipe.."));
                    activeConnections.add (newConnection);
                }
                else
                {
                    delete newConnection;
                }
            }
        }

        if (! openedOk)
        {
            modeSelector->setSelectedId (8);

            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         T("Interprocess Comms Demo"),
                                         T("Failed to open the socket or pipe..."));
        }
    }

    void appendMessage (const String& message)
    {
        incomingMessages->setCaretPosition (INT_MAX);
        incomingMessages->insertTextAtCursor (message + T("\n"));
        incomingMessages->setCaretPosition (INT_MAX);
    }

    //==============================================================================
    class DemoInterprocessConnection  : public InterprocessConnection
    {
        InterprocessCommsDemo& owner;
        int ourNumber;

    public:
        DemoInterprocessConnection (InterprocessCommsDemo& owner_)
            : InterprocessConnection (true),
              owner (owner_)
        {
            static int totalConnections = 0;
            ourNumber = ++totalConnections;
        }

        ~DemoInterprocessConnection()
        {
        }

        void connectionMade()
        {
            owner.appendMessage (T("Connection #") + String (ourNumber) + T(" - connection started"));
        }

        void connectionLost()
        {
            owner.appendMessage (T("Connection #") + String (ourNumber) + T(" - connection lost"));
        }

        void messageReceived (const MemoryBlock& message)
        {
            owner.appendMessage (T("Connection #") + String (ourNumber) + T(" - message received: ") + message.toString());
        }
    };

    //==============================================================================
    class DemoInterprocessConnectionServer   : public InterprocessConnectionServer
    {
        InterprocessCommsDemo& owner;

    public:
        DemoInterprocessConnectionServer (InterprocessCommsDemo& owner_)
            : owner (owner_)
        {
        }

        ~DemoInterprocessConnectionServer()
        {
        }

        InterprocessConnection* createConnectionObject()
        {
            DemoInterprocessConnection* newConnection = new DemoInterprocessConnection (owner);

            owner.activeConnections.add (newConnection);
            return newConnection;
        }
    };

    OwnedArray <DemoInterprocessConnection, CriticalSection> activeConnections;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ComboBox* modeSelector;
    TextEditor* sendText;
    TextButton* sendButton;
    TextEditor* incomingMessages;

    TextEditor* pipeName;
    TextEditor* socketNumber;
    TextEditor* socketHost;

    DemoInterprocessConnectionServer* server;
};


//==============================================================================
Component* createInterprocessCommsDemo()
{
    return new InterprocessCommsDemo();
}
