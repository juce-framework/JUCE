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

#ifndef MFD_CLOEXEC
 #define MFD_CLOEXEC 0x0001U
#endif

namespace juce
{

static int createWaylandAnonymousFile (size_t size)
{
   #if defined (__FreeBSD__)
    const auto fdToMap = shm_open (SHM_ANON, O_RDWR, 0600);
   #elif defined (SYS_memfd_create)
    const auto fdToMap = (int) syscall (SYS_memfd_create, "juce-wayland-shm", MFD_CLOEXEC);
   #else
    const auto fdToMap = -1;
   #endif

    if (fdToMap >= 0)
    {
        if (ftruncate (fdToMap, (off_t) size) == 0)
            return fdToMap;

        close (fdToMap);
    }

    return -1;
}

bool WaylandShmBuffer::canCreate()
{
    const auto fd = createWaylandAnonymousFile (4);

    if (fd < 0)
        return false;

    close (fd);
    return true;
}

std::unique_ptr<WaylandShmBuffer> WaylandShmBuffer::create (wl_shm& shm, int width, int height)
{
    // Wayland shared-memory buffers require positive dimensions.
    jassert (width > 0 && height > 0);

    if (width <= 0 || height <= 0)
        return nullptr;

    const auto stride = width * 4;
    const auto dataSize = (size_t) stride * (size_t) height;
    const auto fdToMap = createWaylandAnonymousFile (dataSize);

    if (fdToMap < 0)
        return nullptr;

    // Closing the fd doesn't free the mapping or the pool, so it can go on every exit path.
    const ScopeGuard closeFd { [fdToMap] { close (fdToMap); } };

    auto buffer = std::make_unique<WaylandShmBuffer>();
    auto* mapped = mmap (nullptr, dataSize, PROT_READ | PROT_WRITE, MAP_SHARED, fdToMap, 0);

    if (mapped == MAP_FAILED)
        return nullptr;

    buffer->data = { static_cast<uint8*> (mapped), Unmapper { dataSize } };
    buffer->width = width;
    buffer->height = height;
    buffer->stride = stride;

    const std::unique_ptr<wl_shm_pool, FunctionPointerDestructor<WaylandProtocol::wlShmPoolDestroy>> pool
        { WaylandProtocol::wlShmCreatePool (&shm, fdToMap, (int32_t) dataSize) };

    if (pool == nullptr)
        return nullptr;

    buffer->handle.reset (WaylandProtocol::wlShmPoolCreateBuffer (pool.get(), 0, width, height, stride,
                                                                  WaylandProtocol::wlShmFormatARGB8888));

    if (buffer->handle == nullptr)
        return nullptr;

    return buffer;
}

void copyImageRegionsToWaylandShmBuffer (const Image& source,
                                         WaylandShmBuffer& destination,
                                         const RectangleList<int>& regions,
                                         std::optional<uint8> alphaMultiplier)
{
    Image::BitmapData sourceData (source, Image::BitmapData::readOnly);
    auto* destinationData = destination.data.get();

    constexpr int destinationPixelStride = 4;
    const auto sourcePixelStride = sourceData.pixelStride;

    // The shared-memory buffer format is ARGB8888.
    jassert (sourcePixelStride == destinationPixelStride);

    if (sourcePixelStride != destinationPixelStride)
        return;

    const auto copyableArea = Rectangle<int> { 0, 0, destination.width, destination.height }
                              .getIntersection (Rectangle<int> { 0, 0, sourceData.width, sourceData.height });

    for (const auto& region : regions)
    {
        // A region outside past the image or buffer would read or write outside one of them.
        jassert (copyableArea.contains (region));

        const auto clipped = region.getIntersection (copyableArea);

        if (clipped.isEmpty())
            continue;

        for (int y = clipped.getY(); y < clipped.getBottom(); ++y)
        {
            auto* destinationLine = destinationData + (size_t) y * (size_t) destination.stride;
            const auto* sourceLine = sourceData.getLinePointer (y);

            if (alphaMultiplier.has_value())
            {
                for (int x = clipped.getX(); x < clipped.getRight(); ++x)
                {
                    auto pixel = *reinterpret_cast<const PixelARGB*> (sourceLine + x * sourcePixelStride);
                    pixel.multiplyAlpha ((int) *alphaMultiplier);
                    *reinterpret_cast<PixelARGB*> (destinationLine + x * destinationPixelStride) = pixel;
                }
            }
            else
            {
                std::memcpy (destinationLine + (size_t) clipped.getX() * (size_t) destinationPixelStride,
                             sourceLine + (size_t) clipped.getX() * (size_t) sourcePixelStride,
                             (size_t) clipped.getWidth() * (size_t) destinationPixelStride);
            }
        }
    }
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandShmBufferPoolTests final : public UnitTest
{
public:
    WaylandShmBufferPoolTests()
        : UnitTest ("WaylandShmBufferPool", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> topLeft { 0, 0, 10, 10 };
        const Rectangle<int> middle { 40, 40, 10, 10 };
        const Rectangle<int> bottomRight { 80, 80, 10, 10 };

        constexpr auto pooledWidth = 100;
        constexpr auto pooledHeight = 50;
        constexpr auto otherWidth = 200;
        constexpr auto otherHeight = 80;

        testCase ("Acquiring from an empty pool returns nothing", [&]
        {
            WaylandShmBufferPool pool;
            expectEquals (pool.size(), 0);
            expectEquals (pool.busyCount(), 0);
            expect (! pool.isFull());
            expect (pool.acquire (pooledWidth, pooledHeight) == nullptr);
        });

        testCase ("A newly added buffer is immediately busy", [&]
        {
            WaylandShmBufferPool pool;
            expect (pool.add (makeBuffer (pooledWidth, pooledHeight)) != nullptr);

            expectEquals (pool.size(), 1);
            expectEquals (pool.busyCount(), 1);
            expect (pool.acquire (pooledWidth, pooledHeight) == nullptr);
        });

        testCase ("An idle buffer with matching dimensions is reacquired", [&]
        {
            WaylandShmBufferPool pool;
            auto* buffer = pool.add (makeBuffer (pooledWidth, pooledHeight));
            pool.markIdleIf ([buffer] (const WaylandShmBuffer& candidate) { return &candidate == buffer; });

            expect (pool.acquire (pooledWidth, pooledHeight) == buffer);
            expectEquals (pool.size(), 1);
            expectEquals (pool.busyCount(), 1);
        });

        testCase ("A busy buffer cannot be acquired again", [&]
        {
            WaylandShmBufferPool pool;
            auto* buffer = pool.add (makeBuffer (pooledWidth, pooledHeight));
            pool.markIdleIf ([buffer] (const WaylandShmBuffer& candidate) { return &candidate == buffer; });

            // A repaint triggered from inside another repaint must not be handed the same buffer.
            expect (pool.acquire (pooledWidth, pooledHeight) == buffer);
            expect (pool.acquire (pooledWidth, pooledHeight) == nullptr);
        });

        testCase ("The pool reports full after adding two buffers", [&]
        {
            WaylandShmBufferPool pool;
            expect (! pool.isFull());

            expect (pool.add (makeBuffer (pooledWidth, pooledHeight)) != nullptr);
            expect (! pool.isFull());

            auto* second = pool.add (makeBuffer (pooledWidth, pooledHeight));
            expect (pool.isFull());
            expectEquals (pool.busyCount(), 2);

            expect (pool.acquire (pooledWidth, pooledHeight) == nullptr);

            pool.markIdleIf ([second] (const WaylandShmBuffer& candidate) { return &candidate == second; });
            expect (pool.acquire (pooledWidth, pooledHeight) == second);
        });

        testCase ("Acquiring a different size removes idle buffers", [&]
        {
            WaylandShmBufferPool pool;
            auto* idle = pool.add (makeBuffer (pooledWidth, pooledHeight));
            expect (pool.add (makeBuffer (otherWidth, otherHeight)) != nullptr);
            pool.markIdleIf ([idle] (const WaylandShmBuffer& candidate) { return &candidate == idle; });

            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 1);
            expectEquals (pool.busyCount(), 1);
        });

        testCase ("A busy buffer of different dimensions remains until marked idle", [&]
        {
            WaylandShmBufferPool pool;
            auto* buffer = pool.add (makeBuffer (pooledWidth, pooledHeight));

            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 1);

            pool.markIdleIf ([buffer] (const WaylandShmBuffer& candidate) { return &candidate == buffer; });
            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 0);
        });

        testCase ("Every buffer matched by markIdleIf is marked idle", [&]
        {
            WaylandShmBufferPool pool;
            auto* first = pool.add (makeBuffer (pooledWidth, pooledHeight));
            expect (pool.add (makeBuffer (pooledWidth, pooledHeight)) != nullptr);

            pool.markIdleIf ([first] (const WaylandShmBuffer& candidate) { return &candidate == first; });
            expectEquals (pool.busyCount(), 1);

            pool.markIdleIf ([] (const WaylandShmBuffer&) { return true; });
            expectEquals (pool.busyCount(), 0);
        });

        testCase ("Removing one buffer leaves pointers to the remaining buffers valid", [&]
        {
            WaylandShmBufferPool pool;
            auto* removed = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* kept = pool.add (makeBuffer (pooledWidth, pooledHeight));
            pool.markIdleIf ([removed] (const WaylandShmBuffer& candidate) { return &candidate == removed; });

            // Erasing the first buffer moves the second one inside the vector.
            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 1);

            pool.markIdleIf ([kept] (const WaylandShmBuffer& candidate) { return &candidate == kept; });
            expect (pool.acquire (pooledWidth, pooledHeight) == kept);
            expectEquals (kept->width, pooledWidth);
            expectEquals (kept->height, pooledHeight);
        });

        testCase ("A newly added buffer has unknown contents", [&]
        {
            WaylandShmBufferPool pool;
            auto* buffer = pool.add (makeBuffer (pooledWidth, pooledHeight));

            expect (pool.getKnownStaleRegions (*buffer) == std::nullopt);
        });

        testCase ("Buffers not owned by the pool have unknown contents", [&]
        {
            WaylandShmBufferPool pool;
            expect (pool.add (makeBuffer (pooledWidth, pooledHeight)) != nullptr);

            const auto outsider = makeBuffer (pooledWidth, pooledHeight);
            expect (pool.getKnownStaleRegions (*outsider) == std::nullopt);
        });

        testCase ("A committed buffer has no stale regions left to repaint", [&]
        {
            WaylandShmBufferPool pool;
            auto* buffer = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*buffer, RectangleList<int> { topLeft });

            const auto knownStaleRegions = pool.getKnownStaleRegions (*buffer);
            expect (knownStaleRegions.has_value());
            expect (knownStaleRegions->isEmpty());
        });

        testCase ("Committing one buffer leaves an unpainted buffer's contents unknown", [&]
        {
            WaylandShmBufferPool pool;
            auto* committed = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* unpainted = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*committed, RectangleList<int> { topLeft });

            expect (pool.getKnownStaleRegions (*unpainted) == std::nullopt);
        });

        testCase ("Successive commits add every dirty region to the inactive buffer's stale regions", [&]
        {
            WaylandShmBufferPool pool;
            auto* skipped = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* current = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*skipped, RectangleList<int> { topLeft });
            pool.recordCommit (*current, RectangleList<int> { middle });
            pool.recordCommit (*current, RectangleList<int> { bottomRight });

            const auto knownStaleRegions = pool.getKnownStaleRegions (*skipped);
            expect (knownStaleRegions.has_value());
            expect (knownStaleRegions->containsRectangle (middle));
            expect (knownStaleRegions->containsRectangle (bottomRight));
            expect (knownStaleRegions->getBounds() == middle.getUnion (bottomRight));
        });

        testCase ("Committing a buffer clears its stale regions and records the dirty regions for the other buffer", [&]
        {
            WaylandShmBufferPool pool;
            auto* front = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* back = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*front, RectangleList<int> { topLeft });
            pool.recordCommit (*back, RectangleList<int> { middle });

            const auto frontKnownStaleRegions = pool.getKnownStaleRegions (*front);
            expect (frontKnownStaleRegions.has_value());
            expect (frontKnownStaleRegions->containsRectangle (middle));
            expect (frontKnownStaleRegions->getBounds() == middle);

            const auto backKnownStaleRegions = pool.getKnownStaleRegions (*back);
            expect (backKnownStaleRegions.has_value());
            expect (backKnownStaleRegions->isEmpty());

            pool.recordCommit (*front, RectangleList<int> { bottomRight });

            const auto frontKnownStaleRegionsAfterSecondCommit = pool.getKnownStaleRegions (*front);
            expect (frontKnownStaleRegionsAfterSecondCommit.has_value());
            expect (frontKnownStaleRegionsAfterSecondCommit->isEmpty());

            const auto backKnownStaleRegionsAfterSecondCommit = pool.getKnownStaleRegions (*back);
            expect (backKnownStaleRegionsAfterSecondCommit.has_value());
            expect (backKnownStaleRegionsAfterSecondCommit->containsRectangle (bottomRight));
            expect (backKnownStaleRegionsAfterSecondCommit->getBounds() == bottomRight);
        });

        testCase ("A buffer keeps its stale regions when the other buffer is removed", [&]
        {
            WaylandShmBufferPool pool;
            auto* kept = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* removed = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*kept, RectangleList<int> { topLeft });
            pool.recordCommit (*removed, RectangleList<int> { middle });
            pool.markIdleIf ([removed] (const WaylandShmBuffer& candidate) { return &candidate == removed; });

            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 1);

            const auto knownStaleRegions = pool.getKnownStaleRegions (*kept);
            expect (knownStaleRegions.has_value());
            expect (knownStaleRegions->getBounds() == middle);
        });

        testCase ("A replacement buffer does not inherit stale regions from the removed buffer", [&]
        {
            WaylandShmBufferPool pool;
            auto* first = pool.add (makeBuffer (pooledWidth, pooledHeight));
            pool.recordCommit (*first, RectangleList<int> { topLeft });
            pool.markIdleIf ([] (const WaylandShmBuffer&) { return true; });

            expect (pool.acquire (otherWidth, otherHeight) == nullptr);
            expectEquals (pool.size(), 0);

            // A fresh allocation can reuse the removed buffer's address, so its stale regions must not be reused.
            auto* replacement = pool.add (makeBuffer (pooledWidth, pooledHeight));
            expect (pool.getKnownStaleRegions (*replacement) == std::nullopt);
        });

        testCase ("A buffer with too many stale regions is treated as having unknown contents", [&]
        {
            constexpr auto manyCommits = 40;

            WaylandShmBufferPool pool;
            auto* skipped = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* current = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*skipped, RectangleList<int> { topLeft });

            // These rectangles do not overlap, so every commit adds another stale region.
            for (int i = 0; i < manyCommits; ++i)
                pool.recordCommit (*current, RectangleList<int> { Rectangle<int> { i * 20, 0, 10, 10 } });

            expect (pool.getKnownStaleRegions (*skipped) == std::nullopt);
        });

        testCase ("An inactive buffer records each dirty region committed to the other buffer", [&]
        {
            constexpr auto fewCommits = 4;

            WaylandShmBufferPool pool;
            auto* skipped = pool.add (makeBuffer (pooledWidth, pooledHeight));
            auto* current = pool.add (makeBuffer (pooledWidth, pooledHeight));

            pool.recordCommit (*skipped, RectangleList<int> { topLeft });

            RectangleList<int> committedRegions;

            for (int i = 0; i < fewCommits; ++i)
            {
                const Rectangle<int> region { i * 20, 0, 10, 10 };
                committedRegions.add (region);
                pool.recordCommit (*current, RectangleList<int> { region });
            }

            const auto knownStaleRegions = pool.getKnownStaleRegions (*skipped);
            expect (knownStaleRegions.has_value());
            expectEquals (knownStaleRegions->getNumRectangles(), fewCommits);

            for (const auto& region : committedRegions)
                expect (knownStaleRegions->containsRectangle (region));
        });
    }

private:
    static std::unique_ptr<WaylandShmBuffer> makeBuffer (int width, int height)
    {
        auto result = std::make_unique<WaylandShmBuffer>();
        result->width = width;
        result->height = height;
        return result;
    }
};

static WaylandShmBufferPoolTests waylandShmBufferPoolTests;

//==============================================================================
class CopyImageRegionsToWaylandShmBufferTests final : public UnitTest
{
public:
    CopyImageRegionsToWaylandShmBufferTests()
        : UnitTest ("copyImageRegionsToWaylandShmBuffer", UnitTestCategories::gui) {}

    void runTest() override
    {
        constexpr auto bufferWidth = 16;
        constexpr auto bufferHeight = 8;

        constexpr uint8 untouchedByte = 0xcd;
        constexpr auto untouchedPixel = (uint32) untouchedByte * 0x01010101u;

        const Rectangle<int> topLeft { 0, 0, 4, 3 };
        const Rectangle<int> lowerRight { 9, 4, 5, 4 };

        testCase ("Copying regions writes those pixels and leaves the rest of the buffer unchanged", [&]
        {
            const auto buffer = makeBufferWithoutCompositor (bufferWidth, bufferHeight);
            expect (buffer != nullptr);

            if (buffer == nullptr)
                return;

            std::memset (buffer->data.get(), untouchedByte, (size_t) buffer->stride * (size_t) buffer->height);

            const auto source = makeSourceImage (bufferWidth, bufferHeight);
            const Image::BitmapData sourceData (source, Image::BitmapData::readOnly);

            RectangleList<int> regions { topLeft };
            regions.add (lowerRight);

            copyImageRegionsToWaylandShmBuffer (source, *buffer, regions, std::nullopt);

            expect (bufferMatches (*buffer, [&] (int x, int y)
            {
                if (! regions.containsPoint (Point<int> { x, y }))
                    return untouchedPixel;

                return pixelBytes (*reinterpret_cast<const PixelARGB*> (sourceData.getPixelPointer (x, y)));
            }));
        });

        testCase ("The alpha multiplier is applied only to copied pixels", [&]
        {
            constexpr uint8 halfAlpha = 128;

            const auto buffer = makeBufferWithoutCompositor (bufferWidth, bufferHeight);
            expect (buffer != nullptr);

            if (buffer == nullptr)
                return;

            std::memset (buffer->data.get(), untouchedByte, (size_t) buffer->stride * (size_t) buffer->height);

            const auto source = makeSourceImage (bufferWidth, bufferHeight);
            const Image::BitmapData sourceData (source, Image::BitmapData::readOnly);

            RectangleList<int> regions { topLeft };
            regions.add (lowerRight);

            copyImageRegionsToWaylandShmBuffer (source, *buffer, regions, halfAlpha);

            expect (bufferMatches (*buffer, [&] (int x, int y)
            {
                if (! regions.containsPoint (Point<int> { x, y }))
                    return untouchedPixel;

                auto expected = *reinterpret_cast<const PixelARGB*> (sourceData.getPixelPointer (x, y));
                expected.multiplyAlpha ((int) halfAlpha);
                return pixelBytes (expected);
            }));
        });
    }

private:
    template <typename GetExpectedPixel>
    static bool bufferMatches (const WaylandShmBuffer& buffer, GetExpectedPixel&& getExpectedPixel)
    {
        for (int y = 0; y < buffer.height; ++y)
        {
            for (int x = 0; x < buffer.width; ++x)
            {
                if (getExpectedPixel (x, y) != readPixel (buffer.data.get(), buffer.stride, x, y))
                    return false;
            }
        }

        return true;
    }

    static std::unique_ptr<WaylandShmBuffer> makeBufferWithoutCompositor (int width, int height)
    {
        const auto dataSize = (size_t) width * (size_t) height * 4u;
        auto* mapped = mmap (nullptr, dataSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (mapped == MAP_FAILED)
            return nullptr;

        auto buffer = std::make_unique<WaylandShmBuffer>();
        buffer->data = { static_cast<uint8*> (mapped), WaylandShmBuffer::Unmapper { dataSize } };
        buffer->width = width;
        buffer->height = height;
        buffer->stride = width * 4;
        return buffer;
    }

    // Each pixel differs from its neighbours, so a copy that lands at the wrong offset is visible.
    static Image makeSourceImage (int width, int height)
    {
        Image image (Image::ARGB, width, height, false);
        const Image::BitmapData data (image, Image::BitmapData::writeOnly);

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const auto value = (uint8) (y * width + x);
                *reinterpret_cast<PixelARGB*> (data.getPixelPointer (x, y))
                    = PixelARGB { 0xff, value, (uint8) (value + 1), (uint8) (value + 2) };
            }
        }

        return image;
    }

    static uint32 readPixel (const uint8* data, int stride, int x, int y)
    {
        uint32 pixel = 0;
        std::memcpy (&pixel, data + (size_t) y * (size_t) stride + (size_t) x * 4u, sizeof (pixel));
        return pixel;
    }

    static uint32 pixelBytes (PixelARGB pixel)
    {
        uint32 bytes = 0;
        std::memcpy (&bytes, &pixel, sizeof (bytes));
        return bytes;
    }
};

static CopyImageRegionsToWaylandShmBufferTests copyImageRegionsToWaylandShmBufferTests;

#endif

} // namespace juce
