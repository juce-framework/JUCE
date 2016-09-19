/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_IMAGE_H_INCLUDED
#define JUCE_IMAGE_H_INCLUDED

class ImageType;
class ImagePixelData;


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

    //==============================================================================
    /** Creates a null image. */
    Image() noexcept;

    /** Creates an image with a specified size and format.

        The image's internal type will be of the NativeImageType class - to specify a
        different type, use the other constructor, which takes an ImageType to use.

        @param format           the number of colour channels in the image
        @param imageWidth       the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a width of 1 will be used)
        @param imageHeight      the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a height of 1 will be used)
        @param clearImage       if true, the image will initially be cleared to black (if it's RGB)
                                or transparent black (if it's ARGB). If false, the image may contain
                                junk initially, so you need to make sure you overwrite it thoroughly.
    */
    Image (PixelFormat format, int imageWidth, int imageHeight, bool clearImage);

    /** Creates an image with a specified size and format.

        @param format           the number of colour channels in the image
        @param imageWidth       the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a width of 1 will be used)
        @param imageHeight      the desired width of the image, in pixels - this value must be
                                greater than zero (otherwise a height of 1 will be used)
        @param clearImage       if true, the image will initially be cleared to black (if it's RGB)
                                or transparent black (if it's ARGB). If false, the image may contain
                                junk initially, so you need to make sure you overwrite it thoroughly.
        @param type             the type of image - this lets you specify the internal format that will
                                be used to allocate and manage the image data.
    */
    Image (PixelFormat format, int imageWidth, int imageHeight, bool clearImage, const ImageType& type);

    /** Creates a shared reference to another image.

        This won't create a duplicate of the image - when Image objects are copied, they simply
        point to the same shared image data. To make sure that an Image object has its own unique,
        unshared internal data, call duplicateIfShared().
    */
    Image (const Image&) noexcept;

    /** Makes this image refer to the same underlying image as another object.

        This won't create a duplicate of the image - when Image objects are copied, they simply
        point to the same shared image data. To make sure that an Image object has its own unique,
        unshared internal data, call duplicateIfShared().
    */
    Image& operator= (const Image&);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    Image (Image&&) noexcept;
    Image& operator= (Image&&) noexcept;
   #endif

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

   #if JUCE_ALLOW_STATIC_NULL_VARIABLES
    /** A null Image object that can be used when you need to return an invalid image.
        This object is the equivalient to an Image created with the default constructor, and
        you should always prefer to use Image() or {} when you need an empty image object.
    */
    static const Image null;
   #endif

    //==============================================================================
    /** Returns the image's width (in pixels). */
    int getWidth() const noexcept;

    /** Returns the image's height (in pixels). */
    int getHeight() const noexcept;

    /** Returns a rectangle with the same size as this image.
        The rectangle's origin is always (0, 0).
    */
    Rectangle<int> getBounds() const noexcept;

    /** Returns the image's pixel format. */
    PixelFormat getFormat() const noexcept;

    /** True if the image's format is ARGB. */
    bool isARGB() const noexcept;

    /** True if the image's format is RGB. */
    bool isRGB() const noexcept;

    /** True if the image's format is a single-channel alpha map. */
    bool isSingleChannel() const noexcept;

    /** True if the image contains an alpha-channel. */
    bool hasAlphaChannel() const noexcept;

    //==============================================================================
    /** Clears a section of the image with a given colour.

        This won't do any alpha-blending - it just sets all pixels in the image to
        the given colour (which may be non-opaque if the image has an alpha channel).
    */
    void clear (const Rectangle<int>& area, Colour colourToClearTo = Colour (0x00000000));

    /** Returns a rescaled version of this image.

        A new image is returned which is a copy of this one, rescaled to the given size.

        Note that if the new size is identical to the existing image, this will just return
        a reference to the original image, and won't actually create a duplicate.
    */
    Image rescaled (int newWidth, int newHeight,
                    Graphics::ResamplingQuality quality = Graphics::mediumResamplingQuality) const;

    /** Creates a copy of this image.
        Note that it's usually more efficient to use duplicateIfShared(), because it may not be necessary
        to copy an image if nothing else is using it.
        @see getReferenceCount
    */
    Image createCopy() const;

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

        If the coordinates given are beyond the image's boundaries, this will
        return Colours::transparentBlack.

        @see setPixelAt, Image::BitmapData::getPixelColour
    */
    Colour getPixelAt (int x, int y) const;

    /** Sets the colour of one of the image's pixels.

        If the coordinates are beyond the image's boundaries, then nothing will happen.

        Note that this won't do any alpha-blending, it'll just replace the existing pixel
        with the given one. The colour's opacity will be ignored if this image doesn't have
        an alpha-channel.

        @see getPixelAt, Image::BitmapData::setPixelColour
    */
    void setPixelAt (int x, int y, Colour colour);

    /** Changes the opacity of a pixel.

        This only has an effect if the image has an alpha channel and if the
        given coordinates are inside the image's boundary.

        The multiplier must be in the range 0 to 1.0, and the current alpha
        at the given coordinates will be multiplied by this value.

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
    class JUCE_API  BitmapData
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
            The coordinate you provide here isn't checked, so it's the caller's responsibility to make
            sure it's not out-of-range.
        */
        inline uint8* getLinePointer (int y) const noexcept                 { return data + y * lineStride; }

        /** Returns a pointer to a pixel in the image.
            The coordinates you give here are not checked, so it's the caller's responsibility to make sure they're
            not out-of-range.
        */
        inline uint8* getPixelPointer (int x, int y) const noexcept         { return data + y * lineStride + x * pixelStride; }

        /** Returns the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            repsonsibility to make sure they're within the image's size.
        */
        Colour getPixelColour (int x, int y) const noexcept;

        /** Sets the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            repsonsibility to make sure they're within the image's size.
        */
        void setPixelColour (int x, int y, Colour colour) const noexcept;

        /** Returns the size of the bitmap. */
        Rectangle<int> getBounds() const noexcept                           { return Rectangle<int> (width, height); }

        uint8* data;             /**< The raw pixel data, packed according to the image's pixel format. */
        PixelFormat pixelFormat; /**< The format of the data. */
        int lineStride;          /**< The number of bytes between each line. */
        int pixelStride;         /**< The number of bytes between each pixel. */
        int width, height;

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
        JUCE_DECLARE_NON_COPYABLE (BitmapData)
    };

    //==============================================================================
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
    void createSolidAreaMask (RectangleList<int>& result, float alphaThreshold) const;

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
    int getReferenceCount() const noexcept;

    //==============================================================================
    /** @internal */
    ImagePixelData* getPixelData() const noexcept       { return image; }

    /** @internal */
    explicit Image (ImagePixelData*) noexcept;

private:
    //==============================================================================
    ReferenceCountedObjectPtr<ImagePixelData> image;

    JUCE_LEAK_DETECTOR (Image)
};


//==============================================================================
/**
    This is a base class for holding image data in implementation-specific ways.

    You may never need to use this class directly - it's used internally
    by the Image class to store the actual image data. To access pixel data directly,
    you should use Image::BitmapData rather than this class.

    ImagePixelData objects are created indirectly, by subclasses of ImageType.
    @see Image, ImageType
*/
class JUCE_API  ImagePixelData  : public ReferenceCountedObject
{
public:
    ImagePixelData (Image::PixelFormat, int width, int height);
    ~ImagePixelData();

    typedef ReferenceCountedObjectPtr<ImagePixelData> Ptr;

    /** Creates a context that will draw into this image. */
    virtual LowLevelGraphicsContext* createLowLevelContext() = 0;
    /** Creates a copy of this image. */
    virtual Ptr clone() = 0;
    /** Creates an instance of the type of this image. */
    virtual ImageType* createType() const = 0;
    /** Initialises a BitmapData object. */
    virtual void initialiseBitmapData (Image::BitmapData&, int x, int y, Image::BitmapData::ReadWriteMode) = 0;
    /** Returns the number of Image objects which are currently referring to the same internal
        shared image data. This is different to the reference count as an instance of ImagePixelData
        can internally depend on another ImagePixelData via it's member variables. */
    virtual int getSharedCount() const noexcept;


    /** The pixel format of the image data. */
    const Image::PixelFormat pixelFormat;
    const int width, height;

    /** User-defined settings that are attached to this image.
        @see Image::getProperties().
    */
    NamedValueSet userData;

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() {}

        virtual void imageDataChanged (ImagePixelData*) = 0;
        virtual void imageDataBeingDeleted (ImagePixelData*) = 0;
    };

    ListenerList<Listener> listeners;

    void sendDataChangeMessage();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagePixelData)
};

//==============================================================================
/**
    This base class is for handlers that control a type of image manipulation format,
    e.g. an in-memory bitmap, an OpenGL image, CoreGraphics image, etc.

    @see SoftwareImageType, NativeImageType, OpenGLImageType
*/
class JUCE_API  ImageType
{
public:
    ImageType();
    virtual ~ImageType();

    /** Creates a new image of this type, and the specified parameters. */
    virtual ImagePixelData::Ptr create (Image::PixelFormat format, int width, int height, bool shouldClearImage) const = 0;

    /** Must return a unique number to identify this type. */
    virtual int getTypeID() const = 0;

    /** Returns an image which is a copy of the source image, but using this type of storage mechanism.
        For example, to make sure that an image is stored in-memory, you could use:
        @code myImage = SoftwareImageType().convert (myImage); @endcode
    */
    virtual Image convert (const Image& source) const;
};

//==============================================================================
/**
    An image storage type which holds the pixels in-memory as a simple block of values.
    @see ImageType, NativeImageType
*/
class JUCE_API  SoftwareImageType   : public ImageType
{
public:
    SoftwareImageType();
    ~SoftwareImageType();

    ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool clearImage) const override;
    int getTypeID() const override;
};

//==============================================================================
/**
    An image storage type which holds the pixels using whatever is the default storage
    format on the current platform.
    @see ImageType, SoftwareImageType
*/
class JUCE_API  NativeImageType   : public ImageType
{
public:
    NativeImageType();
    ~NativeImageType();

    ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool clearImage) const override;
    int getTypeID() const override;
};


#endif   // JUCE_IMAGE_H_INCLUDED
