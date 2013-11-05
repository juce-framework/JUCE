/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

JNIClassBase::JNIClassBase (const char* cp)   : classPath (cp), classRef (0)
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
        std::exit (EXIT_FAILURE);
    }
   #endif

    return threadLocalJNIEnvHolder.getOrAttach();
}

extern "C" jint JNI_OnLoad (JavaVM*, void*)
{
    return JNI_VERSION_1_2;
}

//==============================================================================
AndroidSystem::AndroidSystem() : screenWidth (0), screenHeight (0), dpi (160)
{
}

void AndroidSystem::initialise (JNIEnv* env, jobject act, jstring file, jstring dataDir)
{
    screenWidth = screenHeight = 0;
    dpi = 160;
    JNIClassBase::initialiseAllClasses (env);

    threadLocalJNIEnvHolder.initialise (env);
   #if JUCE_DEBUG
    systemInitialised = true;
   #endif

    activity = GlobalRef (act);
    appFile = juceString (env, file);
    appDataDir = juceString (env, dataDir);
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
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
     STATICMETHOD (getProperty, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;")

    DECLARE_JNI_CLASS (SystemClass, "java/lang/System");
    #undef JNI_CLASS_MEMBERS

    String getSystemProperty (const String& name)
    {
        return juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (SystemClass,
                                                                                          SystemClass.getProperty,
                                                                                          javaString (name).get())));
    }

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

String SystemStats::getDeviceDescription()
{
    return String::empty;
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
    if (const char* user = getenv ("USER"))
        return CharPointer_UTF8 (user);

    if (struct passwd* const pw = getpwuid (getuid()))
        return CharPointer_UTF8 (pw->pw_name);

    return String::empty;
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
void CPUInformation::initialise() noexcept
{
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
