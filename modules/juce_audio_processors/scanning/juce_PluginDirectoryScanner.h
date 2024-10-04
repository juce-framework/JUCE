/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Scans a directory for plugins, and adds them to a KnownPluginList.

    To use one of these, create it and call scanNextFile() repeatedly, until
    it returns false.

    @tags{Audio}
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
        @param deadMansPedalFile        if this isn't File(), then it will be used as a file
                                        to store the names of any plugins that crash during
                                        initialisation. If there are any plugins listed in it,
                                        then these will always be scanned after all other possible
                                        files have been tried - in this way, even if there's a few
                                        dodgy plugins in your path, then a couple of rescans
                                        will still manage to find all the proper plugins.
                                        It's probably best to choose a file in the user's
                                        application data directory (alongside your app's
                                        settings file) for this. The file format it uses
                                        is just a list of filenames of the modules that
                                        failed.
       @param allowPluginsWhichRequireAsynchronousInstantiation
                                        If this is false then the scanner will exclude plug-ins
                                        asynchronous creation - such as AUv3 plug-ins.
    */
    PluginDirectoryScanner (KnownPluginList& listToAddResultsTo,
                            AudioPluginFormat& formatToLookFor,
                            FileSearchPath directoriesToSearch,
                            bool searchRecursively,
                            const File& deadMansPedalFile,
                            bool allowPluginsWhichRequireAsynchronousInstantiation = false);

    /** Destructor. */
    ~PluginDirectoryScanner();

    //==============================================================================
    /** Sets a specific list of filesOrIdentifiersToScan to scan.
        N.B. This list must match the format passed to the constructor.
        @see AudioPluginFormat::searchPathsForPlugins
    */
    void setFilesOrIdentifiersToScan (const StringArray& filesOrIdentifiersToScan);

    /** Tries the next likely-looking file.

        If dontRescanIfAlreadyInList is true, then the file will only be loaded and
        re-tested if it's not already in the list, or if the file's modification
        time has changed since the list was created. If dontRescanIfAlreadyInList is
        false, the file will always be reloaded and tested.
        The nameOfPluginBeingScanned will be updated to the name of the plugin being
        scanned before the scan starts.

        Returns false when there are no more files to try.
    */
    bool scanNextFile (bool dontRescanIfAlreadyInList,
                       String& nameOfPluginBeingScanned);

    /** Skips over the next file without scanning it.
        Returns false when there are no more files to try.
    */
    bool skipNextFile();

    /** Returns the description of the plugin that will be scanned during the next
        call to scanNextFile().

        This is handy if you want to show the user which file is currently getting
        scanned.
    */
    String getNextPluginFileThatWillBeScanned() const;

    /** Returns the estimated progress, between 0 and 1. */
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
    Atomic<int> nextIndex;
    std::atomic<float> progress { 0.0f };
    const bool allowAsync;

    void updateProgress();
    void setDeadMansPedalFile (const StringArray& newContents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginDirectoryScanner)
};

} // namespace juce
