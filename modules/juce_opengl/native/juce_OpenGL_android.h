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

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getParent, "getParent", "()Landroid/view/ViewParent;") \
 METHOD (layout, "layout", "(IIII)V" ) \
 METHOD (getNativeSurface,     "getNativeSurface",        "()Landroid/view/Surface;") \

DECLARE_JNI_CLASS (NativeSurfaceView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$NativeSurfaceView")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (addView,        "addView",        "(Landroid/view/View;)V") \
 METHOD (removeView,     "removeView",        "(Landroid/view/View;)V") \

DECLARE_JNI_CLASS (AndroidViewGroup, "android/view/ViewGroup")
#undef JNI_CLASS_MEMBERS

//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& pixelFormat,
                   void* /*contextToShareWith*/,
                   bool /*useMultisampling*/,
                   OpenGLVersion)
        : component (comp),
          hasInitialised (false),
          juceContext (nullptr), surface (EGL_NO_SURFACE), context (EGL_NO_CONTEXT)
    {
        JNIEnv* env = getEnv();

        // Do we have a native peer that we can attach to?
        if (component.getPeer()->getNativeHandle() == nullptr)
            return;

        // Initialise the EGL display
        if (! initEGLDisplay())
            return;

        // create a native surface view
        surfaceView = GlobalRef (env->CallObjectMethod (android.activity.get(),
                                                        JuceAppActivity.createNativeSurfaceView,
                                                        reinterpret_cast<jlong> (this)));
        if (surfaceView.get() == nullptr)
            return;

        // add the view to the view hierachy
        // after this the nativecontext can receive callbacks
        env->CallVoidMethod ((jobject) component.getPeer()->getNativeHandle(),
                             AndroidViewGroup.addView, surfaceView.get());

        // initialise the geometry of the view
        Rectangle<int> bounds = component.getTopLevelComponent()
            ->getLocalArea (&component, component.getLocalBounds());
        bounds *= component.getDesktopScaleFactor();

        updateWindowPosition (bounds);
        hasInitialised = true;
    }

    ~NativeContext()
    {
        JNIEnv* env = getEnv();

        if (jobject viewParent = env->CallObjectMethod (surfaceView.get(), NativeSurfaceView.getParent))
            env->CallVoidMethod (viewParent, AndroidViewGroup.removeView, surfaceView.get());
    }

    //==============================================================================
    void initialiseOnRenderThread (OpenGLContext& aContext)
    {
        jassert (hasInitialised);

        // has the context already attached?
        jassert (surface == EGL_NO_SURFACE && context == EGL_NO_CONTEXT);

        JNIEnv* env = getEnv();

        // get a pointer to the native window
        ANativeWindow* window = nullptr;
        if (jobject jSurface = env->CallObjectMethod (surfaceView.get(), NativeSurfaceView.getNativeSurface))
            window = ANativeWindow_fromSurface (env, jSurface);

        jassert (window != nullptr);

        // create the surface
        surface = eglCreateWindowSurface(display, config, window, 0);
        jassert (surface != EGL_NO_SURFACE);

        ANativeWindow_release (window);

        // create the OpenGL context
        EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
        jassert (context != EGL_NO_CONTEXT);

        juceContext = &aContext;
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
    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        if (lastBounds != bounds)
        {
            JNIEnv* env = getEnv();

            lastBounds = bounds;
            Rectangle<int> r = bounds * Desktop::getInstance().getDisplays().getMainDisplay().scale;

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
    bool hasInitialised, hasBeenAddedToViewHierachy;

    GlobalRef surfaceView;
    Rectangle<int> lastBounds;

    OpenGLContext* juceContext;
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
