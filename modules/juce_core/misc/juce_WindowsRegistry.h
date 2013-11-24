/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_WINDOWSREGISTRY_H_INCLUDED
#define JUCE_WINDOWSREGISTRY_H_INCLUDED

#if JUCE_WINDOWS || DOXYGEN

/**
    Contains some static helper functions for manipulating the MS Windows registry
    (Only available on Windows, of course!)
*/
class WindowsRegistry
{
public:
    /** These values can be used to specify whether the 32- or 64-bit registry should be used.
        When running on a 32-bit OS, there is no 64-bit registry, so the mode will be ignored.
    */
    enum WoW64Mode
    {
        /** Default handling: 32-bit apps will use the 32-bit registry, and 64-bit apps
            will use the 64-bit registry. */
        WoW64_Default = 0,

        /** Always use the 64-bit registry store. (KEY_WOW64_64KEY). */
        WoW64_64bit  = 0x100,

        /** Always use the 32-bit registry store. (KEY_WOW64_32KEY). */
        WoW64_32bit  = 0x200
    };

    //==============================================================================
    /** Returns a string from the registry.
        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
    */
    static String getValue (const String& regValuePath,
                            const String& defaultValue = String::empty,
                            WoW64Mode mode = WoW64_Default);

    /** Reads a binary block from the registry.
        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
        @returns a DWORD indicating the type of the key.
    */
    static uint32 getBinaryValue (const String& regValuePath, MemoryBlock& resultData, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a string.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool setValue (const String& regValuePath, const String& value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a DWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool setValue (const String& regValuePath, uint32 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a QWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool setValue (const String& regValuePath, uint64 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a binary block.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool setValue (const String& regValuePath, const MemoryBlock& value, WoW64Mode mode = WoW64_Default);

    /** Returns true if the given value exists in the registry. */
    static bool valueExists (const String& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Returns true if the given key exists in the registry. */
    static bool keyExists (const String& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry value. */
    static void deleteValue (const String& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry key (which is registry-talk for 'folder'). */
    static void deleteKey (const String& regKeyPath, WoW64Mode mode = WoW64_Default);

    /** Creates a file association in the registry.

        This lets you set the executable that should be launched by a given file extension.
        @param fileExtension        the file extension to associate, including the
                                    initial dot, e.g. ".txt"
        @param symbolicDescription  a space-free short token to identify the file type
        @param fullDescription      a human-readable description of the file type
        @param targetExecutable     the executable that should be launched
        @param iconResourceNumber   the icon that gets displayed for the file type will be
                                    found by looking up this resource number in the
                                    executable. Pass 0 here to not use an icon
        @param registerForCurrentUserOnly   if false, this will try to register the association
                                    for all users (you might not have permission to do this
                                    unless running in an installer). If true, it will register the
                                    association in HKEY_CURRENT_USER.
        @param mode                 the WoW64 mode to use for choosing the database
    */
    static bool registerFileAssociation (const String& fileExtension,
                                         const String& symbolicDescription,
                                         const String& fullDescription,
                                         const File& targetExecutable,
                                         int iconResourceNumber,
                                         bool registerForCurrentUserOnly,
                                         WoW64Mode mode = WoW64_Default);

    // DEPRECATED: use the other methods with a WoW64Mode parameter of WoW64_64bit instead.
    JUCE_DEPRECATED (static String getValueWow64 (const String&, const String& defaultValue = String::empty));
    JUCE_DEPRECATED (static bool valueExistsWow64 (const String&));
    JUCE_DEPRECATED (static bool keyExistsWow64 (const String&));

private:
    WindowsRegistry() JUCE_DELETED_FUNCTION;
    JUCE_DECLARE_NON_COPYABLE (WindowsRegistry)
};

#endif
#endif   // JUCE_WINDOWSREGISTRY_H_INCLUDED
