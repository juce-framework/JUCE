/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
#define __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__

#include "juce_ComponentBoundsConstrainer.h"


//==============================================================================
/**
    A component that resizes its parent component when dragged.

    This component forms a frame around the edge of a component, allowing it to
    be dragged by the edges or corners to resize it - like the way windows are
    resized in MSWindows or Linux.

    To use it, just add it to your component, making it fill the entire parent component
    (there's a mouse hit-test that only traps mouse-events which land around the
    edge of the component, so it's even ok to put it on top of any other components
    you're using). Make sure you rescale the resizer component to fill the parent
    each time the parent's size changes.

    @see ResizableCornerComponent
*/
class JUCE_API  ResizableBorderComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a resizer.

        Pass in the target component which you want to be resized when this one is
        dragged.

        The target component will usually be a parent of the resizer component, but this
        isn't mandatory.

        Remember that when the target component is resized, it'll need to move and
        resize this component to keep it in place, as this won't happen automatically.

        If the constrainer parameter is non-zero, then this object will be used to enforce
        limits on the size and position that the component can be stretched to. Make sure
        that the constrainer isn't deleted while still in use by this object.

        @see ComponentBoundsConstrainer
    */
    ResizableBorderComponent (Component* componentToResize,
                              ComponentBoundsConstrainer* constrainer);

    /** Destructor. */
    ~ResizableBorderComponent();


    //==============================================================================
    /** Specifies how many pixels wide the draggable edges of this component are.

        @see getBorderThickness
    */
    void setBorderThickness (const BorderSize<int>& newBorderSize);

    /** Returns the number of pixels wide that the draggable edges of this component are.

        @see setBorderThickness
    */
    BorderSize<int> getBorderThickness() const;


    //==============================================================================
    /** Represents the different sections of a resizable border, which allow it to
        resized in different ways.
    */
    class Zone
    {
    public:
        //==============================================================================
        enum Zones
        {
            centre  = 0,
            left    = 1,
            top     = 2,
            right   = 4,
            bottom  = 8
        };

        //==============================================================================
        /** Creates a Zone from a combination of the flags in \enum Zones. */
        explicit Zone (int zoneFlags = 0) noexcept;
        Zone (const Zone& other) noexcept;
        Zone& operator= (const Zone& other) noexcept;

        bool operator== (const Zone& other) const noexcept;
        bool operator!= (const Zone& other) const noexcept;

        //==============================================================================
        /** Given a point within a rectangle with a resizable border, this returns the
            zone that the point lies within.
        */
        static const Zone fromPositionOnBorder (const Rectangle<int>& totalSize,
                                                const BorderSize<int>& border,
                                                const Point<int>& position);

        /** Returns an appropriate mouse-cursor for this resize zone. */
        MouseCursor getMouseCursor() const noexcept;

        /** Returns true if dragging this zone will move the enire object without resizing it. */
        bool isDraggingWholeObject() const noexcept     { return zone == centre; }
        /** Returns true if dragging this zone will move the object's left edge. */
        bool isDraggingLeftEdge() const noexcept        { return (zone & left) != 0; }
        /** Returns true if dragging this zone will move the object's right edge. */
        bool isDraggingRightEdge() const noexcept       { return (zone & right) != 0; }
        /** Returns true if dragging this zone will move the object's top edge. */
        bool isDraggingTopEdge() const noexcept         { return (zone & top) != 0; }
        /** Returns true if dragging this zone will move the object's bottom edge. */
        bool isDraggingBottomEdge() const noexcept      { return (zone & bottom) != 0; }

        /** Resizes this rectangle by the given amount, moving just the edges that this zone
            applies to.
        */
        template <typename ValueType>
        const Rectangle<ValueType> resizeRectangleBy (Rectangle<ValueType> original,
                                                      const Point<ValueType>& distance) const noexcept
        {
            if (isDraggingWholeObject())
                return original + distance;

            if (isDraggingLeftEdge())
                original.setLeft (jmin (original.getRight(), original.getX() + distance.x));

            if (isDraggingRightEdge())
                original.setWidth (jmax (ValueType(), original.getWidth() + distance.x));

            if (isDraggingTopEdge())
                original.setTop (jmin (original.getBottom(), original.getY() + distance.y));

            if (isDraggingBottomEdge())
                original.setHeight (jmax (ValueType(), original.getHeight() + distance.y));

            return original;
        }

        /** Returns the raw flags for this zone. */
        int getZoneFlags() const noexcept               { return zone; }

    private:
        //==============================================================================
        int zone;
    };


protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void mouseEnter (const MouseEvent& e);
    /** @internal */
    void mouseMove (const MouseEvent& e);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    bool hitTest (int x, int y);

private:
    WeakReference<Component> component;
    ComponentBoundsConstrainer* constrainer;
    BorderSize<int> borderSize;
    Rectangle<int> originalBounds;
    Zone mouseZone;

    void updateMouseZone (const MouseEvent& e);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizableBorderComponent)
};


#endif   // __JUCE_RESIZABLEBORDERCOMPONENT_JUCEHEADER__
