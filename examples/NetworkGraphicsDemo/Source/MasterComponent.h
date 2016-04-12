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
    Runs the master node, calls the demo to update the canvas, broadcasts those changes
    out to slaves, and shows a view of all the clients to allow them to be dragged around.
*/
struct MasterContentComponent  : public Component,
                                 private Timer,
                                 private OSCSender,
                                 private OSCReceiver,
                                 private OSCReceiver::Listener<OSCReceiver::MessageLoopCallback>
{
    MasterContentComponent (PropertiesFile& props)
        : properties (props)
    {
        setWantsKeyboardFocus (true);
        createAllDemos (demos);
        setContent (0);

        setSize (15.0f * currentCanvas.getLimits().getWidth(),
                 15.0f * currentCanvas.getLimits().getHeight());

        if (! OSCSender::connect (getBroadcastIPAddress(), masterPortNumber))
            error = "Master app OSC sender: network connection error.";

        if (! OSCReceiver::connect (clientPortNumber))
            error = "Master app OSC receiver: network connection error.";

        OSCReceiver::addListener (this);

        startTimerHz (30);
    }

    ~MasterContentComponent()
    {
        OSCReceiver::removeListener (this);
    }

    //==============================================================================
    struct Client
    {
        String name, ipAddress;
        float widthInches, heightInches;
        Point<float> centre; // in inches
        float scaleFactor;
    };

    Array<Client> clients;

    void addClient (String name, String ipAddress, String areaDescription)
    {
        auto area = Rectangle<float>::fromString (areaDescription);

        if (auto c = getClient (name))
        {
            c->ipAddress = ipAddress;
            c->widthInches = area.getWidth();
            c->heightInches = area.getHeight();
            return;
        }

        DBG (name + "   "  + ipAddress);

        removeClient (name);
        clients.add ({ name, ipAddress, area.getWidth(), area.getHeight(), {}, 1.0f });

        String lastX = properties.getValue ("lastX_" + name);
        String lastY = properties.getValue ("lastY_" + name);
        String lastScale = properties.getValue ("scale_" + name);

        if (lastX.isEmpty() || lastY.isEmpty())
            setClientCentre (name, { Random().nextFloat() * 10.0f,
                                     Random().nextFloat() * 10.0f });
        else
            setClientCentre (name, Point<float> (lastX.getFloatValue(),
                                                 lastY.getFloatValue()));

        if (lastScale.isNotEmpty())
            setClientScale (name, lastScale.getFloatValue());
        else
            setClientScale (name, 1.0f);

        updateDeviceComponents();
    }

    void removeClient (String name)
    {
        for (int i = clients.size(); --i >= 0;)
            if (clients.getReference (0).name == name)
                clients.remove (i);

        updateDeviceComponents();
    }

    void setClientCentre (const String& name, Point<float> newCentre)
    {
        if (auto c = getClient (name))
        {
            newCentre = currentCanvas.getLimits().getConstrainedPoint (newCentre);
            c->centre = newCentre;

            properties.setValue ("lastX_" + name, String (newCentre.x));
            properties.setValue ("lastY_" + name, String (newCentre.y));

            startTimer (1);
        }
    }

    float getClientScale (const String& name) const
    {
        if (auto c = getClient (name))
            return c->scaleFactor;

        return 1.0f;
    }

    void setClientScale (const String& name, float newScale)
    {
        if (auto c = getClient (name))
        {
            c->scaleFactor = jlimit (0.5f, 2.0f, newScale);
            properties.setValue ("scale_" + name, String (newScale));
        }
    }

    Point<float> getClientCentre (const String& name) const
    {
        if (auto c = getClient (name))
            return c->centre;

        return {};
    }

    Rectangle<float> getClientArea (const String& name) const
    {
        if (auto c = getClient (name))
            return Rectangle<float> (c->widthInches, c->heightInches)
                     .withCentre (c->centre);

        return {};
    }

    Rectangle<float> getActiveCanvasArea() const
    {
        Rectangle<float> r;

        if (clients.size() > 0)
            r = Rectangle<float> (1.0f, 1.0f).withCentre (clients.getReference (0).centre);

        for (int i = 1; i < clients.size(); ++i)
            r = r.getUnion (Rectangle<float> (1.0f, 1.0f).withCentre (clients.getReference (i).centre));

        return r.expanded (6.0f);
    }

    int getContentIndex() const
    {
        return demos.indexOf (content);
    }

    void setContent (int demoIndex)
    {
        content = demos[demoIndex];

        if (content != nullptr)
            content->reset();
    }

    bool keyPressed (const KeyPress& key) override
    {
        if (key == KeyPress::spaceKey || key == KeyPress::rightKey || key == KeyPress::downKey)
        {
            setContent ((getContentIndex() + 1) % demos.size());
            return true;
        }

        if (key == KeyPress::upKey || key == KeyPress::leftKey)
        {
            setContent ((getContentIndex() + demos.size() - 1) % demos.size());
            return true;
        }

        return Component::keyPressed (key);
    }

private:
    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        currentCanvas.draw (g, getLocalBounds().toFloat(), currentCanvas.getLimits());

        if (error.isNotEmpty())
        {
            g.setColour (Colours::red);
            g.setFont (20.0f);
            g.drawText (error, getLocalBounds().reduced (10).removeFromBottom (80),
                        Justification::centredRight, true);
        }

        if (content != nullptr)
        {
            g.setColour (Colours::white);
            g.setFont (17.0f);
            g.drawText ("Demo: " + content->getName(),
                        getLocalBounds().reduced (10).removeFromTop (30),
                        Justification::centredLeft, true);
        }
    }

    void resized() override
    {
        updateDeviceComponents();
    }

    void updateDeviceComponents()
    {
        for (int i = devices.size(); --i >= 0;)
            if (getClient (devices.getUnchecked(i)->getName()) == nullptr)
                devices.remove (i);

        for (const auto& c : clients)
            if (getDeviceComponent (c.name) == nullptr)
                addAndMakeVisible (devices.add (new DeviceComponent (*this, c.name)));

        for (auto d : devices)
            d->setBounds (virtualSpaceToLocal (getClientArea (d->getName())).getSmallestIntegerContainer());
    }

    Point<float> virtualSpaceToLocal (Point<float> p) const
    {
        auto total = currentCanvas.getLimits();

        return { getWidth()  * (p.x - total.getX()) / total.getWidth(),
                 getHeight() * (p.y - total.getY()) / total.getHeight() };
    }

    Rectangle<float> virtualSpaceToLocal (Rectangle<float> p) const
    {
        return { virtualSpaceToLocal (p.getTopLeft()),
                 virtualSpaceToLocal (p.getBottomRight()) };
    }

    Point<float> localSpaceToVirtual (Point<float> p) const
    {
        auto total = currentCanvas.getLimits();

        return { total.getX() + total.getWidth() * (p.x / getWidth()),
                 total.getY() + total.getHeight() * (p.y / getHeight()) };
    }

    //==============================================================================
    struct DeviceComponent  : public Component
    {
        DeviceComponent (MasterContentComponent& e, String name)
            : Component (name), editor (e)
        {
            setMouseCursor (MouseCursor::DraggingHandCursor);
        }

        void paint (Graphics& g) override
        {
            g.fillAll (Colours::blue.withAlpha (0.4f));

            g.setColour (Colours::white);
            g.setFont (11.0f);
            g.drawFittedText (getName(), getLocalBounds(), Justification::centred, 2);
        }

        void mouseDown (const MouseEvent&) override
        {
            dragStartLocation = editor.getClientCentre (getName());
        }

        void mouseDrag (const MouseEvent& e) override
        {
            editor.setClientCentre (getName(), dragStartLocation
                                                + editor.localSpaceToVirtual (e.getPosition().toFloat())
                                                - editor.localSpaceToVirtual (e.getMouseDownPosition().toFloat()));
        }

        void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& e) override
        {
            editor.setClientScale (getName(), editor.getClientScale (getName()) + 0.1f * e.deltaY);
        }

        void mouseDoubleClick (const MouseEvent&) override
        {
            editor.setClientScale (getName(), 1.0f);
        }

        MasterContentComponent& editor;
        Point<float> dragStartLocation;
        Rectangle<float> clientArea;
    };

    DeviceComponent* getDeviceComponent (const String& name) const
    {
        for (auto d : devices)
            if (d->getName() == name)
                return d;

        return nullptr;
    }

    //==============================================================================
    void broadcastNewCanvasState (const MemoryBlock& canvasData)
    {
        BlockPacketiser packetiser;
        packetiser.createBlocksFromData (canvasData, 1000);

        for (const auto& client : clients)
            for (auto& b : packetiser.blocks)
                sendToIPAddress (client.ipAddress, masterPortNumber, canvasStateOSCAddress, b);
    }

    void timerCallback() override
    {
        startTimerHz (30);

        currentCanvas.reset();
        updateCanvasInfo (currentCanvas);

        {
            ScopedPointer<CanvasGeneratingContext> context (new CanvasGeneratingContext (currentCanvas));
            Graphics g (*context);

            if (content != nullptr)
                content->generateCanvas (g, currentCanvas, getActiveCanvasArea());
        }

        broadcastNewCanvasState (currentCanvas.toMemoryBlock());

        updateDeviceComponents();
        repaint();
    }

    void updateCanvasInfo (SharedCanvasDescription& canvas)
    {
        canvas.backgroundColour = Colours::black;

        for (const auto& c : clients)
            canvas.clients.add ({ c.name, c.centre, c.scaleFactor });
    }

    Client* getClient (const String& name) const
    {
        for (auto& c : clients)
            if (c.name == name)
                return &c;

        return nullptr;
    }

    //==============================================================================
    void oscMessageReceived (const OSCMessage& message) override
    {
        auto address = message.getAddressPattern();

        if (address.matches (newClientOSCAddress))       newClientOSCMessageReceived (message);
        else if (address.matches (userInputOSCAddress))  userInputOSCMessageReceived (message);
    }

    void newClientOSCMessageReceived (const OSCMessage& message)
    {
        if (message.isEmpty() || ! message[0].isString())
            return;

        StringArray tokens = StringArray::fromTokens (message[0].getString(), ":", "");
        addClient (tokens[0], tokens[1], tokens[2]);
    }

    void userInputOSCMessageReceived (const OSCMessage& message)
    {
        if (message.size() == 3 && message[0].isString() && message[1].isFloat32() && message[2].isFloat32())
        {
            content->handleTouch ({ message[1].getFloat32(),
                                    message[2].getFloat32() });
        }
    }

    //==============================================================================
    AnimatedContent* content = nullptr;
    PropertiesFile& properties;
    OwnedArray<DeviceComponent> devices;
    SharedCanvasDescription currentCanvas;
    String error;

    OwnedArray<AnimatedContent> demos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterContentComponent)
};
