
#ifndef BLOCKCOMPONENTS_H_INCLUDED
#define BLOCKCOMPONENTS_H_INCLUDED

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
    /** Constructor */
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
            grid->setProgram (new BitmapLEDProgram(*grid));

        // If this is a Lightpad then redraw it at 25Hz
        if (block->getType() == Block::lightPadBlock)
            startTimerHz (25);
    }

    /** Destructor */
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
        const float batteryLevel = block->getBatteryLevel();
        handleBatteryLevelUpdate (batteryLevel);

        // Format the tooltip string
        const String ttString = "Name = "          + block->getDeviceDescription() + "\n"
                              + "UID = "           + String (block->uid) + "\n"
                              + "Serial number = " + block->serialNumber + "\n"
                              + "Battery level = " + String ((int) (batteryLevel * 100)) + "%"
                              + (block->isBatteryCharging() ? "++" : "--");

        // Update the tooltip string if it has changed
        if (ttString != getTooltip())
            setTooltip (ttString);
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
        static std::initializer_list<ControlButton::ButtonFunction> map[] =
           {{ControlButton::mode, ControlButton::button0},
            {ControlButton::volume, ControlButton::button1},
            {ControlButton::scale, ControlButton::button2, ControlButton::click},
            {ControlButton::chord, ControlButton::button3, ControlButton::snap},
            {ControlButton::arp,ControlButton:: button4, ControlButton::back},
            {ControlButton::sustain, ControlButton::button5, ControlButton::playOrPause},
            {ControlButton::octave, ControlButton::button6, ControlButton::record},
            {ControlButton::love, ControlButton::button7, ControlButton::learn},
            {ControlButton::up},
            {ControlButton::down}};

        for (size_t i = 0; i < (sizeof (map) / sizeof (map[0])); ++i)
            if (std::find (map[i].begin(), map[i].end(), f) != map[i].end())
                return static_cast<int> (i);

        return -1;
    }

private:
    /** Used to call repaint() periodically */
    void timerCallback() override { repaint(); }

    /** Overridden from TouchSurface::Listener */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& t) override { handleTouchChange (t); }

    /** Overridden from ControlButton::Listener */
    void buttonPressed  (ControlButton& b, Block::Timestamp t) override      { handleButtonPressed  (b.getType(), t); }

    /** Overridden from ControlButton::Listener */
    void buttonReleased (ControlButton& b, Block::Timestamp t) override      { handleButtonReleased (b.getType(), t); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockComponent)
};

/**
    Class that renders a Lightpad on the screen
*/
class LightpadComponent : public BlockComponent
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
        const Point<float> ratio (r.getWidth() / block->getWidth(),
                                  r.getHeight() / block->getHeight());
        const float maxCircleSize = block->getWidth() / 3.0f;

        // iterate over the list of current touches and draw them on the onscreen Block
        for (auto touch : touches)
        {
            const float circleSize           = touch.touch.z * maxCircleSize;
            const Point<float> touchPosition = Point<float> (touch.touch.x, touch.touch.y);

            const Colour c = colourArray[touch.touch.index];
            const Rectangle<float> blob =
                (Rectangle<float> (circleSize, circleSize).withCentre (touchPosition)) * ratio;

            const ColourGradient cg = ColourGradient (colourArray[touch.touch.index],     blob.getCentreX(), blob.getCentreY(),
                                                      Colours::transparentBlack,          blob.getRight(),   blob.getBottom(),
                                                      true);

            g.setGradientFill (cg);
            g.fillEllipse (blob);
        }
    }

    void handleTouchChange (TouchSurface::Touch touch) override { touches.updateTouch (touch); }

private:
    /** An Array of colours to use for touches */
    Array<Colour> colourArray = { Colours::red, Colours::blue, Colours::green,
                                  Colours::yellow, Colours::white, Colours::hotpink,
                                  Colours::mediumpurple };

    /** A list of current Touch events */
    TouchList<TouchSurface::Touch> touches;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LightpadComponent)
};

/**
    Class that renders a Control Block on the screen
*/
class ControlBlockComponent : public BlockComponent
{
public:
    ControlBlockComponent (Block::Ptr blockToUse)
        : BlockComponent (blockToUse),
          numLeds (block->getLEDRow()->getNumLEDs())
    {
        addAndMakeVisible (roundedRectangleButton);

        // Display the battery level on the LEDRow
        int numLedsToTurnOn = static_cast<int> (static_cast<float> (numLeds) * block->getBatteryLevel());

        // add LEDs
        LEDComponent* ledComponent;
        for (int i = 0; i < numLeds; ++i)
        {
            ledComponent = new LEDComponent();
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

        for (int i = 0; i < numLeds; ++i)
        {
            leds.getUnchecked (i)->setBounds (ledRow.removeFromLeft (ledWidth).reduced (2));
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
        const auto r = getLocalBounds().toFloat();

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
        int numLedsOn = static_cast<int> (static_cast<float> (numLeds) * batteryLevel);

        if (numLedsOn != previousNumLedsOn)
            for (int i = 0; i < numLeds; ++i)
                leds.getUnchecked (i)->setOnState (i < numLedsOn);

        previousNumLedsOn = numLedsOn;
        repaint();
    }

private:
    /**
        Base class that renders a Control Block button
    */
    struct ControlBlockSubComponent : public Component,
                                      public TooltipClient
    {
        ControlBlockSubComponent (Colour componentColourToUse)
            : componentColour (componentColourToUse),
              onState (false)
        {}

        /** Subclasses should override this to paint the button on the screen */
        virtual void paint (Graphics&) override = 0;

        /** Sets the colour of the button */
        void setColour  (Colour c)  { componentColour = c; }

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
                if (SettableTooltipClient* sttc = dynamic_cast<SettableTooltipClient*> (comp))
                    return sttc->getTooltip();

            return String();
        }

        //==============================================================================
        Colour componentColour;
        bool onState;

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
            const auto r = getLocalBounds().toFloat();


            g.setColour (componentColour.withAlpha (0.2f));
            g.fillRoundedRectangle (r.toFloat(), 20.0f);
            g.setColour (componentColour.withAlpha (1.0f));

            // is a button pressed?
            if (doubleButtonOnState[0] || doubleButtonOnState[1])
            {
                const float semiButtonWidth = r.getWidth() / 2.0f;
                const auto semiButtonBounds = r.withWidth (semiButtonWidth)
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
        bool doubleButtonOnState[2] = {false, false};
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

#endif  // BLOCKCOMPONENTS_H_INCLUDED
