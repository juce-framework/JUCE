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

extern ::Window juce_messageWindowHandle;

namespace ClipboardHelpers
{
    static String localClipboardContent;
    static Atom   atom_UTF8_STRING;
    static Atom   atom_CLIPBOARD;
    static Atom   atom_TARGETS;

    //==============================================================================
    static void initSelectionAtoms (::Display* display)
    {
        static bool isInitialised = false;

        if (! isInitialised)
        {
            isInitialised = true;

            atom_UTF8_STRING = Atoms::getCreating (display, "UTF8_STRING");
            atom_CLIPBOARD   = Atoms::getCreating (display, "CLIPBOARD");
            atom_TARGETS     = Atoms::getCreating (display, "TARGETS");
        }
    }

    //==============================================================================
    // Read the content of a window property as either a locale-dependent string or an utf8 string
    // works only for strings shorter than 1000000 bytes
    static String readWindowProperty (::Display* display, Window window, Atom prop)
    {
        String returnData;

        if (display != nullptr)
        {
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
                    returnData = String::fromUTF8 (clipData, (int) numItems);
                else if (actualType == XA_STRING && actualFormat == 8)
                    returnData = String (clipData, numItems);

                if (clipData != nullptr)
                    XFree (clipData);

                jassert (bytesLeft == 0 || numItems == 1000000);
            }

            XDeleteProperty (display, window, prop);
        }

        return returnData;
    }

    //==============================================================================
    // Send a SelectionRequest to the window owning the selection and waits for its answer (with a timeout) */
    static bool requestSelectionContent (::Display* display, String& selectionContent,
                                         Atom selection, Atom requestedFormat)
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

                    selectionContent = readWindowProperty (display, event.xselection.requestor,
                                                           event.xselection.property);
                    return true;
                }

                return false;  // the format we asked for was denied.. (event.xselection.property == None)
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
        ClipboardHelpers::initSelectionAtoms (evt.display);

        // the selection content is sent to the target window as a window property
        XSelectionEvent reply;
        reply.type = SelectionNotify;
        reply.display = evt.display;
        reply.requestor = evt.requestor;
        reply.selection = evt.selection;
        reply.target = evt.target;
        reply.property = None; // == "fail"
        reply.time = evt.time;

        HeapBlock<char> data;
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
                                 reinterpret_cast<const unsigned char*> (data.getData()), (int) numDataItems);
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
    ScopedXDisplay xDisplay;

    if (auto display = xDisplay.display)
    {
        ClipboardHelpers::initSelectionAtoms (display);
        ClipboardHelpers::localClipboardContent = clipText;

        XSetSelectionOwner (display, XA_PRIMARY, juce_messageWindowHandle, CurrentTime);
        XSetSelectionOwner (display, ClipboardHelpers::atom_CLIPBOARD, juce_messageWindowHandle, CurrentTime);
    }
}

String SystemClipboard::getTextFromClipboard()
{
    String content;
    ScopedXDisplay xDisplay;

    if (auto display = xDisplay.display)
    {
        ClipboardHelpers::initSelectionAtoms (display);

        /* 1) try to read from the "CLIPBOARD" selection first (the "high
           level" clipboard that is supposed to be filled by ctrl-C
           etc). When a clipboard manager is running, the content of this
           selection is preserved even when the original selection owner
           exits.

           2) and then try to read from "PRIMARY" selection (the "legacy" selection
           filled by good old x11 apps such as xterm)
        */
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
                bool ok = ClipboardHelpers::requestSelectionContent (display, content,
                                                                     selection, ClipboardHelpers::atom_UTF8_STRING);

                if (! ok)
                {
                    // second chance, ask for a good old locale-dependent string ..
                    ok = ClipboardHelpers::requestSelectionContent (display, content,
                                                                    selection, XA_STRING);
                }
            }
        }
    }

    return content;
}

} // namespace juce
