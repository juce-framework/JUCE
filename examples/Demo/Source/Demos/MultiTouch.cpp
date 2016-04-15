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
        g.fillAll (Colour::greyLevel (0.4f));

        g.setColour (Colours::lightgrey);
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
