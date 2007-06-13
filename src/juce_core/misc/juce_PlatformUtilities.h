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

#ifndef __JUCE_PLATFORMUTILITIES_JUCEHEADER__
#define __JUCE_PLATFORMUTILITIES_JUCEHEADER__

#include "../text/juce_StringArray.h"


//==============================================================================
/**
    A collection of miscellaneous platform-specific utilities.

*/
class JUCE_API  PlatformUtilities
{
public:
    //==============================================================================
    /** Plays the operating system's default alert 'beep' sound. */
    static void beep();

#if JUCE_MAC || DOXYGEN
    //==============================================================================
    /** MAC ONLY - Turns a String into a pascal string. */
    static void copyToStr255 (Str255& d, const String& s);
    /** MAC ONLY - Turns a String into a pascal string. */
    static void copyToStr63 (Str63& d, const String& s);

    /** MAC ONLY - Turns a Core CF String into a juce one. */
    static const String cfStringToJuceString (CFStringRef cfString);

    /** MAC ONLY - Turns a juce string into a Core CF one. */
    static CFStringRef juceStringToCFString (const String& s);

    /** MAC ONLY - Converts a UTF16 string to a Juce String. */
    static const String convertUTF16ToString (const UniChar* utf16);

    /** MAC ONLY - Turns a file path into an FSSpec, returning true if it succeeds. */
    static bool makeFSSpecFromPath (FSSpec* destFSSpec, const String& path);

    /** MAC ONLY - Turns a file path into an FSRef, returning true if it succeeds. */
    static bool makeFSRefFromPath (FSRef* destFSRef, const String& path);

    /** MAC ONLY - Turns an FSRef into a juce string path. */
    static const String makePathFromFSRef (FSRef* file);

    /** MAC ONLY - Converts any decomposed unicode characters in a string into
        their precomposed equivalents.
    */
    static const String convertToPrecomposedUnicode (const String& s);

    /** MAC ONLY - Gets the type of a file from the file's resources. */
    static OSType getTypeOfFile (const String& filename);

    /** MAC ONLY - Returns true if this file is actually a bundle. */
    static bool isBundle (const String& filename);

#endif


#if JUCE_WIN32 || DOXYGEN
    //==============================================================================
    // Some registry helper functions:

    /** WIN32 ONLY - Returns a string from the registry.

        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
    */
    static const String getRegistryValue (const String& regValuePath,
                                          const String& defaultValue = String::empty);

    /** WIN32 ONLY - Sets a registry value as a string.

        This will take care of creating any groups needed to get to the given
        registry value.
    */
    static void setRegistryValue (const String& regValuePath,
                                  const String& value);

    /** WIN32 ONLY - Returns true if the given value exists in the registry. */
    static bool registryValueExists (const String& regValuePath);

    /** WIN32 ONLY - Deletes a registry value. */
    static void deleteRegistryValue (const String& regValuePath);

    /** WIN32 ONLY - Deletes a registry key (which is registry-talk for 'folder'). */
    static void deleteRegistryKey (const String& regKeyPath);


    /** WIN32 ONLY - This returns the HINSTANCE of the current module.

        In a normal Juce application this will be set to the module handle
        of the application executable.

        If you're writing a DLL using Juce and plan to use any Juce messaging or
        windows, you'll need to make sure you use the setCurrentModuleInstanceHandle()
        to set the correct module handle in your DllMain() function, because
        the win32 system relies on the correct instance handle when opening windows.
    */
    static void* JUCE_CALLTYPE getCurrentModuleInstanceHandle();

    /** WIN32 ONLY - Sets a new module handle to be used by the library.

        @see getCurrentModuleInstanceHandle()
    */
    static void JUCE_CALLTYPE setCurrentModuleInstanceHandle (void* newHandle);

#endif

    /** Clears the floating point unit's flags.

        Only has an effect under win32, currently.
    */
    static void fpuReset();


#if JUCE_LINUX || DOXYGEN
    //==============================================================================

#endif
};


#endif   // __JUCE_PLATFORMUTILITIES_JUCEHEADER__
