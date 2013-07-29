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

struct RegistryKeyWrapper
{
    RegistryKeyWrapper (String name, const bool createForWriting, const DWORD wow64Flags)
        : key (0), wideCharValueName (nullptr)
    {
        HKEY rootKey = 0;

        if (name.startsWithIgnoreCase ("HKEY_CURRENT_USER\\"))        rootKey = HKEY_CURRENT_USER;
        else if (name.startsWithIgnoreCase ("HKEY_LOCAL_MACHINE\\"))  rootKey = HKEY_LOCAL_MACHINE;
        else if (name.startsWithIgnoreCase ("HKEY_CLASSES_ROOT\\"))   rootKey = HKEY_CLASSES_ROOT;

        if (rootKey != 0)
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

    static bool setValue (const String& regValuePath, const DWORD type,
                          const void* data, size_t dataSize)
    {
        const RegistryKeyWrapper key (regValuePath, true, 0);

        return key.key != 0
                && RegSetValueEx (key.key, key.wideCharValueName, 0, type,
                                  reinterpret_cast <const BYTE*> (data),
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
            case REG_SZ:    return static_cast <const WCHAR*> (buffer.getData());
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

uint32 WindowsRegistry::getBinaryValue (const String& regValuePath, MemoryBlock& result)
{
    return RegistryKeyWrapper::getBinaryValue (regValuePath, result, 0);
}

String WindowsRegistry::getValue (const String& regValuePath, const String& defaultValue)
{
    return RegistryKeyWrapper::getValue (regValuePath, defaultValue, 0);
}

String WindowsRegistry::getValueWow64 (const String& regValuePath, const String& defaultValue)
{
    return RegistryKeyWrapper::getValue (regValuePath, defaultValue, 0x100 /*KEY_WOW64_64KEY*/);
}

bool WindowsRegistry::valueExistsWow64 (const String& regValuePath)
{
    return RegistryKeyWrapper::valueExists (regValuePath, 0x100 /*KEY_WOW64_64KEY*/);
}

bool WindowsRegistry::keyExistsWow64 (const String& regValuePath)
{
    return RegistryKeyWrapper::keyExists (regValuePath, 0x100 /*KEY_WOW64_64KEY*/);
}

bool WindowsRegistry::setValue (const String& regValuePath, const String& value)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_SZ, value.toWideCharPointer(),
                                         CharPointer_UTF16::getBytesRequiredFor (value.getCharPointer()));
}

bool WindowsRegistry::setValue (const String& regValuePath, const uint32 value)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_DWORD, &value, sizeof (value));
}

bool WindowsRegistry::setValue (const String& regValuePath, const uint64 value)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_QWORD, &value, sizeof (value));
}

bool WindowsRegistry::setValue (const String& regValuePath, const MemoryBlock& value)
{
    return RegistryKeyWrapper::setValue (regValuePath, REG_BINARY, value.getData(), value.getSize());
}

bool WindowsRegistry::valueExists (const String& regValuePath)
{
    return RegistryKeyWrapper::valueExists (regValuePath, 0);
}

bool WindowsRegistry::keyExists (const String& regValuePath)
{
    return RegistryKeyWrapper::keyExists (regValuePath, 0);
}

void WindowsRegistry::deleteValue (const String& regValuePath)
{
    const RegistryKeyWrapper key (regValuePath, true, 0);

    if (key.key != 0)
        RegDeleteValue (key.key, key.wideCharValueName);
}

void WindowsRegistry::deleteKey (const String& regKeyPath)
{
    const RegistryKeyWrapper key (regKeyPath, true, 0);

    if (key.key != 0)
        RegDeleteKey (key.key, key.wideCharValueName);
}

bool WindowsRegistry::registerFileAssociation (const String& fileExtension,
                                               const String& symbolicDescription,
                                               const String& fullDescription,
                                               const File& targetExecutable,
                                               const int iconResourceNumber,
                                               const bool registerForCurrentUserOnly)
{
    const char* const root = registerForCurrentUserOnly ? "HKEY_CURRENT_USER\\Software\\Classes\\"
                                                        : "HKEY_CLASSES_ROOT\\";
    const String key (root + symbolicDescription);

    return setValue (root + fileExtension + "\\", symbolicDescription)
        && setValue (key + "\\", fullDescription)
        && setValue (key + "\\shell\\open\\command\\", targetExecutable.getFullPathName() + " \"%1\"")
        && (iconResourceNumber == 0
              || setValue (key + "\\DefaultIcon\\",
                           targetExecutable.getFullPathName() + "," + String (-iconResourceNumber)));
}
