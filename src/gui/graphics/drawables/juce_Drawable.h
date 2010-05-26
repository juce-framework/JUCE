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

#ifndef __JUCE_DRAWABLE_JUCEHEADER__
#define __JUCE_DRAWABLE_JUCEHEADER__

#include "../contexts/juce_Graphics.h"
#include "../geometry/juce_RelativeCoordinate.h"
#include "../../../text/juce_XmlElement.h"
#include "../../../containers/juce_ValueTree.h"
class DrawableComposite;


//==============================================================================
/**
    The base class for objects which can draw themselves, e.g. polygons, images, etc.

    @see DrawableComposite, DrawableImage, DrawablePath, DrawableText
*/
class JUCE_API  Drawable
{
protected:
    //==============================================================================
    /** The base class can't be instantiated directly.

        @see DrawableComposite, DrawableImage, DrawablePath, DrawableText
    */
    Drawable();

public:
    /** Destructor. */
    virtual ~Drawable();

    //==============================================================================
    /** Creates a deep copy of this Drawable object.

        Use this to create a new copy of this and any sub-objects in the tree.
    */
    virtual Drawable* createCopy() const = 0;

    //==============================================================================
    /** Renders this Drawable object.
        @see drawWithin
    */
    void draw (Graphics& g, float opacity,
               const AffineTransform& transform = AffineTransform::identity) const;

    /** Renders the Drawable at a given offset within the Graphics context.

        The co-ordinates passed-in are used to translate the object relative to its own
        origin before drawing it - this is basically a quick way of saying:

        @code
        draw (g, AffineTransform::translation (x, y)).
        @endcode
    */
    void drawAt (Graphics& g,
                 float x, float y,
                 float opacity) const;

    /** Renders the Drawable within a rectangle, scaling it to fit neatly inside without
        changing its aspect-ratio.

        The object can placed arbitrarily within the rectangle based on a Justification type,
        and can either be made as big as possible, or just reduced to fit.

        @param g                        the graphics context to render onto
        @param destX                    top-left of the target rectangle to fit it into
        @param destY                    top-left of the target rectangle to fit it into
        @param destWidth                size of the target rectangle to fit the image into
        @param destHeight               size of the target rectangle to fit the image into
        @param placement                defines the alignment and rescaling to use to fit
                                        this object within the target rectangle.
        @param opacity                  the opacity to use, in the range 0 to 1.0
    */
    void drawWithin (Graphics& g,
                     int destX,
                     int destY,
                     int destWidth,
                     int destHeight,
                     const RectanglePlacement& placement,
                     float opacity) const;


    //==============================================================================
    /** Holds the information needed when telling a drawable to render itself.
        @see Drawable::draw
    */
    class RenderingContext
    {
    public:
        RenderingContext (Graphics& g, const AffineTransform& transform, float opacity) throw();

        Graphics& g;
        AffineTransform transform;
        float opacity;

    private:
        RenderingContext& operator= (const RenderingContext&);
    };

    /** Renders this Drawable object.
        @see draw
    */
    virtual void render (const RenderingContext& context) const = 0;

    //==============================================================================
    /** Returns the smallest rectangle that can contain this Drawable object.

        Co-ordinates are relative to the object's own origin.
    */
    virtual const Rectangle<float> getBounds() const = 0;

    /** Returns true if the given point is somewhere inside this Drawable.

        Co-ordinates are relative to the object's own origin.
    */
    virtual bool hitTest (float x, float y) const = 0;

    //==============================================================================
    /** Returns the name given to this drawable.
        @see setName
    */
    const String& getName() const throw()               { return name; }

    /** Assigns a name to this drawable. */
    void setName (const String& newName) throw()        { name = newName; }

    /** Returns the DrawableComposite that contains this object, if there is one. */
    DrawableComposite* getParent() const throw()        { return parent; }

    //==============================================================================
    /** Tries to turn some kind of image file into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static Drawable* createFromImageData (const void* data, size_t numBytes);

    /** Tries to turn a stream containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static Drawable* createFromImageDataStream (InputStream& dataSource);

    /** Tries to turn a file containing some kind of image data into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static Drawable* createFromImageFile (const File& file);

    /** Attempts to parse an SVG (Scalable Vector Graphics) document, and to turn this
        into a Drawable tree.

        The object returned must be deleted by the caller. If something goes wrong
        while parsing, it may return 0.

        SVG is a pretty large and complex spec, and this doesn't aim to be a full
        implementation, but it can return the basic vector objects.
    */
    static Drawable* createFromSVG (const XmlElement& svgDocument);

    //==============================================================================
    /** This class is used when loading Drawables that contain images, and retrieves
        the image for a stored identifier.
        @see Drawable::createFromValueTree
    */
    class JUCE_API  ImageProvider
    {
    public:
        ImageProvider() {}
        virtual ~ImageProvider() {}

        /** Retrieves the image associated with this identifier, which could be any
            kind of string, number, filename, etc.

            The image that is returned will be owned by the caller, but it may come
            from the ImageCache.
        */
        virtual Image* getImageForIdentifier (const var& imageIdentifier) = 0;

        /** Returns an identifier to be used to refer to a given image.
            This is used when converting a drawable into a ValueTree, so if you're
            only loading drawables, you can just return a var::null here.
        */
        virtual const var getIdentifierForImage (Image* image) = 0;
    };

    /** Tries to create a Drawable from a previously-saved ValueTree.
        The ValueTree must have been created by the createValueTree() method.
        If there are any images used within the drawable, you'll need to provide a valid
        ImageProvider object that can be used to retrieve these images from whatever type
        of identifier is used to represent them.
    */
    static Drawable* createFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);

    /** Tries to refresh a Drawable from the same ValueTree that was used to create it.
        @returns the damage rectangle that will need repainting due to any changes that were made.
    */
    virtual const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider) = 0;

    /** Creates a ValueTree to represent this Drawable.
        The VarTree that is returned can be turned back into a Drawable with
        createFromValueTree().
        If there are any images used in this drawable, you'll need to provide a valid
        ImageProvider object that can be used to create storable representations of them.
    */
    virtual const ValueTree createValueTree (ImageProvider* imageProvider) const = 0;

    /** Returns the tag ID that is used for a ValueTree that stores this type of drawable.  */
    virtual const Identifier getValueTreeType() const = 0;

    //==============================================================================
    /** Internal class used to manage ValueTrees that represent Drawables. */
    class ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapperBase (const ValueTree& state);
        ~ValueTreeWrapperBase();

        ValueTree& getState() throw()           { return state; }

        const String getID() const;
        void setID (const String& newID, UndoManager* undoManager);
        static const Identifier idProperty;

        static const FillType readFillType (const ValueTree& v, RelativePoint* gradientPoint1, RelativePoint* gradientPoint2,
                                            RelativeCoordinate::NamedCoordinateFinder* nameFinder);

        static void writeFillType (ValueTree& v, const FillType& fillType,
                                   const RelativePoint* gradientPoint1, const RelativePoint* gradientPoint2,
                                   UndoManager* undoManager);

    protected:
        ValueTree state;
        static const Identifier type, gradientPoint1, gradientPoint2, colour, radial, colours;

        void replaceFillType (const Identifier& tag, const FillType& fillType,
                              const RelativePoint* gradientPoint1, const RelativePoint* gradientPoint2,
                              UndoManager* undoManager);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    friend class DrawableComposite;
    DrawableComposite* parent;
    virtual void invalidatePoints() = 0;

private:
    String name;

    Drawable (const Drawable&);
    Drawable& operator= (const Drawable&);
};


#endif   // __JUCE_DRAWABLE_JUCEHEADER__
