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

bool File::isOnCDRomDrive() const
{
    return false;
}

bool File::isOnHardDisk() const
{
    return true;
}

bool File::isOnRemovableDrive() const
{
    return false;
}

String File::getVersion() const
{
    return {};
}

static File getSpecialFile (jmethodID type)
{
    return File (juceString (LocalRef<jstring> ((jstring) getEnv()->CallStaticObjectMethod (JuceAppActivity, type))));
}

File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        case userApplicationDataDirectory:
        case userDesktopDirectory:
        case commonApplicationDataDirectory:
            return File (android.appDataDir);

        case userDocumentsDirectory:
        case commonDocumentsDirectory:  return getSpecialFile (JuceAppActivity.getDocumentsFolder);
        case userPicturesDirectory:     return getSpecialFile (JuceAppActivity.getPicturesFolder);
        case userMusicDirectory:        return getSpecialFile (JuceAppActivity.getMusicFolder);
        case userMoviesDirectory:       return getSpecialFile (JuceAppActivity.getMoviesFolder);

        case globalApplicationsDirectory:
            return File ("/system/app");

        case tempDirectory:
            return File (android.appDataDir).getChildFile (".temp");

        case invokedExecutableFile:
        case currentExecutableFile:
        case currentApplicationFile:
        case hostApplicationPath:
            return juce_getExecutableFile();

        default:
            jassertfalse; // unknown type?
            break;
    }

    return {};
}

bool File::moveToTrash() const
{
    if (! exists())
        return true;

    // TODO
    return false;
}

JUCE_API bool JUCE_CALLTYPE Process::openDocument (const String& fileName, const String&)
{
    const LocalRef<jstring> t (javaString (fileName));
    android.activity.callVoidMethod (JuceAppActivity.launchURL, t.get());
    return true;
}

void File::revealToUser() const
{
}
