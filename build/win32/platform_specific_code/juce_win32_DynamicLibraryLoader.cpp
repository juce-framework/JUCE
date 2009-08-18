/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

#include "juce_win32_DynamicLibraryLoader.h"


//==============================================================================
DynamicLibraryLoader::DynamicLibraryLoader (const String& name)
{
    libHandle = LoadLibrary (name);
}

DynamicLibraryLoader::~DynamicLibraryLoader()
{
    FreeLibrary ((HMODULE) libHandle);
}

void* DynamicLibraryLoader::findProcAddress (const String& functionName)
{
    return (void*) GetProcAddress ((HMODULE) libHandle, functionName);
}


#endif
