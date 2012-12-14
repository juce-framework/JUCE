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

#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
#define __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__

#include "juce_KnownPluginList.h"
#include "../format/juce_AudioPluginFormatManager.h"


//==============================================================================
/**
    Scans a directory for plugins, and adds them to a KnownPluginList.

    To use one of these, create it and call scanNextFile() repeatedly, until
    it returns false.
*/
class JUCE_API  PluginDirectoryScanner
{
public:
    //==============================================================================
    /**
        Creates a scanner.

        @param listToAddResultsTo       this will get the new types added to it.
        @param formatToLookFor          this is the type of format that you want to look for
        @param directoriesToSearch      the path to search
        @param searchRecursively        true to search recursively
        @param deadMansPedalFile        if this isn't File::nonexistent, then it will
                                        be used as a file to store the names of any plugins
                                        that crash during initialisation. If there are
                                        any plugins listed in it, then these will always
                                        be scanned after all other possible files have
                                        been tried - in this way, even if there's a few
                                        dodgy plugins in your path, then a couple of rescans
                                        will still manage to find all the proper plugins.
                                        It's probably best to choose a file in the user's
                                        application data directory (alongside your app's
                                        settings file) for this. The file format it uses
                                        is just a list of filenames of the modules that
                                        failed.
    */
    PluginDirectoryScanner (KnownPluginList& listToAddResultsTo,
                            AudioPluginFormat& formatToLookFor,
                            FileSearchPath directoriesToSearch,
                            bool searchRecursively,
                            const File& deadMansPedalFile);

    /** Destructor. */
    ~PluginDirectoryScanner();

    //==============================================================================
    /** Tries the next likely-looking file.

        If dontRescanIfAlreadyInList is true, then the file will only be loaded and
        re-tested if it's not already in the list, or if the file's modification
        time has changed since the list was created. If dontRescanIfAlreadyInList is
        false, the file will always be reloaded and tested.

        Returns false when there are no more files to try.
    */
    bool scanNextFile (bool dontRescanIfAlreadyInList);

    /** Skips over the next file without scanning it.
        Returns false when there are no more files to try.
    */
    bool skipNextFile();

    /** Returns the description of the plugin that will be scanned during the next
        call to scanNextFile().

        This is handy if you want to show the user which file is currently getting
        scanned.
    */
    const String getNextPluginFileThatWillBeScanned() const;

    /** Returns the estimated progress, between 0 and 1.
    */
    float getProgress() const                                       { return progress; }

    /** This returns a list of all the filenames of things that looked like being
        a plugin file, but which failed to open for some reason.
    */
    const StringArray& getFailedFiles() const noexcept              { return failedFiles; }

    /** Reads the given dead-mans-pedal file and applies its contents to the list. */
    static void applyBlacklistingsFromDeadMansPedal (KnownPluginList& listToApplyTo,
                                                     const File& deadMansPedalFile);

private:
    //==============================================================================
    KnownPluginList& list;
    AudioPluginFormat& format;
    StringArray filesOrIdentifiersToScan;
    File deadMansPedalFile;
    StringArray failedFiles;
    int nextIndex;
    float progress;

    void setDeadMansPedalFile (const StringArray& newContents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginDirectoryScanner)
};


#endif   // __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
