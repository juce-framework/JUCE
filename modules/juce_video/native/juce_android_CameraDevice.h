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

#if __ANDROID_API__ >= 21
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICMETHOD (valueOf, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$CompressFormat;")

DECLARE_JNI_CLASS (AndroidBitmapCompressFormat, "android/graphics/Bitmap$CompressFormat")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (close,                "close",                "()V") \
 METHOD (createCaptureRequest, "createCaptureRequest", "(I)Landroid/hardware/camera2/CaptureRequest$Builder;") \
 METHOD (createCaptureSession, "createCaptureSession", "(Ljava/util/List;Landroid/hardware/camera2/CameraCaptureSession$StateCallback;Landroid/os/Handler;)V")

DECLARE_JNI_CLASS (AndroidCameraDevice, "android/hardware/camera2/CameraDevice")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (close,     "close",     "()V") \
 METHOD (getPlanes, "getPlanes", "()[Landroid/media/Image$Plane;")

DECLARE_JNI_CLASS (AndroidImage, "android/media/Image")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getBuffer, "getBuffer", "()Ljava/nio/ByteBuffer;")

DECLARE_JNI_CLASS (AndroidImagePlane, "android/media/Image$Plane")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (acquireLatestImage,          "acquireLatestImage",          "()Landroid/media/Image;") \
 METHOD (close,                       "close",                       "()V") \
 METHOD (getSurface,                  "getSurface",                  "()Landroid/view/Surface;") \
 METHOD (setOnImageAvailableListener, "setOnImageAvailableListener", "(Landroid/media/ImageReader$OnImageAvailableListener;Landroid/os/Handler;)V") \
 STATICMETHOD (newInstance, "newInstance", "(IIII)Landroid/media/ImageReader;")

DECLARE_JNI_CLASS (AndroidImageReader, "android/media/ImageReader")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,             "<init>",                  "()V") \
 METHOD (getSurface,              "getSurface",              "()Landroid/view/Surface;") \
 METHOD (prepare,                 "prepare",                 "()V") \
 METHOD (release,                 "release",                 "()V") \
 METHOD (setAudioEncoder,         "setAudioEncoder",         "(I)V") \
 METHOD (setAudioSource,          "setAudioSource",          "(I)V") \
 METHOD (setOnErrorListener,      "setOnErrorListener",      "(Landroid/media/MediaRecorder$OnErrorListener;)V") \
 METHOD (setOnInfoListener,       "setOnInfoListener",       "(Landroid/media/MediaRecorder$OnInfoListener;)V") \
 METHOD (setOrientationHint,      "setOrientationHint",      "(I)V") \
 METHOD (setOutputFile,           "setOutputFile",           "(Ljava/lang/String;)V") \
 METHOD (setOutputFormat,         "setOutputFormat",         "(I)V") \
 METHOD (setVideoEncoder,         "setVideoEncoder",         "(I)V") \
 METHOD (setVideoEncodingBitRate, "setVideoEncodingBitRate", "(I)V") \
 METHOD (setVideoFrameRate,       "setVideoFrameRate",       "(I)V") \
 METHOD (setVideoSize,            "setVideoSize",            "(II)V") \
 METHOD (setVideoSource,          "setVideoSource",          "(I)V") \
 METHOD (start,                   "start",                   "()V") \
 METHOD (stop,                    "stop",                    "()V")

DECLARE_JNI_CLASS (AndroidMediaRecorder, "android/media/MediaRecorder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,               "<init>",                    "(Landroid/content/Context;)V") \
 METHOD (getSurfaceTexture,         "getSurfaceTexture",         "()Landroid/graphics/SurfaceTexture;") \
 METHOD (isAvailable,               "isAvailable",               "()Z") \
 METHOD (setSurfaceTextureListener, "setSurfaceTextureListener", "(Landroid/view/TextureView$SurfaceTextureListener;)V") \
 METHOD (setTransform,              "setTransform",              "(Landroid/graphics/Matrix;)V")

DECLARE_JNI_CLASS (AndroidTextureView, "android/view/TextureView")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(Landroid/graphics/SurfaceTexture;)V")

DECLARE_JNI_CLASS (AndroidSurface, "android/view/Surface")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (setDefaultBufferSize, "setDefaultBufferSize", "(II)V")

DECLARE_JNI_CLASS (AndroidSurfaceTexture, "android/graphics/SurfaceTexture")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getOutputSizesForClass,      "getOutputSizes",       "(Ljava/lang/Class;)[Landroid/util/Size;") \
 METHOD (getOutputSizesForFormat,     "getOutputSizes",       "(I)[Landroid/util/Size;") \
 METHOD (isOutputSupportedFor,        "isOutputSupportedFor", "(I)Z") \
 METHOD (isOutputSupportedForSurface, "isOutputSupportedFor", "(Landroid/view/Surface;)Z")

DECLARE_JNI_CLASS (AndroidStreamConfigurationMap, "android/hardware/camera2/params/StreamConfigurationMap")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>",      "()V") \
 METHOD (toByteArray, "toByteArray", "()[B") \
 METHOD (size,        "size",        "()I")

DECLARE_JNI_CLASS (ByteArrayOutputStream, "java/io/ByteArrayOutputStream")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (abortCaptures,       "abortCaptures",       "()V") \
 METHOD (capture,             "capture",             "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I") \
 METHOD (close,               "close",               "()V") \
 METHOD (setRepeatingRequest, "setRepeatingRequest", "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I") \
 METHOD (stopRepeating,       "stopRepeating",       "()V")

DECLARE_JNI_CLASS (CameraCaptureSession, "android/hardware/camera2/CameraCaptureSession")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";JZ)V")

DECLARE_JNI_CLASS (CameraCaptureSessionCaptureCallback, JUCE_ANDROID_ACTIVITY_CLASSPATH "$CameraCaptureSessionCaptureCallback")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V")

DECLARE_JNI_CLASS (CameraCaptureSessionStateCallback, JUCE_ANDROID_ACTIVITY_CLASSPATH "$CameraCaptureSessionStateCallback")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (get,     "get",     "(Landroid/hardware/camera2/CameraCharacteristics$Key;)Ljava/lang/Object;") \
 METHOD (getKeys, "getKeys", "()Ljava/util/List;") \
 STATICFIELD (CONTROL_AF_AVAILABLE_MODES,      "CONTROL_AF_AVAILABLE_MODES",      "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (LENS_FACING,                     "LENS_FACING",                     "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (SCALER_STREAM_CONFIGURATION_MAP, "SCALER_STREAM_CONFIGURATION_MAP", "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (SENSOR_ORIENTATION,              "SENSOR_ORIENTATION",              "Landroid/hardware/camera2/CameraCharacteristics$Key;")

DECLARE_JNI_CLASS (CameraCharacteristics, "android/hardware/camera2/CameraCharacteristics")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getName, "getName", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (CameraCharacteristicsKey, "android/hardware/camera2/CameraCharacteristics$Key")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V")

DECLARE_JNI_CLASS (CameraDeviceStateCallback, JUCE_ANDROID_ACTIVITY_CLASSPATH "$CameraDeviceStateCallback")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getCameraCharacteristics, "getCameraCharacteristics", "(Ljava/lang/String;)Landroid/hardware/camera2/CameraCharacteristics;") \
 METHOD (getCameraIdList,          "getCameraIdList",          "()[Ljava/lang/String;") \
 METHOD (openCamera,               "openCamera",               "(Ljava/lang/String;Landroid/hardware/camera2/CameraDevice$StateCallback;Landroid/os/Handler;)V")

DECLARE_JNI_CLASS (CameraManager, "android/hardware/camera2/CameraManager")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 STATICFIELD (CONTROL_AE_PRECAPTURE_TRIGGER, "CONTROL_AE_PRECAPTURE_TRIGGER", "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_AF_MODE,               "CONTROL_AF_MODE",               "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_AF_TRIGGER,            "CONTROL_AF_TRIGGER",            "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_MODE,                  "CONTROL_MODE",                  "Landroid/hardware/camera2/CaptureRequest$Key;")

DECLARE_JNI_CLASS (CaptureRequest, "android/hardware/camera2/CaptureRequest")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (addTarget, "addTarget", "(Landroid/view/Surface;)V") \
 METHOD (build,     "build",     "()Landroid/hardware/camera2/CaptureRequest;") \
 METHOD (set,       "set",       "(Landroid/hardware/camera2/CaptureRequest$Key;Ljava/lang/Object;)V")

DECLARE_JNI_CLASS (CaptureRequestBuilder, "android/hardware/camera2/CaptureRequest$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (get, "get", "(Landroid/hardware/camera2/CaptureResult$Key;)Ljava/lang/Object;") \
 STATICFIELD (CONTROL_AE_STATE, "CONTROL_AE_STATE", "Landroid/hardware/camera2/CaptureResult$Key;") \
 STATICFIELD (CONTROL_AF_STATE, "CONTROL_AF_STATE", "Landroid/hardware/camera2/CaptureResult$Key;")

DECLARE_JNI_CLASS (CaptureResult, "android/hardware/camera2/CaptureResult")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (canDetectOrientation, "canDetectOrientation", "()Z") \
 METHOD (constructor,          "<init>",               "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";JLandroid/content/Context;I)V") \
 METHOD (disable,              "disable",              "()V") \
 METHOD (enable,               "enable",               "()V")

DECLARE_JNI_CLASS (OrientationEventListener, JUCE_ANDROID_ACTIVITY_CLASSPATH "$JuceOrientationEventListener")
#undef JNI_CLASS_MEMBERS
#endif

//==============================================================================
class AndroidRunnable  : public juce::AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void run() = 0;
    };

    AndroidRunnable (Owner& ownerToUse)
        : owner (ownerToUse)
    {}

private:
    Owner& owner;

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();
        auto methodName = juce::juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        if (methodName == "run")
        {
            owner.run();
            return nullptr;
        }

        // invoke base class
        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }
};

//==============================================================================
class TextureViewSurfaceTextureListener     : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onSurfaceTextureAvailable (LocalRef<jobject>& surface, int width, int height) = 0;
        virtual bool onSurfaceTextureDestroyed (LocalRef<jobject>& surface) = 0;
        virtual void onSurfaceTextureSizeChanged (LocalRef<jobject>& surface, int width, int height) = 0;
        virtual void onSurfaceTextureUpdated (LocalRef<jobject>& surface) = 0;
    };

    TextureViewSurfaceTextureListener (Owner& ownerToUse)
        : owner (ownerToUse)
    {}

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();

        auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onSurfaceTextureAvailable" && numArgs == 3)
        {
            auto surface = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto width   = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto height  = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto widthInt  = env->CallIntMethod (width, JavaInteger.intValue);
            auto heightInt = env->CallIntMethod (height, JavaInteger.intValue);

            owner.onSurfaceTextureAvailable (surface, widthInt, heightInt);
            return nullptr;
        }
        else if (methodName == "onSurfaceTextureDestroyed" && numArgs == 1)
        {
            auto surface = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto result = owner.onSurfaceTextureDestroyed (surface);

            return env->CallStaticObjectMethod (JavaBoolean, JavaBoolean.valueOf, result);
        }
        else if (methodName == "onSurfaceTextureSizeChanged" && numArgs == 3)
        {
            auto surface = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto width   = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto height  = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto widthInt  = env->CallIntMethod (width, JavaInteger.intValue);
            auto heightInt = env->CallIntMethod (height, JavaInteger.intValue);

            owner.onSurfaceTextureSizeChanged (surface, widthInt, heightInt);
            return nullptr;
        }
        else if (methodName == "onSurfaceTextureUpdated" && numArgs == 1)
        {
            auto surface = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            owner.onSurfaceTextureUpdated (surface);
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }

private:
    Owner& owner;
};

//==============================================================================
class ImageReaderOnImageAvailableListener     : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onImageAvailable (LocalRef<jobject>& imageReader) = 0;
    };

    ImageReaderOnImageAvailableListener (Owner& ownerToUse)
        : owner (ownerToUse)
    {}

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();

        auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onImageAvailable" && numArgs == 1)
        {
            auto imageReader = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            owner.onImageAvailable (imageReader);
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }

private:
    Owner& owner;
};

//==============================================================================
class MediaRecorderOnInfoListener     : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onInfo (LocalRef<jobject>& mediaRecorder, int what, int extra) = 0;
    };

    MediaRecorderOnInfoListener (Owner& ownerToUse)
        : owner (ownerToUse)
    {}

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();

        auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onInfo" && numArgs == 3)
        {
            auto mediaRecorder = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto what   = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto extra  = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto whatInt  = (int) env->CallIntMethod (what, JavaInteger.intValue);
            auto extraInt = (int) env->CallIntMethod (extra, JavaInteger.intValue);

            owner.onInfo (mediaRecorder, whatInt, extraInt);
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }

private:
    Owner& owner;
};

//==============================================================================
class MediaRecorderOnErrorListener     : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onError (LocalRef<jobject>& mediaRecorder, int what, int extra) = 0;
    };

    MediaRecorderOnErrorListener (Owner& ownerToUse)
        : owner (ownerToUse)
    {}

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();

        auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onError" && numArgs == 3)
        {
            auto mediaRecorder = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto what   = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto extra  = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto whatInt  = (int) env->CallIntMethod (what, JavaInteger.intValue);
            auto extraInt = (int) env->CallIntMethod (extra, JavaInteger.intValue);

            owner.onError (mediaRecorder, whatInt, extraInt);
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }

private:
    Owner& owner;
};

//==============================================================================
struct CameraDevice::Pimpl
#if __ANDROID_API__ >= 21
    : private AppPausedResumedListener::Owner
#endif
{
    using InternalOpenCameraResultCallback = std::function<void (const String& /*cameraId*/, const String& /*error*/)>;

    Pimpl (CameraDevice& ownerToUse, const String& cameraIdToUse, int /*index*/,
           int minWidthToUse, int minHeightToUse, int maxWidthToUse, int maxHeightToUse,
           bool /*useHighQuality*/)
         #if __ANDROID_API__ >= 21
        : owner (ownerToUse),
          minWidth (minWidthToUse),
          minHeight (minHeightToUse),
          maxWidth (maxWidthToUse),
          maxHeight (maxHeightToUse),
          cameraId (cameraIdToUse),
          appPausedResumedListener (*this),
          appPausedResumedListenerNative (CreateJavaInterface (&appPausedResumedListener,
                                                               JUCE_ANDROID_ACTIVITY_CLASSPATH "$AppPausedResumedListener").get()),
          cameraManager (initialiseCameraManager()),
          cameraCharacteristics (initialiseCameraCharacteristics (cameraManager, cameraId)),
          streamConfigurationMap (cameraCharacteristics),
          previewDisplay (streamConfigurationMap.getPreviewBufferSize()),
          deviceOrientationChangeListener (previewDisplay)
         #endif
    {
       #if __ANDROID_API__ >= 21
        startBackgroundThread();
       #endif
    }

    ~Pimpl()
    {
       #if __ANDROID_API__ >= 21
        getEnv()->CallVoidMethod (android.activity, JuceAppActivity.removeAppPausedResumedListener,
                                  appPausedResumedListenerNative.get(), reinterpret_cast<jlong>(this));
       #endif
    }

   #if __ANDROID_API__ < 21
    // Dummy implementations for unsupported API levels.
    void open (InternalOpenCameraResultCallback) {}
    void takeStillPicture (std::function<void (const Image&)>) {}
    void startRecordingToFile (const File&, int) {}
    void stopRecording() {}

    void addListener (CameraDevice::Listener*) {}
    void removeListener (CameraDevice::Listener*) {}

    String getCameraId() const noexcept      { return {}; }
    bool openedOk() const noexcept           { return false; }
    Time getTimeOfFirstRecordedFrame() const { return {}; }
    static StringArray getAvailableDevices()
    {
        // Camera on Android requires API 21 or above.
        jassertfalse;
        return {};
    }
   #else
    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)

    String getCameraId() const noexcept { return cameraId; }

    void open (InternalOpenCameraResultCallback cameraOpenCallbackToUse)
    {
        cameraOpenCallback = static_cast<InternalOpenCameraResultCallback&&> (cameraOpenCallbackToUse);

        // A valid camera open callback must be passed.
        jassert (cameraOpenCallback != nullptr);

        // The same camera can be opened only once!
        jassert (scopedCameraDevice == nullptr);

        if (cameraOpenCallback == nullptr || scopedCameraDevice != nullptr)
            return;

        WeakReference<Pimpl> safeThis (this);
        RuntimePermissions::request (RuntimePermissions::camera, [safeThis] (bool granted) mutable
                                     {
                                         if (safeThis != nullptr)
                                             safeThis->continueOpenRequest (granted);
                                     });
    }

    void continueOpenRequest (bool granted)
    {
        if (granted)
        {
            getEnv()->CallVoidMethod (android.activity, JuceAppActivity.addAppPausedResumedListener,
                                      appPausedResumedListenerNative.get(), reinterpret_cast<jlong> (this));
            scopedCameraDevice.reset (new ScopedCameraDevice (*this, cameraId, cameraManager, handler, getAutoFocusModeToUse()));
        }
        else
        {
            invokeCameraOpenCallback ("Camera permission not granted");
        }
    }

    bool openedOk() const noexcept { return scopedCameraDevice->openedOk(); }

    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr)
        {
            jassertfalse;
            return;
        }

        if (currentCaptureSessionMode->isVideoRecordSession())
        {
            // Taking still pictures while recording video is not supported on Android.
            jassertfalse;
            return;
        }

        pictureTakenCallback = static_cast<std::function<void (const Image&)>&&> (pictureTakenCallbackToUse);

        triggerStillPictureCapture();
    }

    void startRecordingToFile (const File& file, int /*quality*/)
    {
        if (! openedOk())
        {
            jassertfalse;
            return;
        }

        if (! previewDisplay.isReady())
        {
            // Did you remember to create and show a preview display?
            jassertfalse;
            return;
        }

        file.deleteFile();
        file.create();
        jassert (file.existsAsFile());

        // MediaRecorder can't handle videos larger than 1080p
        auto videoSize = chooseBestSize (minWidth, minHeight, jmin (maxWidth, 1080), maxHeight,
                                         streamConfigurationMap.getSupportedVideoRecordingOutputSizes());

        mediaRecorder.reset (new MediaRecorder (file.getFullPathName(), videoSize.getWidth(), videoSize.getHeight(),
                                                getCameraSensorOrientation(), getCameraLensFacing()));

        firstRecordedFrameTimeMs = Time::getCurrentTime();

        currentCaptureSessionMode.reset();
        startVideoRecordingMode (*mediaRecorder);
    }

    void stopRecording()
    {
        currentCaptureSessionMode.reset();
        mediaRecorder.reset();

        startPreviewMode (*imageReader);
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstRecordedFrameTimeMs;
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        auto* env = getEnv();

        auto cameraManagerToUse = initialiseCameraManager();
        auto cameraIdArray = LocalRef<jobjectArray> ((jobjectArray) env->CallObjectMethod (cameraManagerToUse,
                                                                                           CameraManager.getCameraIdList));

        results = javaStringArrayToJuce (cameraIdArray);

        for (auto& result : results)
            printDebugCameraInfo (cameraManagerToUse, result);

        return results;
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.add (listenerToAdd);

        if (listeners.size() == 1)
            triggerStillPictureCapture();
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);
    }

private:
    enum
    {
        ERROR_CAMERA_IN_USE = 1,
        ERROR_MAX_CAMERAS_IN_USE = 2,
        ERROR_CAMERA_DISABLED = 3,
        ERROR_CAMERA_DEVICE = 4,
        ERROR_CAMERA_SERVICE = 5
    };

    static String cameraErrorCodeToString (int errorCode)
    {
        switch (errorCode)
        {
            case ERROR_CAMERA_IN_USE:      return "Camera already in use.";
            case ERROR_MAX_CAMERAS_IN_USE: return "Too many opened camera devices.";
            case ERROR_CAMERA_DISABLED:    return "Camera disabled.";
            case ERROR_CAMERA_DEVICE:      return "Fatal error.";
            case ERROR_CAMERA_SERVICE:     return "Fatal error. Reboot required or persistent hardware problem.";
            default:                       return "Unknown error.";
        }
    }

    static LocalRef<jobject> initialiseCameraManager()
    {
        return LocalRef<jobject> (getEnv()->CallObjectMethod (android.activity, JuceAppActivity.getSystemService,
                                                              javaString ("camera").get()));
    }

    static LocalRef<jobject> initialiseCameraCharacteristics (const GlobalRef& cameraManager, const String& cameraId)
    {
        return LocalRef<jobject> (getEnv()->CallObjectMethod (cameraManager,
                                                              CameraManager.getCameraCharacteristics,
                                                              javaString (cameraId).get()));
    }

    static void printDebugCameraInfo (const LocalRef<jobject>& cameraManagerToUse, const String& cameraId)
    {
        auto* env = getEnv();

        auto characteristics = LocalRef<jobject> (env->CallObjectMethod (cameraManagerToUse,
                                                                         CameraManager.getCameraCharacteristics,
                                                                         javaString (cameraId).get()));

        auto keysList = LocalRef<jobject> (env->CallObjectMethod (characteristics, CameraCharacteristics.getKeys));

        const int size = env->CallIntMethod (keysList, JavaList.size);

        JUCE_CAMERA_LOG ("Camera id: " + cameraId + ", characteristics keys num: " + String (size));

        for (int ikey = 0; ikey < size; ++ikey)
        {
            auto key = LocalRef<jobject> (env->CallObjectMethod (keysList, JavaList.get, ikey));
            auto jKeyName = LocalRef<jstring> ((jstring) env->CallObjectMethod (key, CameraCharacteristicsKey.getName));
            auto keyName = juceString (jKeyName);

            auto keyValue = LocalRef<jobject> (env->CallObjectMethod (characteristics, CameraCharacteristics.get, key.get()));
            auto jKeyValueString = LocalRef<jstring> ((jstring) env->CallObjectMethod (keyValue, JavaObject.toString));
            auto keyValueString = juceString (jKeyValueString);

            auto &kvs = keyValueString;

            if (kvs.startsWith ("[I") || kvs.startsWith ("[F") || kvs.startsWith ("[Z") || kvs.startsWith ("[B"))
            {
                printPrimitiveArrayElements (keyValue, keyName, keyValueString);
            }
            else if (kvs.startsWith ("[Landroid.util.Range"))
            {
                printRangeArrayElements (keyValue);
            }
            else
            {
                int chunkSize = 256;

                if (keyValueString.length() > chunkSize)
                {
                    JUCE_CAMERA_LOG ("Key: " + keyName);

                    for (int i = 0, j = 1; i < keyValueString.length(); i += chunkSize, ++j)
                        JUCE_CAMERA_LOG ("value part " + String (j) + ": " + keyValueString.substring (i, i + chunkSize));
                }
                else
                {
                    JUCE_CAMERA_LOG ("Key: " + keyName + ", value: " + keyValueString);
                }
            }

            ignoreUnused (keyName);
        }
    }

    static void printPrimitiveArrayElements (const LocalRef<jobject>& keyValue, const String& keyName,
                                             const String& keyValueString)
    {
        ignoreUnused (keyName);

        String result = "[";

        auto* env = getEnv();

        #define PRINT_ELEMENTS(elem_type, array_type, fun_name_middle)                                        \
        {                                                                                                     \
            elem_type* elements = env->Get##fun_name_middle##ArrayElements ((array_type) keyValue.get(), 0);  \
            int size = env->GetArrayLength ((array_type) keyValue.get());                                     \
                                                                                                              \
            for (int i = 0; i < size - 1; ++i)                                                                \
                result << String (elements[i]) << " ";                                                        \
                                                                                                              \
            if (size > 0)                                                                                     \
                result << String (elements[size - 1]);                                                        \
                                                                                                              \
            env->Release##fun_name_middle##ArrayElements ((array_type) keyValue.get(), elements, 0);          \
        }

        if (keyValueString.startsWith ("[I"))
            PRINT_ELEMENTS (jint, jintArray, Int)
        else if (keyValueString.startsWith ("[F"))
            PRINT_ELEMENTS (float, jfloatArray, Float)
        else if (keyValueString.startsWith ("[Z"))
            PRINT_ELEMENTS (jboolean, jbooleanArray, Boolean)
        else if (keyValueString.startsWith ("[B"))
            PRINT_ELEMENTS (jbyte, jbyteArray, Byte);

        #undef PRINT_ELEMENTS

        result << "]";
        JUCE_CAMERA_LOG ("Key: " + keyName + ", value: " + result);
    }

    static void printRangeArrayElements (const LocalRef<jobject>& rangeArray)
    {
        auto* env = getEnv();

        jobjectArray ranges = static_cast<jobjectArray> (rangeArray.get());

        int numRanges = env->GetArrayLength (ranges);

        String result;

        for (int i = 0; i < numRanges; ++i)
        {
            auto range = LocalRef<jobject> (env->GetObjectArrayElement (ranges, i));

            auto jRangeString = LocalRef<jstring> ((jstring) env->CallObjectMethod (range, AndroidRange.toString));

            result << juceString (jRangeString) << " ";
        }

        JUCE_CAMERA_LOG ("Key: " + keyName + ", value: " + result);
    }

    //==============================================================================
    class StreamConfigurationMap
    {
    public:
        StreamConfigurationMap (const GlobalRef& characteristics)
            : scalerStreamConfigurationMap (getStreamConfigurationMap (characteristics)),
              supportedPreviewOutputSizes (retrieveOutputSizes (scalerStreamConfigurationMap,
                                                                getClassForName ("android.graphics.SurfaceTexture"),
                                                                -1)),
              supportedStillImageOutputSizes (retrieveOutputSizes (scalerStreamConfigurationMap,
                                                                   LocalRef<jobject>(),
                                                                   jpegImageFormat)),
              supportedVideoRecordingOutputSizes (retrieveOutputSizes (scalerStreamConfigurationMap,
                                                                       getClassForName ("android.media.MediaRecorder"),
                                                                       -1)),
              defaultPreviewSize (getSmallestSize (supportedPreviewOutputSizes)),
              previewBufferSize (getLargestSize (supportedPreviewOutputSizes))
        {
            printSizesLog (supportedPreviewOutputSizes, "SurfaceTexture");
            printSizesLog (supportedStillImageOutputSizes, "JPEG");
            printSizesLog (supportedVideoRecordingOutputSizes, "MediaRecorder");
        }

        Array<Rectangle<int>> getSupportedPreviewOutputSizes()        const noexcept { return supportedPreviewOutputSizes; }
        Array<Rectangle<int>> getSupportedStillImageOutputSizes()     const noexcept { return supportedStillImageOutputSizes; }
        Array<Rectangle<int>> getSupportedVideoRecordingOutputSizes() const noexcept { return supportedVideoRecordingOutputSizes; }

        Rectangle<int> getDefaultPreviewSize() const noexcept { return defaultPreviewSize; }
        Rectangle<int> getPreviewBufferSize()  const noexcept { return previewBufferSize; }

        bool isOutputSupportedForSurface (const LocalRef<jobject>& surface) const
        {
            return getEnv()->CallBooleanMethod (scalerStreamConfigurationMap, AndroidStreamConfigurationMap.isOutputSupportedForSurface, surface.get()) != 0;
        }

        static constexpr int jpegImageFormat = 256;

    private:
        GlobalRef scalerStreamConfigurationMap;

        Array<Rectangle<int>> supportedPreviewOutputSizes;
        Array<Rectangle<int>> supportedStillImageOutputSizes;
        Array<Rectangle<int>> supportedVideoRecordingOutputSizes;
        Rectangle<int> defaultPreviewSize, previewBufferSize;

        GlobalRef getStreamConfigurationMap (const GlobalRef& characteristics)
        {
            auto* env = getEnv();

            auto scalerStreamConfigurationMapKey = LocalRef<jobject> (env->GetStaticObjectField (CameraCharacteristics,
                                                                                                 CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP));

            return GlobalRef (LocalRef<jobject> (env->CallObjectMethod (characteristics,
                                                                        CameraCharacteristics.get,
                                                                        scalerStreamConfigurationMapKey.get())));
        }

        static Array<Rectangle<int>> retrieveOutputSizes (GlobalRef& scalerStreamConfigurationMap,
                                                          const LocalRef<jobject>& outputClass,
                                                          int format)
        {
            Array<Rectangle<int>> result;

            auto* env = getEnv();

            auto outputSizes = outputClass.get() != nullptr
                             ? LocalRef<jobjectArray> ((jobjectArray) env->CallObjectMethod (scalerStreamConfigurationMap,
                                                                                             AndroidStreamConfigurationMap.getOutputSizesForClass,
                                                                                             outputClass.get()))
                             : LocalRef<jobjectArray> ((jobjectArray) env->CallObjectMethod (scalerStreamConfigurationMap,
                                                                                             AndroidStreamConfigurationMap.getOutputSizesForFormat,
                                                                                             (jint) format));

            if (format != -1)
            {
                auto supported = (env->CallBooleanMethod (scalerStreamConfigurationMap, AndroidStreamConfigurationMap.isOutputSupportedFor, (jint) format) != 0);

                if (! supported)
                {
                    // The output format is not supported by this device, still image capture will not work!
                    jassertfalse;
                    return {};
                }
            }

            int numSizes = env->GetArrayLength (outputSizes);

            jassert (numSizes > 0);

            for (int i = 0; i < numSizes; ++i)
            {
                auto size = LocalRef<jobject> (env->GetObjectArrayElement (outputSizes, i));

                auto width  = env->CallIntMethod (size, AndroidSize.getWidth);
                auto height = env->CallIntMethod (size, AndroidSize.getHeight);

                result.add (Rectangle<int> (0, 0, width, height));
            }

            return result;
        }

        static LocalRef<jobject> getClassForName (const String& name)
        {
            return LocalRef<jobject> (getEnv()->CallStaticObjectMethod (JavaClass, JavaClass.forName,
                                                                        javaString (name).get()));
        }

        static void printSizesLog (const Array<Rectangle<int>>& sizes, const String& className)
        {
            ignoreUnused (sizes, className);

            JUCE_CAMERA_LOG ("Sizes for class " + className);

          #if JUCE_CAMERA_LOG_ENABLED
            for (auto& s : sizes)
                JUCE_CAMERA_LOG (s.toString() + "\n");
          #endif
        }

        Rectangle<int> getSmallestSize (const Array<Rectangle<int>>& sizes) const
        {
            if (sizes.size() == 0)
                return {};

            auto smallestSize = sizes[0];

            for (auto& size : sizes)
            {
                if (size.getWidth() < smallestSize.getWidth() && size.getHeight() < smallestSize.getHeight())
                    smallestSize = size;
            }

            return smallestSize;
        }

        Rectangle<int> getLargestSize (const Array<Rectangle<int>>& sizes) const
        {
            if (sizes.size() == 0)
                return {};

            auto largestSize = sizes[0];

            for (auto& size : sizes)
            {
                if (size.getWidth() > largestSize.getWidth() && size.getHeight() > largestSize.getHeight())
                    largestSize = size;
            }

            return largestSize;
        }
    };

    //==============================================================================
    class PreviewDisplay    : private TextureViewSurfaceTextureListener::Owner
    {
    public:
        struct Listener
        {
            virtual ~Listener() {}

            virtual void previewDisplayReady() = 0;
            virtual void previewDisplayAboutToBeDestroyed() = 0;
        };

        PreviewDisplay (Rectangle<int> bufferSize)
            : textureViewSurfaceTextureListener (*this),
              textureView (getEnv()->NewObject (AndroidTextureView, AndroidTextureView.constructor,
                                                android.activity.get())),
              bufferWidth (bufferSize.getWidth()),
              bufferHeight (bufferSize.getHeight())
        {
            auto* env = getEnv();

            if (! isReady())
                env->CallVoidMethod (textureView, AndroidTextureView.setSurfaceTextureListener,
                                     CreateJavaInterface (&textureViewSurfaceTextureListener,
                                                          "android/view/TextureView$SurfaceTextureListener").get());
        }

        ~PreviewDisplay()
        {
            getEnv()->CallVoidMethod (textureView, AndroidTextureView.setSurfaceTextureListener, nullptr);
        }

        void addListener (Listener* l)
        {
            if (l == nullptr)
            {
                jassertfalse;
                return;
            }

            listeners.add (l);

            if (isReady())
                l->previewDisplayReady();
        }

        void removeListener (Listener* l)
        {
            if (l == nullptr)
            {
                jassertfalse;
                return;
            }

            listeners.remove (l);
        }

        bool isReady() const
        {
            return (getEnv()->CallBooleanMethod (textureView, AndroidTextureView.isAvailable) != 0)
                    && width > 0 && height > 0;
        }

        LocalRef<jobject> createSurface()
        {
            // Surface may get destroyed while session is being configured, if
            // the preview gets hidden in the meantime, so bailout.
            if (! isReady())
                return LocalRef<jobject> (nullptr);

            auto* env = getEnv();

            auto surfaceTexture = LocalRef<jobject> (env->CallObjectMethod (textureView,
                                                                            AndroidTextureView.getSurfaceTexture));

            // NB: too small buffer will result in pixelated preview. A buffer with wrong aspect ratio
            //     can result in a cropped preview.
            env->CallVoidMethod (surfaceTexture, AndroidSurfaceTexture.setDefaultBufferSize, (jint) bufferWidth, (jint) bufferHeight);

            auto surface = LocalRef<jobject> (env->NewObject (AndroidSurface, AndroidSurface.constructor, surfaceTexture.get()));

            return surface;
        }

        const GlobalRef& getNativeView() { return textureView; }

        void updateSurfaceTransform()
        {
            auto* env = getEnv();

            auto windowManager = LocalRef<jobject> (env->CallObjectMethod (android.activity, JuceAppActivity.getWindowManager));
            auto display = LocalRef<jobject> (env->CallObjectMethod (windowManager, AndroidWindowManager.getDefaultDisplay));
            auto rotation = env->CallIntMethod (display, AndroidDisplay.getRotation);

            static constexpr int rotation90 = 1;
            static constexpr int rotation270 = 3;

            auto matrix = LocalRef<jobject> (env->NewObject (AndroidMatrix, AndroidMatrix.constructor));

            if (rotation == rotation90 || rotation == rotation270)
            {
                env->CallBooleanMethod (matrix, AndroidMatrix.postScale, jfloat (height / (float) width), jfloat (width / (float) height), (jfloat) 0, (jfloat) 0);
                env->CallBooleanMethod (matrix, AndroidMatrix.postRotate, (jfloat) 90 * (rotation - 2), (jfloat) 0, (jfloat) 0);
                env->CallBooleanMethod (matrix, AndroidMatrix.postTranslate, (jfloat) (rotation == 3 ? width : 0), (jfloat) (rotation == 1 ? height : 0));
            }

            env->CallVoidMethod (textureView, AndroidTextureView.setTransform, matrix.get());
        }

    private:
        ListenerList<Listener> listeners;

        TextureViewSurfaceTextureListener textureViewSurfaceTextureListener;
        GlobalRef textureView;
        int width = -1, height = -1;
        int bufferWidth, bufferHeight;

        void onSurfaceTextureAvailable (LocalRef<jobject>& /*surface*/, int widthToUse, int heightToUse) override
        {
            JUCE_CAMERA_LOG ("onSurfaceTextureAvailable()");

            width = widthToUse;
            height = heightToUse;

            updateSurfaceTransform();

            listeners.call (&Listener::previewDisplayReady);
        }

        bool onSurfaceTextureDestroyed (LocalRef<jobject>& /*surface*/) override
        {
            JUCE_CAMERA_LOG ("onSurfaceTextureDestroyed()");

            listeners.call (&Listener::previewDisplayAboutToBeDestroyed);

            return true;
        }

        void onSurfaceTextureSizeChanged (LocalRef<jobject>& /*surface*/, int widthToUse, int heightToUse) override
        {
            JUCE_CAMERA_LOG ("onSurfaceTextureSizeChanged()");

            width = widthToUse;
            height = heightToUse;

            updateSurfaceTransform();
        }

        void onSurfaceTextureUpdated (LocalRef<jobject>& /*surface*/) override
        {
            JUCE_CAMERA_LOG ("onSurfaceTextureUpdated()");
        }

        JUCE_DECLARE_NON_COPYABLE (PreviewDisplay)
    };

    //==============================================================================
    class ImageReader   : private ImageReaderOnImageAvailableListener::Owner
    {
    public:
        ImageReader (Pimpl& ownerToUse, GlobalRef& handlerToUse,
                     int imageWidth, int imageHeight, int cameraSensorOrientationToUse)
            : owner (ownerToUse),
              cameraSensorOrientation (cameraSensorOrientationToUse),
              imageReader (getEnv()->CallStaticObjectMethod (AndroidImageReader, AndroidImageReader.newInstance,
                                                             imageWidth, imageHeight, StreamConfigurationMap::jpegImageFormat,
                                                             numImagesToKeep)),
              onImageAvailableListener (*this)
        {
            getEnv()->CallVoidMethod (imageReader, AndroidImageReader.setOnImageAvailableListener,
                                      CreateJavaInterface (&onImageAvailableListener,
                                                           "android/media/ImageReader$OnImageAvailableListener").get(),
                                      handlerToUse.get());
        }

        ~ImageReader()
        {
            getEnv()->CallVoidMethod (imageReader, AndroidImageReader.close);
        }

        LocalRef<jobject> getSurface() const
        {
            return LocalRef<jobject> (getEnv()->CallObjectMethod (imageReader, AndroidImageReader.getSurface));
        }

        void resetNotificationFlag()
        {
            hasNotifiedListeners.set (0);
        }

    private:
        Pimpl& owner;
        int cameraSensorOrientation;

        GlobalRef imageReader;
        ImageReaderOnImageAvailableListener onImageAvailableListener;
        static constexpr int numImagesToKeep = 2;
        Atomic<int> hasNotifiedListeners { 0 };

        JUCE_DECLARE_WEAK_REFERENCEABLE (ImageReader)

        void onImageAvailable (LocalRef<jobject>& /*imageReader*/) override
        {
            JUCE_CAMERA_LOG ("onImageAvailable()");

            auto* env = getEnv();

            auto jImage = LocalRef<jobject> (env->CallObjectMethod (imageReader, AndroidImageReader.acquireLatestImage));

            if (jImage.get() == nullptr)
                return;

            auto cameraLensFrontFacing = owner.getCameraLensFacing() == 0;

            // NB: could use sensor orientation here to get real-world orientation, but then the resulting
            //     image could not match the UI orientation.
            auto image = androidImageToJuceWithFixedOrientation (jImage, owner.deviceOrientationChangeListener.getDeviceOrientation(),
                                                                 Desktop::getInstance().getCurrentOrientation(),
                                                                 cameraLensFrontFacing,
                                                                 cameraSensorOrientation);

            env->CallVoidMethod (jImage, AndroidImage.close);

            WeakReference<ImageReader> safeThis (this);

            owner.callListeners (image);

            // Android may take multiple pictures before it handles a request to stop.
            if (hasNotifiedListeners.compareAndSetBool (1, 0))
                MessageManager::callAsync ([safeThis, image]() mutable { if (safeThis != nullptr) safeThis->owner.notifyPictureTaken (image); });
        }

        struct ImageBuffer
        {
            LocalRef<jbyteArray> byteArray;
            int size;
        };

        static Image androidImageToJuceWithFixedOrientation (const LocalRef<jobject>& androidImage,
                                                             Desktop::DisplayOrientation deviceOrientationFromAccelerometerSensor,
                                                             Desktop::DisplayOrientation targetOrientation,
                                                             bool cameraLensFrontFacing,
                                                             int cameraSensorOrientation)
        {
            auto* env = getEnv();

            auto planes = LocalRef<jobjectArray> ((jobjectArray) env->CallObjectMethod (androidImage, AndroidImage.getPlanes));
            jassert (env->GetArrayLength (planes) > 0);

            auto plane = LocalRef<jobject> (env->GetObjectArrayElement (planes, 0));
            auto byteBuffer = LocalRef<jobject> (env->CallObjectMethod (plane, AndroidImagePlane.getBuffer));

            ImageBuffer correctedBuffer = getImageBufferWithCorrectedOrientationFrom (byteBuffer, deviceOrientationFromAccelerometerSensor,
                                                                                      targetOrientation, cameraLensFrontFacing, cameraSensorOrientation);

            jbyte* rawBytes = env->GetByteArrayElements (correctedBuffer.byteArray, nullptr);

            Image result = ImageFileFormat::loadFrom (rawBytes, (size_t) correctedBuffer.size);

            env->ReleaseByteArrayElements (correctedBuffer.byteArray, rawBytes, 0);

            return result;
        }

        static ImageBuffer getImageBufferWithCorrectedOrientationFrom (const LocalRef<jobject>& imagePlaneBuffer,
                                                                       Desktop::DisplayOrientation deviceOrientationFromAccelerometerSensor,
                                                                       Desktop::DisplayOrientation targetOrientation,
                                                                       bool cameraLensFrontFacing,
                                                                       int cameraSensorOrientation)
        {
            auto* env = getEnv();

            auto bufferSize = env->CallIntMethod (imagePlaneBuffer, JavaByteBuffer.remaining);
            auto byteArray = LocalRef<jbyteArray> (env->NewByteArray (bufferSize));
            env->CallObjectMethod (imagePlaneBuffer, JavaByteBuffer.get, byteArray.get());

            auto rotationAngle = getRotationAngle (deviceOrientationFromAccelerometerSensor, targetOrientation,
                                                  cameraLensFrontFacing, cameraSensorOrientation);

            if (rotationAngle == 0)
            {
                // Nothing to do, just get the bytes
                return { byteArray, bufferSize };
            }

            auto origBitmap = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidBitmapFactory,
                                                                              AndroidBitmapFactory.decodeByteArray,
                                                                              byteArray.get(), (jint) 0, (jint) bufferSize));

            auto correctedBitmap = getBitmapWithCorrectOrientationFrom (origBitmap, rotationAngle);

            auto byteArrayOutputStream = LocalRef<jobject> (env->NewObject (ByteArrayOutputStream,
                                                                            ByteArrayOutputStream.constructor));

            auto jCompressFormatString = javaString ("JPEG");
            auto compressFormat = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidBitmapCompressFormat,
                                                                                  AndroidBitmapCompressFormat.valueOf,
                                                                                  jCompressFormatString.get()));

            if (env->CallBooleanMethod (correctedBitmap, AndroidBitmap.compress, compressFormat.get(),
                                        (jint) 100, byteArrayOutputStream.get()) != 0)
            {
                auto correctedByteArray = LocalRef<jbyteArray> ((jbyteArray) env->CallObjectMethod (byteArrayOutputStream,
                                                                                                    ByteArrayOutputStream.toByteArray));

                int correctedByteArraySize = env->CallIntMethod (byteArrayOutputStream, ByteArrayOutputStream.size);

                return { correctedByteArray, correctedByteArraySize };
            }

            jassertfalse;
            // fallback, return original bitmap
            return { byteArray, bufferSize };
        }

        static int getRotationAngle (Desktop::DisplayOrientation deviceOrientationFromAccelerometerSensor,
                                     Desktop::DisplayOrientation targetOrientation,
                                     bool cameraLensFrontFacing,
                                     int cameraSensorOrientation)
        {
            auto isSensorOrientationHorizontal = deviceOrientationFromAccelerometerSensor == Desktop::rotatedAntiClockwise
                                              || deviceOrientationFromAccelerometerSensor == Desktop::rotatedClockwise;

            if (cameraLensFrontFacing && isSensorOrientationHorizontal)
            {
                // flip angles for front camera
                return getRotationAngle (deviceOrientationFromAccelerometerSensor, targetOrientation, false, (cameraSensorOrientation + 180) % 360);
            }

            switch (targetOrientation)
            {
                case Desktop::rotatedAntiClockwise:
                    return cameraSensorOrientation == 90 ? 0 : 180;
                case Desktop::rotatedClockwise:
                    return cameraSensorOrientation == 90 ? 180 : 0;
                case Desktop::upright:
                case Desktop::upsideDown:
                    if ((targetOrientation == Desktop::upright && ! cameraLensFrontFacing)
                        || (targetOrientation == Desktop::upsideDown && cameraLensFrontFacing))
                    {
                        return cameraSensorOrientation;
                    }
                    else
                    {
                        if (deviceOrientationFromAccelerometerSensor == Desktop::upright || deviceOrientationFromAccelerometerSensor == Desktop::upsideDown)
                            return cameraSensorOrientation;
                        else
                            return (cameraSensorOrientation + 180) % 360;
                    }
                    break;
                default:
                    return 0;
            }
        }

        static LocalRef<jobject> getBitmapWithCorrectOrientationFrom (LocalRef<jobject>& origBitmap, int rotationAngle)
        {
            auto* env = getEnv();

            auto origBitmapWidth  = env->CallIntMethod (origBitmap, AndroidBitmap.getWidth);
            auto origBitmapHeight = env->CallIntMethod (origBitmap, AndroidBitmap.getHeight);

            auto matrix = LocalRef<jobject> (env->NewObject (AndroidMatrix, AndroidMatrix.constructor));
            env->CallBooleanMethod (matrix, AndroidMatrix.postRotate, (jfloat) rotationAngle, (jfloat) 0, (jfloat) 0);

            auto rotatedBitmap = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidBitmap, AndroidBitmap.createBitmapFrom,
                                                                                 origBitmap.get(), (jint) 0, (jint) 0,
                                                                                 (jint) origBitmapWidth, (jint) origBitmapHeight,
                                                                                 matrix.get(), true));

            env->CallVoidMethod (origBitmap, AndroidBitmap.recycle);

            return rotatedBitmap;
        }
    };

    //==============================================================================
    class MediaRecorder : private MediaRecorderOnInfoListener::Owner,
                          private MediaRecorderOnErrorListener::Owner
    {
    public:
        MediaRecorder (const String& outputFilePath, int videoWidth, int videoHeight,
                       int sensorOrientation, int cameraLensFacing)
            : onInfoListener (*this),
              onErrorListener (*this),
              mediaRecorder (LocalRef<jobject> (getEnv()->NewObject (AndroidMediaRecorder,
                                                                     AndroidMediaRecorder.constructor)))
        {
            auto* env = getEnv();

            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setOnInfoListener,
                                 CreateJavaInterface (&onInfoListener,
                                                      "android/media/MediaRecorder$OnInfoListener").get());

            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setOnErrorListener,
                                 CreateJavaInterface (&onErrorListener,
                                                      "android/media/MediaRecorder$OnErrorListener").get());

            // NB: the order of function calls here is enforced, and exceptions will be thrown if
            //     the order is changed.
            static constexpr int audioSourceMic = 1;
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setAudioSource, (jint) audioSourceMic);

            static constexpr int videoSourceSurface = 2;
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setVideoSource, (jint) videoSourceSurface);

            static constexpr int outputFormatMPEG4 = 2;
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setOutputFormat, (jint) outputFormatMPEG4);

            static constexpr int audioEncoderAAC = 3;
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setAudioEncoder, (jint) audioEncoderAAC);

            static constexpr int videoEncoderH264 = 2;
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setVideoEncoder, (jint) videoEncoderH264);

            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setVideoEncodingBitRate, (jint) 10000000);
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setVideoFrameRate, (jint) 30);

            auto frontFacing = cameraLensFacing == 0;

            auto useInverseDegrees = frontFacing && sensorOrientation == 90;

            int orientationHint = getOrientationHint (useInverseDegrees, sensorOrientation);
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setOrientationHint, (jint) orientationHint);

            getEnv()->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setVideoSize, (jint) videoWidth, (jint) videoHeight);
            getEnv()->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.setOutputFile, javaString (outputFilePath).get());
            getEnv()->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.prepare);
        }

        ~MediaRecorder()
        {
            getEnv()->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.release);
        }

        LocalRef<jobject> getSurface()
        {
            return LocalRef<jobject> (getEnv()->CallObjectMethod (mediaRecorder, AndroidMediaRecorder.getSurface));
        }

        void start()
        {
            lockScreenOrientation();

            getEnv()->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.start);

            hasStartedRecording = true;
        }

        void stop()
        {
            // A request to stop can be sent before recording has had a chance to start, so
            // ignore the request rather than calling AndroidMediaRecorder.stop because
            // otherwise MediaRecorder will throw an exception and...
            if (! hasStartedRecording)
                return;

            hasStartedRecording = false;

            auto* env = getEnv();
            env->CallVoidMethod (mediaRecorder, AndroidMediaRecorder.stop);

            // ... ignore RuntimeException that can be thrown if stop() was called after recording
            // has started but before any frame was written to a file. This is not an error.
            jniCheckHasExceptionOccurredAndClear();

            unlockScreenOrientation();
        }

    private:
        MediaRecorderOnInfoListener onInfoListener;
        MediaRecorderOnErrorListener onErrorListener;
        GlobalRef mediaRecorder;
        bool hasStartedRecording = false;
        int orientationsEnabled = -1;

        void lockScreenOrientation()
        {
            orientationsEnabled = Desktop::getInstance().getOrientationsEnabled();

            auto o = Desktop::getInstance().getCurrentOrientation();
            Desktop::getInstance().setOrientationsEnabled (o);
        }

        static jint juceOrientationToNativeOrientation (int orientations) noexcept
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

        void unlockScreenOrientation()
        {
            Desktop::getInstance().setOrientationsEnabled (orientationsEnabled);
        }

        void onInfo (LocalRef<jobject>& recorder, int what, int extra) override
        {
            ignoreUnused (recorder, what, extra);

            JUCE_CAMERA_LOG ("MediaRecorder::OnInfo: " + getInfoStringFromCode (what)
                                     + ", extra code = " + String (extra));
        }

        void onError (LocalRef<jobject>& recorder, int what, int extra) override
        {
            ignoreUnused (recorder, what, extra);

            JUCE_CAMERA_LOG ("MediaRecorder::onError: " + getErrorStringFromCode (what)
                                     + ", extra code = " + String (extra));
        }

        static String getInfoStringFromCode (int what)
        {
            enum
            {
                MEDIA_RECORDER_INFO_UNKNOWN = 1,
                MEDIA_RECORDER_INFO_MAX_DURATION_REACHED = 800,
                MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED = 801,
                MEDIA_RECORDER_INFO_MAX_FILESIZE_APPROACHING = 802,
                MEDIA_RECORDER_INFO_NEXT_OUTPUT_FILE_STARTED = 803
            };

            switch (what)
            {
                case MEDIA_RECORDER_INFO_UNKNOWN:                  return { "Unknown info" };
                case MEDIA_RECORDER_INFO_MAX_DURATION_REACHED:     return { "Max duration reached" };
                case MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED:     return { "Max filesize reached" };
                case MEDIA_RECORDER_INFO_MAX_FILESIZE_APPROACHING: return { "Max filesize approaching" };
                case MEDIA_RECORDER_INFO_NEXT_OUTPUT_FILE_STARTED: return { "Next output file started" };
                default: return String (what);
            };
        }

        static String getErrorStringFromCode (int what)
        {
            enum
            {
                MEDIA_RECORDER_ERROR_UNKNOWN = 1,
                MEDIA_ERROR_SERVER_DIED = 100
            };

            switch (what)
            {
                case MEDIA_RECORDER_ERROR_UNKNOWN:   return { "Unknown error" };
                case MEDIA_ERROR_SERVER_DIED:        return { "Server died" };
                default: return String (what);
            };
        }

        static int getOrientationHint (bool useInverseDegrees, int cameraSensorOrientation)
        {
            auto* env = getEnv();

            auto windowManager = LocalRef<jobject> (env->CallObjectMethod (android.activity, JuceAppActivity.getWindowManager));
            auto display = LocalRef<jobject> (env->CallObjectMethod (windowManager, AndroidWindowManager.getDefaultDisplay));
            auto rotation = env->CallIntMethod (display, AndroidDisplay.getRotation);

            enum
            {
                ROTATION_0 = 0,
                ROTATION_90,
                ROTATION_180,
                ROTATION_270
            };

            int hint = 0;

            switch (rotation)
            {
                case ROTATION_0:   hint = cameraSensorOrientation;       break;
                case ROTATION_90:  hint = useInverseDegrees ? 180 : 0;   break;
                case ROTATION_180: hint = cameraSensorOrientation + 180; break;
                case ROTATION_270: hint = useInverseDegrees ? 0 : 180;   break;
                default: jassertfalse;
            }

            return (hint + 360) % 360;
        }
    };

    //==============================================================================
    class ScopedCameraDevice
    {
    public:
        //==============================================================================
        class CaptureSession
        {
        public:
            struct ConfiguredCallback
            {
                virtual ~ConfiguredCallback() {}

                virtual void captureSessionConfigured (CaptureSession*) = 0;
            };

            ~CaptureSession()
            {
                bool calledClose = false;

                auto* env = getEnv();

                {
                    const ScopedLock lock (captureSessionLock);

                    if (captureSession.get() != nullptr)
                    {
                        calledClose = true;

                        env->CallVoidMethod (captureSession, CameraCaptureSession.close);
                    }
                }

                // When exception occurs, CameraCaptureSession.close will never finish, so
                // we should not wait for it. For fatal error an exception does occur, but
                // it is catched internally in Java...
                if (jniCheckHasExceptionOccurredAndClear() || scopedCameraDevice.fatalErrorOccurred.get())
                {
                    JUCE_CAMERA_LOG ("Exception or fatal error occurred while closing Capture Session, closing by force");
                }
                else if (calledClose)
                {
                    pendingClose.set (1);
                    closedEvent.wait (-1);
                }
            }

            bool openedOk() const noexcept { return captureSession != nullptr; }

            const GlobalRef& getNativeSession() const { return captureSession; }

            bool start (const LocalRef<jobject>& targetSurfacesList, GlobalRef& handlerToUse)
            {
                if (! openedOk())
                {
                    jassertfalse;
                    return false;
                }

                auto* env = getEnv();

                auto numSurfaces = env->CallIntMethod (targetSurfacesList, JavaArrayList.size);

                for (int i = 0; i < numSurfaces; ++i)
                {
                    auto surface = LocalRef<jobject> (env->CallObjectMethod (targetSurfacesList, JavaArrayList.get, (jint) i));
                    env->CallVoidMethod (captureRequestBuilder, CaptureRequestBuilder.addTarget, surface.get());
                }

                previewCaptureRequest = GlobalRef (env->CallObjectMethod (captureRequestBuilder, CaptureRequestBuilder.build));

                env->CallIntMethod (captureSession, CameraCaptureSession.setRepeatingRequest,
                                    previewCaptureRequest.get(), nullptr, handlerToUse.get());

                return true;
            }

            void takeStillPicture (jobject targetSurface)
            {
                if (stillPictureTaker == nullptr)
                {
                    // Can only take picture once session was successfully configured!
                    jassertfalse;
                    return;
                }

                auto* env = getEnv();

                static constexpr int templateStillCapture = 2;
                auto builder = LocalRef<jobject> (env->CallObjectMethod (scopedCameraDevice.cameraDevice,
                                                                         AndroidCameraDevice.createCaptureRequest,
                                                                         (jint) templateStillCapture));

                env->CallVoidMethod (builder, CaptureRequestBuilder.addTarget, targetSurface);

                setCaptureRequestBuilderIntegerKey (builder.get(), CaptureRequest.CONTROL_AF_MODE, autoFocusMode);

                auto stillPictureCaptureRequest = LocalRef<jobject> (env->CallObjectMethod (builder, CaptureRequestBuilder.build));

                stillPictureTaker->takePicture (stillPictureCaptureRequest.get());
            }

        private:
            //==============================================================================
            class StillPictureTaker   : private AndroidRunnable::Owner
            {
            public:
                StillPictureTaker (GlobalRef& captureSessionToUse, GlobalRef& captureRequestBuilderToUse,
                                   GlobalRef& previewCaptureRequestToUse, GlobalRef& handlerToUse,
                                   int autoFocusModeToUse)
                    : captureSession (captureSessionToUse),
                      captureRequestBuilder (captureRequestBuilderToUse),
                      previewCaptureRequest (previewCaptureRequestToUse),
                      handler (handlerToUse),
                      runnable (*this),
                      captureSessionPreviewCaptureCallback (LocalRef<jobject> (getEnv()->NewObject (CameraCaptureSessionCaptureCallback,
                                                                                                    CameraCaptureSessionCaptureCallback.constructor,
                                                                                                    android.activity.get(),
                                                                                                    reinterpret_cast<jlong> (this),
                                                                                                    true))),
                      captureSessionStillPictureCaptureCallback (LocalRef<jobject> (getEnv()->NewObject (CameraCaptureSessionCaptureCallback,
                                                                                                         CameraCaptureSessionCaptureCallback.constructor,
                                                                                                         android.activity.get(),
                                                                                                         reinterpret_cast<jlong> (this),
                                                                                                         false))),
                      autoFocusMode (autoFocusModeToUse)
                {
                }

                void takePicture (jobject stillPictureCaptureRequestToUse)
                {
                    JUCE_CAMERA_LOG ("Taking picture...");

                    stillPictureCaptureRequest = GlobalRef (stillPictureCaptureRequestToUse);

                    lockFocus();
                }

            private:
                GlobalRef& captureSession;
                GlobalRef& captureRequestBuilder;
                GlobalRef& previewCaptureRequest;
                GlobalRef& handler;

                AndroidRunnable runnable;
                GlobalRef delayedCaptureRunnable;

                GlobalRef captureSessionPreviewCaptureCallback;

                GlobalRef stillPictureCaptureRequest;
                GlobalRef captureSessionStillPictureCaptureCallback;

                int autoFocusMode;

                enum class State
                {
                    idle = 0,
                    pendingFocusLock,
                    pendingExposurePrecapture,
                    pendingExposurePostPrecapture,
                    pictureTaken
                };

                State currentState = State::idle;

                void lockFocus()
                {
                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    JUCE_CAMERA_LOG ("Performing auto-focus if possible...");

                    currentState = State::pendingFocusLock;

                    auto* env = getEnv();

                    // NB: auto-focus may be unavailable on a device, in which case it may have already
                    // automatically adjusted the exposure. We check for that in updateState().
                    static constexpr int controlAfTriggerStart = 1;
                    CaptureSession::setCaptureRequestBuilderIntegerKey (captureRequestBuilder.get(),
                                                                        CaptureRequest.CONTROL_AF_TRIGGER,
                                                                        controlAfTriggerStart);

                    auto previewRequest = LocalRef<jobject> (env->CallObjectMethod (captureRequestBuilder,
                                                                                    CaptureRequestBuilder.build));

                    env->CallIntMethod (captureSession, CameraCaptureSession.capture, previewRequest.get(),
                                        captureSessionPreviewCaptureCallback.get(), handler.get());
                }

                void updateState (jobject captureResult)
                {
                    // IllegalStateException can be thrown when accessing CaptureSession,
                    // claiming that capture session was already closed but we may not
                    // get relevant callback yet, so check for this and bailout when needed.
                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    switch (currentState)
                    {
                        case State::pendingFocusLock:
                        {
                            JUCE_CAMERA_LOG ("Still picture capture, updateState(), State::pendingFocusLock...");

                            auto controlAfStateValue = getCaptureResultIntegerKeyValue (CaptureResult.CONTROL_AF_STATE, captureResult);

                            if (controlAfStateValue.get() == nullptr)
                            {
                                captureStillPictureDelayed();
                                return;
                            }

                            auto autoToFocusNotAvailable = autoFocusMode == 0;

                            if (autoToFocusNotAvailable || autoFocusHasFinished (controlAfStateValue))
                            {
                                auto controlAeStateIntValue = getControlAEState (captureResult);
                                static constexpr int controlAeStateConverged = 2;

                                if (controlAeStateIntValue == -1 || controlAeStateIntValue == controlAeStateConverged)
                                {
                                    currentState = State::pictureTaken;
                                    captureStillPictureDelayed();
                                }
                                else
                                {
                                    runPrecaptureSequence();
                                }
                            }

                            break;
                        }

                        case State::pendingExposurePrecapture:
                        {
                            JUCE_CAMERA_LOG ("Still picture capture, updateState(), State::pendingExposurePrecapture...");

                            auto controlAeStateIntValue = getControlAEState (captureResult);
                            static constexpr int controlAeStateFlashRequired = 4;
                            static constexpr int controlAeStatePrecapture = 5;

                            if (controlAeStateIntValue == -1 || controlAeStateIntValue == controlAeStateFlashRequired
                                                             || controlAeStateIntValue == controlAeStatePrecapture)
                            {
                                currentState = State::pendingExposurePostPrecapture;
                            }

                            break;
                        }

                        case State::pendingExposurePostPrecapture:
                        {
                            JUCE_CAMERA_LOG ("Still picture capture, updateState(), State::pendingExposurePostPrecapture...");

                            auto controlAeStateIntValue = getControlAEState (captureResult);
                            static constexpr int controlAeStatePrecapture = 5;

                            if (controlAeStateIntValue == -1 || controlAeStateIntValue != controlAeStatePrecapture)
                            {
                                currentState = State::pictureTaken;
                                captureStillPictureDelayed();
                            }

                            break;
                        }
                        case State::idle:
                        case State::pictureTaken:
                            { /* do nothing */ break; }
                    };
                }

                static int getControlAEState (jobject captureResult)
                {
                    auto controlAeStateValue = getCaptureResultIntegerKeyValue (CaptureResult.CONTROL_AE_STATE, captureResult);

                    return controlAeStateValue.get() != nullptr
                                    ? getEnv()->CallIntMethod (controlAeStateValue, JavaInteger.intValue) : -1;
                }

                static bool autoFocusHasFinished (const LocalRef<jobject>& controlAfStateValue)
                {
                    static constexpr int controlAfStateFocusedLocked = 4;
                    static constexpr int controlAfStateNotFocusedLocked = 5;

                    auto controlAfStateIntValue = getEnv()->CallIntMethod (controlAfStateValue, JavaInteger.intValue);

                    return controlAfStateIntValue == controlAfStateFocusedLocked || controlAfStateIntValue == controlAfStateNotFocusedLocked;
                }

                static LocalRef<jobject> getCaptureResultIntegerKeyValue (jfieldID key, jobject captureResult)
                {
                    auto* env = getEnv();

                    auto jKey = LocalRef<jobject> (env->GetStaticObjectField (CaptureResult, key));
                    return LocalRef<jobject> (env->CallObjectMethod (captureResult, CaptureResult.get, jKey.get()));
                }

                void captureStillPictureDelayed()
                {
                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    JUCE_CAMERA_LOG ("Still picture capture, device ready, capturing now...");

                    auto* env = getEnv();

                    env->CallVoidMethod (captureSession, CameraCaptureSession.stopRepeating);

                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    env->CallVoidMethod (captureSession, CameraCaptureSession.abortCaptures);

                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    // Delay still picture capture for devices that can't handle it right after
                    // stopRepeating/abortCaptures calls.
                    if (delayedCaptureRunnable.get() == nullptr)
                        delayedCaptureRunnable = GlobalRef (CreateJavaInterface (&runnable, "java/lang/Runnable").get());

                    env->CallBooleanMethod (handler, AndroidHandler.postDelayed, delayedCaptureRunnable.get(), (jlong) 200);
                }

                void runPrecaptureSequence()
                {
                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    auto* env = getEnv();

                    static constexpr int controlAePrecaptureTriggerStart = 1;
                    CaptureSession::setCaptureRequestBuilderIntegerKey (captureRequestBuilder.get(),
                                                                        CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                                                                        controlAePrecaptureTriggerStart);

                    currentState = State::pendingExposurePrecapture;

                    auto previewRequest = LocalRef<jobject> (env->CallObjectMethod (captureRequestBuilder,
                                                                                    CaptureRequestBuilder.build));

                    env->CallIntMethod (captureSession, CameraCaptureSession.capture, previewRequest.get(),
                                        captureSessionPreviewCaptureCallback.get(), handler.get());
                }

                void unlockFocus()
                {
                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    JUCE_CAMERA_LOG ("Unlocking focus...");

                    currentState = State::idle;

                    auto* env = getEnv();

                    static constexpr int controlAfTriggerCancel = 2;
                    CaptureSession::setCaptureRequestBuilderIntegerKey (captureRequestBuilder.get(),
                                                                        CaptureRequest.CONTROL_AF_TRIGGER,
                                                                        controlAfTriggerCancel);

                    auto resetAutoFocusRequest = LocalRef<jobject> (env->CallObjectMethod (captureRequestBuilder,
                                                                                           CaptureRequestBuilder.build));

                    env->CallIntMethod (captureSession, CameraCaptureSession.capture, resetAutoFocusRequest.get(),
                                        nullptr, handler.get());

                    if (jniCheckHasExceptionOccurredAndClear())
                        return;

                    // NB: for preview, using preview capture request again
                    env->CallIntMethod (captureSession, CameraCaptureSession.setRepeatingRequest, previewCaptureRequest.get(),
                                        nullptr, handler.get());
                }
                //==============================================================================
                void run() override
                {
                    captureStillPicture();
                }

                void captureStillPicture()
                {
                    getEnv()->CallIntMethod (captureSession, CameraCaptureSession.capture,
                                             stillPictureCaptureRequest.get(), captureSessionStillPictureCaptureCallback.get(),
                                             nullptr);
                }

                //==============================================================================
                void cameraCaptureSessionCaptureCompleted (bool isPreview, jobject session, jobject request, jobject result)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureCompleted()");

                    ignoreUnused (session, request);

                    if (isPreview)
                        updateState (result);
                    else if (currentState != State::idle)
                        unlockFocus();
                }

                void cameraCaptureSessionCaptureFailed (bool isPreview, jobject session, jobject request, jobject failure)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureFailed()");

                    ignoreUnused (isPreview, session, request, failure);
                }

                void cameraCaptureSessionCaptureProgressed (bool isPreview, jobject session, jobject request, jobject partialResult)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureProgressed()");

                    ignoreUnused (session, request);

                    if (isPreview)
                        updateState (partialResult);
                }

                void cameraCaptureSessionCaptureSequenceAborted (bool isPreview, jobject session, int sequenceId)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureSequenceAborted()");

                    ignoreUnused (isPreview, isPreview, session, sequenceId);
                }

                void cameraCaptureSessionCaptureSequenceCompleted (bool isPreview, jobject session, int sequenceId, int64 frameNumber)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureSequenceCompleted()");

                    ignoreUnused (isPreview, session, sequenceId, frameNumber);
                }

                void cameraCaptureSessionCaptureStarted (bool isPreview, jobject session, jobject request, int64 timestamp, int64 frameNumber)
                {
                    JUCE_CAMERA_LOG ("cameraCaptureSessionCaptureStarted()");

                    ignoreUnused (isPreview, session, request, timestamp, frameNumber);
                }

                friend void juce_cameraCaptureSessionCaptureCompleted (int64, bool, void*, void*, void*);
                friend void juce_cameraCaptureSessionCaptureFailed (int64, bool, void*, void*, void*);
                friend void juce_cameraCaptureSessionCaptureProgressed (int64, bool, void*, void*, void*);
                friend void juce_cameraCaptureSessionCaptureSequenceAborted (int64, bool, void*, int);
                friend void juce_cameraCaptureSessionCaptureSequenceCompleted (int64, bool, void*, int, int64);
                friend void juce_cameraCaptureSessionCaptureStarted (int64, bool, void*, void*, int64, int64);
            };

            //==============================================================================
            ScopedCameraDevice& scopedCameraDevice;
            ConfiguredCallback& configuredCallback;
            GlobalRef& handler;

            GlobalRef captureRequestBuilder;
            GlobalRef previewCaptureRequest;

            GlobalRef captureSessionStateCallback;
            int autoFocusMode;

            GlobalRef captureSession;
            CriticalSection captureSessionLock;

            Atomic<int> pendingClose { 0 };

            std::unique_ptr<StillPictureTaker> stillPictureTaker;

            WaitableEvent closedEvent;

            JUCE_DECLARE_WEAK_REFERENCEABLE (CaptureSession)

            //==============================================================================
            CaptureSession (ScopedCameraDevice& scopedCameraDeviceToUse, ConfiguredCallback& configuredCallbackToUse,
                            const LocalRef<jobject>& surfacesList, GlobalRef& handlerToUse,
                            int captureSessionTemplate, int autoFocusModeToUse)
                : scopedCameraDevice (scopedCameraDeviceToUse),
                  configuredCallback (configuredCallbackToUse),
                  handler (handlerToUse),
                  captureRequestBuilder (LocalRef<jobject> (getEnv()->CallObjectMethod (scopedCameraDevice.cameraDevice,
                                                                                        AndroidCameraDevice.createCaptureRequest,
                                                                                        (jint) captureSessionTemplate))),
                  captureSessionStateCallback (LocalRef<jobject> (getEnv()->NewObject (CameraCaptureSessionStateCallback,
                                                                                       CameraCaptureSessionStateCallback.constructor,
                                                                                       android.activity.get(),
                                                                                       reinterpret_cast<jlong> (this)))),
                  autoFocusMode (autoFocusModeToUse)
            {
                auto* env = getEnv();

                env->CallVoidMethod (scopedCameraDevice.cameraDevice, AndroidCameraDevice.createCaptureSession,
                                     surfacesList.get(), captureSessionStateCallback.get(), handler.get());

                static constexpr int controlModeAuto = 1;
                setCaptureRequestBuilderIntegerKey (captureRequestBuilder.get(), CaptureRequest.CONTROL_MODE, controlModeAuto);

                setCaptureRequestBuilderIntegerKey (captureRequestBuilder.get(), CaptureRequest.CONTROL_AF_MODE, autoFocusMode);
            }

            static void setCaptureRequestBuilderIntegerKey (jobject captureRequestBuilder, jfieldID key, int value)
            {
                auto* env = getEnv();

                auto jKey = LocalRef<jobject> (env->GetStaticObjectField (CaptureRequest, key));
                auto jValue = LocalRef<jobject> (env->CallStaticObjectMethod (JavaInteger, JavaInteger.valueOf, (jint) value));

                env->CallVoidMethod (captureRequestBuilder, CaptureRequestBuilder.set, jKey.get(), jValue.get());
            }

            void cameraCaptureSessionActive (jobject session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionActive()");
                ignoreUnused (session);
            }

            void cameraCaptureSessionClosed (jobject session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionClosed()");
                ignoreUnused (session);

                closedEvent.signal();
            }

            void cameraCaptureSessionConfigureFailed (jobject session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionConfigureFailed()");
                ignoreUnused (session);

                WeakReference<CaptureSession> weakRef (this);

                MessageManager::callAsync ([this, weakRef]()
                {
                    if (weakRef == nullptr)
                        return;

                    configuredCallback.captureSessionConfigured (nullptr);
                });
            }

            void cameraCaptureSessionConfigured (jobject session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionConfigured()");

                if (pendingClose.get() == 1)
                {
                    // Already closing, bailout.
                    closedEvent.signal();

                    GlobalRef s (session);

                    MessageManager::callAsync ([s]()
                        {
                            getEnv()->CallVoidMethod (s, CameraCaptureSession.close);
                        });

                    return;
                }

                {
                    const ScopedLock lock (captureSessionLock);
                    captureSession = GlobalRef (session);
                }

                WeakReference<CaptureSession> weakRef (this);

                MessageManager::callAsync ([this, weakRef]()
                {
                    if (weakRef == nullptr)
                        return;

                    stillPictureTaker.reset (new StillPictureTaker (captureSession, captureRequestBuilder,
                                                                    previewCaptureRequest, handler, autoFocusMode));

                    configuredCallback.captureSessionConfigured (this);
                });
            }

            void cameraCaptureSessionReady (jobject session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionReady()");
                ignoreUnused (session);
            }

            friend class ScopedCameraDevice;

            friend void juce_cameraCaptureSessionActive (int64, void*);
            friend void juce_cameraCaptureSessionClosed (int64, void*);
            friend void juce_cameraCaptureSessionConfigureFailed (int64, void*);
            friend void juce_cameraCaptureSessionConfigured (int64, void*);
            friend void juce_cameraCaptureSessionReady (int64, void*);

            friend void juce_cameraCaptureSessionCaptureCompleted (int64, bool, void*, void*, void*);
            friend void juce_cameraCaptureSessionCaptureFailed (int64, bool, void*, void*, void*);
            friend void juce_cameraCaptureSessionCaptureProgressed (int64, bool, void*, void*, void*);
            friend void juce_cameraCaptureSessionCaptureSequenceAborted (int64, bool, void*, int);
            friend void juce_cameraCaptureSessionCaptureSequenceCompleted (int64, bool, void*, int, int64);
            friend void juce_cameraCaptureSessionCaptureStarted (int64, bool, void*, void*, int64, int64);

            JUCE_DECLARE_NON_COPYABLE (CaptureSession)
        };

        //==============================================================================
        ScopedCameraDevice (Pimpl& ownerToUse, const String& cameraIdToUse, GlobalRef& cameraManagerToUse,
                            GlobalRef& handlerToUse, int autoFocusModeToUse)
            : owner (ownerToUse),
              cameraId (cameraIdToUse),
              cameraManager (cameraManagerToUse),
              handler (handlerToUse),
              cameraStateCallback (LocalRef<jobject> (getEnv()->NewObject (CameraDeviceStateCallback,
                                                                           CameraDeviceStateCallback.constructor,
                                                                           android.activity.get(),
                                                                           reinterpret_cast<jlong> (this)))),
              autoFocusMode (autoFocusModeToUse)
        {
            open();
        }

        ~ScopedCameraDevice()
        {
            close();
        }

        void open()
        {
            pendingOpen.set (1);

            auto* env = getEnv();

            env->CallVoidMethod (cameraManager, CameraManager.openCamera,
                                 javaString (cameraId).get(),
                                 cameraStateCallback.get(), handler.get());

            // If something went wrong we will be pinged in cameraDeviceStateError()
            // callback, silence the redundant exception.
            jniCheckHasExceptionOccurredAndClear();
        }

        void close()
        {
            if (pendingClose.compareAndSetBool (1, 0))
            {
                auto* env = getEnv();

                if (cameraDevice.get() != nullptr)
                {
                    env->CallVoidMethod (cameraDevice, AndroidCameraDevice.close);
                    closedEvent.wait (-1);
                }

                pendingClose.set (0);
                pendingOpen .set (0);
                cameraDevice.clear();
            }
        }

        bool openedOk() const { return cameraDevice != nullptr; }

        bool hasErrorOccurred() const { return fatalErrorOccurred.get(); }

        CaptureSession* createCaptureSession (CaptureSession::ConfiguredCallback& cc,
                                              const LocalRef<jobject>& surfacesList,
                                              GlobalRef& handlerToUse,
                                              int captureSessionTemplate)
        {
            if (! openedOk())
            {
                jassertfalse;
                return nullptr;
            }

            return new CaptureSession (*this, cc, surfacesList, handlerToUse, captureSessionTemplate, autoFocusMode);
        }

    private:
        Pimpl& owner;
        const String cameraId;
        GlobalRef& cameraManager;
        GlobalRef& handler;

        GlobalRef cameraStateCallback;
        int autoFocusMode;

        GlobalRef cameraDevice;
        Atomic<int> pendingOpen { 0 };
        Atomic<int> pendingClose { 0 };
        Atomic<int> fatalErrorOccurred { 0 };
        String openError;

        WaitableEvent closedEvent;

        void cameraDeviceStateClosed()
        {
            JUCE_CAMERA_LOG ("cameraDeviceStateClosed()");

            closedEvent.signal();
        }

        void cameraDeviceStateDisconnected()
        {
            JUCE_CAMERA_LOG ("cameraDeviceStateDisconnected()");

            if (pendingOpen.compareAndSetBool (0, 1))
            {
                openError = "Device disconnected";

                notifyOpenResult();
            }

            MessageManager::callAsync ([this]() { close(); });
        }

        void cameraDeviceStateError (int errorCode)
        {
            String error = cameraErrorCodeToString (errorCode);

            JUCE_CAMERA_LOG ("cameraDeviceStateError(), error: " + error);

            if (pendingOpen.compareAndSetBool (0, 1))
            {
                openError = error;

                notifyOpenResult();
            }

            fatalErrorOccurred.set (1);

            MessageManager::callAsync ([this, error]()
                                       {
                                           owner.cameraDeviceError (error);
                                           close();
                                       });
        }

        void cameraDeviceStateOpened (jobject cameraDeviceToUse)
        {
            JUCE_CAMERA_LOG ("cameraDeviceStateOpened()");

            pendingOpen.set (0);

            cameraDevice = GlobalRef (cameraDeviceToUse);

            notifyOpenResult();
        }

        void notifyOpenResult()
        {
            MessageManager::callAsync ([this]() { owner.cameraOpenFinished (openError); });
        }

        friend void juce_cameraDeviceStateClosed (int64);
        friend void juce_cameraDeviceStateDisconnected (int64);
        friend void juce_cameraDeviceStateError (int64, int);
        friend void juce_cameraDeviceStateOpened (int64, void*);

        friend void juce_cameraCaptureSessionActive (int64, void*);
        friend void juce_cameraCaptureSessionClosed (int64, void*);
        friend void juce_cameraCaptureSessionConfigureFailed (int64, void*);
        friend void juce_cameraCaptureSessionConfigured (int64, void*);
        friend void juce_cameraCaptureSessionReady (int64, void*);

        friend void juce_cameraCaptureSessionCaptureCompleted (int64, bool, void*, void*, void*);
        friend void juce_cameraCaptureSessionCaptureFailed (int64, bool, void*, void*, void*);
        friend void juce_cameraCaptureSessionCaptureProgressed (int64, bool, void*, void*, void*);
        friend void juce_cameraCaptureSessionCaptureSequenceAborted (int64, bool, void*, int);
        friend void juce_cameraCaptureSessionCaptureSequenceCompleted (int64, bool, void*, int, int64);
        friend void juce_cameraCaptureSessionCaptureStarted (int64, bool, void*, void*, int64, int64);
    };

    //==============================================================================
    struct CaptureSessionModeBase
    {
        virtual ~CaptureSessionModeBase() { }

        virtual bool isVideoRecordSession() const = 0;

        virtual void triggerStillPictureCapture() = 0;
    };

    //==============================================================================
    template <typename Mode>
    struct CaptureSessionMode   : public CaptureSessionModeBase,
                                  private PreviewDisplay::Listener,
                                  private ScopedCameraDevice::CaptureSession::ConfiguredCallback
    {
        ~CaptureSessionMode()
        {
            captureSession.reset();

            previewDisplay.removeListener (this);
        }

        bool isVideoRecordSession() const override
        {
            return Mode::isVideoRecord();
        }

        void triggerStillPictureCapture() override
        {
            if (captureSession == nullptr)
            {
                // The capture session must be ready before taking a still picture.
                // Did you remember to create and show a preview display?
                jassertfalse;
                return;
            }

            crtp().takeStillPicture();
        }

    protected:
        CaptureSessionMode (Pimpl& ownerToUse, ScopedCameraDevice& cameraDeviceToUse,
                            GlobalRef& handlerToUse, PreviewDisplay& pd, int cameraSensorOrientationToUse,
                            int cameraLensFacingToUse, StreamConfigurationMap& streamConfigurationMapToUse)
            : owner (ownerToUse),
              scopedCameraDevice (cameraDeviceToUse),
              handler (handlerToUse),
              previewDisplay (pd),
              cameraSensorOrientation (cameraSensorOrientationToUse),
              cameraLensFacing (cameraLensFacingToUse),
              streamConfigurationMap (streamConfigurationMapToUse)
        {
            WeakReference<CaptureSessionMode<Mode>> weakRef (this);

            if (weakRef == nullptr)
                return;

            // async so that the object is fully constructed before the callback gets invoked
            MessageManager::callAsync ([this, weakRef]()
            {
                if (weakRef == nullptr)
                    return;

                previewDisplay.addListener (this);
            });
        }

        Mode& crtp() { return static_cast<Mode&> (*this); }

        void previewDisplayReady() override
        {
            jassert (previewDisplay.isReady());

            JUCE_CAMERA_LOG ("previewDisplayReady()");

            // close previous capture session first
            captureSession.reset();

            if (scopedCameraDevice.hasErrorOccurred())
            {
                JUCE_CAMERA_LOG ("Device error detected, not recreating a new camera session. The device needs to be reopened.");
                return;
            }

            captureSession.reset (scopedCameraDevice.createCaptureSession (*this, crtp().getCaptureSessionSurfaces(),
                                                                           handler, Mode::getTemplate()));
        }

        void previewDisplayAboutToBeDestroyed() override
        {
            JUCE_CAMERA_LOG ("previewDisplayAboutToBeDestroyed()");

            stopPreview();
        }

        void captureSessionConfigured (ScopedCameraDevice::CaptureSession* session) override
        {
            if (session == nullptr)
            {
                owner.cameraDeviceError ("Failed to configure camera session.");
                return;
            }

            jassert (session == captureSession.get());

            startSession();
        }

        void startSession()
        {
            if (! captureSession->start (crtp().getTargetSurfaces(), handler))
            {
                jassertfalse;
                JUCE_CAMERA_LOG ("Could not start capture session");
            }

            crtp().sessionStarted();
        }

        void stopPreview()
        {
            if (captureSession != nullptr)
            {
                auto session = captureSession->getNativeSession();

                auto* env = getEnv();

                env->CallVoidMethod (session, CameraCaptureSession.stopRepeating);

                if (jniCheckHasExceptionOccurredAndClear())
                    return;

                env->CallVoidMethod (session, CameraCaptureSession.abortCaptures);

                jniCheckHasExceptionOccurredAndClear();
            }
        }

        Pimpl& owner;
        ScopedCameraDevice& scopedCameraDevice;
        GlobalRef& handler;
        PreviewDisplay& previewDisplay;
        int cameraSensorOrientation;
        int cameraLensFacing;
        StreamConfigurationMap& streamConfigurationMap;

        std::unique_ptr<ScopedCameraDevice::CaptureSession> captureSession;

        JUCE_DECLARE_WEAK_REFERENCEABLE (CaptureSessionMode<Mode>)
    };

    //==============================================================================
    struct CaptureSessionPreviewMode : public CaptureSessionMode<CaptureSessionPreviewMode>
    {
        CaptureSessionPreviewMode (Pimpl& ownerToUse, ScopedCameraDevice& cameraDeviceToUse, GlobalRef& handlerToUse,
                                   PreviewDisplay& pd, ImageReader& ir, int cameraSensorOrientation,
                                   int cameraLensFacingToUse, StreamConfigurationMap& streamConfigurationMapToUse)
            : CaptureSessionMode<CaptureSessionPreviewMode> (ownerToUse, cameraDeviceToUse, handlerToUse, pd,
                                                             cameraSensorOrientation, cameraLensFacingToUse, streamConfigurationMapToUse),
              imageReader (ir)
        {
        }

        // Surfaces passed to newly created capture session.
        LocalRef<jobject> getCaptureSessionSurfaces() const
        {
            auto* env = getEnv();

            auto previewSurface = LocalRef<jobject> (previewDisplay.createSurface());
            auto imageSurface = LocalRef<jobject> (imageReader.getSurface());

            auto arrayList = LocalRef<jobject> (env->NewObject (JavaArrayList, JavaArrayList.constructor, 2));
            env->CallBooleanMethod (arrayList, JavaArrayList.add, previewSurface.get());
            env->CallBooleanMethod (arrayList, JavaArrayList.add, imageSurface.get());

            auto supported = streamConfigurationMap.isOutputSupportedForSurface (imageSurface);

            // Output surface is not supported by this device, still image capture will not work!
            jassert (supported);

            return arrayList;
        }

        // Surfaces set as target during capture.
        LocalRef<jobject> getTargetSurfaces() const
        {
            auto* env = getEnv();

            auto previewSurface = LocalRef<jobject> (previewDisplay.createSurface());

            auto arrayList = LocalRef<jobject> (env->NewObject (JavaArrayList, JavaArrayList.constructor, 1));
            env->CallBooleanMethod (arrayList, JavaArrayList.add, previewSurface.get());

            return arrayList;
        }

        static int getTemplate()
        {
            static constexpr int templatePreview = 1;
            return templatePreview;
        }

        static bool isVideoRecord() { return false; }

        void sessionStarted() {}

        void takeStillPicture()
        {
            imageReader.resetNotificationFlag();
            captureSession->takeStillPicture (imageReader.getSurface());
        }

    private:
        ImageReader& imageReader;
    };

    //==============================================================================
    struct CaptureSessionVideoRecordingMode : public CaptureSessionMode<CaptureSessionVideoRecordingMode>
    {
        CaptureSessionVideoRecordingMode (Pimpl& ownerToUse, ScopedCameraDevice& cameraDeviceToUse, GlobalRef& handlerToUse,
                                          PreviewDisplay& pd, MediaRecorder& mr, int cameraSensorOrientation,
                                          int cameraLensFacingToUse, StreamConfigurationMap& streamConfigurationMapToUse)
            : CaptureSessionMode<CaptureSessionVideoRecordingMode> (ownerToUse, cameraDeviceToUse, handlerToUse, pd,
                                                                    cameraSensorOrientation, cameraLensFacingToUse, streamConfigurationMapToUse),
              mediaRecorder (mr)
        {
        }

        ~CaptureSessionVideoRecordingMode()
        {
            // We need to explicitly stop the preview before stopping the media recorder,
            // because legacy devices can't handle recording stop before stopping the preview.
            stopPreview();

            mediaRecorder.stop();
        }

        // Surfaces passed to newly created capture session.
        LocalRef<jobject> getCaptureSessionSurfaces() const
        {
            auto* env = getEnv();

            auto previewSurface = LocalRef<jobject> (previewDisplay.createSurface());
            auto mediaRecorderSurface = LocalRef<jobject> (mediaRecorder.getSurface());

            auto arrayList = LocalRef<jobject> (env->NewObject (JavaArrayList, JavaArrayList.constructor, 2));
            env->CallBooleanMethod (arrayList, JavaArrayList.add, previewSurface.get());
            env->CallBooleanMethod (arrayList, JavaArrayList.add, mediaRecorderSurface.get());

            return arrayList;
        }

        // Surfaces set as target during capture.
        LocalRef<jobject> getTargetSurfaces() const
        {
            // Same surfaces used.
            return getCaptureSessionSurfaces();
        }

        static int getTemplate()
        {
            static constexpr int templateRecord = 3;
            return templateRecord;
        }

        static bool isVideoRecord() { return true; }

        void sessionStarted()
        {
            MessageManager::callAsync ([this]() { mediaRecorder.start(); });
        }

        void takeStillPicture()
        {
            // Taking still pictures while recording video is not supported on Android.
            jassertfalse;
        }

    private:
        MediaRecorder& mediaRecorder;
    };

    //==============================================================================
    class DeviceOrientationChangeListener   : private Timer
    {
    public:
        DeviceOrientationChangeListener (PreviewDisplay& pd)
            : previewDisplay (pd),
              orientationEventListener (getEnv()->NewObject (OrientationEventListener,
                                                             OrientationEventListener.constructor,
                                                             android.activity.get(),
                                                             reinterpret_cast<jlong> (this),
                                                             android.activity.get(),
                                                             sensorDelayUI)),
              canDetectChange (getEnv()->CallBooleanMethod (orientationEventListener,
                                                            OrientationEventListener.canDetectOrientation) != 0),
              deviceOrientation (Desktop::getInstance().getCurrentOrientation()),
              lastKnownScreenOrientation (deviceOrientation)
        {
            setEnabled (true);
        }

        ~DeviceOrientationChangeListener()
        {
            setEnabled (false);
        }

        void setEnabled (bool shouldBeEnabled)
        {
            if (shouldBeEnabled && ! canDetectChange)
            {
                // This device does not support orientation listening, photos may have wrong orientation!
                jassertfalse;
                return;
            }

            if (shouldBeEnabled)
                getEnv()->CallVoidMethod (orientationEventListener, OrientationEventListener.enable);
            else
                getEnv()->CallVoidMethod (orientationEventListener, OrientationEventListener.disable);
        }

        bool isSupported() const noexcept { return canDetectChange; }

        Desktop::DisplayOrientation getDeviceOrientation() const noexcept
        {
            return deviceOrientation;
        }

    private:
        PreviewDisplay& previewDisplay;

        GlobalRef orientationEventListener;
        static constexpr jint sensorDelayUI = 2;

        bool canDetectChange;
        Desktop::DisplayOrientation deviceOrientation;

        Desktop::DisplayOrientation lastKnownScreenOrientation;
        int numChecksForOrientationChange = 10;

        void orientationChanged (int orientation)
        {
            jassert (orientation < 360);

            // -1 == unknown
            if (orientation < 0)
                return;

            auto oldOrientation = deviceOrientation;

            // NB: this assumes natural position to be portrait always, but some devices may be landscape...
            if (orientation > (360 - 45) || orientation < 45)
                deviceOrientation = Desktop::upright;
            else if (orientation < 135)
                deviceOrientation = Desktop::rotatedClockwise;
            else if (orientation < 225)
                deviceOrientation = Desktop::upsideDown;
            else
                deviceOrientation = Desktop::rotatedAntiClockwise;

            if (oldOrientation != deviceOrientation)
            {
                lastKnownScreenOrientation = Desktop::getInstance().getCurrentOrientation();

                // Need to update preview transform, but screen orientation will change slightly
                // later than sensor orientation.
                startTimer (500);
            }
        }

        void timerCallback() override
        {
            auto currentOrientation = Desktop::getInstance().getCurrentOrientation();

            if (lastKnownScreenOrientation != currentOrientation)
            {
                lastKnownScreenOrientation = currentOrientation;

                stopTimer();
                numChecksForOrientationChange = 10;
                previewDisplay.updateSurfaceTransform();

                return;
            }

            if (--numChecksForOrientationChange == 0)
            {
                stopTimer();
                numChecksForOrientationChange = 10;
            }
        }

        friend void juce_deviceOrientationChanged (int64, int);
    };

    //==============================================================================
    CameraDevice& owner;
    int minWidth, minHeight, maxWidth, maxHeight;

    String cameraId;
    InternalOpenCameraResultCallback cameraOpenCallback;

   #if __ANDROID_API__ >= 21
    AppPausedResumedListener appPausedResumedListener;
    GlobalRef appPausedResumedListenerNative;

    GlobalRef cameraManager;
    GlobalRef cameraCharacteristics;
    GlobalRef handlerThread;
    GlobalRef handler;

    StreamConfigurationMap streamConfigurationMap;
    PreviewDisplay previewDisplay;
    DeviceOrientationChangeListener deviceOrientationChangeListener;
    std::unique_ptr<ImageReader> imageReader;
    std::unique_ptr<MediaRecorder> mediaRecorder;

    std::unique_ptr<CaptureSessionModeBase> currentCaptureSessionMode;

    std::unique_ptr<ScopedCameraDevice> scopedCameraDevice;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    std::function<void (const Image&)> pictureTakenCallback;

    Time firstRecordedFrameTimeMs;
    bool notifiedOfCameraOpening = false;
   #endif

    bool appWasPaused = false;

    //==============================================================================
    int getCameraSensorOrientation() const
    {
        return getCameraCharacteristicsIntegerKeyValue (CameraCharacteristics.SENSOR_ORIENTATION);
    }

    int getAutoFocusModeToUse() const
    {
        auto supportedModes = getSupportedAutoFocusModes();

        enum
        {
            CONTROL_AF_MODE_OFF = 0,
            CONTROL_AF_MODE_AUTO = 1,
            CONTROL_AF_MODE_CONTINUOUS_PICTURE = 4
        };

        if (supportedModes.contains (CONTROL_AF_MODE_CONTINUOUS_PICTURE))
            return CONTROL_AF_MODE_CONTINUOUS_PICTURE;

        if (supportedModes.contains (CONTROL_AF_MODE_AUTO))
            return CONTROL_AF_MODE_AUTO;

        return CONTROL_AF_MODE_OFF;
    }

    Array<int> getSupportedAutoFocusModes() const
    {
        auto* env = getEnv();

        auto jKey = LocalRef<jobject> (env->GetStaticObjectField (CameraCharacteristics, CameraCharacteristics.CONTROL_AF_AVAILABLE_MODES));

        auto supportedModes = LocalRef<jintArray> ((jintArray) env->CallObjectMethod (cameraCharacteristics,
                                                                                      CameraCharacteristics.get,
                                                                                      jKey.get()));

        return jintArrayToJuceArray (supportedModes);
    }

    static Array<int> jintArrayToJuceArray (const LocalRef<jintArray>& jArray)
    {
        auto* env = getEnv();

        auto* jArrayElems = env->GetIntArrayElements (jArray, 0);
        auto numElems = env->GetArrayLength (jArray);

        Array<int> juceArray;

        for (int s = 0; s < numElems; ++s)
            juceArray.add (jArrayElems[s]);

        env->ReleaseIntArrayElements (jArray, jArrayElems, 0);
        return juceArray;
    }

    int getCameraCharacteristicsIntegerKeyValue (jfieldID key) const
    {
        auto* env = getEnv();

        auto jKey = LocalRef<jobject> (env->GetStaticObjectField (CameraCharacteristics, key));

        auto jValue = LocalRef<jobject> (env->CallObjectMethod (cameraCharacteristics,
                                                                CameraCharacteristics.get,
                                                                jKey.get()));

        return env->CallIntMethod (jValue, JavaInteger.intValue);
    }

    int getCameraLensFacing() const
    {
        return getCameraCharacteristicsIntegerKeyValue (CameraCharacteristics.LENS_FACING);
    }

    //==============================================================================
    void cameraOpenFinished (const String& error)
    {
        JUCE_CAMERA_LOG ("cameraOpenFinished(), error = " + error);

        if (error.isEmpty())
        {
            setupStillImageSize();
            startPreviewMode (*imageReader);
        }

        // Do not notify about camera being reopened on app resume.
        if (! notifiedOfCameraOpening)
        {
            notifiedOfCameraOpening = true;

            invokeCameraOpenCallback (error);
        }
    }

    void cameraDeviceError (const String& error)
    {
        if (owner.onErrorOccurred != nullptr)
            owner.onErrorOccurred (error);
    }

    void invokeCameraOpenCallback (const String& error)
    {
        JUCE_CAMERA_LOG ("invokeCameraOpenCallback(), error = " + error);

        if (cameraOpenCallback != nullptr)
            cameraOpenCallback (cameraId, error);
    }

    //==============================================================================
    void callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });
    }

    void notifyPictureTaken (const Image& image)
    {
        JUCE_CAMERA_LOG ("notifyPictureTaken()");

        if (pictureTakenCallback != nullptr)
            pictureTakenCallback (image);
    }

    void triggerStillPictureCapture()
    {
        currentCaptureSessionMode->triggerStillPictureCapture();
    }

    //==============================================================================
    void setupStillImageSize()
    {
        imageReader.reset();

        auto imageSize = chooseBestSize (minWidth, minHeight, maxWidth, maxHeight,
                                         streamConfigurationMap.getSupportedStillImageOutputSizes());

        imageReader.reset (new ImageReader (*this, handler, imageSize.getWidth(), imageSize.getHeight(),
                                            getCameraSensorOrientation()));
    }

    static Rectangle<int> chooseBestSize (int minWidth, int minHeight, int maxWidth, int maxHeight,
                                          Array<Rectangle<int>> supportedSizes)
    {
        Rectangle<int> result;

        for (auto& size : supportedSizes)
        {
            auto width  = size.getWidth();
            auto height = size.getHeight();

            if (width < minWidth || width > maxWidth || height < minHeight || height > maxHeight)
                continue;

            if (size.contains (result))
                result = size;
        }

        // None of the supported sizes matches required width & height limitations, picking
        // the first one available...
        jassert (! result.isEmpty());

        if (result.isEmpty())
            result = supportedSizes[0];

        return result;
    }

    //==============================================================================
    void startPreviewMode (ImageReader& ir)
    {
        if (currentCaptureSessionMode != nullptr && ! currentCaptureSessionMode->isVideoRecordSession())
            return;

        // previous mode has to be stopped first
        jassert (currentCaptureSessionMode.get() == nullptr);

        if (scopedCameraDevice == nullptr || ! scopedCameraDevice->openedOk())
            return;

        currentCaptureSessionMode.reset (new CaptureSessionPreviewMode (*this, *scopedCameraDevice, handler,
                                                                        previewDisplay, ir,
                                                                        getCameraSensorOrientation(),
                                                                        getCameraLensFacing(),
                                                                        streamConfigurationMap));
    }

    void startVideoRecordingMode (MediaRecorder& mr)
    {
        if (currentCaptureSessionMode != nullptr && currentCaptureSessionMode->isVideoRecordSession())
            return;

        // previous mode has to be stopped first
        jassert (currentCaptureSessionMode.get() == nullptr);

        jassert (scopedCameraDevice != nullptr && scopedCameraDevice->openedOk());

        if (scopedCameraDevice == nullptr || ! scopedCameraDevice->openedOk())
            return;

        currentCaptureSessionMode.reset (new CaptureSessionVideoRecordingMode (*this, *scopedCameraDevice, handler,
                                                                               previewDisplay, mr,
                                                                               getCameraSensorOrientation(),
                                                                               getCameraLensFacing(),
                                                                               streamConfigurationMap));
    }

    //==============================================================================
    void appPaused() override
    {
        JUCE_CAMERA_LOG ("appPaused, closing camera...");

        appWasPaused = true;

        deviceOrientationChangeListener.setEnabled (false);

        // We need to restart the whole session mode when the app gets resumed.
        currentCaptureSessionMode.reset();

        if (scopedCameraDevice != nullptr)
            scopedCameraDevice->close();

        stopBackgroundThread();
    }

    void appResumed() override
    {
        // Only care about resumed event when paused event was called first.
        if (! appWasPaused)
            return;

        JUCE_CAMERA_LOG ("appResumed, opening camera...");

        deviceOrientationChangeListener.setEnabled (true);

        startBackgroundThread();

        if (scopedCameraDevice != nullptr)
            scopedCameraDevice->open();
    }

    void startBackgroundThread()
    {
        auto* env = getEnv();

        handlerThread = GlobalRef (LocalRef<jobject> (env->NewObject (AndroidHandlerThread,
                                                                      AndroidHandlerThread.constructor,
                                                                      javaString ("JuceCameraDeviceBackgroundThread").get())));
        // handler thread has to be started before its looper can be fetched
        env->CallVoidMethod (handlerThread, AndroidHandlerThread.start);
        handler = GlobalRef (LocalRef<jobject> (env->NewObject (AndroidHandler,
                                                                AndroidHandler.constructorWithLooper,
                                                                env->CallObjectMethod (handlerThread, AndroidHandlerThread.getLooper))));
    }

    void stopBackgroundThread()
    {
        auto* env = getEnv();

        env->CallBooleanMethod (handlerThread, AndroidHandlerThread.quitSafely);
        env->CallVoidMethod (handlerThread, AndroidHandlerThread.join);

        jniCheckHasExceptionOccurredAndClear();

        handlerThread.clear();
        handler.clear();
    }
#endif

    friend struct CameraDevice::ViewerComponent;

    friend void juce_cameraDeviceStateClosed (int64);
    friend void juce_cameraDeviceStateDisconnected (int64);
    friend void juce_cameraDeviceStateError (int64, int);
    friend void juce_cameraDeviceStateOpened (int64, void*);

    friend void juce_cameraCaptureSessionActive (int64, void*);
    friend void juce_cameraCaptureSessionClosed (int64, void*);
    friend void juce_cameraCaptureSessionConfigureFailed (int64, void*);
    friend void juce_cameraCaptureSessionConfigured (int64, void*);
    friend void juce_cameraCaptureSessionReady (int64, void*);

    friend void juce_cameraCaptureSessionCaptureCompleted (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureFailed (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureProgressed (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureSequenceAborted (int64, bool, void*, int);
    friend void juce_cameraCaptureSessionCaptureSequenceCompleted (int64, bool, void*, int, int64);
    friend void juce_cameraCaptureSessionCaptureStarted (int64, bool, void*, void*, int64, int64);

    friend void juce_deviceOrientationChanged (int64, int);

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public Component,
                                        private ComponentMovementWatcher
{
    ViewerComponent (CameraDevice& device) : ComponentMovementWatcher (this)
    {
       #if __ANDROID_API__ >= 21
        auto previewSize = device.pimpl->streamConfigurationMap.getDefaultPreviewSize();

        targetAspectRatio = previewSize.getWidth() / (float) previewSize.getHeight();

        if (isOrientationLandscape())
            setBounds (previewSize);
        else
            setBounds (0, 0, previewSize.getHeight(), previewSize.getWidth());

        addAndMakeVisible (viewerComponent);
        viewerComponent.setView (device.pimpl->previewDisplay.getNativeView());
       #else
        ignoreUnused (device);
       #endif
    }

private:
    AndroidViewComponent viewerComponent;

    float targetAspectRatio = 1.0f;

    void componentMovedOrResized (bool, bool) override
    {
        auto b = getLocalBounds();

        auto targetWidth  = b.getWidth();
        auto targetHeight = b.getHeight();

        if (isOrientationLandscape())
        {
            auto currentAspectRatio = b.getWidth() / (float) b.getHeight();

            if (currentAspectRatio > targetAspectRatio)
                targetWidth = static_cast<int> (targetWidth * targetAspectRatio / currentAspectRatio);
            else
                targetHeight = static_cast<int> (targetHeight * currentAspectRatio / targetAspectRatio);
        }
        else
        {
            auto currentAspectRatio = b.getHeight() / (float) b.getWidth();

            if (currentAspectRatio > targetAspectRatio)
                targetHeight = static_cast<int> (targetHeight * targetAspectRatio / currentAspectRatio);
            else
                targetWidth = static_cast<int> (targetWidth * currentAspectRatio / targetAspectRatio);
        }

        viewerComponent.setBounds (Rectangle<int> (0, 0, targetWidth, targetHeight).withCentre (b.getCentre()));
    }

    bool isOrientationLandscape() const
    {
        auto o = Desktop::getInstance().getCurrentOrientation();
        return o == Desktop::rotatedClockwise || o == Desktop::rotatedAntiClockwise;
    }

    void componentPeerChanged() override {}
    void componentVisibilityChanged() override {}

    JUCE_DECLARE_NON_COPYABLE (ViewerComponent)
};

String CameraDevice::getFileExtension()
{
    return ".mp4";
}

#if __ANDROID_API__ >= 21
//==============================================================================
void juce_cameraDeviceStateClosed (int64 host)
{
    reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice*> (host)->cameraDeviceStateClosed();
}

void juce_cameraDeviceStateDisconnected (int64 host)
{
    reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice*> (host)->cameraDeviceStateDisconnected();
}

void juce_cameraDeviceStateError (int64 host, int error)
{
    reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice*> (host)->cameraDeviceStateError (error);
}

void juce_cameraDeviceStateOpened (int64 host, void* camera)
{
    reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice*> (host)->cameraDeviceStateOpened ((jobject) camera);
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraDeviceStateCallback), cameraDeviceStateClosed, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*camera*/))
{
    setEnv (env);

    juce_cameraDeviceStateClosed (host);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraDeviceStateCallback), cameraDeviceStateDisconnected, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*camera*/))
{
    setEnv (env);

    juce_cameraDeviceStateDisconnected (host);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraDeviceStateCallback), cameraDeviceStateError, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject /*camera*/, int error))
{
    setEnv (env);

    juce_cameraDeviceStateError (host, error);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraDeviceStateCallback), cameraDeviceStateOpened, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject camera))
{
    setEnv (env);

    juce_cameraDeviceStateOpened (host, camera);
}

//==============================================================================
void juce_cameraCaptureSessionActive (int64 host, void* session)
{
    auto* juceCaptureSession = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession*> (host);
    juceCaptureSession->cameraCaptureSessionActive ((jobject) session);
}

void juce_cameraCaptureSessionClosed (int64 host, void* session)
{
    auto* juceCaptureSession = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession*> (host);
    juceCaptureSession->cameraCaptureSessionClosed ((jobject) session);
}

void juce_cameraCaptureSessionConfigureFailed (int64 host, void* session)
{
    auto* juceCaptureSession = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession*> (host);
    juceCaptureSession->cameraCaptureSessionConfigureFailed ((jobject) session);
}

void juce_cameraCaptureSessionConfigured (int64 host, void* session)
{
    auto* juceCaptureSession = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession*> (host);
    juceCaptureSession->cameraCaptureSessionConfigured ((jobject) session);
}

void juce_cameraCaptureSessionReady (int64 host, void* session)
{
    auto* juceCaptureSession = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession*> (host);
    juceCaptureSession->cameraCaptureSessionReady ((jobject) session);
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionStateCallback), cameraCaptureSessionActive, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject session))
{
    setEnv (env);

    juce_cameraCaptureSessionActive (host, session);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionStateCallback), cameraCaptureSessionClosed, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject session))
{
    setEnv (env);

    juce_cameraCaptureSessionClosed (host, session);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionStateCallback), cameraCaptureSessionConfigureFailed, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject session))
{
    setEnv (env);

    juce_cameraCaptureSessionConfigureFailed (host, session);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionStateCallback), cameraCaptureSessionConfigured, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject session))
{
    setEnv (env);

    juce_cameraCaptureSessionConfigured (host, session);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionStateCallback), cameraCaptureSessionReady, void, (JNIEnv* env, jobject /*activity*/, jlong host, jobject session))
{
    setEnv (env);

    juce_cameraCaptureSessionReady (host, session);
}


//==============================================================================
void juce_cameraCaptureSessionCaptureCompleted (int64 host, bool isPreview, void* session, void* request, void* result)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureCompleted (isPreview, (jobject) session, (jobject) request, (jobject) result);
}

void juce_cameraCaptureSessionCaptureFailed (int64 host, bool isPreview, void* session, void* request, void* failure)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureFailed (isPreview, (jobject) session, (jobject) request, (jobject) failure);
}

void juce_cameraCaptureSessionCaptureProgressed (int64 host, bool isPreview, void* session, void* request, void* partialResult)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureProgressed (isPreview, (jobject) session, (jobject) request, (jobject) partialResult);
}

void juce_cameraCaptureSessionCaptureSequenceAborted (int64 host, bool isPreview, void* session, int sequenceId)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureSequenceAborted (isPreview, (jobject) session, sequenceId);
}

void juce_cameraCaptureSessionCaptureSequenceCompleted (int64 host, bool isPreview, void* session, int sequenceId, int64 frameNumber)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureSequenceCompleted (isPreview, (jobject) session, sequenceId, frameNumber);
}

void juce_cameraCaptureSessionCaptureStarted (int64 host, bool isPreview, void* session, void* request, int64 timestamp, int64 frameNumber)
{
    auto* stillPictureTaker = reinterpret_cast<CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker*> (host);
    stillPictureTaker->cameraCaptureSessionCaptureStarted (isPreview, (jobject) session, (jobject) request, timestamp, frameNumber);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureCompleted, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jobject request, jobject result))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureCompleted (host, isPreview, session, request, result);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureFailed, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jobject request, jobject failure))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureFailed (host, isPreview, session, request, failure);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureProgressed, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jobject request, jobject partialResult))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureProgressed (host, isPreview, session, request, partialResult);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureSequenceAborted, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jint sequenceId))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureSequenceAborted (host, isPreview, session, (int) sequenceId);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureSequenceCompleted, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jint sequenceId, jlong frameNumber))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureSequenceCompleted (host, isPreview, session, (int) sequenceId, frameNumber);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024CameraCaptureSessionCaptureCallback), cameraCaptureSessionCaptureStarted, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, bool isPreview, jobject session, jobject request, int64 timestamp, int64 frameNumber))
{
    setEnv (env);

    juce_cameraCaptureSessionCaptureStarted (host, isPreview, session, request, timestamp, frameNumber);
}

//==============================================================================
void juce_deviceOrientationChanged (int64 host, int orientation)
{
    auto* listener = reinterpret_cast<CameraDevice::Pimpl::DeviceOrientationChangeListener*> (host);
    listener->orientationChanged (orientation);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024JuceOrientationEventListener), deviceOrientationChanged, \
                   void, (JNIEnv* env, jobject /*activity*/, jlong host, jint orientation))
{
    setEnv (env);

    juce_deviceOrientationChanged (host, (int) orientation);
}
#endif
