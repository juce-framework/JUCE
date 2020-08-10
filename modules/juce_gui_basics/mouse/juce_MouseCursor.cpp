/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct CustomMouseCursorInfo
{
    CustomMouseCursorInfo (const Image& im, Point<int> hs, float scale = 1.0f) noexcept
        : image (im), hotspot (hs), scaleFactor (scale)
    {}

    void* create() const;

    Image image;
    const Point<int> hotspot;
    const float scaleFactor;

    JUCE_DECLARE_NON_COPYABLE (CustomMouseCursorInfo)
};

class MouseCursor::SharedCursorHandle
{
public:
    explicit SharedCursorHandle (const MouseCursor::StandardCursorType type)
        : handle (createStandardMouseCursor (type)),
          standardType (type),
          isStandard (true)
    {
    }

    SharedCursorHandle (const Image& image, Point<int> hotSpot, float scaleFactor)
        : info (new CustomMouseCursorInfo (image, hotSpot, scaleFactor)),
          handle (info->create()),
          standardType (MouseCursor::NormalCursor),
          isStandard (false)
    {
        // your hotspot needs to be within the bounds of the image!
        jassert (image.getBounds().contains (hotSpot));
    }

    ~SharedCursorHandle()
    {
        deleteMouseCursor (handle, isStandard);
    }

    static SharedCursorHandle* createStandard (const MouseCursor::StandardCursorType type)
    {
        jassert (isPositiveAndBelow (type, MouseCursor::NumStandardCursorTypes));

        const SpinLock::ScopedLockType sl (lock);
        auto& c = getSharedCursor (type);

        if (c == nullptr)
            c = new SharedCursorHandle (type);
        else
            c->retain();

        return c;
    }

    bool isStandardType (MouseCursor::StandardCursorType type) const noexcept
    {
        return type == standardType && isStandard;
    }

    SharedCursorHandle* retain() noexcept
    {
        ++refCount;
        return this;
    }

    void release()
    {
        if (--refCount == 0)
        {
            if (isStandard)
            {
                const SpinLock::ScopedLockType sl (lock);
                getSharedCursor (standardType) = nullptr;
            }

            delete this;
        }
    }

    void* getHandle() const noexcept                            { return handle; }
    void setHandle (void* newHandle)                            { handle = newHandle; }

    MouseCursor::StandardCursorType getType() const noexcept    { return standardType; }
    CustomMouseCursorInfo* getCustomInfo() const noexcept       { return info.get(); }

private:
    std::unique_ptr<CustomMouseCursorInfo> info;
    void* handle;
    Atomic<int> refCount { 1 };
    const MouseCursor::StandardCursorType standardType;
    const bool isStandard;
    static SpinLock lock;

    static SharedCursorHandle*& getSharedCursor (const MouseCursor::StandardCursorType type)
    {
        static SharedCursorHandle* cursors[MouseCursor::NumStandardCursorTypes] = {};
        return cursors[type];
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedCursorHandle)
};

SpinLock MouseCursor::SharedCursorHandle::lock;

//==============================================================================
MouseCursor::MouseCursor() noexcept
{
}

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (type != MouseCursor::NormalCursor ? SharedCursorHandle::createStandard (type) : nullptr)
{
}

MouseCursor::MouseCursor (const Image& image, int hotSpotX, int hotSpotY)
    : MouseCursor (image, hotSpotX, hotSpotY, 1.0f)
{
}

MouseCursor::MouseCursor (const Image& image, int hotSpotX, int hotSpotY, float scaleFactor)
    : cursorHandle (new SharedCursorHandle (image, { hotSpotX, hotSpotY }, scaleFactor))
{
}

MouseCursor::MouseCursor (const MouseCursor& other)
    : cursorHandle (other.cursorHandle == nullptr ? nullptr : other.cursorHandle->retain())
{
}

MouseCursor::~MouseCursor()
{
    if (cursorHandle != nullptr)
        cursorHandle->release();
}

MouseCursor& MouseCursor::operator= (const MouseCursor& other)
{
    if (other.cursorHandle != nullptr)
        other.cursorHandle->retain();

    if (cursorHandle != nullptr)
        cursorHandle->release();

    cursorHandle = other.cursorHandle;
    return *this;
}

MouseCursor::MouseCursor (MouseCursor&& other) noexcept
    : cursorHandle (other.cursorHandle)
{
    other.cursorHandle = nullptr;
}

MouseCursor& MouseCursor::operator= (MouseCursor&& other) noexcept
{
    std::swap (cursorHandle, other.cursorHandle);
    return *this;
}

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

void* MouseCursor::getHandle() const noexcept
{
    return cursorHandle != nullptr ? cursorHandle->getHandle() : nullptr;
}

void MouseCursor::showWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().showMouseCursor (MouseCursor::WaitCursor);
}

void MouseCursor::hideWaitCursor()
{
    Desktop::getInstance().getMainMouseSource().revealCursor();
}

} // namespace juce
