/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

extern ::Display* display;

//==============================================================================
class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (const Image& im, Window windowH)  : image (im)
    {
        ScopedXLock xlock;

        Screen* const screen = XDefaultScreenOfDisplay (display);
        const int screenNumber = XScreenNumberOfScreen (screen);

        String screenAtom ("_NET_SYSTEM_TRAY_S");
        screenAtom << screenNumber;
        Atom selectionAtom = XInternAtom (display, screenAtom.toUTF8(), false);

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
            ev.xclient.message_type = XInternAtom (display, "_NET_SYSTEM_TRAY_OPCODE", False);
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
        Atom trayAtom = XInternAtom (display, "KWM_DOCKWINDOW", false);
        XChangeProperty (display, windowH, trayAtom, trayAtom, 32, PropModeReplace, (unsigned char*) &atomData, 1);

        // For more recent KDE's...
        trayAtom = XInternAtom (display, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", false);
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
