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
namespace RegistryHelpers
{
    HKEY findKeyForPath (String name, const bool createForWriting, String& valueName)
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
            name = name.substring (0, lastSlash);

            HKEY key;
            DWORD result;

            if (createForWriting)
            {
                if (RegCreateKeyEx (rootKey, name.toWideCharPointer(), 0, 0, REG_OPTION_NON_VOLATILE,
                                    (KEY_WRITE | KEY_QUERY_VALUE), 0, &key, &result) == ERROR_SUCCESS)
                    return key;
            }
            else
            {
                if (RegOpenKeyEx (rootKey, name.toWideCharPointer(), 0, KEY_READ, &key) == ERROR_SUCCESS)
                    return key;
            }
        }

        return 0;
    }
}

String WindowsRegistry::getValue (const String& regValuePath, const String& defaultValue)
{
    String valueName, result (defaultValue);
    HKEY k = RegistryHelpers::findKeyForPath (regValuePath, false, valueName);

    if (k != 0)
    {
        WCHAR buffer [2048];
        unsigned long bufferSize = sizeof (buffer);
        DWORD type = REG_SZ;

        if (RegQueryValueEx (k, valueName.toWideCharPointer(), 0, &type, (LPBYTE) buffer, &bufferSize) == ERROR_SUCCESS)
        {
            if (type == REG_SZ)
                result = buffer;
            else if (type == REG_DWORD)
                result = String ((int) *(DWORD*) buffer);
        }

        RegCloseKey (k);
    }

    return result;
}

void WindowsRegistry::setValue (const String& regValuePath, const String& value)
{
    String valueName;
    HKEY k = RegistryHelpers::findKeyForPath (regValuePath, true, valueName);

    if (k != 0)
    {
        RegSetValueEx (k, valueName.toWideCharPointer(), 0, REG_SZ,
                       (const BYTE*) value.toWideCharPointer(),
                       CharPointer_UTF16::getBytesRequiredFor (value.getCharPointer()));

        RegCloseKey (k);
    }
}

bool WindowsRegistry::valueExists (const String& regValuePath)
{
    bool exists = false;
    String valueName;
    HKEY k = RegistryHelpers::findKeyForPath (regValuePath, false, valueName);

    if (k != 0)
    {
        unsigned char buffer [2048];
        unsigned long bufferSize = sizeof (buffer);
        DWORD type = 0;

        if (RegQueryValueEx (k, valueName.toWideCharPointer(), 0, &type, buffer, &bufferSize) == ERROR_SUCCESS)
            exists = true;

        RegCloseKey (k);
    }

    return exists;
}

void WindowsRegistry::deleteValue (const String& regValuePath)
{
    String valueName;
    HKEY k = RegistryHelpers::findKeyForPath (regValuePath, true, valueName);

    if (k != 0)
    {
        RegDeleteValue (k, valueName.toWideCharPointer());
        RegCloseKey (k);
    }
}

void WindowsRegistry::deleteKey (const String& regKeyPath)
{
    String valueName;
    HKEY k = RegistryHelpers::findKeyForPath (regKeyPath, true, valueName);

    if (k != 0)
    {
        RegDeleteKey (k, valueName.toWideCharPointer());
        RegCloseKey (k);
    }
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

#endif
