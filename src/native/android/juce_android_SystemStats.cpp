/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
JNIClassBase::JNIClassBase (const char* classPath_)
    : classPath (classPath_), classRef (0)
{
    getClasses().add (this);
}

JNIClassBase::~JNIClassBase()
{
    getClasses().removeValue (this);
}

Array<JNIClassBase*>& JNIClassBase::getClasses()
{
    static Array<JNIClassBase*> classes;
    return classes;
}

void JNIClassBase::initialise (JNIEnv* env)
{
    classRef = (jclass) env->NewGlobalRef (env->FindClass (classPath));
    jassert (classRef != 0);

    initialiseFields (env);
}

void JNIClassBase::release (JNIEnv* env)
{
    env->DeleteGlobalRef (classRef);
}

void JNIClassBase::initialiseAllClasses (JNIEnv* env)
{
    Array<JNIClassBase*>& classes = getClasses();
    for (int i = classes.size(); --i >= 0;)
        classes.getUnchecked(i)->initialise (env);
}

void JNIClassBase::releaseAllClasses (JNIEnv* env)
{
    Array<JNIClassBase*>& classes = getClasses();
    for (int i = classes.size(); --i >= 0;)
        classes.getUnchecked(i)->release (env);
}

jmethodID JNIClassBase::resolveMethod (JNIEnv* env, const char* methodName, const char* params)
{
    jmethodID m = env->GetMethodID (classRef, methodName, params);
    jassert (m != 0);
    return m;
}

jmethodID JNIClassBase::resolveStaticMethod (JNIEnv* env, const char* methodName, const char* params)
{
    jmethodID m = env->GetStaticMethodID (classRef, methodName, params);
    jassert (m != 0);
    return m;
}

jfieldID JNIClassBase::resolveField (JNIEnv* env, const char* fieldName, const char* signature)
{
    jfieldID f = env->GetFieldID (classRef, fieldName, signature);
    jassert (f != 0);
    return f;
}

jfieldID JNIClassBase::resolveStaticField (JNIEnv* env, const char* fieldName, const char* signature)
{
    jfieldID f = env->GetStaticFieldID (classRef, fieldName, signature);
    jassert (f != 0);
    return f;
}

//==============================================================================
class ThreadLocalJNIEnvHolder
{
public:
    ThreadLocalJNIEnvHolder()
        : jvm (nullptr)
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
        JNIEnv* env = nullptr;
        jvm->AttachCurrentThread (&env, 0);

        if (env != 0)
            addEnv (env);
    }

    void detach()
    {
        jvm->DetachCurrentThread();

        const pthread_t thisThread = pthread_self();

        SpinLock::ScopedLockType sl (addRemoveLock);
        for (int i = 0; i < maxThreads; ++i)
            if (threads[i] == thisThread)
                threads[i] = 0;
    }

    JNIEnv* get() const noexcept
    {
        const pthread_t thisThread = pthread_self();

        for (int i = 0; i < maxThreads; ++i)
            if (threads[i] == thisThread)
                return envs[i];

        return nullptr;
    }

    enum { maxThreads = 16 };

private:
    JavaVM* jvm;
    pthread_t threads [maxThreads];
    JNIEnv* envs [maxThreads];
    SpinLock addRemoveLock;

    void addEnv (JNIEnv* env)
    {
        SpinLock::ScopedLockType sl (addRemoveLock);

        if (get() == nullptr)
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

JNIEnv* getEnv() noexcept
{
    return threadLocalJNIEnvHolder.get();
}

//==============================================================================
AndroidSystem::AndroidSystem() : screenWidth (0), screenHeight (0)
{
}

void AndroidSystem::initialise (JNIEnv* env, jobject activity_,
                                jstring appFile_, jstring appDataDir_)
{
    JNIClassBase::initialiseAllClasses (env);

    threadLocalJNIEnvHolder.initialise (env);
    activity = GlobalRef (activity_);
    appFile = juceString (env, appFile_);
    appDataDir = juceString (env, appDataDir_);
}

void AndroidSystem::shutdown (JNIEnv* env)
{
    activity.clear();
    JNIClassBase::releaseAllClasses (env);
}

jobject AndroidSystem::createPaint (Graphics::ResamplingQuality quality)
{
    jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                            | 4 /*DITHER_FLAG*/
                            | 128 /*SUBPIXEL_TEXT_FLAG*/;

    if (quality > Graphics::lowResamplingQuality)
        constructorFlags |= 2; /*FILTER_BITMAP_FLAG*/

    return getEnv()->NewObject (Paint, Paint.constructor, constructorFlags);
}

const jobject AndroidSystem::createMatrix (JNIEnv* env, const AffineTransform& t)
{
    jobject m = env->NewObject (Matrix, Matrix.constructor);

    jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                         t.mat10, t.mat11, t.mat12,
                         0.0f, 0.0f, 1.0f };

    jfloatArray javaArray = env->NewFloatArray (9);
    env->SetFloatArrayRegion (javaArray, 0, 9, values);

    env->CallVoidMethod (m, Matrix.setValues, javaArray);
    env->DeleteLocalRef (javaArray);

    return m;
}

AndroidSystem android;

//==============================================================================
namespace AndroidStatsHelpers
{
    //==============================================================================
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
     STATICMETHOD (getProperty, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;")

    DECLARE_JNI_CLASS (SystemClass, "java/lang/System");
    #undef JNI_CLASS_MEMBERS

    //==============================================================================
    String getSystemProperty (const String& name)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (SystemClass,
                                                                                          SystemClass.getProperty,
                                                                                          javaString (name).get())));
    }
}

//==============================================================================
SystemStats::OperatingSystemType SystemStats::getOperatingSystemType()
{
    return Android;
}

String SystemStats::getOperatingSystemName()
{
    return "Android " + AndroidStatsHelpers::getSystemProperty ("os.version");
}

bool SystemStats::isOperatingSystem64Bit()
{
   #if JUCE_64BIT
    return true;
   #else
    return false;
   #endif
}

String SystemStats::getCpuVendor()
{
    return AndroidStatsHelpers::getSystemProperty ("os.arch");
}

int SystemStats::getCpuSpeedInMegaherz()
{
    return 0; // TODO
}

int SystemStats::getMemorySizeInMegabytes()
{
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (sysi.totalram * sysi.mem_unit / (1024 * 1024));

    return 0;
}

int SystemStats::getPageSize()
{
    return sysconf (_SC_PAGESIZE);
}

//==============================================================================
String SystemStats::getLogonName()
{
    const char* user = getenv ("USER");

    if (user == 0)
    {
        struct passwd* const pw = getpwuid (getuid());
        if (pw != 0)
            user = pw->pw_name;
    }

    return CharPointer_UTF8 (user);
}

String SystemStats::getFullUserName()
{
    return getLogonName();
}

String SystemStats::getComputerName()
{
    char name [256] = { 0 };
    if (gethostname (name, sizeof (name) - 1) == 0)
        return name;

    return String::empty;
}

//==============================================================================
SystemStats::CPUFlags::CPUFlags()
{
    // TODO
    hasMMX = false;
    hasSSE = false;
    hasSSE2 = false;
    has3DNow = false;

    numCpus = jmax (1, sysconf (_SC_NPROCESSORS_ONLN));
}

//==============================================================================
uint32 juce_millisecondsSinceStartup() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

int64 Time::getHighResolutionTicks() noexcept
{
    timespec t;
    clock_gettime (CLOCK_MONOTONIC, &t);

    return (t.tv_sec * (int64) 1000000) + (t.tv_nsec / 1000);
}

int64 Time::getHighResolutionTicksPerSecond() noexcept
{
    return 1000000;  // (microseconds)
}

double Time::getMillisecondCounterHiRes() noexcept
{
    return getHighResolutionTicks() * 0.001;
}

bool Time::setSystemTimeToThisTime() const
{
    jassertfalse;
    return false;
}

//==============================================================================
// This is an unsatisfactory workaround for a linker warning that appeared in NDK5c.
// If anyone actually understands what this symbol is for and why the linker gets confused by it,
// please let me know!
extern "C" { void* __dso_handle = 0; }

#endif
