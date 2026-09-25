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

Point<int> getWaylandSubsurfacePosition (Rectangle<int> childLogicalBounds,
                                         Rectangle<int> parentLogicalBounds,
                                         Point<int> parentSurfaceSize)
{
    return WaylandSurfaceScale::mapLogicalRectToSurface (childLogicalBounds,
                                                         parentLogicalBounds,
                                                         { parentSurfaceSize.x,
                                                           parentSurfaceSize.y }).getPosition();
}

std::unique_ptr<WaylandSubsurface> WaylandSubsurface::create (wl_surface& surface,
                                                              wl_surface& parent)
{
    auto* subcompositor = WaylandWindowSystem::getInstance()->getSubcompositor();

    if (subcompositor == nullptr || &surface == &parent)
        return {};

    SubsurfaceHandle handle { WaylandProtocol::wlSubcompositorGetSubsurface (subcompositor,
                                                                             &surface,
                                                                             &parent) };

    if (handle == nullptr)
        return {};

    auto result = std::unique_ptr<WaylandSubsurface> { new WaylandSubsurface (std::move (handle)) };
    result->setDesync();
    return result;
}

WaylandSubsurface::WaylandSubsurface (SubsurfaceHandle handle)
    : subsurface (std::move (handle))
{
}

void WaylandSubsurface::setPosition (Point<int> position)
{
    WaylandProtocol::wlSubsurfaceSetPosition (subsurface.get(), position.x, position.y);
}

void WaylandSubsurface::setSync()
{
    WaylandProtocol::wlSubsurfaceSetSync (subsurface.get());
}

void WaylandSubsurface::setDesync()
{
    WaylandProtocol::wlSubsurfaceSetDesync (subsurface.get());
}

//==============================================================================
#if JUCE_UNIT_TESTS

class WaylandSubsurfacePositionTests final : public UnitTest
{
public:
    WaylandSubsurfacePositionTests()
        : UnitTest ("WaylandSubsurfacePosition", UnitTestCategories::gui) {}

    void runTest() override
    {
        const Rectangle<int> parentBounds { 100, 50, 400, 300 };

        testCase ("Logical child bounds are made parent-relative", [&]
        {
            expect (getWaylandSubsurfacePosition ({ 130, 90, 200, 100 },
                                                   parentBounds,
                                                   { 400, 300 }) == Point<int> { 30, 40 });
        });

        testCase ("Parent surface scaling is applied to the child position", [&]
        {
            expect (getWaylandSubsurfacePosition ({ 130, 90, 200, 100 },
                                                   parentBounds,
                                                   { 320, 240 }) == Point<int> { 24, 32 });
        });

        testCase ("A subsurface may extend above and to the left of its parent", [&]
        {
            expect (getWaylandSubsurfacePosition ({ 50, 20, 200, 100 },
                                                   parentBounds,
                                                   { 320, 240 }) == Point<int> { -40, -24 });
        });
    }
};

static WaylandSubsurfacePositionTests waylandSubsurfacePositionTests;

#endif

} // namespace juce
