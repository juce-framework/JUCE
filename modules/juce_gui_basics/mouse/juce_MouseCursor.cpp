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

class MouseCursor::SharedCursorHandle
{
public:
    explicit SharedCursorHandle (const MouseCursor::StandardCursorType type)
        : handle (type),
          standardType (type),
          standard (true)
    {
    }

    SharedCursorHandle (const ScaledImage& image, Point<int> hotSpot)
        : info { image, hotSpot },
          handle (info),
          standardType (MouseCursor::NormalCursor),
          standard (false)
    {
        // your hotspot needs to be within the bounds of the image!
        jassert (image.getScaledBounds().toNearestInt().contains (hotSpot));
    }

    static std::shared_ptr<SharedCursorHandle> createStandard (const MouseCursor::StandardCursorType type)
    {
        if (! isPositiveAndBelow (type, MouseCursor::NumStandardCursorTypes))
            return nullptr;

        static SpinLock mutex;
        static std::array<std::weak_ptr<SharedCursorHandle>, MouseCursor::NumStandardCursorTypes> cursors;

        const SpinLock::ScopedLockType sl (mutex);

        auto& weak = cursors[type];

        if (auto strong = weak.lock())
            return strong;

        auto strong = std::make_shared<SharedCursorHandle> (type);
        weak = strong;
        return strong;
    }

    bool isStandardType (MouseCursor::StandardCursorType type) const noexcept
    {
        return type == standardType && standard;
    }

    PlatformSpecificHandle* getHandle() noexcept                { return &handle; }
    MouseCursor::StandardCursorType getType() const noexcept    { return standardType; }

private:
    detail::CustomMouseCursorInfo info;
    PlatformSpecificHandle handle;
    const MouseCursor::StandardCursorType standardType;
    const bool standard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedCursorHandle)
};

//==============================================================================
MouseCursor::MouseCursor() noexcept = default;

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (type != MouseCursor::NormalCursor ? SharedCursorHandle::createStandard (type) : nullptr)
{
}

MouseCursor::MouseCursor (const Image& image, int hotSpotX, int hotSpotY)
    : MouseCursor (ScaledImage (image), { hotSpotX, hotSpotY })
{
}

MouseCursor::MouseCursor (const Image& image, int hotSpotX, int hotSpotY, float scaleFactor)
    : MouseCursor (ScaledImage (image, scaleFactor), { hotSpotX, hotSpotY })
{
}

MouseCursor::MouseCursor (const ScaledImage& image, Point<int> hotSpot)
        : cursorHandle (std::make_shared<SharedCursorHandle> (image, hotSpot))
{
}

MouseCursor::MouseCursor (const MouseCursor&) = default;

MouseCursor::~MouseCursor() = default;

MouseCursor& MouseCursor::operator= (const MouseCursor&) = default;

MouseCursor::MouseCursor (MouseCursor&&) noexcept = default;

MouseCursor& MouseCursor::operator= (MouseCursor&&) noexcept = default;

bool MouseCursor::operator== (const MouseCursor& other) const noexcept
{
    return getHandle() == other.getHandle();
}

bool MouseCursor::operator== (StandardCursorType type) const noexcept
{
    return cursorHandle != nullptr ? cursorHandle->isStandardType (type)
                                   : (type == NormalCursor);
}

bool MouseCursor::operator!= (const MouseCursor& other) const noexcept  { return ! operator== (other); }
bool MouseCursor::operator!= (StandardCursorType type)  const noexcept  { return ! operator== (type); }

void MouseCursor::showWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().showMouseCursor (MouseCursor::WaitCursor);
}

void MouseCursor::hideWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().revealCursor();
}

MouseCursor::PlatformSpecificHandle* MouseCursor::getHandle() const noexcept
{
    return cursorHandle != nullptr ? cursorHandle->getHandle() : nullptr;
}

void MouseCursor::showInWindow (ComponentPeer* peer) const
{
    PlatformSpecificHandle::showInWindow (getHandle(), peer);
}

} // namespace juce
