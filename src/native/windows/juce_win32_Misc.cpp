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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
bool juce_IsRunningInWine()
{
    HMODULE ntdll = GetModuleHandle (_T("ntdll.dll"));
    return ntdll != 0 && GetProcAddress (ntdll, "wine_get_version") != 0;
}

//==============================================================================
String JUCE_CALLTYPE Process::getCurrentCommandLineParams()
{
    return String (CharacterFunctions::findEndOfToken (CharPointer_UTF16 (GetCommandLineW()),
                                                       CharPointer_UTF16 (L" "),
                                                       CharPointer_UTF16 (L"\""))).trimStart();
}

//==============================================================================
static void* currentModuleHandle = nullptr;

void* Process::getCurrentModuleInstanceHandle() noexcept
{
    if (currentModuleHandle == nullptr)
        currentModuleHandle = GetModuleHandle (0);

    return currentModuleHandle;
}

void Process::setCurrentModuleInstanceHandle (void* const newHandle) noexcept
{
    currentModuleHandle = newHandle;
}

#endif
