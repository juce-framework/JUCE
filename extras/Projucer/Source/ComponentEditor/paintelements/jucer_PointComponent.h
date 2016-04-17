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

#ifndef JUCER_POINTCOMPONENT_H_INCLUDED
#define JUCER_POINTCOMPONENT_H_INCLUDED

#include "jucer_ElementSiblingComponent.h"
#include "../ui/jucer_PaintRoutineEditor.h"


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
        g.drawEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, 2.0f);

        g.setColour (Colours::black);
        g.drawEllipse (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 2.0f);
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


#endif   // JUCER_POINTCOMPONENT_H_INCLUDED
