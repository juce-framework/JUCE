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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getParent, "getParent", "()Landroid/view/ViewParent;") \
 METHOD (layout, "layout", "(IIII)V" ) \
 METHOD (getNativeSurface,     "getNativeSurface",        "()Landroid/view/Surface;") \

DECLARE_JNI_CLASS (NativeSurfaceView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$NativeSurfaceView")
#undef JNI_CLASS_MEMBERS

//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& /*pixelFormat*/,
                   void* /*contextToShareWith*/,
                   bool /*useMultisampling*/,
                   OpenGLVersion)
        : component (comp),
          surface (EGL_NO_SURFACE), context (EGL_NO_CONTEXT)
    {
        auto env = getEnv();

        // Do we have a native peer that we can attach to?
        if (component.getPeer()->getNativeHandle() == nullptr)
            return;

        // Initialise the EGL display
        if (! initEGLDisplay())
            return;

        // create a native surface view
        surfaceView = GlobalRef (env->CallObjectMethod (android.activity.get(),
                                                        JuceAppActivity.createNativeSurfaceView,
                                                        reinterpret_cast<jlong> (this),
                                                        false));
        if (surfaceView.get() == nullptr)
            return;

        // add the view to the view hierarchy
        // after this the nativecontext can receive callbacks
        env->CallVoidMethod ((jobject) component.getPeer()->getNativeHandle(),
                             AndroidViewGroup.addView, surfaceView.get());

        // initialise the geometry of the view
        auto bounds = component.getTopLevelComponent()->getLocalArea (&component, component.getLocalBounds());
        bounds *= component.getDesktopScaleFactor();

        updateWindowPosition (bounds);
        hasInitialised = true;
    }

    ~NativeContext()
    {
        auto env = getEnv();

        if (jobject viewParent = env->CallObjectMethod (surfaceView.get(), NativeSurfaceView.getParent))
            env->CallVoidMethod (viewParent, AndroidViewGroup.removeView, surfaceView.get());
    }

    //==============================================================================
    bool initialiseOnRenderThread (OpenGLContext& aContext)
    {
        jassert (hasInitialised);

        // has the context already attached?
        jassert (surface == EGL_NO_SURFACE && context == EGL_NO_CONTEXT);

        auto env = getEnv();

        ANativeWindow* window = nullptr;

        if (jobject jSurface = env->CallObjectMethod (surfaceView.get(), NativeSurfaceView.getNativeSurface))
        {
            window = ANativeWindow_fromSurface (env, jSurface);

            // if we didn't succeed the first time, wait briefly and try again..
            if (window == nullptr)
            {
                Thread::sleep (200);
                window = ANativeWindow_fromSurface (env, jSurface);
            }
        }

        if (window == nullptr)
        {
            // failed to get a pointer to the native window after second try so bail out
            jassertfalse;
            return false;
        }

        // create the surface
        surface = eglCreateWindowSurface (display, config, window, 0);
        jassert (surface != EGL_NO_SURFACE);

        ANativeWindow_release (window);

        // create the OpenGL context
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
        context = eglCreateContext (display, config, EGL_NO_CONTEXT, contextAttribs);
        jassert (context != EGL_NO_CONTEXT);

        juceContext = &aContext;
        return true;
    }

    void shutdownOnRenderThread()
    {
        jassert (hasInitialised);

        // is there a context available to detach?
        jassert (surface != EGL_NO_SURFACE && context != EGL_NO_CONTEXT);

        eglDestroyContext (display, context);
        context = EGL_NO_CONTEXT;

        eglDestroySurface (display, surface);
        surface = EGL_NO_SURFACE;
    }

    //==============================================================================
    bool makeActive() const noexcept
    {
        if (! hasInitialised)
            return false;

        if (surface == EGL_NO_SURFACE || context == EGL_NO_CONTEXT)
            return false;

        if (! eglMakeCurrent (display, surface, surface, context))
            return false;

        return true;
    }

    bool isActive() const noexcept              { return eglGetCurrentContext() == context; }

    static void deactivateCurrentContext()
    {
        eglMakeCurrent (display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    //==============================================================================
    void swapBuffers() const noexcept           { eglSwapBuffers (display, surface); }
    bool setSwapInterval (const int)            { return false; }
    int getSwapInterval() const                 { return 0; }

    //==============================================================================
    bool createdOk() const noexcept             { return hasInitialised; }
    void* getRawContext() const noexcept        { return surfaceView.get(); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    //==============================================================================
    void updateWindowPosition (Rectangle<int> bounds)
    {
        if (lastBounds != bounds)
        {
            auto env = getEnv();

            lastBounds = bounds;
            auto r = bounds * Desktop::getInstance().getDisplays().getMainDisplay().scale;

            env->CallVoidMethod (surfaceView.get(), NativeSurfaceView.layout,
                                 (jint) r.getX(), (jint) r.getY(), (jint) r.getRight(), (jint) r.getBottom());
        }
    }

    //==============================================================================
    // Android Surface Callbacks:

    void dispatchDraw (jobject canvas)
    {
        ignoreUnused (canvas);

        if (juceContext != nullptr)
            juceContext->triggerRepaint();
    }

    void surfaceChanged (jobject holder, int format, int width, int height)
    {
        ignoreUnused (holder, format, width, height);
    }

    void surfaceCreated (jobject holder);
    void surfaceDestroyed (jobject holder);

    //==============================================================================
    struct Locker { Locker (NativeContext&) {} };

    Component& component;

private:
    //==============================================================================
    bool initEGLDisplay()
    {
        // already initialised?
        if (display != EGL_NO_DISPLAY)
            return true;

        const EGLint attribs[] =
        {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 0,
            EGL_DEPTH_SIZE, 16,
            EGL_NONE
        };

        EGLint numConfigs;

        if ((display = eglGetDisplay (EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
        {
            jassertfalse;
            return false;
        }

        if (! eglInitialize (display, 0, 0))
        {
            jassertfalse;
            return false;
        }

        if (! eglChooseConfig (display, attribs, &config, 1, &numConfigs))
        {
            eglTerminate (display);
            jassertfalse;
            return false;
        }

        return true;
    }

    //==============================================================================
    bool hasInitialised = false;

    GlobalRef surfaceView;
    Rectangle<int> lastBounds;

    OpenGLContext* juceContext = nullptr;
    EGLSurface surface;
    EGLContext context;

    static EGLDisplay display;
    static EGLConfig config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), dispatchDrawNative,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject canvas))
{
    ignoreUnused (nativeView);
    setEnv (env);
    reinterpret_cast<OpenGLContext::NativeContext*> (host)->dispatchDraw (canvas);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceChangedNative,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder, jint format, jint width, jint height))
{
    ignoreUnused (nativeView);
    setEnv (env);
    reinterpret_cast<OpenGLContext::NativeContext*> (host)->surfaceChanged (holder, format, width, height);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceCreatedNative,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder))
{
    ignoreUnused (nativeView);
    setEnv (env);
    reinterpret_cast<OpenGLContext::NativeContext*> (host)->surfaceCreated (holder);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceDestroyedNative,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder))
{
    ignoreUnused (nativeView);
    setEnv (env);
    reinterpret_cast<OpenGLContext::NativeContext*> (host)->surfaceDestroyed (holder);
}

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return eglGetCurrentContext() != EGL_NO_CONTEXT;
}

} // namespace juce
