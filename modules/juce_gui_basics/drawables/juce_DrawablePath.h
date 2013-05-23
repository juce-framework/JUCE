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

#ifndef __JUCE_DRAWABLEPATH_JUCEHEADER__
#define __JUCE_DRAWABLEPATH_JUCEHEADER__

#include "juce_DrawableShape.h"
#include "../positioning/juce_RelativePointPath.h"


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

    /** Sets the path using a RelativePointPath.
        Calling this will set up a Component::Positioner to automatically update the path
        if any of the points in the source path are dynamic.
    */
    void setPath (const RelativePointPath& newPath);

    /** Returns the current path. */
    const Path& getPath() const;

    /** Returns the current path for the outline. */
    const Path& getStrokePath() const;

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;

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

            const Identifier getType() const noexcept   { return state.getType(); }
            int getNumControlPoints() const noexcept;

            RelativePoint getControlPoint (int index) const;
            Value getControlPointValue (int index, UndoManager*);
            RelativePoint getStartPoint() const;
            RelativePoint getEndPoint() const;
            void setControlPoint (int index, const RelativePoint& point, UndoManager*);
            float getLength (Expression::Scope*) const;

            ValueTreeWrapper getParent() const;
            Element getPreviousElement() const;

            String getModeOfEndPoint() const;
            void setModeOfEndPoint (const String& newMode, UndoManager*);

            void convertToLine (UndoManager*);
            void convertToCubic (Expression::Scope*, UndoManager*);
            void convertToPathBreak (UndoManager* undoManager);
            ValueTree insertPoint (Point<float> targetPoint, Expression::Scope*, UndoManager*);
            void removePoint (UndoManager* undoManager);
            float findProportionAlongLine (Point<float> targetPoint, Expression::Scope*) const;

            static const Identifier mode, startSubPathElement, closeSubPathElement,
                                    lineToElement, quadraticToElement, cubicToElement;
            static const char* cornerMode;
            static const char* roundedMode;
            static const char* symmetricMode;

            ValueTree state;
        };

        ValueTree getPathState();

        void readFrom (const RelativePointPath& path, UndoManager* undoManager);
        void writeTo (RelativePointPath& path) const;

        static const Identifier nonZeroWinding, point1, point2, point3;
    };

private:
    //==============================================================================
    ScopedPointer<RelativePointPath> relativePath;

    class RelativePositioner;
    friend class RelativePositioner;
    void applyRelativePath (const RelativePointPath&, Expression::Scope*);

    DrawablePath& operator= (const DrawablePath&);
    JUCE_LEAK_DETECTOR (DrawablePath)
};


#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
