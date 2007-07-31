/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../juce_Config.h"
#if JUCE_BUILD_GUI_CLASSES

#include "linuxincludes.h"
#include <stdio.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_core/threads/juce_WaitableEvent.h"
#include "../../../src/juce_core/threads/juce_Process.h"
#include "../../../src/juce_core/threads/juce_ScopedLock.h"

#ifdef JUCE_DEBUG
  #define JUCE_DEBUG_XERRORS 1
#endif

Display* display = 0;     // This is also referenced from WindowDriver.cpp
static Window juce_messageWindowHandle = None;

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

struct MessageThreadFuncCall
{
    MessageCallbackFunction* func;
    void* parameter;
    void* result;
    CriticalSection lock;
    WaitableEvent event;
};

static bool errorCondition = false;

// (defined in another file to avoid problems including certain headers in this one)
extern bool juce_isRunningAsApplication();

// Usually happens when client-server connection is broken
static int ioErrorHandler (Display* display)
{
    DBG (T("ERROR: connection to X server broken.. terminating."));

    errorCondition = true;

    if (! juce_isRunningAsApplication())
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

static bool breakIn = false;

// Breakin from keyboard
static void sig_handler (int sig)
{
    if (sig == SIGINT)
    {
        breakIn = true;
        return;
    }

    static bool reentrant = false;

    if (reentrant == false)
    {
        reentrant = true;

        // Illegal instruction
        fflush (stdout);
        Logger::outputDebugString ("ERROR: Program executed illegal instruction.. terminating");

        errorCondition = true;

        if (juce_isRunningAsApplication())
            Process::terminate();
    }
    else
    {
        if (juce_isRunningAsApplication())
            exit(0);
    }
}


//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    // This is called if the client/server connection is broken
    XSetIOErrorHandler (ioErrorHandler);

    // This is called if a protocol error occurs
    XSetErrorHandler (errorHandler);

    // Install signal handler for break-in
    struct sigaction saction;
    sigset_t maskSet;
    sigemptyset (&maskSet);
    saction.sa_handler = sig_handler;
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

    // Initialise xlib for multiple thread support
    if (! XInitThreads())
    {
        // This is fatal!  Print error and closedown
        Logger::outputDebugString ("Failed to initialise xlib thread support.");

        if (juce_isRunningAsApplication())
            Process::terminate();
    }

    String displayName (getenv ("DISPLAY"));
    if (displayName.isEmpty())
        displayName = T(":0.0");

    display = XOpenDisplay (displayName);

    if (display == 0)
    {
        // This is fatal!  Print error and closedown
        Logger::outputDebugString ("Failed to open the X display.");

        if (juce_isRunningAsApplication())
            Process::terminate();
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
    if (errorCondition == false)
    {
        XDestroyWindow (display, juce_messageWindowHandle);
        XCloseDisplay (display);
    }
}

bool juce_postMessageToSystemQueue (void* message)
{
    if (errorCondition)
        return false;

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
}

void MessageManager::broadcastMessage (const String& value) throw()
{
}

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* func,
                                                   void* parameter)
{
    void* retVal = 0;

    if (! errorCondition)
    {
        if (! isThisTheMessageThread())
        {
            static MessageThreadFuncCall messageFuncCallContext;

            const ScopedLock sl (messageFuncCallContext.lock);

            XClientMessageEvent clientMsg;
            clientMsg.display = display;
            clientMsg.window = juce_messageWindowHandle;
            clientMsg.type = ClientMessage;
            clientMsg.format = 32;
            clientMsg.message_type = specialCallbackId;
#if JUCE_64BIT
            clientMsg.data.l[0] = (long) (0x00000000ffffffff & (((uint64) &messageFuncCallContext) >> 32));
            clientMsg.data.l[1] = (long) (0x00000000ffffffff & (uint64) &messageFuncCallContext);
#else
            clientMsg.data.l[0] = (long) &messageFuncCallContext;
#endif

            messageFuncCallContext.func = func;
            messageFuncCallContext.parameter = parameter;

            if (XSendEvent (display, juce_messageWindowHandle, false, NoEventMask, (XEvent*) &clientMsg) == 0)
                return 0;

            XFlush (display); // This is necessary to ensure the event is delivered

            // Wait for it to complete before continuing
            messageFuncCallContext.event.wait();

            retVal = messageFuncCallContext.result;
        }
        else
        {
            // Just call the function directly
            retVal = func (parameter);
        }
    }

    return retVal;
}

bool juce_dispatchNextMessageOnSystemQueue (bool returnIfNoPendingMessages)
{
    if (errorCondition)
        return false;

    if (breakIn)
    {
        errorCondition = true;

        if (juce_isRunningAsApplication())
            Process::terminate();
    }

    if (returnIfNoPendingMessages && ! XPending (display))
        return false;

    XEvent evt;
    XNextEvent (display, &evt);

    if (evt.type == ClientMessage && evt.xany.window == juce_messageWindowHandle)
    {
        XClientMessageEvent* const clientMsg = (XClientMessageEvent*) &evt;

        if (clientMsg->format != 32)
        {
            jassertfalse
            DBG ("Error: juce_dispatchNextMessageOnSystemQueue received malformed client message.");
        }
        else
        {
            JUCE_TRY
            {
#if JUCE_64BIT
                void* const messagePtr
                    = (void*) ((0xffffffff00000000 & (((uint64) clientMsg->data.l[0]) << 32))
                               | (clientMsg->data.l[1] & 0x00000000ffffffff));
#else
                void* const messagePtr = (void*) (clientMsg->data.l[0]);
#endif

                if (clientMsg->message_type == specialId)
                {
                    MessageManager::getInstance()->deliverMessage (messagePtr);
                }
                else if (clientMsg->message_type == specialCallbackId)
                {
                    MessageThreadFuncCall* const call = (MessageThreadFuncCall*) messagePtr;
                    MessageCallbackFunction* func = call->func;
                    call->result = (*func) (call->parameter);
                    call->event.signal();
                }
                else if (clientMsg->message_type == broadcastId)
                {
#if 0
                    TCHAR buffer[8192];
                    zeromem (buffer, sizeof (buffer));

                    if (GlobalGetAtomName ((ATOM) lParam, buffer, 8192) != 0)
                        mq->deliverBroadcastMessage (String (buffer));
#endif
                }
                else
                {
                    DBG ("Error: juce_dispatchNextMessageOnSystemQueue received unknown client message.");
                }
            }
            JUCE_CATCH_ALL
        }
    }
    else if (evt.xany.window != juce_messageWindowHandle)
    {
        juce_windowMessageReceive (&evt);
    }

    return true;
}

END_JUCE_NAMESPACE

#endif
