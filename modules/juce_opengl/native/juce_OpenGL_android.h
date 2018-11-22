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

//==============================================================================
// This byte-code is generated from native/java/com/roli/juce/JuceOpenGLView.java with min sdk version 16
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 javaJuceOpenGLView[] =
{31,139,8,8,110,106,229,91,0,3,74,117,99,101,79,112,101,110,71,76,86,105,101,119,46,100,101,120,0,109,84,63,104,19,81,24,
255,222,221,75,82,211,244,26,211,70,180,130,68,172,160,80,189,170,21,11,169,82,105,169,16,15,139,180,164,90,187,28,151,
211,92,105,239,98,114,77,139,56,20,151,58,185,40,34,34,184,56,22,167,14,213,77,165,187,179,58,56,58,43,82,7,7,127,239,79,
76,10,61,248,189,223,247,190,255,119,247,222,87,241,215,210,195,23,46,210,241,45,159,207,157,188,253,250,215,169,241,167,
83,63,159,111,216,239,78,127,114,30,230,63,252,229,68,53,34,90,43,143,228,72,63,11,208,29,38,165,79,2,159,53,255,0,24,48,
140,37,3,30,101,106,255,2,203,164,73,180,13,126,111,16,125,4,118,128,47,192,111,32,11,219,16,224,0,211,192,12,112,11,88,
0,92,32,0,238,3,15,128,199,192,19,224,37,240,22,216,1,190,2,187,64,130,171,122,16,9,34,37,116,95,41,141,132,238,191,75,
203,207,80,251,128,150,95,65,78,107,249,13,228,110,45,111,118,232,183,12,149,55,43,107,152,50,151,9,75,175,174,153,211,
220,47,235,115,105,23,107,70,178,242,227,218,143,163,211,62,189,239,215,251,188,100,131,14,105,125,235,93,196,99,104,30,
81,105,241,94,92,234,186,185,234,175,86,32,58,47,35,231,33,205,15,18,89,136,101,210,55,207,85,255,45,171,37,227,13,89,
227,40,150,131,224,168,192,104,150,102,198,225,133,180,231,80,112,20,223,76,236,107,227,221,196,111,90,242,27,48,217,207,
96,43,38,43,114,236,27,51,220,69,156,117,198,12,233,152,253,189,211,240,142,112,12,184,236,141,237,121,103,67,238,153,
222,183,229,228,88,16,6,241,21,98,37,234,43,173,120,254,116,205,15,175,57,229,192,95,61,187,232,54,93,58,226,184,97,165,
30,5,21,219,139,194,216,15,99,123,66,240,90,92,236,48,221,171,187,181,106,224,53,236,9,55,108,186,141,34,13,252,55,53,
145,201,158,89,169,223,117,61,95,100,45,210,49,199,139,150,237,122,180,20,216,139,40,104,239,173,90,36,86,38,163,92,34,
179,92,114,32,56,16,156,18,37,61,55,244,252,37,201,168,64,41,79,117,65,153,74,208,168,185,177,87,157,172,187,171,196,171,
81,35,166,116,232,198,65,211,159,173,6,13,202,69,225,213,56,118,189,170,95,153,141,230,130,176,18,173,82,94,234,132,74,
41,110,72,119,234,143,194,73,95,185,78,213,163,101,237,60,176,159,86,71,100,96,67,89,189,227,177,40,216,99,244,230,82,
214,165,233,51,56,152,41,235,58,13,49,43,101,93,222,152,167,1,82,124,2,188,113,103,12,63,128,227,240,241,245,117,190,109,
242,71,6,25,0,3,18,236,155,201,248,31,96,147,27,252,59,239,209,255,145,117,112,107,110,24,29,179,195,236,152,31,173,115,
47,102,72,130,218,115,36,73,237,89,194,10,202,38,230,9,203,182,239,178,81,80,249,197,140,49,181,143,184,31,84,80,177,35,
250,242,10,89,204,176,127,191,250,1,66,252,4,0,0};

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

DECLARE_JNI_CLASS_WITH_BYTECODE (JuceOpenGLViewSurface, "com/roli/juce/JuceOpenGLView", 16, javaJuceOpenGLView, sizeof(javaJuceOpenGLView))
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

    ~NativeContext()
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
