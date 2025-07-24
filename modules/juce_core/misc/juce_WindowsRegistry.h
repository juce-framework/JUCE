/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_WINDOWS || DOXYGEN

/**
    Contains some static helper functions for manipulating the MS Windows registry
    (Only available on Windows, of course!)

    @tags{Core}
*/
class JUCE_API  WindowsRegistry
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
    static String JUCE_CALLTYPE getValue (const String& regValuePath,
                                          const String& defaultValue = String(),
                                          WoW64Mode mode = WoW64_Default);

    /** Reads a binary block from the registry.
        The path is a string for the entire path of a value in the registry,
        e.g. "HKEY_CURRENT_USER\Software\foo\bar"
        @returns a DWORD indicating the type of the key.
    */
    static uint32 JUCE_CALLTYPE getBinaryValue (const String& regValuePath, MemoryBlock& resultData, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a string.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool JUCE_CALLTYPE setValue (const String& regValuePath, const String& value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a DWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool JUCE_CALLTYPE setValue (const String& regValuePath, uint32 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a QWORD.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool JUCE_CALLTYPE setValue (const String& regValuePath, uint64 value, WoW64Mode mode = WoW64_Default);

    /** Sets a registry value as a binary block.
        This will take care of creating any groups needed to get to the given registry value.
    */
    static bool JUCE_CALLTYPE setValue (const String& regValuePath, const MemoryBlock& value, WoW64Mode mode = WoW64_Default);

    /** Returns true if the given value exists in the registry. */
    static bool JUCE_CALLTYPE valueExists (const String& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Returns true if the given key exists in the registry. */
    static bool JUCE_CALLTYPE keyExists (const String& regKeyPath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry value. */
    static bool JUCE_CALLTYPE deleteValue (const String& regValuePath, WoW64Mode mode = WoW64_Default);

    /** Deletes a registry key (which is registry-talk for 'folder'). */
    static bool JUCE_CALLTYPE deleteKey (const String& regKeyPath, WoW64Mode mode = WoW64_Default);

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
    static bool JUCE_CALLTYPE registerFileAssociation (const String& fileExtension,
                                                       const String& symbolicDescription,
                                                       const String& fullDescription,
                                                       const File& targetExecutable,
                                                       int iconResourceNumber,
                                                       bool registerForCurrentUserOnly,
                                                       WoW64Mode mode = WoW64_Default);

    /** @cond */
    // DEPRECATED: use the other methods with a WoW64Mode parameter of WoW64_64bit instead.
    [[deprecated]] static String getValueWow64 (const String&, const String& defaultValue = String());
    [[deprecated]] static bool valueExistsWow64 (const String&);
    [[deprecated]] static bool keyExistsWow64 (const String&);
    /** @endcond */

private:
    WindowsRegistry() = delete;
    JUCE_DECLARE_NON_COPYABLE (WindowsRegistry)
};

#endif

} // namespace juce
