/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MouseCursor.h"
#include "../juce_Component.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../mouse/juce_MouseInputSource.h"
#include "../../../threads/juce_ScopedLock.h"

void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY);
void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type);
void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard);


//==============================================================================
class MouseCursor::SharedCursorHandle
{
public:
    explicit SharedCursorHandle (const MouseCursor::StandardCursorType type)
        : handle (juce_createStandardMouseCursor (type)),
          refCount (1),
          standardType (type),
          isStandard (true)
    {
    }

    SharedCursorHandle (const Image& image, const int hotSpotX, const int hotSpotY)
        : handle (juce_createMouseCursorFromImage (image, hotSpotX, hotSpotY)),
          refCount (1),
          standardType (MouseCursor::NormalCursor),
          isStandard (false)
    {
    }

    static SharedCursorHandle* createStandard (const MouseCursor::StandardCursorType type)
    {
        const ScopedLock sl (getLock());

        for (int i = getCursors().size(); --i >= 0;)
        {
            SharedCursorHandle* const sc = getCursors().getUnchecked(i);

            if (sc->standardType == type)
                return sc->retain();
        }

        SharedCursorHandle* const sc = new SharedCursorHandle (type);
        getCursors().add (sc);
        return sc;
    }

    SharedCursorHandle* retain() throw()
    {
        Atomic::increment (refCount);
        return this;
    }

    void release()
    {
        if (Atomic::decrementAndReturn (refCount) == 0)
        {
            if (isStandard)
            {
                const ScopedLock sl (getLock());
                getCursors().removeValue (this);
            }

            delete this;
        }
    }

    void* getHandle() const throw()         { return handle; }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void* const handle;
    int32 refCount;
    const MouseCursor::StandardCursorType standardType;
    const bool isStandard;

    static CriticalSection& getLock()
    {
        static CriticalSection lock;
        return lock;
    }

    static Array <SharedCursorHandle*>& getCursors()
    {
        static Array <SharedCursorHandle*> cursors;
        return cursors;
    }

    ~SharedCursorHandle()
    {
        juce_deleteMouseCursor (handle, isStandard);
    }

    SharedCursorHandle& operator= (const SharedCursorHandle&);
};

//==============================================================================
MouseCursor::MouseCursor()
    : cursorHandle (SharedCursorHandle::createStandard (NormalCursor))
{
    jassert (cursorHandle != 0);
}

MouseCursor::MouseCursor (const StandardCursorType type)
    : cursorHandle (SharedCursorHandle::createStandard (type))
{
    jassert (cursorHandle != 0);
}

MouseCursor::MouseCursor (const Image& image, const int hotSpotX, const int hotSpotY)
    : cursorHandle (new SharedCursorHandle (image, hotSpotX, hotSpotY))
{
}

MouseCursor::MouseCursor (const MouseCursor& other)
    : cursorHandle (other.cursorHandle->retain())
{
}

MouseCursor::~MouseCursor()
{
    cursorHandle->release();
}

MouseCursor& MouseCursor::operator= (const MouseCursor& other)
{
    other.cursorHandle->retain();
    cursorHandle->release();
    cursorHandle = other.cursorHandle;
    return *this;
}

bool MouseCursor::operator== (const MouseCursor& other) const throw()
{
    return getHandle() == other.getHandle();
}

bool MouseCursor::operator!= (const MouseCursor& other) const throw()
{
    return getHandle() != other.getHandle();
}

void* MouseCursor::getHandle() const throw()
{
    return cursorHandle->getHandle();
}

void MouseCursor::showWaitCursor() throw()
{
    Desktop::getInstance().getMainMouseSource().showMouseCursor (MouseCursor::WaitCursor);
}

void MouseCursor::hideWaitCursor() throw()
{
    Desktop::getInstance().getMainMouseSource().revealCursor();
}

END_JUCE_NAMESPACE
