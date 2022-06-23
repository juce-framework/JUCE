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

//==============================================================================
// This byte-code is generated from:
//
// native/java/com/rmsl/juce/CameraCaptureSessionCaptureCallback.java
// native/java/com/rmsl/juce/CameraCaptureSessionStateCallback.java
// native/java/com/rmsl/juce/CameraDeviceStateCallback.java
// native/java/com/rmsl/juce/JuceOrientationEventListener.java
//
// files with min sdk version 21
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 CameraSupportByteCode[] =
{31,139,8,8,187,110,161,94,0,3,67,97,109,101,114,97,83,117,112,112,111,114,116,66,121,116,101,67,111,100,101,46,100,101,120,0,
149,152,93,108,28,213,21,199,207,157,157,221,89,239,174,215,227,181,243,217,36,56,95,182,3,113,54,137,227,40,101,29,59,198,113,
192,102,133,169,215,182,34,3,45,147,221,73,188,176,222,89,102,215,139,171,86,37,68,84,208,151,42,80,90,168,4,136,170,20,181,15,
84,110,9,132,7,30,82,21,149,240,0,10,136,190,165,18,84,133,7,154,7,132,42,20,85,60,244,127,63,198,158,93,175,93,99,233,183,
255,51,115,206,61,247,222,115,239,140,103,38,103,47,68,14,246,246,209,213,103,22,174,255,247,133,96,249,25,246,233,195,151,255,
18,189,241,193,31,255,57,248,226,196,252,238,238,4,81,137,136,22,166,143,192,146,127,79,181,18,157,38,121,190,13,92,99,68,91,
160,219,52,162,32,244,174,0,81,55,247,67,117,232,34,126,110,198,136,14,194,249,86,152,232,10,248,2,36,154,136,14,131,62,112,12,
220,7,170,224,50,248,26,116,69,136,190,15,158,4,127,6,255,0,70,148,232,40,248,1,120,22,188,11,190,2,29,200,191,27,116,129,219,
120,95,160,15,164,192,9,112,39,24,7,167,193,131,32,7,242,192,1,85,240,99,240,24,120,18,252,22,188,7,110,128,182,102,162,65,48,7,
158,6,175,129,143,192,87,160,57,78,212,9,70,192,3,160,10,30,3,191,6,47,131,63,128,55,193,219,224,93,240,33,248,12,124,13,162,
45,68,123,193,16,200,128,7,64,9,252,4,252,28,60,7,126,3,46,131,43,224,125,112,29,124,6,254,13,254,3,190,1,97,147,168,21,236,2,
221,166,172,55,95,3,3,160,196,132,50,18,202,70,40,17,97,58,132,97,19,186,39,52,37,132,19,150,146,18,106,29,219,193,6,176,17,
116,42,109,85,107,190,73,217,139,72,188,89,217,111,25,114,221,185,125,5,246,119,148,125,21,246,86,101,127,236,179,175,251,98,
254,5,123,155,178,191,132,189,93,217,55,97,239,80,182,142,9,220,162,236,118,216,29,202,222,227,179,143,248,236,147,176,119,41,
123,18,246,78,101,223,239,59,159,131,189,91,217,5,216,123,148,189,0,123,175,178,159,240,217,23,125,246,11,190,156,191,247,229,
92,12,243,186,50,234,23,245,77,208,184,168,177,60,110,81,106,42,13,168,122,234,74,67,74,99,98,69,120,251,176,210,56,237,19,26,
163,91,133,54,211,109,66,155,104,191,208,8,245,8,141,210,1,161,27,105,80,104,59,157,16,186,129,134,132,182,210,29,106,92,195,66,
219,232,164,24,159,38,250,137,99,183,116,43,77,42,61,168,244,144,210,195,66,77,26,80,58,162,244,148,210,59,149,166,197,124,
101,222,22,204,172,87,233,17,161,6,245,169,227,163,66,55,137,60,92,239,18,186,153,70,213,241,152,168,151,172,144,137,138,29,83,
245,187,155,228,254,101,190,58,50,165,31,155,164,230,35,207,121,245,213,148,38,212,70,110,81,126,175,238,94,158,83,202,111,
42,127,172,110,125,92,229,15,34,51,247,247,154,242,250,42,153,60,126,6,151,208,253,237,12,189,199,224,231,215,222,113,83,230,
200,92,32,154,122,130,145,241,184,241,75,227,85,227,114,213,8,146,63,238,212,170,113,161,154,184,241,85,227,12,17,23,193,76,248,
88,79,155,242,90,207,184,136,171,32,238,17,227,167,236,87,213,80,88,68,201,245,228,227,126,208,203,247,52,226,126,129,184,139,
198,43,236,13,253,111,213,112,147,136,74,160,87,126,223,200,155,178,166,153,63,33,238,117,196,45,26,239,24,127,215,63,15,226,
198,83,141,68,68,172,142,190,121,77,202,166,172,87,201,100,162,38,154,240,105,98,45,126,164,124,153,14,141,74,39,18,164,29,90,
246,93,168,241,181,213,248,126,86,227,107,175,241,93,172,241,109,168,241,61,87,227,219,168,124,114,156,47,45,141,83,19,227,12,
248,198,249,234,82,187,0,218,109,175,201,249,90,141,111,135,240,5,145,147,223,111,47,249,125,147,183,212,180,123,187,166,93,
135,240,133,68,75,162,191,122,99,57,24,160,161,224,76,66,247,141,229,253,165,118,58,218,117,138,118,222,126,103,10,205,119,61,
48,181,219,229,185,160,82,67,249,12,159,47,33,242,115,13,146,252,127,225,217,222,181,33,143,155,136,137,76,203,199,225,154,248,
38,177,71,252,199,81,213,135,55,174,160,178,131,106,140,94,31,242,255,148,204,31,80,118,120,41,70,230,245,236,168,106,99,168,
92,92,67,253,249,98,190,50,64,221,195,214,156,237,90,195,86,169,50,239,218,25,187,92,206,59,69,117,52,108,21,10,103,172,236,195,
7,30,178,170,22,117,54,138,204,84,172,74,93,220,14,25,119,210,174,230,179,118,3,63,27,37,54,70,59,199,230,179,246,184,155,
183,139,136,64,162,145,42,172,116,190,92,177,139,182,43,3,183,164,173,98,206,117,242,185,100,214,41,226,124,37,57,204,117,161,
146,162,193,37,215,172,229,230,30,181,92,59,153,21,189,30,78,54,26,229,158,186,9,165,232,248,183,76,80,51,143,20,37,191,93,243,
20,245,253,191,6,178,92,245,253,116,173,175,89,138,246,173,21,40,134,114,202,202,23,32,235,9,157,176,31,153,183,203,40,115,247,
58,66,203,243,5,68,246,172,30,57,233,84,172,66,93,248,242,188,170,121,251,209,228,106,251,32,69,189,233,172,51,151,116,231,
202,133,228,67,216,48,13,171,187,98,113,15,173,163,81,93,161,247,53,108,210,96,15,167,104,127,93,232,90,27,57,69,108,154,180,
233,81,48,70,129,233,177,81,254,147,38,29,63,194,156,161,32,126,96,135,132,140,145,193,53,61,54,38,79,164,211,104,152,70,32,34,
244,105,238,15,78,11,47,14,224,99,51,20,146,85,166,173,217,6,115,28,202,86,242,85,155,246,52,242,121,69,115,230,74,5,187,98,231,
104,231,26,81,124,239,32,100,239,26,33,247,186,206,57,23,103,16,118,235,26,97,25,190,181,138,89,123,232,140,227,242,94,247,
175,35,118,121,140,187,214,138,174,88,34,99,195,74,12,23,28,62,178,221,13,125,78,241,108,254,220,242,44,119,172,25,148,163,45,
141,252,19,182,149,251,33,109,202,174,216,54,178,227,237,43,28,39,243,101,220,213,138,118,150,143,121,227,10,247,136,235,58,
110,131,124,227,37,108,171,28,25,89,121,39,164,205,57,225,243,237,191,225,89,171,120,14,33,65,91,164,48,206,202,235,158,162,103,
93,36,187,103,126,238,140,237,146,62,235,148,43,212,196,127,39,157,169,178,77,225,165,221,146,104,176,55,90,234,119,66,91,
163,117,223,188,234,42,111,93,99,77,205,21,43,24,94,90,175,214,149,171,19,171,89,139,184,83,172,41,164,129,235,79,76,27,57,84,
169,218,97,173,172,142,225,45,89,212,89,118,82,115,9,35,200,91,5,121,155,34,163,228,218,252,254,68,186,139,210,147,225,202,251,
34,133,92,233,143,148,213,108,70,145,176,44,55,2,181,148,103,157,249,66,238,14,84,70,53,174,204,230,203,212,84,201,207,161,173,
53,87,162,30,109,96,218,136,247,247,244,208,209,192,212,68,198,136,95,162,126,24,41,110,12,195,24,231,198,168,54,53,105,196,
127,71,227,129,169,201,126,126,98,90,159,154,56,13,235,35,218,207,6,120,115,234,101,83,70,124,134,190,43,101,80,202,136,148,187,
165,116,169,208,3,172,155,31,30,145,114,187,214,125,59,215,19,242,176,51,48,112,236,123,70,252,56,15,187,135,159,32,13,15,
207,26,211,244,243,231,245,197,8,187,128,167,188,6,68,217,181,8,99,55,193,203,81,198,174,130,47,193,243,49,98,161,160,166,181,
162,237,59,177,70,237,54,177,79,98,140,125,3,158,111,102,236,18,184,134,151,191,160,30,208,182,161,205,141,230,250,248,157,236,
169,56,99,175,128,43,224,58,30,225,52,166,107,123,31,63,175,223,140,115,127,23,123,182,69,190,215,120,207,117,158,122,223,88,
248,51,143,247,157,133,63,19,121,223,90,188,119,125,254,189,133,171,247,205,37,68,203,223,93,88,135,124,175,231,223,94,152,41,
223,221,249,59,188,214,33,243,243,239,49,1,21,195,223,105,248,203,52,111,43,222,167,76,57,14,254,189,231,127,249,28,121,55,40,18,
0,0,0,0};

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK)    \
 STATICMETHOD (valueOf, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$CompressFormat;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidBitmapCompressFormat, "android/graphics/Bitmap$CompressFormat", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (close,                "close",                "()V") \
 METHOD (createCaptureRequest, "createCaptureRequest", "(I)Landroid/hardware/camera2/CaptureRequest$Builder;") \
 METHOD (createCaptureSession, "createCaptureSession", "(Ljava/util/List;Landroid/hardware/camera2/CameraCaptureSession$StateCallback;Landroid/os/Handler;)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidCameraDevice, "android/hardware/camera2/CameraDevice", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (close,     "close",     "()V") \
 METHOD (getPlanes, "getPlanes", "()[Landroid/media/Image$Plane;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidImage, "android/media/Image", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getBuffer, "getBuffer", "()Ljava/nio/ByteBuffer;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidImagePlane, "android/media/Image$Plane", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (acquireLatestImage,          "acquireLatestImage",          "()Landroid/media/Image;") \
 METHOD (close,                       "close",                       "()V") \
 METHOD (getSurface,                  "getSurface",                  "()Landroid/view/Surface;") \
 METHOD (setOnImageAvailableListener, "setOnImageAvailableListener", "(Landroid/media/ImageReader$OnImageAvailableListener;Landroid/os/Handler;)V") \
 STATICMETHOD (newInstance, "newInstance", "(IIII)Landroid/media/ImageReader;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidImageReader, "android/media/ImageReader", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
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

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidMediaRecorder, "android/media/MediaRecorder", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor,               "<init>",                    "(Landroid/content/Context;)V") \
 METHOD (getSurfaceTexture,         "getSurfaceTexture",         "()Landroid/graphics/SurfaceTexture;") \
 METHOD (isAvailable,               "isAvailable",               "()Z") \
 METHOD (setSurfaceTextureListener, "setSurfaceTextureListener", "(Landroid/view/TextureView$SurfaceTextureListener;)V") \
 METHOD (setTransform,              "setTransform",              "(Landroid/graphics/Matrix;)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidTextureView, "android/view/TextureView", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>", "(Landroid/graphics/SurfaceTexture;)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidSurface, "android/view/Surface", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (setDefaultBufferSize, "setDefaultBufferSize", "(II)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidSurfaceTexture, "android/graphics/SurfaceTexture", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getOutputSizesForClass,      "getOutputSizes",       "(Ljava/lang/Class;)[Landroid/util/Size;") \
 METHOD (getOutputSizesForFormat,     "getOutputSizes",       "(I)[Landroid/util/Size;") \
 METHOD (isOutputSupportedFor,        "isOutputSupportedFor", "(I)Z") \
 METHOD (isOutputSupportedForSurface, "isOutputSupportedFor", "(Landroid/view/Surface;)Z")

DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidStreamConfigurationMap, "android/hardware/camera2/params/StreamConfigurationMap", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>",      "()V") \
 METHOD (toByteArray, "toByteArray", "()[B") \
 METHOD (size,        "size",        "()I")

DECLARE_JNI_CLASS_WITH_MIN_SDK (ByteArrayOutputStream, "java/io/ByteArrayOutputStream", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (abortCaptures,       "abortCaptures",       "()V") \
 METHOD (capture,             "capture",             "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I") \
 METHOD (close,               "close",               "()V") \
 METHOD (setRepeatingRequest, "setRepeatingRequest", "(Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CameraCaptureSession$CaptureCallback;Landroid/os/Handler;)I") \
 METHOD (stopRepeating,       "stopRepeating",       "()V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraCaptureSession, "android/hardware/camera2/CameraCaptureSession", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (get,     "get",     "(Landroid/hardware/camera2/CameraCharacteristics$Key;)Ljava/lang/Object;") \
 METHOD (getKeys, "getKeys", "()Ljava/util/List;") \
 STATICFIELD (CONTROL_AF_AVAILABLE_MODES,      "CONTROL_AF_AVAILABLE_MODES",      "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (LENS_FACING,                     "LENS_FACING",                     "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (SCALER_STREAM_CONFIGURATION_MAP, "SCALER_STREAM_CONFIGURATION_MAP", "Landroid/hardware/camera2/CameraCharacteristics$Key;") \
 STATICFIELD (SENSOR_ORIENTATION,              "SENSOR_ORIENTATION",              "Landroid/hardware/camera2/CameraCharacteristics$Key;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraCharacteristics, "android/hardware/camera2/CameraCharacteristics", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getName, "getName", "()Ljava/lang/String;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraCharacteristicsKey, "android/hardware/camera2/CameraCharacteristics$Key", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getCameraCharacteristics, "getCameraCharacteristics", "(Ljava/lang/String;)Landroid/hardware/camera2/CameraCharacteristics;") \
 METHOD (getCameraIdList,          "getCameraIdList",          "()[Ljava/lang/String;") \
 METHOD (openCamera,               "openCamera",               "(Ljava/lang/String;Landroid/hardware/camera2/CameraDevice$StateCallback;Landroid/os/Handler;)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraManager, "android/hardware/camera2/CameraManager", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICFIELD (CONTROL_AE_PRECAPTURE_TRIGGER, "CONTROL_AE_PRECAPTURE_TRIGGER", "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_AF_MODE,               "CONTROL_AF_MODE",               "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_AF_TRIGGER,            "CONTROL_AF_TRIGGER",            "Landroid/hardware/camera2/CaptureRequest$Key;") \
 STATICFIELD (CONTROL_MODE,                  "CONTROL_MODE",                  "Landroid/hardware/camera2/CaptureRequest$Key;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CaptureRequest, "android/hardware/camera2/CaptureRequest", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (addTarget, "addTarget", "(Landroid/view/Surface;)V") \
 METHOD (build,     "build",     "()Landroid/hardware/camera2/CaptureRequest;") \
 METHOD (set,       "set",       "(Landroid/hardware/camera2/CaptureRequest$Key;Ljava/lang/Object;)V")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CaptureRequestBuilder, "android/hardware/camera2/CaptureRequest$Builder", 21)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (get, "get", "(Landroid/hardware/camera2/CaptureResult$Key;)Ljava/lang/Object;") \
 STATICFIELD (CONTROL_AE_STATE, "CONTROL_AE_STATE", "Landroid/hardware/camera2/CaptureResult$Key;") \
 STATICFIELD (CONTROL_AF_STATE, "CONTROL_AF_STATE", "Landroid/hardware/camera2/CaptureResult$Key;")

DECLARE_JNI_CLASS_WITH_MIN_SDK (CaptureResult, "android/hardware/camera2/CaptureResult", 21)
#undef JNI_CLASS_MEMBERS

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
    : private ActivityLifecycleCallbacks
{
    using InternalOpenCameraResultCallback = std::function<void (const String& /*cameraId*/, const String& /*error*/)>;

    Pimpl (CameraDevice& ownerToUse, const String& cameraIdToUse, int /*index*/,
           int minWidthToUse, int minHeightToUse, int maxWidthToUse, int maxHeightToUse,
           bool /*useHighQuality*/)
        : owner (ownerToUse),
          minWidth (minWidthToUse),
          minHeight (minHeightToUse),
          maxWidth (maxWidthToUse),
          maxHeight (maxHeightToUse),
          cameraId (cameraIdToUse),
          activityLifeListener (CreateJavaInterface (this, "android/app/Application$ActivityLifecycleCallbacks")),
          cameraManager (initialiseCameraManager()),
          cameraCharacteristics (initialiseCameraCharacteristics (cameraManager, cameraId)),
          streamConfigurationMap (cameraCharacteristics),
          previewDisplay (streamConfigurationMap.getPreviewBufferSize()),
          deviceOrientationChangeListener (previewDisplay)
    {
        startBackgroundThread();
    }

    ~Pimpl() override
    {
        auto* env = getEnv();

        env->CallVoidMethod (getAppContext().get(), AndroidApplication.unregisterActivityLifecycleCallbacks, activityLifeListener.get());
        activityLifeListener.clear();
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)

    String getCameraId() const noexcept { return cameraId; }

    void open (InternalOpenCameraResultCallback cameraOpenCallbackToUse)
    {
        cameraOpenCallback = std::move (cameraOpenCallbackToUse);

        // A valid camera open callback must be passed.
        jassert (cameraOpenCallback != nullptr);

        // The same camera can be opened only once!
        jassert (scopedCameraDevice == nullptr);

        if (cameraOpenCallback == nullptr || scopedCameraDevice != nullptr)
            return;

        RuntimePermissions::request (RuntimePermissions::camera,
                                     [safeThis = WeakReference<Pimpl> { this }] (bool granted) mutable
                                     {
                                         if (safeThis != nullptr)
                                             safeThis->continueOpenRequest (granted);
                                     });
    }

    void continueOpenRequest (bool granted)
    {
        if (getAndroidSDKVersion() >= 21)
        {
            if (granted)
            {
                getEnv()->CallVoidMethod (getAppContext().get(), AndroidApplication.registerActivityLifecycleCallbacks, activityLifeListener.get());
                scopedCameraDevice.reset (new ScopedCameraDevice (*this, cameraId, cameraManager, handler, getAutoFocusModeToUse()));
            }
            else
            {
                invokeCameraOpenCallback ("Camera permission not granted");
            }
        }
        else
        {
            invokeCameraOpenCallback ("Camera requires android sdk version 21 or greater");
        }
    }

    bool openedOk() const noexcept { return scopedCameraDevice->openedOk(); }

    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr || currentCaptureSessionMode == nullptr)
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

        pictureTakenCallback = std::move (pictureTakenCallbackToUse);

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
        if (getAndroidSDKVersion() < 21)
            return StringArray(); // Camera requires SDK version 21 or later

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
        return LocalRef<jobject> (getEnv()->CallObjectMethod (getAppContext().get(), AndroidContext.getSystemService,
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

        for (int i = 0; i < size; ++i)
        {
            auto key = LocalRef<jobject> (env->CallObjectMethod (keysList, JavaList.get, i));
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
                printRangeArrayElements (keyValue, keyName);
            }
            else
            {
                int chunkSize = 256;

                if (keyValueString.length() > chunkSize)
                {
                    JUCE_CAMERA_LOG ("Key: " + keyName);

                    for (int j = 0, k = 1; j < keyValueString.length(); j += chunkSize, ++k)
                        JUCE_CAMERA_LOG ("value part " + String (k) + ": " + keyValueString.substring (j, k + chunkSize));
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

        #define PRINT_ELEMENTS(elem_type, array_type, fun_name_middle)                                              \
        {                                                                                                           \
            elem_type* elements = env->Get##fun_name_middle##ArrayElements ((array_type) keyValue.get(), nullptr);  \
            int size = env->GetArrayLength ((array_type) keyValue.get());                                           \
                                                                                                                    \
            for (int i = 0; i < size - 1; ++i)                                                                      \
                result << String (elements[i]) << " ";                                                              \
                                                                                                                    \
            if (size > 0)                                                                                           \
                result << String (elements[size - 1]);                                                              \
                                                                                                                    \
            env->Release##fun_name_middle##ArrayElements ((array_type) keyValue.get(), elements, 0);                \
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

    static void printRangeArrayElements (const LocalRef<jobject>& rangeArray, const String& keyName)
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

        ignoreUnused (keyName);
        JUCE_CAMERA_LOG ("Key: " + keyName + ", value: " + result);
    }

    //==============================================================================
    class StreamConfigurationMap
    {
    public:
        StreamConfigurationMap (const GlobalRef& cameraCharacteristicsToUse)
            : scalerStreamConfigurationMap (getStreamConfigurationMap (cameraCharacteristicsToUse)),
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

        GlobalRef getStreamConfigurationMap (const GlobalRef& cameraCharacteristicsToUse)
        {
            auto* env = getEnv();

            auto scalerStreamConfigurationMapKey = LocalRef<jobject> (env->GetStaticObjectField (CameraCharacteristics,
                                                                                                 CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP));

            return GlobalRef (LocalRef<jobject> (env->CallObjectMethod (cameraCharacteristicsToUse,
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
              textureView (LocalRef<jobject> (getEnv()->NewObject (AndroidTextureView, AndroidTextureView.constructor,
                                                                   getAppContext().get()))),
              bufferWidth (bufferSize.getWidth()),
              bufferHeight (bufferSize.getHeight())
        {
            auto* env = getEnv();

            if (! isReady())
                env->CallVoidMethod (textureView, AndroidTextureView.setSurfaceTextureListener,
                                     CreateJavaInterface (&textureViewSurfaceTextureListener,
                                                          "android/view/TextureView$SurfaceTextureListener").get());
        }

        ~PreviewDisplay() override
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

            auto windowManager = LocalRef<jobject> (env->CallObjectMethod (getAppContext(), AndroidContext.getSystemService, javaString ("window").get()));
            auto display = LocalRef<jobject> (env->CallObjectMethod (windowManager, AndroidWindowManager.getDefaultDisplay));
            auto rotation = env->CallIntMethod (display, AndroidDisplay.getRotation);

            static constexpr int rotation90 = 1;
            static constexpr int rotation270 = 3;

            auto matrix = LocalRef<jobject> (env->NewObject (AndroidMatrix, AndroidMatrix.constructor));

            if (rotation == rotation90 || rotation == rotation270)
            {
                env->CallBooleanMethod (matrix, AndroidMatrix.postScale, jfloat ((float) height / (float) width), jfloat ((float) width / (float) height), (jfloat) 0, (jfloat) 0);
                env->CallBooleanMethod (matrix, AndroidMatrix.postRotate, (jfloat) 90 * ((float) rotation - 2), (jfloat) 0, (jfloat) 0);
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
              imageReader (LocalRef<jobject> (getEnv()->CallStaticObjectMethod (AndroidImageReader, AndroidImageReader.newInstance,
                                                                                imageWidth, imageHeight, StreamConfigurationMap::jpegImageFormat,
                                                                                numImagesToKeep))),
              onImageAvailableListener (*this)
        {
            getEnv()->CallVoidMethod (imageReader, AndroidImageReader.setOnImageAvailableListener,
                                      CreateJavaInterface (&onImageAvailableListener,
                                                           "android/media/ImageReader$OnImageAvailableListener").get(),
                                      handlerToUse.get());
        }

        ~ImageReader() override
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

            owner.callListeners (image);

            // Android may take multiple pictures before it handles a request to stop.
            if (hasNotifiedListeners.compareAndSetBool (1, 0))
                MessageManager::callAsync ([safeThis = WeakReference<ImageReader> { this }, image]() mutable
                {
                    if (safeThis != nullptr)
                        safeThis->owner.notifyPictureTaken (image);
                });
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

            if (origBitmap == nullptr)
            {
                // Nothing to do, just get the bytes
                return { byteArray, bufferSize };
            }

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
                case Desktop::allOrientations:
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

        ~MediaRecorder() override
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

            auto windowManager = LocalRef<jobject> (env->CallObjectMethod (getAppContext(), AndroidContext.getSystemService, javaString ("window").get()));
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
                // it is caught internally in Java...
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

                previewCaptureRequest = GlobalRef (LocalRef<jobject>(env->CallObjectMethod (captureRequestBuilder, CaptureRequestBuilder.build)));

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

                setCaptureRequestBuilderIntegerKey (builder, CaptureRequest.CONTROL_AF_MODE, autoFocusMode);

                auto stillPictureCaptureRequest = LocalRef<jobject> (env->CallObjectMethod (builder, CaptureRequestBuilder.build));

                stillPictureTaker->takePicture (stillPictureCaptureRequest);
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
                      captureSessionPreviewCaptureCallback (createCaptureSessionCallback (true)),
                      captureSessionStillPictureCaptureCallback (createCaptureSessionCallback (false)),
                      autoFocusMode (autoFocusModeToUse)
                {
                }

                void takePicture (const LocalRef<jobject>& stillPictureCaptureRequestToUse)
                {
                    JUCE_CAMERA_LOG ("Taking picture...");

                    stillPictureCaptureRequest = GlobalRef (LocalRef<jobject>(stillPictureCaptureRequestToUse));

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

                //==============================================================================
                #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
                   METHOD (constructor, "<init>", "(JZ)V")              \
                   CALLBACK (cameraCaptureSessionCaptureCompletedCallback,         "cameraCaptureSessionCaptureCompleted",         "(JZLandroid/hardware/camera2/CameraCaptureSession;Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/TotalCaptureResult;)V") \
                   CALLBACK (cameraCaptureSessionCaptureFailedCallback,            "cameraCaptureSessionCaptureFailed",            "(JZLandroid/hardware/camera2/CameraCaptureSession;Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CaptureFailure;)V") \
                   CALLBACK (cameraCaptureSessionCaptureProgressedCallback,        "cameraCaptureSessionCaptureProgressed",        "(JZLandroid/hardware/camera2/CameraCaptureSession;Landroid/hardware/camera2/CaptureRequest;Landroid/hardware/camera2/CaptureResult;)V") \
                   CALLBACK (cameraCaptureSessionCaptureStartedCallback,           "cameraCaptureSessionCaptureStarted",           "(JZLandroid/hardware/camera2/CameraCaptureSession;Landroid/hardware/camera2/CaptureRequest;JJ)V") \
                   CALLBACK (cameraCaptureSessionCaptureSequenceAbortedCallback,   "cameraCaptureSessionCaptureSequenceAborted",   "(JZLandroid/hardware/camera2/CameraCaptureSession;I)V") \
                   CALLBACK (cameraCaptureSessionCaptureSequenceCompletedCallback, "cameraCaptureSessionCaptureSequenceCompleted", "(JZLandroid/hardware/camera2/CameraCaptureSession;IJ)V")

                DECLARE_JNI_CLASS_WITH_BYTECODE (CameraCaptureSessionCaptureCallback, "com/rmsl/juce/CameraCaptureSessionCaptureCallback", 21, CameraSupportByteCode, sizeof(CameraSupportByteCode))
                #undef JNI_CLASS_MEMBERS

                LocalRef<jobject> createCaptureSessionCallback (bool createPreviewSession)
                {
                    return LocalRef<jobject>(getEnv()->NewObject (CameraCaptureSessionCaptureCallback,
                                                                  CameraCaptureSessionCaptureCallback.constructor,
                                                                  reinterpret_cast<jlong> (this),
                                                                  createPreviewSession ? 1 : 0));
                }

                //==============================================================================

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
                        delayedCaptureRunnable = GlobalRef (CreateJavaInterface (&runnable, "java/lang/Runnable"));

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

                //==============================================================================
                static void cameraCaptureSessionCaptureCompletedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jobject rawRequest, jobject rawResult)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));
                        LocalRef<jobject> request (getEnv()->NewLocalRef(rawRequest));
                        LocalRef<jobject> result (getEnv()->NewLocalRef(rawResult));

                        myself->cameraCaptureSessionCaptureCompleted (isPreview != 0, session, request, result);
                    }
                }

                static void cameraCaptureSessionCaptureFailedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jobject rawRequest, jobject rawResult)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));
                        LocalRef<jobject> request (getEnv()->NewLocalRef(rawRequest));
                        LocalRef<jobject> result (getEnv()->NewLocalRef(rawResult));

                        myself->cameraCaptureSessionCaptureFailed (isPreview != 0, session, request, result);
                    }
                }

                static void cameraCaptureSessionCaptureProgressedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jobject rawRequest, jobject rawResult)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));
                        LocalRef<jobject> request (getEnv()->NewLocalRef(rawRequest));
                        LocalRef<jobject> result (getEnv()->NewLocalRef(rawResult));

                        myself->cameraCaptureSessionCaptureProgressed (isPreview != 0, session, request, result);
                    }
                }

                static void cameraCaptureSessionCaptureSequenceAbortedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jint sequenceId)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                        myself->cameraCaptureSessionCaptureSequenceAborted (isPreview != 0, session, sequenceId);
                    }
                }

                static void cameraCaptureSessionCaptureSequenceCompletedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jint sequenceId, jlong frameNumber)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                        myself->cameraCaptureSessionCaptureSequenceCompleted (isPreview != 0, session, sequenceId, frameNumber);
                    }
                }

                static void cameraCaptureSessionCaptureStartedCallback (JNIEnv*, jobject /*object*/, jlong host, jboolean isPreview, jobject rawSession, jobject rawRequest, jlong timestamp, jlong frameNumber)
                {
                    if (auto* myself = reinterpret_cast<StillPictureTaker*> (host))
                    {
                        LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));
                        LocalRef<jobject> request (getEnv()->NewLocalRef(rawRequest));

                        myself->cameraCaptureSessionCaptureStarted (isPreview != 0, session, request, timestamp, frameNumber);
                    }
                }
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
            #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK)    \
                METHOD (constructor, "<init>", "(J)V") \
                CALLBACK(cameraCaptureSessionActiveCallback,          "cameraCaptureSessionActive",          "(JLandroid/hardware/camera2/CameraCaptureSession;)V") \
                CALLBACK(cameraCaptureSessionClosedCallback,          "cameraCaptureSessionClosed",          "(JLandroid/hardware/camera2/CameraCaptureSession;)V") \
                CALLBACK(cameraCaptureSessionConfigureFailedCallback, "cameraCaptureSessionConfigureFailed", "(JLandroid/hardware/camera2/CameraCaptureSession;)V") \
                CALLBACK(cameraCaptureSessionConfiguredCallback,      "cameraCaptureSessionConfigured",      "(JLandroid/hardware/camera2/CameraCaptureSession;)V") \
                CALLBACK(cameraCaptureSessionReadyCallback,           "cameraCaptureSessionReady",           "(JLandroid/hardware/camera2/CameraCaptureSession;)V")

            DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraCaptureSessionStateCallback, "com/rmsl/juce/CameraCaptureSessionStateCallback", 21)
            #undef JNI_CLASS_MEMBERS


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

                MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }]
                {
                    if (weakRef != nullptr)
                        weakRef->configuredCallback.captureSessionConfigured (nullptr);
                });
            }

            void cameraCaptureSessionConfigured (const LocalRef<jobject>& session)
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

                MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }]
                {
                    if (weakRef == nullptr)
                        return;

                    weakRef->stillPictureTaker.reset (new StillPictureTaker (weakRef->captureSession,
                                                                             weakRef->captureRequestBuilder,
                                                                             weakRef->previewCaptureRequest,
                                                                             weakRef->handler,
                                                                             weakRef->autoFocusMode));

                    weakRef->configuredCallback.captureSessionConfigured (weakRef.get());
                });
            }

            void cameraCaptureSessionReady (const LocalRef<jobject>& session)
            {
                JUCE_CAMERA_LOG ("cameraCaptureSessionReady()");
                ignoreUnused (session);
            }

            //==============================================================================
            static void cameraCaptureSessionActiveCallback (JNIEnv*, jobject, jlong host, jobject rawSession)
            {
                if (auto* myself = reinterpret_cast<CaptureSession*> (host))
                {
                    LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                    myself->cameraCaptureSessionActive (session);
                }
            }

            static void cameraCaptureSessionClosedCallback (JNIEnv*, jobject, jlong host, jobject rawSession)
            {
                if (auto* myself = reinterpret_cast<CaptureSession*> (host))
                {
                    LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                    myself->cameraCaptureSessionClosed (session);
                }
            }

            static void cameraCaptureSessionConfigureFailedCallback (JNIEnv*, jobject, jlong host, jobject rawSession)
            {
                if (auto* myself = reinterpret_cast<CaptureSession*> (host))
                {
                    LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                    myself->cameraCaptureSessionConfigureFailed (session);
                }
            }

            static void cameraCaptureSessionConfiguredCallback (JNIEnv*, jobject, jlong host, jobject rawSession)
            {
                if (auto* myself = reinterpret_cast<CaptureSession*> (host))
                {
                    LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                    myself->cameraCaptureSessionConfigured (session);
                }
            }

            static void cameraCaptureSessionReadyCallback (JNIEnv*, jobject, jlong host, jobject rawSession)
            {
                if (auto* myself = reinterpret_cast<CaptureSession*> (host))
                {
                    LocalRef<jobject> session (getEnv()->NewLocalRef(rawSession));

                    myself->cameraCaptureSessionReady (session);
                }
            }

            //==============================================================================
            friend class ScopedCameraDevice;

            JUCE_DECLARE_NON_COPYABLE (CaptureSession)
        };

        //==============================================================================
        ScopedCameraDevice (Pimpl& ownerToUse, const String& cameraIdToUse, GlobalRef& cameraManagerToUse,
                            GlobalRef& handlerToUse, int autoFocusModeToUse)
            : owner (ownerToUse),
              cameraId (cameraIdToUse),
              cameraManager (cameraManagerToUse),
              handler (handlerToUse),
              cameraStateCallback (createCameraStateCallbackObject()),
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

        //==============================================================================
        #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK)    \
            METHOD (constructor, "<init>", "(J)V") \
            CALLBACK (cameraDeviceStateClosedCallback,       "cameraDeviceStateClosed",       "(JLandroid/hardware/camera2/CameraDevice;)V")  \
            CALLBACK (cameraDeviceStateDisconnectedCallback, "cameraDeviceStateDisconnected", "(JLandroid/hardware/camera2/CameraDevice;)V")  \
            CALLBACK (cameraDeviceStateErrorCallback,        "cameraDeviceStateError",        "(JLandroid/hardware/camera2/CameraDevice;I)V") \
            CALLBACK (cameraDeviceStateOpenedCallback,       "cameraDeviceStateOpened",       "(JLandroid/hardware/camera2/CameraDevice;)V")

        DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraDeviceStateCallback, "com/rmsl/juce/CameraDeviceStateCallback", 21)
        #undef JNI_CLASS_MEMBERS

        LocalRef<jobject> createCameraStateCallbackObject()
        {
            return LocalRef<jobject> (getEnv()->NewObject (CameraDeviceStateCallback,
                                                           CameraDeviceStateCallback.constructor,
                                                           reinterpret_cast<jlong> (this)));
        }

        //==============================================================================
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

        void cameraDeviceStateOpened (const LocalRef<jobject>& cameraDeviceToUse)
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

        //==============================================================================
        static void JNICALL cameraDeviceStateClosedCallback (JNIEnv*, jobject, jlong host, jobject)
        {
            if (auto* myself = reinterpret_cast<ScopedCameraDevice*>(host))
                myself->cameraDeviceStateClosed();
        }

        static void JNICALL cameraDeviceStateDisconnectedCallback (JNIEnv*, jobject, jlong host, jobject)
        {
            if (auto* myself = reinterpret_cast<ScopedCameraDevice*>(host))
                myself->cameraDeviceStateDisconnected();
        }

        static void JNICALL cameraDeviceStateErrorCallback (JNIEnv*, jobject, jlong host, jobject, jint error)
        {
            if (auto* myself = reinterpret_cast<ScopedCameraDevice*>(host))
                myself->cameraDeviceStateError (error);
        }

        static void JNICALL cameraDeviceStateOpenedCallback (JNIEnv*, jobject, jlong host, jobject rawCamera)
        {
            if (auto* myself = reinterpret_cast<ScopedCameraDevice*>(host))
            {
                LocalRef<jobject> camera(getEnv()->NewLocalRef(rawCamera));

                myself->cameraDeviceStateOpened (camera);
            }
        }
    };

    //==============================================================================
    struct CaptureSessionModeBase
    {
        virtual ~CaptureSessionModeBase() = default;

        virtual bool isVideoRecordSession() const = 0;

        virtual void triggerStillPictureCapture() = 0;
    };

    //==============================================================================
    template <typename Mode>
    struct CaptureSessionMode   : public CaptureSessionModeBase,
                                  private PreviewDisplay::Listener,
                                  private ScopedCameraDevice::CaptureSession::ConfiguredCallback
    {
        ~CaptureSessionMode() override
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
            // async so that the object is fully constructed before the callback gets invoked
            MessageManager::callAsync ([weakRef = WeakReference<CaptureSessionMode<Mode>> { this }]
            {
                if (weakRef != nullptr)
                    weakRef->previewDisplay.addListener (weakRef.get());
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
                                   PreviewDisplay& pd, ImageReader& ir, int sensorOrientation,
                                   int cameraLensFacingToUse, StreamConfigurationMap& streamConfigurationMapToUse)
            : CaptureSessionMode<CaptureSessionPreviewMode> (ownerToUse, cameraDeviceToUse, handlerToUse, pd,
                                                             sensorOrientation, cameraLensFacingToUse, streamConfigurationMapToUse),
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
                                          PreviewDisplay& pd, MediaRecorder& mr, int sensorOrientation,
                                          int cameraLensFacingToUse, StreamConfigurationMap& streamConfigurationMapToUse)
            : CaptureSessionMode<CaptureSessionVideoRecordingMode> (ownerToUse, cameraDeviceToUse, handlerToUse, pd,
                                                                    sensorOrientation, cameraLensFacingToUse, streamConfigurationMapToUse),
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
              orientationEventListener (createOrientationEventListener()),
              canDetectChange (getEnv()->CallBooleanMethod (orientationEventListener,
                                                            OrientationEventListener.canDetectOrientation) != 0),
              deviceOrientation (Desktop::getInstance().getCurrentOrientation()),
              lastKnownScreenOrientation (deviceOrientation)
        {
            setEnabled (true);
        }

        ~DeviceOrientationChangeListener() override
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

        //==============================================================================
        #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
            METHOD (canDetectOrientation, "canDetectOrientation", "()Z") \
            METHOD (constructor,          "<init>",               "(JLandroid/content/Context;I)V") \
            METHOD (disable,              "disable",              "()V") \
            METHOD (enable,               "enable",               "()V") \
            CALLBACK (deviceOrientationChanged, "deviceOrientationChanged", "(JI)V")

        DECLARE_JNI_CLASS_WITH_MIN_SDK (OrientationEventListener, "com/rmsl/juce/JuceOrientationEventListener", 21)
        #undef JNI_CLASS_MEMBERS

        LocalRef<jobject> createOrientationEventListener()
        {
            return LocalRef<jobject> (getEnv()->NewObject (OrientationEventListener,
                                                           OrientationEventListener.constructor,
                                                           reinterpret_cast<jlong> (this),
                                                           getAppContext().get(),
                                                           sensorDelayUI));
        }

        //==============================================================================
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

        static void deviceOrientationChanged (JNIEnv*, jobject /*obj*/, jlong host, jint orientation)
        {
            if (auto* myself = reinterpret_cast<DeviceOrientationChangeListener*> (host))
                myself->orientationChanged (orientation);
        }
    };

    //==============================================================================
    CameraDevice& owner;
    int minWidth, minHeight, maxWidth, maxHeight;

    String cameraId;
    InternalOpenCameraResultCallback cameraOpenCallback;

    GlobalRef activityLifeListener;

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

        auto* jArrayElems = env->GetIntArrayElements (jArray, nullptr);
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
    void onActivityPaused (jobject) override
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

    void onActivityResumed (jobject) override
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

        auto quitSafelyMethod = env->GetMethodID(AndroidHandlerThread, "quitSafely", "()Z");

        // this code will only run on SDK >= 21
        jassert(quitSafelyMethod != nullptr);

        env->CallBooleanMethod (handlerThread, quitSafelyMethod);
        env->CallVoidMethod (handlerThread, AndroidHandlerThread.join);

        jniCheckHasExceptionOccurredAndClear();

        handlerThread.clear();
        handler.clear();
    }

    friend struct CameraDevice::ViewerComponent;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public Component,
                                        private ComponentMovementWatcher
{
    ViewerComponent (CameraDevice& device) : ComponentMovementWatcher (this)
    {
        auto previewSize = device.pimpl->streamConfigurationMap.getDefaultPreviewSize();

        targetAspectRatio = (float) previewSize.getWidth() / (float) previewSize.getHeight();

        if (isOrientationLandscape())
            setBounds (previewSize);
        else
            setBounds (0, 0, previewSize.getHeight(), previewSize.getWidth());

        addAndMakeVisible (viewerComponent);
        viewerComponent.setView (device.pimpl->previewDisplay.getNativeView());
    }

private:
    AndroidViewComponent viewerComponent;

    float targetAspectRatio = 1.0f;

    void componentMovedOrResized (bool, bool) override
    {
        auto b = getLocalBounds().toFloat();

        auto targetWidth  = b.getWidth();
        auto targetHeight = b.getHeight();

        if (isOrientationLandscape())
        {
            auto currentAspectRatio = b.getWidth() / b.getHeight();

            if (currentAspectRatio > targetAspectRatio)
                targetWidth = targetWidth * targetAspectRatio / currentAspectRatio;
            else
                targetHeight = targetHeight * currentAspectRatio / targetAspectRatio;
        }
        else
        {
            auto currentAspectRatio = b.getHeight() / b.getWidth();

            if (currentAspectRatio > targetAspectRatio)
                targetHeight = targetHeight * targetAspectRatio / currentAspectRatio;
            else
                targetWidth = targetWidth * currentAspectRatio / targetAspectRatio;
        }

        viewerComponent.setBounds (Rectangle<float> (targetWidth, targetHeight).withCentre (b.getCentre()).toNearestInt());
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

//==============================================================================
CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker::CameraCaptureSessionCaptureCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker::CameraCaptureSessionCaptureCallback;
CameraDevice::Pimpl::ScopedCameraDevice::CameraDeviceStateCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CameraDeviceStateCallback;
CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::CameraCaptureSessionStateCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::CameraCaptureSessionStateCallback;
CameraDevice::Pimpl::DeviceOrientationChangeListener::OrientationEventListener_Class CameraDevice::Pimpl::DeviceOrientationChangeListener::OrientationEventListener;
