/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

// Hack to forward declare _XDisplay outside the
// juce namespace

}

struct _XDisplay;

#define ATOM_TYPE unsigned long
#define WINDOW_TYPE unsigned long

namespace juce {

//==============================================================================
class XWindowSystem
{
public:
    ::_XDisplay* displayRef() noexcept;
    ::_XDisplay* displayUnref() noexcept;
    juce_DeclareSingleton (XWindowSystem, false)

private:
    ::_XDisplay* display;
    Atomic<int> displayCount;

    XWindowSystem() noexcept;
    ~XWindowSystem() noexcept;

    void initialiseXDisplay() noexcept;
    void destroyXDisplay() noexcept;
};

//==============================================================================
class ScopedXDisplay
{
public:
    ScopedXDisplay();
    ~ScopedXDisplay();
    ::_XDisplay* get();
private:
    ::_XDisplay* display;
};

/** A handy class that uses XLockDisplay and XUnlockDisplay to lock the X server
    using RAII (Only available in Linux!).
*/
class ScopedXLock
{
public:
    /** Creating a ScopedXLock object locks the X display.
        This uses XLockDisplay() to grab the display that Juce is using.
    */
    ScopedXLock (::_XDisplay* _display);

    /** Deleting a ScopedXLock object unlocks the X display.
        This calls XUnlockDisplay() to release the lock.
    */
    ~ScopedXLock();
private:
    // defined in juce_linux_X11.h
    ::_XDisplay* display;
};

//==============================================================================
struct Atoms
{
    Atoms(::_XDisplay* display);

    enum ProtocolItems
    {
        TAKE_FOCUS = 0,
        DELETE_WINDOW = 1,
        PING = 2
    };

    ATOM_TYPE protocols, protocolList[3], changeState, state, userTime,
         activeWin, pid, windowType, windowState,
         XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus,
         XdndDrop, XdndFinished, XdndSelection, XdndTypeList, XdndActionList,
         XdndActionDescription, XdndActionCopy, XdndActionPrivate,
         XembedMsgType, XembedInfo,
         allowedActions[5],
         allowedMimeTypes[4];

    static const unsigned long DndVersion;

    static ATOM_TYPE getIfExists (::_XDisplay* display, const char* name);
    static ATOM_TYPE getCreating (::_XDisplay* display, const char* name);

    static String getName (::_XDisplay* display, const ATOM_TYPE atom);

    static bool isMimeTypeFile (::_XDisplay* display, const ATOM_TYPE atom);
};

//==============================================================================
struct GetXProperty
{
    GetXProperty (::_XDisplay* display, WINDOW_TYPE window, ATOM_TYPE atom,
                  long offset, long length, bool shouldDelete,
                  ATOM_TYPE requestedType);

    ~GetXProperty();

    bool success;
    unsigned char* data;
    unsigned long numItems, bytesLeft;
    ATOM_TYPE actualType;
    int actualFormat;
};

#undef ATOM_TYPE
