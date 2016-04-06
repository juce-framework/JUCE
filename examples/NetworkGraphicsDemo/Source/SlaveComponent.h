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


/**
    This component runs in a slave process, draws the part of the canvas that this
    particular client covers, and updates itself when messages arrive from the master
    containing new canvas states.
*/
class SlaveCanvasComponent  : public Component,
                              private OSCSender,
                              private OSCReceiver,
                              private OSCReceiver::Listener<OSCReceiver::RealtimeCallback>,
                              private AsyncUpdater,
                              private Timer
{
public:
    SlaveCanvasComponent (PropertiesFile& p, int windowIndex)  : properties (p)
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

        timerCallback();
        startTimer (2000);
    }

    ~SlaveCanvasComponent()
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
            message.addFloat32 (e.position.x * clientArea.getWidth()  / getWidth()  + clientArea.getX());
            message.addFloat32 (e.position.y * clientArea.getHeight() / getHeight() + clientArea.getY());

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

    struct NewStateMessage  : public Message
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
        //auto display = Desktop::getInstance().getDisplays().getDisplayContaining (getScreenBounds().getCentre());
        return getOSName();//     + "   " + String (display.dpi) + "   "  + String (display.scale);
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
       #endif
    }

    void paint (Graphics& g) override
    {
        g.fillAll (canvas.backgroundColour);

        auto clientArea = getAreaInGlobalSpace();

        if (clientArea.isEmpty())
        {
            g.setColour (Colours::red.withAlpha (0.5f));
            g.setFont (20.0f);
            g.drawText ("Not Connected", getLocalBounds(), Justification::centred, false);
            return;
        }

        canvas.draw (g, getLocalBounds().toFloat(), clientArea);

        g.setFont (Font (34.0f));
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
            auto display = Desktop::getInstance().getDisplays().getDisplayContaining (screenBounds.getCentre());
            return ((screenBounds - display.userArea.getCentre()).toFloat() / (client->scaleFactor * display.dpi / display.scale)) + client->centre;
        }

        return {};
    }

    Rectangle<float> getScreenAreaInGlobalSpace() const
    {
        if (auto client = canvas.findClient (clientName))
        {
            auto display = Desktop::getInstance().getDisplays().getDisplayContaining (getScreenBounds().getCentre());
            return (display.userArea.toFloat() / (client->scaleFactor * display.dpi / display.scale)).withCentre (client->centre);
        }

        return {};
    }

    void timerCallback() override
    {
        send (newClientOSCAddress, clientName + ":" + getIPAddress()
                                              + ":" + getScreenAreaInGlobalSpace().toString());
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SlaveCanvasComponent)
};
