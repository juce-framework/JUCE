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

    @tags{GUI}
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

   #if JUCE_MAC
    /** Shows a menu attached to the OSX menu bar icon. */
    void showDropdownMenu (const PopupMenu& menu);
   #endif

private:
    //==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (class Pimpl)
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemTrayIconComponent)
};


#endif

} // namespace juce
