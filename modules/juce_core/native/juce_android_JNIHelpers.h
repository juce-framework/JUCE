/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_ANDROID_JNIHELPERS_H_INCLUDED
#define JUCE_ANDROID_JNIHELPERS_H_INCLUDED

#if ! (defined (JUCE_ANDROID_ACTIVITY_CLASSNAME) && defined (JUCE_ANDROID_ACTIVITY_CLASSPATH))
 #error "The JUCE_ANDROID_ACTIVITY_CLASSNAME and JUCE_ANDROID_ACTIVITY_CLASSPATH macros must be set!"
#endif

//==============================================================================
extern JNIEnv* getEnv() noexcept;

// You should rarely need to use this function. Only if you expect callbacks
// on a java thread which you did not create yourself.
extern void setEnv (JNIEnv* env) noexcept;

//==============================================================================
class GlobalRef
{
public:
    inline GlobalRef() noexcept                 : obj (0) {}
    inline explicit GlobalRef (jobject o)       : obj (retain (o)) {}
    inline GlobalRef (const GlobalRef& other)   : obj (retain (other.obj)) {}
    ~GlobalRef()                                { clear(); }

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
    jobject obj;

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
    explicit inline LocalRef (JavaType o) noexcept      : obj (o) {}
    inline LocalRef (const LocalRef& other) noexcept    : obj (retain (other.obj)) {}
    ~LocalRef()                                         { clear(); }

    void clear()
    {
        if (obj != 0)
            getEnv()->DeleteLocalRef (obj);
    }

    LocalRef& operator= (const LocalRef& other)
    {
        jobject newObj = retain (other.obj);
        clear();
        obj = newObj;
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
namespace
{
    String juceString (JNIEnv* env, jstring s)
    {
        const char* const utf8 = env->GetStringUTFChars (s, nullptr);
        CharPointer_UTF8 utf8CP (utf8);
        const String result (utf8CP);
        env->ReleaseStringUTFChars (s, utf8);
        return result;
    }

    String juceString (jstring s)
    {
        return juceString (getEnv(), s);
    }

    LocalRef<jstring> javaString (const String& s)
    {
        return LocalRef<jstring> (getEnv()->NewStringUTF (s.toUTF8()));
    }

    LocalRef<jstring> javaStringFromChar (const juce_wchar c)
    {
        char utf8[8] = { 0 };
        CharPointer_UTF8 (utf8).write (c);
        return LocalRef<jstring> (getEnv()->NewStringUTF (utf8));
    }
}

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
            JNI_CLASS_MEMBERS (CREATE_JNI_METHOD, CREATE_JNI_STATICMETHOD, CREATE_JNI_FIELD, CREATE_JNI_STATICFIELD); \
        } \
    \
        JNI_CLASS_MEMBERS (DECLARE_JNI_METHOD, DECLARE_JNI_METHOD, DECLARE_JNI_FIELD, DECLARE_JNI_FIELD); \
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
 METHOD (createNewView,          "createNewView",        "(ZJ)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView;") \
 METHOD (deleteView,             "deleteView",           "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$ComponentPeerView;)V") \
 METHOD (createNativeSurfaceView, "createNativeSurfaceView", "(J)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$NativeSurfaceView;") \
 METHOD (postMessage,            "postMessage",          "(J)V") \
 METHOD (finish,                 "finish",               "()V") \
 METHOD (getClipboardContent,    "getClipboardContent",  "()Ljava/lang/String;") \
 METHOD (setClipboardContent,    "setClipboardContent",  "(Ljava/lang/String;)V") \
 METHOD (excludeClipRegion,      "excludeClipRegion",    "(Landroid/graphics/Canvas;FFFF)V") \
 METHOD (renderGlyph,            "renderGlyph",          "(CLandroid/graphics/Paint;Landroid/graphics/Matrix;Landroid/graphics/Rect;)[I") \
 STATICMETHOD (createHTTPStream, "createHTTPStream",     "(Ljava/lang/String;Z[BLjava/lang/String;I[ILjava/lang/StringBuffer;ILjava/lang/String;)L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$HTTPStream;") \
 METHOD (launchURL,              "launchURL",            "(Ljava/lang/String;)V") \
 METHOD (showMessageBox,         "showMessageBox",       "(Ljava/lang/String;Ljava/lang/String;J)V") \
 METHOD (showOkCancelBox,        "showOkCancelBox",      "(Ljava/lang/String;Ljava/lang/String;J)V") \
 METHOD (showYesNoCancelBox,     "showYesNoCancelBox",   "(Ljava/lang/String;Ljava/lang/String;J)V") \
 STATICMETHOD (getLocaleValue,   "getLocaleValue",       "(Z)Ljava/lang/String;") \
 METHOD (scanFile,               "scanFile",             "(Ljava/lang/String;)V") \
 METHOD (getTypeFaceFromAsset,   "getTypeFaceFromAsset", "(Ljava/lang/String;)Landroid/graphics/Typeface;") \
 METHOD (getTypeFaceFromByteArray,"getTypeFaceFromByteArray","([B)Landroid/graphics/Typeface;") \
 METHOD (setScreenSaver,          "setScreenSaver",       "(Z)V") \
 METHOD (getScreenSaver,          "getScreenSaver",       "()Z") \
 METHOD (getAndroidMidiDeviceManager, "getAndroidMidiDeviceManager", "()L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$MidiDeviceManager;") \
 METHOD (getAndroidBluetoothManager, "getAndroidBluetoothManager", "()L" JUCE_ANDROID_ACTIVITY_CLASSPATH "$BluetoothManager;") \
 METHOD (getAndroidSDKVersion,    "getAndroidSDKVersion", "()I") \
 METHOD (audioManagerGetProperty, "audioManagerGetProperty", "(Ljava/lang/String;)Ljava/lang/String;") \
 METHOD (setCurrentThreadPriority, "setCurrentThreadPriority", "(I)I") \
 METHOD (hasSystemFeature,         "hasSystemFeature", "(Ljava/lang/String;)Z" ) \
 METHOD (createNewThread,          "createNewThread", "(J)Ljava/lang/Thread;") \

DECLARE_JNI_CLASS (JuceAppActivity, JUCE_ANDROID_ACTIVITY_CLASSPATH);
#undef JNI_CLASS_MEMBERS

//==============================================================================
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

DECLARE_JNI_CLASS (Paint, "android/graphics/Paint");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",    "()V") \
 METHOD (setValues,     "setValues", "([F)V") \

DECLARE_JNI_CLASS (Matrix, "android/graphics/Matrix");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (start, "start", "()V") \
 METHOD (stop, "stop", "()V") \
 METHOD (setName, "setName", "(Ljava/lang/String;)V") \
 METHOD (getName, "getName", "()Ljava/lang/String;") \
 METHOD (getId, "getId", "()J") \
 STATICMETHOD (currentThread, "currentThread", "()Ljava/lang/Thread;") \
 METHOD (setPriority, "setPriority", "(I)V") \

DECLARE_JNI_CLASS (JuceThread, "java/lang/Thread");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,   "<init>",   "(IIII)V") \
 FIELD (left,           "left",     "I") \
 FIELD (right,          "right",    "I") \
 FIELD (top,            "top",      "I") \
 FIELD (bottom,         "bottom",   "I") \

DECLARE_JNI_CLASS (RectClass, "android/graphics/Rect");
#undef JNI_CLASS_MEMBERS

#endif   // JUCE_ANDROID_JNIHELPERS_H_INCLUDED
