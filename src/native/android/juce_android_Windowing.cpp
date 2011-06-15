/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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


//==============================================================================
class AndroidComponentPeer  : public ComponentPeer
{
public:
    //==============================================================================
    AndroidComponentPeer (Component* const component, const int windowStyleFlags)
        : ComponentPeer (component, windowStyleFlags),
          view (android.activity.callObjectMethod (android.createNewView, component->isOpaque())),
          usingAndroidGraphics (false),
          fullScreen (false),
          sizeAllocated (0)
    {
        if (isFocused())
            handleFocusGain();
    }

    ~AndroidComponentPeer()
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            android.activity.callVoidMethod (android.deleteView, view.get());
        }
        else
        {
            class ViewDeleter  : public CallbackMessage
            {
            public:
                ViewDeleter (const GlobalRef& view_)
                    : view (view_)
                {
                    post();
                }

                void messageCallback()
                {
                    android.activity.callVoidMethod (android.deleteView, view.get());
                }

            private:
                GlobalRef view;
            };

            new ViewDeleter (view);
        }

        view.clear();
    }

    void* getNativeHandle() const
    {
        return (void*) view.get();
    }

    void setVisible (bool shouldBeVisible)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (android.setVisible, shouldBeVisible);
        }
        else
        {
            class VisibilityChanger  : public CallbackMessage
            {
            public:
                VisibilityChanger (const GlobalRef& view_, bool shouldBeVisible_)
                    : view (view_), shouldBeVisible (shouldBeVisible_)
                {
                    post();
                }

                void messageCallback()
                {
                    view.callVoidMethod (android.setVisible, shouldBeVisible);
                }

            private:
                GlobalRef view;
                bool shouldBeVisible;
            };

            new VisibilityChanger (view, shouldBeVisible);
        }
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
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            fullScreen = isNowFullScreen;
            w = jmax (0, w);
            h = jmax (0, h);

            view.callVoidMethod (android.layout, x, y, x + w, y + h);
        }
        else
        {
            class ViewMover  : public CallbackMessage
            {
            public:
                ViewMover (const GlobalRef& view_, int x_, int y_, int w_, int h_)
                    : view (view_), x (x_), y (y_), w (w_), h (h_)
                {
                    post();
                }

                void messageCallback()
                {
                    view.callVoidMethod (android.layout, x, y, x + w, y + h);
                }

            private:
                GlobalRef view;
                int x, y, w, h;
            };

            new ViewMover (view, x, y, w, h);
        }
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
        return Point<int> (view.callIntMethod (android.getLeft),
                           view.callIntMethod (android.getTop));
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
        // n/a
    }

    bool isMinimised() const
    {
        return false;
    }

    void setFullScreen (bool shouldBeFullScreen)
    {
        Rectangle<int> r (shouldBeFullScreen ? Desktop::getInstance().getMainMonitorArea()
                                             : lastNonFullscreenBounds);

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (r.getX(), r.getY(), r.getWidth(), r.getHeight(), shouldBeFullScreen);

        component->repaint();
    }

    bool isFullScreen() const
    {
        return fullScreen;
    }

    void setIcon (const Image& newIcon)
    {
        // n/a
    }

    bool contains (const Point<int>& position, bool trueIfInAChildWindow) const
    {
        return isPositiveAndBelow (position.getX(), component->getWidth())
            && isPositiveAndBelow (position.getY(), component->getHeight())
            && ((! trueIfInAChildWindow) || view.callBooleanMethod (android.containsPoint, position.getX(), position.getY()));
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

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer* other)
    {
        // TODO
    }

    //==============================================================================
    void handleMouseDownCallback (float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (0, lastMousePos, currentModifiers, time);
        currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        handleMouseEvent (0, lastMousePos, currentModifiers, time);
    }

    void handleMouseDragCallback (float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        handleMouseEvent (0, lastMousePos, currentModifiers, time);
    }

    void handleMouseUpCallback (float x, float y, int64 time)
    {
        lastMousePos.setXY ((int) x, (int) y);
        currentModifiers = currentModifiers.withoutMouseButtons();
        handleMouseEvent (0, lastMousePos, currentModifiers, time);
    }

    //==============================================================================
    bool isFocused() const
    {
        return view.callBooleanMethod (android.hasFocus);
    }

    void grabFocus()
    {
        view.callBooleanMethod (android.requestFocus);
    }

    void handleFocusChangeCallback (bool hasFocus)
    {
        if (hasFocus)
            handleFocusGain();
        else
            handleFocusLoss();
    }

    void textInputRequired (const Point<int>& position)
    {
        // TODO
    }

    //==============================================================================
    void handlePaintCallback (JNIEnv* env, jobject canvas)
    {
#if USE_ANDROID_CANVAS
        if (usingAndroidGraphics)
        {
            AndroidLowLevelGraphicsContext g (canvas);
            handlePaint (g);
        }
        else
#endif
        {
            jobject rect = env->CallObjectMethod (canvas, android.getClipBounds2);
            const int left = env->GetIntField (rect, android.rectLeft);
            const int top = env->GetIntField (rect, android.rectTop);
            const int right = env->GetIntField (rect, android.rectRight);
            const int bottom = env->GetIntField (rect, android.rectBottom);
            env->DeleteLocalRef (rect);

            const Rectangle<int> clip (left, top, right - left, bottom - top);

            const int sizeNeeded = clip.getWidth() * clip.getHeight();
            if (sizeAllocated < sizeNeeded)
            {
                buffer.clear();
                sizeAllocated = sizeNeeded;
                buffer = GlobalRef (env->NewIntArray (sizeNeeded));
            }

            jint* dest = env->GetIntArrayElements ((jintArray) buffer.get(), 0);

            if (dest != 0)
            {
                {
                    Image temp (new PreallocatedImage (clip.getWidth(), clip.getHeight(),
                                                       dest, ! component->isOpaque()));

                    {
                        LowLevelGraphicsSoftwareRenderer g (temp);
                        g.setOrigin (-clip.getX(), -clip.getY());
                        handlePaint (g);
                    }
                }

                env->ReleaseIntArrayElements ((jintArray) buffer.get(), dest, 0);

                env->CallVoidMethod (canvas, android.drawMemoryBitmap, (jintArray) buffer.get(), 0, clip.getWidth(),
                                     (jfloat) clip.getX(), (jfloat) clip.getY(),
                                     clip.getWidth(), clip.getHeight(), true, (jobject) 0);
            }
        }
    }

    void repaint (const Rectangle<int>& area)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (android.invalidate, area.getX(), area.getY(), area.getRight(), area.getBottom());
        }
        else
        {
            class ViewRepainter  : public CallbackMessage
            {
            public:
                ViewRepainter (const GlobalRef& view_, const Rectangle<int>& area_)
                    : view (view_), area (area_)
                {
                    post();
                }

                void messageCallback()
                {
                    view.callVoidMethod (android.invalidate, area.getX(), area.getY(), area.getRight(), area.getBottom());
                }

            private:
                GlobalRef view;
                const Rectangle<int>& area;
            };

            new ViewRepainter (view, area);
        }
    }

    void performAnyPendingRepaintsNow()
    {
        // TODO
    }

    void setAlpha (float newAlpha)
    {
        // TODO
    }

    StringArray getAvailableRenderingEngines()
    {
        StringArray s (ComponentPeer::getAvailableRenderingEngines());
        s.add ("Android Canvas Renderer");
        return s;
    }

   #if USE_ANDROID_CANVAS
    int getCurrentRenderingEngine() const
    {
        return usingAndroidGraphics ? 1 : 0;
    }

    void setCurrentRenderingEngine (int index)
    {
        if (usingAndroidGraphics != (index > 0))
        {
            usingAndroidGraphics = index > 0;
            component->repaint();
        }
    }
   #endif

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

        return nullptr;
    }

    static ModifierKeys currentModifiers;
    static Point<int> lastMousePos;

private:
    //==============================================================================
    GlobalRef view;
    GlobalRef buffer;
    bool usingAndroidGraphics, fullScreen;
    int sizeAllocated;

    class PreallocatedImage  : public Image::SharedImage
    {
    public:
        //==============================================================================
        PreallocatedImage (const int width_, const int height_, jint* data_, bool hasAlpha_)
            : Image::SharedImage (Image::ARGB, width_, height_), data (data_), hasAlpha (hasAlpha_)
        {
            if (hasAlpha_)
                zeromem (data_, width * height * sizeof (jint));
        }

        ~PreallocatedImage()
        {
            if (hasAlpha)
            {
                PixelARGB* pix = (PixelARGB*) data;

                for (int i = width * height; --i >= 0;)
                {
                    pix->unpremultiply();
                    ++pix;
                }
            }
        }

        Image::ImageType getType() const                    { return Image::SoftwareImage; }
        LowLevelGraphicsContext* createLowLevelContext()    { return new LowLevelGraphicsSoftwareRenderer (Image (this)); }

        void initialiseBitmapData (Image::BitmapData& bm, int x, int y, Image::BitmapData::ReadWriteMode mode)
        {
            bm.lineStride = width * sizeof (jint);
            bm.pixelStride = sizeof (jint);
            bm.pixelFormat = Image::ARGB;
            bm.data = (uint8*) (data + x + y * width);
        }

        SharedImage* clone()
        {
            PreallocatedImage* s = new PreallocatedImage (width, height, 0, hasAlpha);
            s->allocatedData.malloc (sizeof (jint) * width * height);
            s->data = s->allocatedData;
            memcpy (s->data, data, sizeof (jint) * width * height);
            return s;
        }

        //==============================================================================
    private:
        jint* data;
        HeapBlock<jint> allocatedData;
        bool hasAlpha;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreallocatedImage);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidComponentPeer);
};

ModifierKeys AndroidComponentPeer::currentModifiers = 0;
Point<int> AndroidComponentPeer::lastMousePos;

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

JUCE_VIEW_CALLBACK (void, viewSizeChanged, (JNIEnv*, jobject view),
                    handleMovedOrResized())

JUCE_VIEW_CALLBACK (void, focusChanged, (JNIEnv*, jobject view, jboolean hasFocus),
                    handleFocusChangeCallback (hasFocus))

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void*)
{
    return new AndroidComponentPeer (this, styleFlags);
}


//==============================================================================
bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
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
    return AndroidComponentPeer::lastMousePos;
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

void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = AndroidComponentPeer::currentModifiers;
}

const ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    return AndroidComponentPeer::currentModifiers;
}

//==============================================================================
bool Process::isForegroundProcess()
{
    return true;      // TODO
}

//==============================================================================
void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType iconType,
                                                          const String& title, const String& message,
                                                          Component* associatedComponent)
{
    android.activity.callVoidMethod (android.showMessageBox, javaString (title).get(), javaString (message).get(), (jlong) 0);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType iconType,
                                                      const String& title, const String& message,
                                                      Component* associatedComponent,
                                                      ModalComponentManager::Callback* callback)
{
    jassert (callback != 0); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (android.showOkCancelBox, javaString (title).get(), javaString (message).get(),
                                     (jlong) (pointer_sized_int) callback);
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType iconType,
                                                        const String& title, const String& message,
                                                        Component* associatedComponent,
                                                        ModalComponentManager::Callback* callback)
{
    jassert (callback != 0); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (android.showYesNoCancelBox, javaString (title).get(), javaString (message).get(),
                                     (jlong) (pointer_sized_int) callback);
    return 0;
}

JUCE_JNI_CALLBACK (JuceAppActivity, alertDismissed, void, (JNIEnv* env, jobject activity,
                                                           jlong callbackAsLong, jint result))
{
    ModalComponentManager::Callback* callback = (ModalComponentManager::Callback*) callbackAsLong;

    if (callback != 0)
        callback->modalStateFinished (result);
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    // TODO
}

bool Desktop::isScreenSaverEnabled()
{
    return true;
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars)
{
    // TODO
}

//==============================================================================
void Desktop::getCurrentMonitorPositions (Array <Rectangle<int> >& monitorCoords, const bool clipToWorkArea)
{
    monitorCoords.add (Rectangle<int> (0, 0, android.screenWidth, android.screenHeight));
}

JUCE_JNI_CALLBACK (JuceAppActivity, setScreenSize, void, (JNIEnv* env, jobject activity,
                                                          jint screenWidth, jint screenHeight))
{
    const bool isSystemInitialised = android.screenWidth != 0;
    android.screenWidth = screenWidth;
    android.screenHeight = screenHeight;

    if (isSystemInitialised)
        Desktop::getInstance().refreshMonitorSizes();
}

//==============================================================================
Image juce_createIconForFile (const File& file)
{
    return Image::null;
}

//==============================================================================
void* MouseCursor::createMouseCursorFromImage (const Image&, int, int)                          { return nullptr; }
void* MouseCursor::createStandardMouseCursor (const MouseCursor::StandardCursorType)            { return nullptr; }
void MouseCursor::deleteMouseCursor (void* const /*cursorHandle*/, const bool /*isStandard*/)   {}

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
