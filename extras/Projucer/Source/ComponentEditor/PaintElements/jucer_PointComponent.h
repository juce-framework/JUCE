/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ElementSiblingComponent.h"
#include "../UI/jucer_PaintRoutineEditor.h"

//==============================================================================
class PointComponent    : public ElementSiblingComponent
{
public:
    PointComponent (PaintElement* const e)
        : ElementSiblingComponent (e)
    {
        setSize (11, 11);
        setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
    }

    virtual RelativePositionedRectangle getPosition() = 0;
    virtual void setPosition (const RelativePositionedRectangle& newPos) = 0;

    void updatePosition() override
    {
        if (dynamic_cast<PaintRoutineEditor*> (getParentComponent()) != nullptr)
        {
            const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
            const Rectangle<int> r (getPosition().getRectangle (area, owner->getDocument()->getComponentLayout()));

            setCentrePosition (r.getX(), r.getY());
        }
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.setColour (Colours::white);
        g.drawEllipse (2.0f, 2.0f, (float) getWidth() - 4.0f, (float) getHeight() - 4.0f, 2.0f);

        g.setColour (Colours::black);
        g.drawEllipse (1.0f, 1.0f, (float) getWidth() - 2.0f, (float) getHeight() - 2.0f, 2.0f);
    }

    //==============================================================================
    void mouseDown (const MouseEvent&) override
    {
        const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
        dragX = getX() + getWidth() / 2 - area.getX();
        dragY = getY() + getHeight() / 2 - area.getY();
    }

    void mouseDrag (const MouseEvent& e) override
    {
        const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
        int x = dragX + e.getDistanceFromDragStartX();
        int y = dragY + e.getDistanceFromDragStartY();

        if (JucerDocument* const document = owner->getDocument())
        {
            x = document->snapPosition (x);
            y = document->snapPosition (y);

            const RelativePositionedRectangle original (getPosition());
            RelativePositionedRectangle pr (original);

            Rectangle<int> r (pr.getRectangle (Rectangle<int> (0, 0, area.getWidth(), area.getHeight()),
                                               document->getComponentLayout()));
            r.setPosition (x, y);

            pr.updateFrom (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                           Rectangle<int> (0, 0, area.getWidth(), area.getHeight()),
                           document->getComponentLayout());

            if (pr != original)
                setPosition (pr);
        }
    }

    void mouseUp (const MouseEvent&) override
    {
    }

private:
    int dragX, dragY;
};
