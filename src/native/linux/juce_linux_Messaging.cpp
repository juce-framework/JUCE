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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
#ifdef JUCE_DEBUG
 #define JUCE_DEBUG_XERRORS 1
#endif

Display* display = 0;     // This is also referenced from WindowDriver.cpp
Window juce_messageWindowHandle = None;

#define SpecialAtom         "JUCESpecialAtom"
#define BroadcastAtom       "JUCEBroadcastAtom"
#define SpecialCallbackAtom "JUCESpecialCallbackAtom"

static Atom specialId;
static Atom broadcastId;
static Atom specialCallbackId;

// This is referenced from WindowDriver.cpp
XContext improbableNumber;

// Defined in WindowDriver.cpp
extern void juce_windowMessageReceive (XEvent* event);

// Defined in ClipboardDriver.cpp
extern void juce_handleSelectionRequest (XSelectionRequestEvent &evt);

//==============================================================================
class InternalMessageQueue
{
public:
    InternalMessageQueue()
    {
        int ret = ::socketpair (AF_LOCAL, SOCK_STREAM, 0, fd);
        (void) ret; jassert (ret == 0);
    }

    ~InternalMessageQueue()
    {
        close (fd[0]);
        close (fd[1]);
    }

    void postMessage (Message* msg)
    {
        ScopedLock sl (lock);
        queue.add (msg);

        const unsigned char x = 0xff;
        write (fd[0], &x, 1);
    }

    bool isEmpty() const
    {
        ScopedLock sl (lock);
        return queue.size() == 0;
    }

    Message* popMessage()
    {
        Message* m = 0;
        ScopedLock sl (lock);

        if (queue.size() != 0)
        {
            unsigned char x;
            read (fd[1], &x, 1);

            m = queue.getUnchecked(0);
            queue.remove (0, false /* deleteObject */);
        }

        return m;
    }

    int getWaitHandle() const     { return fd[1]; }

private:
    CriticalSection lock;
    OwnedArray <Message> queue;
    int fd[2];
};

//==============================================================================
struct MessageThreadFuncCall
{
    enum { uniqueID = 0x73774623 };

    MessageCallbackFunction* func;
    void* parameter;
    void* result;
    CriticalSection lock;
    WaitableEvent event;
};

//==============================================================================
static InternalMessageQueue* juce_internalMessageQueue = 0;

// error handling in X11
static bool errorOccurred = false;
static bool keyboardBreakOccurred = false;
static XErrorHandler oldErrorHandler = (XErrorHandler) 0;
static XIOErrorHandler oldIOErrorHandler = (XIOErrorHandler) 0;

// Usually happens when client-server connection is broken
static int ioErrorHandler (Display* display)
{
    DBG (T("ERROR: connection to X server broken.. terminating."));

    errorOccurred = true;

    if (JUCEApplication::getInstance() != 0)
        Process::terminate();

    return 0;
}

// A protocol error has occurred
static int errorHandler (Display* display, XErrorEvent* event)
{
#ifdef JUCE_DEBUG_XERRORS
    char errorStr[64] = { 0 };
    char requestStr[64] = { 0 };

    XGetErrorText (display, event->error_code, errorStr, 64);

    XGetErrorDatabaseText (display,
                           "XRequest",
                           (const char*) String (event->request_code),
                           "Unknown",
                           requestStr,
                           64);

    DBG (T("ERROR: X returned ") + String (errorStr) + T(" for operation ") + String (requestStr));
#endif

    return 0;
}

// Breakin from keyboard
static void signalHandler (int sig)
{
    if (sig == SIGINT)
    {
        keyboardBreakOccurred = true;
        return;
    }

    static bool reentrant = false;

    if (! reentrant)
    {
        reentrant = true;

        // Illegal instruction
        fflush (stdout);
        Logger::outputDebugString ("ERROR: Program executed illegal instruction.. terminating");

        errorOccurred = true;

        if (JUCEApplication::getInstance() != 0)
            Process::terminate();
    }
    else
    {
        if (JUCEApplication::getInstance() != 0)
            exit(0);
    }
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    // Initialise xlib for multiple thread support
    static bool initThreadCalled = false;

    if (! initThreadCalled)
    {
        if (! XInitThreads())
        {
            // This is fatal!  Print error and closedown
            Logger::outputDebugString ("Failed to initialise xlib thread support.");

            if (JUCEApplication::getInstance() != 0)
                Process::terminate();

            return;
        }

        initThreadCalled = true;
    }

    // This is called if the client/server connection is broken
    oldIOErrorHandler = XSetIOErrorHandler (ioErrorHandler);

    // This is called if a protocol error occurs
    oldErrorHandler = XSetErrorHandler (errorHandler);

    // Install signal handler for break-in
    struct sigaction saction;
    sigset_t maskSet;
    sigemptyset (&maskSet);
    saction.sa_handler = signalHandler;
    saction.sa_mask = maskSet;
    saction.sa_flags = 0;
    sigaction (SIGINT, &saction, NULL);

#ifndef _DEBUG
    // Setup signal handlers for various fatal errors
    sigaction (SIGILL, &saction, NULL);
    sigaction (SIGBUS, &saction, NULL);
    sigaction (SIGFPE, &saction, NULL);
    sigaction (SIGSEGV, &saction, NULL);
    sigaction (SIGSYS, &saction, NULL);
#endif

    // Create the internal message queue
    juce_internalMessageQueue = new InternalMessageQueue();

    // Try to connect to a display
    String displayName (getenv ("DISPLAY"));
    if (displayName.isEmpty())
        displayName = T(":0.0");

    display = XOpenDisplay (displayName);

    if (display == 0)
    {
        // This is not fatal! we can run headless.
        return;
    }

    // Get defaults for various properties
    int screen = DefaultScreen (display);
    Window root = RootWindow (display, screen);
    Visual* visual = DefaultVisual (display, screen);

    // Create atoms for our ClientMessages (these cannot be deleted)
    specialId = XInternAtom (display, SpecialAtom, false);
    broadcastId = XInternAtom (display, BroadcastAtom, false);
    specialCallbackId = XInternAtom (display, SpecialCallbackAtom, false);

    // Create a context to store user data associated with Windows we
    // create in WindowDriver
    improbableNumber = XUniqueContext();

    // We're only interested in client messages for this window
    // which are always sent
    XSetWindowAttributes swa;
    swa.event_mask = NoEventMask;

    // Create our message window (this will never be mapped)
    juce_messageWindowHandle = XCreateWindow (display, root,
                                              0, 0, 1, 1, 0, 0, InputOnly,
                                              visual, CWEventMask, &swa);
}

void MessageManager::doPlatformSpecificShutdown()
{
    deleteAndZero (juce_internalMessageQueue);

    if (display != 0 && ! errorOccurred)
    {
        XDestroyWindow (display, juce_messageWindowHandle);
        XCloseDisplay (display);

        // reset pointers
        juce_messageWindowHandle = 0;
        display = 0;

        // Restore original error handlers
        XSetIOErrorHandler (oldIOErrorHandler);
        oldIOErrorHandler = 0;
        XSetErrorHandler (oldErrorHandler);
        oldErrorHandler = 0;
    }
}

bool juce_postMessageToSystemQueue (void* message)
{
    if (errorOccurred)
        return false;

    juce_internalMessageQueue->postMessage ((Message*) message);
    return true;
}

/*bool juce_postMessageToX11Queue (void *message)
{
    XClientMessageEvent clientMsg;
    clientMsg.display = display;
    clientMsg.window = juce_messageWindowHandle;
    clientMsg.type = ClientMessage;
    clientMsg.format = 32;
    clientMsg.message_type = specialId;
#if JUCE_64BIT
    clientMsg.data.l[0] = (long) (0x00000000ffffffff & (((uint64) message) >> 32));
    clientMsg.data.l[1] = (long) (0x00000000ffffffff & (long) message);
#else
    clientMsg.data.l[0] = (long) message;
#endif

    XSendEvent (display, juce_messageWindowHandle, false,
                NoEventMask, (XEvent*) &clientMsg);

    XFlush (display); // This is necessary to ensure the event is delivered

    return true;
}*/

void MessageManager::broadcastMessage (const String& value) throw()
{
    /* TODO */
}

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* func,
                                                   void* parameter)
{
    if (errorOccurred)
        return 0;

    if (! isThisTheMessageThread())
    {
        MessageThreadFuncCall messageCallContext;
        messageCallContext.func = func;
        messageCallContext.parameter = parameter;

        juce_internalMessageQueue->postMessage (new Message (MessageThreadFuncCall::uniqueID,
                                                             0, 0, &messageCallContext));

        // Wait for it to complete before continuing
        messageCallContext.event.wait();

        return messageCallContext.result;
    }
    else
    {
        // Just call the function directly
        return func (parameter);
    }
}

// Wait for an event (either XEvent, or an internal Message)
static bool juce_sleepUntilEvent (const int timeoutMs)
{
    if ((display != 0 && XPending (display)) || ! juce_internalMessageQueue->isEmpty())
        return true;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeoutMs * 1000;
    int fd0 = juce_internalMessageQueue->getWaitHandle();
    int fdmax = fd0;

    fd_set readset;
    FD_ZERO (&readset);
    FD_SET (fd0, &readset);

    if (display != 0)
    {
        int fd1 = XConnectionNumber (display);
        FD_SET (fd1, &readset);
        fdmax = jmax (fd0, fd1);
    }

    int ret = select (fdmax + 1, &readset, 0, 0, &tv);
    return (ret > 0); // ret <= 0 if error or timeout
}

// Handle next XEvent (if any)
static bool juce_dispatchNextXEvent()
{
    if (display == 0 || ! XPending (display))
        return false;

    XEvent evt;
    XNextEvent (display, &evt);

    if (evt.type == SelectionRequest && evt.xany.window == juce_messageWindowHandle)
    {
        juce_handleSelectionRequest (evt.xselectionrequest);
    }
    else if (evt.xany.window != juce_messageWindowHandle)
    {
        juce_windowMessageReceive (&evt);
    }

    return true;
}

// Handle next internal Message (if any)
static bool juce_dispatchNextInternalMessage()
{
    if (juce_internalMessageQueue->isEmpty())
        return false;

    ScopedPointer <Message> msg (juce_internalMessageQueue->popMessage());

    if (msg->intParameter1 == MessageThreadFuncCall::uniqueID)
    {
        // Handle callback message
        MessageThreadFuncCall* const call = (MessageThreadFuncCall*) msg->pointerParameter;

        call->result = (*(call->func)) (call->parameter);
        call->event.signal();
    }
    else
    {
        // Handle "normal" messages
        MessageManager::getInstance()->deliverMessage (msg.release());
    }

    return true;
}

// this function expects that it will NEVER be called simultaneously for two concurrent threads
bool juce_dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages)
{
    for (;;)
    {
        if (errorOccurred)
            break;

        if (keyboardBreakOccurred)
        {
            errorOccurred = true;

            if (JUCEApplication::getInstance() != 0)
                Process::terminate();

            break;
        }

        static int totalEventCount = 0;
        ++totalEventCount;

        // The purpose here is to give either priority to XEvents or
        // to internal messages This is necessary to keep a "good"
        // behaviour when the cpu is overloaded
        if (totalEventCount & 1)
        {
            if (juce_dispatchNextXEvent() || juce_dispatchNextInternalMessage())
                return true;
        }
        else
        {
            if (juce_dispatchNextInternalMessage() || juce_dispatchNextXEvent())
                return true;
        }

        if (returnIfNoPendingMessages) // early exit
            break;

        // the timeout is to be on the safe side, but it does not seem to be useful
        juce_sleepUntilEvent (2000);
    }

    return false;
}

#endif
