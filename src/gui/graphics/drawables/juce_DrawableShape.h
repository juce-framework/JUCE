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

#ifndef __JUCE_DRAWABLESHAPE_JUCEHEADER__
#define __JUCE_DRAWABLESHAPE_JUCEHEADER__

#include "juce_Drawable.h"
#include "../contexts/juce_FillType.h"
#include "../colour/juce_ColourGradient.h"


//==============================================================================
/**
    A base class implementing common functionality for Drawable classes which
    consist of some kind of filled and stroked outline.

    @see DrawablePath, DrawableRectangle
*/
class JUCE_API  DrawableShape   : public Drawable
{
protected:
    //==============================================================================
    DrawableShape();
    DrawableShape (const DrawableShape&);

public:
    /** Destructor. */
    ~DrawableShape();

    //==============================================================================
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
    /** @internal */
    class FillAndStrokeState  : public  Drawable::ValueTreeWrapperBase
    {
    public:
        FillAndStrokeState (const ValueTree& state);

        const FillType getMainFill (Expression::EvaluationContext* nameFinder,
                                    ImageProvider* imageProvider) const;
        ValueTree getMainFillState();
        void setMainFill (const FillType& newFill, const RelativePoint* gradientPoint1,
                          const RelativePoint* gradientPoint2, const RelativePoint* gradientPoint3,
                          ImageProvider* imageProvider, UndoManager* undoManager);

        const FillType getStrokeFill (Expression::EvaluationContext* nameFinder,
                                      ImageProvider* imageProvider) const;
        ValueTree getStrokeFillState();
        void setStrokeFill (const FillType& newFill, const RelativePoint* gradientPoint1,
                            const RelativePoint* gradientPoint2, const RelativePoint* gradientPoint3,
                            ImageProvider* imageProvider, UndoManager* undoManager);

        const PathStrokeType getStrokeType() const;
        void setStrokeType (const PathStrokeType& newStrokeType, UndoManager* undoManager);

        static const FillType readFillType (const ValueTree& v, RelativePoint* gradientPoint1,
                                            RelativePoint* gradientPoint2, RelativePoint* gradientPoint3,
                                            Expression::EvaluationContext* nameFinder,
                                            ImageProvider* imageProvider);

        static void writeFillType (ValueTree& v, const FillType& fillType,
                                   const RelativePoint* gradientPoint1, const RelativePoint* gradientPoint2,
                                   const RelativePoint* gradientPoint3, ImageProvider* imageProvider,
                                   UndoManager* undoManager);

        static const Identifier type, colour, colours, fill, stroke, path, jointStyle, capStyle, strokeWidth,
                                gradientPoint1, gradientPoint2, gradientPoint3, radial, imageId, imageOpacity;
    };

    /** @internal */
    const Rectangle<float> getDrawableBounds() const;
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    bool hitTest (int x, int y) const;

protected:
    //==============================================================================
    /** Called when the cached path should be updated. */
    void pathChanged();
    /** Called when the cached stroke should be updated. */
    void strokeChanged();

    /** Implemented by subclasses to regenerate the path. */
    virtual bool rebuildPath (Path& path) const = 0;

    /** True if there's a stroke with a non-zero thickness and non-transparent colour. */
    bool isStrokeVisible() const throw();

    /** Updates the details from a FillAndStrokeState object, returning true if something changed. */
    bool refreshFillTypes (const FillAndStrokeState& newState,
                           Expression::EvaluationContext* nameFinder,
                           ImageProvider* imageProvider);

    /** Writes the stroke and fill details to a FillAndStrokeState object. */
    void writeTo (FillAndStrokeState& state, ImageProvider* imageProvider, UndoManager* undoManager) const;


    //==============================================================================
    PathStrokeType strokeType;
    Path path, strokePath;

private:
    FillType mainFill, strokeFill;

    DrawableShape& operator= (const DrawableShape&);
};


#endif   // __JUCE_DRAWABLESHAPE_JUCEHEADER__
