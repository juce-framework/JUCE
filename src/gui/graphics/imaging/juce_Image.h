/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_IMAGE_JUCEHEADER__
#define __JUCE_IMAGE_JUCEHEADER__

#include "../colour/juce_Colour.h"
#include "../contexts/juce_Graphics.h"


//==============================================================================
/**
    Holds a fixed-size bitmap.

    The image is stored in either 24-bit RGB or 32-bit premultiplied-ARGB format.

    To draw into an image, create a Graphics object for it.
    e.g. @code

    // create a transparent 500x500 image..
    Image myImage (Image::RGB, 500, 500, true);

    Graphics g (myImage);
    g.setColour (Colours::red);
    g.fillEllipse (20, 20, 300, 200);  // draws a red ellipse in our image.
    @endcode

    Other useful ways to create an image are with the ImageCache class, or the
    ImageFileFormat, which provides a way to load common image files.

    @see Graphics, ImageFileFormat, ImageCache, ImageConvolutionKernel
*/
class JUCE_API  Image
{
public:
    //==============================================================================
    enum PixelFormat
    {
        RGB,                /**<< each pixel is a 3-byte packed RGB colour value. For byte order, see the PixelRGB class. */
        ARGB,               /**<< each pixel is a 4-byte ARGB premultiplied colour value. For byte order, see the PixelARGB class. */
        SingleChannel       /**<< each pixel is a 1-byte alpha channel value. */
    };

    //==============================================================================
    /** Creates an in-memory image with a specified size and format.

        To create an image that can use native OS rendering methods, see createNativeImage().

        @param format           the number of colour channels in the image
        @param imageWidth       the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a width of 1 will be used)
        @param imageHeight      the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a height of 1 will be used)
        @param clearImage       if true, the image will initially be cleared to black or transparent
                                black. If false, the image may contain random data, and the
                                user will have to deal with this
    */
    Image (const PixelFormat format,
           const int imageWidth,
           const int imageHeight,
           const bool clearImage);

    /** Creates a copy of another image.

        @see createCopy
    */
    Image (const Image& other);

    /** Destructor. */
    virtual ~Image();

    //==============================================================================
    /** Tries to create an image that is uses native drawing methods when you render
        onto it.

        On some platforms this will just return a normal software-based image.
    */
    static Image* createNativeImage (const PixelFormat format,
                                     const int imageWidth,
                                     const int imageHeight,
                                     const bool clearImage);

    //==============================================================================
    /** Returns the image's width (in pixels). */
    int getWidth() const throw()                    { return imageWidth; }

    /** Returns the image's height (in pixels). */
    int getHeight() const throw()                   { return imageHeight; }

    /** Returns a rectangle with the same size as this image.
        The rectangle is always at position (0, 0).
    */
    const Rectangle getBounds() const throw()       { return Rectangle (0, 0, imageWidth, imageHeight); }

    /** Returns the image's pixel format. */
    PixelFormat getFormat() const throw()           { return format; }

    /** True if the image's format is ARGB. */
    bool isARGB() const throw()                     { return format == ARGB; }

    /** True if the image's format is RGB. */
    bool isRGB() const throw()                      { return format == RGB; }

    /** True if the image contains an alpha-channel. */
    bool hasAlphaChannel() const throw()            { return format != RGB; }

    //==============================================================================
    /** Clears a section of the image with a given colour.

        This won't do any alpha-blending - it just sets all pixels in the image to
        the given colour (which may be non-opaque if the image has an alpha channel).
    */
    virtual void clear (int x, int y, int w, int h,
                        const Colour& colourToClearTo = Colour (0x00000000));

    /** Returns a new image that's a copy of this one.

        A new size for the copied image can be specified, or values less than
        zero can be passed-in to use the image's existing dimensions.

        It's up to the caller to delete the image when no longer needed.
    */
    virtual Image* createCopy (int newWidth = -1,
                               int newHeight = -1,
                               const Graphics::ResamplingQuality quality = Graphics::mediumResamplingQuality) const;

    /** Returns a new single-channel image which is a copy of the alpha-channel of this image.
    */
    virtual Image* createCopyOfAlphaChannel() const;

    //==============================================================================
    /** Returns the colour of one of the pixels in the image.

        If the co-ordinates given are beyond the image's boundaries, this will
        return Colours::transparentBlack.

        (0, 0) is the image's top-left corner.

        @see getAlphaAt, setPixelAt, blendPixelAt
    */
    virtual const Colour getPixelAt (const int x, const int y) const;

    /** Sets the colour of one of the image's pixels.

        If the co-ordinates are beyond the image's boundaries, then nothing will
        happen.

        Note that unlike blendPixelAt(), this won't do any alpha-blending, it'll
        just replace the existing pixel with the given one. The colour's opacity
        will be ignored if this image doesn't have an alpha-channel.

        (0, 0) is the image's top-left corner.

        @see blendPixelAt
    */
    virtual void setPixelAt (const int x, const int y, const Colour& colour);

    /** Changes the opacity of a pixel.

        This only has an effect if the image has an alpha channel and if the
        given co-ordinates are inside the image's boundary.

        The multiplier must be in the range 0 to 1.0, and the current alpha
        at the given co-ordinates will be multiplied by this value.

        @see getAlphaAt, setPixelAt
    */
    virtual void multiplyAlphaAt (const int x, const int y, const float multiplier);

    /** Changes the overall opacity of the image.

        This will multiply the alpha value of each pixel in the image by the given
        amount (limiting the resulting alpha values between 0 and 255). This allows
        you to make an image more or less transparent.

        If the image doesn't have an alpha channel, this won't have any effect.
    */
    virtual void multiplyAllAlphas (const float amountToMultiplyBy);

    /** Changes all the colours to be shades of grey, based on their current luminosity.
    */
    virtual void desaturate();

    //==============================================================================
    /** Retrieves a section of an image as raw pixel data, so it can be read or written to.

        You should only use this class as a last resort - messing about with the internals of
        an image is only recommended for people who really know what they're doing!

        A BitmapData object should be used as a temporary, stack-based object. Don't keep one
        hanging around while the image is being used elsewhere.

        Depending on the way the image class is implemented, this may create a temporary buffer
        which is copied back to the image when the object is deleted, or it may just get a pointer
        directly into the image's raw data.

        You can use the stride and data values in this class directly, but don't alter them!
        The actual format of the pixel data depends on the image's format - see Image::getFormat(),
        and the PixelRGB, PixelARGB and PixelAlpha classes for more info.
    */
    class BitmapData
    {
    public:
        BitmapData (Image& image, int x, int y, int w, int h, const bool needsToBeWritable);
        BitmapData (const Image& image, int x, int y, int w, int h);
        ~BitmapData();

        /** Returns a pointer to the start of a line in the image.
            The co-ordinate you provide here isn't checked, so it's the caller's responsibility to make
            sure it's not out-of-range.
        */
        inline uint8* getLinePointer (const int y) const                        { return data + y * lineStride; }

        /** Returns a pointer to a pixel in the image.
            The co-ordinates you give here are not checked, so it's the caller's responsibility to make sure they're
            not out-of-range.
        */
        inline uint8* getPixelPointer (const int x, const int y) const          { return data + y * lineStride + x * pixelStride; }

        uint8* data;
        int lineStride, pixelStride, width, height;

    private:
        BitmapData (const BitmapData&);
        const BitmapData& operator= (const BitmapData&);
    };

    /** Copies some pixel values to a rectangle of the image.

        The format of the pixel data must match that of the image itself, and the
        rectangle supplied must be within the image's bounds.
    */
    virtual void setPixelData (int destX, int destY, int destW, int destH,
                               const uint8* sourcePixelData, int sourceLineStride);

    /** Copies a section of the image to somewhere else within itself.
    */
    virtual void moveImageSection (int destX, int destY,
                                   int sourceX, int sourceY,
                                   int width, int height);

    /** Creates a RectangleList containing rectangles for all non-transparent pixels
        of the image.

        @param result           the list that will have the area added to it
        @param alphaThreshold   for a semi-transparent image, any pixels whose alpha is
                                above this level will be considered opaque
    */
    void createSolidAreaMask (RectangleList& result,
                              const float alphaThreshold = 0.5f) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

    /** Creates a context suitable for drawing onto this image.

        Don't call this method directly! It's used internally by the Graphics class.
    */
    virtual LowLevelGraphicsContext* createLowLevelContext();

protected:
    friend class BitmapData;
    const PixelFormat format;
    const int imageWidth, imageHeight;

    /** Used internally so that subclasses can call a constructor that doesn't allocate memory */
    Image (const PixelFormat format,
           const int imageWidth,
           const int imageHeight);

    int pixelStride, lineStride;
    HeapBlock <uint8> imageDataAllocated;
    uint8* imageData;

private:
    //==============================================================================
    const Image& operator= (const Image&);
};


#endif   // __JUCE_IMAGE_JUCEHEADER__
