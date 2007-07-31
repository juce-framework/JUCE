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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ResizableBorderComponent.h"
#include "../juce_Desktop.h"
#include "../../graphics/geometry/juce_RectangleList.h"
#include "../../graphics/geometry/juce_Line.h"
#include "../lookandfeel/juce_LookAndFeel.h"

const int zoneL = 1;
const int zoneR = 2;
const int zoneT = 4;
const int zoneB = 8;

//==============================================================================
ResizableBorderComponent::ResizableBorderComponent (Component* const componentToResize,
                                                    ComponentBoundsConstrainer* const constrainer_)
   : component (componentToResize),
     constrainer (constrainer_),
     borderSize (5),
     mouseZone (0)
{
}

ResizableBorderComponent::~ResizableBorderComponent()
{
}

//==============================================================================
void ResizableBorderComponent::paint (Graphics& g)
{
    getLookAndFeel().drawResizableFrame (g, getWidth(), getHeight(), borderSize);
}

void ResizableBorderComponent::mouseEnter (const MouseEvent& e)
{
    updateMouseZone (e);
}

void ResizableBorderComponent::mouseMove (const MouseEvent& e)
{
    updateMouseZone (e);
}

void ResizableBorderComponent::mouseDown (const MouseEvent& e)
{
    if (component->isValidComponent())
    {
        updateMouseZone (e);

        originalX = component->getX();
        originalY = component->getY();
        originalW = component->getWidth();
        originalH = component->getHeight();

        if (constrainer != 0)
            constrainer->resizeStart();
    }
    else
    {
        jassertfalse
    }
}

void ResizableBorderComponent::mouseDrag (const MouseEvent& e)
{
    if (! component->isValidComponent())
    {
        jassertfalse
        return;
    }

    int x = originalX;
    int y = originalY;
    int w = originalW;
    int h = originalH;

    const int dx = e.getDistanceFromDragStartX();
    const int dy = e.getDistanceFromDragStartY();

    if ((mouseZone & zoneL) != 0)
    {
        x += dx;
        w -= dx;
    }

    if ((mouseZone & zoneT) != 0)
    {
        y += dy;
        h -= dy;
    }

    if ((mouseZone & zoneR) != 0)
        w += dx;

    if ((mouseZone & zoneB) != 0)
        h += dy;

    if (constrainer != 0)
        constrainer->setBoundsForComponent (component,
                                            x, y, w, h,
                                            (mouseZone & zoneT) != 0,
                                            (mouseZone & zoneL) != 0,
                                            (mouseZone & zoneB) != 0,
                                            (mouseZone & zoneR) != 0);
    else
        component->setBounds (x, y, w, h);
}

void ResizableBorderComponent::mouseUp (const MouseEvent&)
{
    if (constrainer != 0)
        constrainer->resizeEnd();
}

bool ResizableBorderComponent::hitTest (int x, int y)
{
    return x < borderSize.getLeft()
            || x >= getWidth() - borderSize.getRight()
            || y < borderSize.getTop()
            || y >= getHeight() - borderSize.getBottom();
}

void ResizableBorderComponent::setBorderThickness (const BorderSize& newBorderSize) throw()
{
    if (borderSize != newBorderSize)
    {
        borderSize = newBorderSize;
        repaint();
    }
}

const BorderSize ResizableBorderComponent::getBorderThickness() const throw()
{
    return borderSize;
}

void ResizableBorderComponent::updateMouseZone (const MouseEvent& e) throw()
{
    int newZone = 0;

    if (ResizableBorderComponent::hitTest (e.x, e.y))
    {
        if (e.x < jmax (borderSize.getLeft(), proportionOfWidth (0.1f)))
            newZone |= zoneL;
        else if (e.x >= jmin (getWidth() - borderSize.getRight(), proportionOfWidth (0.9f)))
            newZone |= zoneR;

        if (e.y < jmax (borderSize.getTop(), proportionOfHeight (0.1f)))
            newZone |= zoneT;
        else if (e.y >= jmin (getHeight() - borderSize.getBottom(), proportionOfHeight (0.9f)))
            newZone |= zoneB;
    }

    if (mouseZone != newZone)
    {
        mouseZone = newZone;

        MouseCursor::StandardCursorType mc = MouseCursor::NormalCursor;

        switch (newZone)
        {
        case (zoneL | zoneT):
            mc = MouseCursor::TopLeftCornerResizeCursor;
            break;

        case zoneT:
            mc = MouseCursor::TopEdgeResizeCursor;
            break;

        case (zoneR | zoneT):
            mc = MouseCursor::TopRightCornerResizeCursor;
            break;

        case zoneL:
            mc = MouseCursor::LeftEdgeResizeCursor;
            break;

        case zoneR:
            mc = MouseCursor::RightEdgeResizeCursor;
            break;

        case (zoneL | zoneB):
            mc = MouseCursor::BottomLeftCornerResizeCursor;
            break;

        case zoneB:
            mc = MouseCursor::BottomEdgeResizeCursor;
            break;

        case (zoneR | zoneB):
            mc = MouseCursor::BottomRightCornerResizeCursor;
            break;

        default:
            break;
        }

        setMouseCursor (mc);
    }
}

END_JUCE_NAMESPACE
