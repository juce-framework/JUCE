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

static RectangleList<int> computeRegionsToPaint (const RectangleList<int>& dirtyRegions,
                                                 const std::optional<RectangleList<int>>& knownStaleRegions,
                                                 bool scratchImageWasRecreated,
                                                 Rectangle<int> logicalBounds)
{
    if (! knownStaleRegions.has_value() || scratchImageWasRecreated)
        return logicalBounds;

    auto result = dirtyRegions;
    result.add (*knownStaleRegions);
    return result;
}

static RectangleList<int> computeRegionsToReportAsDamage (const RectangleList<int>& dirtyBufferRegions,
                                                          const std::optional<RectangleList<int>>& knownStaleRegions,
                                                          Rectangle<int> bufferBounds)
{
    if (! knownStaleRegions.has_value())
        return bufferBounds;

    // Discard repaint requests that do not overlap the current window before calling this function.
    jassert (! dirtyBufferRegions.isEmpty());

    if (dirtyBufferRegions.isEmpty())
        return bufferBounds;

    return dirtyBufferRegions;
}

//==============================================================================
WaylandRepaintManager::WaylandRepaintManager (Delegate& delegateIn,
                                              detail::WaylandPeerDiagnostics& diagnosticsIn)
    : delegate (delegateIn), diagnostics (diagnosticsIn)
{
}

void WaylandRepaintManager::repaint (Rectangle<int> area)
{
    if (area.isEmpty())
        return;

    ++diagnostics.repaintsRequested;

    // Repaint regions remain in logical coordinates until the next frame is painted.
    regionsNeedingRepaint.add (area);

    // A frame callback processes queued regions when the compositor allows another frame.
    if (! delegate.isFrameCallbackPending())
        triggerAsyncUpdate();
}

bool WaylandRepaintManager::hasPendingRepaints() const
{
    return ! regionsNeedingRepaint.isEmpty();
}

void WaylandRepaintManager::performAnyPendingRepaintsNow()
{
    if (isPerformingRepaint)
        return;

    const ScopedValueSetter scope { isPerformingRepaint, true };

    if (regionsNeedingRepaint.isEmpty())
        return;

    const auto frameState = delegate.getFrameState();

    if (! frameState.has_value())
        return;

    const auto bufferBounds = frameState->geometry.bufferBounds;

    if (bufferBounds.isEmpty())
        return;

    auto* buffer = acquireBuffer (bufferBounds.getWidth(), bufferBounds.getHeight());

    // Keep repaint regions queued until handleBufferRelease() makes a buffer available.
    if (buffer == nullptr)
    {
        ++diagnostics.commitsDeferredNoBuffer;
        return;
    }

    cancelPendingUpdate();

    const auto dirtyRegions = regionsNeedingRepaint;
    regionsNeedingRepaint.clear();

    const auto scratchImageWasRecreated = ensureScratchImage (bufferBounds);
    const auto knownStaleRegions = bufferPool.getKnownStaleRegions (*buffer);

    const auto logicalPaintRegions = computeRegionsToPaint (dirtyRegions, knownStaleRegions,
                                                            scratchImageWasRecreated, frameState->logicalBounds);

    const auto bufferPaintRegions = mapLogicalRegionsToBuffer (logicalPaintRegions,
                                                               frameState->logicalBounds, bufferBounds);

    delegate.paint (scratchImage, bufferPaintRegions, bufferBounds, frameState->logicalBounds);
    delegate.copyToBuffer (scratchImage, *buffer, bufferPaintRegions);

    const auto bufferDamageRegions = computeRegionsToReportAsDamage (
        mapLogicalRegionsToBuffer (dirtyRegions, frameState->logicalBounds, bufferBounds),
        knownStaleRegions, bufferBounds);

    recordCommitDiagnostics (bufferDamageRegions, bufferBounds);
    bufferPool.recordCommit (*buffer, dirtyRegions);

    delegate.submitFrame (frameState->geometry, *buffer, bufferDamageRegions);

    startTimer (3000);
}

int WaylandRepaintManager::getBufferPoolSize() const noexcept
{
    return bufferPool.size();
}

int WaylandRepaintManager::getBusyBufferCount() const noexcept
{
    return bufferPool.busyCount();
}

void WaylandRepaintManager::handleAsyncUpdate()
{
    performAnyPendingRepaintsNow();
}

void WaylandRepaintManager::timerCallback()
{
    // Like X11 does, free the scratch image when the window has been idle
    stopTimer();
    scratchImage = Image();
}

WaylandShmBuffer* WaylandRepaintManager::acquireBuffer (int width, int height)
{
    if (auto* existing = bufferPool.acquire (width, height))
        return existing;

    if (bufferPool.isFull())
        return nullptr;

    auto buffer = delegate.createBuffer (width, height, &bufferReleaseListener, this);

    if (buffer == nullptr)
        return nullptr;

    if (auto* added = bufferPool.add (std::move (buffer)))
    {
        ++diagnostics.buffersCreatedTotal;
        return added;
    }

    return nullptr;
}

RectangleList<int> WaylandRepaintManager::mapLogicalRegionsToBuffer (const RectangleList<int>& logicalRegions,
                                                                     Rectangle<int> logicalBounds,
                                                                     Rectangle<int> bufferBounds)
{
    RectangleList<int> result;

    for (const auto& region : logicalRegions)
    {
        const auto mapped = WaylandSurfaceScale::mapLogicalRectToBuffer (region, logicalBounds, bufferBounds);
        result.add (mapped.getIntersection (bufferBounds));
    }

    return result;
}

bool WaylandRepaintManager::ensureScratchImage (Rectangle<int> bufferBounds)
{
    const auto needsRecreating = scratchImage.isNull()
                              || scratchImage.getWidth() < bufferBounds.getWidth()
                              || scratchImage.getHeight() < bufferBounds.getHeight();

    if (needsRecreating)
        scratchImage = Image (Image::ARGB, bufferBounds.getWidth(), bufferBounds.getHeight(), false);

    return needsRecreating;
}

void WaylandRepaintManager::recordCommitDiagnostics (const RectangleList<int>& damageRegions,
                                                     Rectangle<int> bufferBounds)
{
    ++diagnostics.commitsSubmitted;
    diagnostics.lastCommitDamageRectCount = damageRegions.getNumRectangles();
    diagnostics.lastCommitDamageArea = 0;
    diagnostics.lastCommitBufferWidth = bufferBounds.getWidth();
    diagnostics.lastCommitBufferHeight = bufferBounds.getHeight();

    for (const auto& region : damageRegions)
        diagnostics.lastCommitDamageArea += (int64) region.getWidth() * (int64) region.getHeight();
}

void WaylandRepaintManager::handleBufferRelease (wl_buffer* released)
{
    bufferPool.markIdleIf ([released] (const WaylandShmBuffer& buffer)
    {
        return buffer.handle.get() == released;
    });
    performAnyPendingRepaintsNow();
}

// The manager owns its buffers and destroys them in its destructor. That turns their
// proxies into zombies, so this listener can never fire on a dead manager.
const wl_buffer_listener WaylandRepaintManager::bufferReleaseListener
{
    [] (void* data, wl_buffer* buffer)
    {
        static_cast<WaylandRepaintManager*> (data)->handleBufferRelease (buffer);
    }
};

#if JUCE_UNIT_TESTS

//==============================================================================
class WaylandRepaintManagerTests final : public UnitTest
{
public:
    WaylandRepaintManagerTests()
        : UnitTest ("WaylandRepaintManager", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> initialRepaint { 10, 10, 20, 20 };
        const Rectangle<int> secondRepaint { 40, 10, 20, 20 };
        const Rectangle<int> thirdRepaint { 70, 10, 20, 20 };

        testCase ("A queued repaint paints, copies, and submits one frame", [&]
        {
            detail::WaylandPeerDiagnostics diagnostics;
            RecordingDelegate delegate;
            WaylandRepaintManager manager (delegate, diagnostics);

            manager.repaint (initialRepaint);
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.buffersCreated, 1);
            expectEquals (delegate.paintCalls, 1);
            expectEquals (delegate.copyCalls, 1);
            expectEquals (delegate.submitCalls, 1);
            expect (delegate.operations == std::vector<Operation> { Operation::createBuffer,
                                                                    Operation::paint,
                                                                    Operation::copy,
                                                                    Operation::submit });
            expect (delegate.lastPaintRegions.getBounds() == delegate.bufferBounds);
            expect (delegate.lastDamageRegions.getBounds() == delegate.bufferBounds);
            expectEquals (manager.getBufferPoolSize(), 1);
            expectEquals (manager.getBusyBufferCount(), 1);
            expect (! manager.hasPendingRepaints());
            expect (delegate.frameCallbackPending);
        });

        testCase ("A pending frame callback delays repainting", [&]
        {
            detail::WaylandPeerDiagnostics diagnostics;
            RecordingDelegate delegate;
            WaylandRepaintManager manager (delegate, diagnostics);

            delegate.frameCallbackPending = true;
            manager.repaint (initialRepaint);
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.buffersCreated, 0);
            expectEquals (delegate.paintCalls, 0);
            expectEquals (delegate.copyCalls, 0);
            expectEquals (delegate.submitCalls, 0);
            expect (manager.hasPendingRepaints());

            delegate.frameCallbackPending = false;
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.paintCalls, 1);
            expectEquals (delegate.copyCalls, 1);
            expectEquals (delegate.submitCalls, 1);
            expect (! manager.hasPendingRepaints());
        });

        testCase ("A repaint remains queued while both buffers are busy", [&]
        {
            detail::WaylandPeerDiagnostics diagnostics;
            RecordingDelegate delegate;
            WaylandRepaintManager manager (delegate, diagnostics);

            manager.repaint (initialRepaint);
            manager.performAnyPendingRepaintsNow();

            delegate.frameCallbackPending = false;
            manager.repaint (secondRepaint);
            manager.performAnyPendingRepaintsNow();

            delegate.frameCallbackPending = false;
            manager.repaint (thirdRepaint);
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.buffersCreated, 2);
            expectEquals (delegate.paintCalls, 2);
            expectEquals (delegate.copyCalls, 2);
            expectEquals (delegate.submitCalls, 2);
            expectEquals (manager.getBufferPoolSize(), 2);
            expectEquals (manager.getBusyBufferCount(), 2);
            expectEquals (diagnostics.commitsDeferredNoBuffer, (uint64) 1);
            expect (manager.hasPendingRepaints());
        });

        testCase ("A nested repaint waits until the current repaint completes", [&]
        {
            detail::WaylandPeerDiagnostics diagnostics;
            RecordingDelegate delegate;
            WaylandRepaintManager manager (delegate, diagnostics);
            auto requestedNestedRepaint = false;

            delegate.onPaint = [&]
            {
                if (std::exchange (requestedNestedRepaint, true))
                    return;

                manager.repaint (secondRepaint);
                manager.performAnyPendingRepaintsNow();
            };

            manager.repaint (initialRepaint);
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.paintCalls, 1);
            expectEquals (delegate.copyCalls, 1);
            expectEquals (delegate.submitCalls, 1);
            expect (manager.hasPendingRepaints());

            delegate.frameCallbackPending = false;
            manager.performAnyPendingRepaintsNow();

            expectEquals (delegate.paintCalls, 2);
            expectEquals (delegate.copyCalls, 2);
            expectEquals (delegate.submitCalls, 2);
            expect (! manager.hasPendingRepaints());
        });
    }

private:
    enum class Operation
    {
        createBuffer,
        paint,
        copy,
        submit
    };

    struct RecordingDelegate final : public WaylandRepaintManager::Delegate
    {
        bool isFrameCallbackPending() const override
        {
            return frameCallbackPending;
        }

        std::optional<WaylandRepaintManager::FrameState> getFrameState() const override
        {
            if (frameCallbackPending)
                return std::nullopt;

            return WaylandRepaintManager::FrameState { logicalBounds, { bufferBounds, 1, std::nullopt } };
        }

        std::unique_ptr<WaylandShmBuffer> createBuffer (int width, int height,
                                                        const wl_buffer_listener*, void*) override
        {
            auto result = std::make_unique<WaylandShmBuffer>();
            result->width = width;
            result->height = height;
            result->stride = width * 4;

            ++buffersCreated;
            operations.push_back (Operation::createBuffer);
            return result;
        }

        void paint (Image&, const RectangleList<int>& paintRegions,
                    Rectangle<int>, Rectangle<int>) override
        {
            ++paintCalls;
            operations.push_back (Operation::paint);
            lastPaintRegions = paintRegions;
            NullCheckedInvocation::invoke (onPaint);
        }

        void copyToBuffer (const Image&, WaylandShmBuffer&,
                           const RectangleList<int>&) override
        {
            ++copyCalls;
            operations.push_back (Operation::copy);
        }

        void submitFrame (const WaylandSurfaceScale::BufferGeometry&,
                          WaylandShmBuffer&,
                          const RectangleList<int>& damageRegions) override
        {
            ++submitCalls;
            operations.push_back (Operation::submit);
            lastDamageRegions = damageRegions;
            frameCallbackPending = true;
        }

        Rectangle<int> logicalBounds { 0, 0, 100, 100 };
        Rectangle<int> bufferBounds { 0, 0, 100, 100 };
        RectangleList<int> lastPaintRegions;
        RectangleList<int> lastDamageRegions;
        std::vector<Operation> operations;
        std::function<void()> onPaint;
        int buffersCreated = 0;
        int paintCalls = 0;
        int copyCalls = 0;
        int submitCalls = 0;
        bool frameCallbackPending = false;
    };
};

static WaylandRepaintManagerTests waylandRepaintManagerTests;

//==============================================================================
class ComputeRegionsToPaintTests final : public UnitTest
{
public:
    ComputeRegionsToPaintTests()
        : UnitTest ("computeRegionsToPaint", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> logicalBounds { 0, 0, 100, 100 };
        const Rectangle<int> topLeft { 0, 0, 10, 10 };
        const Rectangle<int> middle { 40, 40, 10, 10 };

        constexpr auto scratchImageWasRecreated = true;
        constexpr auto scratchImageWasKept = false;

        testCase ("A buffer with no stale regions paints only its dirty regions", [&]
        {
            const RectangleList<int> dirtyRegions { topLeft };
            const std::optional<RectangleList<int>> knownStaleRegions { RectangleList<int>{} };

            const auto regions = computeRegionsToPaint (dirtyRegions, knownStaleRegions,
                                                        scratchImageWasKept, logicalBounds);
            expect (regions.getBounds() == topLeft);
        });

        testCase ("A buffer paints both stale and dirty regions", [&]
        {
            const RectangleList<int> dirtyRegions { topLeft };
            const std::optional<RectangleList<int>> knownStaleRegions { middle };

            const auto regions = computeRegionsToPaint (dirtyRegions, knownStaleRegions,
                                                        scratchImageWasKept, logicalBounds);
            expect (regions.containsRectangle (topLeft));
            expect (regions.containsRectangle (middle));
            expect (regions.getBounds() == topLeft.getUnion (middle));
        });

        testCase ("Unknown buffer contents force a full paint even for a small dirty region", [&]
        {
            const RectangleList<int> dirtyRegions { topLeft };

            const auto regions = computeRegionsToPaint (dirtyRegions, std::nullopt,
                                                        scratchImageWasKept, logicalBounds);
            expect (regions.containsRectangle (logicalBounds));
            expect (regions.getBounds() == logicalBounds);
        });

        testCase ("Recreating the scratch image forces a full paint even when buffer contents are known", [&]
        {
            const RectangleList<int> dirtyRegions { topLeft };
            const std::optional<RectangleList<int>> knownStaleRegions { middle };

            const auto regions = computeRegionsToPaint (dirtyRegions, knownStaleRegions,
                                                        scratchImageWasRecreated, logicalBounds);
            expect (regions.containsRectangle (logicalBounds));
            expect (regions.getBounds() == logicalBounds);
        });
    }
};

static ComputeRegionsToPaintTests computeRegionsToPaintTests;

//==============================================================================
class ComputeRegionsToReportAsDamageTests final : public UnitTest
{
public:
    ComputeRegionsToReportAsDamageTests()
        : UnitTest ("computeRegionsToReportAsDamage", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> bufferBounds { 0, 0, 200, 100 };
        const Rectangle<int> topLeft { 0, 0, 10, 10 };
        const Rectangle<int> middle { 40, 40, 10, 10 };

        testCase ("Stale buffer regions do not expand compositor damage", [&]
        {
            const RectangleList<int> dirtyBufferRegions { topLeft };
            const std::optional<RectangleList<int>> knownStaleRegions { middle };

            const auto regions = computeRegionsToReportAsDamage (dirtyBufferRegions, knownStaleRegions, bufferBounds);
            expect (regions.getBounds() == topLeft);
        });

        testCase ("Unknown buffer contents make the peer report the full buffer as damaged", [&]
        {
            const RectangleList<int> dirtyBufferRegions { topLeft };

            const auto regions = computeRegionsToReportAsDamage (dirtyBufferRegions, std::nullopt, bufferBounds);
            expect (regions.getBounds() == bufferBounds);
        });
    }
};

static ComputeRegionsToReportAsDamageTests computeRegionsToReportAsDamageTests;

#endif

} // namespace juce
