/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if ! (defined (JUCE_ANDROID_ACTIVITY_CLASSNAME) && defined (JUCE_ANDROID_ACTIVITY_CLASSPATH))
 #error "The JUCE_ANDROID_ACTIVITY_CLASSNAME and JUCE_ANDROID_ACTIVITY_CLASSPATH macros must be set!"
#endif

//==============================================================================
extern JNIEnv* getEnv() noexcept;

// You should rarely need to use this function. Only if you expect callbacks
// on a java thread which you did not create yourself.
extern void setEnv (JNIEnv* env) noexcept;

/* @internal */
extern JNIEnv* attachAndroidJNI() noexcept;

//==============================================================================
class GlobalRef
{
public:
    inline GlobalRef() noexcept                    : obj (0) {}
    inline explicit GlobalRef (jobject o)          : obj (retain (o)) {}
    inline GlobalRef (const GlobalRef& other)      : obj (retain (other.obj)) {}
    inline GlobalRef (GlobalRef && other) noexcept : obj (0) { std::swap (other.obj, obj); }
    ~GlobalRef()                                             { clear(); }

    inline void clear()
    {
        if (obj != 0)
        {
            getEnv()->DeleteGlobalRef (obj);
            obj = 0;
        }
    }

    inline GlobalRef& operator= (const GlobalRef& other)
    {
        jobject newObj = retain (other.obj);
        clear();
        obj = newObj;
        return *this;
    }

    inline GlobalRef& operator= (GlobalRef&& other)
    {
        clear();
        std::swap (obj, other.obj);

        return *this;
    }

    //==============================================================================
    inline operator jobject() const noexcept    { return obj; }
    inline jobject get() const noexcept         { return obj; }

    //==============================================================================
    #define DECLARE_CALL_TYPE_METHOD(returnType, typeName) \
        returnType call##typeName##Method (jmethodID methodID, ... ) const \
        { \
            va_list args; \
            va_start (args, methodID); \
            returnType result = getEnv()->Call##typeName##MethodV (obj, methodID, args); \
            va_end (args); \
            return result; \
        }

    DECLARE_CALL_TYPE_METHOD (jobject, Object)
    DECLARE_CALL_TYPE_METHOD (jboolean, Boolean)
    DECLARE_CALL_TYPE_METHOD (jbyte, Byte)
    DECLARE_CALL_TYPE_METHOD (jchar, Char)
    DECLARE_CALL_TYPE_METHOD (jshort, Short)
    DECLARE_CALL_TYPE_METHOD (jint, Int)
    DECLARE_CALL_TYPE_METHOD (jlong, Long)
    DECLARE_CALL_TYPE_METHOD (jfloat, Float)
    DECLARE_CALL_TYPE_METHOD (jdouble, Double)
    #undef DECLARE_CALL_TYPE_METHOD

    void callVoidMethod (jmethodID methodID, ... ) const
    {
        va_list args;
        va_start (args, methodID);
        getEnv()->CallVoidMethodV (obj, methodID, args);
        va_end (args);
    }

private:
    //==============================================================================
    jobject obj = 0;

    static inline jobject retain (jobject obj)
    {
        return obj == 0 ? 0 : getEnv()->NewGlobalRef (obj);
    }
};

//==============================================================================
template <typename JavaType>
class LocalRef
{
public:
    explicit inline LocalRef() noexcept                 : obj (0) {}
    explicit inline LocalRef (JavaType o) noexcept      : obj (o) {}
    inline LocalRef (const LocalRef& other) noexcept    : obj (retain (other.obj)) {}
    inline LocalRef (LocalRef&& other) noexcept         : obj (0) { std::swap (obj, other.obj); }
    ~LocalRef()                                         { clear(); }

    void clear()
    {
        if (obj != 0)
        {
            getEnv()->DeleteLocalRef (obj);
            obj = 0;
        }
    }

    LocalRef& operator= (const LocalRef& other)
    {
        JavaType newObj = retain (other.obj);
        clear();
        obj = newObj;
        return *this;
    }

    LocalRef& operator= (LocalRef&& other)
    {
        clear();
        std::swap (other.obj, obj);
        return *this;
    }

    inline operator JavaType() const noexcept   { return obj; }
    inline JavaType get() const noexcept        { return obj; }

private:
    JavaType obj;

    static JavaType retain (JavaType obj)
    {
        return obj == 0 ? 0 : (JavaType) getEnv()->NewLocalRef (obj);
    }
};

//==============================================================================
class JNIClassBase
{
public:
    explicit JNIClassBase (const char* classPath);
    virtual ~JNIClassBase();

    inline operator jclass() const noexcept { return classRef; }

    static void initialiseAllClasses (JNIEnv*);
    static void releaseAllClasses (JNIEnv*);

protected:
    virtual void initialiseFields (JNIEnv*) = 0;

    jmethodID resolveMethod (JNIEnv*, const char* methodName, const char* params);
    jmethodID resolveStaticMethod (JNIEnv*, const char* methodName, const char* params);
    jfieldID resolveField (JNIEnv*, const char* fieldName, const char* signature);
    jfieldID resolveStaticField (JNIEnv*, const char* fieldName, const char* signature);

private:
    const char* const classPath;
    jclass classRef;

    static Array<JNIClassBase*>& getClasses();
    void initialise (JNIEnv*);
    void release (JNIEnv*);

    JUCE_DECLARE_NON_COPYABLE (JNIClassBase)
};

//==============================================================================
#define CREATE_JNI_METHOD(methodID, stringName, params)         methodID = resolveMethod (env, stringName, params);
#define CREATE_JNI_STATICMETHOD(methodID, stringName, params)   methodID = resolveStaticMethod (env, stringName, params);
#define CREATE_JNI_FIELD(fieldID, stringName, signature)        fieldID  = resolveField (env, stringName, signature);
#define CREATE_JNI_STATICFIELD(fieldID, stringName, signature)  fieldID  = resolveStaticField (env, stringName, signature);
#define DECLARE_JNI_METHOD(methodID, stringName, params)        jmethodID methodID;
#define DECLARE_JNI_FIELD(fieldID, stringName, signature)       jfieldID  fieldID;

#define DECLARE_JNI_CLASS(CppClassName, javaPath) \
    class CppClassName ## _Class   : public JNIClassBase \
    { \
    public: \
        CppClassName ## _Class() : JNIClassBase (javaPath) {} \
    \
        void initialiseFields (JNIEnv* env) \
        { \
            ignoreUnused (env); \
            JNI_CLASS_MEMBERS (CREATE_JNI_METHOD, CREATE_JNI_STATICMETHOD, CREATE_JNI_FIELD, CREATE_JNI_STATICFIELD); \
        } \
    \
        JNI_CLASS_MEMBERS (DECLARE_JNI_METHOD, DECLARE_JNI_METHOD, DECLARE_JNI_FIELD, DECLARE_JNI_FIELD) \
    }; \
    static CppClassName ## _Class CppClassName;


//==============================================================================
#if defined (__arm__)
 #define JUCE_ARM_SOFT_FLOAT_ABI  __attribute__ ((pcs("aapcs")))
#else
 #define JUCE_ARM_SOFT_FLOAT_ABI
#endif

#define JUCE_JNI_CALLBACK(className, methodName, returnType, params) \
  extern "C" __attribute__ ((visibility("default"))) JUCE_ARM_SOFT_FLOAT_ABI returnType JUCE_JOIN_MACRO (JUCE_JOIN_MACRO (Java_, className), _ ## methodName) params



//==============================================================================
class AndroidSystem
{
public:
    AndroidSystem();

    void initialise (JNIEnv*, jobject activity, jstring appFile, jstring appDataDir);
    void shutdown (JNIEnv*);

    //==============================================================================
    GlobalRef activity;
    String appFile, appDataDir;
    int screenWidth, screenHeight, dpi;
};

extern AndroidSystem android;

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (createNewView,                        "createNewView",                   "(ZJ)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView;") \
 METHOD (deleteView,                           "deleteView",                      "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView;)V") \
 METHOD (createNativeSurfaceView,              "createNativeSurfaceView",         "(JZ)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$NativeSurfaceView;") \
 METHOD (finish,                               "finish",                          "()V") \
 METHOD (getWindowManager,                     "getWindowManager",                "()Landroid/view/WindowManager;") \
 METHOD (setRequestedOrientation,              "setRequestedOrientation",         "(I)V") \
 METHOD (getClipboardContent,                  "getClipboardContent",             "()Ljava/lang/String;") \
 METHOD (setClipboardContent,                  "setClipboardContent",             "(Ljava/lang/String;)V") \
 METHOD (excludeClipRegion,                    "excludeClipRegion",               "(Landroid/graphics/Canvas;FFFF)V") \
 METHOD (renderGlyph,                          "renderGlyph",                     "(CCLandroid/graphics/Paint;Landroid/graphics/Matrix;Landroid/graphics/Rect;)[I") \
 STATICMETHOD (createHTTPStream,               "createHTTPStream",                "(Ljava/lang/String;Z[BLjava/lang/String;I[ILjava/lang/StringBuffer;ILjava/lang/String;)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$HTTPStream;") \
 METHOD (launchURL,                            "launchURL",                       "(Ljava/lang/String;)V") \
 METHOD (showMessageBox,                       "showMessageBox",                  "(Ljava/lang/String;Ljava/lang/String;J)V") \
 METHOD (showOkCancelBox,                      "showOkCancelBox",                 "(Ljava/lang/String;Ljava/lang/String;JLjava/lang/String;Ljava/lang/String;)V") \
 METHOD (showYesNoCancelBox,                   "showYesNoCancelBox",              "(Ljava/lang/String;Ljava/lang/String;J)V") \
 STATICMETHOD (getLocaleValue,                 "getLocaleValue",                  "(Z)Ljava/lang/String;") \
 STATICMETHOD (getDocumentsFolder,             "getDocumentsFolder",              "()Ljava/lang/String;") \
 STATICMETHOD (getPicturesFolder,              "getPicturesFolder",               "()Ljava/lang/String;") \
 STATICMETHOD (getMusicFolder,                 "getMusicFolder",                  "()Ljava/lang/String;") \
 STATICMETHOD (getDownloadsFolder,             "getDownloadsFolder",              "()Ljava/lang/String;") \
 STATICMETHOD (getMoviesFolder,                "getMoviesFolder",                 "()Ljava/lang/String;") \
 METHOD (getTypeFaceFromAsset,                 "getTypeFaceFromAsset",            "(Ljava/lang/String;)Landroid/graphics/Typeface;") \
 METHOD (getTypeFaceFromByteArray,             "getTypeFaceFromByteArray",        "([B)Landroid/graphics/Typeface;") \
 METHOD (setScreenSaver,                       "setScreenSaver",                  "(Z)V") \
 METHOD (getScreenSaver,                       "getScreenSaver",                  "()Z") \
 METHOD (getAndroidMidiDeviceManager,          "getAndroidMidiDeviceManager",     "()L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$MidiDeviceManager;") \
 METHOD (getAndroidBluetoothManager,           "getAndroidBluetoothManager",      "()L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$BluetoothManager;") \
 STATICMETHOD (getAndroidSDKVersion,           "getAndroidSDKVersion",            "()I") \
 METHOD (audioManagerGetProperty,              "audioManagerGetProperty",         "(Ljava/lang/String;)Ljava/lang/String;") \
 METHOD (hasSystemFeature,                     "hasSystemFeature",                "(Ljava/lang/String;)Z" ) \
 METHOD (requestRuntimePermission,             "requestRuntimePermission",        "(IJ)V" ) \
 METHOD (isPermissionGranted,                  "isPermissionGranted",             "(I)Z" ) \
 METHOD (isPermissionDeclaredInManifest,       "isPermissionDeclaredInManifest",  "(I)Z" ) \
 METHOD (isPermissionDeclaredInManifestString, "isPermissionDeclaredInManifest",  "(Ljava/lang/String;)Z") \
 METHOD (getAssets,                            "getAssets",                       "()Landroid/content/res/AssetManager;") \
 METHOD (getSystemService,                     "getSystemService",                "(Ljava/lang/String;)Ljava/lang/Object;") \
 METHOD (getPackageManager,                    "getPackageManager",               "()Landroid/content/pm/PackageManager;") \
 METHOD (getPackageName,                       "getPackageName",                  "()Ljava/lang/String;") \
 METHOD (getResources,                         "getResources",                    "()Landroid/content/res/Resources;") \
 METHOD (createInvocationHandler,              "createInvocationHandler",         "(J)Ljava/lang/reflect/InvocationHandler;") \
 METHOD (invocationHandlerContextDeleted,      "invocationHandlerContextDeleted", "(Ljava/lang/reflect/InvocationHandler;)V") \
 METHOD (bindService,                          "bindService",                     "(Landroid/content/Intent;Landroid/content/ServiceConnection;I)Z") \
 METHOD (unbindService,                        "unbindService",                   "(Landroid/content/ServiceConnection;)V") \
 METHOD (startIntentSenderForResult,           "startIntentSenderForResult",      "(Landroid/content/IntentSender;ILandroid/content/Intent;III)V") \
 METHOD (moveTaskToBack,                       "moveTaskToBack",                  "(Z)Z") \
 METHOD (startActivity,                        "startActivity",                   "(Landroid/content/Intent;)V") \
 METHOD (startActivityForResult,               "startActivityForResult",          "(Landroid/content/Intent;I)V") \
 METHOD (getContentResolver,                   "getContentResolver",              "()Landroid/content/ContentResolver;") \
 METHOD (addAppPausedResumedListener,          "addAppPausedResumedListener",     "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$AppPausedResumedListener;J)V") \
 METHOD (removeAppPausedResumedListener,       "removeAppPausedResumedListener",  "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$AppPausedResumedListener;J)V")

DECLARE_JNI_CLASS (JuceAppActivity, JUCE_ANDROID_ACTIVITY_CLASSPATH)
#undef JNI_CLASS_MEMBERS

//==============================================================================
#if __ANDROID_API__ >= 21
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (build,          "build",          "()Landroid/media/AudioAttributes;") \
  METHOD (constructor,    "<init>",         "()V") \
  METHOD (setContentType, "setContentType", "(I)Landroid/media/AudioAttributes$Builder;") \
  METHOD (setUsage,       "setUsage",       "(I)Landroid/media/AudioAttributes$Builder;")

DECLARE_JNI_CLASS (AndroidAudioAttributesBuilder, "android/media/AudioAttributes$Builder")
#undef JNI_CLASS_MEMBERS
#endif

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (abandonAudioFocus, "abandonAudioFocus", "(Landroid/media/AudioManager$OnAudioFocusChangeListener;)I") \
  METHOD (requestAudioFocus, "requestAudioFocus", "(Landroid/media/AudioManager$OnAudioFocusChangeListener;II)I")

DECLARE_JNI_CLASS (AndroidAudioManager, "android/media/AudioManager")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (createBitmap,     "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;") \
  STATICMETHOD (createBitmapFrom, "createBitmap", "(Landroid/graphics/Bitmap;IIIILandroid/graphics/Matrix;Z)Landroid/graphics/Bitmap;") \
  METHOD (compress,  "compress",  "(Landroid/graphics/Bitmap$CompressFormat;ILjava/io/OutputStream;)Z") \
  METHOD (getHeight, "getHeight", "()I") \
  METHOD (getWidth,  "getWidth",  "()I") \
  METHOD (recycle,   "recycle",   "()V") \
  METHOD (setPixel,  "setPixel",  "(III)V")

DECLARE_JNI_CLASS (AndroidBitmap, "android/graphics/Bitmap")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (valueOf, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;")

DECLARE_JNI_CLASS (AndroidBitmapConfig, "android/graphics/Bitmap$Config")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (decodeByteArray, "decodeByteArray", "([BII)Landroid/graphics/Bitmap;")

DECLARE_JNI_CLASS (AndroidBitmapFactory, "android/graphics/BitmapFactory")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor,        "<init>",             "()V") \
  METHOD (containsKey,        "containsKey",        "(Ljava/lang/String;)Z") \
  METHOD (get,                "get",                "(Ljava/lang/String;)Ljava/lang/Object;") \
  METHOD (getBoolean,         "getBoolean",         "(Ljava/lang/String;)Z") \
  METHOD (getBundle,          "getBundle",          "(Ljava/lang/String;)Landroid/os/Bundle;") \
  METHOD (getCharSequence,    "getCharSequence",    "(Ljava/lang/String;)Ljava/lang/CharSequence;") \
  METHOD (getInt,             "getInt",             "(Ljava/lang/String;)I") \
  METHOD (getLong,            "getLong",            "(Ljava/lang/String;)J") \
  METHOD (getLongArray,       "getLongArray",       "(Ljava/lang/String;)[J") \
  METHOD (getParcelable,      "getParcelable",      "(Ljava/lang/String;)Landroid/os/Parcelable;") \
  METHOD (getString,          "getString",          "(Ljava/lang/String;)Ljava/lang/String;") \
  METHOD (getStringArrayList, "getStringArrayList", "(Ljava/lang/String;)Ljava/util/ArrayList;") \
  METHOD (keySet,             "keySet",             "()Ljava/util/Set;") \
  METHOD (putBoolean,         "putBoolean",         "(Ljava/lang/String;Z)V") \
  METHOD (putBundle,          "putBundle",          "(Ljava/lang/String;Landroid/os/Bundle;)V") \
  METHOD (putFloat,           "putFloat",           "(Ljava/lang/String;F)V") \
  METHOD (putInt,             "putInt",             "(Ljava/lang/String;I)V") \
  METHOD (putLong,            "putLong",            "(Ljava/lang/String;J)V") \
  METHOD (putLongArray,       "putLongArray",       "(Ljava/lang/String;[J)V") \
  METHOD (putString,          "putString",          "(Ljava/lang/String;Ljava/lang/String;)V") \
  METHOD (putStringArrayList, "putStringArrayList", "(Ljava/lang/String;Ljava/util/ArrayList;)V")

DECLARE_JNI_CLASS (AndroidBundle, "android/os/Bundle")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (dumpReferenceTables, "dumpReferenceTables", "()V")

  DECLARE_JNI_CLASS (AndroidDebug, "android/os/Debug")
#undef JNI_CLASS_MEMBERS

#define JUCE_LOG_JNI_REFERENCES_TABLE getEnv()->CallStaticVoidMethod (AndroidDebug, AndroidDebug.dumpReferenceTables);

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getRotation, "getRotation", "()I")

DECLARE_JNI_CLASS (AndroidDisplay, "android/view/Display")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor,           "<init>",      "()V") \
  METHOD (constructorWithLooper, "<init>",      "(Landroid/os/Looper;)V") \
  METHOD (post,                  "post",        "(Ljava/lang/Runnable;)Z") \
  METHOD (postDelayed,           "postDelayed", "(Ljava/lang/Runnable;J)Z") \

DECLARE_JNI_CLASS (AndroidHandler, "android/os/Handler")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>",     "(Ljava/lang/String;)V") \
  METHOD (getLooper,   "getLooper",  "()Landroid/os/Looper;") \
  METHOD (join,        "join",       "()V") \
  METHOD (quitSafely,  "quitSafely", "()Z") \
  METHOD (start,       "start",      "()V")

DECLARE_JNI_CLASS (AndroidHandlerThread, "android/os/HandlerThread")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (createChooser, "createChooser", "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;") \
  METHOD (addCategory,                    "addCategory",    "(Ljava/lang/String;)Landroid/content/Intent;") \
  METHOD (constructor,                    "<init>",         "()V") \
  METHOD (constructorWithContextAndClass, "<init>",         "(Landroid/content/Context;Ljava/lang/Class;)V") \
  METHOD (constructWithString,            "<init>",         "(Ljava/lang/String;)V") \
  METHOD (getAction,                      "getAction",      "()Ljava/lang/String;") \
  METHOD (getCategories,                  "getCategories",  "()Ljava/util/Set;") \
  METHOD (getData,                        "getData",        "()Landroid/net/Uri;") \
  METHOD (getExtras,                      "getExtras",      "()Landroid/os/Bundle;") \
  METHOD (getIntExtra,                    "getIntExtra",    "(Ljava/lang/String;I)I") \
  METHOD (getStringExtra,                 "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;") \
  METHOD (putExtra,                       "putExtra",       "(Ljava/lang/String;Ljava/lang/CharSequence;)Landroid/content/Intent;") \
  METHOD (putExtras,                      "putExtras",      "(Landroid/os/Bundle;)Landroid/content/Intent;") \
  METHOD (putExtraString,                 "putExtra",       "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;") \
  METHOD (putExtraStrings,                "putExtra",       "(Ljava/lang/String;[Ljava/lang/String;)Landroid/content/Intent;") \
  METHOD (putExtraParcelable,             "putExtra",       "(Ljava/lang/String;Landroid/os/Parcelable;)Landroid/content/Intent;") \
  METHOD (putParcelableArrayListExtra,    "putParcelableArrayListExtra", "(Ljava/lang/String;Ljava/util/ArrayList;)Landroid/content/Intent;") \
  METHOD (setAction,                      "setAction",      "(Ljava/lang/String;)Landroid/content/Intent;") \
  METHOD (setFlags,                       "setFlags",       "(I)Landroid/content/Intent;") \
  METHOD (setPackage,                     "setPackage",     "(Ljava/lang/String;)Landroid/content/Intent;") \
  METHOD (setType,                        "setType",        "(Ljava/lang/String;)Landroid/content/Intent;") \

DECLARE_JNI_CLASS (AndroidIntent, "android/content/Intent")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,    "<init>",        "()V") \
 METHOD (postRotate,     "postRotate",    "(FFF)Z") \
 METHOD (postScale,      "postScale",     "(FFFF)Z") \
 METHOD (postTranslate,  "postTranslate", "(FF)Z") \
 METHOD (setValues,      "setValues",     "([F)V")

DECLARE_JNI_CLASS (AndroidMatrix, "android/graphics/Matrix")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getPackageInfo, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;")

DECLARE_JNI_CLASS (AndroidPackageManager, "android/content/pm/PackageManager")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",           "(I)V") \
 METHOD (setColor,      "setColor",         "(I)V") \
 METHOD (setAlpha,      "setAlpha",         "(I)V") \
 METHOD (setTypeface,   "setTypeface",      "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;") \
 METHOD (ascent,        "ascent",           "()F") \
 METHOD (descent,       "descent",          "()F") \
 METHOD (setTextSize,   "setTextSize",      "(F)V") \
 METHOD (getTextWidths, "getTextWidths",    "(Ljava/lang/String;[F)I") \
 METHOD (setTextScaleX, "setTextScaleX",    "(F)V") \
 METHOD (getTextPath,   "getTextPath",      "(Ljava/lang/String;IIFFLandroid/graphics/Path;)V") \
 METHOD (setShader,     "setShader",        "(Landroid/graphics/Shader;)Landroid/graphics/Shader;") \

DECLARE_JNI_CLASS (AndroidPaint, "android/graphics/Paint")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (getActivity, "getActivity", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;") \
  METHOD (getIntentSender, "getIntentSender", "()Landroid/content/IntentSender;")

DECLARE_JNI_CLASS (AndroidPendingIntent, "android/app/PendingIntent")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (AndroidRange, "android/util/Range")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",   "(IIII)V") \
 FIELD (left,           "left",     "I") \
 FIELD (right,          "right",    "I") \
 FIELD (top,            "top",      "I") \
 FIELD (bottom,         "bottom",   "I") \

DECLARE_JNI_CLASS (AndroidRect, "android/graphics/Rect")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (getIdentifier,     "getIdentifier",     "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I") \
  METHOD (openRawResourceFd, "openRawResourceFd", "(I)Landroid/content/res/AssetFileDescriptor;")

DECLARE_JNI_CLASS (AndroidResources, "android/content/res/Resources")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (getHeight, "getHeight", "()I") \
  METHOD (getWidth,  "getWidth",  "()I")

DECLARE_JNI_CLASS (AndroidSize, "android/util/Size")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (parse, "parse", "(Ljava/lang/String;)Landroid/net/Uri;") \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (AndroidUri, "android/net/Uri")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (layout,              "layout",              "(IIII)V") \
 METHOD (getLeft,             "getLeft",             "()I") \
 METHOD (getTop,              "getTop",              "()I") \
 METHOD (getWidth,            "getWidth",            "()I") \
 METHOD (getHeight,           "getHeight",           "()I") \
 METHOD (getLocationOnScreen, "getLocationOnScreen", "([I)V") \
 METHOD (getParent,           "getParent",           "()Landroid/view/ViewParent;") \
 METHOD (bringToFront,        "bringToFront",        "()V") \
 METHOD (requestFocus,        "requestFocus",        "()Z") \
 METHOD (hasFocus,            "hasFocus",            "()Z") \
 METHOD (invalidate,          "invalidate",          "(IIII)V") \
 METHOD (setVisibility,       "setVisibility",       "(I)V")

DECLARE_JNI_CLASS (AndroidView, "android/view/View")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (addView,    "addView",    "(Landroid/view/View;)V") \
 METHOD (removeView, "removeView", "(Landroid/view/View;)V")

DECLARE_JNI_CLASS (AndroidViewGroup, "android/view/ViewGroup")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getDefaultDisplay, "getDefaultDisplay", "()Landroid/view/Display;")

DECLARE_JNI_CLASS (AndroidWindowManager, "android/view/WindowManager")
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>",   "(I)V") \
  METHOD (add,         "add",      "(Ljava/lang/Object;)Z") \
  METHOD (iterator,    "iterator", "()Ljava/util/Iterator;") \
  METHOD (get,         "get",      "(I)Ljava/lang/Object;") \
  METHOD (size,        "size",     "()I")

DECLARE_JNI_CLASS (JavaArrayList, "java/util/ArrayList")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (valueOf, "valueOf", "(Z)Ljava/lang/Boolean;") \
  METHOD (booleanValue, "booleanValue", "()Z")

DECLARE_JNI_CLASS (JavaBoolean, "java/lang/Boolean")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (get,       "get",       "([B)Ljava/nio/ByteBuffer;") \
  METHOD (remaining, "remaining", "()I")

DECLARE_JNI_CLASS (JavaByteBuffer, "java/nio/ByteBuffer")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (JavaCharSequence, "java/lang/CharSequence")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (forName, "forName", "(Ljava/lang/String;)Ljava/lang/Class;") \
  METHOD (getName,           "getName",           "()Ljava/lang/String;") \
  METHOD (getModifiers,      "getModifiers",      "()I")            \
  METHOD (isAnnotation,      "isAnnotation",      "()Z") \
  METHOD (isAnonymousClass,  "isAnonymousClass",  "()Z") \
  METHOD (isArray,           "isArray",           "()Z") \
  METHOD (isEnum,            "isEnum",            "()Z") \
  METHOD (isInterface,       "isInterface",       "()Z") \
  METHOD (isLocalClass,      "isLocalClass",      "()Z") \
  METHOD (isMemberClass,     "isMemberClass",     "()Z") \
  METHOD (isPrimitive,       "isPrimitive",       "()Z") \
  METHOD (isSynthetic,       "isSynthetic",       "()Z") \
  METHOD (getComponentType,  "getComponentType",  "()Ljava/lang/Class;") \
  METHOD (getSuperclass,     "getSuperclass",     "()Ljava/lang/Class;") \
  METHOD (getClassLoader,    "getClassLoader",    "()Ljava/lang/ClassLoader;") \

DECLARE_JNI_CLASS (JavaClass, "java/lang/Class")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (JavaEnum, "java/lang/Enum")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,     "<init>",          "(Ljava/lang/String;)V") \
 METHOD (getAbsolutePath, "getAbsolutePath", "()Ljava/lang/String;") \
 METHOD (length,          "length",          "()J")

DECLARE_JNI_CLASS (JavaFile, "java/io/File")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(Ljava/lang/String;)V") \
 METHOD (close,       "close",  "()V") \
 METHOD (read,        "read",   "([B)I")

DECLARE_JNI_CLASS (JavaFileInputStream, "java/io/FileInputStream")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(Ljava/lang/String;)V") \
 METHOD (close,       "close",  "()V") \
 METHOD (write,       "write",  "([BII)V")

DECLARE_JNI_CLASS (JavaFileOutputStream, "java/io/FileOutputStream")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor,             "<init>", "()V") \
  METHOD (constructorWithCapacity, "<init>", "(I)V")

DECLARE_JNI_CLASS (JavaHashMap, "java/util/HashMap")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  STATICMETHOD (parseInt, "parseInt", "(Ljava/lang/String;I)I") \
  STATICMETHOD (valueOf,  "valueOf",  "(I)Ljava/lang/Integer;") \
  METHOD (intValue, "intValue", "()I")

DECLARE_JNI_CLASS (JavaInteger, "java/lang/Integer")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (hasNext, "hasNext", "()Z") \
  METHOD (next,    "next",    "()Ljava/lang/Object;")

DECLARE_JNI_CLASS (JavaIterator, "java/util/Iterator")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (get,  "get",  "(I)Ljava/lang/Object;") \
  METHOD (size, "size", "()I")

DECLARE_JNI_CLASS (JavaList, "java/util/List")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>", "(J)V")

DECLARE_JNI_CLASS (JavaLong, "java/lang/Long")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (get,    "get",    "(Ljava/lang/Object;)Ljava/lang/Object;") \
  METHOD (keySet, "keySet", "()Ljava/util/Set;") \
  METHOD (put,    "put",    "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;")

DECLARE_JNI_CLASS (JavaMap, "java/util/Map")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (getName,           "getName",           "()Ljava/lang/String;") \
  METHOD (getModifiers,      "getModifiers",      "()I")            \
  METHOD (getParameterTypes, "getParameterTypes", "()[Ljava/lang/Class;") \
  METHOD (getReturnType,     "getReturnType",     "()Ljava/lang/Class;") \
  METHOD (invoke,            "invoke",            "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;") \
  METHOD (hashCode,          "hashCode",          "()I") \
  METHOD (equals,            "equals",            "(Ljava/lang/Object;)Z") \

DECLARE_JNI_CLASS (JavaMethod, "java/lang/reflect/Method")
#undef JNI_CLASS_MEMBERS


#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (constructor, "<init>", "()V") \
  METHOD (getClass, "getClass", "()Ljava/lang/Class;") \
  METHOD (toString, "toString", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (JavaObject, "java/lang/Object")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (contains, "contains", "(Ljava/lang/Object;)Z") \
  METHOD (iterator, "iterator", "()Ljava/util/Iterator;") \
  METHOD (size,     "size",     "()I")

DECLARE_JNI_CLASS (JavaSet, "java/util/Set")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
  METHOD (concat,   "concat",   "(Ljava/lang/String;)Ljava/lang/String;") \
  METHOD (getBytes, "getBytes", "()[B")

DECLARE_JNI_CLASS (JavaString, "java/lang/String")
#undef JNI_CLASS_MEMBERS

//==============================================================================
namespace
{
    inline String juceString (JNIEnv* env, jstring s)
    {
        if (s == 0)
            return {};

        const char* const utf8 = env->GetStringUTFChars (s, nullptr);
        CharPointer_UTF8 utf8CP (utf8);
        const String result (utf8CP);
        env->ReleaseStringUTFChars (s, utf8);
        return result;
    }

    inline String juceString (jstring s)
    {
        return juceString (getEnv(), s);
    }

    inline LocalRef<jstring> javaString (const String& s)
    {
        return LocalRef<jstring> (getEnv()->NewStringUTF (s.toUTF8()));
    }

    inline LocalRef<jstring> javaStringFromChar (const juce_wchar c)
    {
        char utf8[8] = { 0 };
        CharPointer_UTF8 (utf8).write (c);
        return LocalRef<jstring> (getEnv()->NewStringUTF (utf8));
    }

    inline LocalRef<jobjectArray> juceStringArrayToJava (const StringArray& juceArray)
    {
        auto* env = getEnv();

        LocalRef<jobjectArray> result (env->NewObjectArray ((jsize) juceArray.size(),
                                                            JavaString,
                                                            javaString ("").get()));

        for (int i = 0; i < juceArray.size(); ++i)
            env->SetObjectArrayElement (result, i, javaString (juceArray [i]).get());

        return result;
    }

    inline StringArray javaStringArrayToJuce (const LocalRef<jobjectArray>& javaArray)
    {
        if (javaArray.get() == nullptr)
            return {};

        auto* env = getEnv();

        StringArray result;

        for (int i = 0; i < env->GetArrayLength (javaArray.get()); ++i)
        {
            LocalRef<jstring> javaString ((jstring) env->GetObjectArrayElement (javaArray.get(), i));
            result.add (juceString (javaString.get()));
        }

        return result;
    }

    inline bool jniCheckHasExceptionOccurredAndClear()
    {
        auto* env = getEnv();

        LocalRef<jobject> exception (env->ExceptionOccurred());

        if (exception != nullptr)
        {
            env->ExceptionClear();
            return true;
        }

        return false;
    }
}

//==============================================================================
class AndroidInterfaceImplementer;

// This function takes ownership of the implementer. When the returned GlobalRef
// goes out of scope (and no other Java routine has a reference on the return-value)
// then the implementer will be deleted as well.
LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const StringArray& interfaceNames,
                                       LocalRef<jobject> subclass);

//==============================================================================
jobject juce_invokeImplementer (JNIEnv*, jlong, jobject, jobject, jobjectArray);
void    juce_dispatchDelete    (JNIEnv*, jlong);

//==============================================================================
class AndroidInterfaceImplementer
{
protected:
    virtual ~AndroidInterfaceImplementer();
    virtual jobject invoke (jobject proxy, jobject method, jobjectArray args);

    //==============================================================================
    friend LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer*, const StringArray&, LocalRef<jobject>);
    friend jobject juce_invokeImplementer (JNIEnv*, jlong, jobject, jobject, jobjectArray);
    friend void juce_dispatchDelete (JNIEnv*, jlong);
private:
    GlobalRef javaSubClass;
    GlobalRef invocationHandler;
};

LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const StringArray& interfaceNames);
LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const String& interfaceName);

//==============================================================================
class AppPausedResumedListener     : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void appPaused() = 0;
        virtual void appResumed() = 0;
    };

    AppPausedResumedListener (Owner&);

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override;

private:
    Owner& owner;
};

} // namespace juce
