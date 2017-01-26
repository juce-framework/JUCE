/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

struct RegistryKeyWrapper
{
    RegistryKeyWrapper (String name, const bool createForWriting, const DWORD wow64Flags)
        : key (0), wideCharValueName (nullptr)
    {
        if (HKEY rootKey = getRootKey (name))
        {
            name = name.substring (name.indexOfChar ('\\') + 1);

            const int lastSlash = name.lastIndexOfChar ('\\');
            valueName = name.substring (lastSlash + 1);
            wideCharValueName = valueName.toWideCharPointer();

            name = name.substring (0, lastSlash);
            const wchar_t* const wideCharName = name.toWideCharPointer();
            DWORD result;

            if (createForWriting)
                RegCreateKeyEx (rootKey, wideCharName, 0, 0, REG_OPTION_NON_VOLATILE,
                                KEY_WRITE | KEY_QUERY_VALUE | wow64Flags, 0, &key, &result);
            else
                RegOpenKeyEx (rootKey, wideCharName, 0, KEY_READ | wow64Flags, &key);
        }
    }

    ~RegistryKeyWrapper()
    {
        if (key != 0)
            RegCloseKey (key);
    }

    static HKEY getRootKey (const String& name) noexcept
    {
        if (name.startsWithIgnoreCase ("HKEY_CURRENT_USER\\"))  return HKEY_CURRENT_USER;
        if (name.startsWithIgnoreCase ("HKCU\\"))               return HKEY_CURRENT_USER;
        if (name.startsWithIgnoreCase ("HKEY_LOCAL_MACHINE\\")) return HKEY_LOCAL_MACHINE;
        if (name.startsWithIgnoreCase ("HKLM\\"))               return HKEY_LOCAL_MACHINE;
        if (name.startsWithIgnoreCase ("HKEY_CLASSES_ROOT\\"))  return HKEY_CLASSES_ROOT;
        if (name.startsWithIgnoreCase ("HKCR\\"))               return HKEY_CLASSES_ROOT;
        if (name.startsWithIgnoreCase ("HKEY_USERS\\"))         return HKEY_USERS;
        if (name.startsWithIgnoreCase ("HKU\\"))                return HKEY_USERS;

        jassertfalse; // The name starts with an unknown root key (or maybe an old Win9x type)
        return 0;
    }

    static bool setValue (const String& regValuePath, const DWORD type,
                          const void* data, size_t dataSize, const DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, true, wow64Flags);

        return key.key != 0
                && RegSetValueEx (key.key, key.wideCharValueName, 0, type,
                                  reinterpret_cast<const BYTE*> (data),
                                  (DWORD) dataSize) == ERROR_SUCCESS;
    }

    static uint32 getBinaryValue (const String& regValuePath, MemoryBlock& result, DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, false, wow64Flags);

        if (key.key != 0)
        {
            for (unsigned long bufferSize = 1024; ; bufferSize *= 2)
            {
                result.setSize (bufferSize, false);
                DWORD type = REG_NONE;

                const LONG err = RegQueryValueEx (key.key, key.wideCharValueName, 0, &type,
                                                  (LPBYTE) result.getData(), &bufferSize);

                if (err == ERROR_SUCCESS)
                {
                    result.setSize (bufferSize, false);
                    return type;
                }

                if (err != ERROR_MORE_DATA)
                    break;
            }
        }

        return REG_NONE;
    }

    static String getValue (const String& regValuePath, const String& defaultValue, DWORD wow64Flags)
    {
        MemoryBlock buffer;
        switch (getBinaryValue (regValuePath, buffer, wow64Flags))
        {
            case REG_SZ:    return static_cast<const WCHAR*> (buffer.getData());
            case REG_DWORD: return String ((int) *reinterpret_cast<const DWORD*> (buffer.getData()));
            default:        break;
        }

        return defaultValue;
    }

    static bool keyExists (const String& regValuePath, const DWORD wow64Flags)
    {
        return RegistryKeyWrapper (regValuePath, false, wow64Flags).key != 0;
    }

    static bool valueExists (const String& regValuePath, const DWORD wow64Flags)
    {
        const RegistryKeyWrapper key (regValuePath, false, wow64Flags);

        if (key.key == 0)
            return false;

        unsigned char buffer [512];
        unsigned long bufferSize = sizeof (buffer);
        DWORD type = 0;

        const LONG result = RegQueryValueEx (key.key, key.wideCharValueName,
                                             0, &type, buffer, &bufferSize);

        return result == ERROR_SUCCESS || result == ERROR_MORE_DATA;
    }

    HKEY key;
    const wchar_t* wideCharValueName;
    String valueName;

    JUCE_DECLARE_NON_COPYABLE (RegistryKeyWrapper)
};

uint32 JUCE_CALLTYPE WindowsRegistry::getBinaryValue (const String& regValuePath, MemoryBlock& result, WoW64Mode mode)
{
    return RegistryKeyWrapper::getBinaryValue (regValuePath, result, (DWORD) mode);
}

String JUCE_CALLTYPE WindowsRegistry::getValue (const String& regValuePath, const String& defaultValue, WoW64Mode mode)
{
    return RegistryKeyWrapper::getValue (regValuePath, defaultValue, (DWORD) mode);
}

bool JUCE_CALLTYPE WindowsRegistry::setValue (const String& regValuePath, const String& value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_SZ, value.toWideCharPointer(),
                                         CharPointer_UTF16::getBytesRequiredFor (value.getCharPointer()), mode);
}

bool JUCE_CALLTYPE WindowsRegistry::setValue (const String& regValuePath, const uint32 value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_DWORD, &value, sizeof (value), (DWORD) mode);
}

bool JUCE_CALLTYPE WindowsRegistry::setValue (const String& regValuePath, const uint64 value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_QWORD, &value, sizeof (value), (DWORD) mode);
}

bool JUCE_CALLTYPE WindowsRegistry::setValue (const String& regValuePath, const MemoryBlock& value, WoW64Mode mode)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_BINARY, value.getData(), value.getSize(), (DWORD) mode);
}

bool JUCE_CALLTYPE WindowsRegistry::valueExists (const String& regValuePath, WoW64Mode mode)
{
    return RegistryKeyWrapper::valueExists (regValuePath, (DWORD) mode);
}

bool JUCE_CALLTYPE WindowsRegistry::keyExists (const String& regValuePath, WoW64Mode mode)
{
    return RegistryKeyWrapper::keyExists (regValuePath, (DWORD) mode);
}

void JUCE_CALLTYPE WindowsRegistry::deleteValue (const String& regValuePath, WoW64Mode mode)
{
    const RegistryKeyWrapper key (regValuePath, true, (DWORD) mode);

    if (key.key != 0)
        RegDeleteValue (key.key, key.wideCharValueName);
}

void JUCE_CALLTYPE WindowsRegistry::deleteKey (const String& regKeyPath, WoW64Mode mode)
{
    const RegistryKeyWrapper key (regKeyPath, true, (DWORD) mode);

    if (key.key != 0)
        RegDeleteKey (key.key, key.wideCharValueName);
}

bool JUCE_CALLTYPE WindowsRegistry::registerFileAssociation (const String& fileExtension,
                                                             const String& symbolicDescription,
                                                             const String& fullDescription,
                                                             const File& targetExecutable,
                                                             const int iconResourceNumber,
                                                             const bool registerForCurrentUserOnly,
                                                             WoW64Mode mode)
{
    const char* const root = registerForCurrentUserOnly ? "HKEY_CURRENT_USER\\Software\\Classes\\"
                                                        : "HKEY_CLASSES_ROOT\\";
    const String key (root + symbolicDescription);

    return setValue (root + fileExtension + "\\", symbolicDescription, mode)
        && setValue (key + "\\", fullDescription, mode)
        && setValue (key + "\\shell\\open\\command\\", targetExecutable.getFullPathName() + " \"%1\"", mode)
        && (iconResourceNumber == 0
              || setValue (key + "\\DefaultIcon\\",
                           targetExecutable.getFullPathName() + "," + String (iconResourceNumber)));
}

// These methods are deprecated:
String WindowsRegistry::getValueWow64 (const String& p, const String& defVal)  { return getValue (p, defVal, WoW64_64bit); }
bool WindowsRegistry::valueExistsWow64 (const String& p)                       { return valueExists (p, WoW64_64bit); }
bool WindowsRegistry::keyExistsWow64 (const String& p)                         { return keyExists (p, WoW64_64bit); }
