/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

static Cursor createDraggingHandCursor();
ComponentPeer* getPeerFor (::Window);

//==============================================================================
class X11DragState
{
public:
    X11DragState() = default;

    //==============================================================================
    bool isDragging() const noexcept
    {
        return dragging;
    }

    //==============================================================================
    void handleExternalSelectionClear()
    {
        if (dragging)
            externalResetDragAndDrop();
    }

    void handleExternalSelectionRequest (const XEvent& evt)
    {
        auto targetType = evt.xselectionrequest.target;

        XEvent s;
        s.xselection.type      = SelectionNotify;
        s.xselection.requestor = evt.xselectionrequest.requestor;
        s.xselection.selection = evt.xselectionrequest.selection;
        s.xselection.target    = targetType;
        s.xselection.property  = None;
        s.xselection.time      = evt.xselectionrequest.time;

        auto* display = getDisplay();

        if (allowedTypes.contains (targetType))
        {
            s.xselection.property = evt.xselectionrequest.property;

            X11Symbols::getInstance()->xChangeProperty (display, evt.xselectionrequest.requestor, evt.xselectionrequest.property,
                                                        targetType, 8, PropModeReplace,
                                                        reinterpret_cast<const unsigned char*> (textOrFiles.toRawUTF8()),
                                                        (int) textOrFiles.getNumBytesAsUTF8());
        }

        X11Symbols::getInstance()->xSendEvent (display, evt.xselectionrequest.requestor, True, 0, &s);
    }

    void handleExternalDragAndDropStatus (const XClientMessageEvent& clientMsg)
    {
        if (expectingStatus)
        {
            expectingStatus = false;
            canDrop         = false;
            silentRect      = {};

            const auto& atoms = getAtoms();

            if ((clientMsg.data.l[1] & 1) != 0
                && ((Atom) clientMsg.data.l[4] == atoms.XdndActionCopy
                    || (Atom) clientMsg.data.l[4] == atoms.XdndActionPrivate))
            {
                if ((clientMsg.data.l[1] & 2) == 0) // target requests silent rectangle
                    silentRect.setBounds ((int) clientMsg.data.l[2] >> 16, (int) clientMsg.data.l[2] & 0xffff,
                                          (int) clientMsg.data.l[3] >> 16, (int) clientMsg.data.l[3] & 0xffff);

                canDrop = true;
            }
        }
    }

    void handleExternalDragButtonReleaseEvent()
    {
        if (dragging)
            X11Symbols::getInstance()->xUngrabPointer (getDisplay(), CurrentTime);

        if (canDrop)
        {
            sendExternalDragAndDropDrop();
        }
        else
        {
            sendExternalDragAndDropLeave();
            externalResetDragAndDrop();
        }
    }

    void handleExternalDragMotionNotify()
    {
        auto* display = getDisplay();

        auto newTargetWindow = externalFindDragTargetWindow (X11Symbols::getInstance()
                                                               ->xRootWindow (display,
                                                                              X11Symbols::getInstance()->xDefaultScreen (display)));

        if (targetWindow != newTargetWindow)
        {
            if (targetWindow != None)
                sendExternalDragAndDropLeave();

            canDrop = false;
            silentRect = {};

            if (newTargetWindow == None)
                return;

            xdndVersion = getDnDVersionForWindow (newTargetWindow);

            if (xdndVersion == -1)
                return;

            targetWindow = newTargetWindow;
            sendExternalDragAndDropEnter();
        }

        if (! expectingStatus)
            sendExternalDragAndDropPosition();
    }

    void handleDragAndDropPosition (const XClientMessageEvent& clientMsg, ComponentPeer* peer)
    {
        if (dragAndDropSourceWindow == 0)
            return;

        dragAndDropSourceWindow = (::Window) clientMsg.data.l[0];

        if (windowH == 0)
            windowH = (::Window) peer->getNativeHandle();

        const auto displays = Desktop::getInstance().getDisplays();
        const auto logicalPos = displays.physicalToLogical (Point<int> ((int) clientMsg.data.l[2] >> 16,
                                                                        (int) clientMsg.data.l[2] & 0xffff));
        const auto dropPos = detail::ScalingHelpers::screenPosToLocalPos (peer->getComponent(), logicalPos.toFloat()).roundToInt();

        const auto& atoms = getAtoms();

        auto targetAction = atoms.XdndActionCopy;

        for (int i = numElementsInArray (atoms.allowedActions); --i >= 0;)
        {
            if ((Atom) clientMsg.data.l[4] == atoms.allowedActions[i])
            {
                targetAction = atoms.allowedActions[i];
                break;
            }
        }

        sendDragAndDropStatus (true, targetAction);

        if (dragInfo.position != dropPos)
        {
            dragInfo.position = dropPos;

            if (dragInfo.isEmpty())
                updateDraggedFileList (clientMsg, (::Window) peer->getNativeHandle());

            if (! dragInfo.isEmpty())
                peer->handleDragMove (dragInfo);
        }
    }

    void handleDragAndDropDrop (const XClientMessageEvent& clientMsg, ComponentPeer* peer)
    {
        if (dragInfo.isEmpty())
        {
            // no data, transaction finished in handleDragAndDropSelection()
            finishAfterDropDataReceived = true;
            updateDraggedFileList (clientMsg, (::Window) peer->getNativeHandle());
        }
        else
        {
            handleDragAndDropDataReceived();  // data was already received
        }
    }

    void handleDragAndDropEnter (const XClientMessageEvent& clientMsg, ComponentPeer* peer)
    {
        dragInfo.clear();
        srcMimeTypeAtomList.clear();

        dragAndDropCurrentMimeType = 0;
        auto dndCurrentVersion = (static_cast<unsigned long> (clientMsg.data.l[1]) & 0xff000000) >> 24;

        if (dndCurrentVersion < 3 || dndCurrentVersion > XWindowSystemUtilities::Atoms::DndVersion)
        {
            dragAndDropSourceWindow = 0;
            return;
        }

        const auto& atoms = getAtoms();

        dragAndDropSourceWindow = (::Window) clientMsg.data.l[0];

        if ((clientMsg.data.l[1] & 1) != 0)
        {
            XWindowSystemUtilities::ScopedXLock xLock;

            XWindowSystemUtilities::GetXProperty prop (getDisplay(),
                                                       dragAndDropSourceWindow,
                                                       atoms.XdndTypeList,
                                                       0,
                                                       0x8000000L,
                                                       false,
                                                       XA_ATOM);

            if (prop.success && prop.actualType == XA_ATOM && prop.actualFormat == 32 && prop.numItems != 0)
            {
                auto* types = prop.data;

                for (unsigned long i = 0; i < prop.numItems; ++i)
                {
                    unsigned long type;
                    memcpy (&type, types, sizeof (unsigned long));

                    if (type != None)
                        srcMimeTypeAtomList.add (type);

                    types += sizeof (unsigned long);
                }
            }
        }

        if (srcMimeTypeAtomList.isEmpty())
        {
            for (int i = 2; i < 5; ++i)
                if (clientMsg.data.l[i] != None)
                    srcMimeTypeAtomList.add ((unsigned long) clientMsg.data.l[i]);

            if (srcMimeTypeAtomList.isEmpty())
            {
                dragAndDropSourceWindow = 0;
                return;
            }
        }

        for (int i = 0; i < srcMimeTypeAtomList.size() && dragAndDropCurrentMimeType == 0; ++i)
            for (int j = 0; j < numElementsInArray (atoms.allowedMimeTypes); ++j)
                if (srcMimeTypeAtomList[i] == atoms.allowedMimeTypes[j])
                    dragAndDropCurrentMimeType = atoms.allowedMimeTypes[j];

        handleDragAndDropPosition (clientMsg, peer);
    }

    void handleDragAndDropExit()
    {
        if (auto* peer = getPeerFor (windowH))
            peer->handleDragExit (dragInfo);

        resetDragAndDrop();
    }

    void handleDragAndDropSelection (const XEvent& evt)
    {
        dragInfo.clear();

        if (evt.xselection.property != None)
        {
            StringArray lines;

            {
                MemoryBlock dropData;

                for (;;)
                {
                    XWindowSystemUtilities::GetXProperty prop (getDisplay(),
                                                               evt.xany.window,
                                                               evt.xselection.property,
                                                               (long) (dropData.getSize() / 4),
                                                               65536,
                                                               false,
                                                               AnyPropertyType);

                    if (! prop.success)
                        break;

                    dropData.append (prop.data, (size_t) (prop.actualFormat / 8) * prop.numItems);

                    if (prop.bytesLeft <= 0)
                        break;
                }

                lines.addLines (dropData.toString());
            }

            if (XWindowSystemUtilities::Atoms::isMimeTypeFile (getDisplay(), dragAndDropCurrentMimeType))
            {
                for (const auto& line : lines)
                {
                    const auto escaped = line.replace ("+", "%2B").replace ("file://", String(), true);
                    dragInfo.files.add (URL::removeEscapeChars (escaped));
                }

                dragInfo.files.trim();
                dragInfo.files.removeEmptyStrings();
            }
            else
            {
                dragInfo.text = lines.joinIntoString ("\n");
            }

            if (finishAfterDropDataReceived)
                handleDragAndDropDataReceived();
        }
    }

    void externalResetDragAndDrop()
    {
        if (dragging)
        {
            XWindowSystemUtilities::ScopedXLock xLock;
            X11Symbols::getInstance()->xUngrabPointer (getDisplay(), CurrentTime);
        }

        if (completionCallback != nullptr)
            completionCallback();

        dragging = false;
    }

    bool externalDragInit (::Window window, bool text, const String& str, std::function<void()>&& cb)
    {
        windowH            = window;
        isText             = text;
        textOrFiles        = str;
        targetWindow       = windowH;
        completionCallback = std::move (cb);

        auto* display = getDisplay();

        allowedTypes.add (XWindowSystemUtilities::Atoms::getCreating (display, isText ? "text/plain" : "text/uri-list"));

        auto pointerGrabMask = (unsigned int) (Button1MotionMask | ButtonReleaseMask);

        XWindowSystemUtilities::ScopedXLock xLock;

        if (X11Symbols::getInstance()->xGrabPointer (display, windowH, True, pointerGrabMask,
                                                     GrabModeAsync, GrabModeAsync, None, None, CurrentTime) == GrabSuccess)
        {
            const auto& atoms = getAtoms();

            // No other method of changing the pointer seems to work, this call is needed from this very context
            X11Symbols::getInstance()->xChangeActivePointerGrab (display, pointerGrabMask, (Cursor) createDraggingHandCursor(), CurrentTime);

            X11Symbols::getInstance()->xSetSelectionOwner (display, atoms.XdndSelection, windowH, CurrentTime);

            // save the available types to XdndTypeList
            X11Symbols::getInstance()->xChangeProperty (display, windowH, atoms.XdndTypeList, XA_ATOM, 32, PropModeReplace,
                                                        reinterpret_cast<const unsigned char*> (allowedTypes.getRawDataPointer()), allowedTypes.size());

            dragging = true;
            xdndVersion = getDnDVersionForWindow (targetWindow);

            sendExternalDragAndDropEnter();
            handleExternalDragMotionNotify();

            return true;
        }

        return false;
    }

private:
    //==============================================================================
    const XWindowSystemUtilities::Atoms& getAtoms() const noexcept  { return XWindowSystem::getInstance()->getAtoms(); }
    ::Display* getDisplay() const noexcept                          { return XWindowSystem::getInstance()->getDisplay(); }

    //==============================================================================
    void sendDragAndDropMessage (XClientMessageEvent& msg)
    {
        auto* display = getDisplay();

        msg.type      = ClientMessage;
        msg.display   = display;
        msg.window    = dragAndDropSourceWindow;
        msg.format    = 32;
        msg.data.l[0] = (long) windowH;

        XWindowSystemUtilities::ScopedXLock xLock;
        X11Symbols::getInstance()->xSendEvent (display, dragAndDropSourceWindow, False, 0, (XEvent*) &msg);
    }

    bool sendExternalDragAndDropMessage (XClientMessageEvent& msg)
    {
        auto* display = getDisplay();

        msg.type      = ClientMessage;
        msg.display   = display;
        msg.window    = targetWindow;
        msg.format    = 32;
        msg.data.l[0] = (long) windowH;

        XWindowSystemUtilities::ScopedXLock xLock;
        return X11Symbols::getInstance()->xSendEvent (display, targetWindow, False, 0, (XEvent*) &msg) != 0;
    }

    void sendExternalDragAndDropDrop()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = getAtoms().XdndDrop;
        msg.data.l[2] = CurrentTime;

        sendExternalDragAndDropMessage (msg);
    }

    void sendExternalDragAndDropEnter()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = getAtoms().XdndEnter;
        msg.data.l[1] = (xdndVersion << 24);

        for (int i = 0; i < 3; ++i)
            msg.data.l[i + 2] = (long) allowedTypes[i];

        sendExternalDragAndDropMessage (msg);
    }

    void sendExternalDragAndDropPosition()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        const auto& atoms = getAtoms();

        msg.message_type = atoms.XdndPosition;

        auto mousePos = Desktop::getInstance().getMousePosition();

        if (silentRect.contains (mousePos)) // we've been asked to keep silent
            return;

        mousePos = Desktop::getInstance().getDisplays().logicalToPhysical (mousePos);

        msg.data.l[1] = 0;
        msg.data.l[2] = (mousePos.x << 16) | mousePos.y;
        msg.data.l[3] = CurrentTime;
        msg.data.l[4] = (long) atoms.XdndActionCopy; // this is all JUCE currently supports

        expectingStatus = sendExternalDragAndDropMessage (msg);
    }

    void sendDragAndDropStatus (bool acceptDrop, Atom dropAction)
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = getAtoms().XdndStatus;
        msg.data.l[1]    = (acceptDrop ? 1 : 0) | 2; // 2 indicates that we want to receive position messages
        msg.data.l[4]    = (long) dropAction;

        sendDragAndDropMessage (msg);
    }

    void sendExternalDragAndDropLeave()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = getAtoms().XdndLeave;
        sendExternalDragAndDropMessage (msg);
    }

    void sendDragAndDropFinish()
    {
        XClientMessageEvent msg;
        zerostruct (msg);

        msg.message_type = getAtoms().XdndFinished;
        sendDragAndDropMessage (msg);
    }

    void updateDraggedFileList (const XClientMessageEvent& clientMsg, ::Window requestor)
    {
        jassert (dragInfo.isEmpty());

        if (dragAndDropSourceWindow != None && dragAndDropCurrentMimeType != None)
        {
            auto* display = getDisplay();

            XWindowSystemUtilities::ScopedXLock xLock;
            X11Symbols::getInstance()->xConvertSelection (display, getAtoms().XdndSelection, dragAndDropCurrentMimeType,
                                                          XWindowSystemUtilities::Atoms::getCreating (display, "JXSelectionWindowProperty"),
                                                          requestor, (::Time) clientMsg.data.l[2]);
        }
    }

    bool isWindowDnDAware (::Window w) const
    {
        int numProperties = 0;
        auto* properties = X11Symbols::getInstance()->xListProperties (getDisplay(), w, &numProperties);

        bool dndAwarePropFound = false;

        for (int i = 0; i < numProperties; ++i)
            if (properties[i] == getAtoms().XdndAware)
                dndAwarePropFound = true;

        if (properties != nullptr)
            X11Symbols::getInstance()->xFree (properties);

        return dndAwarePropFound;
    }

    int getDnDVersionForWindow (::Window target)
    {
        XWindowSystemUtilities::GetXProperty prop (getDisplay(),
                                                   target,
                                                   getAtoms().XdndAware,
                                                   0,
                                                   2,
                                                   false,
                                                   AnyPropertyType);

        if (prop.success && prop.data != nullptr && prop.actualFormat == 32 && prop.numItems == 1)
            return jmin ((int) prop.data[0], (int) XWindowSystemUtilities::Atoms::DndVersion);

        return -1;
    }

    ::Window externalFindDragTargetWindow (::Window target)
    {
        if (target == None)
            return None;

        if (isWindowDnDAware (target))
            return target;

        ::Window child, phonyWin;
        int phony;
        unsigned int uphony;

        X11Symbols::getInstance()->xQueryPointer (getDisplay(), target, &phonyWin, &child, &phony, &phony, &phony, &phony, &uphony);

        return externalFindDragTargetWindow (child);
    }

    void handleDragAndDropDataReceived()
    {
        ComponentPeer::DragInfo dragInfoCopy (dragInfo);

        sendDragAndDropFinish();
        resetDragAndDrop();

        if (! dragInfoCopy.isEmpty())
            if (auto* peer = getPeerFor (windowH))
                peer->handleDragDrop (dragInfoCopy);
    }

    void resetDragAndDrop()
    {
        dragInfo.clear();
        dragInfo.position = Point<int> (-1, -1);
        dragAndDropCurrentMimeType = 0;
        dragAndDropSourceWindow = 0;
        srcMimeTypeAtomList.clear();
        finishAfterDropDataReceived = false;
    }

    //==============================================================================
    ::Window windowH = 0, targetWindow = 0, dragAndDropSourceWindow = 0;

    int xdndVersion = -1;
    bool isText = false, dragging = false, expectingStatus = false, canDrop = false, finishAfterDropDataReceived = false;

    Atom dragAndDropCurrentMimeType;
    Array<Atom> allowedTypes, srcMimeTypeAtomList;

    ComponentPeer::DragInfo dragInfo;
    Rectangle<int> silentRect;
    String textOrFiles;

    std::function<void()> completionCallback = nullptr;

    //==============================================================================
    JUCE_LEAK_DETECTOR (X11DragState)
};

} // namespace juce
