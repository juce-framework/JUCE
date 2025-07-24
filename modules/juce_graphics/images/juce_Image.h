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

    @tags{Graphics}
*/
class JUCE_API  Image  final
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

        @param format           the preferred pixel format. Note that this is only a *hint* which
                                is passed to the ImageType class - different ImageTypes may not support
                                all formats, so may substitute e.g. ARGB for RGB.
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

        @param format           the preferred pixel format. Note that this is only a *hint* which
                                is passed to the ImageType class - different ImageTypes may not support
                                all formats, so may substitute e.g. ARGB for RGB.
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

    /** Move constructor */
    Image (Image&&) noexcept;

    /** Move assignment operator */
    Image& operator= (Image&&) noexcept;

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
    bool isValid() const noexcept;

    /** Returns true if this image is not valid.
        If you create an Image with the default constructor, it has no size or content, and is null
        until you reassign it to an Image which contains some actual data.
        The isNull() method is the opposite of isValid().
        @see isValid
    */
    bool isNull() const noexcept { return ! isValid(); }

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

    /** This is a shorthand for dereferencing the internal ImagePixelData's BackupExtensions
        and calling setBackupEnabled() if the extensions exist.

        @returns true if the extensions exist and the backup flag was updated, or false otherwise

        @see ImagePixelDataBackupExtensions::setBackupEnabled()
    */
    bool setBackupEnabled (bool);

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
    class JUCE_API  BitmapData  final
    {
    public:
        enum ReadWriteMode
        {
            readOnly,
            writeOnly,
            readWrite
        };

        BitmapData (Image& image, int x, int y, int w, int h, ReadWriteMode mode);
        BitmapData (const Image& image, Rectangle<int>, ReadWriteMode mode);
        BitmapData (const Image& image, int x, int y, int w, int h);
        BitmapData (const Image& image, ReadWriteMode mode);

        /** Returns a pointer to the start of a line in the image.
            The coordinate you provide here isn't checked, so it's the caller's responsibility to make
            sure it's not out-of-range.
        */
        inline uint8* getLinePointer (int y) const noexcept
        {
            return data + (ptrdiff_t) y * (ptrdiff_t) lineStride;
        }

        /** Returns a pointer to a pixel in the image.
            The coordinates you give here are not checked, so it's the caller's responsibility to make sure they're
            not out-of-range.
        */
        inline uint8* getPixelPointer (int x, int y) const noexcept
        {
            return data + (ptrdiff_t) y * (ptrdiff_t) lineStride + (ptrdiff_t) x * (ptrdiff_t) pixelStride;
        }

        /** Returns the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            responsibility to make sure they're within the image's size.
        */
        Colour getPixelColour (int x, int y) const noexcept;

        /** Sets the colour of a given pixel.
            For performance reasons, this won't do any bounds-checking on the coordinates, so it's the caller's
            responsibility to make sure they're within the image's size.
        */
        void setPixelColour (int x, int y, Colour colour) const noexcept;

        /** Returns the size of the bitmap. */
        Rectangle<int> getBounds() const noexcept                           { return Rectangle<int> (width, height); }

        /** Attempts to copy the contents of src into this bitmap data.
            Returns true on success, and false otherwise.

            The source BitmapData must be readable, and the destination (current) BitmapData must
            be writeable. This function cannot check for this precondition, so you must ensure this
            yourself!
        */
        bool convertFrom (const Image::BitmapData& src);

        uint8* data;             /**< The raw pixel data, packed according to the image's pixel format. */
        size_t size;             /**< The number of valid/allocated bytes after data. May be smaller than "lineStride * height" if this is a section of a larger image. */
        PixelFormat pixelFormat; /**< The format of the data. */
        int lineStride;          /**< The number of bytes between each line. */
        int pixelStride;         /**< The number of bytes between each pixel. */
        int width, height;

        //==============================================================================
        /** Used internally by custom image types to manage pixel data lifetime. */
        class BitmapDataReleaser
        {
        protected:
            BitmapDataReleaser() = default;
        public:
            virtual ~BitmapDataReleaser() = default;
        };

        std::unique_ptr<BitmapDataReleaser> dataReleaser;

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
    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() const;

    /** Returns the number of Image objects which are currently referring to the same internal
        shared image data.

        @see duplicateIfShared
    */
    int getReferenceCount() const noexcept;

    //==============================================================================
    /** @internal */
    ReferenceCountedObjectPtr<ImagePixelData> getPixelData() const noexcept       { return image; }

    /** @internal */
    explicit Image (ReferenceCountedObjectPtr<ImagePixelData>) noexcept;

    //==============================================================================
   #if JUCE_ALLOW_STATIC_NULL_VARIABLES
    /** @cond */
    /* A null Image object that can be used when you need to return an invalid image. */
    [[deprecated ("If you need a default-constructed var, just use Image() or {}.")]]
    static const Image null;
    /** @endcond */
   #endif

private:
    //==============================================================================
    ReferenceCountedObjectPtr<ImagePixelData> image;

    JUCE_LEAK_DETECTOR (Image)
};

//==============================================================================
/**
    The methods on this interface allow clients of ImagePixelData to query and control
    the automatic-backup process from graphics memory to main memory, if this mechanism is
    relevant and supported.

    Some image types (Direct2D, OpenGL) are backed by textures that live in graphics memory.
    Such textures are quick to display, but they will be lost if the graphics device goes away.

    Normally, a backup of the texture will be kept in main memory, so that the image can still
    be used even if any graphics device goes away. While this has the benefit that programs are
    automatically resilient to graphics devices going away, it also incurs some performance
    overhead, because the texture content must be copied back to main memory after each
    modification.

    For performance-sensitive applications it can be beneficial to disable the automatic sync
    behaviour, and to sync manually instead, which can be achieved using the methods of this type.

    The following table shows how to interpret the results of this type's member functions.

    needsBackup()   | canBackup()   | meaning
    --------------------------------------------------------------------------------------------
    true            | true          | the main-memory copy of the image is outdated, but there's an
                    |               | up-to-date copy in graphics memory
                    |               |
    true            | false         | although main memory is out-of-date, the most recent copy of
                    |               | the image in graphics memory has been lost
                    |               |
    false           | true          | main memory is up-to-date with graphics memory; graphics
                    |               | memory is still available;
                    |               |
    false           | false         | main memory has an up-to-date copy of the image, but the most
                    |               | recent copy of the image in graphics memory has been lost

    @tags{Graphics}
*/
class ImagePixelDataBackupExtensions
{
public:
    /** The automatic image backup mechanism can be disabled by passing false to this function, or
        enabled by passing true.

        If you disable automatic backup for a particular image, make sure you test that your
        software behaves as expected when graphics devices are disconnected. One easy way to test
        this on Windows is to use your program over a remote desktop session, and to end and
        re-start the session while the image is being displayed.

        The most common scenario where this flag is useful is when drawing single-use images.
        e.g. for a drop shadow or other effect, the following series of steps might be carried
        out on each paint call:
        - Create a path that matches the shadowed component's outline
        - Draw the path into a temporary image
        - Blur the temporary image
        - Draw the temporary image into some other context
        - (destroy the temporary image)

        In this case, where the image is created, modified, used, and destroyed in quick succession,
        there's no need to keep a resilient backup of the image around, so it's reasonable to call
        setBackupEnabled(false) after constructing the image.

        @see isBackupEnabled(), backupNow()
    */
    virtual void setBackupEnabled (bool) = 0;

    /** @see setBackupEnabled(), backupNow() */
    virtual bool isBackupEnabled() const = 0;

    /** This function will attempt to make the image resilient to graphics device disconnection by
        copying from graphics memory to main memory.

        By default, backups happen automatically, so there's no need to call this function unless
        auto-backup has been disabled on this image.

        Flushing may fail if the graphics device goes away before its memory can be read.
        If needsBackup() returns false, then backupNow() will always return true without doing any
        work.

        @returns true if the main-memory copy of the image is up-to-date, or false otherwise.

        @see setBackupEnabled(), isBackupEnabled()
    */
    virtual bool backupNow() = 0;

    /** Returns true if the main-memory copy of the image is out-of-date, false if it's up-to-date.

        @see canBackup()
    */
    virtual bool needsBackup() const = 0;

    /** Returns if there is an up-to-date copy of this image in graphics memory, or false otherwise.

        @see needsBackup()
    */
    virtual bool canBackup() const = 0;

protected:
    // Not intended for virtual destruction
    ~ImagePixelDataBackupExtensions() = default;
};

/** @internal
    @tags{Graphics}
 */
class ImagePixelDataNativeExtensions;

//==============================================================================
/**
    This is a base class for holding image data in implementation-specific ways.

    You may never need to use this class directly - it's used internally
    by the Image class to store the actual image data. To access pixel data directly,
    you should use Image::BitmapData rather than this class.

    ImagePixelData objects are created indirectly, by subclasses of ImageType.
    @see Image, ImageType

    @tags{Graphics}
*/
class JUCE_API  ImagePixelData  : public ReferenceCountedObject
{
public:
    using BackupExtensions = ImagePixelDataBackupExtensions;
    using NativeExtensions = ImagePixelDataNativeExtensions;

    ImagePixelData (Image::PixelFormat, int width, int height);
    ~ImagePixelData() override;

    using Ptr = ReferenceCountedObjectPtr<ImagePixelData>;

    /** Creates a context that will draw into this image. */
    virtual std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() = 0;
    /** Creates a copy of this image. */
    virtual Ptr clone() = 0;
    /** Creates an instance of the type of this image. */
    virtual std::unique_ptr<ImageType> createType() const = 0;

    /** Returns a raw pointer to an instance of ImagePixelDataBackupExtensions if this ImagePixelData
        provides this extension, or nullptr otherwise.
    */
    virtual BackupExtensions* getBackupExtensions() { return nullptr; }
    virtual const BackupExtensions* getBackupExtensions() const { return nullptr; }

    /** Initialises a BitmapData object. */
    virtual void initialiseBitmapData (Image::BitmapData&, int x, int y, Image::BitmapData::ReadWriteMode) = 0;
    /** Returns the number of Image objects which are currently referring to the same internal
        shared image data. This is different to the reference count as an instance of ImagePixelData
        can internally depend on another ImagePixelData via it's member variables.
    */
    virtual int getSharedCount() const noexcept;

    //==============================================================================
    /** Copies a section of the image to somewhere else within itself. */
    void moveImageSection (Point<int> destTopLeft, Rectangle<int> sourceRect);

    /** Applies a native blur effect to this image, if available.
        This blur applies to all channels of the input image. It may be more expensive to
        calculate than a box blur, but should produce higher-quality results.

        The default implementation will modify the image pixel-by-pixel on the CPU, which will be slow.
        Native image types may provide optimised implementations.
    */
    virtual void applyGaussianBlurEffectInArea (Rectangle<int> bounds, float radius);

    /** @see applyGaussianBlurEffectInArea() */
    void applyGaussianBlurEffect (float radius)
    {
        applyGaussianBlurEffectInArea ({ width, height }, radius);
    }

    /** Applies a native blur effect to this image, if available.
        This is intended for blurring single-channel images, which is useful when rendering drop
        shadows. This is implemented as several box-blurs in series. The results should be visually
        similar to a Gaussian blur, but less accurate.

        The default implementation will modify the image pixel-by-pixel on the CPU, which will be slow.
        Native image types may provide optimised implementations.
    */
    virtual void applySingleChannelBoxBlurEffectInArea (Rectangle<int> bounds, int radius);

    /** @see applySingleChannelBoxBlurEffectInArea() */
    void applySingleChannelBoxBlurEffect (int radius)
    {
        applySingleChannelBoxBlurEffectInArea ({ width, height }, radius);
    }

    /** Multiples all alpha-channel values in the image by the specified amount.

        The default implementation will modify the image pixel-by-pixel on the CPU, which will be slow.
        Native image types may provide optimised implementations.
    */
    virtual void multiplyAllAlphasInArea (Rectangle<int> bounds, float amount);

    /** @see multiplyAllAlphasInArea() */
    void multiplyAllAlphas (float amount)
    {
        multiplyAllAlphasInArea ({ width, height }, amount);
    }

    /** Changes all the colours to be shades of grey, based on their current luminosity.

        The default implementation will modify the image pixel-by-pixel on the CPU, which will be slow.
        Native image types may provide optimised implementations.
    */
    virtual void desaturateInArea (Rectangle<int> bounds);

    /** @see desaturateInArea() */
    void desaturate()
    {
        desaturateInArea ({ width, height });
    }

    /** The pixel format of the image data. */
    const Image::PixelFormat pixelFormat;
    const int width, height;

    /** User-defined settings that are attached to this image.
        @see Image::getProperties().
    */
    NamedValueSet userData;

    //==============================================================================
    /** Used to receive callbacks for image data changes */
    struct Listener
    {
        virtual ~Listener() = default;

        virtual void imageDataChanged (ImagePixelData*) = 0;
        virtual void imageDataBeingDeleted (ImagePixelData*) = 0;
    };

    ListenerList<Listener> listeners;

    void sendDataChangeMessage();

    /** @internal intentionally not callable from user code */
    virtual NativeExtensions getNativeExtensions();

private:
    /*  Called by moveImageSection().
        The source and destination rects are both guaranteed to be within the image bounds, and
        non-empty.
    */
    virtual void moveValidatedImageSection (Point<int> destTopLeft, Rectangle<int> sourceRect);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagePixelData)
};

//==============================================================================
/**
    This base class is for handlers that control a type of image manipulation format,
    e.g. an in-memory bitmap, an OpenGL image, CoreGraphics image, etc.

    @see SoftwareImageType, NativeImageType, OpenGLImageType

    @tags{Graphics}
*/
class JUCE_API  ImageType
{
public:
    ImageType();
    virtual ~ImageType();

    /** Creates a new image of this type, and the specified parameters. */
    virtual ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool shouldClearImage) const = 0;

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

    @tags{Graphics}
*/
class JUCE_API  SoftwareImageType   : public ImageType
{
public:
    SoftwareImageType();
    ~SoftwareImageType() override;

    ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool clearImage) const override;
    int getTypeID() const override;
};

//==============================================================================
/**
    An image storage type which holds the pixels using whatever is the default storage
    format on the current platform.
    @see ImageType, SoftwareImageType

    @tags{Graphics}
*/
class JUCE_API  NativeImageType   : public ImageType
{
public:
    NativeImageType();
    ~NativeImageType() override;

    ImagePixelData::Ptr create (Image::PixelFormat, int width, int height, bool clearImage) const override;
    int getTypeID() const override;
};

} // namespace juce
