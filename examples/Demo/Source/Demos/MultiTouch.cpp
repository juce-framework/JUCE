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

#include "../JuceDemoHeader.h"


//==============================================================================
class MultiTouchDemo    : public Component
{
public:
    MultiTouchDemo()
    {
        setOpaque (true);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colour::greyLevel (0.4f)));

        g.setColour (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::defaultText,
                                             Colours::lightgrey));
        g.setFont (14.0f);
        g.drawFittedText ("Drag here with as many fingers as you have!",
                          getLocalBounds().reduced (30), Justification::centred, 4);

        for (int i = 0; i < trails.size(); ++i)
            drawTrail (*trails.getUnchecked(i), g);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        Trail* t = getTrail (e.source);

        if (t == nullptr)
        {
            t = new Trail (e.source);
            t->path.startNewSubPath (e.position);
            trails.add (t);
        }

        t->pushPoint (e.position, e.mods, e.pressure);
        repaint();
    }

    void mouseUp (const MouseEvent& e) override
    {
        trails.removeObject (getTrail (e.source));
        repaint();
    }

    struct Trail
    {
        Trail (const MouseInputSource& ms)
            : source (ms), colour (getRandomBrightColour().withAlpha (0.6f))
        {}

        void pushPoint (Point<float> newPoint, ModifierKeys newMods, float pressure)
        {
            currentPosition = newPoint;
            modifierKeys = newMods;

            if (lastPoint.getDistanceFrom (newPoint) > 5.0f)
            {
                if (lastPoint != Point<float>())
                {
                    Path newSegment;
                    newSegment.startNewSubPath (lastPoint);
                    newSegment.lineTo (newPoint);

                    float diameter = 20.0f * (pressure > 0 && pressure < 1.0f ? pressure : 1.0f);

                    PathStrokeType (diameter, PathStrokeType::curved, PathStrokeType::rounded).createStrokedPath (newSegment, newSegment);
                    path.addPath (newSegment);
                }

                lastPoint = newPoint;
            }
        }

        MouseInputSource source;
        Path path;
        Colour colour;
        Point<float> lastPoint, currentPosition;
        ModifierKeys modifierKeys;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Trail)
    };

    OwnedArray<Trail> trails;

    void drawTrail (Trail& trail, Graphics& g)
    {
        g.setColour (trail.colour);
        g.fillPath (trail.path);

        const float radius = 40.0f;

        g.setColour (Colours::black);
        g.drawEllipse (trail.currentPosition.x - radius,
                       trail.currentPosition.y - radius,
                       radius * 2.0f, radius * 2.0f, 2.0f);

        g.setFont (14.0f);

        String desc ("Mouse #");
        desc << trail.source.getIndex();

        float pressure = trail.source.getCurrentPressure();

        if (pressure > 0.0f && pressure < 1.0f)
            desc << "  (pressure: " << (int) (pressure * 100.0f) << "%)";

        if (trail.modifierKeys.isCommandDown()) desc << " (CMD)";
        if (trail.modifierKeys.isShiftDown())   desc << " (SHIFT)";
        if (trail.modifierKeys.isCtrlDown())    desc << " (CTRL)";
        if (trail.modifierKeys.isAltDown())     desc << " (ALT)";

        g.drawText (desc,
                    Rectangle<int> ((int) trail.currentPosition.x - 200,
                                    (int) trail.currentPosition.y - 60,
                                    400, 20),
                    Justification::centredTop, false);
    }

    Trail* getTrail (const MouseInputSource& source)
    {
        for (int i = 0; i < trails.size(); ++i)
        {
            Trail* t = trails.getUnchecked(i);

            if (t->source == source)
                return t;
        }

        return nullptr;
    }
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<MultiTouchDemo> demo ("10 Components: Multi-touch");
