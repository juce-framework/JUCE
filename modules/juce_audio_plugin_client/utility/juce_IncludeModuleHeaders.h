/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_INCLUDEMODULEHEADERS_H_INCLUDED
#define JUCE_INCLUDEMODULEHEADERS_H_INCLUDED

#include "../juce_audio_plugin_client.h"

using namespace juce;

namespace juce
{
    #define Component juce::Component

   #if JUCE_MAC
    #define Point juce::Point
    void repostCurrentNSEvent();
   #endif

    //==============================================================================
    inline const PluginHostType& getHostType()
    {
        static PluginHostType hostType;
        return hostType;
    }
}

extern AudioProcessor* JUCE_API JUCE_CALLTYPE createPluginFilterOfType (AudioProcessor::WrapperType);

#endif   // JUCE_INCLUDEMODULEHEADERS_H_INCLUDED
