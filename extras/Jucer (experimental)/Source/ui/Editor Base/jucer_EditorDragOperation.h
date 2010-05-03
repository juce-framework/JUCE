/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_EDITORDRAGOPERATION_H_720CD068__
#define __JUCER_EDITORDRAGOPERATION_H_720CD068__

#include "jucer_EditorCanvas.h"


//==============================================================================
class EditorDragOperation  : public EditorCanvasBase::DragOperation
{
public:
    EditorDragOperation (EditorCanvasBase* canvas_, const MouseEvent& e,
                         Component* snapGuideParentComp_,
                         const ResizableBorderComponent::Zone& zone_)
        : canvas (canvas_),
          snapGuideParentComp (snapGuideParentComp_),
          zone (zone_),
          mouseDownPos (e.getPosition())
    {
    }

    ~EditorDragOperation()
    {
    }

    void initialise (const Array<ValueTree>& objects,
                     const Array<ValueTree>& objectsToSnapTo)
    {
        draggedObjects = objects;

        int i;
        for (i = 0; i < objects.size(); ++i)
        {
            const Rectangle<float> floatPos (getObjectPosition (objects.getReference(i)));
            originalPositions.add (floatPos);

            if (zone.isDraggingWholeObject() || zone.isDraggingLeftEdge())
                verticalSnapPositions.add (SnapLine (floatPos.getX(), floatPos.getY(), floatPos.getBottom()));

            if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                verticalSnapPositions.add (SnapLine (floatPos.getCentreX(), floatPos.getY(), floatPos.getBottom()));

            if (zone.isDraggingWholeObject() || zone.isDraggingRightEdge())
                verticalSnapPositions.add (SnapLine (floatPos.getRight(), floatPos.getY(), floatPos.getBottom()));

            if (zone.isDraggingWholeObject() || zone.isDraggingTopEdge())
                horizontalSnapPositions.add (SnapLine (floatPos.getY(), floatPos.getX(), floatPos.getRight()));

            if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                horizontalSnapPositions.add (SnapLine (floatPos.getCentreY(), floatPos.getX(), floatPos.getRight()));

            if (zone.isDraggingWholeObject() || zone.isDraggingBottomEdge())
                horizontalSnapPositions.add (SnapLine (floatPos.getBottom(), floatPos.getX(), floatPos.getRight()));
        }

        if (isDraggingLeftRight())
        {
            verticalSnapTargets.add (SnapLine (0, -100.0f, 10000.0f));
            verticalSnapTargets.add (SnapLine ((float) getCanvasWidth(), -100.0f, 10000.0f));

            if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                verticalSnapTargets.add (SnapLine ((float) getCanvasWidth() / 2.0f, 0, 10000.0f));
        }

        if (isDraggingUpDown())
        {
            horizontalSnapTargets.add (SnapLine (0, -100.0f, 10000.0f));
            horizontalSnapTargets.add (SnapLine ((float) getCanvasHeight(), -100.0f, 10000.0f));

            if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                horizontalSnapTargets.add (SnapLine ((float) getCanvasHeight() / 2.0f, 0, 10000.0f));
        }

        for (i = 0; i < objectsToSnapTo.size(); ++i)
        {
            const Rectangle<float> floatPos (getObjectPosition (objectsToSnapTo.getReference (i)));

            if (isDraggingLeftRight())
            {
                verticalSnapTargets.add (SnapLine (floatPos.getX(), floatPos.getY(), floatPos.getBottom()));
                verticalSnapTargets.add (SnapLine (floatPos.getRight(), floatPos.getY(), floatPos.getBottom()));
            }

            if (zone.isDraggingWholeObject() || (zone.isDraggingLeftEdge() && zone.isDraggingRightEdge()))
                verticalSnapTargets.add (SnapLine (floatPos.getCentreX(), floatPos.getY(), floatPos.getBottom()));

            if (isDraggingUpDown())
            {
                horizontalSnapTargets.add (SnapLine (floatPos.getY(), floatPos.getX(), floatPos.getRight()));
                horizontalSnapTargets.add (SnapLine (floatPos.getBottom(), floatPos.getX(), floatPos.getRight()));
            }

            if (zone.isDraggingWholeObject() || (zone.isDraggingTopEdge() && zone.isDraggingBottomEdge()))
                horizontalSnapTargets.add (SnapLine (floatPos.getCentreY(), floatPos.getX(), floatPos.getRight()));
        }

        mergeSnapLines (verticalSnapTargets);
        mergeSnapLines (horizontalSnapTargets);

        getUndoManager().beginNewTransaction();
    }

    //==============================================================================
    struct SnapLine
    {
        SnapLine() : position (0), start (0), end (0)  {}

        SnapLine (const float position_, const float start_, const float end_)
            : position (position_), start (start_), end (end_)
        {}

        float position, start, end;
    };

    //==============================================================================
    class AlignmentHintComponent  : public EditorCanvasBase::OverlayItemComponent
    {
    public:
        AlignmentHintComponent (EditorCanvasBase* canvas_, const SnapLine& line_, bool isVertical_, Component* parent)
            : OverlayItemComponent (canvas_), line (line_), isVertical (isVertical_)
        {
            const int extraEndLength = 5;
            setAlwaysOnTop (true);
            parent->addAndMakeVisible (this);

            if (isVertical)
                setBoundsInTargetSpace (Rectangle<int> (roundToInt (line.position), roundToInt (line.start) - extraEndLength,
                                                        1, roundToInt (line.end - line.start) + extraEndLength * 2));
            else
                setBoundsInTargetSpace (Rectangle<int> (roundToInt (line.start) - extraEndLength, roundToInt (line.position),
                                                        roundToInt (line.end - line.start) + extraEndLength * 2, 1));
        }

        void paint (Graphics& g)
        {
            g.fillAll (alignmentMarkerColour);
        }

    private:
        const SnapLine line;
        const bool isVertical;

        AlignmentHintComponent (const AlignmentHintComponent&);
        AlignmentHintComponent& operator= (const AlignmentHintComponent&);
    };

    //==============================================================================
    void drag (const MouseEvent& e)
    {
        getUndoManager().undoCurrentTransactionOnly();

        // (can't use getOffsetFromDragStart() because of auto-scrolling)
        Point<int> distance (e.getPosition() - mouseDownPos);
        if (! isDraggingLeftRight())
            distance = distance.withX (0);

        if (! isDraggingUpDown())
            distance = distance.withY (0);

        snapGuides.clear();

        performSnap (verticalSnapTargets, getVerticalSnapPositions (distance), true, distance);
        performSnap (horizontalSnapTargets, getHorizontalSnapPositions (distance), false, distance);

        for (int n = 50;;)
        {
            // Need to repeatedly apply the new positions until they all settle down, in case some of
            // the coords are relative to each other..
            bool anyUpdated = false;

            for (int i = 0; i < draggedObjects.size(); ++i)
                if (dragItem (draggedObjects.getReference(i), distance, originalPositions.getReference(i)))
                    anyUpdated = true;

            if (! anyUpdated)
                break;

            if (--n == 0)
            {
                jassertfalse;
                break;
            }
        }
    }

    bool dragItem (ValueTree& v, const Point<int>& distance, const Rectangle<float>& originalPos)
    {
        const Rectangle<float> newBounds (zone.resizeRectangleBy (originalPos, Point<float> ((float) distance.getX(),
                                                                                             (float) distance.getY())));
        return setObjectPosition (v, newBounds);
    }

protected:
    //==============================================================================
    virtual int getCanvasWidth() = 0;
    virtual int getCanvasHeight() = 0;

    virtual const Rectangle<float> getObjectPosition (const ValueTree& state) = 0;
    virtual bool setObjectPosition (ValueTree& state, const Rectangle<float>& newBounds) = 0;

    virtual UndoManager& getUndoManager() = 0;

    EditorCanvasBase* canvas;

private:
    //==============================================================================
    Array <ValueTree> draggedObjects;
    Array <Rectangle<float> > originalPositions;
    Array <SnapLine> verticalSnapPositions, horizontalSnapPositions;
    Array <SnapLine> verticalSnapTargets, horizontalSnapTargets;
    const ResizableBorderComponent::Zone zone;
    OwnedArray<Component> snapGuides;
    Component* snapGuideParentComp;
    Point<int> mouseDownPos;

    static void mergeSnapLines (Array <SnapLine>& lines)
    {
        for (int i = lines.size(); --i > 0;)
        {
            SnapLine& s1 = lines.getReference(i);

            for (int j = i; --j >= 0;)
            {
                SnapLine& s2 = lines.getReference(j);

                if (s1.position == s2.position)
                {
                    s2.start = jmin (s1.start, s2.start);
                    s2.end = jmax (s1.end, s2.end);
                    lines.remove (i);
                }
            }
        }
    }

    void performSnap (const Array<SnapLine>& targets, const Array<SnapLine>& sources, bool isVertical, Point<int>& distance)
    {
        if (targets.size() == 0 || sources.size() == 0)
            return;

        float best = std::numeric_limits<float>::max();
        float absBest = fabsf (best);
        Array <SnapLine> lines;

        for (int i = 0; i < targets.size(); ++i)
        {
            const SnapLine& target = targets.getReference(i);

            for (int j = 0; j < sources.size(); ++j)
            {
                const SnapLine& source = sources.getReference(j);
                const float diff = target.position - source.position;
                const float absDiff = fabsf (diff);

                if (absDiff <= absBest)
                {
                    if (absDiff < absBest)
                        lines.clearQuick();

                    lines.add (SnapLine (target.position, jmin (target.start, source.start), jmax (target.end, source.end)));
                    best = diff;
                    absBest = absDiff;
                }
            }
        }

        jassert (absBest < std::numeric_limits<float>::max());

        if (absBest < snapDistance)
        {
            distance += isVertical ? Point<int> (roundToInt (best), 0) : Point<int> (0, roundToInt (best));

            for (int i = lines.size(); --i >= 0;)
                snapGuides.add (new AlignmentHintComponent (canvas, lines.getReference(i), isVertical, snapGuideParentComp));
        }
    }

    const Array<SnapLine> getVerticalSnapPositions (const Point<int>& distance) const
    {
        Array<SnapLine> p (verticalSnapPositions);
        for (int i = p.size(); --i >= 0;)
        {
            SnapLine& s = p.getReference(i);
            s.position += distance.getX();
            s.start += distance.getY();
            s.end += distance.getY();
        }

        return p;
    }

    const Array<SnapLine> getHorizontalSnapPositions (const Point<int>& distance) const
    {
        Array<SnapLine> p (horizontalSnapPositions);
        for (int i = p.size(); --i >= 0;)
        {
            SnapLine& s = p.getReference(i);
            s.position += distance.getY();
            s.start += distance.getX();
            s.end += distance.getX();
        }

        return p;
    }

    bool isDraggingLeftRight() const        { return zone.isDraggingWholeObject() || zone.isDraggingLeftEdge() || zone.isDraggingRightEdge(); }
    bool isDraggingUpDown() const           { return zone.isDraggingWholeObject() || zone.isDraggingTopEdge() || zone.isDraggingBottomEdge(); }

    EditorDragOperation (const EditorDragOperation&);
    EditorDragOperation& operator= (const EditorDragOperation&);
};


#endif  // __JUCER_EDITORDRAGOPERATION_H_720CD068__
