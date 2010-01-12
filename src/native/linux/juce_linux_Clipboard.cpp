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

extern Display* display;
extern Window juce_messageWindowHandle;

static String localClipboardContent;
static Atom   atom_UTF8_STRING;
static Atom   atom_CLIPBOARD;
static Atom   atom_TARGETS;

//==============================================================================
static void initSelectionAtoms()
{
    static bool isInitialised = false;
    if (! isInitialised)
    {
        atom_UTF8_STRING = XInternAtom (display, "UTF8_STRING", False);
        atom_CLIPBOARD   = XInternAtom (display, "CLIPBOARD", False);
        atom_TARGETS     = XInternAtom (display, "TARGETS", False);
    }
}

//==============================================================================
// Read the content of a window property as either a locale-dependent string or an utf8 string
// works only for strings shorter than 1000000 bytes
static String juce_readWindowProperty (Window window, Atom prop,
                                       Atom fmt, // XA_STRING or UTF8_STRING
                                       bool deleteAfterReading)
{
    String returnData;
    uint8 *clipData;
    Atom actualType;
    int  actualFormat;
    unsigned long numItems, bytesLeft;

    if (XGetWindowProperty (display, window, prop,
                            0L /* offset */, 1000000 /* length (max) */, False,
                            AnyPropertyType /* format */,
                            &actualType, &actualFormat, &numItems, &bytesLeft,
                            &clipData) == Success)
    {
        if (actualType == atom_UTF8_STRING && actualFormat == 8)
        {
            returnData = String::fromUTF8 (clipData, numItems);
        }
        else if (actualType == XA_STRING && actualFormat == 8)
        {
            returnData = String ((const char*) clipData, numItems);
        }

        if (clipData != 0)
            XFree (clipData);

        jassert (bytesLeft == 0 || numItems == 1000000);
    }

    if (deleteAfterReading)
        XDeleteProperty (display, window, prop);

    return returnData;
}

//==============================================================================
// Send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout) */
static bool juce_requestSelectionContent (String &selection_content, Atom selection, Atom requested_format)
{
    Atom property_name = XInternAtom (display, "JUCE_SEL", false);

    // The selection owner will be asked to set the JUCE_SEL property on the
    // juce_messageWindowHandle with the selection content
    XConvertSelection (display, selection, requested_format, property_name,
                       juce_messageWindowHandle, CurrentTime);

    int timeoutMs = 200; // will wait at most for 200 ms

    do
    {
        XEvent event;

        if (XCheckTypedWindowEvent (display, juce_messageWindowHandle, SelectionNotify, &event))
        {
            if (event.xselection.property == property_name)
            {
                jassert (event.xselection.requestor == juce_messageWindowHandle);

                selection_content = juce_readWindowProperty (event.xselection.requestor,
                                                             event.xselection.property,
                                                             requested_format, true);
                return true;
            }
            else
            {
                return false; // the format we asked for was denied.. (event.xselection.property == None)
            }
        }

        // not very elegant.. we could do a select() or something like that...
        // however clipboard content requesting is inherently slow on x11, it
        // often takes 50ms or more so...
        Thread::sleep (4);
        timeoutMs -= 4;
    }
    while (timeoutMs > 0);

    DBG("timeout for juce_requestSelectionContent");
    return false;
}

//==============================================================================
// Called from the event loop in juce_linux_Messaging in response to SelectionRequest events
void juce_handleSelectionRequest (XSelectionRequestEvent &evt)
{
    initSelectionAtoms();

    // the selection content is sent to the target window as a window property
    XSelectionEvent reply;
    reply.type = SelectionNotify;
    reply.display = evt.display;
    reply.requestor = evt.requestor;
    reply.selection = evt.selection;
    reply.target = evt.target;
    reply.property = None; // == "fail"
    reply.time = evt.time;

    HeapBlock <char> data;
    int propertyFormat = 0, numDataItems = 0;

    if (evt.selection == XA_PRIMARY || evt.selection == atom_CLIPBOARD)
    {
        if (evt.target == XA_STRING)
        {
            // format data according to system locale
            numDataItems = localClipboardContent.length();
            data.calloc (numDataItems + 2);
            localClipboardContent.copyToBuffer ((char*) data, numDataItems + 1);
            propertyFormat = 8; // bits/item
        }
        else if (evt.target == atom_UTF8_STRING)
        {
            // translate to utf8
            numDataItems = localClipboardContent.copyToUTF8 (0);
            data.calloc (numDataItems + 2);
            localClipboardContent.copyToUTF8 (data, numDataItems + 1);
            propertyFormat = 8; // bits/item
        }
        else if (evt.target == atom_TARGETS)
        {
            // another application wants to know what we are able to send
            numDataItems = 2;
            propertyFormat = 32; // atoms are 32-bit
            data.calloc (numDataItems * 4);
            ((Atom*) data)[0] = atom_UTF8_STRING;
            ((Atom*) data)[1] = XA_STRING;
        }
    }
    else
    {
        DBG ("requested unsupported clipboard");
    }

    if (data != 0)
    {
        const int maxReasonableSelectionSize = 1000000;

        // for very big chunks of data, we should use the "INCR" protocol , which is a pain in the *ss
        if (evt.property != None && numDataItems < maxReasonableSelectionSize)
        {
            XChangeProperty (evt.display, evt.requestor,
                             evt.property, evt.target,
                             propertyFormat /* 8 or 32 */, PropModeReplace,
                             (const unsigned char*) data, numDataItems);
            reply.property = evt.property; // " == success"
        }
    }

    XSendEvent (evt.display, evt.requestor, 0, NoEventMask, (XEvent*) &reply);
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& clipText) throw()
{
    initSelectionAtoms();
    localClipboardContent = clipText;

    XSetSelectionOwner (display, XA_PRIMARY, juce_messageWindowHandle, CurrentTime);
    XSetSelectionOwner (display, atom_CLIPBOARD, juce_messageWindowHandle, CurrentTime);
}

const String SystemClipboard::getTextFromClipboard() throw()
{
    initSelectionAtoms();

    /* 1) try to read from the "CLIPBOARD" selection first (the "high
       level" clipboard that is supposed to be filled by ctrl-C
       etc). When a clipboard manager is running, the content of this
       selection is preserved even when the original selection owner
       exits.

       2) and then try to read from "PRIMARY" selection (the "legacy" selection
       filled by good old x11 apps such as xterm)
    */
    String content;
    Atom selection = XA_PRIMARY;
    Window selectionOwner = None;

    if ((selectionOwner = XGetSelectionOwner (display, selection)) == None)
    {
        selection = atom_CLIPBOARD;
        selectionOwner = XGetSelectionOwner (display, selection);
    }

    if (selectionOwner != None)
    {
        if (selectionOwner == juce_messageWindowHandle)
        {
            content = localClipboardContent;
        }
        else
        {
            // first try: we want an utf8 string
            bool ok = juce_requestSelectionContent (content, selection, atom_UTF8_STRING);

            if (! ok)
            {
                // second chance, ask for a good old locale-dependent string ..
                ok = juce_requestSelectionContent (content, selection, XA_STRING);
            }
        }
    }

    return content;
}

#endif
