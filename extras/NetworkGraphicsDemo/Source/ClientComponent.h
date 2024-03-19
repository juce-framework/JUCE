/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/


/**
    This component runs in a client process, draws the part of the canvas that this
    particular client covers, and updates itself when messages arrive from the master
    containing new canvas states.
*/
class ClientCanvasComponent final : public Component,
                                    private OSCSender,
                                    private OSCReceiver,
                                    private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                                    private AsyncUpdater,
                                    private Timer
{
public:
    ClientCanvasComponent (PropertiesFile& p, int windowIndex)  : properties (p)
    {
        {
            String uuidPropName ("UUID" + String (windowIndex));
            clientName = properties.getValue (uuidPropName);

            if (clientName.isEmpty())
            {
                clientName = "CLIENT_" + String (Random().nextInt (10000)).toUpperCase();
                properties.setValue (uuidPropName, clientName);
            }
        }

        setOpaque (true);
        setSize (1500, 900);

        if (! OSCSender::connect (getBroadcastIPAddress(), clientPortNumber))
            error = "Client app OSC sender: network connection error.";

        if (! OSCReceiver::connect (masterPortNumber))
            error = "Client app OSC receiver: network connection error.";

        OSCReceiver::addListener (this);

        handleTimerCallback();
        startTimer (2000);
    }

    ~ClientCanvasComponent() override
    {
        OSCReceiver::removeListener (this);
    }

private:
    void mouseDrag (const MouseEvent& e) override
    {
        auto clientArea = getAreaInGlobalSpace();

        if (! clientArea.isEmpty())
        {
            OSCMessage message (userInputOSCAddress);

            message.addString (clientName);
            message.addFloat32 (e.position.x * clientArea.getWidth()  / (float) getWidth()  + clientArea.getX());
            message.addFloat32 (e.position.y * clientArea.getHeight() / (float) getHeight() + clientArea.getY());

            send (message);
        }
    }

    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        auto address = message.getAddressPattern();

        if (address.matches (canvasStateOSCAddress))
            canvasStateOSCMessageReceived (message);
    }

    struct NewStateMessage final : public Message
    {
        NewStateMessage (const MemoryBlock& d) : data (d) {}
        MemoryBlock data;
    };

    void canvasStateOSCMessageReceived (const OSCMessage& message)
    {
        if (message.isEmpty() || ! message[0].isBlob())
            return;

        if (packetiser.appendIncomingBlock (message[0].getBlob()))
        {
            const ScopedLock sl (canvasLock);

            MemoryBlock newCanvasData;

            if (packetiser.reassemble (newCanvasData))
            {
                MemoryInputStream i (newCanvasData.getData(), newCanvasData.getSize(), false);
                canvas2.load (i);
                triggerAsyncUpdate();
            }
        }
    }

    //==============================================================================
    String getMachineInfoToDisplay() const
    {
        auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (getScreenBounds().getCentre());
        return getOSName() + "   " + String (display->dpi) + "   "  + String (display->scale);
    }

    static String getOSName()
    {
       #if JUCE_MAC
        return "Mac OSX";
       #elif JUCE_ANDROID
        return "Android";
       #elif JUCE_IOS
        return "iOS";
       #elif JUCE_WINDOWS
        return "Windows";
       #elif JUCE_LINUX
        return "Linux";
       #elif JUCE_BSD
        return "BSD";
       #endif
    }

    void paint (Graphics& g) override
    {
        g.fillAll (canvas.backgroundColour);

        auto clientArea = getAreaInGlobalSpace();

        if (clientArea.isEmpty())
        {
            g.setColour (Colours::red.withAlpha (0.5f));
            g.setFont (FontOptions (20.0f));
            g.drawText ("Not Connected", getLocalBounds(), Justification::centred, false);
            return;
        }

        canvas.draw (g, getLocalBounds().toFloat(), clientArea);

        g.setFont (FontOptions (34.0f));
        g.setColour (Colours::white.withAlpha (0.6f));

        g.drawText (getMachineInfoToDisplay(),
                    getLocalBounds().reduced (10).removeFromBottom (20),
                    Justification::centredRight, true);

        if (error.isNotEmpty())
        {
            g.setColour (Colours::red);
            g.drawText (error, getLocalBounds().reduced (10).removeFromBottom (80),
                        Justification::centredRight, true);
        }
    }

    Rectangle<float> getAreaInGlobalSpace() const
    {
        if (auto client = canvas.findClient (clientName))
        {
            auto screenBounds = getScreenBounds();
            auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (screenBounds.getCentre());
            return ((screenBounds - display->userArea.getCentre()).toFloat() / (client->scaleFactor * display->dpi / display->scale)) + client->centre;
        }

        return {};
    }

    Rectangle<float> getScreenAreaInGlobalSpace() const
    {
        if (auto client = canvas.findClient (clientName))
        {
            auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (getScreenBounds().getCentre());
            return (display->userArea.toFloat() / (client->scaleFactor * display->dpi / display->scale)).withCentre (client->centre);
        }

        return {};
    }

    void handleTimerCallback()
    {
        send (newClientOSCAddress, clientName + ":" + IPAddress::getLocalAddress().toString()
                                              + ":" + getScreenAreaInGlobalSpace().toString());
    }


    void timerCallback() override
    {
        handleTimerCallback();
    }

    void handleAsyncUpdate() override
    {
        const ScopedLock sl (canvasLock);
        canvas.swapWith (canvas2);
        repaint();
    }

    SharedCanvasDescription canvas, canvas2;
    PropertiesFile& properties;
    String clientName, error;

    CriticalSection canvasLock;
    BlockPacketiser packetiser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ClientCanvasComponent)
};
