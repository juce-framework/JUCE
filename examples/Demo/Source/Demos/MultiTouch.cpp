/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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

        t->pushPoint (e.position, e.mods);
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

        void pushPoint (Point<float> p, ModifierKeys newMods)
        {
            currentPosition = p;
            modifierKeys = newMods;

            if (lastPoint.getDistanceFrom(p) > 5.0f)
            {
                if (lastPoint != Point<float>())
                {
                    path.quadraticTo (lastPoint, p);
                    lastPoint = Point<float>();
                }
                else
                {
                    lastPoint = p;
                }
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
        g.strokePath (trail.path, PathStrokeType (20.0f, PathStrokeType::curved, PathStrokeType::rounded));

        const float radius = 40.0f;

        g.setColour (Colours::black);
        g.drawEllipse (trail.currentPosition.x - radius,
                       trail.currentPosition.y - radius,
                       radius * 2.0f, radius * 2.0f, 2.0f);

        g.setFont (14.0f);

        String desc ("Mouse #");
        desc << trail.source.getIndex();

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