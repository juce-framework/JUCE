/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A base class implementing common functionality for Drawable classes which
    consist of some kind of filled and stroked outline.

    @see DrawablePath, DrawableRectangle

    @tags{GUI}
*/
class JUCE_API  DrawableShape   : public Drawable
{
protected:
    //==============================================================================
    DrawableShape();
    DrawableShape (const DrawableShape&);

public:
    /** Destructor. */
    ~DrawableShape() override;

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
    const FillType& getFill() const noexcept                        { return mainFill; }

    /** Sets the fill type with which the outline will be drawn.
        @see setFill
    */
    void setStrokeFill (const FillType& newStrokeFill);

    /** Returns the current stroke fill.
        @see setStrokeFill
    */
    const FillType& getStrokeFill() const noexcept                  { return strokeFill; }

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

    /** Provides a set of dash lengths to use for stroking the path. */
    void setDashLengths (const Array<float>& newDashLengths);

    /** Returns the set of dash lengths that the path is using. */
    const Array<float>& getDashLengths() const noexcept             { return dashLengths; }

    //==============================================================================
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    bool hitTest (int x, int y) override;
    /** @internal */
    bool replaceColour (Colour originalColour, Colour replacementColour) override;
    /** @internal */
    Path getOutlineAsPath() const override;

protected:
    //==============================================================================
    /** Called when the cached path should be updated. */
    void pathChanged();
    /** Called when the cached stroke should be updated. */
    void strokeChanged();
    /** True if there's a stroke with a non-zero thickness and non-transparent colour. */
    bool isStrokeVisible() const noexcept;

    //==============================================================================
    PathStrokeType strokeType;
    Array<float> dashLengths;
    Path path, strokePath;

private:
    FillType mainFill, strokeFill;

    DrawableShape& operator= (const DrawableShape&);
};

} // namespace juce
