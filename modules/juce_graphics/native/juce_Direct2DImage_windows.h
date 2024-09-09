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

/*  A single bitmap that represents a subsection of a virtual bitmap. */
struct Direct2DPixelDataPage
{
    /*  The bounds of the stored bitmap inside the virtual bitmap. */
    Rectangle<int> getBounds() const;

    /*  The stored subsection bitmap. */
    ComSmartPtr<ID2D1Bitmap1> bitmap;

    /*  The top-left position of this virtual bitmap inside the virtual bitmap. */
    Point<int> topLeft;
};

/*  A set of pages that together represent a full virtual bitmap.
    All pages in the set always share the same resource context.
    Additionally, stores a reference to a software-backed bitmap, the content of which will
    be copied to the pages when necessary in order to ensure that the software- and hardware-backed
    bitmaps match.
*/
class Direct2DPixelDataPages
{
public:
    using Page = Direct2DPixelDataPage;

    enum class State
    {
        unsuitableToRead,   // Image data is outdated
        suitableToRead,     // Image data is up-to-date with the backing data
        cleared,            // Implies suitableToRead
    };

    /*  Creates a single page containing the provided bitmap and main-memory storage, marking the
        hardware data as up-to-date.
    */
    Direct2DPixelDataPages (ComSmartPtr<ID2D1Bitmap1>, ImagePixelData::Ptr);

    /*  Allocates hardware storage for the provided software bitmap.
        Depending on the initial state, will:
        - mark the GPU images as needing to be copied from main memory before they are next accessed, or
        - mark the GPU images as up-to-date, or
        - clear the GPU images, then mark them as up-to-date
    */
    Direct2DPixelDataPages (ComSmartPtr<ID2D1DeviceContext1>, ImagePixelData::Ptr, State);

    /*  Returns all pages included in this set.
        This will be called before reading from the pages (e.g. when drawing them).
        Therefore, this function will check whether the hardware data is out-of-date and
        copy from the software image if necessary before returning.
    */
    Span<const Page> getPages();

    /*  Marks this set as needing to be updated from the software image.
        We don't actually do the copy until the next time that we need to read the hardware pages.
        This is to avoid redundant copies in the common case that pages are only drawn on a single
        device at a time.
    */
    void markOutdated()
    {
        upToDate = false;
    }

private:
    ImagePixelData::Ptr backingData;
    std::vector<Direct2DPixelDataPage> pages;
    bool upToDate = false;
};

/*  Pixel data type providing accelerated access to cached Direct2D textures.

    Direct2D bitmaps are device-dependent resources, but frequently a computer will
    have multiple devices, e.g. if there are several GPUs available which is common for laptops.
    In order to support a fast image type that can be drawn by any one of the available devices,
    we store a software bitmap which acts as the source-of-truth, and cache per-device hardware
    bitmaps alongside it. The caching mechanism tries to minimise the amount of redundant work.

    When attempting to access hardware bitmaps, we first check the cache to see whether we've
    previously allocated bitmaps for the requested device, and only create bitmaps if none already
    exist.

    We only copy from the software backup to hardware memory immediately before accessing the
    bitmaps for a particular device, and then only if that hardware bitmap is outdated. All
    hardware bitmaps are marked as outdated when a writeable BitmapData is created for the current
    PixelData. When creating a low-level graphics context, all hardware bitmaps other than the
    render target are marked as outdated.
*/
class Direct2DPixelData : public ImagePixelData,
                          private DxgiAdapterListener
{
public:
    using Ptr = ReferenceCountedObjectPtr<Direct2DPixelData>;
    using Page = Direct2DPixelDataPage;
    using Pages = Direct2DPixelDataPages;

    /*  Creates image storage, taking ownership of the provided bitmap.
        This will immediately copy the content of the image to the software backup, so that the
        image can still be drawn if original device goes away.
    */
    Direct2DPixelData (ComSmartPtr<ID2D1DeviceContext1>, ComSmartPtr<ID2D1Bitmap1>);

    /*  Creates software image storage of the requested size. */
    Direct2DPixelData (Image::PixelFormat, int, int, bool);

    ~Direct2DPixelData() override;

    /*  Creates new software image storage with content matching the content of this image.
        Does not copy any hardware resources.
    */
    ImagePixelData::Ptr clone() override
    {
        return new Direct2DPixelData (backingData->clone(), State::drawn);
    }

    std::unique_ptr<ImageType> createType() const override
    {
        return std::make_unique<NativeImageType>();
    }

    /*  Creates a graphics context that will use the default device to draw into hardware bitmaps
        for that device. When the context is destroyed, the rendered hardware bitmap will be copied
        back to software storage.

        This PixelData may hold device resources for devices other than the default device. In that
        case, the other device resources will be marked as outdated, to ensure that they are updated
        from the software backup before they are next accessed.
    */
    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override;

    /*  Provides access to the software image storage.

        If the bitmap data provides write access, then all device resources will be marked as
        outdated, to ensure that they are updated from the software backup before they are next
        accessed.
    */
    void initialiseBitmapData (Image::BitmapData&, int, int, Image::BitmapData::ReadWriteMode) override;

    void applyGaussianBlurEffect (float radius, Image& result) override;
    void applySingleChannelBoxBlurEffect (int radius, Image& result) override;

    /*  This returns image data that is suitable for use when drawing with the provided context.
        This image data should be treated as a read-only view - making modifications directly
        through the Direct2D API will have unpredictable results.
        If you want to render into this image using D2D, call createLowLevelContext.
    */
    Span<const Page> getPagesForContext (ComSmartPtr<ID2D1DeviceContext1>);

    /*  Utility function that just returns a pointer to the bitmap for the first page returned from
        getPagesForContext.
    */
    ComSmartPtr<ID2D1Bitmap1> getFirstPageForContext (ComSmartPtr<ID2D1DeviceContext1> context)
    {
        const auto pages = getPagesForContext (context);
        return ! pages.empty() ? pages.front().bitmap : nullptr;
    }

private:
    enum class State
    {
        initiallyUndefined,
        initiallyCleared,
        drawing,
        drawn,
    };

    Direct2DPixelData (ImagePixelData::Ptr, State);
    auto getIteratorForContext (ComSmartPtr<ID2D1DeviceContext1>);

    void adapterCreated (DxgiAdapter::Ptr) override {}
    void adapterRemoved (DxgiAdapter::Ptr adapter) override
    {
        if (adapter != nullptr)
            pagesForDevice.erase (adapter->direct2DDevice);
    }

    SharedResourcePointer<DirectX> directX;
    ImagePixelData::Ptr backingData;
    std::map<ComSmartPtr<ID2D1Device1>, Pages> pagesForDevice;
    State state;

    JUCE_LEAK_DETECTOR (Direct2DPixelData)
};

} // namespace juce
