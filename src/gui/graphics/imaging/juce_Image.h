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

#ifndef __JUCE_IMAGE_JUCEHEADER__
#define __JUCE_IMAGE_JUCEHEADER__

#include "../colour/juce_Colour.h"
#include "../contexts/juce_Graphics.h"
#include "../../../containers/juce_Variant.h"
#include "../../../containers/juce_NamedValueSet.h"

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
    /**
    */
    enum PixelFormat
    {
        UnknownFormat,
        RGB,                /**<< each pixel is a 3-byte packed RGB colour value. For byte order, see the PixelRGB class. */
        ARGB,               /**<< each pixel is a 4-byte ARGB premultiplied colour value. For byte order, see the PixelARGB class. */
        SingleChannel       /**<< each pixel is a 1-byte alpha channel value. */
    };

    /**
    */
    enum ImageType
    {
        SoftwareImage = 0,
        NativeImage
    };

    //==============================================================================
    /** Creates a null image. */
    Image();

    /** Creates an image with a specified size and format.

        @param format           the number of colour channels in the image
        @param imageWidth       the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a width of 1 will be used)
        @param imageHeight      the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a height of 1 will be used)
        @param clearImage       if true, the image will initially be cleared to black (if it's RGB)
                                or transparent black (if it's ARGB). If false, the image may contain
                                junk initially, so you need to make sure you overwrite it thoroughly.
        @param type             the type of image - this lets you specify whether you want a purely
                                memory-based image, or one that may be managed by the OS if possible.
    */
    Image (PixelFormat format,
           int imageWidth,
           int imageHeight,
           bool clearImage,
           ImageType type = NativeImage);

    /** Creates a shared reference to another image.

        This won't create a duplicate of the image - when Image objects are copied, they simply
        point to the same shared image data. To make sure that an Image object has its own unique,
        unshared internal data, call duplicateIfShared().
    */
    Image (const Image& other);

    /** Makes this image refer to the same underlying image as another object.

        This won't create a duplicate of the image - when Image objects are copied, they simply
        point to the same shared image data. To make sure that an Image object has its own unique,
        unshared internal data, call duplicateIfShared().
    */
    Image& operator= (const Image&);

    /** Destructor. */
    ~Image();

    /** Returns true if the two images are referring to the same internal, shared image. */
    bool operator== (const Image& other) const noexcept     { return image == other.image; }

    /** Returns true if the two images are not referring to the same internal, shared image. */
    bool operator!= (const Image& other) const noexcept     { return image != other.image; }

    /** Returns true if this image isn't null.
        If you create an Image with the default constructor, it has no size or content, and is null
        until you reassign it to an Image which contains some actual data.
        The isNull() method is the opposite of isValid().
        @see isNull
    */
    inline bool isValid() const noexcept                    { return image != nullptr; }

    /** Returns true if this image is not valid.
        If you create an Image with the default constructor, it has no size or content, and is null
        until you reassign it to an Image which contains some actual data.
        The isNull() method is the opposite of isValid().
        @see isValid
    */
    inline bool isNull() const noexcept                     { return image == nullptr; }

    /** A null Image object that can be used when you need to return an invalid image.
        This object is the equivalient to an Image created with the default constructor.
    */
    static const Image null;

    //==============================================================================
    /** Returns the image's width (in pixels). */
    int getWidth() const noexcept                           { return image == nullptr ? 0 : image->width; }

    /** Returns the image's height (in pixels). */
    int getHeight() const noexcept                          { return image == nullptr ? 0 : image->height; }

    /** Returns a rectangle with the same size as this image.
        The rectangle's origin is always (0, 0).
    */
    const Rectangle<int> getBounds() const noexcept         { return image == nullptr ? Rectangle<int>() : Rectangle<int> (image->width, image->height); }

    /** Returns the image's pixel format. */
    PixelFormat getFormat() const noexcept                  { return image == nullptr ? UnknownFormat : image->format; }

    /** True if the image's format is ARGB. */
    bool isARGB() const noexcept                            { return getFormat() == ARGB; }

    /** True if the image's format is RGB. */
    bool isRGB() const noexcept                             { return getFormat() == RGB; }

    /** True if the image's format is a single-channel alpha map. */
    bool isSingleChannel() const noexcept                   { return getFormat() == SingleChannel; }

    /** True if the image contains an alpha-channel. */
    bool hasAlphaChannel() const noexcept                   { return getFormat() != RGB; }

    //==============================================================================
    /** Clears a section of the image with a given colour.

        This won't do any alpha-blending - it just sets all pixels in the image to
        the given colour (which may be non-opaque if the image has an alpha channel).
    */
    void clear (const Rectangle<int>& area, const Colour& colourToClearTo = Colour (0x00000000));

    /** Returns a rescaled version of this image.

        A new image is returned which is a copy of this one, rescaled to the given size.

        Note that if the new size is identical to the existing image, this will just return
        a reference to the original image, and won't actually create a duplicate.
    */
    Image rescaled (int newWidth, int newHeight,
                    Graphics::ResamplingQuality quality = Graphics::mediumResamplingQuality) const;

    /** Returns a version of this image with a different image format.

        A new image is returned which has been converted to the specified format.

        Note that if the new format is no different to the current one, this will just return
        a reference to the original image, and won't actually create a copy.
    */
    Image convertedToFormat (PixelFormat newFormat) const;

    /** Makes sure that no other Image objects share the same underlying data as this one.

        If no other Image objects refer to the same shared data as this one, this method has no
        effect. But if there are other references to the data, this will create a new copy of
        the data internally.

        Call this if you want to draw onto the image, but want to make sure that this doesn't
        affect any other code that may be sharing the same data.

        @see getReferenceCount
    */
    void duplicateIfShared();

    /** Returns an image which refers to a subsection of this image.

        This will not make a copy of the original - the new image will keep a reference to it, so that
        if the original image is changed, the contents of the subsection will also change. Likewise if you
        draw into the subimage, you'll also be drawing onto that area of the original image. Note that if
        you use operator= to make the original Image object refer to something else, the subsection image
        won't pick up this change, it'll remain pointing at the original.

        The area passed-in will be clipped to the bounds of this image, so this may return a smaller
        image than the area you asked for, or even a null image if the area was out-of-bounds.
    */
    Image getClippedImage (const Rectangle<int>& area) const;

    //==============================================================================
    /** Returns the colour of one of the pixels in the image.

        If the co-ordinates given are beyond the image's boundaries, this will
        return Colours::transparentBlack.

        @see setPixelAt, Image::BitmapData::getPixelColour
    */
    const Colour getPixelAt (int x, int y) const;

    /** Sets the colour of one of the image's pixels.

        If the co-ordinates are beyond the image's boundaries, then nothing will happen.

        Note that this won't do any alpha-blending, it'll just replace the existing pixel
        with the given one. The colour's opacity will be ignored if this image doesn't have
        an alpha-channel.

        @see getPixelAt, Image::BitmapData::setPixelColour
    */
    void setPixelAt (int x, int y, const Colour& colour);

    /** Changes the opacity of a pixel.

        This only has an effect if the image has an alpha channel and if the
        given co-ordinates are inside the image's boundary.

        The multiplier must be in the range 0 to 1.0, and the current alpha
        at the given co-ordinates will be multiplied by this value.

        @see setPixelAt
    */
    void multiplyAlphaAt (int x, int y, float multiplier);

    /** Changes the overall opacity of the image.

        This will multiply the alpha value of each pixel in the image by the given
        amount (limiting the resulting alpha values between 0 and 255). This allows
        you to make an image more or less transparent.

        If the image doesn't have an alpha channel, this won't have any effect.
    */
    void multiplyAllAlphas (float amountToMultiplyBy);

    /** Changes all the colours to be shades of grey, based on their current luminosity.
    */
    void desaturate();

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
        enum ReadWriteMode
        {
            readOnly,
            writeOnly,
            readWrite
        };

        BitmapData (Image& image, int x, int y, int w, int h, ReadWriteMode mode);
        BitmapData (const Image& image, int x, int y, int w, int h);
        BitmapData (const Image& image, ReadWriteMode mode);
        ~BitmapData();

        /** Returns a pointer to the start of a line in the image.
            The co-ordinate you provide here isn't checked, so it's the caller's responsibility to make
            sure it's not out-of-range.
        */
        inline uint8* getLinePointer (int y) const noexcept                 { return data + y * lineStride; }

        /** Returns a pointer to a pixel in the image.
            The co-ordinates you give here are not checked, so it's the caller's responsibility to make sure they're
            not out-of-range.
        */
        inline uint8* getPixelPointer (int x, int y) const noexcept         { return data + y * lineStride + x * pixelStride; }

        /** Returns the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            repsonsibility to make sure they're within the image's size.
        */
        const Colour getPixelColour (int x, int y) const noexcept;

        /** Sets the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            repsonsibility to make sure they're within the image's size.
        */
        void setPixelColour (int x, int y, const Colour& colour) const noexcept;

        uint8* data;
        PixelFormat pixelFormat;
        int lineStride, pixelStride, width, height;

        //==============================================================================
        /** Used internally by custom image types to manage pixel data lifetime. */
        class BitmapDataReleaser
        {
        protected:
            BitmapDataReleaser() {}
        public:
            virtual ~BitmapDataReleaser() {}
        };

        ScopedPointer<BitmapDataReleaser> dataReleaser;

    private:
        JUCE_DECLARE_NON_COPYABLE (BitmapData);
    };

    /** Copies some pixel values to a rectangle of the image.

        The format of the pixel data must match that of the image itself, and the
        rectangle supplied must be within the image's bounds.
    */
    void setPixelData (int destX, int destY, int destW, int destH,
                       const uint8* sourcePixelData, int sourceLineStride);

    /** Copies a section of the image to somewhere else within itself. */
    void moveImageSection (int destX, int destY,
                           int sourceX, int sourceY,
                           int width, int height);

    /** Creates a RectangleList containing rectangles for all non-transparent pixels
        of the image.

        @param result           the list that will have the area added to it
        @param alphaThreshold   for a semi-transparent image, any pixels whose alpha is
                                above this level will be considered opaque
    */
    void createSolidAreaMask (RectangleList& result,
                              float alphaThreshold = 0.5f) const;

    //==============================================================================
    /** Returns a NamedValueSet that is attached to the image and which can be used for
        associating custom values with it.

        If this is a null image, this will return a null pointer.
    */
    NamedValueSet* getProperties() const;

    //==============================================================================
    /** Creates a context suitable for drawing onto this image.
        Don't call this method directly! It's used internally by the Graphics class.
    */
    LowLevelGraphicsContext* createLowLevelContext() const;

    /** Returns the number of Image objects which are currently referring to the same internal
        shared image data.

        @see duplicateIfShared
    */
    int getReferenceCount() const noexcept              { return image == nullptr ? 0 : image->getReferenceCount(); }

    //==============================================================================
    /** This is a base class for task-specific types of image.

        Don't use this class directly! It's used internally by the Image class.
    */
    class SharedImage  : public ReferenceCountedObject
    {
    public:
        SharedImage (PixelFormat format, int width, int height);
        ~SharedImage();

        virtual LowLevelGraphicsContext* createLowLevelContext() = 0;
        virtual SharedImage* clone() = 0;
        virtual ImageType getType() const = 0;
        virtual void initialiseBitmapData (BitmapData& bitmapData, int x, int y, BitmapData::ReadWriteMode mode) = 0;

        static SharedImage* createNativeImage (PixelFormat format, int width, int height, bool clearImage);
        static SharedImage* createSoftwareImage (PixelFormat format, int width, int height, bool clearImage);

        const PixelFormat getPixelFormat() const noexcept   { return format; }
        int getWidth() const noexcept                       { return width; }
        int getHeight() const noexcept                      { return height; }

    protected:
        friend class Image;
        friend class BitmapData;
        const PixelFormat format;
        const int width, height;
        NamedValueSet userData;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedImage);
    };

    /** @internal */
    SharedImage* getSharedImage() const noexcept        { return image; }
    /** @internal */
    explicit Image (SharedImage* instance);

private:
    //==============================================================================
    friend class SharedImage;
    friend class BitmapData;

    ReferenceCountedObjectPtr<SharedImage> image;

    JUCE_LEAK_DETECTOR (Image);
};


#endif   // __JUCE_IMAGE_JUCEHEADER__
