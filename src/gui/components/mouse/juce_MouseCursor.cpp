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
#include "../../../threads/juce_ScopedLock.h"

void* juce_createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY) throw();
void* juce_createStandardMouseCursor (MouseCursor::StandardCursorType type) throw();
// isStandard set depending on which interface was used to create the cursor
void juce_deleteMouseCursor (void* const cursorHandle, const bool isStandard) throw();


//==============================================================================
static CriticalSection activeCursorListLock;
static VoidArray activeCursors;

//==============================================================================
class SharedMouseCursorInternal  : public ReferenceCountedObject
{
public:
    SharedMouseCursorInternal (const MouseCursor::StandardCursorType type) throw()
        : standardType (type),
          isStandard (true)
    {
        handle = juce_createStandardMouseCursor (standardType);
        activeCursors.add (this);
    }

    SharedMouseCursorInternal (const Image& image, const int hotSpotX, const int hotSpotY) throw()
        : standardType (MouseCursor::NormalCursor),
          isStandard (false)
    {
        handle = juce_createMouseCursorFromImage (image, hotSpotX, hotSpotY);
    }

    ~SharedMouseCursorInternal() throw()
    {
        juce_deleteMouseCursor (handle, isStandard);
        activeCursors.removeValue (this);
    }

    void* getHandle() const throw()
    {
        return handle;
    }

    static SharedMouseCursorInternal* findInstance (MouseCursor::StandardCursorType type) throw()
    {
        for (int i = activeCursors.size(); --i >= 0;)
        {
            SharedMouseCursorInternal* const r = (SharedMouseCursorInternal*) activeCursors.getUnchecked(i);

            if (r->standardType == type)
                return r;
        }

        return new SharedMouseCursorInternal (type);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void* handle;
    const MouseCursor::StandardCursorType standardType;
    const bool isStandard;

    const SharedMouseCursorInternal& operator= (const SharedMouseCursorInternal&);
};


//==============================================================================
MouseCursor::MouseCursor() throw()
{
    const ScopedLock sl (activeCursorListLock);
    cursorHandle = SharedMouseCursorInternal::findInstance (NormalCursor);
}

MouseCursor::MouseCursor (const StandardCursorType type) throw()
{
    const ScopedLock sl (activeCursorListLock);
    cursorHandle = SharedMouseCursorInternal::findInstance (type);
}

MouseCursor::MouseCursor (const Image& image, const int hotSpotX, const int hotSpotY) throw()
{
    const ScopedLock sl (activeCursorListLock);
    cursorHandle = new SharedMouseCursorInternal (image, hotSpotX, hotSpotY);
}

MouseCursor::MouseCursor (const MouseCursor& other) throw()
    : cursorHandle (other.cursorHandle)
{
}

MouseCursor::~MouseCursor() throw()
{
}

const MouseCursor& MouseCursor::operator= (const MouseCursor& other) throw()
{
    cursorHandle = other.cursorHandle;
    return *this;
}

bool MouseCursor::operator== (const MouseCursor& other) const throw()
{
    return cursorHandle == other.cursorHandle;
}

bool MouseCursor::operator!= (const MouseCursor& other) const throw()
{
    return cursorHandle != other.cursorHandle;
}

void* MouseCursor::getHandle() const throw()
{
    return cursorHandle->getHandle();
}

void MouseCursor::showWaitCursor() throw()
{
    const MouseCursor mc (MouseCursor::WaitCursor);
    mc.showInAllWindows();
}

void MouseCursor::hideWaitCursor() throw()
{
    Component* const c = Component::getComponentUnderMouse();

    MouseCursor mc (c->isValidComponent() ? c->getLookAndFeel().getMouseCursorFor (*c)
                                          : MouseCursor::NormalCursor);

    mc.showInAllWindows();
}

END_JUCE_NAMESPACE
