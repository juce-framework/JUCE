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

struct _XDisplay;

namespace juce
{

typedef ::_XDisplay* XDisplay;

typedef unsigned long AtomType;
typedef unsigned long WindowType;


//==============================================================================
class XWindowSystem :  public DeletedAtShutdown
{
public:
    XDisplay displayRef() noexcept;
    XDisplay displayUnref() noexcept;

    JUCE_DECLARE_SINGLETON (XWindowSystem, false)

private:
    XDisplay display = {};
    Atomic<int> displayCount;

    XWindowSystem() noexcept;
    ~XWindowSystem() noexcept;

    void initialiseXDisplay() noexcept;
    void destroyXDisplay() noexcept;
};

//==============================================================================
/** Creates and holds a reference to the X display. */
struct ScopedXDisplay
{
    ScopedXDisplay();
    ~ScopedXDisplay();

    const XDisplay display;
};

//==============================================================================
/** A handy class that uses XLockDisplay and XUnlockDisplay to lock the X server
    using RAII (Only available in Linux!).
*/
class ScopedXLock
{
public:
    /** Creating a ScopedXLock object locks the X display.
        This uses XLockDisplay() to grab the display that JUCE is using.
    */
    ScopedXLock (XDisplay);

    /** Deleting a ScopedXLock object unlocks the X display.
        This calls XUnlockDisplay() to release the lock.
    */
    ~ScopedXLock();

private:
    // defined in juce_linux_X11.h
    XDisplay display;
};

//==============================================================================
struct Atoms
{
    Atoms (XDisplay);

    enum ProtocolItems
    {
        TAKE_FOCUS = 0,
        DELETE_WINDOW = 1,
        PING = 2
    };

    AtomType protocols, protocolList[3], changeState, state, userTime,
             activeWin, pid, windowType, windowState,
             XdndAware, XdndEnter, XdndLeave, XdndPosition, XdndStatus,
             XdndDrop, XdndFinished, XdndSelection, XdndTypeList, XdndActionList,
             XdndActionDescription, XdndActionCopy, XdndActionPrivate,
             XembedMsgType, XembedInfo,
             allowedActions[5],
             allowedMimeTypes[4];

    static const unsigned long DndVersion;

    static AtomType getIfExists (XDisplay, const char* name);
    static AtomType getCreating (XDisplay, const char* name);

    static String getName (XDisplay, AtomType);

    static bool isMimeTypeFile (XDisplay, AtomType);
};

//==============================================================================
struct GetXProperty
{
    GetXProperty (XDisplay, WindowType, AtomType,
                  long offset, long length, bool shouldDelete,
                  AtomType requestedType);

    ~GetXProperty();

    bool success;
    unsigned char* data = nullptr;
    unsigned long numItems, bytesLeft;
    AtomType actualType;
    int actualFormat;
};

} // namespace juce
