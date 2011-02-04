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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

END_JUCE_NAMESPACE
extern JUCE_NAMESPACE::JUCEApplication* juce_CreateApplication(); // (from START_JUCE_APPLICATION)
BEGIN_JUCE_NAMESPACE

//==============================================================================
JUCE_JNI_CALLBACK (JuceAppActivity, launchApp, void, (JNIEnv* env, jobject activity, int screenWidth, int screenHeight))
{
    android.initialise (env, activity, screenWidth, screenHeight);

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
    // TODO
}

//==============================================================================
void Logger::outputDebugString (const String& text)
{
    android.env->CallStaticVoidMethod (android.activityClass, android.printToConsole,
                                       android.javaString (text));
}

//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    // TODO
}

const String SystemClipboard::getTextFromClipboard()
{
    String result;

    // TODO

    return result;
}


#endif
