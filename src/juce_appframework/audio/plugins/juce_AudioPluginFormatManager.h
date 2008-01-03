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

#ifndef __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
#define __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__

#include "juce_AudioPluginFormat.h"
#include "../../application/juce_DeletedAtShutdown.h"
#include "../../../juce_core/basics/juce_Singleton.h"


//==============================================================================
/**
    This maintains a list of known AudioPluginFormats.

    @see AudioPluginFormat
*/
class JUCE_API  AudioPluginFormatManager  : public DeletedAtShutdown
{
public:
    //==============================================================================
    AudioPluginFormatManager() throw();

    /** Destructor. */
    ~AudioPluginFormatManager() throw();

    juce_DeclareSingleton_SingleThreaded (AudioPluginFormatManager, false);

    //==============================================================================
    /** Adds any formats that it knows about, e.g. VST.
    */
    void addDefaultFormats();

    //==============================================================================
    /** Returns the number of types of format that are available.

        Use getFormat() to get one of them.
    */
    int getNumFormats() throw();

    /** Returns one of the available formats.

        @see getNumFormats
    */
    AudioPluginFormat* getFormat (const int index) throw();

    //==============================================================================
    /** Adds a format to the list.

        The object passed in will be owned and deleted by the manager.
    */
    void addFormat (AudioPluginFormat* const format) throw();


    //==============================================================================
    /** Tries to load the type for this description, by trying all the formats
        that this manager knows about.

        The caller is responsible for deleting the object that is returned.

        If it can't load the plugin, it returns 0 and leaves a message in the
        errorMessage string.
    */
    AudioPluginInstance* createPluginInstance (const PluginDescription& description,
                                               String& errorMessage) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <AudioPluginFormat> formats;

    AudioPluginFormatManager (const AudioPluginFormatManager&);
    const AudioPluginFormatManager& operator= (const AudioPluginFormatManager&);
};



#endif   // __JUCE_AUDIOPLUGINFORMATMANAGER_JUCEHEADER__
