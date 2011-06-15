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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../text/juce_String.h"
#include "juce_SystemStats.h"
#include "juce_Time.h"
#include "juce_PlatformUtilities.h"


//==============================================================================
const SystemStats::CPUFlags& SystemStats::getCPUFlags()
{
    static CPUFlags cpuFlags;
    return cpuFlags;
}

String SystemStats::getJUCEVersion()
{
    // Some basic tests, to keep an eye on things and make sure these types work ok
    // on all platforms. Let me know if any of these assertions fail on your system!
    static_jassert (sizeof (pointer_sized_int) == sizeof (void*));
    static_jassert (sizeof (int8) == 1);
    static_jassert (sizeof (uint8) == 1);
    static_jassert (sizeof (int16) == 2);
    static_jassert (sizeof (uint16) == 2);
    static_jassert (sizeof (int32) == 4);
    static_jassert (sizeof (uint32) == 4);
    static_jassert (sizeof (int64) == 8);
    static_jassert (sizeof (uint64) == 8);

    // (these confusing macros convert numbers into a single string literal)
    #define JUCE_STRINGIFYVERSION2(a) #a
    #define JUCE_STRINGIFYVERSION(a) JUCE_STRINGIFYVERSION2(a)

    return "JUCE v" JUCE_STRINGIFYVERSION(JUCE_MAJOR_VERSION)
                "." JUCE_STRINGIFYVERSION(JUCE_MINOR_VERSION)
                "." JUCE_STRINGIFYVERSION(JUCE_BUILDNUMBER);

    #undef JUCE_STRINGIFYVERSION
    #undef JUCE_STRINGIFYVERSION2
}

#if JUCE_DEBUG && ! JUCE_ANDROID
 struct JuceVersionPrinter
 {
     JuceVersionPrinter()
     {
         DBG (SystemStats::getJUCEVersion());
     }
 };

 static JuceVersionPrinter juceVersionPrinter;
#endif


END_JUCE_NAMESPACE
