/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

extern Display* display;
extern Window juce_messageWindowHandle;

namespace ClipboardHelpers
{
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
    static String readWindowProperty (Window window, Atom prop)
    {
        String returnData;
        char* clipData;
        Atom actualType;
        int actualFormat;
        unsigned long numItems, bytesLeft;

        if (XGetWindowProperty (display, window, prop,
                                0L /* offset */, 1000000 /* length (max) */, False,
                                AnyPropertyType /* format */,
                                &actualType, &actualFormat, &numItems, &bytesLeft,
                                (unsigned char**) &clipData) == Success)
        {
            if (actualType == atom_UTF8_STRING && actualFormat == 8)
                returnData = String::fromUTF8 (clipData, numItems);
            else if (actualType == XA_STRING && actualFormat == 8)
                returnData = String (clipData, numItems);

            if (clipData != nullptr)
                XFree (clipData);

            jassert (bytesLeft == 0 || numItems == 1000000);
        }

        XDeleteProperty (display, window, prop);
        return returnData;
    }

    //==============================================================================
    // Send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout) */
    static bool requestSelectionContent (String& selectionContent, Atom selection, Atom requestedFormat)
    {
        Atom property_name = XInternAtom (display, "JUCE_SEL", false);

        // The selection owner will be asked to set the JUCE_SEL property on the
        // juce_messageWindowHandle with the selection content
        XConvertSelection (display, selection, requestedFormat, property_name,
                           juce_messageWindowHandle, CurrentTime);

        int count = 50; // will wait at most for 200 ms

        while (--count >= 0)
        {
            XEvent event;
            if (XCheckTypedWindowEvent (display, juce_messageWindowHandle, SelectionNotify, &event))
            {
                if (event.xselection.property == property_name)
                {
                    jassert (event.xselection.requestor == juce_messageWindowHandle);

                    selectionContent = readWindowProperty (event.xselection.requestor,
                                                           event.xselection.property);
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
        }

        return false;
    }

    //==============================================================================
    // Called from the event loop in juce_linux_Messaging in response to SelectionRequest events
    static void handleSelection (XSelectionRequestEvent& evt)
    {
        ClipboardHelpers::initSelectionAtoms();

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
        int propertyFormat = 0;
        size_t numDataItems = 0;

        if (evt.selection == XA_PRIMARY || evt.selection == ClipboardHelpers::atom_CLIPBOARD)
        {
            if (evt.target == XA_STRING || evt.target == ClipboardHelpers::atom_UTF8_STRING)
            {
                // translate to utf8
                numDataItems = ClipboardHelpers::localClipboardContent.getNumBytesAsUTF8() + 1;
                data.calloc (numDataItems + 1);
                ClipboardHelpers::localClipboardContent.copyToUTF8 (data, numDataItems);
                propertyFormat = 8; // bits/item
            }
            else if (evt.target == ClipboardHelpers::atom_TARGETS)
            {
                // another application wants to know what we are able to send
                numDataItems = 2;
                propertyFormat = 32; // atoms are 32-bit
                data.calloc (numDataItems * 4);
                Atom* atoms = reinterpret_cast<Atom*> (data.getData());
                atoms[0] = ClipboardHelpers::atom_UTF8_STRING;
                atoms[1] = XA_STRING;

                evt.target = XA_ATOM;
            }
        }
        else
        {
            DBG ("requested unsupported clipboard");
        }

        if (data != nullptr)
        {
            const size_t maxReasonableSelectionSize = 1000000;

            // for very big chunks of data, we should use the "INCR" protocol , which is a pain in the *ss
            if (evt.property != None && numDataItems < maxReasonableSelectionSize)
            {
                XChangeProperty (evt.display, evt.requestor,
                                 evt.property, evt.target,
                                 propertyFormat /* 8 or 32 */, PropModeReplace,
                                 reinterpret_cast<const unsigned char*> (data.getData()), numDataItems);
                reply.property = evt.property; // " == success"
            }
        }

        XSendEvent (evt.display, evt.requestor, 0, NoEventMask, (XEvent*) &reply);
    }
}

//==============================================================================
typedef void (*SelectionRequestCallback) (XSelectionRequestEvent&);
extern SelectionRequestCallback handleSelectionRequest;

struct ClipboardCallbackInitialiser
{
    ClipboardCallbackInitialiser()
    {
        handleSelectionRequest = ClipboardHelpers::handleSelection;
    }
};

static ClipboardCallbackInitialiser clipboardInitialiser;

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& clipText)
{
    ClipboardHelpers::initSelectionAtoms();
    ClipboardHelpers::localClipboardContent = clipText;

    XSetSelectionOwner (display, XA_PRIMARY, juce_messageWindowHandle, CurrentTime);
    XSetSelectionOwner (display, ClipboardHelpers::atom_CLIPBOARD, juce_messageWindowHandle, CurrentTime);
}

String SystemClipboard::getTextFromClipboard()
{
    ClipboardHelpers::initSelectionAtoms();

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
        selection = ClipboardHelpers::atom_CLIPBOARD;
        selectionOwner = XGetSelectionOwner (display, selection);
    }

    if (selectionOwner != None)
    {
        if (selectionOwner == juce_messageWindowHandle)
        {
            content = ClipboardHelpers::localClipboardContent;
        }
        else
        {
            // first try: we want an utf8 string
            bool ok = ClipboardHelpers::requestSelectionContent (content, selection, ClipboardHelpers::atom_UTF8_STRING);

            if (! ok)
            {
                // second chance, ask for a good old locale-dependent string ..
                ok = ClipboardHelpers::requestSelectionContent (content, selection, XA_STRING);
            }
        }
    }

    return content;
}
