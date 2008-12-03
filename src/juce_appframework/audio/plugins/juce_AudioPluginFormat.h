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

#ifndef __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
#define __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__

#include "juce_AudioPluginInstance.h"
#include "../../../juce_core/io/files/juce_FileSearchPath.h"
class PluginDescription;


//==============================================================================
/**
    The base class for a type of plugin format, such as VST, AudioUnit, LADSPA, etc.

    Use the static getNumFormats() and getFormat() calls to find the types
    of format that are available.
*/
class JUCE_API  AudioPluginFormat
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~AudioPluginFormat();

    //==============================================================================
    /** Returns the format name.

        E.g. "VST", "AudioUnit", etc.
    */
    virtual const String getName() const = 0;

    /** This tries to create descriptions for all the plugin types available in
        a binary module file.

        The file will be some kind of DLL or bundle.

        Normally there will only be one type returned, but some plugins
        (e.g. VST shells) can use a single DLL to create a set of different plugin
        subtypes, so in that case, each subtype is returned as a separate object.
    */
    virtual void findAllTypesForFile (OwnedArray <PluginDescription>& results,
                                      const String& fileOrIdentifier) = 0;

    /** Tries to recreate a type from a previously generated PluginDescription.

        @see PluginDescription::createInstance
    */
    virtual AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc) = 0;

    /** Should do a quick check to see if this file or directory might be a plugin of
        this format.

        This is for searching for potential files, so it shouldn't actually try to
        load the plugin or do anything time-consuming.
    */
    virtual bool fileMightContainThisPluginType (const String& fileOrIdentifier) = 0;

    /** Returns a readable version of the name of the plugin that this identifier refers to.
    */
    virtual const String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) = 0;

    /** Checks whether this plugin could possibly be loaded.

        It doesn't actually need to load it, just to check whether the file or component
        still exists.
    */
    virtual bool doesPluginStillExist (const PluginDescription& desc) = 0;

    /** Searches a suggested set of directories for any plugins in this format.

        The path might be ignored, e.g. by AUs, which are found by the OS rather
        than manually.
    */
    virtual const StringArray searchPathsForPlugins (const FileSearchPath& directoriesToSearch,
                                                     const bool recursive) = 0;

    /** Returns the typical places to look for this kind of plugin.

        Note that if this returns no paths, it means that the format can't be scanned-for
        (i.e. it's an internal format that doesn't live in files)
    */
    virtual const FileSearchPath getDefaultLocationsToSearch() = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    AudioPluginFormat() throw();

    AudioPluginFormat (const AudioPluginFormat&);
    const AudioPluginFormat& operator= (const AudioPluginFormat&);
};


#endif   // __JUCE_AUDIOPLUGINFORMAT_JUCEHEADER__
