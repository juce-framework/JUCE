/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_DRAWABLE_JUCEHEADER__
#define __JUCE_DRAWABLE_JUCEHEADER__

#include "../contexts/juce_Graphics.h"
#include "../../../../juce_core/text/juce_XmlElement.h"


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

        This is the main rendering method you should call to render a Drawable.

        @see drawWithin
    */
    virtual void draw (Graphics& g,
                       const AffineTransform& transform = AffineTransform::identity) const = 0;

    /** Renders the Drawable at a given offset within the Graphics context.

        This is basically a quick way of saying:

        @code
        draw (g, AffineTransform::translation (x, y)).
        @endcode
    */
    void drawAt (Graphics& g,
                 const float x,
                 const float y) const;

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
    */
    void drawWithin (Graphics& g,
                     const int destX,
                     const int destY,
                     const int destWidth,
                     const int destHeight,
                     const RectanglePlacement& placement) const;


    //==============================================================================
    /** Returns the smallest rectangle that can contain this Drawable object.
    */
    virtual void getBounds (float& x, float& y, float& width, float& height) const = 0;

    /** Returns true if the given point is somewhere inside this Drawable.
    */
    virtual bool hitTest (float x, float y) const = 0;

    //==============================================================================
    /** Tries to turn some kind of image file into a drawable.

        The data could be an image that the ImageFileFormat class understands, or it
        could be SVG.
    */
    static Drawable* createFromImageData (const void* data, const int numBytes);

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
    juce_UseDebuggingNewOperator

private:
    Drawable (const Drawable&);
    const Drawable& operator= (const Drawable&);
};


#endif   // __JUCE_DRAWABLE_JUCEHEADER__
