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

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioIODeviceType.h"


//==============================================================================
AudioIODeviceType::AudioIODeviceType (const tchar* const name)
    : typeName (name)
{
}

AudioIODeviceType::~AudioIODeviceType()
{
}

//==============================================================================
extern AudioIODeviceType* juce_createDefaultAudioIODeviceType();

#if JUCE_ASIO && JUCE_WIN32
  extern AudioIODeviceType* juce_createASIOAudioIODeviceType();
#endif

void AudioIODeviceType::createDeviceTypes (OwnedArray <AudioIODeviceType>& list)
{
    AudioIODeviceType* const defaultDeviceType = juce_createDefaultAudioIODeviceType();

    if (defaultDeviceType != 0)
        list.add (defaultDeviceType);

#if JUCE_ASIO && JUCE_WIN32
    list.add (juce_createASIOAudioIODeviceType());
#endif
}


END_JUCE_NAMESPACE
