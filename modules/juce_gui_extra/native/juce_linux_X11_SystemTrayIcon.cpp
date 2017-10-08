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

class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (const Image& im, Window windowH)  : image (im)
    {
        ScopedXDisplay xDisplay;
        auto display = xDisplay.display;

        ScopedXLock xlock (display);

        Screen* const screen = XDefaultScreenOfDisplay (display);
        const int screenNumber = XScreenNumberOfScreen (screen);

        String screenAtom ("_NET_SYSTEM_TRAY_S");
        screenAtom << screenNumber;
        Atom selectionAtom = Atoms::getCreating (display, screenAtom.toUTF8());

        XGrabServer (display);
        Window managerWin = XGetSelectionOwner (display, selectionAtom);

        if (managerWin != None)
            XSelectInput (display, managerWin, StructureNotifyMask);

        XUngrabServer (display);
        XFlush (display);

        if (managerWin != None)
        {
            XEvent ev = { 0 };
            ev.xclient.type = ClientMessage;
            ev.xclient.window = managerWin;
            ev.xclient.message_type = Atoms::getCreating (display, "_NET_SYSTEM_TRAY_OPCODE");
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = CurrentTime;
            ev.xclient.data.l[1] = 0 /*SYSTEM_TRAY_REQUEST_DOCK*/;
            ev.xclient.data.l[2] = (long) windowH;
            ev.xclient.data.l[3] = 0;
            ev.xclient.data.l[4] = 0;

            XSendEvent (display, managerWin, False, NoEventMask, &ev);
            XSync (display, False);
        }

        // For older KDE's ...
        long atomData = 1;
        Atom trayAtom = Atoms::getCreating (display, "KWM_DOCKWINDOW");
        XChangeProperty (display, windowH, trayAtom, trayAtom, 32, PropModeReplace, (unsigned char*) &atomData, 1);

        // For more recent KDE's...
        trayAtom = Atoms::getCreating (display, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
        XChangeProperty (display, windowH, trayAtom, XA_WINDOW, 32, PropModeReplace, (unsigned char*) &windowH, 1);

        // A minimum size must be specified for GNOME and Xfce, otherwise the icon is displayed with a width of 1
        XSizeHints* hints = XAllocSizeHints();
        hints->flags = PMinSize;
        hints->min_width = 22;
        hints->min_height = 22;
        XSetWMNormalHints (display, windowH, hints);
        XFree (hints);
    }

    Image image;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& newImage)
{
    pimpl = nullptr;

    if (newImage.isValid())
    {
        if (! isOnDesktop())
            addToDesktop (0);

        pimpl = new Pimpl (newImage, (Window) getWindowHandle());

        setVisible (true);
        toFront (false);
    }

    repaint();
}

void SystemTrayIconComponent::paint (Graphics& g)
{
    if (pimpl != nullptr)
        g.drawImage (pimpl->image, getLocalBounds().toFloat(),
                     RectanglePlacement::xLeft | RectanglePlacement::yTop | RectanglePlacement::onlyReduceInSize);
}

void SystemTrayIconComponent::setIconTooltip (const String& /*tooltip*/)
{
    // xxx Not implemented!
}

void SystemTrayIconComponent::setHighlighted (bool)
{
    // xxx Not implemented!
}

void SystemTrayIconComponent::showInfoBubble (const String& /*title*/, const String& /*content*/)
{
    // xxx Not implemented!
}

void SystemTrayIconComponent::hideInfoBubble()
{
    // xxx Not implemented!
}

void* SystemTrayIconComponent::getNativeHandle() const
{
    return getWindowHandle();
}

} // namespace juce
