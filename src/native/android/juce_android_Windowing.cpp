/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

static ModifierKeys currentModifiers;

//==============================================================================
class AndroidComponentPeer  : public ComponentPeer
{
public:
    //==============================================================================
    AndroidComponentPeer (Component* const component, const int windowStyleFlags)
        : ComponentPeer (component, windowStyleFlags),
          view (android.activity.callObjectMethod (android.createNewView))
    {
    }

    ~AndroidComponentPeer()
    {
        android.activity.callVoidMethod (android.deleteView, view.get());
        view.clear();
    }

    void* getNativeHandle() const
    {
        return (void*) view.get();
    }

    void setVisible (bool shouldBeVisible)
    {
        view.callVoidMethod (android.setVisible, shouldBeVisible);
    }

    void setTitle (const String& title)
    {
        view.callVoidMethod (android.setViewName, javaString (title).get());
    }

    void setPosition (int x, int y)
    {
        const Rectangle<int> pos (getBounds());
        setBounds (x, y, pos.getWidth(), pos.getHeight(), false);
    }

    void setSize (int w, int h)
    {
        const Rectangle<int> pos (getBounds());
        setBounds (pos.getX(), pos.getY(), w, h, false);
    }

    void setBounds (int x, int y, int w, int h, bool isNowFullScreen)
    {
        view.callVoidMethod (android.layout, x, y, x + w, y + h);
    }

    const Rectangle<int> getBounds() const
    {
        return Rectangle<int> (view.callIntMethod (android.getLeft),
                               view.callIntMethod (android.getTop),
                               view.callIntMethod (android.getWidth),
                               view.callIntMethod (android.getHeight));
    }

    const Point<int> getScreenPosition() const
    {
        JNIEnv* const env = getEnv();

        jintArray pos = env->NewIntArray (2);
        view.callVoidMethod (android.getLocationOnScreen, pos);

        jint coords[2];
        jint i, sum = 0;
        env->GetIntArrayRegion (pos, 0, 2, coords);
        env->DeleteLocalRef (pos);

        return Point<int> (coords[0], coords[1]);
    }

    const Point<int> localToGlobal (const Point<int>& relativePosition)
    {
        return relativePosition + getScreenPosition();
    }

    const Point<int> globalToLocal (const Point<int>& screenPosition)
    {
        return screenPosition - getScreenPosition();
    }

    void setMinimised (bool shouldBeMinimised)
    {
        // TODO
    }

    bool isMinimised() const
    {
        return false;
    }

    void setFullScreen (bool shouldBeFullScreen)
    {
        // TODO
    }

    bool isFullScreen() const
    {
        // TODO
        return false;
    }

    void setIcon (const Image& newIcon)
    {
        // TODO
    }

    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const
    {
        // TODO

        return isPositiveAndBelow (position.getX(), component->getWidth())
            && isPositiveAndBelow (position.getY(), component->getHeight());
    }

    const BorderSize<int> getFrameSize() const
    {
        // TODO
        return BorderSize<int>();
    }

    bool setAlwaysOnTop (bool alwaysOnTop)
    {
        // TODO
        return false;
    }

    void toFront (bool makeActive)
    {
        view.callVoidMethod (android.bringToFront);

        if (makeActive)
            grabFocus();
    }

    void toBehind (ComponentPeer* other)
    {
        // TODO
    }

    //==============================================================================
    void handleMouseDownCallback (float x, float y, int64 time)
    {
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (0, Point<int> ((int) x, (int) y), currentModifiers, time);
        currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        handleMouseEvent (0, Point<int> ((int) x, (int) y), currentModifiers, time);
    }

    void handleMouseDragCallback (float x, float y, int64 time)
    {
        handleMouseEvent (0, Point<int> ((int) x, (int) y), currentModifiers, time);
    }

    void handleMouseUpCallback (float x, float y, int64 time)
    {
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (0, Point<int> ((int) x, (int) y), currentModifiers, time);
    }

    //==============================================================================
    bool isFocused() const
    {
        return view.callBooleanMethod (android.hasFocus);
    }

    void grabFocus()
    {
        (void) view.callBooleanMethod (android.requestFocus);
    }

    void textInputRequired (const Point<int>& position)
    {
        // TODO
    }

    //==============================================================================
    void handlePaintCallback (JNIEnv* env, jobject canvas)
    {
        GlobalRef canvasRef (canvas);
        AndroidLowLevelGraphicsContext g (canvasRef);
        handlePaint (g);
    }

    void repaint (const Rectangle<int>& area)
    {
        view.callVoidMethod (android.invalidate, area.getX(), area.getY(), area.getRight(), area.getBottom());
    }

    void performAnyPendingRepaintsNow()
    {
        // TODO
    }

    void setAlpha (float newAlpha)
    {
        // TODO
    }

    //==============================================================================
    static AndroidComponentPeer* findPeerForJavaView (jobject viewToFind)
    {
        for (int i = getNumPeers(); --i >= 0;)
        {
            AndroidComponentPeer* const ap = static_cast <AndroidComponentPeer*> (getPeer(i));
            jassert (dynamic_cast <AndroidComponentPeer*> (getPeer(i)) != 0);

            if (ap->view == viewToFind)
                return ap;
        }

        return 0;
    }

private:
    //==============================================================================
    GlobalRef view;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidComponentPeer);
};

//==============================================================================
#define JUCE_VIEW_CALLBACK(returnType, javaMethodName, params, juceMethodInvocation) \
  JUCE_JNI_CALLBACK (ComponentPeerView, javaMethodName, returnType, params) \
  { \
      AndroidComponentPeer* const peer = AndroidComponentPeer::findPeerForJavaView (view); \
      if (peer != 0) \
          peer->juceMethodInvocation; \
  }

JUCE_VIEW_CALLBACK (void, handlePaint, (JNIEnv* env, jobject view, jobject canvas),
                    handlePaintCallback (env, canvas))

JUCE_VIEW_CALLBACK (void, handleMouseDown, (JNIEnv*, jobject view, jfloat x, jfloat y, jlong time),
                    handleMouseDownCallback ((float) x, (float) y, (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseDrag, (JNIEnv*, jobject view, jfloat x, jfloat y, jlong time),
                    handleMouseDragCallback ((float) x, (float) y, (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseUp,   (JNIEnv*, jobject view, jfloat x, jfloat y, jlong time),
                    handleMouseUpCallback ((float) x, (float) y, (int64) time))

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void*)
{
    return new AndroidComponentPeer (this, styleFlags);
}


//==============================================================================
bool Desktop::canUseSemiTransparentWindows() throw()
{
    return true;  // TODO
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    // TODO
    return upright;
}

void Desktop::createMouseInputSources()
{
    // This creates a mouse input source for each possible finger

    for (int i = 0; i < 10; ++i)
        mouseSources.add (new MouseInputSource (i, false));
}

const Point<int> MouseInputSource::getCurrentMousePosition()
{
    // TODO
    return Point<int>();
}

void Desktop::setMousePosition (const Point<int>& newPosition)
{
    // not needed
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int keyCode)
{
    // TODO
    return false;
}

void ModifierKeys::updateCurrentModifiers() throw()
{
    // not needed
}

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() throw()
{
    return currentModifiers;
}

//==============================================================================
bool Process::isForegroundProcess()
{
    return true;      // TODO
}

//==============================================================================
bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    // TODO

}

//==============================================================================
Image::SharedImage* Image::SharedImage::createNativeImage (PixelFormat format, int width, int height, bool clearImage)
{
    return createSoftwareImage (format, width, height, clearImage);
}

void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    // TODO
}

bool Desktop::isScreenSaverEnabled()
{
    return true;
}

//==============================================================================
void juce_setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool /*allowMenusAndBars*/)
{
}

//==============================================================================
void juce_updateMultiMonitorInfo (Array <Rectangle<int> >& monitorCoords, const bool clipToWorkArea)
{
    monitorCoords.add (Rectangle<int> (0, 0, android.screenWidth, android.screenHeight));
}

//==============================================================================
const Image juce_createIconForFile (const File& file)
{
    Image image;

    // TODO

    return image;
}

//==============================================================================
void* MouseCursor::createMouseCursorFromImage (const Image& image, int hotspotX, int hotspotY)
{
    return 0;
}

void MouseCursor::deleteMouseCursor (void* const cursorHandle, const bool isStandard)
{
}

void* MouseCursor::createStandardMouseCursor (const MouseCursor::StandardCursorType type)
{
    return 0;
}

//==============================================================================
void MouseCursor::showInWindow (ComponentPeer*) const   {}
void MouseCursor::showInAllWindows() const  {}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove)
{
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    return false;
}

//==============================================================================
const int extendedKeyModifier       = 0x10000;

const int KeyPress::spaceKey        = ' ';
const int KeyPress::returnKey       = 0x0d;
const int KeyPress::escapeKey       = 0x1b;
const int KeyPress::backspaceKey    = 0x7f;
const int KeyPress::leftKey         = extendedKeyModifier + 1;
const int KeyPress::rightKey        = extendedKeyModifier + 2;
const int KeyPress::upKey           = extendedKeyModifier + 3;
const int KeyPress::downKey         = extendedKeyModifier + 4;
const int KeyPress::pageUpKey       = extendedKeyModifier + 5;
const int KeyPress::pageDownKey     = extendedKeyModifier + 6;
const int KeyPress::endKey          = extendedKeyModifier + 7;
const int KeyPress::homeKey         = extendedKeyModifier + 8;
const int KeyPress::deleteKey       = extendedKeyModifier + 9;
const int KeyPress::insertKey       = -1;
const int KeyPress::tabKey          = 9;
const int KeyPress::F1Key           = extendedKeyModifier + 10;
const int KeyPress::F2Key           = extendedKeyModifier + 11;
const int KeyPress::F3Key           = extendedKeyModifier + 12;
const int KeyPress::F4Key           = extendedKeyModifier + 13;
const int KeyPress::F5Key           = extendedKeyModifier + 14;
const int KeyPress::F6Key           = extendedKeyModifier + 16;
const int KeyPress::F7Key           = extendedKeyModifier + 17;
const int KeyPress::F8Key           = extendedKeyModifier + 18;
const int KeyPress::F9Key           = extendedKeyModifier + 19;
const int KeyPress::F10Key          = extendedKeyModifier + 20;
const int KeyPress::F11Key          = extendedKeyModifier + 21;
const int KeyPress::F12Key          = extendedKeyModifier + 22;
const int KeyPress::F13Key          = extendedKeyModifier + 23;
const int KeyPress::F14Key          = extendedKeyModifier + 24;
const int KeyPress::F15Key          = extendedKeyModifier + 25;
const int KeyPress::F16Key          = extendedKeyModifier + 26;
const int KeyPress::numberPad0      = extendedKeyModifier + 27;
const int KeyPress::numberPad1      = extendedKeyModifier + 28;
const int KeyPress::numberPad2      = extendedKeyModifier + 29;
const int KeyPress::numberPad3      = extendedKeyModifier + 30;
const int KeyPress::numberPad4      = extendedKeyModifier + 31;
const int KeyPress::numberPad5      = extendedKeyModifier + 32;
const int KeyPress::numberPad6      = extendedKeyModifier + 33;
const int KeyPress::numberPad7      = extendedKeyModifier + 34;
const int KeyPress::numberPad8      = extendedKeyModifier + 35;
const int KeyPress::numberPad9      = extendedKeyModifier + 36;
const int KeyPress::numberPadAdd            = extendedKeyModifier + 37;
const int KeyPress::numberPadSubtract       = extendedKeyModifier + 38;
const int KeyPress::numberPadMultiply       = extendedKeyModifier + 39;
const int KeyPress::numberPadDivide         = extendedKeyModifier + 40;
const int KeyPress::numberPadSeparator      = extendedKeyModifier + 41;
const int KeyPress::numberPadDecimalPoint   = extendedKeyModifier + 42;
const int KeyPress::numberPadEquals         = extendedKeyModifier + 43;
const int KeyPress::numberPadDelete         = extendedKeyModifier + 44;
const int KeyPress::playKey         = extendedKeyModifier + 45;
const int KeyPress::stopKey         = extendedKeyModifier + 46;
const int KeyPress::fastForwardKey  = extendedKeyModifier + 47;
const int KeyPress::rewindKey       = extendedKeyModifier + 48;

#endif
