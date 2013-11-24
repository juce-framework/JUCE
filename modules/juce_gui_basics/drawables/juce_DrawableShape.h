/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_DRAWABLESHAPE_H_INCLUDED
#define JUCE_DRAWABLESHAPE_H_INCLUDED


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
    /** A FillType wrapper that allows the gradient coordinates to be implemented using RelativePoint.
    */
    class RelativeFillType
    {
    public:
        RelativeFillType();
        RelativeFillType (const FillType& fill);
        RelativeFillType (const RelativeFillType&);
        RelativeFillType& operator= (const RelativeFillType&);

        bool operator== (const RelativeFillType&) const;
        bool operator!= (const RelativeFillType&) const;

        bool isDynamic() const;
        bool recalculateCoords (Expression::Scope* scope);

        void writeTo (ValueTree& v, ComponentBuilder::ImageProvider*, UndoManager*) const;
        bool readFrom (const ValueTree& v, ComponentBuilder::ImageProvider*);

        //==============================================================================
        FillType fill;
        RelativePoint gradientPoint1, gradientPoint2, gradientPoint3;
    };

    //==============================================================================
    /** Sets a fill type for the path.
        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this to a transparent
        colour.

        @see setPath, setStrokeFill
    */
    void setFill (const FillType& newFill);

    /** Sets a fill type for the path.
        This colour is used to fill the path - if you don't want the path to be
        filled (e.g. if you're just drawing an outline), set this to a transparent
        colour.

        @see setPath, setStrokeFill
    */
    void setFill (const RelativeFillType& newFill);

    /** Returns the current fill type.
        @see setFill
    */
    const RelativeFillType& getFill() const noexcept                { return mainFill; }

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    void setStrokeFill (const FillType& newStrokeFill);

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    void setStrokeFill (const RelativeFillType& newStrokeFill);

    /** Returns the current stroke fill.
        @see setStrokeFill
    */
    const RelativeFillType& getStrokeFill() const noexcept          { return strokeFill; }

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
    const PathStrokeType& getStrokeType() const noexcept            { return strokeType; }

    //==============================================================================
    /** @internal */
    class FillAndStrokeState  : public  Drawable::ValueTreeWrapperBase
    {
    public:
        FillAndStrokeState (const ValueTree& state);

        ValueTree getFillState (const Identifier& fillOrStrokeType);
        RelativeFillType getFill (const Identifier& fillOrStrokeType, ComponentBuilder::ImageProvider*) const;
        void setFill (const Identifier& fillOrStrokeType, const RelativeFillType& newFill,
                      ComponentBuilder::ImageProvider*, UndoManager*);

        PathStrokeType getStrokeType() const;
        void setStrokeType (const PathStrokeType& newStrokeType, UndoManager*);

        static const Identifier type, colour, colours, fill, stroke, path, jointStyle, capStyle, strokeWidth,
                                gradientPoint1, gradientPoint2, gradientPoint3, radial, imageId, imageOpacity;
    };

    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    bool hitTest (int x, int y) override;

protected:
    //==============================================================================
    /** Called when the cached path should be updated. */
    void pathChanged();
    /** Called when the cached stroke should be updated. */
    void strokeChanged();
    /** True if there's a stroke with a non-zero thickness and non-transparent colour. */
    bool isStrokeVisible() const noexcept;
    /** Updates the details from a FillAndStrokeState object, returning true if something changed. */
    void refreshFillTypes (const FillAndStrokeState& newState, ComponentBuilder::ImageProvider*);
    /** Writes the stroke and fill details to a FillAndStrokeState object. */
    void writeTo (FillAndStrokeState& state, ComponentBuilder::ImageProvider*, UndoManager*) const;

    //==============================================================================
    PathStrokeType strokeType;
    Path path, strokePath;

private:
    class RelativePositioner;
    RelativeFillType mainFill, strokeFill;
    ScopedPointer<RelativeCoordinatePositionerBase> mainFillPositioner, strokeFillPositioner;

    void setFillInternal (RelativeFillType& fill, const RelativeFillType& newFill,
                          ScopedPointer<RelativeCoordinatePositionerBase>& positioner);

    DrawableShape& operator= (const DrawableShape&);
};


#endif   // JUCE_DRAWABLESHAPE_H_INCLUDED
