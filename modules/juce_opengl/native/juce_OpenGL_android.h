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

//==============================================================================
// This byte-code is generated from native/java/com/rmsl/juce/JuceOpenGLView.java with min sdk version 16
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 javaJuceOpenGLView[] =
{
    0x1f, 0x8b, 0x08, 0x08, 0x7e, 0xb3, 0x66, 0x68, 0x00, 0x03, 0x63, 0x6c,
    0x61, 0x73, 0x73, 0x65, 0x73, 0x2e, 0x64, 0x65, 0x78, 0x00, 0x6d, 0x94,
    0xbd, 0x6f, 0xd3, 0x50, 0x10, 0xc0, 0xef, 0x3d, 0xbb, 0xa1, 0x5f, 0xb4,
    0x69, 0x4b, 0xa9, 0xe8, 0x50, 0x05, 0x33, 0x20, 0xa1, 0xa4, 0x4e, 0xda,
    0xb4, 0x49, 0x9a, 0x82, 0x8a, 0x1a, 0xbe, 0x42, 0x04, 0x82, 0x56, 0x01,
    0x45, 0x0c, 0xbc, 0xd8, 0x2f, 0x8d, 0xdb, 0xc4, 0xb6, 0x6c, 0x27, 0x8d,
    0x84, 0x68, 0x2b, 0x84, 0x04, 0x62, 0x42, 0x62, 0x64, 0x60, 0x62, 0x60,
    0xeb, 0x9f, 0xd0, 0x81, 0x09, 0x21, 0x31, 0x31, 0xb1, 0xb0, 0xb0, 0x21,
    0x56, 0x24, 0x40, 0x88, 0xb3, 0xfd, 0x42, 0x53, 0x84, 0xa5, 0x9f, 0xef,
    0xde, 0xdd, 0xbd, 0xbb, 0x7b, 0xb2, 0xdf, 0xe9, 0xbc, 0x33, 0x98, 0x9c,
    0xcf, 0xc0, 0xdd, 0x0f, 0x3f, 0xde, 0x7d, 0x23, 0xab, 0x3f, 0xb7, 0x9e,
    0x7e, 0x19, 0x1a, 0x39, 0xd8, 0xfd, 0xfd, 0xe2, 0xe3, 0x7e, 0xf5, 0x71,
    0xf9, 0xe3, 0xe9, 0x68, 0x1f, 0x80, 0x0d, 0x00, 0x9d, 0x72, 0x7a, 0x1c,
    0xc4, 0x63, 0xcb, 0x00, 0x53, 0x10, 0xda, 0xfb, 0x91, 0xf7, 0xc8, 0x31,
    0xe4, 0x2b, 0x42, 0x90, 0x15, 0x7c, 0x0d, 0xa0, 0xbc, 0x4a, 0xc2, 0xf5,
    0x73, 0x7c, 0xdd, 0x97, 0x00, 0xf6, 0x51, 0x6e, 0x52, 0x00, 0x07, 0x69,
    0x21, 0xcf, 0x90, 0xd7, 0xc8, 0x01, 0xf2, 0x09, 0xf9, 0x8e, 0x4c, 0x61,
    0xdc, 0x34, 0x32, 0x83, 0x9c, 0x41, 0xce, 0x22, 0x71, 0x44, 0x45, 0xe6,
    0x91, 0x3c, 0x72, 0x0d, 0xa9, 0x22, 0x75, 0x64, 0x17, 0x79, 0x89, 0xbc,
    0x91, 0xc2, 0x5a, 0x28, 0x00, 0xdb, 0x03, 0x6c, 0x1b, 0x22, 0xa2, 0x2f,
    0xbf, 0xc7, 0xe3, 0x42, 0x46, 0xc4, 0x19, 0x06, 0x84, 0xbe, 0x8c, 0x35,
    0x07, 0x85, 0x5e, 0x40, 0x7d, 0x48, 0xe8, 0x25, 0xd4, 0x87, 0x85, 0xbe,
    0xde, 0x63, 0xbf, 0x87, 0xfa, 0x88, 0xc8, 0xab, 0xd3, 0xb0, 0xce, 0x78,
    0x50, 0x53, 0x0a, 0xf2, 0xca, 0x68, 0x89, 0x06, 0xf5, 0xe5, 0x60, 0xed,
    0xfb, 0x47, 0x03, 0xd9, 0xb5, 0x47, 0x60, 0x2c, 0x90, 0x04, 0x26, 0x84,
    0x3c, 0x11, 0x48, 0x0a, 0x93, 0x22, 0x9e, 0x88, 0x33, 0xf8, 0x0f, 0x15,
    0xf2, 0xba, 0xdc, 0x5d, 0x77, 0x2d, 0xa1, 0x6f, 0x7c, 0x6c, 0x94, 0xc8,
    0x18, 0xed, 0xeb, 0xe7, 0x68, 0xd8, 0xa3, 0x1d, 0x03, 0xac, 0x55, 0x09,
    0x3a, 0x95, 0x82, 0x6c, 0x00, 0x29, 0xd1, 0xeb, 0x49, 0xa4, 0x12, 0x0b,
    0xcf, 0x10, 0xc1, 0x5d, 0x7e, 0xda, 0x05, 0x1a, 0xf6, 0x68, 0xc5, 0x08,
    0xac, 0xc3, 0xda, 0x0a, 0x46, 0xd1, 0x60, 0x07, 0xcd, 0xd2, 0x3e, 0xb0,
    0x57, 0xfa, 0x41, 0xbe, 0x35, 0x02, 0x49, 0x3f, 0xdf, 0x32, 0x2c, 0x42,
    0x1c, 0x2e, 0x11, 0x18, 0x39, 0xff, 0x44, 0x64, 0xee, 0xf6, 0x48, 0x83,
    0x35, 0x11, 0xeb, 0x7f, 0x75, 0x09, 0x22, 0xcb, 0x86, 0x69, 0x78, 0x17,
    0x80, 0x14, 0x61, 0xa2, 0xd8, 0xd2, 0xf8, 0x4d, 0x9b, 0x9b, 0x57, 0x4a,
    0x65, 0x83, 0x6f, 0xcf, 0x6e, 0xb2, 0x36, 0x83, 0x53, 0x25, 0x66, 0xea,
    0x8e, 0x65, 0xe8, 0xaa, 0x66, 0x99, 0x1e, 0x37, 0x3d, 0x75, 0xd5, 0x97,
    0x1d, 0x2f, 0xdf, 0xe3, 0xda, 0x70, 0x98, 0x5d, 0x37, 0x34, 0x57, 0x5d,
    0x65, 0x66, 0x9b, 0xb9, 0xff, 0x75, 0xdd, 0xe6, 0x1b, 0x86, 0x65, 0xe6,
    0x61, 0xfa, 0xaf, 0xab, 0x8d, 0x45, 0xd4, 0xb5, 0x96, 0x53, 0x63, 0x1a,
    0xf7, 0x0b, 0xe6, 0x61, 0xa6, 0xa4, 0x59, 0x4d, 0xd5, 0x69, 0xba, 0x0d,
    0x75, 0x13, 0x7b, 0x51, 0x8f, 0x36, 0x94, 0x07, 0x52, 0x06, 0x5a, 0x2e,
    0x82, 0x54, 0x2e, 0x96, 0x50, 0x29, 0xa1, 0x52, 0x2a, 0x02, 0xa9, 0x00,
    0xad, 0x94, 0x20, 0xa2, 0x31, 0x53, 0xe3, 0x0d, 0x18, 0xd6, 0x0d, 0xd7,
    0x66, 0x9e, 0x56, 0x2f, 0x38, 0x6c, 0x1b, 0xa6, 0x36, 0x98, 0x57, 0xe7,
    0xce, 0xba, 0xc3, 0x4c, 0xb4, 0x3a, 0xd8, 0x7f, 0xd8, 0x07, 0xc8, 0x75,
    0xcb, 0xf5, 0x60, 0xd2, 0x32, 0x2f, 0x7a, 0x18, 0xcb, 0xf5, 0x3b, 0x86,
    0xa9, 0x5b, 0xdb, 0x37, 0x98, 0x67, 0xb4, 0x39, 0x4c, 0x5b, 0x66, 0x81,
    0x7b, 0xcc, 0xb7, 0x5f, 0x76, 0xac, 0xe6, 0x11, 0xdf, 0x30, 0xfa, 0x30,
    0xb3, 0x58, 0xbd, 0x22, 0x3b, 0x3b, 0x85, 0xec, 0x03, 0xa5, 0xca, 0xb4,
    0x2d, 0x6e, 0xea, 0xca, 0x92, 0xa2, 0xf3, 0x8e, 0x12, 0x57, 0xf0, 0x1c,
    0xb6, 0xd1, 0xc0, 0x18, 0xcb, 0x4c, 0x34, 0x2d, 0x9d, 0xa3, 0xc3, 0xe1,
    0x0d, 0xce, 0x5c, 0x8e, 0xce, 0x3a, 0x73, 0x13, 0x98, 0x5b, 0xdb, 0x72,
    0x5b, 0x4d, 0x57, 0x59, 0xaa, 0xb1, 0x86, 0xcb, 0xe3, 0x4a, 0xd3, 0x30,
    0x13, 0xcc, 0x36, 0x94, 0xa5, 0xb9, 0x74, 0x5c, 0x71, 0xeb, 0x2c, 0x91,
    0xc2, 0x4d, 0x2c, 0xc3, 0xf4, 0x54, 0x96, 0x65, 0x92, 0xe9, 0xc5, 0x64,
    0x35, 0x93, 0xcb, 0xe9, 0xc9, 0x74, 0x76, 0x8e, 0xa7, 0x73, 0x19, 0x2d,
    0x95, 0xcc, 0xb1, 0xcc, 0x42, 0xb5, 0x96, 0xa9, 0xe5, 0x52, 0xba, 0x9f,
    0xb5, 0xcd, 0x1d, 0x17, 0xcb, 0xe1, 0xa6, 0xec, 0x6c, 0x2a, 0x39, 0x9b,
    0x4b, 0xe8, 0xbc, 0xad, 0x3c, 0xc4, 0xef, 0x2c, 0x4b, 0x40, 0xe9, 0xde,
    0x9e, 0xfc, 0x56, 0x92, 0x1f, 0xe1, 0xef, 0x20, 0x90, 0xc8, 0x67, 0x89,
    0xc8, 0xbf, 0x24, 0x42, 0xf6, 0xa5, 0xe0, 0x1e, 0x82, 0xf8, 0x27, 0xba,
    0xb2, 0x3b, 0x4b, 0x68, 0xcf, 0x3c, 0x91, 0x7a, 0x66, 0x8a, 0xdc, 0x33,
    0x57, 0xfa, 0xe0, 0x70, 0xb6, 0x44, 0xe0, 0x70, 0xbe, 0x90, 0x58, 0x18,
    0xe7, 0xcf, 0x18, 0x29, 0x16, 0xee, 0xf7, 0xef, 0x03, 0x89, 0x1e, 0xde,
    0x73, 0x1a, 0x0b, 0x6b, 0xf9, 0x33, 0x08, 0x62, 0xe1, 0xde, 0xe0, 0x5e,
    0x45, 0x43, 0xdd, 0x9f, 0x6b, 0x7f, 0x00, 0xa1, 0xe3, 0x13, 0x23, 0x10,
    0x05, 0x00, 0x00
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
        surfaceView = GlobalRef (LocalRef<jobject> (env->NewObject (JuceOpenGLViewSurface,
                                                                    JuceOpenGLViewSurface.constructor,
                                                                    getAppContext().get(),
                                                                    reinterpret_cast<jlong> (this))));
        if (surfaceView.get() == nullptr)
            return;

        surfaceHolderCallback = GlobalRef (CreateJavaInterface (this, "android/view/SurfaceHolder$Callback"));

        if (surfaceHolderCallback == nullptr)
            return;

        if (LocalRef<jobject> holder { env->CallObjectMethod (surfaceView, AndroidSurfaceView.getHolder) })
            env->CallVoidMethod (holder, AndroidSurfaceHolder.addCallback, surfaceHolderCallback.get());

        // add the view to the view hierarchy
        // after this the nativecontext can receive callbacks
        env->CallVoidMethod ((jobject) component.getPeer()->getNativeHandle(),
                             AndroidViewGroup.addView,
                             surfaceView.get());

        // initialise the geometry of the view
        updateWindowPosition (component.localAreaToGlobal (component.getLocalBounds()));
        hasInitialised = true;
    }

    ~NativeContext() override
    {
        auto env = getEnv();

        if (surfaceView != nullptr && surfaceHolderCallback != nullptr)
            if (LocalRef<jobject> holder { env->CallObjectMethod (surfaceView, AndroidSurfaceView.getHolder) })
                env->CallVoidMethod (holder, AndroidSurfaceHolder.removeCallback, surfaceHolderCallback.get());

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
    bool setSwapInterval (int)                  { return false; }
    int getSwapInterval() const                 { return 0; }

    //==============================================================================
    bool createdOk() const noexcept             { return hasInitialised; }
    void* getRawContext() const noexcept        { return surfaceView.get(); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    //==============================================================================
    void updateWindowPosition (Rectangle<int> bounds)
    {
        const auto physical = Desktop::getInstance().getDisplays().logicalToPhysical (bounds.toFloat()).toNearestInt();

        if (std::exchange (physicalBounds, physical) == physical)
            return;

        getEnv()->CallVoidMethod (surfaceView.get(),
                                  JuceOpenGLViewSurface.layout,
                                  (jint) physical.getX(),
                                  (jint) physical.getY(),
                                  (jint) physical.getRight(),
                                  (jint) physical.getBottom());
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

    void addListener (NativeContextListener& l)
    {
        listeners.add (&l);
    }

    void removeListener (NativeContextListener& l)
    {
        listeners.remove (&l);
    }

    void notifyWillPause()
    {
        listeners.call ([&] (auto& l) { l.contextWillPause(); });
    }

    void notifyDidResume()
    {
        listeners.call ([&] (auto& l) { l.contextDidResume(); });
    }

    Component& component;

private:
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
     METHOD (constructor, "<init>",    "(Landroid/content/Context;J)V") \
     METHOD (getParent,   "getParent", "()Landroid/view/ViewParent;") \
     METHOD (layout,      "layout",    "(IIII)V" ) \
     CALLBACK (generatedCallback<&NativeContext::dispatchDraw>,       "onDrawNative",               "(JLandroid/graphics/Canvas;)V")

     DECLARE_JNI_CLASS_WITH_BYTECODE (JuceOpenGLViewSurface, "com/rmsl/juce/JuceOpenGLView", 16, javaJuceOpenGLView)
    #undef JNI_CLASS_MEMBERS

    //==============================================================================
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

    static std::unique_ptr<ANativeWindow, NativeWindowReleaser> getNativeWindowFromSurfaceHolder (jobject holder)
    {
        auto* env = getEnv();

        if (holder == nullptr)
            return nullptr;

        const LocalRef<jobject> jSurface (env->CallObjectMethod (holder, AndroidSurfaceHolder.getSurface));

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
    Rectangle<int> physicalBounds;

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
    ListenerList<NativeContextListener> listeners;
    std::unique_ptr<std::remove_pointer_t<EGLSurface>, SurfaceDestructor> surface { EGL_NO_SURFACE };
    std::unique_ptr<std::remove_pointer_t<EGLContext>, ContextDestructor> context { EGL_NO_CONTEXT };

    GlobalRef surfaceHolderCallback;

    inline static EGLDisplay display = EGL_NO_DISPLAY;
    inline static EGLConfig config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return eglGetCurrentContext() != EGL_NO_CONTEXT;
}

} // namespace juce
