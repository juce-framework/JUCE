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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

class DrawableComposite;

//==============================================================================
/**
    The base class for objects which can draw themselves, e.g. polygons, images, etc.

    @see DrawableComposite, DrawableImage, DrawablePath, DrawableText

    @tags{GUI}
*/
class JUCE_API  Drawable
{
public:
    /** @internal */
    class JUCE_API Listener
    {
    public:
        virtual ~Listener()                            = default;
        virtual void drawableBoundsChanged (Drawable*) = 0;
    };

    /** Constructor. */
    Drawable() = default;

    /** Copy constructor. */
    Drawable (const Drawable& other);

     /** Destructor. */
    virtual ~Drawable() = default;

    //==============================================================================
    /** Creates a deep copy of this Drawable object.

        Use this to create a new copy of this and any sub-objects in the tree.
    */
    virtual std::unique_ptr<Drawable> createCopy() const = 0;

    /** Creates a path that describes the outline of this drawable. */
    virtual Path getOutlineAsPath() const = 0;

    //==============================================================================
    /** Renders this Drawable object.

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
    /** Sets a the clipping region of this drawable using another drawable.
        The drawable passed in will be deleted when no longer needed.
    */
    void setClipPath (std::unique_ptr<Drawable> drawableClipPath);

    /** Tests whether a given point is inside the Drawable. */
    virtual bool hitTest (Point<float>) const = 0;

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

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a file,
        and to turn this into a Drawable tree.

        If something goes wrong while parsing, it may return nullptr.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.

        Any references to references to external image files will be relative to
        the parent directory of the file passed.
    */
    static std::unique_ptr<Drawable> createFromSVGFile (const File& svgFile);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document from a string,
        and to turn this into a Drawable tree.
    */
    static std::unique_ptr<DrawableComposite> createFromSVGString (const String& svgString);

    /** Parses an SVG path string and returns it. */
    static Path parseSVGPath (const String& svgPath);

    //==============================================================================
    /** Returns the area that this drawable covers. These bounds are affected by the transform
        passed to setDrawableTransform(). To get the original, untransformed bounds use
        getDrawableBoundsUntransformed().
    */
    Rectangle<float> getDrawableBounds() const;

    /** Returns the area that this drawable covers in its original coordinate system.
        These bounds are not affected by setDrawableTransform().

        @see getDrawableBounds
    */
    virtual Rectangle<float> getDrawableBoundsUntransformed() const = 0;

    /** Returns the bounds that this drawable is intended to occupy. These bounds are affected by
        the transform passed to setDrawableTransform().

        A DrawableComposite will return the value of getContentArea(), while the base implementation
        will return the value of getDrawableBounds().

        Consequently, a Drawable parsed from an SVG file will return the SVG viewBox from this
        function.
    */
    Rectangle<float> getContentBounds() const;

    /** Returns the area that this drawable is intended to occupy in its original coordinate
        system. These bounds are not affected by setDrawableTransform().

        A DrawableComposite will return the value of getContentArea(), while the base implementation
        will return the value of getDrawableBounds().

        Consequently, a Drawable parsed from an SVG file will return the SVG viewBox from this
        function.

        @see getContentBounds
    */
    virtual Rectangle<float> getContentBoundsUntransformed() const;

    /** Returns the width of the drawable bounds rounded up to the nearest integer.

        @see getDrawableBounds
    */
    int getWidth() const
    {
        return getDrawableBounds().toNearestInt().getWidth();
    }

    /** Returns the height of the drawable bounds rounded up to the nearest integer.

        @see getDrawableBounds
    */
    int getHeight() const
    {
        return getDrawableBounds().toNearestInt().getHeight();
    }

    /** Recursively replaces a colour that might be used for filling or stroking.
        return true if any instances of this colour were found.
    */
    virtual bool replaceColour (Colour originalColour, Colour replacementColour);

    /** Sets a transformation that applies to the same coordinate system in which the rest of the
        draw calls are made. You almost certainly want to call this function when working with
        Drawables as opposed to Component::setTransform().

        The reason for this is that the origin of a Drawable is not the same as the point returned
        by Component::getPosition() but has an additional offset internal to the Drawable class.

        Using setDrawableTransform() will take this internal offset into account when applying the
        transform to the Component base.

        You can only use Drawable::setDrawableTransform() or Component::setTransform() for a given
        object. Using both will lead to unpredictable behaviour.
    */
    void setDrawableTransform (const AffineTransform& transform);

    /** Calls setDrawableTransform() with a transform that will position this Drawable within the
        specified area in the untransformed coordinate system of the Drawable.
    */
    void setDrawableTransformToFit (const Rectangle<float>& area, RectanglePlacement placement);

    /** Returns the transform that is currently being applied to this drawable.

        @see setTransform
    */
    AffineTransform getDrawableTransform() const;

    /** Returns the name of this Drawable object.

        The SVG parser attempts to set this value to the id attribute of the corresponding SVG
        element. However, since it's possible to reference and draw a single SVG element multiple
        times, this value is not guaranteed to be unique.

        @see setName
    */
    const String& getName() const
    {
        return name;
    }

    /** Sets the name of this drawable object.

        @see getName
    */
    void setName (const String& newName)
    {
        name = newName;
    }

    /** Adds a listener. */
    void addListener (Listener* newListener)
    {
        listeners.add (newListener);
    }

    /** Removes a listener. */
    void removeListener (Listener* listenerToRemove)
    {
        listeners.remove (listenerToRemove);
    }

    /** @internal */
    virtual bool isImage() const                           { return false; }

    /** @internal */
    virtual std::optional<String> getContainedText() const { return std::nullopt; }

    /** @internal */
    virtual bool requiresBlendBuffer() const { return false; }

protected:
    //==============================================================================
    virtual void paint (Graphics& g) const = 0;

    // Overridden only by DrawableText to support setBoundingBox(). No easy workaround.
    virtual AffineTransform getTransform() const;

    void callListeners()
    {
        listeners.call (&Listener::drawableBoundsChanged, this);
    }

private:
    void applyDrawableClipPath (Graphics&) const;

    AffineTransform transform;
    std::unique_ptr<Drawable> clipPath;
    String name;
    bool visible = true;
    ListenerList<Listener> listeners;

    JUCE_LEAK_DETECTOR (Drawable)
};

} // namespace juce
