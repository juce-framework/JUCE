/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
SystemStats::CPUFlags SystemStats::cpuFlags;

const String SystemStats::getJUCEVersion()
{
    return "JUCE v" + String (JUCE_MAJOR_VERSION)
              + "." + String (JUCE_MINOR_VERSION)
              + "." + String (JUCE_BUILDNUMBER);
}

//==============================================================================
#ifdef JUCE_DLL
 void* juce_Malloc (int size)                   { return malloc (size); }
 void* juce_Calloc (int size)                   { return calloc (1, size); }
 void* juce_Realloc (void* block, int size)     { return realloc (block, size); }
 void juce_Free (void* block)                   { free (block); }

 #if JUCE_MSVC && JUCE_CHECK_MEMORY_LEAKS
  void* juce_DebugMalloc (int size, const char* file, int line)                 { return _malloc_dbg  (size, _NORMAL_BLOCK, file, line); }
  void* juce_DebugCalloc (int size, const char* file, int line)                 { return _calloc_dbg  (1, size, _NORMAL_BLOCK, file, line); }
  void* juce_DebugRealloc (void* block, int size, const char* file, int line)   { return _realloc_dbg  (block, size, _NORMAL_BLOCK, file, line); }
  void juce_DebugFree (void* block)                                             { _free_dbg (block, _NORMAL_BLOCK); }
 #endif
#endif

END_JUCE_NAMESPACE
