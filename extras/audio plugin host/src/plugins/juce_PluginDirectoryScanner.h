/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
#define __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__

#include "juce_KnownPluginList.h"


//==============================================================================
/**
    Scans a directory for plugins, and adds them to a KnownPluginList.

    To use one of these, create it and call scanNextFile() repeatedly, until
    it returns false.
*/
class PluginDirectoryScanner
{
public:
    //==============================================================================
    /**
        Creates a scanner.

        @param listToAddResultsTo       this will get the new types added to it.
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
                            FileSearchPath directoriesToSearch,
                            const bool searchRecursively,
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
    bool scanNextFile (const bool dontRescanIfAlreadyInList);

    /** Returns the file that will be scanned during the next call to scanNextFile().

        This is handy if you want to show the user which file is currently getting
        scanned.
    */
    const File getNextPluginFileThatWillBeScanned() const throw();

    /** Returns the estimated progress, between 0 and 1.
    */
    float getProgress() const                                       { return progress; }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    KnownPluginList& list;
    OwnedArray <File> filesToScan;
    File deadMansPedalFile;
    int nextIndex;
    float progress;

    const StringArray getDeadMansPedalFile() throw();
    void setDeadMansPedalFile (const StringArray& newContents) throw();
};


#endif   // __JUCE_PLUGINDIRECTORYSCANNER_JUCEHEADER__
