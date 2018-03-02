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

extern juce::JUCEApplicationBase* juce_CreateApplication(); // (from START_JUCE_APPLICATION)

namespace juce
{

//==============================================================================
#if JUCE_PUSH_NOTIFICATIONS && JUCE_MODULE_AVAILABLE_juce_gui_extra
 // Returns true if the intent was handled.
 extern bool juce_handleNotificationIntent (void*);
 extern void juce_firebaseDeviceNotificationsTokenRefreshed (void*);
 extern void juce_firebaseRemoteNotificationReceived (void*);
 extern void juce_firebaseRemoteMessagesDeleted();
 extern void juce_firebaseRemoteMessageSent(void*);
 extern void juce_firebaseRemoteMessageSendError (void*, void*);
#endif

#if JUCE_IN_APP_PURCHASES && JUCE_MODULE_AVAILABLE_juce_product_unlocking
 extern void juce_inAppPurchaseCompleted (void*);
#endif

#if ! JUCE_DISABLE_NATIVE_FILECHOOSERS
 extern void juce_fileChooserCompleted (int, void*);
#endif

extern void juce_contentSharingCompleted (int);

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, launchApp, void, (JNIEnv* env, jobject activity,
                                                                      jstring appFile, jstring appDataDir))
{
    setEnv (env);

    android.initialise (env, activity, appFile, appDataDir);

    DBG (SystemStats::getJUCEVersion());

    JUCEApplicationBase::createInstance = &juce_CreateApplication;

    initialiseJuce_GUI();

    if (JUCEApplicationBase* app = JUCEApplicationBase::createInstance())
    {
        if (! app->initialiseApp())
            exit (app->shutdownApp());
    }
    else
    {
        jassertfalse; // you must supply an application object for an android app!
    }

    jassert (MessageManager::getInstance()->isThisTheMessageThread());
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, suspendApp, void, (JNIEnv* env, jobject))
{
    setEnv (env);

    if (auto* app = JUCEApplicationBase::getInstance())
        app->suspended();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, resumeApp, void, (JNIEnv* env, jobject))
{
    setEnv (env);

    if (auto* app = JUCEApplicationBase::getInstance())
        app->resumed();
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, quitApp, void, (JNIEnv* env, jobject))
{
    setEnv (env);

    JUCEApplicationBase::appWillTerminateByForce();

    android.shutdown (env);

    jclass systemClass = (jclass) env->FindClass ("java/lang/System");
    jmethodID exitMethod = env->GetStaticMethodID (systemClass, "exit", "(I)V");
    env->CallStaticVoidMethod (systemClass, exitMethod, 0);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, appActivityResult, void, (JNIEnv* env, jobject, jint requestCode, jint resultCode, jobject intentData))
{
    setEnv (env);

   #if JUCE_IN_APP_PURCHASES && JUCE_MODULE_AVAILABLE_juce_product_unlocking
    if (requestCode == 1001)
        juce_inAppPurchaseCompleted (intentData);
   #endif

   #if ! JUCE_DISABLE_NATIVE_FILECHOOSERS
    if (requestCode == /*READ_REQUEST_CODE*/42)
        juce_fileChooserCompleted (resultCode, intentData);
   #endif

    if (requestCode == 1003)
        juce_contentSharingCompleted (resultCode);

    ignoreUnused (intentData, requestCode);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, appNewIntent, void, (JNIEnv* env, jobject, jobject intentData))
{
    setEnv (env);

  #if JUCE_PUSH_NOTIFICATIONS && JUCE_MODULE_AVAILABLE_juce_gui_extra
    if (juce_handleNotificationIntent ((void *)intentData))
        return;

    // Add other functions processing intents here as needed.
  #else
    ignoreUnused (intentData);
  #endif
}

#if defined(JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME)
JUCE_JNI_CALLBACK (JUCE_FIREBASE_INSTANCE_ID_SERVICE_CLASSNAME, firebaseInstanceIdTokenRefreshed, void, (JNIEnv* env, jobject /*activity*/, jstring token))
{
    setEnv (env);

  #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    juce_firebaseDeviceNotificationsTokenRefreshed (token);
  #else
    ignoreUnused (token);
  #endif
}

JUCE_JNI_CALLBACK (JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME, firebaseRemoteMessageReceived, void, (JNIEnv* env, jobject /*activity*/, jobject remoteMessage))
{
    setEnv (env);

  #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    juce_firebaseRemoteNotificationReceived (remoteMessage);
  #else
    ignoreUnused (remoteMessage);
  #endif
}

JUCE_JNI_CALLBACK (JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME, firebaseRemoteMessagesDeleted, void, (JNIEnv* env, jobject /*activity*/))
{
    setEnv (env);

  #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    juce_firebaseRemoteMessagesDeleted();
  #endif
}

JUCE_JNI_CALLBACK (JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME, firebaseRemoteMessageSent, void, (JNIEnv* env, jobject /*activity*/, jstring messageId))
{
    setEnv (env);

  #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    juce_firebaseRemoteMessageSent (messageId);
  #else
    ignoreUnused (messageId);
  #endif
}

JUCE_JNI_CALLBACK (JUCE_FIREBASE_MESSAGING_SERVICE_CLASSNAME, firebaseRemoteMessageSendError, void, (JNIEnv* env, jobject /*activity*/, jstring messageId, jstring error))
{
    setEnv (env);

  #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    juce_firebaseRemoteMessageSendError (messageId, error);
  #else
    ignoreUnused (messageId, error);
  #endif
}
#endif

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (drawBitmap,       "drawBitmap",    "([IIIFFIIZLandroid/graphics/Paint;)V") \
 METHOD (getClipBounds,    "getClipBounds", "()Landroid/graphics/Rect;")

DECLARE_JNI_CLASS (CanvasMinimal, "android/graphics/Canvas");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (setViewName,                 "setViewName",                 "(Ljava/lang/String;)V") \
 METHOD (setVisible,                  "setVisible",                  "(Z)V") \
 METHOD (isVisible,                   "isVisible",                   "()Z") \
 METHOD (containsPoint,               "containsPoint",               "(II)Z") \
 METHOD (showKeyboard,                "showKeyboard",                "(Ljava/lang/String;)V") \
 METHOD (setSystemUiVisibilityCompat, "setSystemUiVisibilityCompat", "(I)V") \

DECLARE_JNI_CLASS (ComponentPeerView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView");
#undef JNI_CLASS_MEMBERS


//==============================================================================
class AndroidComponentPeer  : public ComponentPeer,
                              private Timer
{
public:
    AndroidComponentPeer (Component& comp, const int windowStyleFlags)
        : ComponentPeer (comp, windowStyleFlags),
          fullScreen (false),
          navBarsHidden (false),
          sizeAllocated (0),
          scale ((float) Desktop::getInstance().getDisplays().getMainDisplay().scale)
    {
        // NB: must not put this in the initialiser list, as it invokes a callback,
        // which will fail if the peer is only half-constructed.
        view = GlobalRef (android.activity.callObjectMethod (JuceAppActivity.createNewView,
                                                             (jboolean) component.isOpaque(),
                                                             (jlong) this));

        if (isFocused())
            handleFocusGain();
    }

    ~AndroidComponentPeer()
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            frontWindow = nullptr;
            android.activity.callVoidMethod (JuceAppActivity.deleteView, view.get());
        }
        else
        {
            struct ViewDeleter  : public CallbackMessage
            {
                ViewDeleter (const GlobalRef& view_) : view (view_) {}

                void messageCallback() override
                {
                    android.activity.callVoidMethod (JuceAppActivity.deleteView, view.get());
                }

            private:
                GlobalRef view;
            };

            (new ViewDeleter (view))->post();
        }

        view.clear();
    }

    void* getNativeHandle() const override
    {
        return (void*) view.get();
    }

    void setVisible (bool shouldBeVisible) override
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (ComponentPeerView.setVisible, shouldBeVisible);
        }
        else
        {
            struct VisibilityChanger  : public CallbackMessage
            {
                VisibilityChanger (const GlobalRef& view_, bool shouldBeVisible_)
                    : view (view_), shouldBeVisible (shouldBeVisible_)
                {}

                void messageCallback() override
                {
                    view.callVoidMethod (ComponentPeerView.setVisible, shouldBeVisible);
                }

                GlobalRef view;
                bool shouldBeVisible;
            };

            (new VisibilityChanger (view, shouldBeVisible))->post();
        }
    }

    void setTitle (const String& title) override
    {
        view.callVoidMethod (ComponentPeerView.setViewName, javaString (title).get());
    }

    void setBounds (const Rectangle<int>& userRect, bool isNowFullScreen) override
    {
        Rectangle<int> r = (userRect.toFloat() * scale).toNearestInt();

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            fullScreen = isNowFullScreen;
            view.callVoidMethod (AndroidView.layout,
                                 r.getX(), r.getY(), r.getRight(), r.getBottom());
        }
        else
        {
            class ViewMover  : public CallbackMessage
            {
            public:
                ViewMover (const GlobalRef& v, const Rectangle<int>& boundsToUse)  : view (v), bounds (boundsToUse) {}

                void messageCallback() override
                {
                    view.callVoidMethod (AndroidView.layout,
                                         bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getBottom());
                }

            private:
                GlobalRef view;
                Rectangle<int> bounds;
            };

            (new ViewMover (view, r))->post();
        }
    }

    Rectangle<int> getBounds() const override
    {
        return (Rectangle<float> (view.callIntMethod (AndroidView.getLeft),
                                  view.callIntMethod (AndroidView.getTop),
                                  view.callIntMethod (AndroidView.getWidth),
                                  view.callIntMethod (AndroidView.getHeight)) / scale).toNearestInt();
    }

    void handleScreenSizeChange() override
    {
        ComponentPeer::handleScreenSizeChange();

        if (isFullScreen())
            setFullScreen (true);
    }

    Point<float> getScreenPosition() const
    {
        return Point<float> (view.callIntMethod (AndroidView.getLeft),
                             view.callIntMethod (AndroidView.getTop)) / scale;
    }

    Point<float> localToGlobal (Point<float> relativePosition) override
    {
        return relativePosition + getScreenPosition();
    }

    Point<float> globalToLocal (Point<float> screenPosition) override
    {
        return screenPosition - getScreenPosition();
    }

    void setMinimised (bool /*shouldBeMinimised*/) override
    {
        // n/a
    }

    bool isMinimised() const override
    {
        return false;
    }

    bool shouldNavBarsBeHidden (bool shouldBeFullScreen) const
    {
        if (shouldBeFullScreen)
            if (Component* kiosk = Desktop::getInstance().getKioskModeComponent())
                if (kiosk->getPeer() == this)
                    return true;

        return false;
    }

    void setNavBarsHidden (bool hidden)
    {
        enum
        {
            SYSTEM_UI_FLAG_VISIBLE                  = 0,
            SYSTEM_UI_FLAG_LOW_PROFILE              = 1,
            SYSTEM_UI_FLAG_HIDE_NAVIGATION          = 2,
            SYSTEM_UI_FLAG_FULLSCREEN               = 4,
            SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION   = 512,
            SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN        = 1024,
            SYSTEM_UI_FLAG_IMMERSIVE                = 2048,
            SYSTEM_UI_FLAG_IMMERSIVE_STICKY         = 4096
        };

        view.callVoidMethod (ComponentPeerView.setSystemUiVisibilityCompat,
                             hidden ? (jint) (SYSTEM_UI_FLAG_HIDE_NAVIGATION | SYSTEM_UI_FLAG_FULLSCREEN | SYSTEM_UI_FLAG_IMMERSIVE_STICKY)
                                    : (jint) (SYSTEM_UI_FLAG_VISIBLE));

        navBarsHidden = hidden;
    }

    void setFullScreen (bool shouldBeFullScreen) override
    {
        // updating the nav bar visibility is a bit odd on Android - need to wait for
        if (shouldNavBarsBeHidden (shouldBeFullScreen))
        {
            if (! navBarsHidden && ! isTimerRunning())
            {
                startTimer (500);
            }
        }
        else
        {
            setNavBarsHidden (false);
        }

        Rectangle<int> r (shouldBeFullScreen ? Desktop::getInstance().getDisplays().getMainDisplay().userArea
                                             : lastNonFullscreenBounds);

        if ((! shouldBeFullScreen) && r.isEmpty())
            r = getBounds();

        // (can't call the component's setBounds method because that'll reset our fullscreen flag)
        if (! r.isEmpty())
            setBounds (r, shouldBeFullScreen);

        component.repaint();
    }

    bool isFullScreen() const override
    {
        return fullScreen;
    }

    void timerCallback() override
    {
        setNavBarsHidden (shouldNavBarsBeHidden (fullScreen));
        setFullScreen (fullScreen);
        stopTimer();
    }

    void setIcon (const Image& /*newIcon*/) override
    {
        // n/a
    }

    bool contains (Point<int> localPos, bool trueIfInAChildWindow) const override
    {
        return isPositiveAndBelow (localPos.x, component.getWidth())
            && isPositiveAndBelow (localPos.y, component.getHeight())
            && ((! trueIfInAChildWindow) || view.callBooleanMethod (ComponentPeerView.containsPoint,
                                                                    localPos.x * scale,
                                                                    localPos.y * scale));
    }

    BorderSize<int> getFrameSize() const override
    {
        // TODO
        return BorderSize<int>();
    }

    bool setAlwaysOnTop (bool /*alwaysOnTop*/) override
    {
        // TODO
        return false;
    }

    void toFront (bool makeActive) override
    {
        // Avoid calling bringToFront excessively: it's very slow
        if (frontWindow != this)
        {
            view.callVoidMethod (AndroidView.bringToFront);

            frontWindow = this;
        }

        if (makeActive)
            grabFocus();

        handleBroughtToFront();
    }

    void toBehind (ComponentPeer*) override
    {
        // TODO
    }

    //==============================================================================
    void handleMouseDownCallback (int index, Point<float> sysPos, int64 time)
    {
        Point<float> pos = sysPos / scale;
        lastMousePos = localToGlobal (pos);

        // this forces a mouse-enter/up event, in case for some reason we didn't get a mouse-up before.
        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, currentModifiers.withoutMouseButtons(),
                          MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, time, {}, index);

        if (isValidPeer (this))
            handleMouseDragCallback (index, sysPos, time);
    }

    void handleMouseDragCallback (int index, Point<float> pos, int64 time)
    {
        pos /= scale;
        lastMousePos = localToGlobal (pos);

        jassert (index < 64);
        touchesDown = (touchesDown | (1 << (index & 63)));
        currentModifiers = currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier);
        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, currentModifiers.withoutMouseButtons().withFlags (ModifierKeys::leftButtonModifier),
                          MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation, time, {}, index);
    }

    void handleMouseUpCallback (int index, Point<float> pos, int64 time)
    {
        pos /= scale;
        lastMousePos = localToGlobal (pos);

        jassert (index < 64);
        touchesDown = (touchesDown & ~(1 << (index & 63)));

        if (touchesDown == 0)
            currentModifiers = currentModifiers.withoutMouseButtons();

        handleMouseEvent (MouseInputSource::InputSourceType::touch, pos, currentModifiers.withoutMouseButtons(), MouseInputSource::invalidPressure,
                          MouseInputSource::invalidOrientation, time, {}, index);
    }

    void handleKeyDownCallback (int k, int kc)
    {
        handleKeyPress (k, static_cast<juce_wchar> (kc));
    }

    void handleKeyUpCallback (int /*k*/, int /*kc*/)
    {
    }

    void handleBackButtonCallback()
    {
        if (auto* app = JUCEApplicationBase::getInstance())
            app->backButtonPressed();

        if (Component* kiosk = Desktop::getInstance().getKioskModeComponent())
            if (kiosk->getPeer() == this)
                setNavBarsHidden (navBarsHidden);
    }

    void handleKeyboardHiddenCallback()
    {
        Component::unfocusAllComponents();
    }

    void handleAppResumedCallback()
    {
        if (Component* kiosk = Desktop::getInstance().getKioskModeComponent())
            if (kiosk->getPeer() == this)
                setNavBarsHidden (navBarsHidden);
    }

    //==============================================================================
    bool isFocused() const override
    {
        if (view != nullptr)
            return view.callBooleanMethod (AndroidView.hasFocus);

        return false;
    }

    void grabFocus() override
    {
        if (view != nullptr)
            view.callBooleanMethod (AndroidView.requestFocus);
    }

    void handleFocusChangeCallback (bool hasFocus)
    {
        if (hasFocus)
            handleFocusGain();
        else
            handleFocusLoss();
    }

    static const char* getVirtualKeyboardType (TextInputTarget::VirtualKeyboardType type) noexcept
    {
        switch (type)
        {
            case TextInputTarget::textKeyboard:          return "text";
            case TextInputTarget::numericKeyboard:       return "number";
            case TextInputTarget::decimalKeyboard:       return "numberDecimal";
            case TextInputTarget::urlKeyboard:           return "textUri";
            case TextInputTarget::emailAddressKeyboard:  return "textEmailAddress";
            case TextInputTarget::phoneNumberKeyboard:   return "phone";
            default:                                     jassertfalse; break;
        }

        return "text";
    }

    void textInputRequired (Point<int>, TextInputTarget& target) override
    {
        view.callVoidMethod (ComponentPeerView.showKeyboard,
                             javaString (getVirtualKeyboardType (target.getKeyboardType())).get());
    }

    void dismissPendingTextInput() override
    {
        view.callVoidMethod (ComponentPeerView.showKeyboard, javaString ("").get());

        // updating the nav bar visibility is a bit odd on Android - need to wait for
        if (! isTimerRunning())
            hideNavBarDelayed();
     }

    void hideNavBarDelayed()
    {
        startTimer (500);
    }

    //==============================================================================
    void handlePaintCallback (JNIEnv* env, jobject canvas, jobject paint)
    {
        jobject rect = env->CallObjectMethod (canvas, CanvasMinimal.getClipBounds);
        const int left   = env->GetIntField (rect, AndroidRectClass.left);
        const int top    = env->GetIntField (rect, AndroidRectClass.top);
        const int right  = env->GetIntField (rect, AndroidRectClass.right);
        const int bottom = env->GetIntField (rect, AndroidRectClass.bottom);
        env->DeleteLocalRef (rect);

        const Rectangle<int> clip (left, top, right - left, bottom - top);

        const int sizeNeeded = clip.getWidth() * clip.getHeight();
        if (sizeAllocated < sizeNeeded)
        {
            buffer.clear();
            sizeAllocated = sizeNeeded;
            buffer = GlobalRef (env->NewIntArray (sizeNeeded));
        }
        else if (sizeNeeded == 0)
        {
            return;
        }

        if (jint* dest = env->GetIntArrayElements ((jintArray) buffer.get(), 0))
        {
            {
                Image temp (new PreallocatedImage (clip.getWidth(), clip.getHeight(),
                                                   dest, ! component.isOpaque()));

                {
                    LowLevelGraphicsSoftwareRenderer g (temp);
                    g.setOrigin (-clip.getPosition());
                    g.addTransform (AffineTransform::scale (scale));
                    handlePaint (g);
                }
            }

            env->ReleaseIntArrayElements ((jintArray) buffer.get(), dest, 0);

            env->CallVoidMethod (canvas, CanvasMinimal.drawBitmap, (jintArray) buffer.get(), 0, clip.getWidth(),
                                 (jfloat) clip.getX(), (jfloat) clip.getY(),
                                 clip.getWidth(), clip.getHeight(), true, paint);
        }
    }

    void repaint (const Rectangle<int>& userArea) override
    {
        Rectangle<int> area = userArea * scale;

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            view.callVoidMethod (AndroidView.invalidate, area.getX(), area.getY(), area.getRight(), area.getBottom());
        }
        else
        {
            struct ViewRepainter  : public CallbackMessage
            {
                ViewRepainter (const GlobalRef& view_, const Rectangle<int>& area_)
                    : view (view_), area (area_) {}

                void messageCallback() override
                {
                    view.callVoidMethod (AndroidView.invalidate, area.getX(), area.getY(),
                                         area.getRight(), area.getBottom());
                }

            private:
                GlobalRef view;
                const Rectangle<int> area;
            };

            (new ViewRepainter (view, area))->post();
        }
    }

    void performAnyPendingRepaintsNow() override
    {
        // TODO
    }

    void setAlpha (float /*newAlpha*/) override
    {
        // TODO
    }

    StringArray getAvailableRenderingEngines() override
    {
        return StringArray ("Software Renderer");
    }

    //==============================================================================
    static ModifierKeys currentModifiers;
    static Point<float> lastMousePos;
    static int64 touchesDown;

private:
    //==============================================================================
    GlobalRef view;
    GlobalRef buffer;
    bool fullScreen;
    bool navBarsHidden;
    int sizeAllocated;
    float scale;
    static AndroidComponentPeer* frontWindow;

    struct PreallocatedImage  : public ImagePixelData
    {
        PreallocatedImage (const int width_, const int height_, jint* data_, bool hasAlpha_)
            : ImagePixelData (Image::ARGB, width_, height_), data (data_), hasAlpha (hasAlpha_)
        {
            if (hasAlpha_)
                zeromem (data_, static_cast<size_t> (width * height) * sizeof (jint));
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

        ImageType* createType() const override                      { return new SoftwareImageType(); }
        LowLevelGraphicsContext* createLowLevelContext() override   { return new LowLevelGraphicsSoftwareRenderer (Image (this)); }

        void initialiseBitmapData (Image::BitmapData& bm, int x, int y, Image::BitmapData::ReadWriteMode /*mode*/) override
        {
            bm.lineStride = width * static_cast<int> (sizeof (jint));
            bm.pixelStride = static_cast<int> (sizeof (jint));
            bm.pixelFormat = Image::ARGB;
            bm.data = (uint8*) (data + x + y * width);
        }

        ImagePixelData::Ptr clone() override
        {
            PreallocatedImage* s = new PreallocatedImage (width, height, 0, hasAlpha);
            s->allocatedData.malloc (sizeof (jint) * static_cast<size_t> (width * height));
            s->data = s->allocatedData;
            memcpy (s->data, data, sizeof (jint) * static_cast<size_t> (width * height));
            return s;
        }

    private:
        jint* data;
        HeapBlock<jint> allocatedData;
        bool hasAlpha;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreallocatedImage)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidComponentPeer)
};

ModifierKeys AndroidComponentPeer::currentModifiers = 0;
Point<float> AndroidComponentPeer::lastMousePos;
int64 AndroidComponentPeer::touchesDown = 0;
AndroidComponentPeer* AndroidComponentPeer::frontWindow = nullptr;

//==============================================================================
#define JUCE_VIEW_CALLBACK(returnType, javaMethodName, params, juceMethodInvocation) \
  JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024ComponentPeerView), javaMethodName, returnType, params) \
  { \
      setEnv (env); \
      if (AndroidComponentPeer* peer = (AndroidComponentPeer*) (pointer_sized_uint) host) \
          peer->juceMethodInvocation; \
  }

JUCE_VIEW_CALLBACK (void, handlePaint,          (JNIEnv* env, jobject /*view*/, jlong host, jobject canvas, jobject paint),                          handlePaintCallback (env, canvas, paint))
JUCE_VIEW_CALLBACK (void, handleMouseDown,      (JNIEnv* env, jobject /*view*/, jlong host, jint i, jfloat x, jfloat y, jlong time),  handleMouseDownCallback (i, Point<float> ((float) x, (float) y), (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseDrag,      (JNIEnv* env, jobject /*view*/, jlong host, jint i, jfloat x, jfloat y, jlong time),  handleMouseDragCallback (i, Point<float> ((float) x, (float) y), (int64) time))
JUCE_VIEW_CALLBACK (void, handleMouseUp,        (JNIEnv* env, jobject /*view*/, jlong host, jint i, jfloat x, jfloat y, jlong time),  handleMouseUpCallback   (i, Point<float> ((float) x, (float) y), (int64) time))
JUCE_VIEW_CALLBACK (void, viewSizeChanged,      (JNIEnv* env, jobject /*view*/, jlong host),                                          handleMovedOrResized())
JUCE_VIEW_CALLBACK (void, focusChanged,         (JNIEnv* env, jobject /*view*/, jlong host, jboolean hasFocus),                       handleFocusChangeCallback (hasFocus))
JUCE_VIEW_CALLBACK (void, handleKeyDown,        (JNIEnv* env, jobject /*view*/, jlong host, jint k, jint kc),                         handleKeyDownCallback ((int) k, (int) kc))
JUCE_VIEW_CALLBACK (void, handleKeyUp,          (JNIEnv* env, jobject /*view*/, jlong host, jint k, jint kc),                         handleKeyUpCallback ((int) k, (int) kc))
JUCE_VIEW_CALLBACK (void, handleBackButton,     (JNIEnv* env, jobject /*view*/, jlong host),                                          handleBackButtonCallback())
JUCE_VIEW_CALLBACK (void, handleKeyboardHidden, (JNIEnv* env, jobject /*view*/, jlong host),                                          handleKeyboardHiddenCallback())
JUCE_VIEW_CALLBACK (void, handleAppResumed,     (JNIEnv* env, jobject /*view*/, jlong host),                                          handleAppResumedCallback())

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void*)
{
    return new AndroidComponentPeer (*this, styleFlags);
}

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getRotation, "getRotation", "()I")

DECLARE_JNI_CLASS (Display, "android/view/Display");
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getDefaultDisplay, "getDefaultDisplay", "()Landroid/view/Display;")

DECLARE_JNI_CLASS (WindowManager, "android/view/WindowManager");
#undef JNI_CLASS_MEMBERS

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    return true;
}

double Desktop::getDefaultMasterScale()
{
    return 1.0;
}

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const
{
    enum
    {
        ROTATION_0   = 0,
        ROTATION_90  = 1,
        ROTATION_180 = 2,
        ROTATION_270 = 3
    };

    JNIEnv* env = getEnv();
    LocalRef<jstring> windowServiceString (javaString ("window"));
    LocalRef<jobject> windowManager = LocalRef<jobject> (env->CallObjectMethod (android.activity, JuceAppActivity.getSystemService, windowServiceString.get()));

    if (windowManager.get() != 0)
    {
        LocalRef<jobject> display = LocalRef<jobject> (env->CallObjectMethod (windowManager, WindowManager.getDefaultDisplay));

        if (display.get() != 0)
        {
            int rotation = env->CallIntMethod (display, Display.getRotation);

            switch (rotation)
            {
                case ROTATION_0:   return upright;
                case ROTATION_90:  return rotatedAntiClockwise;
                case ROTATION_180: return upsideDown;
                case ROTATION_270: return rotatedClockwise;
            }
        }
    }

    jassertfalse;
    return upright;
}

bool MouseInputSource::SourceList::addSource()
{
    addSource (sources.size(), MouseInputSource::InputSourceType::touch);
    return true;
}

bool MouseInputSource::SourceList::canUseTouch()
{
    return true;
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    return AndroidComponentPeer::lastMousePos;
}

void MouseInputSource::setRawMousePosition (Point<float>)
{
    // not needed
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (const int /*keyCode*/)
{
    // TODO
    return false;
}

void ModifierKeys::updateCurrentModifiers() noexcept
{
    currentModifiers = AndroidComponentPeer::currentModifiers;
}

ModifierKeys ModifierKeys::getCurrentModifiersRealtime() noexcept
{
    return AndroidComponentPeer::currentModifiers;
}

JUCE_API void JUCE_CALLTYPE Process::hide()
{
    if (android.activity.callBooleanMethod (JuceAppActivity.moveTaskToBack, true) == 0)
    {
        auto* env = getEnv();

        GlobalRef intent (env->NewObject (AndroidIntent, AndroidIntent.constructor));
        env->CallObjectMethod (intent, AndroidIntent.setAction,   javaString ("android.intent.action.MAIN")  .get());
        env->CallObjectMethod (intent, AndroidIntent.addCategory, javaString ("android.intent.category.HOME").get());

        android.activity.callVoidMethod (JuceAppActivity.startActivity, intent.get());
    }
}

//==============================================================================
// TODO
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess() { return true; }
JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess() {}

//==============================================================================
void JUCE_CALLTYPE NativeMessageBox::showMessageBoxAsync (AlertWindow::AlertIconType /*iconType*/,
                                                          const String& title, const String& message,
                                                          Component* /*associatedComponent*/,
                                                          ModalComponentManager::Callback* callback)
{
    android.activity.callVoidMethod (JuceAppActivity.showMessageBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback);
}

bool JUCE_CALLTYPE NativeMessageBox::showOkCancelBox (AlertWindow::AlertIconType /*iconType*/,
                                                      const String& title, const String& message,
                                                      Component* /*associatedComponent*/,
                                                      ModalComponentManager::Callback* callback)
{
    jassert (callback != nullptr); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (JuceAppActivity.showOkCancelBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback,
                                     javaString (TRANS ("OK")).get(), javaString (TRANS ("Cancel")).get());
    return false;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoCancelBox (AlertWindow::AlertIconType /*iconType*/,
                                                        const String& title, const String& message,
                                                        Component* /*associatedComponent*/,
                                                        ModalComponentManager::Callback* callback)
{
    jassert (callback != nullptr); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (JuceAppActivity.showYesNoCancelBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback);
    return 0;
}

int JUCE_CALLTYPE NativeMessageBox::showYesNoBox (AlertWindow::AlertIconType /*iconType*/,
                                                   const String& title, const String& message,
                                                   Component* /*associatedComponent*/,
                                                   ModalComponentManager::Callback* callback)
{
    jassert (callback != nullptr); // on android, all alerts must be non-modal!!

    android.activity.callVoidMethod (JuceAppActivity.showOkCancelBox, javaString (title).get(),
                                     javaString (message).get(), (jlong) (pointer_sized_int) callback,
                                     javaString (TRANS ("Yes")).get(), javaString (TRANS ("No")).get());
    return 0;
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, alertDismissed, void, (JNIEnv* env, jobject /*activity*/,
                                                                           jlong callbackAsLong, jint result))
{
    setEnv (env);

    if (ModalComponentManager::Callback* callback = (ModalComponentManager::Callback*) callbackAsLong)
    {
        callback->modalStateFinished (result);
        delete callback;
    }
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled)
{
    android.activity.callVoidMethod (JuceAppActivity.setScreenSaver, isEnabled);
}

bool Desktop::isScreenSaverEnabled()
{
    return android.activity.callBooleanMethod (JuceAppActivity.getScreenSaver);
}

//==============================================================================
void Desktop::setKioskComponent (Component* kioskComp, bool enableOrDisable, bool allowMenusAndBars)
{
    ignoreUnused (allowMenusAndBars);

    if (AndroidComponentPeer* peer = dynamic_cast<AndroidComponentPeer*> (kioskComp->getPeer()))
        peer->setFullScreen (enableOrDisable);
    else
        jassertfalse; // (this should have been checked by the caller)
}

//==============================================================================
static jint getAndroidOrientationFlag (int orientations) noexcept
{
    enum
    {
        SCREEN_ORIENTATION_LANDSCAPE          = 0,
        SCREEN_ORIENTATION_PORTRAIT           = 1,
        SCREEN_ORIENTATION_USER               = 2,
        SCREEN_ORIENTATION_REVERSE_LANDSCAPE  = 8,
        SCREEN_ORIENTATION_REVERSE_PORTRAIT   = 9,
        SCREEN_ORIENTATION_USER_LANDSCAPE     = 11,
        SCREEN_ORIENTATION_USER_PORTRAIT      = 12,
    };

    switch (orientations)
    {
        case Desktop::upright:                                          return (jint) SCREEN_ORIENTATION_PORTRAIT;
        case Desktop::upsideDown:                                       return (jint) SCREEN_ORIENTATION_REVERSE_PORTRAIT;
        case Desktop::upright + Desktop::upsideDown:                    return (jint) SCREEN_ORIENTATION_USER_PORTRAIT;
        case Desktop::rotatedAntiClockwise:                             return (jint) SCREEN_ORIENTATION_LANDSCAPE;
        case Desktop::rotatedClockwise:                                 return (jint) SCREEN_ORIENTATION_REVERSE_LANDSCAPE;
        case Desktop::rotatedClockwise + Desktop::rotatedAntiClockwise: return (jint) SCREEN_ORIENTATION_USER_LANDSCAPE;
        default:                                                        return (jint) SCREEN_ORIENTATION_USER;
    }
}

void Desktop::allowedOrientationsChanged()
{
    android.activity.callVoidMethod (JuceAppActivity.setRequestedOrientation,
                                     getAndroidOrientationFlag (allowedOrientations));
}

//==============================================================================
bool juce_areThereAnyAlwaysOnTopWindows()
{
    return false;
}

//==============================================================================
void Desktop::Displays::findDisplays (float masterScale)
{
    Display d;

    d.isMain = true;
    d.dpi = android.dpi;
    d.scale = masterScale * (d.dpi / 150.);
    d.userArea = d.totalArea = Rectangle<int> (android.screenWidth,
                                               android.screenHeight) / d.scale;

    displays.add (d);
}

JUCE_JNI_CALLBACK (JUCE_ANDROID_ACTIVITY_CLASSNAME, setScreenSize, void, (JNIEnv* env, jobject /*activity*/,
                                                                          jint screenWidth, jint screenHeight,
                                                                          jint dpi))
{
    setEnv (env);

    android.screenWidth = screenWidth;
    android.screenHeight = screenHeight;
    android.dpi = dpi;

    const_cast<Desktop::Displays&> (Desktop::getInstance().getDisplays()).refresh();
}

//==============================================================================
Image juce_createIconForFile (const File& /*file*/)
{
    return Image();
}

//==============================================================================
void* CustomMouseCursorInfo::create() const                                                     { return nullptr; }
void* MouseCursor::createStandardMouseCursor (const MouseCursor::StandardCursorType)            { return nullptr; }
void MouseCursor::deleteMouseCursor (void* const /*cursorHandle*/, const bool /*isStandard*/)   {}

//==============================================================================
void MouseCursor::showInWindow (ComponentPeer*) const   {}
void MouseCursor::showInAllWindows() const  {}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& /*files*/, const bool /*canMove*/,
                                                           Component* /*srcComp*/)
{
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& /*text*/, Component* /*srcComp*/)
{
    return false;
}

//==============================================================================
void LookAndFeel::playAlertSound()
{
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    const LocalRef<jstring> t (javaString (text));
    android.activity.callVoidMethod (JuceAppActivity.setClipboardContent, t.get());
}

String SystemClipboard::getTextFromClipboard()
{
    const LocalRef<jstring> text ((jstring) android.activity.callObjectMethod (JuceAppActivity.getClipboardContent));
    return juceString (text);
}

//==============================================================================
const int extendedKeyModifier       = 0x10000;

const int KeyPress::spaceKey        = ' ';
const int KeyPress::returnKey       = 66;
const int KeyPress::escapeKey       = 4;
const int KeyPress::backspaceKey    = 67;
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
const int KeyPress::tabKey          = 61;
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
const int KeyPress::F17Key          = extendedKeyModifier + 50;
const int KeyPress::F18Key          = extendedKeyModifier + 51;
const int KeyPress::F19Key          = extendedKeyModifier + 52;
const int KeyPress::F20Key          = extendedKeyModifier + 53;
const int KeyPress::F21Key          = extendedKeyModifier + 54;
const int KeyPress::F22Key          = extendedKeyModifier + 55;
const int KeyPress::F23Key          = extendedKeyModifier + 56;
const int KeyPress::F24Key          = extendedKeyModifier + 57;
const int KeyPress::F25Key          = extendedKeyModifier + 58;
const int KeyPress::F26Key          = extendedKeyModifier + 59;
const int KeyPress::F27Key          = extendedKeyModifier + 60;
const int KeyPress::F28Key          = extendedKeyModifier + 61;
const int KeyPress::F29Key          = extendedKeyModifier + 62;
const int KeyPress::F30Key          = extendedKeyModifier + 63;
const int KeyPress::F31Key          = extendedKeyModifier + 64;
const int KeyPress::F32Key          = extendedKeyModifier + 65;
const int KeyPress::F33Key          = extendedKeyModifier + 66;
const int KeyPress::F34Key          = extendedKeyModifier + 67;
const int KeyPress::F35Key          = extendedKeyModifier + 68;
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

} // namespace juce
