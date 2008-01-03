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

#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
#define __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__

#include "../processors/juce_AudioProcessor.h"
#include "juce_PluginDescription.h"


//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

    @see AudioProcessor, AudioPluginFormat
*/
class JUCE_API  AudioPluginInstance   : public AudioProcessor
{
public:
    //==============================================================================
    /** Destructor.

        Make sure that you delete any UI components that belong to this plugin before
        deleting the plugin.
    */
    virtual ~AudioPluginInstance();

    //==============================================================================
    /** Fills-in the appropriate parts of this plugin description object.
    */
    virtual void fillInPluginDescription (PluginDescription& description) const = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    AudioPluginInstance();

    AudioPluginInstance (const AudioPluginInstance&);
    const AudioPluginInstance& operator= (const AudioPluginInstance&);
};


#endif   // __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
