/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
using WindowMessageReceiveCallback = void (*) (XEvent&);
WindowMessageReceiveCallback dispatchWindowMessage = nullptr;

using SelectionRequestCallback = void (*) (XSelectionRequestEvent&);
SelectionRequestCallback handleSelectionRequest = nullptr;

::Window juce_messageWindowHandle;
XContext windowHandleXContext;

//==============================================================================
namespace X11ErrorHandling
{
    static XErrorHandler   oldErrorHandler   = {};
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
        char errorStr[64]   = { 0 };
        char requestStr[64] = { 0 };

        X11Symbols::getInstance()->xGetErrorText (display, event->error_code, errorStr, 64);
        X11Symbols::getInstance()->xGetErrorDatabaseText (display, "XRequest", String (event->request_code).toUTF8(), "Unknown", requestStr, 64);

        DBG ("ERROR: X returned " << errorStr << " for operation " << requestStr);
       #endif

        return 0;
    }

    void installXErrorHandlers()
    {
        oldIOErrorHandler = X11Symbols::getInstance()->xSetIOErrorHandler (ioErrorHandler);
        oldErrorHandler   = X11Symbols::getInstance()->xSetErrorHandler   (errorHandler);
    }

    void removeXErrorHandlers()
    {
        X11Symbols::getInstance()->xSetIOErrorHandler (oldIOErrorHandler);
        oldIOErrorHandler = {};

        X11Symbols::getInstance()->xSetErrorHandler (oldErrorHandler);
        oldErrorHandler = {};
    }
}

XWindowSystem::XWindowSystem() noexcept
{
    xIsAvailable = X11Symbols::getInstance()->areXFunctionsAvailable();

    if (JUCEApplicationBase::isStandaloneApp() && xIsAvailable)
    {
        // Initialise xlib for multiple thread support
        static bool initThreadCalled = false;

        if (! initThreadCalled)
        {
            if (! X11Symbols::getInstance()->xInitThreads())
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
    if (JUCEApplicationBase::isStandaloneApp() && xIsAvailable)
    {
        X11ErrorHandling::removeXErrorHandlers();
        X11Symbols::deleteInstance();
    }

    clearSingletonInstance();
}

::Display* XWindowSystem::displayRef() noexcept
{
    if (xIsAvailable && ++displayCount == 1)
    {
        jassert (display == nullptr);

        String displayName (getenv ("DISPLAY"));

        if (displayName.isEmpty())
            displayName = ":0.0";

        // it seems that on some systems XOpenDisplay will occasionally
        // fail the first time, but succeed on a second attempt..
        for (int retries = 2; --retries >= 0;)
        {
            display = X11Symbols::getInstance()->xOpenDisplay (displayName.toUTF8());

            if (display != nullptr)
                break;
        }

        initialiseXDisplay();
    }

    return display;
}

::Display* XWindowSystem::displayUnref() noexcept
{
    if (xIsAvailable)
    {
        jassert (display != nullptr);
        jassert (displayCount.get() > 0);

        if (--displayCount == 0)
        {
            destroyXDisplay();
            X11Symbols::getInstance()->xCloseDisplay (display);
            display = nullptr;
        }
    }

    return display;
}

void XWindowSystem::initialiseXDisplay() noexcept
{
    if (xIsAvailable)
    {
        // This is fatal!  Print error and closedown
        if (display == nullptr)
        {
            Logger::outputDebugString ("Failed to connect to the X Server.");
            Process::terminate();
        }

        // Create a context to store user data associated with Windows we create
        windowHandleXContext = (XContext) X11Symbols::getInstance()->xrmUniqueQuark();

        // We're only interested in client messages for this window, which are always sent
        XSetWindowAttributes swa;
        swa.event_mask = NoEventMask;

        // Create our message window (this will never be mapped)
        auto screen = X11Symbols::getInstance()->xDefaultScreen (display);
        juce_messageWindowHandle = X11Symbols::getInstance()->xCreateWindow (display, X11Symbols::getInstance()->xRootWindow (display, screen),
                                                                             0, 0, 1, 1, 0, 0, InputOnly,
                                                                             X11Symbols::getInstance()->xDefaultVisual (display, screen),
                                                                             CWEventMask, &swa);

        X11Symbols::getInstance()->xSync (display, False);

        // Setup input event handler
        LinuxEventLoop::registerFdCallback (X11Symbols::getInstance()->xConnectionNumber (display),
             [this](int)
             {
                do
                {
                    XEvent evt;

                    {
                        ScopedXLock xlock (display);

                        if (! X11Symbols::getInstance()->xPending (display))
                            return;

                        X11Symbols::getInstance()->xNextEvent (display, &evt);
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
            });
    }
}

void XWindowSystem::destroyXDisplay() noexcept
{
    if (xIsAvailable)
    {
        ScopedXLock xlock (display);

        X11Symbols::getInstance()->xDestroyWindow (display, juce_messageWindowHandle);
        juce_messageWindowHandle = 0;
        X11Symbols::getInstance()->xSync (display, True);

        LinuxEventLoop::unregisterFdCallback (X11Symbols::getInstance()->xConnectionNumber (display));
    }
}

JUCE_IMPLEMENT_SINGLETON (XWindowSystem)

//==============================================================================
ScopedXDisplay::ScopedXDisplay()
    : display (XWindowSystem::getInstance()->displayRef())
{
}

ScopedXDisplay::~ScopedXDisplay()
{
    XWindowSystem::getInstance()->displayUnref();
}

//==============================================================================
ScopedXLock::ScopedXLock (::Display* d)
    : display (d)
{
    if (display != nullptr)
        X11Symbols::getInstance()->xLockDisplay (display);
}

ScopedXLock::~ScopedXLock()
{
    if (display != nullptr)
        X11Symbols::getInstance()->xUnlockDisplay (display);
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

Atom Atoms::getIfExists (::Display* display, const char* name)  { return X11Symbols::getInstance()->xInternAtom (display, name, True); }
Atom Atoms::getCreating (::Display* display, const char* name)  { return X11Symbols::getInstance()->xInternAtom (display, name, False); }

String Atoms::getName (::Display* display, const Atom atom)
{
    if (atom == None)
        return "None";

    return X11Symbols::getInstance()->xGetAtomName (display, atom);
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
    success = (X11Symbols::getInstance()->xGetWindowProperty (display, window, atom, offset, length,
                                                              (Bool) shouldDelete, requestedType, &actualType,
                                                              &actualFormat, &numItems, &bytesLeft, &data) == Success)
                && data != nullptr;
}

GetXProperty::~GetXProperty()
{
    if (data != nullptr)
        X11Symbols::getInstance()->xFree (data);
}

} // namespace juce
