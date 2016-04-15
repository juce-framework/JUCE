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
