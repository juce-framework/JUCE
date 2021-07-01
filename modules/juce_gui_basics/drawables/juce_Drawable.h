/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    The base class for objects which can draw themselves, e.g. polygons, images, etc.

    @see DrawableComposite, DrawableImage, DrawablePath, DrawableText

    @tags{GUI}
*/
class JUCE_API  Drawable  : public Component
{
protected:
    //==============================================================================
    /** The base class can't be instantiated directly.

        @see DrawableComposite, DrawableImage, DrawablePath, DrawableText
    */
    Drawable();

public:
    /** Destructor. */
    ~Drawable() override;

    //==============================================================================
    /** Creates a deep copy of this Drawable object.

        Use this to create a new copy of this and any sub-objects in the tree.
    */
    virtual std::unique_ptr<Drawable> createCopy() const = 0;

    /** Creates a path that describes the outline of this drawable. */
    virtual Path getOutlineAsPath() const = 0;

    //==============================================================================
    /** Renders this Drawable object.

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.

        @see drawWithin
    */
    void draw (Graphics& g, float opacity,
               const AffineTransform& transform = AffineTransform()) const;

    /** Renders the Drawable at a given offset within the Graphics context.

        The coordinates passed-in are used to translate the object relative to its own
        origin before drawing it - this is basically a quick way of saying:

        @code
        draw (g, AffineTransform::translation (x, y)).
        @endcode

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.
    */
    void drawAt (Graphics& g, float x, float y, float opacity) const;

    /** Renders the Drawable within a rectangle, scaling it to fit neatly inside without
        changing its aspect-ratio.

        The object can placed arbitrarily within the rectangle based on a Justification type,
        and can either be made as big as possible, or just reduced to fit.

        Note that the preferred way to render a drawable in future is by using it
        as a component and adding it to a parent, so you might want to consider that
        before using this method.

        @param g                        the graphics context to render onto
        @param destArea                 the target rectangle to fit the drawable into
        @param placement                defines the alignment and rescaling to use to fit
                                        this object within the target rectangle.
        @param opacity                  the opacity to use, in the range 0 to 1.0
    */
    void drawWithin (Graphics& g,
                     Rectangle<float> destArea,
                     RectanglePlacement placement,
                     float opacity) const;


    //==============================================================================
    /** Resets any transformations on this drawable, and positions its origin within
        its parent component.
    */
    void setOriginWithOriginalSize (Point<float> originWithinParent);

    /** Sets a transform for this drawable that will position it within the specified
        area of its parent component.
    */
    void setTransformToFit (const Rectangle<float>& areaInParent, RectanglePlacement placement);

    /** Returns the DrawableComposite that contains this object, if there is one. */
    DrawableComposite* getParent() const;

    /** Sets a the clipping region of this drawable using another drawable.
        The drawable passed in will be deleted when no longer needed.
    */
    void setClipPath (std::unique_ptr<Drawable> drawableClipPath);

    //==============================================================================
    /** Tries to turn some kind of image file into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageData (const void* data, size_t numBytes);

    /** Tries to turn a stream containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageDataStream (InputStream& dataSource);

    /** Tries to turn a file containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static std::unique_ptr<Drawable> createFromImageFile (const File& file);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document, and to turn this
        into a Drawable tree.

        If something goes wrong while parsing, it may return nullptr.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.
    */
    static std::unique_ptr<Drawable> createFromSVG (const XmlElement& svgDocument);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a file,
        and to turn this into a Drawable tree.

        If something goes wrong while parsing, it may return nullptr.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.

        Any references to references to external image files will be relative to
        the parent directory of the file passed.
    */
    static std::unique_ptr<Drawable> createFromSVGFile (const File& svgFile);

    /** Parses an SVG path string and returns it. */
    static Path parseSVGPath (const String& svgPath);

    //==============================================================================
    /** Returns the area that this drawable covers.
        The result is expressed in this drawable's own coordinate space, and does not take
        into account any transforms that may be applied to the component.
    */
    virtual Rectangle<float> getDrawableBounds() const = 0;

    /** Recursively replaces a colour that might be used for filling or stroking.
        return true if any instances of this colour were found.
    */
    virtual bool replaceColour (Colour originalColour, Colour replacementColour);

protected:
    //==============================================================================
    friend class DrawableComposite;
    friend class DrawableShape;

    /** @internal */
    void transformContextToCorrectOrigin (Graphics&);
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    void setBoundsToEnclose (Rectangle<float>);
    /** @internal */
    void applyDrawableClipPath (Graphics&);

    Point<int> originRelativeToComponent;
    std::unique_ptr<Drawable> drawableClipPath;

    void nonConstDraw (Graphics&, float opacity, const AffineTransform&);

    Drawable (const Drawable&);
    Drawable& operator= (const Drawable&);
    JUCE_LEAK_DETECTOR (Drawable)
};

} // namespace juce
