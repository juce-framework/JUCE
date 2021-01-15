/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
// This byte-code is generated from native/java/com/rmsl/juce/JuceOpenGLView.java with min sdk version 16
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 javaJuceOpenGLView[] =
{31,139,8,8,95,114,161,94,0,3,74,117,99,101,79,112,101,110,71,76,86,105,101,119,83,117,114,102,97,99,101,46,100,101,120,0,109,
84,75,107,19,81,20,62,119,230,38,169,105,76,99,218,138,86,144,32,5,55,173,83,53,98,33,173,86,90,170,196,193,34,173,81,106,93,
12,147,169,153,210,204,196,100,154,22,65,40,110,234,170,43,23,34,46,93,22,87,62,138,27,65,168,235,174,213,133,11,23,254,129,
162,136,130,223,125,196,164,208,129,239,126,231,125,206,188,78,217,91,75,142,156,191,64,151,223,190,187,182,183,251,251,212,240,
247,216,223,197,205,174,170,255,233,209,230,135,123,183,222,252,225,68,53,34,90,43,229,179,164,175,5,216,142,145,178,199,129,
93,205,63,0,6,140,224,72,129,71,153,210,159,225,152,50,137,182,193,239,13,162,143,192,14,240,25,216,3,50,240,13,1,54,48,3,204,
2,119,128,5,192,1,124,224,1,240,16,120,2,108,2,207,129,87,192,14,240,5,248,9,196,184,234,7,145,32,82,76,207,149,208,136,233,
249,187,180,252,20,189,15,105,249,5,228,164,150,95,66,238,214,242,86,135,253,181,161,234,102,100,15,83,214,50,225,233,209,61,179,
154,251,100,127,46,253,226,76,73,86,113,92,199,113,76,218,171,245,62,173,247,75,54,232,168,182,183,238,69,92,134,230,188,42,139,
251,226,210,214,205,213,124,181,28,209,57,153,57,15,105,126,144,40,141,92,38,99,251,185,154,191,229,77,203,124,67,246,56,129,
227,8,56,204,49,154,163,217,9,68,161,236,89,52,28,197,51,19,122,109,162,155,248,205,180,124,6,76,206,51,216,202,201,136,26,7,
230,140,116,17,103,157,57,67,58,231,224,232,36,162,67,124,6,92,206,198,246,221,179,33,117,166,245,182,28,31,243,3,63,186,68,172,
72,189,197,21,215,155,169,121,193,85,187,228,123,171,103,150,156,166,67,199,109,39,40,215,67,191,108,185,97,16,121,65,100,77,
10,94,139,10,29,174,251,117,167,86,241,221,134,53,233,4,77,167,81,160,129,255,174,38,42,89,179,43,245,69,199,245,68,213,2,157,
180,221,176,106,213,171,141,101,107,9,13,173,253,93,11,196,74,100,148,138,100,150,138,54,4,27,130,93,164,184,235,4,174,183,44,
25,29,40,225,170,41,40,85,246,27,53,39,114,43,83,117,103,149,120,37,108,68,148,12,156,200,111,122,115,21,191,65,217,48,184,18,
69,142,91,241,202,115,225,109,63,40,135,171,212,47,109,194,164,12,55,100,56,245,133,193,148,167,66,167,235,97,85,7,15,28,100,
213,25,41,248,208,86,107,60,18,13,79,27,61,217,68,250,226,204,48,13,83,34,125,157,166,89,58,145,30,223,152,167,60,41,30,7,111,
220,29,195,11,224,248,248,248,250,58,223,54,249,99,131,12,128,1,49,246,213,100,252,23,176,197,13,254,141,31,214,239,145,117,
112,107,111,24,29,187,195,236,216,31,173,239,94,236,144,24,181,247,72,156,218,187,132,229,148,79,236,19,150,105,255,203,70,78,
213,23,59,198,212,49,226,255,160,156,202,205,235,159,87,200,98,135,253,3,40,26,5,36,252,4,0,0,0,0};

//==============================================================================
struct AndroidGLCallbacks
{
    static void attachedToWindow   (JNIEnv*, jobject, jlong);
    static void detachedFromWindow (JNIEnv*, jobject, jlong);
    static void dispatchDraw       (JNIEnv*, jobject, jlong, jobject);
};

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>",    "(Landroid/content/Context;J)V") \
 METHOD (getParent,   "getParent", "()Landroid/view/ViewParent;") \
 METHOD (getHolder,   "getHolder", "()Landroid/view/SurfaceHolder;") \
 METHOD (layout,      "layout",    "(IIII)V" ) \
 CALLBACK (AndroidGLCallbacks::attachedToWindow,   "onAttchedWindowNative",      "(J)V") \
 CALLBACK (AndroidGLCallbacks::detachedFromWindow, "onDetachedFromWindowNative", "(J)V") \
 CALLBACK (AndroidGLCallbacks::dispatchDraw,       "onDrawNative",               "(JLandroid/graphics/Canvas;)V")

DECLARE_JNI_CLASS_WITH_BYTECODE (JuceOpenGLViewSurface, "com/rmsl/juce/JuceOpenGLView", 16, javaJuceOpenGLView, sizeof(javaJuceOpenGLView))
#undef JNI_CLASS_MEMBERS

//==============================================================================
class OpenGLContext::NativeContext   : private SurfaceHolderCallback
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
        surfaceView = GlobalRef (LocalRef<jobject>(env->NewObject (JuceOpenGLViewSurface,
                                                                   JuceOpenGLViewSurface.constructor,
                                                                   getAppContext().get(),
                                                                   reinterpret_cast<jlong> (this))));
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

    ~NativeContext() override
    {
        auto env = getEnv();

        if (jobject viewParent = env->CallObjectMethod (surfaceView.get(), JuceOpenGLViewSurface.getParent))
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

        LocalRef<jobject> holder (env->CallObjectMethod (surfaceView.get(), JuceOpenGLViewSurface.getHolder));

        if (holder != nullptr)
        {
            LocalRef<jobject> jSurface (env->CallObjectMethod (holder.get(), AndroidSurfaceHolder.getSurface));

            if (jSurface != nullptr)
            {
                window = ANativeWindow_fromSurface(env, jSurface.get());

                // if we didn't succeed the first time, wait 25ms and try again
                if (window == nullptr)
                {
                    Thread::sleep (200);
                    window = ANativeWindow_fromSurface (env, jSurface.get());
                }
            }
        }

        if (window == nullptr)
        {
            // failed to get a pointer to the native window after second try so bail out
            jassertfalse;
            return false;
        }

        // create the surface
        surface = eglCreateWindowSurface (display, config, window, nullptr);
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
            auto r = bounds * Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale;

            env->CallVoidMethod (surfaceView.get(), JuceOpenGLViewSurface.layout,
                                 (jint) r.getX(), (jint) r.getY(), (jint) r.getRight(), (jint) r.getBottom());
        }
    }

    //==============================================================================
    // Android Surface Callbacks:
    void surfaceChanged (LocalRef<jobject> holder, int format, int width, int height) override
    {
        ignoreUnused (holder, format, width, height);
    }

    void surfaceCreated (LocalRef<jobject> holder) override;
    void surfaceDestroyed (LocalRef<jobject> holder) override;

    //==============================================================================
    struct Locker { Locker (NativeContext&) {} };

    Component& component;

private:
    //==============================================================================
    friend struct AndroidGLCallbacks;

    void attachedToWindow()
    {
        auto* env = getEnv();

        LocalRef<jobject> holder (env->CallObjectMethod (surfaceView.get(), JuceOpenGLViewSurface.getHolder));

        if (surfaceHolderCallback == nullptr)
            surfaceHolderCallback = GlobalRef (CreateJavaInterface (this, "android/view/SurfaceHolder$Callback"));

        env->CallVoidMethod (holder, AndroidSurfaceHolder.addCallback, surfaceHolderCallback.get());
    }

    void detachedFromWindow()
    {
        if (surfaceHolderCallback != nullptr)
        {
            auto* env = getEnv();

            LocalRef<jobject> holder (env->CallObjectMethod (surfaceView.get(), JuceOpenGLViewSurface.getHolder));

            env->CallVoidMethod (holder.get(), AndroidSurfaceHolder.removeCallback, surfaceHolderCallback.get());
            surfaceHolderCallback.clear();
        }
    }

    void dispatchDraw (jobject /*canvas*/)
    {
        if (juceContext != nullptr)
            juceContext->triggerRepaint();
    }

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

        if (! eglInitialize (display, nullptr, nullptr))
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

    GlobalRef surfaceHolderCallback;

    static EGLDisplay display;
    static EGLConfig config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
void AndroidGLCallbacks::attachedToWindow (JNIEnv*, jobject /*this*/, jlong host)
{
    if (auto* nativeContext = reinterpret_cast<OpenGLContext::NativeContext*> (host))
        nativeContext->attachedToWindow();
}

void AndroidGLCallbacks::detachedFromWindow (JNIEnv*, jobject /*this*/, jlong host)
{
    if (auto* nativeContext = reinterpret_cast<OpenGLContext::NativeContext*> (host))
        nativeContext->detachedFromWindow();
}

void AndroidGLCallbacks::dispatchDraw (JNIEnv*, jobject /*this*/, jlong host, jobject canvas)
{
    if (auto* nativeContext = reinterpret_cast<OpenGLContext::NativeContext*> (host))
        nativeContext->dispatchDraw (canvas);
}

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return eglGetCurrentContext() != EGL_NO_CONTEXT;
}

} // namespace juce
