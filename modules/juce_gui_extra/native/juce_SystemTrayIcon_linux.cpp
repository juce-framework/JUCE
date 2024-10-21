/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (const Image& im, Window windowH)  : image (im)
    {
        XWindowSystemUtilities::ScopedXLock xLock;

        auto* display = XWindowSystem::getInstance()->getDisplay();

        auto* screen = X11Symbols::getInstance()->xDefaultScreenOfDisplay (display);
        auto screenNumber = X11Symbols::getInstance()->xScreenNumberOfScreen (screen);

        String screenAtom ("_NET_SYSTEM_TRAY_S");
        screenAtom << screenNumber;
        Atom selectionAtom = XWindowSystemUtilities::Atoms::getCreating (display, screenAtom.toUTF8());

        X11Symbols::getInstance()->xGrabServer (display);
        auto managerWin = X11Symbols::getInstance()->xGetSelectionOwner (display, selectionAtom);

        if (managerWin != None)
            X11Symbols::getInstance()->xSelectInput (display, managerWin, StructureNotifyMask);

        X11Symbols::getInstance()->xUngrabServer (display);
        X11Symbols::getInstance()->xFlush (display);

        if (managerWin != None)
        {
            XEvent ev = { 0 };
            ev.xclient.type = ClientMessage;
            ev.xclient.window = managerWin;
            ev.xclient.message_type = XWindowSystemUtilities::Atoms::getCreating (display, "_NET_SYSTEM_TRAY_OPCODE");
            ev.xclient.format = 32;
            ev.xclient.data.l[0] = CurrentTime;
            ev.xclient.data.l[1] = 0 /*SYSTEM_TRAY_REQUEST_DOCK*/;
            ev.xclient.data.l[2] = (long) windowH;
            ev.xclient.data.l[3] = 0;
            ev.xclient.data.l[4] = 0;

            X11Symbols::getInstance()->xSendEvent (display, managerWin, False, NoEventMask, &ev);
            X11Symbols::getInstance()->xSync (display, False);
        }

        // For older KDE's ...
        long atomData = 1;
        Atom trayAtom = XWindowSystemUtilities::Atoms::getCreating (display, "KWM_DOCKWINDOW");
        X11Symbols::getInstance()->xChangeProperty (display, windowH, trayAtom, trayAtom,
                                                    32, PropModeReplace, (unsigned char*) &atomData, 1);

        // For more recent KDE's...
        trayAtom = XWindowSystemUtilities::Atoms::getCreating (display, "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
        X11Symbols::getInstance()->xChangeProperty (display, windowH, trayAtom, XA_WINDOW,
                                                    32, PropModeReplace, (unsigned char*) &windowH, 1);

        // A minimum size must be specified for GNOME and Xfce, otherwise the icon is displayed with a width of 1
        if (auto* hints = X11Symbols::getInstance()->xAllocSizeHints())
        {
            hints->flags = PMinSize;
            hints->min_width = 22;
            hints->min_height = 22;
            X11Symbols::getInstance()->xSetWMNormalHints (display, windowH, hints);
            X11Symbols::getInstance()->xFree (hints);
        }
    }

    Image image;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& colourImage, const Image&)
{
    pimpl.reset();

    if (colourImage.isValid())
    {
        if (! isOnDesktop())
            addToDesktop (0);

        pimpl.reset (new Pimpl (colourImage, (Window) getWindowHandle()));

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
