/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             OSCDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Application using the OSC protocol.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_osc
 exporters:        xcode_mac, vs2022, linux_make

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OSCDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class OSCLogListBox    : public ListBox,
                         private ListBoxModel,
                         private AsyncUpdater
{
public:
    OSCLogListBox()
    {
        setModel (this);
    }

    ~OSCLogListBox() override = default;

    //==============================================================================
    int getNumRows() override
    {
        return oscLogList.size();
    }

    //==============================================================================
    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (rowIsSelected);

        if (isPositiveAndBelow (row, oscLogList.size()))
        {
            g.setColour (Colours::white);

            g.drawText (oscLogList[row],
                        Rectangle<int> (width, height).reduced (4, 0),
                        Justification::centredLeft, true);
        }
    }

    //==============================================================================
    void addOSCMessage (const OSCMessage& message, int level = 0)
    {
        oscLogList.add (getIndentationString (level)
                        + "- osc message, address = '"
                        + message.getAddressPattern().toString()
                        + "', "
                        + String (message.size())
                        + " argument(s)");

        if (! message.isEmpty())
        {
            for (auto& arg : message)
                addOSCMessageArgument (arg, level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    void addOSCBundle (const OSCBundle& bundle, int level = 0)
    {
        OSCTimeTag timeTag = bundle.getTimeTag();

        oscLogList.add (getIndentationString (level)
                        + "- osc bundle, time tag = "
                        + timeTag.toTime().toString (true, true, true, true));

        for (auto& element : bundle)
        {
            if (element.isMessage())
                addOSCMessage (element.getMessage(), level + 1);
            else if (element.isBundle())
                addOSCBundle (element.getBundle(), level + 1);
        }

        triggerAsyncUpdate();
    }

    //==============================================================================
    void addOSCMessageArgument (const OSCArgument& arg, int level)
    {
        String typeAsString;
        String valueAsString;

        if (arg.isFloat32())
        {
            typeAsString = "float32";
            valueAsString = String (arg.getFloat32());
        }
        else if (arg.isInt32())
        {
            typeAsString = "int32";
            valueAsString = String (arg.getInt32());
        }
        else if (arg.isString())
        {
            typeAsString = "string";
            valueAsString = arg.getString();
        }
        else if (arg.isBlob())
        {
            typeAsString = "blob";
            auto& blob = arg.getBlob();
            valueAsString = String::fromUTF8 ((const char*) blob.getData(), (int) blob.getSize());
        }
        else
        {
            typeAsString = "(unknown)";
        }

        oscLogList.add (getIndentationString (level + 1) + "- " + typeAsString.paddedRight(' ', 12) + valueAsString);
    }

    //==============================================================================
    void addInvalidOSCPacket (const char* /* data */, int dataSize)
    {
        oscLogList.add ("- (" + String(dataSize) + "bytes with invalid format)");
    }

    //==============================================================================
    void clear()
    {
        oscLogList.clear();
        triggerAsyncUpdate();
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        updateContent();
        scrollToEnsureRowIsOnscreen (oscLogList.size() - 1);
        repaint();
    }

private:
    static String getIndentationString (int level)
    {
        return String().paddedRight (' ', 2 * level);
    }

    //==============================================================================
    StringArray oscLogList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCLogListBox)
};

//==============================================================================
class OSCSenderDemo   : public Component
{
public:
    OSCSenderDemo()
    {
        addAndMakeVisible (senderLabel);
        senderLabel.attachToComponent (&rotaryKnob, false);

        rotaryKnob.setRange (0.0, 1.0);
        rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
        rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
        rotaryKnob.setBounds (50, 50, 180, 180);
        addAndMakeVisible (rotaryKnob);
        rotaryKnob.onValueChange = [this]
        {
            // create and send an OSC message with an address and a float value:
            if (! sender1.send ("/juce/rotaryknob", (float) rotaryKnob.getValue()))
                showConnectionErrorMessage ("Error: could not send OSC message.");
            if (! sender2.send ("/juce/rotaryknob", (float) rotaryKnob.getValue()))
                showConnectionErrorMessage ("Error: could not send OSC message.");
        };

        // specify here where to send OSC messages to: host URL and UDP port number
        if (! sender1.connect ("127.0.0.1", 9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");
        if (! sender2.connect ("127.0.0.1", 9002))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9002.");
    }

private:
    //==============================================================================
    void showConnectionErrorMessage (const String& messageText)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Connection error",
                                                         messageText);
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    Slider rotaryKnob;
    OSCSender sender1, sender2;
    Label senderLabel { {}, "Sender" };
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCSenderDemo)
};

//==============================================================================
class OSCReceiverDemo   : public Component,
                          private OSCReceiver,
                          private OSCReceiver::ListenerWithOSCAddress<OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    OSCReceiverDemo()
    {
        addAndMakeVisible (receiverLabel);
        receiverLabel.attachToComponent (&rotaryKnob, false);

        rotaryKnob.setRange (0.0, 1.0);
        rotaryKnob.setSliderStyle (Slider::RotaryVerticalDrag);
        rotaryKnob.setTextBoxStyle (Slider::TextBoxBelow, true, 150, 25);
        rotaryKnob.setBounds (50, 50, 180, 180);
        rotaryKnob.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (rotaryKnob);

        // specify here on which UDP port number to receive incoming OSC messages
        if (! connect (9001))
            showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");

        // tell the component to listen for OSC messages matching this address:
        addListener (this, "/juce/rotaryknob");
    }

private:
    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        if (message.size() == 1 && message[0].isFloat32())
            rotaryKnob.setValue (jlimit (0.0f, 10.0f, message[0].getFloat32()));
    }

    void showConnectionErrorMessage (const String& messageText)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Connection error",
                                                         messageText);
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    Slider rotaryKnob;
    Label receiverLabel { {}, "Receiver" };
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCReceiverDemo)
};

//==============================================================================
class OSCMonitorDemo   : public Component,
                         private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    OSCMonitorDemo()
    {
        portNumberLabel.setBounds (10, 18, 130, 25);
        addAndMakeVisible (portNumberLabel);

        portNumberField.setEditable (true, true, true);
        portNumberField.setBounds (140, 18, 50, 25);
        addAndMakeVisible (portNumberField);

        connectButton.setBounds (210, 18, 100, 25);
        addAndMakeVisible (connectButton);
        connectButton.onClick = [this] { connectButtonClicked(); };

        clearButton.setBounds (320, 18, 60, 25);
        addAndMakeVisible (clearButton);
        clearButton.onClick = [this] { clearButtonClicked(); };

        connectionStatusLabel.setBounds (450, 18, 240, 25);
        updateConnectionStatusLabel();
        addAndMakeVisible (connectionStatusLabel);

        oscLogListBox.setBounds (0, 60, 700, 340);
        addAndMakeVisible (oscLogListBox);

        oscReceiver.addListener (this);
        oscReceiver.registerFormatErrorHandler ([this] (const char* data, int dataSize)
                                                {
                                                    oscLogListBox.addInvalidOSCPacket (data, dataSize);
                                                });
    }

private:
    //==============================================================================
    Label portNumberLabel    { {}, "UDP Port Number: " };
    Label portNumberField    { {}, "9002" };
    TextButton connectButton { "Connect" };
    TextButton clearButton   { "Clear" };
    Label connectionStatusLabel;

    OSCLogListBox oscLogListBox;
    OSCReceiver oscReceiver;

    int currentPortNumber = -1;

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
        oscLogListBox.clear();
    }

    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        oscLogListBox.addOSCMessage (message);
    }

    void oscBundleReceived (const OSCBundle& bundle) override
    {
        oscLogListBox.addOSCBundle (bundle);
    }

    //==============================================================================
    void connect()
    {
        auto portToConnect = portNumberField.getText().getIntValue();

        if (! isValidOscPort (portToConnect))
        {
            handleInvalidPortNumberEntered();
            return;
        }

        if (oscReceiver.connect (portToConnect))
        {
            currentPortNumber = portToConnect;
            connectButton.setButtonText ("Disconnect");
        }
        else
        {
            handleConnectError (portToConnect);
        }
    }

    //==============================================================================
    void disconnect()
    {
        if (oscReceiver.disconnect())
        {
            currentPortNumber = -1;
            connectButton.setButtonText ("Connect");
        }
        else
        {
            handleDisconnectError();
        }
    }

    //==============================================================================
    void handleConnectError (int failedPort)
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "OSC Connection error",
                                                         "Error: could not connect to port " + String (failedPort));
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    void handleDisconnectError()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Unknown error",
                                                         "An unknown error occurred while trying to disconnect from UDP port.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    void handleInvalidPortNumberEntered()
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "Invalid port number",
                                                         "Error: you have entered an invalid UDP port number.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
    }

    //==============================================================================
    bool isConnected() const
    {
        return currentPortNumber != -1;
    }

    //==============================================================================
    bool isValidOscPort (int port) const
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

        auto textColour = isConnected() ? Colours::green : Colours::red;

        connectionStatusLabel.setText (text, dontSendNotification);
        connectionStatusLabel.setFont (Font (15.00f, Font::bold));
        connectionStatusLabel.setColour (Label::textColourId, textColour);
        connectionStatusLabel.setJustificationType (Justification::centredRight);
    }

    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCMonitorDemo)
};

//==============================================================================
class OSCDemo   : public Component
{
public:
    OSCDemo()
    {
        addAndMakeVisible (monitor);
        addAndMakeVisible (receiver);
        addAndMakeVisible (sender);

        setSize (700, 400);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto lowerBounds = bounds.removeFromBottom (getHeight() / 2);
        auto halfBounds  = bounds.removeFromRight  (getWidth()  / 2);

        sender  .setBounds (bounds);
        receiver.setBounds (halfBounds);
        monitor .setBounds (lowerBounds.removeFromTop (getHeight() / 2));
    }

private:
    OSCMonitorDemo  monitor;
    OSCReceiverDemo receiver;
    OSCSenderDemo   sender;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OSCDemo)
};
