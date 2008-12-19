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

#include "juce_RTAS_DigiCode_Header.h"

/*
    This file is used to include and build the required digidesign CPP files without your project
    needing to reference the files directly. Because these files will be found via your include path,
    this means that the project doesn't have to change to cope with people's SDKs being in different
    locations.

    Important note on Windows: In your project settings for the three juce_RTAS_DigiCode.cpp files and
    the juce_RTAS_Wrapper.cpp file, you need to set the calling convention to "__stdcall".
    If you don't do this, you'll get some unresolved externals and will spend a long time wondering what's
    going on... All the other files in your project can be set to use the normal __cdecl convention.

    If you get an error building the includes statements below, check your paths - there's a full
    list of the necessary Digidesign paths in juce_RTAS_Wrapper.cpp
*/

#if JucePlugin_Build_RTAS


#if WINDOWS_VERSION
//==============================================================================

 #undef _UNICODE
 #undef UNICODE

 #if JucePlugin_Build_VST

 #define DllMain DllMainRTAS
 #include "DLLMain.cpp"
 #undef DllMain

 extern BOOL WINAPI DllMainVST (HINSTANCE instance, DWORD dwReason, LPVOID);

 // This overloaded DllMain can work as either an RTAS or a VST..
 extern "C" BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD ul_reason_for_call, LPVOID lpReserved)
 {
    if (GetModuleHandle ("DAE.DLL") != 0)
        return DllMainRTAS (hInstance, ul_reason_for_call, lpReserved);
    else
        return DllMainVST (hInstance, ul_reason_for_call, lpReserved);
 }
 #else
 #include "DLLMain.cpp"
 #endif

 #include "DefaultSwap.cpp"

#else
 //==============================================================================
 #include "PlugInInitialize.cpp"
 #include "Dispatcher.cpp"
#endif

#else

#if _MSC_VER
 // (defining these stubs just makes it easier to quickly turn off the RTAS build without removing
 // the exports table from your project settings)
 short __stdcall NewPlugIn (void*)                          { return 0; }
 short __stdcall _PI_GetRoutineDescriptor (long, void*)     { return 0; }
#endif

#endif
