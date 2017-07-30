/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

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
        if (auto grid = block->getLEDGrid())
            block->setProgram (new BitmapLEDProgram (*block));

        // If this is a Lightpad then redraw it at 25Hz
        if (block->getType() == Block::lightPadBlock)
            startTimerHz (25);

        // Make sure the component can't go offscreen if it is draggable
        constrainer.setMinimumOnscreenAmounts (50, 50, 50, 50);
    }

    ~BlockComponent()
    {
        // Remove any listeners
        if (auto touchSurface = block->getTouchSurface())
            touchSurface->removeListener (this);

        for (auto button : block->getButtons())
            button->removeListener (this);
    }

    /** Called periodically to update the tooltip with inforamtion about the Block */
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
    virtual void paint (Graphics&) override = 0;

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
            { CB::mode,     CB::button0 },
            { CB::volume,   CB::button1 },
            { CB::scale,    CB::button2,    CB::click },
            { CB::chord,    CB::button3,    CB::snap },
            { CB::arp,      CB::button4,    CB::back },
            { CB::sustain,  CB::button5,    CB::playOrPause },
            { CB::octave,   CB::button6,    CB::record },
            { CB::love,     CB::button7,    CB::learn },
            { CB::up },
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

        return Point<float>();
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
    {
    }

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

        float maxCircleSize = block->getWidth() / 3.0f;

        // iterate over the list of current touches and draw them on the onscreen Block
        for (auto touch : touches)
        {
            float circleSize = touch.touch.z * maxCircleSize;

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
        for (int i = 0; i < numLeds; ++i)
        {
            auto ledComponent = new LEDComponent();
            ledComponent->setOnState (i < numLedsToTurnOn);

            addAndMakeVisible (leds.add (ledComponent));
        }

        previousNumLedsOn = numLedsToTurnOn;

        // add buttons
        for (int i = 0; i < 8; ++i)
            addAndMakeVisible (circleButtons[i]);
    }

    void resized() override
    {
        const auto r = getLocalBounds().reduced (10);

        const int rowHeight   = r.getHeight() / 5;
        const int ledWidth    = (r.getWidth() - 70) / numLeds;
        const int buttonWidth = (r.getWidth() - 40) / 5;

        auto row = r;

        auto ledRow     = row.removeFromTop (rowHeight)    .withSizeKeepingCentre (r.getWidth(), ledWidth);
        auto buttonRow1 = row.removeFromTop (rowHeight * 2).withSizeKeepingCentre (r.getWidth(), buttonWidth);
        auto buttonRow2 = row.removeFromTop (rowHeight * 2).withSizeKeepingCentre (r.getWidth(), buttonWidth);

        for (auto* led : leds)
        {
            led->setBounds (ledRow.removeFromLeft (ledWidth).reduced (2));
            ledRow.removeFromLeft (5);
        }

        for (int i = 0; i < 5; ++i)
        {
            circleButtons[i].setBounds (buttonRow1.removeFromLeft (buttonWidth).reduced (2));
            buttonRow1.removeFromLeft (10);
        }

        for (int i = 5; i < 8; ++i)
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
        int numLedsOn = static_cast<int> (numLeds * batteryLevel);

        if (numLedsOn != previousNumLedsOn)
            for (int i = 0; i < numLeds; ++i)
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
        virtual void paint (Graphics&) override = 0;

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
