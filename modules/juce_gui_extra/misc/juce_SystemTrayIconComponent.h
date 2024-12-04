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

#if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD || JUCE_MAC || DOXYGEN

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
    /** Constructor. */
    SystemTrayIconComponent();

    /** Destructor. */
    ~SystemTrayIconComponent() override;

    //==============================================================================
    /** Changes the image shown in the taskbar.

        On Windows and Linux a full colour Image is used as an icon.
        On macOS a template image is used, where all non-transparent regions will be
        rendered in a monochrome colour selected dynamically by the operating system.

        @param colourImage     An colour image to use as an icon on Windows and Linux
        @param templateImage   A template image to use as an icon on macOS
    */
    void setIconImage (const Image& colourImage, const Image& templateImage);

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

   #if JUCE_LINUX || JUCE_BSD
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
    std::unique_ptr<Pimpl> pimpl;

    [[deprecated ("The new setIconImage function signature requires different images for macOS and the other platforms.")]]
    void setIconImage (const Image& newImage);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SystemTrayIconComponent)
};

#endif

} // namespace juce
