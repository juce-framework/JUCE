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
        {
            for (auto y = 0; y < 15; ++y)
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
        for (auto x = 0; x < 15; ++x)
        {
            for (auto y = 0; y < 15; ++y)
            {
                if (leds.getUnchecked ((x * 15) + y)->getBounds().contains (e.position.toInt()))
                {
                    const auto t = e.eventTime;

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
