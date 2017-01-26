#ifndef LIGHTPADCOMPONENT_H_INCLUDED
#define LIGHTPADCOMPONENT_H_INCLUDED

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
class LightpadComponent : public Component
{
public:
    LightpadComponent ()
    {
        for (int x = 0; x < 15; ++x)
            for (int y = 0; y < 15; ++y)
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
        Rectangle<int> r = getLocalBounds().reduced (10);

        int circleWidth = r.getWidth() / 15;
        int circleHeight = r.getHeight() / 15;

        for (int x = 0; x < 15; ++x)
            for (int y = 0; y < 15; ++y)
                leds.getUnchecked ((x * 15) + y)->setBounds (r.getX() + (x * circleWidth),
                                                             r.getY() + (y * circleHeight),
                                                             circleWidth, circleHeight);
    }

    void mouseDown (const MouseEvent& e) override
    {
        for (int x = 0; x < 15; ++x)
        {
            for (int y = 0; y < 15; ++y)
            {
                if (leds.getUnchecked ((x * 15) + y)->getBounds().contains (e.position.toInt()))
                {
                    listeners.call (&Listener::ledClicked, x, y, e.pressure);
                }
            }
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        for (int x = 0; x < 15; ++x)
        {
            for (int y = 0; y < 15; ++y)
            {
                if (leds.getUnchecked ((x * 15) + y)->getBounds().contains (e.position.toInt()))
                {
                    const Time t = e.eventTime;

                    if (lastLED == Point<int> (x, y) && t.toMilliseconds() - lastMouseEventTime.toMilliseconds() < 50)
                        return;

                    listeners.call (&Listener::ledClicked, x, y, e.pressure);

                    lastLED = Point<int> (x, y);
                    lastMouseEventTime = t;
                }
            }
        }
    }

    //==============================================================================
    /** Sets the colour of one of the LEDComponents */
    void setLEDColour (int x, int y, Colour c)
    {
        jassert (isPositiveAndBelow (x, 15) && isPositiveAndBelow (y, 15));

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


#endif  // LIGHTPADCOMPONENT_H_INCLUDED
