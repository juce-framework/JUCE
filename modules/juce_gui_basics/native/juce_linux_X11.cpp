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

namespace juce
{

typedef void (*WindowMessageReceiveCallback) (XEvent&);
WindowMessageReceiveCallback dispatchWindowMessage = nullptr;

typedef void (*SelectionRequestCallback) (XSelectionRequestEvent&);
SelectionRequestCallback handleSelectionRequest = nullptr;

::Window juce_messageWindowHandle;
XContext windowHandleXContext;

//==============================================================================
namespace X11ErrorHandling
{
    static XErrorHandler   oldErrorHandler = {};
    static XIOErrorHandler oldIOErrorHandler = {};

    //==============================================================================
    // Usually happens when client-server connection is broken
    int ioErrorHandler (::Display*)
    {
        DBG ("ERROR: connection to X server broken.. terminating.");

        if (JUCEApplicationBase::isStandaloneApp())
            MessageManager::getInstance()->stopDispatchLoop();

        return 0;
    }


    int errorHandler (::Display* display, XErrorEvent* event)
    {
        ignoreUnused (display, event);

       #if JUCE_DEBUG_XERRORS
        char errorStr[64] = { 0 };
        char requestStr[64] = { 0 };

        XGetErrorText (display, event->error_code, errorStr, 64);
        XGetErrorDatabaseText (display, "XRequest", String (event->request_code).toUTF8(), "Unknown", requestStr, 64);
        DBG ("ERROR: X returned " << errorStr << " for operation " << requestStr);
       #endif

        return 0;
    }

    void installXErrorHandlers()
    {
        oldIOErrorHandler = XSetIOErrorHandler (ioErrorHandler);
        oldErrorHandler = XSetErrorHandler (errorHandler);
    }

    void removeXErrorHandlers()
    {
        XSetIOErrorHandler (oldIOErrorHandler);
        oldIOErrorHandler = {};

        XSetErrorHandler (oldErrorHandler);
        oldErrorHandler = {};
    }
}

//==============================================================================
XWindowSystem::XWindowSystem() noexcept
{
    if (JUCEApplicationBase::isStandaloneApp())
    {
        // Initialise xlib for multiple thread support
        static bool initThreadCalled = false;

        if (! initThreadCalled)
        {
            if (! XInitThreads())
            {
                // This is fatal!  Print error and closedown
                Logger::outputDebugString ("Failed to initialise xlib thread support.");
                Process::terminate();
                return;
            }

            initThreadCalled = true;
        }

        X11ErrorHandling::installXErrorHandlers();
    }
}

XWindowSystem::~XWindowSystem() noexcept
{
    if (JUCEApplicationBase::isStandaloneApp())
        X11ErrorHandling::removeXErrorHandlers();

    clearSingletonInstance();
}

::Display* XWindowSystem::displayRef() noexcept
{
    if (++displayCount == 1)
    {
        jassert (display == nullptr);

        String displayName (getenv ("DISPLAY"));

        if (displayName.isEmpty())
            displayName = ":0.0";

        // it seems that on some systems XOpenDisplay will occasionally
        // fail the first time, but succeed on a second attempt..
        for (int retries = 2; --retries >= 0;)
        {
            display = XOpenDisplay (displayName.toUTF8());

            if (display != nullptr)
                break;
        }

        initialiseXDisplay();
    }

    return display;
}

::Display* XWindowSystem::displayUnref() noexcept
{
    jassert (display != nullptr);
    jassert (displayCount.get() > 0);

    if (--displayCount == 0)
    {
        destroyXDisplay();
        XCloseDisplay (display);
        display = nullptr;
    }

    return display;
}

void XWindowSystem::initialiseXDisplay() noexcept
{
    // This is fatal!  Print error and closedown
    if (display == nullptr)
    {
        Logger::outputDebugString ("Failed to connect to the X Server.");
        Process::terminate();
    }

    // Create a context to store user data associated with Windows we create
    windowHandleXContext = XUniqueContext();

    // We're only interested in client messages for this window, which are always sent
    XSetWindowAttributes swa;
    swa.event_mask = NoEventMask;

    // Create our message window (this will never be mapped)
    const int screen = DefaultScreen (display);
    juce_messageWindowHandle = XCreateWindow (display, RootWindow (display, screen),
                                              0, 0, 1, 1, 0, 0, InputOnly,
                                              DefaultVisual (display, screen),
                                              CWEventMask, &swa);

    XSync (display, False);

    // Setup input event handler
    int fd = XConnectionNumber (display);

    LinuxEventLoop::setWindowSystemFd (fd,
         [this](int /*fd*/)
         {
            do
            {
                XEvent evt;

                {
                    ScopedXLock xlock (display);

                    if (! XPending (display))
                        return false;

                    XNextEvent (display, &evt);
                }

                if (evt.type == SelectionRequest && evt.xany.window == juce_messageWindowHandle
                     && handleSelectionRequest != nullptr)
                {
                    handleSelectionRequest (evt.xselectionrequest);
                }
                else if (evt.xany.window != juce_messageWindowHandle
                          && dispatchWindowMessage != nullptr)
                {
                    dispatchWindowMessage (evt);
                }

            } while (display != nullptr);

            return false;
        });
}

void XWindowSystem::destroyXDisplay() noexcept
{
    ScopedXLock xlock (display);
    XDestroyWindow (display, juce_messageWindowHandle);
    juce_messageWindowHandle = 0;
    XSync (display, True);
    LinuxEventLoop::removeWindowSystemFd();
}

JUCE_IMPLEMENT_SINGLETON (XWindowSystem)

//==============================================================================
ScopedXDisplay::ScopedXDisplay() : display (XWindowSystem::getInstance()->displayRef())
{
}

ScopedXDisplay::~ScopedXDisplay()
{
    XWindowSystem::getInstance()->displayUnref();
}

//==============================================================================
ScopedXLock::ScopedXLock (::Display* d) : display (d)
{
    if (display != nullptr)
        XLockDisplay (display);
}

ScopedXLock::~ScopedXLock()
{
    if (display != nullptr)
        XUnlockDisplay (display);
}

//==============================================================================
Atoms::Atoms (::Display* display)
{
    protocols                    = getIfExists (display, "WM_PROTOCOLS");
    protocolList [TAKE_FOCUS]    = getIfExists (display, "WM_TAKE_FOCUS");
    protocolList [DELETE_WINDOW] = getIfExists (display, "WM_DELETE_WINDOW");
    protocolList [PING]          = getIfExists (display, "_NET_WM_PING");
    changeState                  = getIfExists (display, "WM_CHANGE_STATE");
    state                        = getIfExists (display, "WM_STATE");
    userTime                     = getCreating (display, "_NET_WM_USER_TIME");
    activeWin                    = getCreating (display, "_NET_ACTIVE_WINDOW");
    pid                          = getCreating (display, "_NET_WM_PID");
    windowType                   = getIfExists (display, "_NET_WM_WINDOW_TYPE");
    windowState                  = getIfExists (display, "_NET_WM_STATE");

    XdndAware                    = getCreating (display, "XdndAware");
    XdndEnter                    = getCreating (display, "XdndEnter");
    XdndLeave                    = getCreating (display, "XdndLeave");
    XdndPosition                 = getCreating (display, "XdndPosition");
    XdndStatus                   = getCreating (display, "XdndStatus");
    XdndDrop                     = getCreating (display, "XdndDrop");
    XdndFinished                 = getCreating (display, "XdndFinished");
    XdndSelection                = getCreating (display, "XdndSelection");

    XdndTypeList                 = getCreating (display, "XdndTypeList");
    XdndActionList               = getCreating (display, "XdndActionList");
    XdndActionCopy               = getCreating (display, "XdndActionCopy");
    XdndActionPrivate            = getCreating (display, "XdndActionPrivate");
    XdndActionDescription        = getCreating (display, "XdndActionDescription");

    XembedMsgType                = getCreating (display, "_XEMBED");
    XembedInfo                   = getCreating (display, "_XEMBED_INFO");

    allowedMimeTypes[0]          = getCreating (display, "UTF8_STRING");
    allowedMimeTypes[1]          = getCreating (display, "text/plain;charset=utf-8");
    allowedMimeTypes[2]          = getCreating (display, "text/plain");
    allowedMimeTypes[3]          = getCreating (display, "text/uri-list");

    allowedActions[0]            = getCreating (display, "XdndActionMove");
    allowedActions[1]            = XdndActionCopy;
    allowedActions[2]            = getCreating (display, "XdndActionLink");
    allowedActions[3]            = getCreating (display, "XdndActionAsk");
    allowedActions[4]            = XdndActionPrivate;
}

Atom Atoms::getIfExists (::Display* display, const char* name)  { return XInternAtom (display, name, True); }
Atom Atoms::getCreating (::Display* display, const char* name)  { return XInternAtom (display, name, False); }

String Atoms::getName (::Display* display, const Atom atom)
{
    if (atom == None)
        return "None";

    return String (XGetAtomName (display, atom));
}

bool Atoms::isMimeTypeFile (::Display* display, const Atom atom)
{
    return getName (display, atom).equalsIgnoreCase ("text/uri-list");
}


const unsigned long Atoms::DndVersion = 3;

//==============================================================================
GetXProperty::GetXProperty (::Display* display, Window window, Atom atom,
              long offset, long length, bool shouldDelete,
              Atom requestedType)
{
    success = (XGetWindowProperty (display, window, atom, offset, length,
                                   (Bool) shouldDelete, requestedType, &actualType,
                                   &actualFormat, &numItems, &bytesLeft, &data) == Success)
                && data != nullptr;
}

GetXProperty::~GetXProperty()
{
    if (data != nullptr)
        XFree (data);
}

} // namespace juce
