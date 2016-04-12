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

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wparentheses"
 #pragma clang diagnostic ignored "-Wextra-tokens"
 #pragma clang diagnostic ignored "-Wcomment"
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wunused-parameter"
 #pragma clang diagnostic ignored "-Wunused"
#endif

#ifdef _MSC_VER
 #pragma warning (push)
// #pragma warning (disable : 4127)
#endif

#include "AU/CoreAudioUtilityClasses/AUBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUBuffer.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewControl.cpp"
#include "AU/CoreAudioUtilityClasses/AUCarbonViewDispatch.cpp"
#include "AU/CoreAudioUtilityClasses/AUDispatch.cpp"
#include "AU/CoreAudioUtilityClasses/AUInputElement.cpp"
#include "AU/CoreAudioUtilityClasses/AUMIDIBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUOutputBase.cpp"
#include "AU/CoreAudioUtilityClasses/AUOutputElement.cpp"
#include "AU/CoreAudioUtilityClasses/AUScopeElement.cpp"
#include "AU/CoreAudioUtilityClasses/CAAUParameter.cpp"
#include "AU/CoreAudioUtilityClasses/CAAudioChannelLayout.cpp"
#include "AU/CoreAudioUtilityClasses/CAMutex.cpp"
#include "AU/CoreAudioUtilityClasses/CAStreamBasicDescription.cpp"
#include "AU/CoreAudioUtilityClasses/CAVectorUnit.cpp"
#include "AU/CoreAudioUtilityClasses/CarbonEventHandler.cpp"
#include "AU/CoreAudioUtilityClasses/ComponentBase.cpp"
#include "AU/CoreAudioUtilityClasses/MusicDeviceBase.cpp"

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
 #pragma warning (pop)
#endif
