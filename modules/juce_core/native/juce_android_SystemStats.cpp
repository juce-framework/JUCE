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

JNIClassBase::JNIClassBase (const char* classPath_)
    : classPath (classPath_), classRef (0)
{
    getClasses().add (this);
}

JNIClassBase::~JNIClassBase()
{
    getClasses().removeFirstMatchingValue (this);
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
    const Array<JNIClassBase*>& classes = getClasses();
    for (int i = classes.size(); --i >= 0;)
        classes.getUnchecked(i)->initialise (env);
}

void JNIClassBase::releaseAllClasses (JNIEnv* env)
{
    const Array<JNIClassBase*>& classes = getClasses();
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
ThreadLocalJNIEnvHolder threadLocalJNIEnvHolder;

#if JUCE_DEBUG
static bool systemInitialised = false;
#endif

JNIEnv* getEnv() noexcept
{
   #if JUCE_DEBUG
    if (! systemInitialised)
    {
        DBG ("*** Call to getEnv() when system not initialised");
        jassertfalse;
        exit (0);
    }
   #endif

    return threadLocalJNIEnvHolder.getOrAttach();
}

extern "C" jint JNI_OnLoad (JavaVM*, void*)
{
    return JNI_VERSION_1_2;
}

//==============================================================================
AndroidSystem::AndroidSystem() : screenWidth (0), screenHeight (0)
{
}

void AndroidSystem::initialise (JNIEnv* env, jobject activity_,
                                jstring appFile_, jstring appDataDir_)
{
    screenWidth = screenHeight = 0;
    JNIClassBase::initialiseAllClasses (env);

    threadLocalJNIEnvHolder.initialise (env);
   #if JUCE_DEBUG
    systemInitialised = true;
   #endif

    activity = GlobalRef (activity_);
    appFile = juceString (env, appFile_);
    appDataDir = juceString (env, appDataDir_);
}

void AndroidSystem::shutdown (JNIEnv* env)
{
    activity.clear();

   #if JUCE_DEBUG
    systemInitialised = false;
   #endif

    JNIClassBase::releaseAllClasses (env);
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

    //==============================================================================
    String getLocaleValue (bool isRegion)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (JuceAppActivity,
                                                                                          JuceAppActivity.getLocaleValue,
                                                                                          isRegion)));
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
   #if __ANDROID_API__ >= 9
    struct sysinfo sysi;

    if (sysinfo (&sysi) == 0)
        return (sysi.totalram * sysi.mem_unit / (1024 * 1024));
   #endif

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


String SystemStats::getUserLanguage()    { return AndroidStatsHelpers::getLocaleValue (false); }
String SystemStats::getUserRegion()      { return AndroidStatsHelpers::getLocaleValue (true); }
String SystemStats::getDisplayLanguage() { return getUserLanguage(); }

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
