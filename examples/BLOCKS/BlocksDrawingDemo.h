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

 name:             BlocksDrawingDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Blocks application to draw shapes.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_blocks_basics,
                   juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        BlocksDrawingDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
/**
    Represents a single LED on a Lightpad
*/
struct LEDComponent : public Component
{
    LEDComponent() : ledColour (Colours::black) { setInterceptsMouseClicks (false, false); }

    void setColour (Colour newColour)
    {
        ledColour = newColour;
        repaint();
    }

    void paint (Graphics& g) override
    {
        g.setColour (ledColour);
        g.fillEllipse (getLocalBounds().toFloat());
    }

    Colour ledColour;
};

//==============================================================================
/**
    A component that is used to represent a Lightpad on-screen
*/
class DrawableLightpadComponent : public Component
{
public:
    DrawableLightpadComponent ()
    {
        for (auto x = 0; x < 15; ++x)
            for (auto y = 0; y < 15; ++y)
                addAndMakeVisible (leds.add (new LEDComponent()));
    }

    void paint (Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();

        // Clip the drawing area to only draw in the block area
        {
            Path clipArea;
            clipArea.addRoundedRectangle (r, r.getWidth() / 20.0f);

            g.reduceClipRegion (clipArea);
        }

        // Fill a black square for the Lightpad
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (10);

        auto circleWidth = r.getWidth() / 15;
        auto circleHeight = r.getHeight() / 15;

        for (auto x = 0; x < 15; ++x)
            for (auto y = 0; y < 15; ++y)
                leds.getUnchecked ((x * 15) + y)->setBounds (r.getX() + (x * circleWidth),
                                                             r.getY() + (y * circleHeight),
                                                             circleWidth, circleHeight);
    }

    void mouseDown (const MouseEvent& e) override
    {
        for (auto x = 0; x < 15; ++x)
            for (auto y = 0; y < 15; ++y)
                if (leds.getUnchecked ((x * 15) + y)->getBounds().contains (e.position.toInt()))
                    listeners.call ([&] (Listener& l) { l.ledClicked (x, y, e.pressure); });
    }

    void mouseDrag (const MouseEvent& e) override
    {
        for (auto x = 0; x < 15; ++x)
        {
            for (auto y = 0; y < 15; ++y)
            {
                if (leds.getUnchecked ((x * 15) + y)->getBounds().contains (e.position.toInt()))
                {
                    auto t = e.eventTime;

                    if (lastLED == Point<int> (x, y) && t.toMilliseconds() - lastMouseEventTime.toMilliseconds() < 50)
                        return;

                    listeners.call ([&] (Listener& l) { l.ledClicked (x, y, e.pressure); });

                    lastLED = { x, y };
                    lastMouseEventTime = t;
                }
            }
        }
    }

    //==============================================================================
    /** Sets the colour of one of the LEDComponents */
    void setLEDColour (int x, int y, Colour c)
    {
        x = jmin (x, 14);
        y = jmin (y, 14);

        leds.getUnchecked ((x * 15) + y)->setColour (c);
    }

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() {}

        /** Called when an LEDComponent has been clicked */
        virtual void ledClicked (int x, int y, float z) = 0;
    };

    void addListener (Listener* l)       { listeners.add (l); }
    void removeListener (Listener* l)    { listeners.remove (l); }

private:
    OwnedArray<LEDComponent> leds;
    ListenerList<Listener> listeners;

    Time lastMouseEventTime;
    Point<int> lastLED;
};

//==============================================================================
/**
    A struct that handles the setup and layout of the DrumPadGridProgram
*/
struct ColourGrid
{
    ColourGrid (int cols, int rows)
        : numColumns (cols),
          numRows (rows)
    {
        constructGridFillArray();
    }

    /** Creates a GridFill object for each pad in the grid and sets its colour
        and fill before adding it to an array of GridFill objects
    */
    void constructGridFillArray()
    {
        gridFillArray.clear();

        auto counter = 0;

        for (auto i = 0; i < numColumns; ++i)
        {
            for (auto j = 0; j < numRows; ++j)
            {
                DrumPadGridProgram::GridFill fill;
                Colour colourToUse = colourArray.getUnchecked (counter);

                fill.colour = colourToUse.withBrightness (colourToUse == currentColour ? 1.0f : 0.1f);

                if (colourToUse == Colours::black)
                    fill.fillType = DrumPadGridProgram::GridFill::FillType::hollow;
                else
                    fill.fillType = DrumPadGridProgram::GridFill::FillType::filled;

                gridFillArray.add (fill);

                if (++counter == colourArray.size())
                    counter = 0;
            }
        }
    }

    /** Sets which colour should be active for a given touch co-ordinate. Returns
        true if the colour has changed
    */
    bool setActiveColourForTouch (int x, int y)
    {
        auto colourHasChanged = false;

        auto xindex = x / 5;
        auto yindex = y / 5;

        auto newColour = colourArray.getUnchecked ((yindex * 3) + xindex);
        if (currentColour != newColour)
        {
            currentColour = newColour;
            constructGridFillArray();
            colourHasChanged = true;
        }

        return colourHasChanged;
    }

    //==============================================================================
    int numColumns, numRows;

    Array<DrumPadGridProgram::GridFill> gridFillArray;

    Array<Colour> colourArray = { Colours::white, Colours::red, Colours::green,
                                  Colours::blue, Colours::hotpink, Colours::orange,
                                  Colours::magenta, Colours::cyan, Colours::black };

    Colour currentColour = Colours::hotpink;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourGrid)
};

//==============================================================================
/**
    The main component
*/
class BlocksDrawingDemo   : public Component,
                            public TopologySource::Listener,
                            private TouchSurface::Listener,
                            private ControlButton::Listener,
                            private DrawableLightpadComponent::Listener,
                            private Timer
{
public:
    //==============================================================================
    BlocksDrawingDemo()
    {
        activeLeds.clear();

        // Register MainContentComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

        infoLabel.setText ("Connect a Lightpad Block to draw.", dontSendNotification);
        infoLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (infoLabel);

        addAndMakeVisible (lightpadComponent);
        lightpadComponent.setVisible (false);
        lightpadComponent.addListener (this);

        clearButton.setButtonText ("Clear");
        clearButton.onClick = [this] { clearLEDs(); };
        clearButton.setAlwaysOnTop (true);
        addAndMakeVisible (clearButton);

        brightnessSlider.setRange (0.0, 1.0);
        brightnessSlider.setValue (1.0);
        brightnessSlider.setAlwaysOnTop (true);
        brightnessSlider.setTextBoxStyle (Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
        brightnessSlider.onValueChange = [this]
        {
            brightnessLED.setColour (layout.currentColour
                                           .withBrightness (layout.currentColour == Colours::black ? 0.0f
                                                                                                   : static_cast<float> (brightnessSlider.getValue())));
        };
        addAndMakeVisible (brightnessSlider);

        brightnessLED.setAlwaysOnTop (true);
        brightnessLED.setColour (layout.currentColour.withBrightness (static_cast<float> (brightnessSlider.getValue())));
        addAndMakeVisible (brightnessLED);

       #if JUCE_IOS
        connectButton.setButtonText ("Connect");
        connectButton.onClick = [] { BluetoothMidiDevicePairingDialogue::open(); };
        connectButton.setAlwaysOnTop (true);
        addAndMakeVisible (connectButton);
       #endif

        setSize (600, 600);

        topologyChanged();
    }

    ~BlocksDrawingDemo()
    {
        if (activeBlock != nullptr)
            detachActiveBlock();

        lightpadComponent.removeListener (this);
        topologySource.removeListener (this);
    }

    void resized() override
    {
        infoLabel.centreWithSize (getWidth(), 100);

        auto bounds = getLocalBounds().reduced (20);

        // top buttons
        auto topButtonArea = bounds.removeFromTop (getHeight() / 20);

        topButtonArea.removeFromLeft (20);
        clearButton.setBounds (topButtonArea.removeFromLeft (80));

       #if JUCE_IOS
        topButtonArea.removeFromRight (20);
        connectButton.setBounds (topButtonArea.removeFromRight (80));
       #endif

        bounds.removeFromTop (20);

        auto orientation = Desktop::getInstance().getCurrentOrientation();

        if (orientation == Desktop::DisplayOrientation::upright
               || orientation == Desktop::DisplayOrientation::upsideDown)
        {
            auto brightnessControlBounds = bounds.removeFromBottom (getHeight() / 10);

            brightnessSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
            brightnessLED.setBounds (brightnessControlBounds.removeFromLeft (getHeight() / 10));
            brightnessSlider.setBounds (brightnessControlBounds);
        }
        else
        {
            auto brightnessControlBounds = bounds.removeFromRight (getWidth() / 10);

            brightnessSlider.setSliderStyle (Slider::SliderStyle::LinearVertical);
            brightnessLED.setBounds (brightnessControlBounds.removeFromTop (getWidth() / 10));
            brightnessSlider.setBounds (brightnessControlBounds);
        }

        // lightpad component
        auto sideLength = jmin (bounds.getWidth() - 40, bounds.getHeight() - 40);
        lightpadComponent.centreWithSize (sideLength, sideLength);
    }

    /** Overridden from TopologySource::Listener. Called when the topology changes */
    void topologyChanged() override
    {
        lightpadComponent.setVisible (false);
        infoLabel.setVisible (true);

        // Reset the activeBlock object
        if (activeBlock != nullptr)
            detachActiveBlock();

        // Get the array of currently connected Block objects from the PhysicalTopologySource
        auto blocks = topologySource.getCurrentTopology().blocks;

        // Iterate over the array of Block objects
        for (auto b : blocks)
        {
            // Find the first Lightpad
            if (b->getType() == Block::Type::lightPadBlock)
            {
                activeBlock = b;

                // Register MainContentComponent as a listener to the touch surface
                if (auto surface = activeBlock->getTouchSurface())
                    surface->addListener (this);

                // Register MainContentComponent as a listener to any buttons
                for (auto button : activeBlock->getButtons())
                    button->addListener (this);

                // Get the LEDGrid object from the Lightpad and set its program to the program for the current mode
                if (auto grid = activeBlock->getLEDGrid())
                {
                    // Work out scale factors to translate X and Y touches to LED indexes
                    scaleX = (float) (grid->getNumColumns() - 1) / activeBlock->getWidth();
                    scaleY = (float) (grid->getNumRows() - 1)    / activeBlock->getHeight();

                    setLEDProgram (*activeBlock);
                }

                // Make the on screen Lighpad component visible
                lightpadComponent.setVisible (true);
                infoLabel.setVisible (false);

                break;
            }
        }
    }

private:
    //==============================================================================
    /** Overridden from TouchSurface::Listener. Called when a Touch is received on the Lightpad */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& touch) override
    {
        // Translate X and Y touch events to LED indexes
        auto xLed = roundToInt (touch.x * scaleX);
        auto yLed = roundToInt (touch.y * scaleY);

        if (currentMode == colourPalette)
        {
            if (layout.setActiveColourForTouch (xLed, yLed))
            {
                if (auto* colourPaletteProgram = getPaletteProgram())
                {
                    colourPaletteProgram->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
                    brightnessLED.setColour (layout.currentColour
                                                   .withBrightness (layout.currentColour == Colours::black ? 0.0f
                                                                                                           : static_cast<float> (brightnessSlider.getValue())));
                }
            }
        }
        else if (currentMode == canvas)
        {
            drawLED ((uint32) xLed, (uint32) yLed, touch.z, layout.currentColour);
        }
    }

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is pressed */
    void buttonPressed (ControlButton&, Block::Timestamp) override {}

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is released */
    void buttonReleased (ControlButton&, Block::Timestamp) override
    {
        if (currentMode == canvas)
        {
            // Wait 500ms to see if there is a second press
            if (! isTimerRunning())
                startTimer (500);
            else
                doublePress = true;
        }
        else if (currentMode == colourPalette)
        {
            // Switch to canvas mode and set the LEDGrid program
            currentMode = canvas;
            setLEDProgram (*activeBlock);
        }
    }

    void ledClicked (int x, int y, float z) override
    {
        drawLED ((uint32) x, (uint32) y,
                 z == 0.0f ? static_cast<float> (brightnessSlider.getValue())
                           : z * static_cast<float> (brightnessSlider.getValue()), layout.currentColour);
    }

    void timerCallback() override
    {
        if (doublePress)
        {
            clearLEDs();

            // Reset the doublePress flag
            doublePress = false;
        }
        else
        {
            // Switch to colour palette mode and set the LEDGrid program
            currentMode = colourPalette;
            setLEDProgram (*activeBlock);
        }

        stopTimer();
    }

    /** Removes TouchSurface and ControlButton listeners and sets activeBlock to nullptr */
    void detachActiveBlock()
    {
        if (auto surface = activeBlock->getTouchSurface())
            surface->removeListener (this);

        for (auto button : activeBlock->getButtons())
            button->removeListener (this);

        activeBlock = nullptr;
    }

    /** Sets the LEDGrid Program for the selected mode */
    void setLEDProgram (Block& block)
    {
        if (currentMode == canvas)
        {
            block.setProgram (new BitmapLEDProgram (block));

            // Redraw any previously drawn LEDs
            redrawLEDs();
        }
        else if (currentMode == colourPalette)
        {
            block.setProgram (new DrumPadGridProgram (block));

            // Setup the grid layout
            if (auto* program = getPaletteProgram())
                program->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
        }
    }

    void clearLEDs()
    {
        if (auto* canvasProgram = getCanvasProgram())
        {
            // Clear the LED grid
            for (uint32 x = 0; x < 15; ++x)
            {
                for (uint32 y = 0; y < 15; ++ y)
                {
                    canvasProgram->setLED (x, y, Colours::black);
                    lightpadComponent.setLEDColour ((int) x, (int) y, Colours::black);
                }
            }

            // Clear the ActiveLED array
            activeLeds.clear();
        }
    }

    /** Sets an LED on the Lightpad for a given touch co-ordinate and pressure */
    void drawLED (uint32 x0, uint32 y0, float z, Colour drawColour)
    {
        if (auto* canvasProgram = getCanvasProgram())
        {
            // Check if the activeLeds array already contains an ActiveLED object for this LED
            auto index = getLEDAt (x0, y0);

            // If the colour is black then just set the LED to black and return
            if (drawColour == Colours::black)
            {
                if (index >= 0)
                {
                    canvasProgram->setLED (x0, y0, Colours::black);
                    lightpadComponent.setLEDColour ((int) x0, (int) y0, Colours::black);
                    activeLeds.remove (index);
                }

                return;
            }

            // If there is no ActiveLED obejct for this LED then create one,
            // add it to the array, set the LED on the Block and return
            if (index < 0)
            {
                ActiveLED led;
                led.x = x0;
                led.y = y0;
                led.colour = drawColour;
                led.brightness = z;

                activeLeds.add (led);
                canvasProgram->setLED (led.x, led.y, led.colour.withBrightness (led.brightness));

                lightpadComponent.setLEDColour ((int) led.x, (int) led.y, led.colour.withBrightness (led.brightness));

                return;
            }

            // Get the ActiveLED object for this LED
            auto currentLed = activeLeds.getReference (index);

            // If the LED colour is the same as the draw colour, add the brightnesses together.
            // If it is different, blend the colours
            if (currentLed.colour == drawColour)
                currentLed.brightness = jmin (currentLed.brightness + z, 1.0f);
            else
                currentLed.colour = currentLed.colour.interpolatedWith (drawColour, z);


            // Set the LED on the Block and change the ActiveLED object in the activeLeds array
            if (canvasProgram != nullptr)
                canvasProgram->setLED (currentLed.x, currentLed.y, currentLed.colour.withBrightness (currentLed.brightness));

            lightpadComponent.setLEDColour ((int) currentLed.x, (int) currentLed.y, currentLed.colour.withBrightness (currentLed.brightness));

            activeLeds.set (index, currentLed);
        }
    }

    /** Redraws the LEDs on the Lightpad from the activeLeds array */
    void redrawLEDs()
    {
        if (auto* canvasProgram = getCanvasProgram())
        {
            // Iterate over the activeLeds array and set the LEDs on the Block
            for (auto led : activeLeds)
            {
                canvasProgram->setLED (led.x, led.y, led.colour.withBrightness (led.brightness));
                lightpadComponent.setLEDColour ((int) led.x, (int) led.y, led.colour.withBrightness (led.brightness));
            }
        }
    }

    //==============================================================================
    BitmapLEDProgram* getCanvasProgram()
    {
        if (activeBlock != nullptr)
            return dynamic_cast<BitmapLEDProgram*> (activeBlock->getProgram());

        return nullptr;
    }

    DrumPadGridProgram* getPaletteProgram()
    {
        if (activeBlock != nullptr)
            return dynamic_cast<DrumPadGridProgram*> (activeBlock->getProgram());

        return nullptr;
    }

    //==============================================================================
    /**
        A struct that represents an active LED on the Lightpad.
        Has a position, colour and brightness.
    */
    struct ActiveLED
    {
        uint32 x, y;
        Colour colour;
        float brightness;

        /** Returns true if this LED occupies the given co-ordinates */
        bool occupies (uint32 xPos, uint32 yPos) const
        {
            return xPos == x && yPos == y;
        }
    };

    Array<ActiveLED> activeLeds;

    int getLEDAt (uint32 x, uint32 y) const
    {
        for (auto i = 0; i < activeLeds.size(); ++i)
            if (activeLeds.getReference (i).occupies (x, y))
                return i;

        return -1;
    }

    //==============================================================================
    enum DisplayMode
    {
        colourPalette = 0,
        canvas
    };

    DisplayMode currentMode = colourPalette;

    //==============================================================================
    ColourGrid layout { 3, 3 };
    PhysicalTopologySource topologySource;
    Block::Ptr activeBlock;

    float scaleX = 0.0f;
    float scaleY = 0.0f;

    bool doublePress = false;

    Label infoLabel;
    DrawableLightpadComponent lightpadComponent;
    TextButton clearButton;
    LEDComponent brightnessLED;
    Slider brightnessSlider;

   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlocksDrawingDemo)
};
