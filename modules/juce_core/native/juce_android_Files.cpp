/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

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
    return String();
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

    return File();
}

bool File::moveToTrash() const
{
    if (! exists())
        return true;

    // TODO
    return false;
}

JUCE_API bool JUCE_CALLTYPE Process::openDocument (const String& fileName, const String& parameters)
{
    const LocalRef<jstring> t (javaString (fileName));
    android.activity.callVoidMethod (JuceAppActivity.launchURL, t.get());
    return true;
}

void File::revealToUser() const
{
}
