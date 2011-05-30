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

END_JUCE_NAMESPACE
extern JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication(); // (from START_JUCE_APPLICATION)
BEGIN_JUCE_NAMESPACE

//==============================================================================
JUCE_JNI_CALLBACK (JuceAppActivity, launchApp, void, (JNIEnv* env, jobject activity,
                                                      jstring appFile, jstring appDataDir))
{
    android.initialise (env, activity, appFile, appDataDir);

    DBG (SystemStats::getJUCEVersion());

    JUCEApplication::createInstance = &juce_CreateApplication;

    initialiseJuce_GUI();

    if (! JUCEApplication::createInstance()->initialiseApp (String::empty))
        exit (0);
}

JUCE_JNI_CALLBACK (JuceAppActivity, quitApp, void, (JNIEnv* env, jobject activity))
{
    JUCEApplication::appWillTerminateByForce();

    android.shutdown();
}

//==============================================================================
void PlatformUtilities::beep()
{
}

//==============================================================================
void Logger::outputDebugString (const String& text)
{
    JNIEnv* const env = getEnv();

    if (env != nullptr)
        env->CallStaticVoidMethod (android.activityClass, android.printToConsole,
                                   javaString (text).get());
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    const LocalRef<jstring> t (javaString (text));
    android.activity.callVoidMethod (android.setClipboardContent, t.get());
}

String SystemClipboard::getTextFromClipboard()
{
    const LocalRef<jstring> text ((jstring) android.activity.callObjectMethod (android.getClipboardContent));
    return juceString (text);
}


#endif
