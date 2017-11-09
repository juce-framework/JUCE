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

#if JUCE_DEBUG && ! defined (JUCE_DEBUG_XERRORS)
 #define JUCE_DEBUG_XERRORS 1
#endif

//==============================================================================

namespace Keys
{
    enum MouseButtons
    {
        NoButton = 0,
        LeftButton = 1,
        MiddleButton = 2,
        RightButton = 3,
        WheelUp = 4,
        WheelDown = 5
    };

    static int AltMask = 0;
    static int NumLockMask = 0;
    static bool numLock = false;
    static bool capsLock = false;
    static char keyStates [32];
    static const int extendedKeyModifier = 0x10000000;
}

bool KeyPress::isKeyCurrentlyDown (const int /* keyCode */)
{
    return false;
}

const int KeyPress::spaceKey                = XK_space & 0xff;
const int KeyPress::returnKey               = XK_Return & 0xff;
const int KeyPress::escapeKey               = XK_Escape & 0xff;
const int KeyPress::backspaceKey            = XK_BackSpace & 0xff;
const int KeyPress::leftKey                 = (XK_Left & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::rightKey                = (XK_Right & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::upKey                   = (XK_Up & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::downKey                 = (XK_Down & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::pageUpKey               = (XK_Page_Up & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::pageDownKey             = (XK_Page_Down & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::endKey                  = (XK_End & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::homeKey                 = (XK_Home & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::insertKey               = (XK_Insert & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::deleteKey               = (XK_Delete & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::tabKey                  = XK_Tab & 0xff;
const int KeyPress::F1Key                   = (XK_F1 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F2Key                   = (XK_F2 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F3Key                   = (XK_F3 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F4Key                   = (XK_F4 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F5Key                   = (XK_F5 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F6Key                   = (XK_F6 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F7Key                   = (XK_F7 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F8Key                   = (XK_F8 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F9Key                   = (XK_F9 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F10Key                  = (XK_F10 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F11Key                  = (XK_F11 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F12Key                  = (XK_F12 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F13Key                  = (XK_F13 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F14Key                  = (XK_F14 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F15Key                  = (XK_F15 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F16Key                  = (XK_F16 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F17Key                  = (XK_F17 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F18Key                  = (XK_F18 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F19Key                  = (XK_F19 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F20Key                  = (XK_F20 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F21Key                  = (XK_F21 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F22Key                  = (XK_F22 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F23Key                  = (XK_F23 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F24Key                  = (XK_F24 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F25Key                  = (XK_F25 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F26Key                  = (XK_F26 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F27Key                  = (XK_F27 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F28Key                  = (XK_F28 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F29Key                  = (XK_F29 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F30Key                  = (XK_F30 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F31Key                  = (XK_F31 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F32Key                  = (XK_F32 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F33Key                  = (XK_F33 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F34Key                  = (XK_F34 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::F35Key                  = (XK_F35 & 0xff) | Keys::extendedKeyModifier;

const int KeyPress::numberPad0              = (XK_KP_0 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad1              = (XK_KP_1 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad2              = (XK_KP_2 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad3              = (XK_KP_3 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad4              = (XK_KP_4 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad5              = (XK_KP_5 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad6              = (XK_KP_6 & 0xff) | Keys::extendedKeyModifier;
const int KeyPress::numberPad7              = (XK_KP_7 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPad8              = (XK_KP_8 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPad9              = (XK_KP_9 & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadAdd            = (XK_KP_Add & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadSubtract       = (XK_KP_Subtract & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadMultiply       = (XK_KP_Multiply & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDivide         = (XK_KP_Divide & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadSeparator      = (XK_KP_Separator & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDecimalPoint   = (XK_KP_Decimal & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadEquals         = (XK_KP_Equal & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::numberPadDelete         = (XK_KP_Delete & 0xff)| Keys::extendedKeyModifier;
const int KeyPress::playKey                 = ((int) 0xffeeff00) | Keys::extendedKeyModifier;
const int KeyPress::stopKey                 = ((int) 0xffeeff01) | Keys::extendedKeyModifier;
const int KeyPress::fastForwardKey          = ((int) 0xffeeff02) | Keys::extendedKeyModifier;
const int KeyPress::rewindKey               = ((int) 0xffeeff03) | Keys::extendedKeyModifier;

bool juce_areThereAnyAlwaysOnTopWindows()
{
    return false;
}

//==============================================================================

class LinuxComponentPeer  : public ComponentPeer
{
public:
    LinuxComponentPeer (Component& comp, const int windowStyleFlags, void* /* parentToAddTo */)
        : ComponentPeer (comp, windowStyleFlags) {}

    ~LinuxComponentPeer() {}

    //==============================================================================
    void* getNativeHandle() const override
    {
        return nullptr;
    }

    void setVisible (bool /* shouldBeVisible */) override {}

    void setTitle (const String& /* title */) override {}

    void setBounds (const Rectangle<int>& /* newBounds */, bool /* isNowFullScreen */) override {}

    Rectangle<int> getBounds() const override
    {
        Rectangle<int> bounds;
        return bounds;
    }

    Point<float> localToGlobal (Point<float> relativePosition) override
    {
        return relativePosition;
    }

    Point<float> globalToLocal (Point<float> screenPosition) override
    {
        return screenPosition;
    }

    void setAlpha (float /* newAlpha */) override {}

    void setMinimised (bool /* shouldBeMinimised */) override {}

    bool isMinimised() const override
    {
        return false;
    }

    void setFullScreen (const bool /* shouldBeFullScreen */) override {}

    bool isFullScreen() const override
    {
        return false;
    }

    bool contains (Point<int> /* localPos */, bool /* trueIfInAChildWindow */) const override
    {
        return false;
    }

    BorderSize<int> getFrameSize() const override
    {
        return {};
    }

    bool setAlwaysOnTop (bool /* alwaysOnTop */) override
    {
        return false;
    }

    void toFront (bool /* makeActive */) override {}

    void toBehind (ComponentPeer* /* other */) override {}

    bool isFocused() const override
    {
        return false;
    }

    void grabFocus() override {}

    void textInputRequired (Point<int>, TextInputTarget&) override {}

    void repaint (const Rectangle<int>& /* area */) override {}

    void performAnyPendingRepaintsNow() override {}

    void setIcon (const Image& /* newIcon */) override {}

    StringArray getAvailableRenderingEngines() override
    {
        return StringArray ("Null Renderer");
    }

    //==============================================================================
    static ModifierKeys currentModifiers;
    static bool isActiveApplication;

private:
    static Point<int> lastMousePos;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LinuxComponentPeer)
};

ModifierKeys LinuxComponentPeer::currentModifiers;
bool LinuxComponentPeer::isActiveApplication = false;
Point<int> LinuxComponentPeer::lastMousePos;

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    return LinuxComponentPeer::isActiveApplication;
}

// N/A on Linux as far as I know.
JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess() {}
JUCE_API void JUCE_CALLTYPE Process::hide() {}

//==============================================================================
void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = LinuxComponentPeer::currentModifiers;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    return LinuxComponentPeer::currentModifiers;
}


//==============================================================================
void Desktop::setKioskComponent (Component* /* comp */, bool /* enableOrDisable */, bool /* allowMenusAndBars */) {}

void Desktop::allowedOrientationsChanged() {}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    return new LinuxComponentPeer (*this, styleFlags, nativeWindowToAttachTo);
}

//==============================================================================
void Desktop::Displays::findDisplays (float /* masterScale */) {}

bool MouseInputSource::SourceList::canUseTouch()
{
    return false;
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return false;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    return Point<float> (0.0f, 0.0f);
}

void MouseInputSource::setRawMousePosition (Point<float> /* newPosition */) {}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

void Desktop::setScreenSaverEnabled (const bool /* isEnabled */) {}

bool Desktop::isScreenSaverEnabled()
{
    return false;
}

Image juce_createIconForFile (const File& /* file */)
{
    return {};
}

void LookAndFeel::playAlertSound() {}

#if JUCE_MODAL_LOOPS_PERMITTED
void JUCE_CALLTYPE NativeMessageBox::showMessageBox (AlertWindow::AlertIconType /* iconType */,
                                                     const String& /* title */, const String& /* message */,
                                                     Component* /* associatedComponent */) {}
#endif

void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType /* iconType */,
                                                          const String& /* title */, const String& /* message */,
                                                          Component* /* associatedComponent */,
                                                          ModalComponentManager::Callback* /* callback */) {}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType /* iconType */,
                                                      const String& /* title */, const String& /* message */,
                                                      Component* /* associatedComponent */,
                                                      ModalComponentManager::Callback* /* callback */)
{
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType /* iconType */,
                                                        const String& /* title */, const String& /* message */,
                                                        Component* /* associatedComponent */,
                                                        ModalComponentManager::Callback* /* callback */)
{
    return 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType /* iconType */,
                                                  const String& /* title */, const String& /* message */,
                                                  Component* /* associatedComponent */,
                                                  ModalComponentManager::Callback* /* callback */)
{
    return 0;
}

void* CustomMouseCursorInfo::create() const
{
    return nullptr;
}

void MouseCursor::deleteMouseCursor (void* const /* cursorHandle */, const bool) {}

void* MouseCursor::createStandardMouseCursor (MouseCursor::StandardCursorType /* type */)
{
    return nullptr;
}

void MouseCursor::showInWindow (ComponentPeer* /* peer */) const {}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& /* files */, const bool /* canMoveFiles */,
                                                           Component* /* sourceComp */)
{
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& /* text */, Component* /* sourceComp */)
{
    return false;
}

} // namespace juce
