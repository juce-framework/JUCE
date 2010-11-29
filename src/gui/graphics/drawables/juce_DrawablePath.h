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

#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
#define __JUCE_DRAWABLEPATH_JUCEHEADER__

#include "juce_DrawableShape.h"


//==============================================================================
/**
    A drawable object which renders a filled or outlined shape.

    For details on how to change the fill and stroke, see the DrawableShape class.

    @see Drawable, DrawableShape
*/
class JUCE_API  DrawablePath  : public DrawableShape
{
public:
    //==============================================================================
    /** Creates a DrawablePath. */
    DrawablePath();
    DrawablePath (const DrawablePath& other);

    /** Destructor. */
    ~DrawablePath();

    //==============================================================================
    /** Changes the path that will be drawn.
        @see setFillColour, setStrokeType
    */
    void setPath (const Path& newPath);

    /** Returns the current path. */
    const Path& getPath() const;

    /** Returns the current path for the outline. */
    const Path& getStrokePath() const;

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const       { return valueTreeType; }

    //==============================================================================
    /** Internally-used class for wrapping a DrawablePath's state into a ValueTree. */
    class ValueTreeWrapper   : public DrawableShape::FillAndStrokeState
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        bool usesNonZeroWinding() const;
        void setUsesNonZeroWinding (bool b, UndoManager* undoManager);

        class Element
        {
        public:
            explicit Element (const ValueTree& state);
            ~Element();

            const Identifier getType() const throw()    { return state.getType(); }
            int getNumControlPoints() const throw();

            const RelativePoint getControlPoint (int index) const;
            Value getControlPointValue (int index, UndoManager* undoManager) const;
            const RelativePoint getStartPoint() const;
            const RelativePoint getEndPoint() const;
            void setControlPoint (int index, const RelativePoint& point, UndoManager* undoManager);
            float getLength (Expression::EvaluationContext* nameFinder) const;

            ValueTreeWrapper getParent() const;
            Element getPreviousElement() const;

            const String getModeOfEndPoint() const;
            void setModeOfEndPoint (const String& newMode, UndoManager* undoManager);

            void convertToLine (UndoManager* undoManager);
            void convertToCubic (Expression::EvaluationContext* nameFinder, UndoManager* undoManager);
            void convertToPathBreak (UndoManager* undoManager);
            ValueTree insertPoint (const Point<float>& targetPoint, Expression::EvaluationContext* nameFinder, UndoManager* undoManager);
            void removePoint (UndoManager* undoManager);
            float findProportionAlongLine (const Point<float>& targetPoint, Expression::EvaluationContext* nameFinder) const;

            static const Identifier mode, startSubPathElement, closeSubPathElement,
                                    lineToElement, quadraticToElement, cubicToElement;
            static const char* cornerMode;
            static const char* roundedMode;
            static const char* symmetricMode;

            ValueTree state;
        };

        ValueTree getPathState();

        static const Identifier nonZeroWinding, point1, point2, point3;
    };

protected:
    bool rebuildPath (Path& path) const;

private:
    //==============================================================================
    ScopedPointer<RelativePointPath> relativePath;

    DrawablePath& operator= (const DrawablePath&);
    JUCE_LEAK_DETECTOR (DrawablePath);
};


#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
