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

//==============================================================================
// This byte-code is generated from:
//
// native/java/com/roli/juce/CameraCaptureSessionCaptureCallback.java
// native/java/com/roli/juce/CameraCaptureSessionStateCallback.java
// native/java/com/roli/juce/CameraDeviceStateCallback.java
// native/java/com/roli/juce/JuceOrientationEventListener.java
//
// files with min sdk version 21
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 CameraSupportByteCode[] = {
31,139,8,8,45,45,227,91,0,3,67,97,109,101,114,97,83,117,112,112,111,114,116,46,100,101,120,0,149,152,93,108,28,213,21,
199,207,157,157,221,89,239,174,215,227,181,243,73,18,236,124,216,14,196,108,190,140,162,174,227,196,56,9,216,93,197,169,
215,118,35,3,45,147,221,73,60,116,189,179,204,174,151,84,173,74,136,168,8,47,85,80,105,1,9,33,16,1,209,7,36,183,4,194,3,
15,169,138,10,85,133,148,34,170,246,33,149,120,40,60,180,84,138,80,133,242,192,67,255,247,99,236,217,245,218,53,150,126,
251,63,51,231,220,115,239,61,247,206,120,102,10,246,249,216,222,3,3,244,159,203,87,222,62,245,197,223,255,124,233,193,91,
39,95,59,122,248,179,137,191,189,250,253,83,157,191,127,189,47,69,84,38,162,243,211,7,97,201,191,75,237,68,167,73,158,
239,0,55,24,209,38,232,22,141,40,12,125,32,68,212,199,253,80,29,186,128,159,219,9,162,189,112,190,23,37,186,14,254,5,82,
45,68,251,193,0,56,4,30,4,53,112,13,124,13,122,99,68,63,0,79,131,223,129,127,0,35,78,116,47,248,33,120,14,124,8,190,2,93,
200,191,3,244,130,187,121,95,96,0,100,192,81,112,63,24,7,167,193,35,160,0,28,224,130,26,248,41,120,2,60,13,94,3,127,2,95,
130,142,86,162,35,96,14,60,11,222,2,159,128,175,64,107,146,168,7,28,7,15,131,26,120,2,188,8,94,1,191,1,239,130,247,193,
135,224,47,224,115,240,53,136,183,17,237,2,195,32,7,30,6,101,240,51,240,11,240,60,120,21,92,3,215,193,199,224,38,248,28,
252,27,252,23,124,3,162,38,81,59,216,14,250,76,89,111,190,6,6,64,137,9,101,36,148,141,80,34,194,116,8,195,38,116,79,104,
74,8,39,44,37,165,212,58,118,130,117,96,61,232,81,218,174,214,124,131,178,23,144,120,163,178,223,51,228,186,115,251,58,
236,59,148,253,17,236,205,202,254,52,96,223,12,196,252,19,246,22,101,223,130,189,85,217,183,97,111,83,182,142,9,220,169,
236,78,216,93,202,222,25,176,15,6,236,99,176,183,43,123,18,118,183,178,31,10,156,47,192,222,161,236,34,236,157,202,62,15,
123,151,178,159,10,216,151,3,246,75,129,156,111,6,114,46,68,121,93,25,13,138,250,166,104,92,212,88,30,183,41,53,149,134,
84,61,117,165,17,165,9,177,34,188,125,84,105,146,118,11,77,208,93,66,91,233,110,161,45,180,71,104,140,250,133,198,233,30,
161,235,233,136,208,78,58,42,116,29,13,11,109,167,251,212,184,70,132,118,208,49,49,62,77,244,147,196,110,233,83,154,86,
186,87,233,62,165,251,133,154,52,164,244,184,210,19,74,239,87,154,21,243,149,121,219,48,179,3,74,15,10,53,104,64,29,223,
43,116,131,200,195,245,1,161,27,105,84,29,143,137,122,201,10,153,168,216,33,85,191,239,146,220,191,44,80,71,166,244,83,
147,212,124,228,57,191,190,154,210,148,218,200,109,202,239,215,221,207,115,66,249,77,229,79,52,172,143,167,252,97,100,
230,254,3,166,188,190,202,38,143,159,193,37,244,80,39,67,239,9,248,249,181,119,216,148,57,114,23,137,166,158,98,100,60,
105,252,202,120,195,184,86,51,194,20,140,59,177,98,92,164,46,110,124,197,56,67,196,197,48,19,62,214,211,166,188,214,115,
30,226,170,136,123,204,248,57,251,117,45,18,21,81,114,61,249,184,31,241,243,61,139,184,95,34,238,178,113,133,189,163,255,
177,22,109,17,81,41,244,202,239,27,142,41,107,154,251,45,226,222,70,220,130,241,129,241,87,253,139,48,110,60,181,88,76,
196,234,232,155,215,164,98,202,122,149,77,38,106,162,9,159,38,214,226,39,202,151,235,210,168,124,52,69,218,190,37,223,
197,58,95,71,157,239,153,58,95,103,157,239,114,157,111,93,157,239,249,58,223,122,229,147,227,124,121,113,156,154,24,103,
40,48,206,55,22,219,133,208,110,107,93,206,183,234,124,219,132,47,140,156,252,126,123,53,232,155,188,179,174,221,251,117,
237,186,132,47,34,90,18,253,193,31,203,222,16,13,135,103,82,122,96,44,31,47,182,211,209,174,71,180,243,247,59,83,104,129,
235,129,169,221,46,207,133,149,26,202,103,4,124,41,145,159,107,152,228,255,11,223,246,175,13,121,220,66,76,100,90,58,142,
214,197,183,136,61,18,60,142,171,62,252,113,133,149,29,86,99,244,251,144,255,167,100,254,144,178,163,139,49,50,175,111,
199,85,27,67,229,226,26,25,116,74,78,117,136,250,70,172,57,219,179,70,172,114,117,222,179,115,118,165,226,184,37,117,52,
98,21,139,103,172,252,143,238,121,212,170,89,212,211,44,50,87,181,170,13,113,219,100,220,49,187,230,228,237,38,126,54,74,
108,140,186,199,230,243,246,184,231,216,37,68,32,209,241,26,172,172,83,169,218,37,219,147,129,155,178,86,169,224,185,78,
33,157,119,75,56,95,77,143,112,61,95,205,208,145,69,215,172,229,21,30,183,60,59,157,23,189,238,79,55,27,229,206,134,9,
101,232,240,183,76,80,55,143,12,165,191,93,243,12,13,252,191,6,178,92,141,253,244,174,173,89,134,118,175,22,40,134,114,
194,114,138,144,181,132,78,216,143,205,219,21,148,185,111,13,161,149,249,34,34,251,87,142,156,116,171,86,177,33,124,105,
94,53,199,126,60,189,210,62,200,208,129,108,222,157,75,123,110,209,73,63,138,13,211,180,186,203,22,119,223,26,26,53,20,
122,119,211,38,77,246,112,134,246,52,132,174,182,145,51,196,166,73,155,30,5,99,20,154,30,27,229,63,89,210,241,35,204,25,
10,227,7,118,68,200,24,25,92,179,99,99,242,68,54,139,134,89,4,34,66,159,230,254,240,180,240,226,0,62,54,67,17,89,101,218,
156,111,50,199,225,124,213,169,217,180,179,153,207,47,154,59,87,46,218,85,187,64,221,171,68,241,189,131,144,93,171,132,
156,242,220,115,30,206,32,236,174,85,194,114,124,107,149,242,246,240,25,215,227,189,238,89,67,236,210,24,183,175,22,93,
181,68,198,166,149,24,41,186,124,100,59,154,250,220,210,89,231,220,210,44,183,173,26,84,160,77,205,252,19,182,85,248,49,
109,200,47,219,54,178,227,173,203,28,199,156,10,238,106,37,59,207,199,188,126,153,251,184,231,185,94,147,124,227,101,108,
171,2,25,121,121,39,164,141,5,225,11,236,191,145,89,171,116,14,33,97,91,164,48,206,202,235,158,226,103,61,36,59,57,63,
119,198,246,72,159,117,43,85,106,225,191,147,238,84,197,166,232,226,110,73,53,217,27,109,141,59,161,163,217,186,111,92,
113,149,55,175,178,166,230,178,21,140,46,174,87,251,242,213,73,212,173,69,210,45,213,21,210,192,245,39,166,141,28,170,84,
157,176,150,87,199,240,151,44,238,46,57,169,181,140,17,56,86,81,222,166,200,40,123,54,191,63,145,238,161,244,100,120,242,
190,72,17,79,250,99,21,53,155,81,36,172,200,141,64,109,149,89,119,190,88,184,15,149,81,141,171,179,78,133,90,170,206,28,
218,90,115,101,234,208,134,166,141,228,96,127,63,109,9,77,77,228,140,228,85,218,14,35,195,141,221,48,198,185,145,214,166,
38,141,228,235,52,16,154,154,28,228,39,6,245,169,137,211,176,62,161,36,27,226,205,105,29,155,50,146,51,116,135,148,110,
41,189,82,250,165,180,170,208,78,214,199,15,55,75,233,210,250,190,195,181,71,30,38,66,67,135,190,103,36,15,243,176,147,
252,4,105,120,120,214,152,166,95,184,160,47,196,216,69,60,229,53,33,206,110,196,24,187,13,94,137,51,246,17,184,5,94,72,
16,139,132,53,173,29,109,63,72,52,107,183,129,125,150,96,236,27,240,66,43,99,87,193,13,188,252,133,245,144,182,5,109,190,
108,109,140,239,102,151,146,140,93,1,215,193,77,60,194,105,76,215,118,61,121,65,191,157,228,254,94,246,92,155,124,175,
241,159,235,124,245,191,177,240,103,30,255,59,11,127,38,242,191,181,248,239,250,252,123,11,87,255,155,75,132,150,190,187,
176,46,249,94,207,191,189,48,83,190,187,243,119,120,173,75,230,231,223,99,66,42,134,191,211,240,151,105,222,86,188,79,
153,114,28,252,123,207,255,0,68,14,12,167,40,18,0,0};

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

    ~Pimpl()
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

        WeakReference<Pimpl> safeThis (this);
        RuntimePermissions::request (RuntimePermissions::camera, [safeThis] (bool granted) mutable
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

            auto windowManager = LocalRef<jobject> (env->CallObjectMethod (getAppContext(), AndroidContext.getSystemService, javaString ("window").get()));
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

                DECLARE_JNI_CLASS_WITH_BYTECODE (CameraCaptureSessionCaptureCallback, "com/roli/juce/CameraCaptureSessionCaptureCallback", 21, CameraSupportByteCode, sizeof(CameraSupportByteCode))
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

            DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraCaptureSessionStateCallback, "com/roli/juce/CameraCaptureSessionStateCallback", 21)
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

                WeakReference<CaptureSession> weakRef (this);

                MessageManager::callAsync ([this, weakRef]()
                {
                    if (weakRef == nullptr)
                        return;

                    configuredCallback.captureSessionConfigured (nullptr);
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

        DECLARE_JNI_CLASS_WITH_MIN_SDK (CameraDeviceStateCallback, "com/roli/juce/CameraDeviceStateCallback", 21)
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
              orientationEventListener (createOrientationEventListener()),
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

        //==============================================================================
        #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
            METHOD (canDetectOrientation, "canDetectOrientation", "()Z") \
            METHOD (constructor,          "<init>",               "(JLandroid/content/Context;I)V") \
            METHOD (disable,              "disable",              "()V") \
            METHOD (enable,               "enable",               "()V") \
            CALLBACK (deviceOrientationChanged, "deviceOrientationChanged", "(JI)V")

        DECLARE_JNI_CLASS_WITH_MIN_SDK (OrientationEventListener, "com/roli/juce/JuceOrientationEventListener", 21)
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

        targetAspectRatio = previewSize.getWidth() / (float) previewSize.getHeight();

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

//==============================================================================
CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker::CameraCaptureSessionCaptureCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::StillPictureTaker::CameraCaptureSessionCaptureCallback;
CameraDevice::Pimpl::ScopedCameraDevice::CameraDeviceStateCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CameraDeviceStateCallback;
CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::CameraCaptureSessionStateCallback_Class CameraDevice::Pimpl::ScopedCameraDevice::CaptureSession::CameraCaptureSessionStateCallback;
CameraDevice::Pimpl::DeviceOrientationChangeListener::OrientationEventListener_Class CameraDevice::Pimpl::DeviceOrientationChangeListener::OrientationEventListener;
