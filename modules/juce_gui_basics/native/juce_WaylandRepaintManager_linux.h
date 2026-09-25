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

class WaylandRepaintManager final : private AsyncUpdater,
                                    private Timer
{
public:
    struct FrameState
    {
        Rectangle<int> logicalBounds;
        WaylandSurfaceScale::BufferGeometry geometry;
    };

    struct Delegate
    {
        virtual ~Delegate() = default;

        virtual bool isFrameCallbackPending() const = 0;
        virtual std::optional<FrameState> getFrameState() const = 0;
        virtual std::unique_ptr<WaylandShmBuffer> createBuffer (int width, int height,
                                                                const wl_buffer_listener* listener,
                                                                void* listenerData) = 0;
        virtual void paint (Image& target, const RectangleList<int>& paintRegions,
                            Rectangle<int> bufferBounds, Rectangle<int> logicalBounds) = 0;
        virtual void copyToBuffer (const Image& source, WaylandShmBuffer& buffer,
                                   const RectangleList<int>& paintRegions) = 0;
        virtual void submitFrame (const WaylandSurfaceScale::BufferGeometry& geometry,
                                  WaylandShmBuffer& buffer,
                                  const RectangleList<int>& damageRegions) = 0;
    };

    WaylandRepaintManager (Delegate& delegate, detail::WaylandPeerDiagnostics& diagnostics);

    void repaint (Rectangle<int> area);
    bool hasPendingRepaints() const;
    void performAnyPendingRepaintsNow();

    int getBufferPoolSize() const noexcept;
    int getBusyBufferCount() const noexcept;

private:
    void handleAsyncUpdate() override;
    void timerCallback() override;

    [[nodiscard]] WaylandShmBuffer* acquireBuffer (int width, int height);

    static RectangleList<int> mapLogicalRegionsToBuffer (const RectangleList<int>& logicalRegions,
                                                         Rectangle<int> logicalBounds,
                                                         Rectangle<int> bufferBounds);

    [[nodiscard]] bool ensureScratchImage (Rectangle<int> bufferBounds);
    void recordCommitDiagnostics (const RectangleList<int>& damageRegions, Rectangle<int> bufferBounds);
    void handleBufferRelease (wl_buffer* released);

    static const wl_buffer_listener bufferReleaseListener;

    Delegate& delegate;
    detail::WaylandPeerDiagnostics& diagnostics;
    WaylandShmBufferPool bufferPool;
    RectangleList<int> regionsNeedingRepaint;
    Image scratchImage;
    bool isPerformingRepaint = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaylandRepaintManager)
};

} // namespace juce
