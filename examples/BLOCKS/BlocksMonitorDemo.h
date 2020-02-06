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

 name:             BlocksMonitorDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Application to monitor Blocks devices.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_blocks_basics,
                   juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        BlocksMonitorDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    Base class that renders a Block on the screen
*/
class BlockComponent : public Component,
                       public SettableTooltipClient,
                       private TouchSurface::Listener,
                       private ControlButton::Listener,
                       private Timer
{
public:
    BlockComponent (Block::Ptr blockToUse)
        : block (blockToUse)
    {
        updateStatsAndTooltip();

        // Register BlockComponent as a listener to the touch surface
        if (auto touchSurface = block->getTouchSurface())
            touchSurface->addListener (this);

        // Register BlockComponent as a listener to any buttons
        for (auto button : block->getButtons())
            button->addListener (this);

        // If this is a Lightpad then set the grid program to be blank
        if (block->getLEDGrid() != nullptr)
            block->setProgram (new BitmapLEDProgram (*block));

        // If this is a Lightpad then redraw it at 25Hz
        if (block->getType() == Block::lightPadBlock)
            startTimerHz (25);

        // Make sure the component can't go offscreen if it is draggable
        constrainer.setMinimumOnscreenAmounts (50, 50, 50, 50);
    }

    ~BlockComponent() override
    {
        // Remove any listeners
        if (auto touchSurface = block->getTouchSurface())
            touchSurface->removeListener (this);

        for (auto button : block->getButtons())
            button->removeListener (this);
    }

    /** Called periodically to update the tooltip with information about the Block */
    void updateStatsAndTooltip()
    {
        // Get the battery level of this Block and inform any subclasses
        auto batteryLevel = block->getBatteryLevel();
        handleBatteryLevelUpdate (batteryLevel);

        // Update the tooltip
        setTooltip ("Name = "          + block->getDeviceDescription() + "\n"
                  + "UID = "           + String (block->uid) + "\n"
                  + "Serial number = " + block->serialNumber + "\n"
                  + "Battery level = " + String ((int) (batteryLevel * 100)) + "%"
                  + (block->isBatteryCharging() ? "++"
                                                : "--"));
    }

    /** Subclasses should override this to paint the Block object on the screen */
    void paint (Graphics&) override = 0;

    /** Subclasses can override this to receive button down events from the Block */
    virtual void handleButtonPressed  (ControlButton::ButtonFunction, uint32) {}

    /** Subclasses can override this to receive button up events from the Block */
    virtual void handleButtonReleased (ControlButton::ButtonFunction, uint32) {}

    /** Subclasses can override this to receive touch events from the Block */
    virtual void handleTouchChange (TouchSurface::Touch) {}

    /** Subclasses can override this to battery level updates from the Block */
    virtual void handleBatteryLevelUpdate (float) {}

    /** The Block object that this class represents */
    Block::Ptr block;

    //==============================================================================
    /** Returns an integer index corresponding to a physical position on the hardware
        for each type of Control Block. */
    static int controlButtonFunctionToIndex (ControlButton::ButtonFunction f)
    {
        using CB = ControlButton;

        static Array<ControlButton::ButtonFunction> map[] =
        {
            { CB::mode,     CB::button0,  CB::velocitySensitivity },
            { CB::volume,   CB::button1,  CB::glideSensitivity    },
            { CB::scale,    CB::button2,  CB::slideSensitivity,  CB::click       },
            { CB::chord,    CB::button3,  CB::pressSensitivity,  CB::snap        },
            { CB::arp,      CB::button4,  CB::liftSensitivity,   CB::back        },
            { CB::sustain,  CB::button5,  CB::fixedVelocity,     CB::playOrPause },
            { CB::octave,   CB::button6,  CB::glideLock,         CB::record      },
            { CB::love,     CB::button7,  CB::pianoMode,         CB::learn       },
            { CB::up   },
            { CB::down }
        };

        for (int i = 0; i < numElementsInArray (map); ++i)
            if (map[i].contains (f))
                return i;

        return -1;
    }

    Point<float> getOffsetForPort (Block::ConnectionPort port)
    {
        using e = Block::ConnectionPort::DeviceEdge;

        switch (rotation)
        {
            case 0:
            {
                switch (port.edge)
                {
                    case e::north:
                        return { static_cast<float> (port.index), 0.0f };
                    case e::east:
                        return { static_cast<float> (block->getWidth()), static_cast<float> (port.index) };
                    case e::south:
                        return { static_cast<float> (port.index), static_cast<float> (block->getHeight()) };
                    case e::west:
                        return { 0.0f, static_cast<float> (port.index) };
                }
            }
            case 90:
            {
                switch (port.edge)
                {
                    case e::north:
                        return { 0.0f, static_cast<float> (port.index) };
                    case e::east:
                        return { static_cast<float> (-1.0f - port.index), static_cast<float> (block->getWidth()) };
                    case e::south:
                        return { static_cast<float> (0.0f - block->getHeight()), static_cast<float> (port.index) };
                    case e::west:
                        return { static_cast<float> (-1.0f - port.index), 0.0f };
                }
            }
            case 180:
            {
                switch (port.edge)
                {
                    case e::north:
                        return { static_cast<float> (-1.0f - port.index), 0.0f };
                    case e::east:
                        return { static_cast<float> (0.0f - block->getWidth()), static_cast<float> (-1.0f - port.index) };
                    case e::south:
                        return { static_cast<float> (-1.0f - port.index), static_cast<float> (0.0f - block->getHeight()) };
                    case e::west:
                        return { 0.0f, static_cast<float> (-1.0f - port.index) };
                }
            }
            case 270:
            {
                switch (port.edge)
                {
                    case e::north:
                        return { 0.0f, static_cast<float> (-1.0f - port.index) };
                    case e::east:
                        return { static_cast<float> (port.index), static_cast<float> (0 - block->getWidth()) };
                    case e::south:
                        return { static_cast<float> (block->getHeight()), static_cast<float> (-1.0f - port.index) };
                    case e::west:
                        return { static_cast<float> (port.index), 0.0f };
                }
            }
        }

        return {};
    }

    int rotation = 0;
    Point<float> topLeft = { 0.0f, 0.0f };

private:
    /** Used to call repaint() periodically */
    void timerCallback() override   { repaint(); }

    /** Overridden from TouchSurface::Listener */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& t) override { handleTouchChange (t); }

    /** Overridden from ControlButton::Listener */
    void buttonPressed  (ControlButton& b, Block::Timestamp t) override      { handleButtonPressed  (b.getType(), t); }

    /** Overridden from ControlButton::Listener */
    void buttonReleased (ControlButton& b, Block::Timestamp t) override      { handleButtonReleased (b.getType(), t); }

    /** Overridden from MouseListener. Prepares the master Block component for dragging. */
    void mouseDown (const MouseEvent& e) override
    {
        if (block->isMasterBlock())
            componentDragger.startDraggingComponent (this, e);
    }

    /** Overridden from MouseListener. Drags the master Block component */
    void mouseDrag (const MouseEvent& e) override
    {
        if (block->isMasterBlock())
        {
            componentDragger.dragComponent (this, e, &constrainer);
            getParentComponent()->resized();
        }
    }

    ComponentDragger componentDragger;
    ComponentBoundsConstrainer constrainer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockComponent)
};

//==============================================================================
/**
    Class that renders a Lightpad on the screen
*/
class LightpadComponent   : public BlockComponent
{
public:
    LightpadComponent (Block::Ptr blockToUse)
        : BlockComponent (blockToUse)
    {}

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();

        // clip the drawing area to only draw in the block area
        {
            Path clipArea;
            clipArea.addRoundedRectangle (r, r.getWidth() / 20.0f);

            g.reduceClipRegion (clipArea);
        }

        // Fill a black square for the Lightpad
        g.fillAll (Colours::black);

        // size ration between physical and on-screen blocks
        Point<float> ratio (r.getWidth()  / block->getWidth(),
                            r.getHeight() / block->getHeight());

        auto maxCircleSize = block->getWidth() / 3.0f;

        // iterate over the list of current touches and draw them on the onscreen Block
        for (auto touch : touches)
        {
            auto circleSize = touch.touch.z * maxCircleSize;

            Point<float> touchPosition (touch.touch.x,
                                        touch.touch.y);

            auto blob = Rectangle<float> (circleSize, circleSize)
                           .withCentre (touchPosition) * ratio;

            ColourGradient cg (colourArray[touch.touch.index],  blob.getCentreX(), blob.getCentreY(),
                               Colours::transparentBlack,       blob.getRight(),   blob.getBottom(),
                               true);

            g.setGradientFill (cg);
            g.fillEllipse (blob);
        }
    }

    void handleTouchChange (TouchSurface::Touch touch) override { touches.updateTouch (touch); }

private:
    /** An Array of colours to use for touches */
    Array<Colour> colourArray = { Colours::red,
                                  Colours::blue,
                                  Colours::green,
                                  Colours::yellow,
                                  Colours::white,
                                  Colours::hotpink,
                                  Colours::mediumpurple };

    /** A list of current Touch events */
    TouchList<TouchSurface::Touch> touches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightpadComponent)
};


//==============================================================================
/**
    Class that renders a Control Block on the screen
*/
class ControlBlockComponent   : public BlockComponent
{
public:
    ControlBlockComponent (Block::Ptr blockToUse)
        : BlockComponent (blockToUse),
          numLeds (block->getLEDRow()->getNumLEDs())
    {
        addAndMakeVisible (roundedRectangleButton);

        // Display the battery level on the LEDRow
        auto numLedsToTurnOn = static_cast<int> (numLeds * block->getBatteryLevel());

        // add LEDs
        for (auto i = 0; i < numLeds; ++i)
        {
            auto ledComponent = new LEDComponent();
            ledComponent->setOnState (i < numLedsToTurnOn);

            addAndMakeVisible (leds.add (ledComponent));
        }

        previousNumLedsOn = numLedsToTurnOn;

        // add buttons
        for (auto i = 0; i < 8; ++i)
            addAndMakeVisible (circleButtons[i]);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        auto rowHeight   = r.getHeight() / 5;
        auto ledWidth    = (r.getWidth() - 70) / numLeds;
        auto buttonWidth = (r.getWidth() - 40) / 5;

        auto row = r;

        auto ledRow     = row.removeFromTop (rowHeight)    .withSizeKeepingCentre (r.getWidth(), ledWidth);
        auto buttonRow1 = row.removeFromTop (rowHeight * 2).withSizeKeepingCentre (r.getWidth(), buttonWidth);
        auto buttonRow2 = row.removeFromTop (rowHeight * 2).withSizeKeepingCentre (r.getWidth(), buttonWidth);

        for (auto* led : leds)
        {
            led->setBounds (ledRow.removeFromLeft (ledWidth).reduced (2));
            ledRow.removeFromLeft (5);
        }

        for (auto i = 0; i < 5; ++i)
        {
            circleButtons[i].setBounds (buttonRow1.removeFromLeft (buttonWidth).reduced (2));
            buttonRow1.removeFromLeft (10);
        }

        for (auto i = 5; i < 8; ++i)
        {
            circleButtons[i].setBounds (buttonRow2.removeFromLeft (buttonWidth).reduced (2));
            buttonRow2.removeFromLeft (10);
        }

        roundedRectangleButton.setBounds (buttonRow2);
    }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();

        // Fill a black rectangle for the Control Block
        g.setColour (Colours::black);
        g.fillRoundedRectangle (r, r.getWidth() / 20.0f);
    }

    void handleButtonPressed  (ControlButton::ButtonFunction function, uint32) override
    {
        displayButtonInteraction (controlButtonFunctionToIndex (function), true);
    }

    void handleButtonReleased (ControlButton::ButtonFunction function, uint32) override
    {
        displayButtonInteraction (controlButtonFunctionToIndex (function), false);
    }

    void handleBatteryLevelUpdate (float batteryLevel) override
    {
        // Update the number of LEDs that are on to represent the battery level
        auto numLedsOn = static_cast<int> (numLeds * batteryLevel);

        if (numLedsOn != previousNumLedsOn)
            for (auto i = 0; i < numLeds; ++i)
                leds.getUnchecked (i)->setOnState (i < numLedsOn);

        previousNumLedsOn = numLedsOn;
        repaint();
    }

private:
    //==============================================================================
    /**
        Base class that renders a Control Block button
    */
    struct ControlBlockSubComponent   : public Component,
                                        public TooltipClient
    {
        ControlBlockSubComponent (Colour componentColourToUse)
            : componentColour (componentColourToUse)
        {}

        /** Subclasses should override this to paint the button on the screen */
        void paint (Graphics&) override = 0;

        /** Sets the colour of the button */
        void setColour (Colour c)   { componentColour = c; }

        /** Sets the on state of the button */
        void setOnState (bool isOn)
        {
            onState = isOn;
            repaint();
        }

        /** Returns the Control Block tooltip */
        String getTooltip() override
        {
            for (Component* comp = this; comp != nullptr; comp = comp->getParentComponent())
                if (auto* sttc = dynamic_cast<SettableTooltipClient*> (comp))
                    return sttc->getTooltip();

            return {};
        }

        //==============================================================================
        Colour componentColour;
        bool onState = false;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlBlockSubComponent)
    };

    /**
        Class that renders a Control Block LED on the screen
    */
    struct LEDComponent  : public ControlBlockSubComponent
    {
        LEDComponent() : ControlBlockSubComponent (Colours::green) {}

        void paint (Graphics& g) override
        {
            g.setColour (componentColour.withAlpha (onState ? 1.0f : 0.2f));
            g.fillEllipse (getLocalBounds().toFloat());
        }
    };

    /**
        Class that renders a Control Block single circular button on the screen
    */
    struct CircleButtonComponent  : public ControlBlockSubComponent
    {
        CircleButtonComponent() : ControlBlockSubComponent (Colours::blue) {}

        void paint (Graphics& g) override
        {
            g.setColour (componentColour.withAlpha (onState ? 1.0f : 0.2f));
            g.fillEllipse (getLocalBounds().toFloat());
        }
    };

    /**
        Class that renders a Control Block rounded rectangular button containing two buttons
        on the screen
    */
    struct RoundedRectangleButtonComponent  : public ControlBlockSubComponent
    {
        RoundedRectangleButtonComponent() : ControlBlockSubComponent (Colours::blue) {}

        void paint (Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();

            g.setColour (componentColour.withAlpha (0.2f));
            g.fillRoundedRectangle (r.toFloat(), 20.0f);
            g.setColour (componentColour.withAlpha (1.0f));

            // is a button pressed?
            if (doubleButtonOnState[0] || doubleButtonOnState[1])
            {
                auto semiButtonWidth = r.getWidth() / 2.0f;

                auto semiButtonBounds = r.withWidth (semiButtonWidth)
                                         .withX (doubleButtonOnState[1] ? semiButtonWidth : 0)
                                         .reduced (5.0f, 2.0f);

                g.fillEllipse (semiButtonBounds);
            }
        }

        void setPressedState (bool isPressed, int button)
        {
            doubleButtonOnState[button] = isPressed;
            repaint();
        }

    private:
        bool doubleButtonOnState[2] = { false, false };
    };

    /** Displays a button press or release interaction for a button at a given index */
    void displayButtonInteraction (int buttonIndex, bool isPressed)
    {
        if (! isPositiveAndBelow (buttonIndex, 10))
            return;

        if (buttonIndex >= 8)
            roundedRectangleButton.setPressedState (isPressed, buttonIndex == 8);
        else
            circleButtons[buttonIndex].setOnState (isPressed);
    }

    //==============================================================================
    int numLeds;
    OwnedArray<LEDComponent> leds;
    CircleButtonComponent circleButtons[8];
    RoundedRectangleButtonComponent roundedRectangleButton;
    int previousNumLedsOn;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlBlockComponent)
};

//==============================================================================
/**
    The main component where the Block components will be displayed
*/
class BlocksMonitorDemo   : public Component,
                            public TopologySource::Listener,
                            private Timer
{
public:
    BlocksMonitorDemo()
    {
        noBlocksLabel.setText ("No BLOCKS connected...", dontSendNotification);
        noBlocksLabel.setJustificationType (Justification::centred);

        zoomOutButton.setButtonText ("+");
        zoomOutButton.onClick = [this] { blockUnitInPixels = (int) (blockUnitInPixels * 1.05f); resized(); };
        zoomOutButton.setAlwaysOnTop (true);

        zoomInButton.setButtonText ("-");
        zoomInButton.onClick = [this] { blockUnitInPixels = (int) (blockUnitInPixels * 0.95f); resized(); };
        zoomInButton.setAlwaysOnTop (true);

        // Register BlocksMonitorDemo as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

        startTimer (10000);

        addAndMakeVisible (noBlocksLabel);
        addAndMakeVisible (zoomOutButton);
        addAndMakeVisible (zoomInButton);

       #if JUCE_IOS
        connectButton.setButtonText ("Connect");
        connectButton.onClick = [] { BluetoothMidiDevicePairingDialogue::open(); };
        connectButton.setAlwaysOnTop (true);
        addAndMakeVisible (connectButton);
       #endif

        setSize (600, 600);

        topologyChanged();
    }

    ~BlocksMonitorDemo() override
    {
        topologySource.removeListener (this);
    }

    void paint (Graphics&) override {}

    void resized() override
    {
       #if JUCE_IOS
        connectButton.setBounds (getRight() - 100, 20, 80, 30);
       #endif

        noBlocksLabel.setVisible (false);
        auto numBlockComponents = blockComponents.size();

        // If there are no currently connected Blocks then display some text on the screen
        if (numBlockComponents == 0)
        {
            noBlocksLabel.setVisible (true);
            noBlocksLabel.setBounds (0, (getHeight() / 2) - 50, getWidth(), 100);
            return;
        }

        zoomOutButton.setBounds (10, getHeight() - 40, 40, 30);
        zoomInButton.setBounds  (zoomOutButton.getRight(), zoomOutButton.getY(), 40, 30);

        if (isInitialResized)
        {
            // Work out the area needed in terms of Block units
            Rectangle<float> maxArea;
            for (auto blockComponent : blockComponents)
            {
                auto topLeft = blockComponent->topLeft;
                auto rotation = blockComponent->rotation;
                auto blockSize = 0;

                if (rotation == 180)
                    blockSize = blockComponent->block->getWidth();
                else if (rotation == 90)
                    blockSize = blockComponent->block->getHeight();

                if (topLeft.x - blockSize < maxArea.getX())
                    maxArea.setX (topLeft.x - blockSize);

                blockSize = 0;
                if (rotation == 0)
                    blockSize = blockComponent->block->getWidth();
                else if (rotation == 270)
                    blockSize = blockComponent->block->getHeight();

                if (topLeft.x + blockSize > maxArea.getRight())
                    maxArea.setWidth (topLeft.x + blockSize);

                blockSize = 0;
                if (rotation == 180)
                    blockSize = blockComponent->block->getHeight();
                else if (rotation == 270)
                    blockSize = blockComponent->block->getWidth();

                if (topLeft.y - blockSize < maxArea.getY())
                    maxArea.setY (topLeft.y - blockSize);

                blockSize = 0;
                if (rotation == 0)
                    blockSize = blockComponent->block->getHeight();
                else if (rotation == 90)
                    blockSize = blockComponent->block->getWidth();

                if (topLeft.y + blockSize > maxArea.getBottom())
                    maxArea.setHeight (topLeft.y + blockSize);
            }

            auto totalWidth  = std::abs (maxArea.getX()) + maxArea.getWidth();
            auto totalHeight = std::abs (maxArea.getY()) + maxArea.getHeight();

            blockUnitInPixels = static_cast<int> (jmin ((getHeight() / totalHeight) - 50, (getWidth() / totalWidth) - 50));

            masterBlockComponent->centreWithSize (masterBlockComponent->block->getWidth()  * blockUnitInPixels,
                                                  masterBlockComponent->block->getHeight() * blockUnitInPixels);

            isInitialResized = false;
        }
        else
        {
            masterBlockComponent->setSize (masterBlockComponent->block->getWidth() * blockUnitInPixels, masterBlockComponent->block->getHeight() * blockUnitInPixels);
        }

        for (auto blockComponent : blockComponents)
        {
            if (blockComponent == masterBlockComponent)
                continue;

            blockComponent->setBounds (masterBlockComponent->getX() + static_cast<int> (blockComponent->topLeft.x * blockUnitInPixels),
                                       masterBlockComponent->getY() + static_cast<int> (blockComponent->topLeft.y * blockUnitInPixels),
                                       blockComponent->block->getWidth()  * blockUnitInPixels,
                                       blockComponent->block->getHeight() * blockUnitInPixels);

            if (blockComponent->rotation != 0)
                blockComponent->setTransform (AffineTransform::rotation (static_cast<float> (degreesToRadians (blockComponent->rotation)),
                                                                         static_cast<float> (blockComponent->getX()),
                                                                         static_cast<float> (blockComponent->getY())));
        }
    }

    /** Overridden from TopologySource::Listener, called when the topology changes */
    void topologyChanged() override
    {
        // Clear the array of Block components
        blockComponents.clear();
        masterBlockComponent = nullptr;

        // Get the current topology
        auto topology = topologySource.getCurrentTopology();

        // Create a BlockComponent object for each Block object and store a pointer to the master
        for (auto& block : topology.blocks)
        {
            if (auto* blockComponent = createBlockComponent (block))
            {
                addAndMakeVisible (blockComponents.add (blockComponent));

                if (blockComponent->block->isMasterBlock())
                    masterBlockComponent = blockComponent;
            }
        }

        // Must have a master Block!
        if (topology.blocks.size() > 0)
            jassert (masterBlockComponent != nullptr);

        // Calculate the relative position and rotation for each Block
        positionBlocks (topology);

        // Update the display
        isInitialResized = true;
        resized();
    }

private:
    /** Creates a BlockComponent object for a new Block and adds it to the content component */
    BlockComponent* createBlockComponent (Block::Ptr newBlock)
    {
        auto type = newBlock->getType();

        if (type == Block::lightPadBlock)
            return new LightpadComponent (newBlock);

        if (type == Block::loopBlock || type == Block::liveBlock
            || type == Block::touchBlock || type == Block::developerControlBlock)
            return new ControlBlockComponent (newBlock);

        // Should only be connecting a Lightpad or Control Block!
        jassertfalse;
        return nullptr;
    }

    /** Periodically updates the displayed BlockComponent tooltips */
    void timerCallback() override
    {
        for (auto c : blockComponents)
            c->updateStatsAndTooltip();
    }

    /** Calculates the position and rotation of each connected Block relative to the master Block */
    void positionBlocks (BlockTopology topology)
    {
        Array<BlockComponent*> blocksConnectedToMaster;

        auto maxDelta = std::numeric_limits<float>::max();
        auto maxLoops = 50;

        // Store all the connections to the master Block
        Array<BlockDeviceConnection> masterBlockConnections;
        for (auto connection : topology.connections)
            if (connection.device1 == masterBlockComponent->block->uid || connection.device2 == masterBlockComponent->block->uid)
                masterBlockConnections.add (connection);

        // Position all the Blocks that are connected to the master Block
        while (maxDelta > 0.001f && --maxLoops)
        {
            maxDelta = 0.0f;

            // Loop through each connection on the master Block
            for (auto connection : masterBlockConnections)
            {
                // Work out whether the master Block is device 1 or device 2 in the BlockDeviceConnection struct
                bool isDevice1 = true;
                if (masterBlockComponent->block->uid == connection.device2)
                    isDevice1 = false;

                // Get the connected ports
                auto masterPort = isDevice1 ? connection.connectionPortOnDevice1 : connection.connectionPortOnDevice2;
                auto otherPort  = isDevice1 ? connection.connectionPortOnDevice2 : connection.connectionPortOnDevice1;

                for (auto otherBlockComponent : blockComponents)
                {
                    // Get the other block
                    if (otherBlockComponent->block->uid == (isDevice1 ? connection.device2 : connection.device1))
                    {
                        blocksConnectedToMaster.addIfNotAlreadyThere (otherBlockComponent);

                        // Get the rotation of the other Block relative to the master Block
                        otherBlockComponent->rotation = getRotation (masterPort.edge, otherPort.edge);

                        // Get the offsets for the connected ports
                        auto masterBlockOffset = masterBlockComponent->getOffsetForPort (masterPort);
                        auto otherBlockOffset  = otherBlockComponent->topLeft + otherBlockComponent->getOffsetForPort (otherPort);

                        // Work out the distance between the two connected ports
                        auto delta = masterBlockOffset - otherBlockOffset;

                        // Move the other block half the distance to the connection
                        otherBlockComponent->topLeft += delta / 2.0f;

                        // Work out whether we are close enough for the loop to end
                        maxDelta = jmax (maxDelta, std::abs (delta.x), std::abs (delta.y));
                    }
                }
            }
        }

        // Check if there are any Blocks that have not been positioned yet
        Array<BlockComponent*> unpositionedBlocks;

        for (auto blockComponent : blockComponents)
            if (blockComponent != masterBlockComponent && ! blocksConnectedToMaster.contains (blockComponent))
                unpositionedBlocks.add (blockComponent);

        if (unpositionedBlocks.size() > 0)
        {
            // Reset the loop conditions
            maxDelta = std::numeric_limits<float>::max();
            maxLoops = 50;

            // Position all the remaining Blocks
            while (maxDelta > 0.001f && --maxLoops)
            {
                maxDelta = 0.0f;

                // Loop through each unpositioned Block
                for (auto blockComponent : unpositionedBlocks)
                {
                    // Store all the connections to this Block
                    Array<BlockDeviceConnection> blockConnections;
                    for (auto connection : topology.connections)
                        if (connection.device1 == blockComponent->block->uid || connection.device2 == blockComponent->block->uid)
                            blockConnections.add (connection);

                    // Loop through each connection on this Block
                    for (auto connection : blockConnections)
                    {
                        // Work out whether this Block is device 1 or device 2 in the BlockDeviceConnection struct
                        auto isDevice1 = true;
                        if (blockComponent->block->uid == connection.device2)
                            isDevice1 = false;

                        // Get the connected ports
                        auto thisPort  = isDevice1 ? connection.connectionPortOnDevice1 : connection.connectionPortOnDevice2;
                        auto otherPort = isDevice1 ? connection.connectionPortOnDevice2 : connection.connectionPortOnDevice1;

                        // Get the other Block
                        for (auto otherBlockComponent : blockComponents)
                        {
                            if (otherBlockComponent->block->uid == (isDevice1 ? connection.device2 : connection.device1))
                            {
                                // Get the rotation
                                auto rotation = getRotation (otherPort.edge, thisPort.edge) + otherBlockComponent->rotation;
                                if (rotation > 360)
                                    rotation -= 360;

                                blockComponent->rotation = rotation;

                                // Get the offsets for the connected ports
                                auto otherBlockOffset = (otherBlockComponent->topLeft + otherBlockComponent->getOffsetForPort (otherPort));
                                auto thisBlockOffset  = (blockComponent->topLeft + blockComponent->getOffsetForPort (thisPort));

                                // Work out the distance between the two connected ports
                                auto delta = otherBlockOffset - thisBlockOffset;

                                // Move this block half the distance to the connection
                                blockComponent->topLeft += delta / 2.0f;

                                // Work out whether we are close enough for the loop to end
                                maxDelta = jmax (maxDelta, std::abs (delta.x), std::abs (delta.y));
                            }
                        }
                    }
                }
            }
        }
    }

    /** Returns a rotation in degrees based on the connected edges of two blocks */
    int getRotation (Block::ConnectionPort::DeviceEdge staticEdge, Block::ConnectionPort::DeviceEdge rotatedEdge)
    {
        using edge = Block::ConnectionPort::DeviceEdge;

        switch (staticEdge)
        {
            case edge::north:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 180;
                    case edge::south:
                        return 0;
                    case edge::east:
                        return 90;
                    case edge::west:
                        return 270;
                }
            }
            case edge::south:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 0;
                    case edge::south:
                        return 180;
                    case edge::east:
                        return 270;
                    case edge::west:
                        return 90;
                }
            }
            case edge::east:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 270;
                    case edge::south:
                        return 90;
                    case edge::east:
                        return 180;
                    case edge::west:
                        return 0;
                }
            }

            case edge::west:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 90;
                    case edge::south:
                        return 270;
                    case edge::east:
                        return 0;
                    case edge::west:
                        return 180;
                }
            }
        }

        return 0;
    }

    //==============================================================================
    TooltipWindow tooltipWindow;

    PhysicalTopologySource topologySource;
    OwnedArray<BlockComponent> blockComponents;
    BlockComponent* masterBlockComponent = nullptr;

    Label noBlocksLabel;

    TextButton zoomOutButton;
    TextButton zoomInButton;

    int blockUnitInPixels;
    bool isInitialResized;

   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlocksMonitorDemo)
};
