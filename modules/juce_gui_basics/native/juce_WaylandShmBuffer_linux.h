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

struct WaylandShmBuffer
{
    struct Unmapper
    {
        // GCC 7 cannot instantiate the unique_ptr below while a nested deleter still has an
        // unparsed default member initialiser, so the size is set by constructors instead.
        Unmapper() noexcept : Unmapper (0) {}
        explicit Unmapper (size_t sizeIn) noexcept : size (sizeIn) {}

        void operator() (uint8* ptr) const
        {
            if (ptr != nullptr)
                munmap (ptr, size);
        }

        size_t size;
    };

    WaylandShmBuffer() = default;

    static bool canCreate();
    static std::unique_ptr<WaylandShmBuffer> create (wl_shm&, int width, int height);

    std::unique_ptr<uint8, Unmapper> data { nullptr, Unmapper{} };
    std::unique_ptr<wl_buffer, FunctionPointerDestructor<WaylandProtocol::wlBufferDestroy>> handle;
    int width = 0;
    int height = 0;
    int stride = 0;

    JUCE_DECLARE_NON_COPYABLE (WaylandShmBuffer)
};

// Regions are in buffer pixel coordinates and must fit inside both the image and the buffer.
void copyImageRegionsToWaylandShmBuffer (const Image& source,
                                         WaylandShmBuffer& destination,
                                         const RectangleList<int>& regions,
                                         std::optional<uint8> alphaMultiplier);

//==============================================================================
class WaylandShmBufferPool final
{
public:
    WaylandShmBufferPool() = default;

    [[nodiscard]] WaylandShmBuffer* acquire (int width, int height)
    {
        const auto isIdleBufferOfAnotherSize = [width, height] (const Slot& slot)
        {
            return ! slot.busy && (slot.buffer->width != width || slot.buffer->height != height);
        };

        slots.erase (std::remove_if (slots.begin(), slots.end(), isIdleBufferOfAnotherSize),
                     slots.end());

        for (auto& slot : slots)
        {
            if (! slot.busy && slot.buffer->width == width && slot.buffer->height == height)
            {
                slot.busy = true;
                return slot.buffer.get();
            }
        }

        return nullptr;
    }

    bool isFull() const noexcept    { return maxSlots <= slots.size(); }

    [[nodiscard]] WaylandShmBuffer* add (std::unique_ptr<WaylandShmBuffer> buffer)
    {
        if (isFull())
        {
            // If this assertion is hit, the caller tried to add an unnecessary buffer.
            // This should never happen, so please report an issue on the JUCE forum if you encounter this assertion.
            jassertfalse;
            return nullptr;
        }

        slots.push_back (Slot { std::move (buffer) });
        return slots.back().buffer.get();
    }

    // wl_buffer.release only carries the handle, so the caller does the matching.
    template <typename Predicate>
    void markIdleIf (Predicate&& shouldMarkIdle)
    {
        for (auto& slot : slots)
            if (shouldMarkIdle (std::as_const (*slot.buffer)))
                slot.busy = false;
    }

    // A value lists every region that changed after this buffer's last commit.
    // No value means that the buffer contents are unknown, so the buffer must be repainted in full.
    // Regions use logical coordinates.
    std::optional<RectangleList<int>> getKnownStaleRegions (const WaylandShmBuffer& buffer) const
    {
        for (const auto& slot : slots)
            if (slot.buffer.get() == &buffer)
                return slot.knownStaleRegions;

        return std::nullopt;
    }

    void recordCommit (const WaylandShmBuffer& buffer, const RectangleList<int>& dirtyRegions)
    {
        const auto committed = std::find_if (slots.begin(),
                                             slots.end(),
                                             [&buffer] (const Slot& slot) { return slot.buffer.get() == &buffer; });

        if (committed == slots.end())
        {
            // If this assertion is hit, the caller passed a buffer that this pool does not hold, so
            // the commit cannot be recorded. Pass a buffer that came from acquire or add.
            jassertfalse;
            return;
        }

        committed->knownStaleRegions = RectangleList<int>{};

        // Changes are only tracked for buffers with known contents.
        for (auto& slot : slots)
        {
            if (slot.buffer.get() != &buffer && slot.knownStaleRegions.has_value())
            {
                slot.knownStaleRegions->add (dirtyRegions);

                if (maxStaleRectangles < slot.knownStaleRegions->getNumRectangles())
                    slot.knownStaleRegions.reset();
            }
        }
    }

    int size() const noexcept       { return (int) slots.size(); }

    int busyCount() const noexcept
    {
        return (int) std::count_if (slots.begin(),
                                    slots.end(),
                                    [] (const Slot& slot) { return slot.busy; });
    }

private:
    // Two buffers allow a new frame to be prepared while the compositor uses the previous one.
    // If both are busy, repainting waits until one is released.
    static constexpr size_t maxSlots = 2;

    // Once this many regions are dirty, repainting the full buffer is more efficient.
    static constexpr int maxStaleRectangles = 32;

    struct Slot
    {
        explicit Slot (std::unique_ptr<WaylandShmBuffer> b) : buffer (std::move (b)) {}

        std::unique_ptr<WaylandShmBuffer> buffer;

        std::optional<RectangleList<int>> knownStaleRegions;

        // Slots start busy because add hands the buffer straight to its caller.
        bool busy = true;
    };

    // Held through unique_ptr so the raw pointers returned by acquire and add
    // stay valid while the vector erases slots or grows.
    std::vector<Slot> slots;

    JUCE_DECLARE_NON_COPYABLE (WaylandShmBufferPool)
};

} // namespace juce
