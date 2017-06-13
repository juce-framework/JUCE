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
#include "LightpadComponent.h"

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
class MainComponent   : public Component,
                        public TopologySource::Listener,
                        private TouchSurface::Listener,
                        private ControlButton::Listener,
                        private LightpadComponent::Listener,
                        private Button::Listener,
                        private Slider::Listener,
                        private Timer
{
public:
    MainComponent();
    ~MainComponent();

    void resized() override;

    /** Overridden from TopologySource::Listener. Called when the topology changes */
    void topologyChanged() override;

private:
    /** Overridden from TouchSurface::Listener. Called when a Touch is received on the Lightpad */
    void touchChanged (TouchSurface&, const TouchSurface::Touch&) override;

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is pressed */
    void buttonPressed (ControlButton&, Block::Timestamp) override { }

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is released */
    void buttonReleased (ControlButton&, Block::Timestamp) override;

    void ledClicked (int x, int y, float z) override;

    void buttonClicked (Button*) override;

    void sliderValueChanged (Slider*) override;

    void timerCallback() override;

    /** Removes TouchSurface and ControlButton listeners and sets activeBlock to nullptr */
    void detachActiveBlock();

    /** Sets the LEDGrid Program for the selected mode */
    void setLEDProgram (Block&);

    void clearLEDs();

    /** Sets an LED on the Lightpad for a given touch co-ordinate and pressure */
    void drawLED (uint32 x0, uint32 y0, float z, Colour drawColour);

    /** Redraws the LEDs on the Lightpad from the activeLeds array */
    void redrawLEDs();

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
        for (int i = 0; i < activeLeds.size(); ++i)
            if (activeLeds.getReference(i).occupies (x, y))
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

    float scaleX = 0.0;
    float scaleY = 0.0;

    bool doublePress = false;

    Label infoLabel;
    LightpadComponent lightpadComponent;
    TextButton clearButton;
    LEDComponent brightnessLED;
    Slider brightnessSlider;

   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
