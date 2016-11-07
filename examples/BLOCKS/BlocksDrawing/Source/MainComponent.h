
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

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

        int counter = 0;

        for (int i = 0; i < numColumns; ++i)
        {
            for (int j = 0; j < numRows; ++j)
            {
                DrumPadGridProgram::GridFill fill;
                Colour colourToUse = colourArray.getUnchecked (counter);

                fill.colour = colourToUse.withBrightness (colourToUse == currentColour ? 1.0 : 0.1);

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
        bool colourHasChanged = false;

        int xindex = x / 5;
        int yindex = y / 5;

        Colour newColour = colourArray.getUnchecked ((yindex * 3) + xindex);
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
    float width, height;

    Array<DrumPadGridProgram::GridFill> gridFillArray;
    Array<Colour> colourArray = { Colours::white, Colours::red, Colours::green, Colours::blue, Colours::hotpink,
                                  Colours::orange, Colours::magenta, Colours::cyan, Colours::black };
    Colour currentColour = Colours::hotpink;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourGrid)
};

/**
    The main component
*/
class MainComponent   : public Component,
                               public TopologySource::Listener,
                               private TouchSurface::Listener,
                               private ControlButton::Listener,
                               private Timer
{
public:
    MainComponent() : layout (3, 3)
    {
        setSize (600, 400);

        activeLeds.clear();

        // Register MainContentComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);
    }

    ~MainComponent()
    {
        if (activeBlock != nullptr)
            detachActiveBlock();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);
        g.drawText ("Connect a Lightpad Block to draw.", getLocalBounds(), Justification::centred, false);
    }

    void resized() override {}

    /** Overridden from TopologySource::Listener. Called when the topology changes */
    void topologyChanged() override
    {
        // Reset the activeBlock object
        if (activeBlock != nullptr)
            detachActiveBlock();

        // Get the array of currently connected Block objects from the PhysicalTopologySource
        Block::Array blocks = topologySource.getCurrentTopology().blocks;

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

                    setLEDProgram (grid);
                }

                break;
            }
        }
    }

private:
    /** Overridden from TouchSurface::Listener. Called when a Touch is received on the Lightpad */
    void touchChanged (TouchSurface&, const TouchSurface::Touch& touch) override
    {
        // Translate X and Y touch events to LED indexes
        int xLed = roundToInt (touch.x * scaleX);
        int yLed = roundToInt (touch.y * scaleY);

        if (currentMode == colourPalette)
        {
            if (layout.setActiveColourForTouch (xLed, yLed))
                colourPaletteProgram->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
        }
        else if (currentMode == canvas)
        {
            drawLEDs ((uint32) xLed, (uint32) yLed, touch.z, layout.currentColour);
        }
    }

    /** Overridden from ControlButton::Listener. Called when a button on the Lightpad is pressed */
    void buttonPressed (ControlButton&, Block::Timestamp) override {};

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
            setLEDProgram (activeBlock->getLEDGrid());
        }
    }

    void timerCallback() override
    {
        if (doublePress)
        {
            // Clear the LED grid
            for (uint32 x = 0; x < 15; ++x)
                for (uint32 y = 0; y < 15; ++ y)
                    canvasProgram->setLED (x, y, Colours::black);

            // Clear the ActiveLED array
            activeLeds.clear();

            // Reset the doublePress flag
            doublePress = false;
        }
        else
        {
            // Switch to colour palette mode and set the LEDGrid program
            currentMode = colourPalette;
            setLEDProgram (activeBlock->getLEDGrid());
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
    void setLEDProgram (LEDGrid* grid)
    {
        if (currentMode == canvas)
        {
            // Create a new BitmapLEDProgram for the LEDGrid
            canvasProgram = new BitmapLEDProgram (*grid);

            // Set the LEDGrid program
            grid->setProgram (canvasProgram);

            // Redraw any previously drawn LEDs
            redrawLEDs();
        }
        else if (currentMode == colourPalette)
        {
            // Create a new DrumPadGridProgram for the LEDGrid
            colourPaletteProgram = new DrumPadGridProgram (*grid);

            // Set the LEDGrid program
            grid->setProgram (colourPaletteProgram);

            // Setup the grid layout
            colourPaletteProgram->setGridFills (layout.numColumns, layout.numRows, layout.gridFillArray);
        }
    }

    /** Sets an LED on the Lightpad for a given touch co-ordinate and pressure */
    void drawLEDs (uint32 x0, uint32 y0, float z, Colour drawColour)
    {
        // Check if the activeLeds array already contains an ActiveLED object for this LED
        int index = -1;
        for (int i = 0; i < activeLeds.size(); ++i)
        {
            if (activeLeds.getReference(i).occupies (x0, y0))
            {
                index = i;
                break;
            }
        }

        // If the colour is black then just set the LED to black and return
        if (drawColour == Colours::black)
        {
            if (index != -1)
            {
                canvasProgram->setLED (x0, y0, Colours::black);
                activeLeds.remove (index);
            }

            return;
        }

        // If there is no ActiveLED obejct for this LED then create one,
        // add it to the array, set the LED on the Block and return
        if (index == -1)
        {
            ActiveLED led;
            led.x = x0;
            led.y = y0;
            led.colour = drawColour;
            led.brightness = z;

            activeLeds.add (led);
            canvasProgram->setLED (led.x, led.y, led.colour.withBrightness (led.brightness));

            return;
        }

        // Get the ActiveLED object for this LED
        ActiveLED currentLed = activeLeds.getReference (index);

        // If the LED colour is the same as the draw colour, add the brightnesses together.
        // If it is different, blend the colours
        if (currentLed.colour == drawColour)
            currentLed.brightness = jmin (currentLed.brightness + z, 1.0f);
        else
            currentLed.colour = currentLed.colour.interpolatedWith (drawColour, z);


        // Set the LED on the Block and change the ActiveLED object in the activeLeds array
        canvasProgram->setLED (currentLed.x, currentLed.y, currentLed.colour.withBrightness (currentLed.brightness));
        activeLeds.set (index, currentLed);
    }

    /** Redraws the LEDs on the Lightpad from the activeLeds array */
    void redrawLEDs()
    {
        // Iterate over the activeLeds array and set the LEDs on the Block
        for (auto led : activeLeds)
            canvasProgram->setLED (led.x, led.y, led.colour.withBrightness (led.brightness));
    }

    /**
        A struct that represents an active LED on the Lightpad.
        Has a position, colour and brightness.
    */
    struct ActiveLED
    {
        uint32 x;
        uint32 y;
        Colour colour;
        float brightness;

        /** Returns true if this LED occupies the given co-ordiantes */
        bool occupies (uint32 xPos, uint32 yPos) const
        {
            if (xPos == x && yPos == y)
                return true;

            return false;
        }
    };
    Array<ActiveLED> activeLeds;

    /**
        enum for the two modes
    */
    enum DisplayMode
    {
        colourPalette = 0,
        canvas
    };
    DisplayMode currentMode = colourPalette;

    //==============================================================================
    BitmapLEDProgram* canvasProgram;
    DrumPadGridProgram* colourPaletteProgram;

    ColourGrid layout;
    PhysicalTopologySource topologySource;
    Block::Ptr activeBlock;

    float scaleX = 0.0;
    float scaleY = 0.0;

    bool doublePress = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
