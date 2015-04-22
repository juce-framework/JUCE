/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

struct CustomMouseCursorInfo
{
    CustomMouseCursorInfo (const Image& im, int hsX, int hsY) noexcept
        : image (im), hotspot (hsX, hsY), scaleFactor (1.0f)
    {}

    CustomMouseCursorInfo (const Image& im, Point<int> hs, float scale) noexcept
        : image (im), hotspot (hs), scaleFactor (scale)
    {}

    void* create() const;

    Image image;
    const Point<int> hotspot;
    float scaleFactor;

private:
    JUCE_DECLARE_NON_COPYABLE (CustomMouseCursorInfo)
};

class MouseCursor::SharedCursorHandle
{
public:
    explicit SharedCursorHandle (const MouseCursor::StandardCursorType type)
        : handle (createStandardMouseCursor (type)),
          refCount (1),
          standardType (type),
          isStandard (true)
    {
    }

    SharedCursorHandle (const Image& image, Point<int> hotSpot, const float scaleFactor)
        : handle (CustomMouseCursorInfo (image, hotSpot, scaleFactor).create()),
          refCount (1),
          standardType (MouseCursor::NormalCursor),
          isStandard (false)
    {
    }

    ~SharedCursorHandle()
    {
        deleteMouseCursor (handle, isStandard);
    }

    static SharedCursorHandle* createStandard (const MouseCursor::StandardCursorType type)
    {
        jassert (isPositiveAndBelow (type, MouseCursor::NumStandardCursorTypes));

        const SpinLock::ScopedLockType sl (lock);

        SharedCursorHandle*& c = getSharedCursor (type);

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

    void* getHandle() const noexcept        { return handle; }

private:
    void* const handle;
    Atomic <int> refCount;
    const MouseCursor::StandardCursorType standardType;
    const bool isStandard;
    static SpinLock lock;

    static SharedCursorHandle*& getSharedCursor (const MouseCursor::StandardCursorType type)
    {
        static SharedCursorHandle* cursors [MouseCursor::NumStandardCursorTypes] = {};
        return cursors [type];
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedCursorHandle)
};

SpinLock MouseCursor::SharedCursorHandle::lock;

//==============================================================================
MouseCursor::MouseCursor() noexcept
    : cursorHandle (nullptr)
{
}

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (type != MouseCursor::NormalCursor ? SharedCursorHandle::createStandard (type) : nullptr)
{
}

MouseCursor::MouseCursor (const Image& image, const int hotSpotX, const int hotSpotY)
    : cursorHandle (new SharedCursorHandle (image, Point<int> (hotSpotX, hotSpotY), 1.0f))
{
}

MouseCursor::MouseCursor (const Image& image, const int hotSpotX, const int hotSpotY, float scaleFactor)
    : cursorHandle (new SharedCursorHandle (image, Point<int> (hotSpotX, hotSpotY), scaleFactor))
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

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
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
#endif

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
