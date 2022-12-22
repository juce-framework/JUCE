/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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
{
    0x1f, 0x8b, 0x08, 0x08, 0xac, 0xdb, 0x8b, 0x62, 0x00, 0x03, 0x4a, 0x61,
    0x76, 0x61, 0x44, 0x65, 0x78, 0x42, 0x79, 0x74, 0x65, 0x43, 0x6f, 0x64,
    0x65, 0x2e, 0x64, 0x65, 0x78, 0x00, 0x6d, 0x54, 0x4f, 0x48, 0x14, 0x51,
    0x18, 0xff, 0x66, 0xe6, 0xed, 0x6e, 0xea, 0x3a, 0xae, 0xeb, 0x7f, 0xc1,
    0xd8, 0x20, 0xea, 0x64, 0x6b, 0x7f, 0xac, 0x40, 0x0b, 0x4d, 0xfb, 0xb7,
    0x0d, 0x4a, 0x69, 0x5b, 0x6c, 0x1d, 0x9a, 0x66, 0x27, 0x77, 0x44, 0x67,
    0x96, 0xd9, 0xd9, 0x55, 0x28, 0x44, 0xba, 0x78, 0xf1, 0x54, 0x41, 0xd1,
    0xd9, 0x43, 0x04, 0x45, 0x20, 0x05, 0x75, 0x48, 0xa2, 0x8b, 0xe1, 0xa1,
    0x53, 0xd8, 0x21, 0xa8, 0x43, 0x07, 0x8f, 0x9d, 0xc2, 0x93, 0xf4, 0x7b,
    0x6f, 0x9e, 0xad, 0x85, 0xc3, 0xfe, 0xe6, 0xfb, 0xfb, 0xbe, 0xef, 0xf7,
    0xde, 0xce, 0xfb, 0xf2, 0xf6, 0x6c, 0x6d, 0xcf, 0xd1, 0x5e, 0x7a, 0xbc,
    0x7e, 0xe1, 0x25, 0x7d, 0xb9, 0xba, 0x35, 0x37, 0x59, 0xff, 0xe2, 0xd1,
    0xda, 0xeb, 0x7b, 0x2b, 0x67, 0xb6, 0x8c, 0xbb, 0xef, 0x2f, 0x0e, 0x3f,
    0x89, 0x10, 0x15, 0x89, 0x68, 0x36, 0x7b, 0x2c, 0x49, 0xf2, 0xd9, 0x64,
    0x44, 0x5d, 0x14, 0xfa, 0xf7, 0x00, 0x3f, 0x81, 0x18, 0xc0, 0x14, 0x22,
    0xfc, 0xe8, 0x3a, 0x5e, 0xf5, 0x90, 0xb7, 0xa4, 0xbd, 0x8a, 0xd7, 0x2b,
    0x8d, 0x68, 0x03, 0x32, 0x0a, 0xa9, 0x03, 0x8d, 0xc0, 0x01, 0x60, 0x10,
    0xb8, 0x09, 0xcc, 0x00, 0x0f, 0x81, 0x65, 0xe0, 0x0d, 0xf0, 0x0e, 0x58,
    0x01, 0x3e, 0x02, 0xab, 0xc0, 0x1a, 0xf0, 0x19, 0x58, 0x07, 0xbe, 0xf3,
    0x5a, 0xc0, 0x6f, 0xa0, 0x01, 0x5c, 0x5a, 0x80, 0x7d, 0x40, 0x2f, 0x60,
    0x00, 0xb7, 0x81, 0x39, 0x60, 0x11, 0x78, 0xc0, 0x42, 0x0e, 0x1a, 0xe7,
    0x07, 0x60, 0x3b, 0x14, 0x95, 0x7c, 0x39, 0xf7, 0x7a, 0x29, 0xa3, 0x72,
    0x6f, 0x35, 0x52, 0xff, 0xaa, 0x12, 0xd5, 0x4a, 0xfd, 0x07, 0xf4, 0x3a,
    0xa9, 0x6f, 0x40, 0x8f, 0x4b, 0xfd, 0xd7, 0x0e, 0xff, 0x26, 0x74, 0x5d,
    0xd6, 0xe5, 0xcd, 0x78, 0x9f, 0x66, 0xd1, 0x53, 0x13, 0x75, 0x19, 0x3c,
    0x49, 0xc9, 0xa1, 0x55, 0xca, 0x76, 0xc1, 0x87, 0x89, 0x38, 0xcf, 0x6f,
    0x10, 0x32, 0xcc, 0x8b, 0xa0, 0x6a, 0x93, 0xf4, 0xb7, 0x0a, 0xa9, 0x50,
    0x9b, 0xb4, 0xdb, 0xa5, 0xdd, 0x21, 0xa4, 0x4a, 0x9d, 0xd2, 0xaf, 0xc8,
    0xba, 0xfc, 0x51, 0xa5, 0xfc, 0x24, 0x1d, 0x51, 0x44, 0xb8, 0xef, 0x29,
    0x0b, 0xf7, 0x55, 0x4c, 0x11, 0x1d, 0x11, 0x95, 0x73, 0xd0, 0x72, 0xfb,
    0x39, 0x7b, 0x4d, 0x54, 0x20, 0x5a, 0x62, 0xd5, 0xbe, 0x3c, 0xaa, 0x8b,
    0xf5, 0xaa, 0xa8, 0xfd, 0x1c, 0xaf, 0x46, 0x48, 0x2f, 0xa5, 0xd0, 0x38,
    0x8d, 0x0d, 0x20, 0x0b, 0x65, 0x0f, 0xa3, 0xe1, 0x49, 0xec, 0x9d, 0xdb,
    0xc5, 0x81, 0x38, 0xb1, 0xcb, 0xba, 0x38, 0x86, 0x90, 0xc5, 0x32, 0x0b,
    0xf9, 0x24, 0x13, 0x0d, 0x82, 0x37, 0x3f, 0x91, 0xb7, 0xdb, 0x75, 0x12,
    0xbc, 0xee, 0xae, 0x75, 0x7a, 0x6a, 0xf0, 0x45, 0xe9, 0x72, 0xaf, 0x7c,
    0xcd, 0x07, 0xb9, 0x66, 0xf7, 0xec, 0x3a, 0x64, 0x7b, 0x09, 0x0d, 0xd5,
    0x74, 0x79, 0x16, 0xd5, 0x73, 0x50, 0x85, 0xad, 0x48, 0xfb, 0x7f, 0x5d,
    0xa3, 0x68, 0xbf, 0xe3, 0x3a, 0xc1, 0x69, 0x52, 0x32, 0xd4, 0x94, 0x29,
    0x5b, 0xf6, 0x68, 0xd1, 0x76, 0xcf, 0x1b, 0x59, 0xc7, 0x9e, 0x39, 0x34,
    0x69, 0x56, 0x4c, 0xea, 0x30, 0x4c, 0x37, 0xef, 0x7b, 0x4e, 0x3e, 0x6d,
    0x79, 0x6e, 0x60, 0xbb, 0x41, 0x7a, 0x88, 0xcb, 0xd9, 0xa0, 0x6f, 0x47,
    0x68, 0xc2, 0x37, 0x8b, 0x05, 0xc7, 0x2a, 0xa5, 0x87, 0x4c, 0xb7, 0x62,
    0x96, 0x76, 0x0d, 0x5d, 0xb1, 0x27, 0x1c, 0xcf, 0xed, 0xa3, 0xce, 0xbf,
    0xa1, 0x0a, 0x9a, 0xa4, 0xc7, 0xca, 0xfe, 0x1d, 0xd3, 0xb2, 0x79, 0xc3,
    0x3e, 0xda, 0x6b, 0x58, 0xde, 0x74, 0xda, 0x9f, 0x2e, 0x4d, 0xa5, 0x27,
    0xc1, 0x25, 0xfd, 0x2f, 0xa1, 0x3e, 0x52, 0xb2, 0xa4, 0x66, 0x33, 0xa4,
    0x65, 0x33, 0x06, 0x14, 0x03, 0x8a, 0x91, 0x21, 0x25, 0x47, 0x6a, 0xce,
    0xa0, 0xa8, 0x65, 0xba, 0x96, 0x3d, 0x25, 0x24, 0x38, 0x50, 0xcc, 0x0a,
    0x79, 0x52, 0x3c, 0xef, 0x94, 0x8a, 0x66, 0x60, 0x15, 0x86, 0x7d, 0x73,
    0x86, 0xda, 0x26, 0xcc, 0xa0, 0x60, 0xfb, 0xe3, 0xbe, 0xe9, 0xc2, 0xeb,
    0x63, 0x43, 0x21, 0x31, 0x62, 0x05, 0xaf, 0x14, 0x50, 0xad, 0x6b, 0x06,
    0x4e, 0xc5, 0x1e, 0x2f, 0x38, 0x25, 0x4a, 0x7a, 0xee, 0x60, 0x10, 0x98,
    0x56, 0xc1, 0xce, 0x8f, 0x7b, 0xd7, 0x1c, 0x37, 0xef, 0xcd, 0x50, 0x8b,
    0xf0, 0x71, 0x57, 0xe8, 0x18, 0x11, 0xe9, 0xd4, 0xec, 0xb9, 0xc3, 0x76,
    0x98, 0x7a, 0xce, 0xf7, 0xa6, 0x65, 0x72, 0xe7, 0x6e, 0x5e, 0xb9, 0x22,
    0x8e, 0x18, 0xf8, 0x48, 0x8b, 0x05, 0xbc, 0x61, 0xb4, 0xec, 0x96, 0x4b,
    0x76, 0x9e, 0x0e, 0xaa, 0xc9, 0xd6, 0x98, 0x7e, 0x62, 0xb4, 0x9b, 0xba,
    0x29, 0xa6, 0x5f, 0xa2, 0x11, 0xa5, 0x31, 0xa6, 0x9f, 0x5a, 0xc8, 0xd1,
    0x71, 0xa5, 0x2b, 0xa6, 0x53, 0x3f, 0x85, 0xd6, 0x59, 0xc8, 0x85, 0x1b,
    0xfd, 0xf8, 0x27, 0x19, 0xee, 0x02, 0x9b, 0x9f, 0x67, 0x1b, 0x5a, 0xe4,
    0xbe, 0x4a, 0x2a, 0xa0, 0x00, 0x11, 0x65, 0x91, 0x29, 0xec, 0x19, 0x53,
    0x94, 0x6f, 0x90, 0xbf, 0x98, 0xca, 0x96, 0x22, 0xf2, 0xde, 0xd3, 0x8e,
    0xef, 0x84, 0xcb, 0xed, 0x99, 0xa6, 0x52, 0x75, 0xae, 0x69, 0x54, 0x9d,
    0x6d, 0x8c, 0xaa, 0xf3, 0x6d, 0xbb, 0x06, 0x9f, 0x71, 0x51, 0xaa, 0xce,
    0x39, 0x25, 0x25, 0xe7, 0x04, 0xd7, 0x13, 0xd5, 0x59, 0xa2, 0xa6, 0xc2,
    0xfa, 0x7c, 0xfe, 0x69, 0x32, 0x87, 0xdf, 0x45, 0x4a, 0x85, 0x6b, 0xc5,
    0x3d, 0x4d, 0x84, 0x3a, 0x9f, 0xaf, 0x7f, 0x00, 0x34, 0xf2, 0xd3, 0x47,
    0x98, 0x05, 0x00, 0x00
};

//==============================================================================
//==============================================================================
class OpenGLContext::NativeContext : private SurfaceHolderCallback
{
public:
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& pixelFormat,
                   void* /*contextToShareWith*/,
                   bool useMultisamplingIn,
                   OpenGLVersion)
        : component (comp)
    {
        auto env = getEnv();

        // Do we have a native peer that we can attach to?
        if (component.getPeer()->getNativeHandle() == nullptr)
            return;

        // Initialise the EGL display
        if (! initEGLDisplay (pixelFormat, useMultisamplingIn))
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
    InitResult initialiseOnRenderThread (OpenGLContext& ctx)
    {
        // The "real" initialisation happens when the surface is created. Here, we'll
        // just return true if the initialisation happened successfully, or false if
        // it hasn't happened yet, or was unsuccessful.
        const std::lock_guard lock { nativeHandleMutex };

        if (! hasInitialised)
            return InitResult::fatal;

        if (context.get() == EGL_NO_CONTEXT && surface.get() == EGL_NO_SURFACE)
            return InitResult::retry;

        juceContext = &ctx;
        return InitResult::success;
    }

    void shutdownOnRenderThread()
    {
        const std::lock_guard lock { nativeHandleMutex };
        juceContext = nullptr;
    }

    //==============================================================================
    bool makeActive() const noexcept
    {
        const std::lock_guard lock { nativeHandleMutex };

        return hasInitialised
            && surface.get() != EGL_NO_SURFACE
            && context.get() != EGL_NO_CONTEXT
            && eglMakeCurrent (display, surface.get(), surface.get(), context.get());
    }

    bool isActive() const noexcept
    {
        const std::lock_guard lock { nativeHandleMutex };
        return eglGetCurrentContext() == context.get();
    }

    static void deactivateCurrentContext()
    {
        eglMakeCurrent (display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    //==============================================================================
    void swapBuffers() const noexcept           { eglSwapBuffers (display, surface.get()); }
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
    void surfaceChanged ([[maybe_unused]] LocalRef<jobject> holder,
                         [[maybe_unused]] int format,
                         [[maybe_unused]] int width,
                         [[maybe_unused]] int height) override
    {
    }

    void surfaceCreated (LocalRef<jobject>) override;
    void surfaceDestroyed (LocalRef<jobject>) override;

    //==============================================================================
    struct Locker
    {
        explicit Locker (NativeContext& ctx) : lock (ctx.mutex) {}
        const ScopedLock lock;
    };

    Component& component;

private:
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
     METHOD (constructor, "<init>",    "(Landroid/content/Context;J)V") \
     METHOD (getParent,   "getParent", "()Landroid/view/ViewParent;") \
     METHOD (getHolder,   "getHolder", "()Landroid/view/SurfaceHolder;") \
     METHOD (layout,      "layout",    "(IIII)V" ) \
     CALLBACK (generatedCallback<&NativeContext::attachedToWindow>,   "onAttchedWindowNative",      "(J)V") \
     CALLBACK (generatedCallback<&NativeContext::detachedFromWindow>, "onDetachedFromWindowNative", "(J)V") \
     CALLBACK (generatedCallback<&NativeContext::dispatchDraw>,       "onDrawNative",               "(JLandroid/graphics/Canvas;)V")

     DECLARE_JNI_CLASS_WITH_BYTECODE (JuceOpenGLViewSurface, "com/rmsl/juce/JuceOpenGLView", 16, javaJuceOpenGLView)
    #undef JNI_CLASS_MEMBERS

    //==============================================================================
    static void attachedToWindow (JNIEnv* env, NativeContext& t)
    {
        LocalRef<jobject> holder (env->CallObjectMethod (t.surfaceView.get(), JuceOpenGLViewSurface.getHolder));

        if (t.surfaceHolderCallback == nullptr)
            t.surfaceHolderCallback = GlobalRef (CreateJavaInterface (&t, "android/view/SurfaceHolder$Callback"));

        env->CallVoidMethod (holder, AndroidSurfaceHolder.addCallback, t.surfaceHolderCallback.get());
    }

    static void detachedFromWindow (JNIEnv* env, NativeContext& t)
    {
        if (t.surfaceHolderCallback != nullptr)
        {
            LocalRef<jobject> holder (env->CallObjectMethod (t.surfaceView.get(), JuceOpenGLViewSurface.getHolder));

            env->CallVoidMethod (holder.get(), AndroidSurfaceHolder.removeCallback, t.surfaceHolderCallback.get());
            t.surfaceHolderCallback.clear();
        }
    }

    static void dispatchDraw (JNIEnv*, NativeContext& t, jobject /*canvas*/)
    {
        const std::lock_guard lock { t.nativeHandleMutex };

        if (t.juceContext != nullptr)
            t.juceContext->triggerRepaint();
    }

    bool tryChooseConfig (const std::vector<EGLint>& optionalAttribs)
    {
        std::vector<EGLint> allAttribs
        {
            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
            EGL_BLUE_SIZE,          8,
            EGL_GREEN_SIZE,         8,
            EGL_RED_SIZE,           8,
            EGL_ALPHA_SIZE,         0,
            EGL_DEPTH_SIZE,         16
        };

        allAttribs.insert (allAttribs.end(), optionalAttribs.begin(), optionalAttribs.end());

        allAttribs.push_back (EGL_NONE);

        EGLint numConfigs{};
        return eglChooseConfig (display, allAttribs.data(), &config, 1, &numConfigs);
    }

    //==============================================================================
    bool initEGLDisplay (const OpenGLPixelFormat& pixelFormat, bool multisample)
    {
        // already initialised?
        if (display != EGL_NO_DISPLAY)
            return true;

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

        if (tryChooseConfig ({ EGL_SAMPLE_BUFFERS, multisample ? 1 : 0, EGL_SAMPLES, pixelFormat.multisamplingLevel }))
            return true;

        if (tryChooseConfig ({}))
            return true;

        eglTerminate (display);
        jassertfalse;
        return false;
    }

    struct NativeWindowReleaser
    {
        void operator() (ANativeWindow* ptr) const { if (ptr != nullptr) ANativeWindow_release (ptr); }
    };

    std::unique_ptr<ANativeWindow, NativeWindowReleaser> getNativeWindow() const
    {
        auto* env = getEnv();

        const LocalRef<jobject> holder (env->CallObjectMethod (surfaceView.get(), JuceOpenGLViewSurface.getHolder));

        if (holder == nullptr)
            return nullptr;

        const LocalRef<jobject> jSurface (env->CallObjectMethod (holder.get(), AndroidSurfaceHolder.getSurface));

        if (jSurface == nullptr)
            return nullptr;

        constexpr auto numAttempts = 2;

        for (auto i = 0; i < numAttempts; Thread::sleep (200), ++i)
            if (auto* ptr = ANativeWindow_fromSurface (env, jSurface.get()))
                return std::unique_ptr<ANativeWindow, NativeWindowReleaser> { ptr };

        return nullptr;
    }

    //==============================================================================
    CriticalSection mutex;
    bool hasInitialised = false;

    GlobalRef surfaceView;
    Rectangle<int> lastBounds;

    struct SurfaceDestructor
    {
        void operator() (EGLSurface x) const { if (x != EGL_NO_SURFACE) eglDestroySurface (display, x); }
    };

    struct ContextDestructor
    {
        void operator() (EGLContext x) const { if (x != EGL_NO_CONTEXT) eglDestroyContext (display, x); }
    };

    mutable std::mutex nativeHandleMutex;
    OpenGLContext* juceContext = nullptr;
    std::unique_ptr<std::remove_pointer_t<EGLSurface>, SurfaceDestructor> surface { EGL_NO_SURFACE };
    std::unique_ptr<std::remove_pointer_t<EGLContext>, ContextDestructor> context { EGL_NO_CONTEXT };

    GlobalRef surfaceHolderCallback;

    static EGLDisplay display;
    static EGLConfig config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

EGLDisplay OpenGLContext::NativeContext::display = EGL_NO_DISPLAY;
EGLDisplay OpenGLContext::NativeContext::config;

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return eglGetCurrentContext() != EGL_NO_CONTEXT;
}

} // namespace juce
