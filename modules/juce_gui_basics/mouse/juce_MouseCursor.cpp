/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

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

    SharedCursorHandle (const Image& image, const int hotSpotX, const int hotSpotY)
        : handle (createMouseCursorFromImage (image, hotSpotX, hotSpotY)),
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
        const SpinLock::ScopedLockType sl (lock);

        for (int i = 0; i < getCursors().size(); ++i)
        {
            SharedCursorHandle* const sc = getCursors().getUnchecked(i);

            if (sc->standardType == type)
                return sc->retain();
        }

        SharedCursorHandle* const sc = new SharedCursorHandle (type);
        getCursors().add (sc);
        return sc;
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
                getCursors().removeFirstMatchingValue (this);
            }

            delete this;
        }
    }

    void* getHandle() const noexcept        { return handle; }


private:
    //==============================================================================
    void* const handle;
    Atomic <int> refCount;
    const MouseCursor::StandardCursorType standardType;
    const bool isStandard;
    static SpinLock lock;

    static Array <SharedCursorHandle*>& getCursors()
    {
        static Array <SharedCursorHandle*> cursors;
        return cursors;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SharedCursorHandle);
};

SpinLock MouseCursor::SharedCursorHandle::lock;

//==============================================================================
MouseCursor::MouseCursor()
    : cursorHandle (nullptr)
{
}

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (type != MouseCursor::NormalCursor ? SharedCursorHandle::createStandard (type) : 0)
{
}

MouseCursor::MouseCursor (const Image& image, const int hotSpotX, const int hotSpotY)
    : cursorHandle (new SharedCursorHandle (image, hotSpotX, hotSpotY))
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

bool MouseCursor::operator!= (const MouseCursor& other) const noexcept
{
    return getHandle() != other.getHandle();
}

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
