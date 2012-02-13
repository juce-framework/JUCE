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

struct RegistryKeyWrapper
{
    RegistryKeyWrapper (String name, const bool createForWriting)
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
                                (KEY_WRITE | KEY_QUERY_VALUE), 0, &key, &result);
            else
                RegOpenKeyEx (rootKey, wideCharName, 0, KEY_READ, &key);
        }
    }

    ~RegistryKeyWrapper()
    {
        if (key != 0)
            RegCloseKey (key);
    }

    HKEY key;
    const wchar_t* wideCharValueName;
    String valueName;

    JUCE_DECLARE_NON_COPYABLE (RegistryKeyWrapper);
};

String WindowsRegistry::getValue (const String& regValuePath, const String& defaultValue)
{
    const RegistryKeyWrapper key (regValuePath, false);

    if (key.key != 0)
    {
        WCHAR buffer [2048];
        unsigned long bufferSize = sizeof (buffer);
        DWORD type = REG_SZ;

        if (RegQueryValueEx (key.key, key.wideCharValueName, 0, &type, (LPBYTE) buffer, &bufferSize) == ERROR_SUCCESS)
        {
            if (type == REG_SZ)
                return buffer;
            else if (type == REG_DWORD)
                return String ((int) *(DWORD*) buffer);
        }
    }

    return defaultValue;
}

void WindowsRegistry::setValue (const String& regValuePath, const String& value)
{
    const RegistryKeyWrapper key (regValuePath, true);

    if (key.key != 0)
        RegSetValueEx (key.key, key.wideCharValueName, 0, REG_SZ,
                       (const BYTE*) value.toWideCharPointer(),
                       (DWORD) CharPointer_UTF16::getBytesRequiredFor (value.getCharPointer()));
}

bool WindowsRegistry::valueExists (const String& regValuePath)
{
    const RegistryKeyWrapper key (regValuePath, false);

    if (key.key == 0)
        return false;

    unsigned char buffer [2048];
    unsigned long bufferSize = sizeof (buffer);
    DWORD type = 0;

    return RegQueryValueEx (key.key, key.wideCharValueName,
                            0, &type, buffer, &bufferSize) == ERROR_SUCCESS;
}

void WindowsRegistry::deleteValue (const String& regValuePath)
{
    const RegistryKeyWrapper key (regValuePath, true);

    if (key.key != 0)
        RegDeleteValue (key.key, key.wideCharValueName);
}

void WindowsRegistry::deleteKey (const String& regKeyPath)
{
    const RegistryKeyWrapper key (regKeyPath, true);

    if (key.key != 0)
        RegDeleteKey (key.key, key.wideCharValueName);
}

void WindowsRegistry::registerFileAssociation (const String& fileExtension,
                                               const String& symbolicDescription,
                                               const String& fullDescription,
                                               const File& targetExecutable,
                                               int iconResourceNumber)
{
    setValue ("HKEY_CLASSES_ROOT\\" + fileExtension + "\\", symbolicDescription);

    const String key ("HKEY_CLASSES_ROOT\\" + symbolicDescription);

    if (iconResourceNumber != 0)
        setValue (key + "\\DefaultIcon\\",
                  targetExecutable.getFullPathName() + "," + String (-iconResourceNumber));

    setValue (key + "\\", fullDescription);
    setValue (key + "\\shell\\open\\command\\", targetExecutable.getFullPathName() + " %1");
}
