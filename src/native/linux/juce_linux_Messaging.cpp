/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

XContext improbableNumber;   // This is referenced from Windowing.cpp

extern void juce_windowMessageReceive (XEvent* event);  // Defined in Windowing.cpp
extern void juce_handleSelectionRequest (XSelectionRequestEvent &evt);  // Defined in Clipboard.cpp

//==============================================================================
ScopedXLock::ScopedXLock()       { XLockDisplay (display); }
ScopedXLock::~ScopedXLock()      { XUnlockDisplay (display); }

//==============================================================================
class InternalMessageQueue
{
public:
    InternalMessageQueue()
        : bytesInSocket (0)
    {
        int ret = ::socketpair (AF_LOCAL, SOCK_STREAM, 0, fd);
        (void) ret; jassert (ret == 0);

        //setNonBlocking (fd[0]);
        //setNonBlocking (fd[1]);
    }

    ~InternalMessageQueue()
    {
        close (fd[0]);
        close (fd[1]);
    }

    void postMessage (Message* msg)
    {
        const int maxBytesInSocketQueue = 128;

        ScopedLock sl (lock);
        queue.add (msg);

        if (bytesInSocket < maxBytesInSocketQueue)
        {
            ++bytesInSocket;

            ScopedUnlock ul (lock);
            const unsigned char x = 0xff;
            size_t bytesWritten = write (fd[0], &x, 1);
            (void) bytesWritten;
        }
    }

    bool isEmpty() const
    {
        ScopedLock sl (lock);
        return queue.size() == 0;
    }

    Message* popNextMessage()
    {
        ScopedLock sl (lock);

        if (bytesInSocket > 0)
        {
            --bytesInSocket;

            ScopedUnlock ul (lock);
            unsigned char x;
            size_t numBytes = read (fd[1], &x, 1);
            (void) numBytes;
        }

        Message* m = queue[0];
        queue.remove (0, false /* deleteObject */);
        return m;
    }

    int getWaitHandle() const     { return fd[1]; }

private:
    CriticalSection lock;
    OwnedArray <Message> queue;
    int fd[2];
    int bytesInSocket;

    static bool setNonBlocking (int handle)
    {
        int socketFlags = fcntl (handle, F_GETFL, 0);

        if (socketFlags == -1)
            return false;

        socketFlags |= O_NONBLOCK;

        return fcntl (handle, F_SETFL, socketFlags) == 0;
    }
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
    DBG ("ERROR: connection to X server broken.. terminating.");

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

    XGetErrorDatabaseText (display, "XRequest", String (event->request_code).toCString(),
                           "Unknown", requestStr, 64);

    DBG ("ERROR: X returned " + String (errorStr) + " for operation " + String (requestStr));
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
        displayName = ":0.0";

    display = XOpenDisplay (displayName.toCString());

    if (display == 0)
    {
        // This is not fatal! we can run headless.
        return;
    }

    // Get defaults for various properties
    int screen = DefaultScreen (display);
    Window root = RootWindow (display, screen);
    Visual* visual = DefaultVisual (display, screen);

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
    if (! juce_internalMessageQueue->isEmpty())
        return true;

    if (display != 0)
    {
        ScopedXLock xlock;
        if (XPending (display))
            return true;
    }

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
        ScopedXLock xlock;
        int fd1 = XConnectionNumber (display);
        FD_SET (fd1, &readset);
        fdmax = jmax (fd0, fd1);
    }

    const int ret = select (fdmax + 1, &readset, 0, 0, &tv);
    return (ret > 0); // ret <= 0 if error or timeout
}

// Handle next XEvent (if any)
static bool juce_dispatchNextXEvent()
{
    if (display == 0)
        return false;

    XEvent evt;

    {
        ScopedXLock xlock;

        if (! XPending (display))
            return false;

        XNextEvent (display, &evt);
    }

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
    ScopedPointer <Message> msg (juce_internalMessageQueue->popNextMessage());

    if (msg == 0)
        return false;

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
