/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

/*
    This file wraps together all the android-specific code, so that
    we can include all the native headers just once, and compile all our
    platform-specific stuff in one big lump, keeping it out of the way of
    the rest of the codebase.
*/

#include "../../core/juce_TargetPlatform.h"

#if JUCE_ANDROID

#undef JUCE_BUILD_NATIVE
#define JUCE_BUILD_NATIVE 1

#include "juce_android_NativeIncludes.h"
#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

//==============================================================================
#include "../../core/juce_Singleton.h"
#include "../../maths/juce_Random.h"
#include "../../core/juce_SystemStats.h"
#include "../../threads/juce_Process.h"
#include "../../threads/juce_Thread.h"
#include "../../threads/juce_InterProcessLock.h"
#include "../../io/files/juce_FileInputStream.h"
#include "../../io/files/juce_FileOutputStream.h"
#include "../../io/files/juce_NamedPipe.h"
#include "../../io/files/juce_DirectoryIterator.h"
#include "../../io/network/juce_URL.h"
#include "../../io/network/juce_MACAddress.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../utilities/juce_DeletedAtShutdown.h"
#include "../../application/juce_Application.h"
#include "../../utilities/juce_SystemClipboard.h"
#include "../../events/juce_MessageManager.h"
#include "../../gui/graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h"
#include "../../gui/graphics/imaging/juce_ImageFileFormat.h"
#include "../../gui/graphics/imaging/juce_CameraDevice.h"
#include "../../gui/components/windows/juce_ComponentPeer.h"
#include "../../gui/components/windows/juce_AlertWindow.h"
#include "../../gui/components/juce_Desktop.h"
#include "../../gui/components/menus/juce_MenuBarModel.h"
#include "../../gui/components/special/juce_OpenGLComponent.h"
#include "../../gui/components/special/juce_QuickTimeMovieComponent.h"
#include "../../gui/components/mouse/juce_DragAndDropContainer.h"
#include "../../gui/components/mouse/juce_MouseInputSource.h"
#include "../../gui/components/keyboard/juce_KeyPressMappingSet.h"
#include "../../gui/components/layout/juce_ComponentMovementWatcher.h"
#include "../../gui/components/special/juce_ActiveXControlComponent.h"
#include "../../gui/components/special/juce_WebBrowserComponent.h"
#include "../../gui/components/special/juce_DropShadower.h"
#include "../../gui/components/special/juce_SystemTrayIconComponent.h"
#include "../../gui/components/filebrowser/juce_FileChooser.h"
#include "../../gui/components/lookandfeel/juce_LookAndFeel.h"
#include "../../audio/audio_file_formats/juce_AudioCDBurner.h"
#include "../../audio/audio_file_formats/juce_AudioCDReader.h"
#include "../../audio/audio_sources/juce_AudioSource.h"
#include "../../audio/dsp/juce_AudioDataConverters.h"
#include "../../audio/devices/juce_AudioIODeviceType.h"
#include "../../audio/devices/juce_MidiOutput.h"
#include "../../audio/devices/juce_MidiInput.h"
#include "../../containers/juce_ScopedValueSetter.h"
#include "../common/juce_MidiDataConcatenator.h"

//==============================================================================
#define JUCE_JNI_CALLBACK(className, methodName, returnType, params) \
  extern "C" __attribute__ ((visibility("default"))) returnType Java_com_juce_ ## className ## _ ## methodName params

//==============================================================================
#define JUCE_JNI_CLASSES(JAVACLASS) \
 JAVACLASS (activityClass, "com/juce/JuceAppActivity") \
 JAVACLASS (componentPeerViewClass, "com/juce/ComponentPeerView") \
 JAVACLASS (fileClass, "java/io/File") \
 JAVACLASS (contextClass, "android/content/Context") \
 JAVACLASS (canvasClass, "android/graphics/Canvas") \
 JAVACLASS (paintClass, "android/graphics/Paint") \
 JAVACLASS (pathClass, "android/graphics/Path") \
 JAVACLASS (matrixClass, "android/graphics/Matrix") \
 JAVACLASS (rectClass, "android/graphics/Rect") \
 JAVACLASS (regionClass, "android/graphics/Region") \
 JAVACLASS (shaderClass, "android/graphics/Shader") \
 JAVACLASS (typefaceClass, "android/graphics/Typeface") \
 JAVACLASS (shaderTileModeClass, "android/graphics/Shader$TileMode") \
 JAVACLASS (linearGradientClass, "android/graphics/LinearGradient") \
 JAVACLASS (radialGradientClass, "android/graphics/RadialGradient") \

//==============================================================================
#define JUCE_JNI_METHODS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
\
 STATICMETHOD (activityClass, printToConsole, "printToConsole", "(Ljava/lang/String;)V") \
 METHOD (activityClass, createNewView, "createNewView", "()Lcom/juce/ComponentPeerView;") \
 METHOD (activityClass, deleteView, "deleteView", "(Lcom/juce/ComponentPeerView;)V") \
 METHOD (activityClass, postMessage, "postMessage", "(J)V") \
 METHOD (activityClass, getClipboardContent, "getClipboardContent", "()Ljava/lang/String;") \
 METHOD (activityClass, setClipboardContent, "setClipboardContent", "(Ljava/lang/String;)V") \
\
 METHOD (fileClass, fileExists, "exists", "()Z") \
\
 METHOD (componentPeerViewClass, setViewName, "setViewName", "(Ljava/lang/String;)V") \
 METHOD (componentPeerViewClass, layout, "layout", "(IIII)V") \
 METHOD (componentPeerViewClass, getLeft, "getLeft", "()I") \
 METHOD (componentPeerViewClass, getTop, "getTop", "()I") \
 METHOD (componentPeerViewClass, getWidth, "getWidth", "()I") \
 METHOD (componentPeerViewClass, getHeight, "getHeight", "()I") \
 METHOD (componentPeerViewClass, getLocationOnScreen, "getLocationOnScreen", "([I)V") \
 METHOD (componentPeerViewClass, bringToFront, "bringToFront", "()V") \
 METHOD (componentPeerViewClass, requestFocus, "requestFocus", "()Z") \
 METHOD (componentPeerViewClass, setVisible, "setVisible", "(Z)V") \
 METHOD (componentPeerViewClass, isVisible, "isVisible", "()Z") \
 METHOD (componentPeerViewClass, hasFocus, "hasFocus", "()Z") \
 METHOD (componentPeerViewClass, invalidate, "invalidate", "(IIII)V") \
\
 METHOD (canvasClass, drawRect, "drawRect", "(FFFFLandroid/graphics/Paint;)V") \
 METHOD (canvasClass, translate, "translate", "(FF)V") \
 METHOD (canvasClass, clipPath, "clipPath", "(Landroid/graphics/Path;)Z") \
 METHOD (canvasClass, clipRect, "clipRect", "(FFFF)Z") \
 METHOD (canvasClass, clipRegion, "clipRegion", "(Landroid/graphics/Region;)Z") \
 METHOD (canvasClass, concat, "concat", "(Landroid/graphics/Matrix;)V") \
 METHOD (canvasClass, drawBitmap, "drawBitmap", "(Landroid/graphics/Bitmap;Landroid/graphics/Matrix;Landroid/graphics/Paint;)V") \
 METHOD (canvasClass, drawLine, "drawLine", "(FFFFLandroid/graphics/Paint;)V") \
 METHOD (canvasClass, drawPath, "drawPath", "(Landroid/graphics/Path;Landroid/graphics/Paint;)V") \
 METHOD (canvasClass, drawText, "drawText", "(Ljava/lang/String;FFLandroid/graphics/Paint;)V") \
 METHOD (canvasClass, getClipBounds, "getClipBounds", "(Landroid/graphics/Rect;)Z") \
 METHOD (canvasClass, getClipBounds2, "getClipBounds", "()Landroid/graphics/Rect;") \
 METHOD (canvasClass, getMatrix, "getMatrix", "()Landroid/graphics/Matrix;") \
 METHOD (canvasClass, save, "save", "()I") \
 METHOD (canvasClass, restore, "restore", "()V") \
 METHOD (canvasClass, saveLayerAlpha, "saveLayerAlpha", "(FFFFII)I") \
\
 METHOD (paintClass, paintClassConstructor, "<init>", "(I)V") \
 METHOD (paintClass, setColor, "setColor", "(I)V") \
 METHOD (paintClass, setShader, "setShader", "(Landroid/graphics/Shader;)Landroid/graphics/Shader;") \
 METHOD (paintClass, setTypeface, "setTypeface", "(Landroid/graphics/Typeface;)Landroid/graphics/Typeface;") \
 METHOD (paintClass, ascent, "ascent", "()F") \
 METHOD (paintClass, descent, "descent", "()F") \
 METHOD (paintClass, setTextSize, "setTextSize", "(F)V") \
 METHOD (paintClass, getTextWidths, "getTextWidths", "(Ljava/lang/String;[F)I") \
 METHOD (paintClass, getTextPath, "getTextPath", "(Ljava/lang/String;IIFFLandroid/graphics/Path;)V") \
\
 METHOD (shaderClass, setLocalMatrix, "setLocalMatrix", "(Landroid/graphics/Matrix;)V") \
 STATICFIELD (shaderTileModeClass, clampMode, "CLAMP", "Landroid/graphics/Shader$TileMode;") \
\
 METHOD (pathClass, pathClassConstructor, "<init>", "()V") \
 METHOD (pathClass, moveTo, "moveTo", "(FF)V") \
 METHOD (pathClass, lineTo, "lineTo", "(FF)V") \
 METHOD (pathClass, quadTo, "quadTo", "(FFFF)V") \
 METHOD (pathClass, cubicTo, "cubicTo", "(FFFFFF)V") \
 METHOD (pathClass, closePath, "close", "()V") \
\
 METHOD (matrixClass, matrixClassConstructor, "<init>", "()V") \
 METHOD (matrixClass, setValues, "setValues", "([F)V") \
\
 STATICMETHOD (typefaceClass, create, "create", "(Ljava/lang/String;I)Landroid/graphics/Typeface;") \
\
 METHOD (rectClass, rectConstructor, "<init>", "(IIII)V") \
 FIELD (rectClass, rectLeft, "left", "I") \
 FIELD (rectClass, rectRight, "right", "I") \
 FIELD (rectClass, rectTop, "top", "I") \
 FIELD (rectClass, rectBottom, "bottom", "I") \
\
 METHOD (linearGradientClass, linearGradientConstructor, "<init>", "(FFFF[I[FLandroid/graphics/Shader$TileMode;)V") \
\
 METHOD (radialGradientClass, radialGradientConstructor, "<init>", "(FFF[I[FLandroid/graphics/Shader$TileMode;)V") \
\
 METHOD (regionClass, regionConstructor, "<init>", "()V"); \
 METHOD (regionClass, regionUnion, "union", "(Landroid/graphics/Rect;)Z"); \


//==============================================================================
class ThreadLocalJNIEnvHolder
{
public:
    ThreadLocalJNIEnvHolder()
        : jvm (0)
    {
        zeromem (threads, sizeof (threads));
        zeromem (envs, sizeof (envs));
    }

    void initialise (JNIEnv* env)
    {
        env->GetJavaVM (&jvm);
        addEnv (env);
    }

    void attach()
    {
        JNIEnv* env = 0;
        jvm->AttachCurrentThread (&env, 0);

        if (env != 0)
            addEnv (env);
    }

    void detach()
    {
        jvm->DetachCurrentThread();

        const pthread_t thisThread = pthread_self();

        ScopedLock sl (addRemoveLock);
        for (int i = 0; i < maxThreads; ++i)
            if (threads[i] == thisThread)
                threads[i] = 0;
    }

    JNIEnv* get() const throw()
    {
        const pthread_t thisThread = pthread_self();

        for (int i = 0; i < maxThreads; ++i)
            if (threads[i] == thisThread)
                return envs[i];

        return 0;
    }

    enum { maxThreads = 16 };

private:
    JavaVM* jvm;
    pthread_t threads [maxThreads];
    JNIEnv* envs [maxThreads];
    CriticalSection addRemoveLock;

    void addEnv (JNIEnv* env)
    {
        ScopedLock sl (addRemoveLock);

        if (get() == 0)
        {
            const pthread_t thisThread = pthread_self();

            for (int i = 0; i < maxThreads; ++i)
            {
                if (threads[i] == 0)
                {
                    envs[i] = env;
                    threads[i] = thisThread;
                    return;
                }
            }
        }

        jassertfalse; // too many threads!
    }
};

static ThreadLocalJNIEnvHolder threadLocalJNIEnvHolder;

struct AndroidThreadScope
{
    AndroidThreadScope()   { threadLocalJNIEnvHolder.attach(); }
    ~AndroidThreadScope()  { threadLocalJNIEnvHolder.detach(); }
};

static inline JNIEnv* getEnv() throw()
{
    return threadLocalJNIEnvHolder.get();
}


//==============================================================================
class GlobalRef
{
public:
    inline GlobalRef() throw()
        : obj (0)
    {
    }

    inline explicit GlobalRef (jobject obj_)
        : obj (retain (obj_))
    {
    }

    inline GlobalRef (const GlobalRef& other)
        : obj (retain (other.obj))
    {
    }

    ~GlobalRef()
    {
        clear();
    }

    inline void clear()
    {
        if (obj != 0)
            getEnv()->DeleteGlobalRef (obj);
    }

    inline GlobalRef& operator= (const GlobalRef& other)
    {
        clear();
        obj = retain (other.obj);
        return *this;
    }

    //==============================================================================
    inline operator jobject() const throw()     { return obj; }
    inline jobject get() const throw()          { return obj; }

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

    static inline jobject retain (jobject obj_)
    {
        return obj_ == 0 ? 0 : getEnv()->NewGlobalRef (obj_);
    }
};

//==============================================================================
template <typename JavaType>
class LocalRef
{
public:
    explicit inline LocalRef (JavaType obj_) throw()
        : obj (obj_)
    {
    }

    inline LocalRef (const LocalRef& other) throw()
        : obj (retain (other.obj))
    {
    }

    ~LocalRef()
    {
        if (obj != 0)
            getEnv()->DeleteLocalRef (obj);
    }

    LocalRef& operator= (const LocalRef& other)
    {
        if (obj != other.obj)
        {
            if (obj != 0)
                getEnv()->DeleteLocalRef (obj);

            obj = retain (other.obj);
        }

        return *this;
    }

    inline operator JavaType() const throw()    { return obj; }
    inline JavaType get() const throw()         { return obj; }

private:
    JavaType obj;

    static JavaType retain (JavaType obj_)
    {
        return obj_ == 0 ? 0 : (JavaType) getEnv()->NewLocalRef (obj_);
    }
};

//==============================================================================
static const String juceString (jstring s)
{
    JNIEnv* env = getEnv();

    jboolean isCopy;
    const char* const utf8 = env->GetStringUTFChars (s, &isCopy);
    CharPointer_UTF8 utf8CP (utf8);
    const String result (utf8CP);
    env->ReleaseStringUTFChars (s, utf8);
    return result;
}

static const LocalRef<jstring> javaString (const String& s)
{
    return LocalRef<jstring> (getEnv()->NewStringUTF (s.toUTF8()));
}

static const LocalRef<jstring> javaStringFromChar (const juce_wchar c)
{
    char utf8[5] = { 0 };
    CharPointer_UTF8 (utf8).write (c);
    return LocalRef<jstring> (getEnv()->NewStringUTF (utf8));
}


//==============================================================================
class AndroidJavaCallbacks
{
public:
    AndroidJavaCallbacks() : screenWidth (0), screenHeight (0)
    {
    }

    void initialise (JNIEnv* env, jobject activity_,
                     jstring appFile_, jstring appDataDir_,
                     int screenWidth_, int screenHeight_)
    {
        threadLocalJNIEnvHolder.initialise (env);
        activity = GlobalRef (activity_);
        appFile = juceString (appFile_);
        appDataDir = juceString (appDataDir_);
        screenWidth = screenWidth_;
        screenHeight = screenHeight_;

        #define CREATE_JNI_CLASS(className, path) \
            className = (jclass) env->NewGlobalRef (env->FindClass (path)); \
            jassert (className != 0);
        JUCE_JNI_CLASSES (CREATE_JNI_CLASS);
        #undef CREATE_JNI_CLASS

        #define CREATE_JNI_METHOD(ownerClass, methodID, stringName, params) \
            methodID = env->GetMethodID (ownerClass, stringName, params); \
            jassert (methodID != 0);
        #define CREATE_JNI_STATICMETHOD(ownerClass, methodID, stringName, params) \
            methodID = env->GetStaticMethodID (ownerClass, stringName, params); \
            jassert (methodID != 0);
        #define CREATE_JNI_FIELD(ownerClass, fieldID, stringName, signature) \
            fieldID = env->GetFieldID (ownerClass, stringName, signature); \
            jassert (fieldID != 0);
        #define CREATE_JNI_STATICFIELD(ownerClass, fieldID, stringName, signature) \
            fieldID = env->GetStaticFieldID (ownerClass, stringName, signature); \
            jassert (fieldID != 0);
        JUCE_JNI_METHODS (CREATE_JNI_METHOD, CREATE_JNI_STATICMETHOD, CREATE_JNI_FIELD, CREATE_JNI_STATICFIELD);
        #undef CREATE_JNI_METHOD
    }

    void shutdown()
    {
        JNIEnv* env = getEnv();

        if (env != 0)
        {
            #define RELEASE_JNI_CLASS(className, path)    env->DeleteGlobalRef (className);
            JUCE_JNI_CLASSES (RELEASE_JNI_CLASS);
            #undef RELEASE_JNI_CLASS

            activity.clear();
        }
    }

    //==============================================================================
    GlobalRef activity;
    String appFile, appDataDir;
    int screenWidth, screenHeight;

    jobject createPaint()
    {
        const jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                                    | 2 /*FILTER_BITMAP_FLAG*/
                                    | 4 /*DITHER_FLAG*/
                                    | 128 /*SUBPIXEL_TEXT_FLAG*/;
        return getEnv()->NewObject (paintClass, paintClassConstructor, constructorFlags);
    }

    //==============================================================================
    #define DECLARE_JNI_CLASS(className, path) jclass className;
    JUCE_JNI_CLASSES (DECLARE_JNI_CLASS);
    #undef DECLARE_JNI_CLASS

    #define DECLARE_JNI_METHOD(ownerClass, methodID, stringName, params) jmethodID methodID;
    #define DECLARE_JNI_FIELD(ownerClass, fieldID, stringName, signature) jfieldID fieldID;
    JUCE_JNI_METHODS (DECLARE_JNI_METHOD, DECLARE_JNI_METHOD, DECLARE_JNI_FIELD, DECLARE_JNI_FIELD);
    #undef DECLARE_JNI_METHOD
};

static AndroidJavaCallbacks android;

//==============================================================================
#define JUCE_INCLUDED_FILE 1

// Now include the actual code files..
#include "juce_android_Misc.cpp"
#include "juce_android_SystemStats.cpp"
#include "../common/juce_posix_SharedCode.h"
#include "juce_android_Files.cpp"
#include "../common/juce_posix_NamedPipe.cpp"
#include "juce_android_Threads.cpp"
#include "juce_android_Network.cpp"
#include "juce_android_Messaging.cpp"
#include "juce_android_Fonts.cpp"
#include "juce_android_GraphicsContext.cpp"
#include "juce_android_Windowing.cpp"
#include "juce_android_FileChooser.cpp"
#include "juce_android_WebBrowserComponent.cpp"
#include "juce_android_OpenGLComponent.cpp"
#include "juce_android_Midi.cpp"
#include "juce_android_Audio.cpp"
#include "juce_android_CameraDevice.cpp"

END_JUCE_NAMESPACE

#endif
