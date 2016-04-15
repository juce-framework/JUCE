/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "OSCLogListBox.h"


//==============================================================================
class MainContentComponent   : public Component,
                               private Button::Listener,
                               private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:

    //==============================================================================
    MainContentComponent()
        : portNumberLabel (new Label),
          portNumberField (new Label),
          connectButton (new TextButton ("Connect")),
          clearButton (new TextButton ("Clear")),
          connectionStatusLabel (new Label),
          oscLogListBox (new OSCLogListBox),
          oscReceiver (new OSCReceiver),
          currentPortNumber (-1)
    {
        setSize (700, 400);

        portNumberLabel->setText ("UDP Port Number: ", dontSendNotification);
        portNumberLabel->setBounds (10, 18, 130, 25);
        addAndMakeVisible (portNumberLabel);

        portNumberField->setText ("9001", dontSendNotification);
        portNumberField->setEditable (true, true, true);
        portNumberField->setBounds (140, 18, 50, 25);
        addAndMakeVisible (portNumberField);

        connectButton->setBounds (210, 18, 100, 25);
        addAndMakeVisible (connectButton);
        connectButton->addListener (this);

        clearButton->setBounds (320, 18, 60, 25);
        addAndMakeVisible (clearButton);
        clearButton->addListener (this);

        connectionStatusLabel->setBounds (450, 18, 240, 25);
        updateConnectionStatusLabel();
        addAndMakeVisible (connectionStatusLabel);

        oscLogListBox->setBounds (0, 60, 700, 340);
        addAndMakeVisible (oscLogListBox);

        oscReceiver->addListener (this);
        oscReceiver->registerFormatErrorHandler (
            [this] (const char* data, int dataSize)
            {
                oscLogListBox->addInvalidOSCPacket (data, dataSize);
            }
        );
    }

private:
    //==============================================================================
    ScopedPointer<Label> portNumberLabel;
    ScopedPointer<Label> portNumberField;
    ScopedPointer<TextButton> connectButton;
    ScopedPointer<TextButton> clearButton;
    ScopedPointer<Label> connectionStatusLabel;

    ScopedPointer<OSCLogListBox> oscLogListBox;
    ScopedPointer<OSCReceiver> oscReceiver;

    int currentPortNumber;


    //==============================================================================
    void buttonClicked (Button* b) override
    {
        if (b == connectButton)
            connectButtonClicked();
        else if (b == clearButton)
            clearButtonClicked();
    }

    //==============================================================================
    void connectButtonClicked()
    {
        if (! isConnected())
            connect();
        else
            disconnect();

        updateConnectionStatusLabel();
    }

    //==============================================================================
    void clearButtonClicked()
    {
        oscLogListBox->clear();
    }

    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        oscLogListBox->addOSCMessage (message);
    }

    void oscBundleReceived (const OSCBundle& bundle) override
    {
        oscLogListBox->addOSCBundle (bundle);
    }

    //==============================================================================
    void connect()
    {
        int portToConnect = portNumberField->getText().getIntValue();

        if (! isValidOscPort (portToConnect))
        {
            handleInvalidPortNumberEntered();
            return;
        }

        if (oscReceiver->connect (portToConnect))
        {
            currentPortNumber = portToConnect;
            connectButton->setButtonText ("Disconnect");
        }
        else
        {
            handleConnectError (portToConnect);
        }
    }

    //==============================================================================
    void disconnect()
    {
        if (oscReceiver->disconnect())
        {
            currentPortNumber = -1;
            connectButton->setButtonText ("Connect");
        }
        else
        {
            handleDisconnectError();
        }
    }

    //==============================================================================
    void handleConnectError (int failedPort)
    {
        AlertWindow::showMessageBoxAsync (
            AlertWindow::WarningIcon,
            "OSC Connection error",
            "Error: could not connect to port " + String (failedPort),
            "OK");
    }

    //==============================================================================
    void handleDisconnectError()
    {
        AlertWindow::showMessageBoxAsync (
            AlertWindow::WarningIcon,
            "Unknown error",
            "An unknown error occured while trying to disconnect from UPD port.",
            "OK");
    }

    //==============================================================================
    void handleInvalidPortNumberEntered()
    {
        AlertWindow::showMessageBoxAsync (
            AlertWindow::WarningIcon,
            "Invalid port number",
            "Error: you have entered an invalid UDP port number.",
            "OK");
    }

    //==============================================================================
    bool isConnected()
    {
        return currentPortNumber != -1;
    }

    //==============================================================================
    bool isValidOscPort (int port)
    {
        return port > 0 && port < 65536;
    }

    //==============================================================================
    void updateConnectionStatusLabel()
    {
        String text = "Status: ";
        if (isConnected())
            text += "Connected to UDP port " + String (currentPortNumber);
        else
            text += "Disconnected";

        Colour textColour = isConnected() ? Colours::green : Colours::red;

        connectionStatusLabel->setText (text, dontSendNotification);
        connectionStatusLabel->setFont (Font (15.00f, Font::bold));
        connectionStatusLabel->setColour (Label::textColourId, textColour);
        connectionStatusLabel->setJustificationType (Justification::centredRight);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
