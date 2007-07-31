/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_POINTCOMPONENT_JUCEHEADER__
#define __JUCER_POINTCOMPONENT_JUCEHEADER__

#include "jucer_ElementSiblingComponent.h"
#include "../../ui/jucer_PaintRoutineEditor.h"


//==============================================================================
/**
*/
class PointComponent    : public ElementSiblingComponent
{
public:
    //==============================================================================
    PointComponent (PaintElement* const owner_)
        : ElementSiblingComponent (owner_)
    {
        setSize (11, 11);
        setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
    }

    ~PointComponent()
    {
    }

    virtual const RelativePositionedRectangle getPosition() = 0;
    virtual void setPosition (const RelativePositionedRectangle& newPos) = 0;

    virtual void updatePosition()
    {
        if (dynamic_cast <PaintRoutineEditor*> (getParentComponent()) != 0)
        {
            const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

            const Rectangle r (getPosition().getRectangle (area, owner->getDocument()->getComponentLayout()));

            setCentrePosition (r.getX(), r.getY());
        }
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        g.setColour (Colours::white);
        g.drawEllipse (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, 2.0f);

        g.setColour (Colours::black);
        g.drawEllipse (1.0f, 1.0f, getWidth() - 2.0f, getHeight() - 2.0f, 2.0f);
    }

    //==============================================================================
    void mouseDown (const MouseEvent& e)
    {
        const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
        dragX = getX() + getWidth() / 2 - area.getX();
        dragY = getY() + getHeight() / 2 - area.getY();
    }

    void mouseDrag (const MouseEvent& e)
    {
        const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
        int x = dragX + e.getDistanceFromDragStartX();
        int y = dragY + e.getDistanceFromDragStartY();

        JucerDocument* const document = owner->getDocument();

        if (document != 0)
        {
            x = document->snapPosition (x);
            y = document->snapPosition (y);
        }

        const RelativePositionedRectangle original (getPosition());
        RelativePositionedRectangle pr (original);

        Rectangle r (pr.getRectangle (Rectangle (0, 0, area.getWidth(), area.getHeight()),
                                      document->getComponentLayout()));
        r.setPosition (x, y);

        pr.updateFrom (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                       Rectangle (0, 0, area.getWidth(), area.getHeight()),
                       document->getComponentLayout());

        if (pr != original)
            setPosition (pr);
    }

    void mouseUp (const MouseEvent& e)
    {
    }

private:
    int dragX, dragY;
};


#endif   // __JUCER_POINTCOMPONENT_JUCEHEADER__
