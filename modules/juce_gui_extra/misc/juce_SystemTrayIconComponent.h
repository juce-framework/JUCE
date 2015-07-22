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

#ifndef JUCE_SYSTEMTRAYICONCOMPONENT_H_INCLUDED
#define JUCE_SYSTEMTRAYICONCOMPONENT_H_INCLUDED

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_MAC || DOXYGEN


//==============================================================================
/**
    This component sits in the taskbar tray as a small icon.

    (NB: The exact behaviour of this class will differ between OSes, and it
    isn't fully implemented for all OSes)

    To use it, just create one of these components, but don't attempt to make it
    visible, add it to a parent, or put it on the desktop.

    You can then call setIconImage() to create an icon for it in the taskbar.

    To change the icon's tooltip, you can use setIconTooltip().

    To respond to mouse-events, you can override the normal mouseDown(),
    mouseUp(), mouseDoubleClick() and mouseMove() methods, and although the x, y
    position will not be valid, you can use this to respond to clicks. Traditionally
    you'd use a left-click to show your application's window, and a right-click
    to show a pop-up menu.
*/
class JUCE_API  SystemTrayIconComponent  : public Component
{
public:
    //==============================================================================
    SystemTrayIconComponent();

    /** Destructor. */
    ~SystemTrayIconComponent();

    //==============================================================================
    /** Changes the image shown in the taskbar. */
    void setIconImage (const Image& newImage);

    /** Changes the icon's tooltip (if the current OS supports this). */
    void setIconTooltip (const String& tooltip);

    /** Highlights the icon (if the current OS supports this). */
    void setHighlighted (bool);

    /** Shows a floating text bubble pointing to the icon (if the current OS supports this). */
    void showInfoBubble (const String& title, const String& content);

    /** Hides the icon's floating text bubble (if the current OS supports this). */
    void hideInfoBubble();

    /** Returns the raw handle to whatever kind of internal OS structure is
        involved in showing this icon.
        @see ComponentPeer::getNativeHandle()
    */
    void* getNativeHandle() const;

   #if JUCE_LINUX
    /** @internal */
    void paint (Graphics&) override;
   #endif

private:
    //==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (class Pimpl)
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemTrayIconComponent)
};


#endif
#endif   // JUCE_SYSTEMTRAYICONCOMPONENT_H_INCLUDED
