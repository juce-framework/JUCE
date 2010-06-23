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

#include "juce_Drawable.h"
#include "../colour/juce_ColourGradient.h"
#include "../contexts/juce_FillType.h"


//==============================================================================
/**
    A drawable object which renders a filled or outlined shape.

    @see Drawable
*/
class JUCE_API  DrawablePath  : public Drawable
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

    /** Sets a fill type for the path.

        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this to a transparent
        colour.

        @see setPath, setStrokeFill
    */
    void setFill (const FillType& newFill);

    /** Returns the current fill type.
        @see setFill
    */
    const FillType& getFill() const throw()                     { return mainFill; }

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    void setStrokeFill (const FillType& newStrokeFill);

    /** Returns the current stroke fill.
        @see setStrokeFill
    */
    const FillType& getStrokeFill() const throw()               { return strokeFill; }

    /** Changes the properties of the outline that will be drawn around the path.
        If the stroke has 0 thickness, no stroke will be drawn.
        @see setStrokeThickness, setStrokeColour
    */
    void setStrokeType (const PathStrokeType& newStrokeType);

    /** Changes the stroke thickness.
        This is a shortcut for calling setStrokeType.
    */
    void setStrokeThickness (float newThickness);

    /** Returns the current outline style. */
    const PathStrokeType& getStrokeType() const throw()         { return strokeType; }


    //==============================================================================
    /** Returns the current path. */
    const Path& getPath() const;

    /** Returns the current path for the outline. */
    const Path& getStrokePath() const;

    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    const Rectangle<float> getBounds() const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void invalidatePoints();
    /** @internal */
    const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const       { return valueTreeType; }

    //==============================================================================
    /** Internally-used class for wrapping a DrawablePath's state into a ValueTree. */
    class ValueTreeWrapper   : public ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        const FillType getMainFill (RelativeCoordinate::NamedCoordinateFinder* nameFinder,
                                    ImageProvider* imageProvider) const;
        ValueTree getMainFillState();
        void setMainFill (const FillType& newFill, const RelativePoint* gradientPoint1,
                          const RelativePoint* gradientPoint2, const RelativePoint* gradientPoint3,
                          ImageProvider* imageProvider, UndoManager* undoManager);

        const FillType getStrokeFill (RelativeCoordinate::NamedCoordinateFinder* nameFinder,
                                      ImageProvider* imageProvider) const;
        ValueTree getStrokeFillState();
        void setStrokeFill (const FillType& newFill, const RelativePoint* gradientPoint1,
                            const RelativePoint* gradientPoint2, const RelativePoint* gradientPoint3,
                            ImageProvider* imageProvider, UndoManager* undoManager);

        const PathStrokeType getStrokeType() const;
        void setStrokeType (const PathStrokeType& newStrokeType, UndoManager* undoManager);

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

            ValueTreeWrapper getParent() const;
            Element getPreviousElement() const;

            const String getModeOfEndPoint() const;
            void setModeOfEndPoint (const String& newMode, UndoManager* undoManager);

            void convertToLine (UndoManager* undoManager);
            void convertToCubic (RelativeCoordinate::NamedCoordinateFinder* nameFinder, UndoManager* undoManager);
            void convertToPathBreak (UndoManager* undoManager);
            void insertPoint (double proportionOfLength, RelativeCoordinate::NamedCoordinateFinder* nameFinder, UndoManager* undoManager);
            void removePoint (UndoManager* undoManager);

            static const Identifier mode, startSubPathElement, closeSubPathElement,
                                    lineToElement, quadraticToElement, cubicToElement;
            static const char* cornerMode;
            static const char* roundedMode;
            static const char* symmetricMode;

            ValueTree state;
        };

        ValueTree getPathState();

        static const Identifier fill, stroke, path, jointStyle, capStyle, strokeWidth,
                                nonZeroWinding, point1, point2, point3;
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FillType mainFill, strokeFill;
    PathStrokeType strokeType;
    ScopedPointer<RelativePointPath> relativePath;
    mutable Path path, stroke;
    mutable bool pathNeedsUpdating, strokeNeedsUpdating;

    void updatePath() const;
    void updateStroke() const;
    bool isStrokeVisible() const throw();

    DrawablePath& operator= (const DrawablePath&);
};


#endif   // __JUCE_DRAWABLEPATH_JUCEHEADER__
