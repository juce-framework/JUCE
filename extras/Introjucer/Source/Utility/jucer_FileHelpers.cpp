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

#include "../jucer_Headers.h"
#include "jucer_CodeHelpers.h"


//==============================================================================
namespace FileHelpers
{
    int64 calculateStreamHashCode (InputStream& in)
    {
        int64 t = 0;

        const int bufferSize = 4096;
        HeapBlock <uint8> buffer;
        buffer.malloc (bufferSize);

        for (;;)
        {
            const int num = in.read (buffer, bufferSize);

            if (num <= 0)
                break;

            for (int i = 0; i < num; ++i)
                t = t * 65599 + buffer[i];
        }

        return t;
    }

    int64 calculateFileHashCode (const File& file)
    {
        ScopedPointer <FileInputStream> stream (file.createInputStream());
        return stream != nullptr ? calculateStreamHashCode (*stream) : 0;
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const void* data, int numBytes)
    {
        if (file.getSize() == numBytes)
        {
            MemoryInputStream newStream (data, numBytes, false);

            if (calculateStreamHashCode (newStream) == calculateFileHashCode (file))
                return true;
        }

        TemporaryFile temp (file);

        return temp.getFile().appendData (data, numBytes)
                 && temp.overwriteTargetFileWithTemporary();
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const MemoryOutputStream& newData)
    {
        return overwriteFileWithNewDataIfDifferent (file, newData.getData(), newData.getDataSize());
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const String& newData)
    {
        const char* const utf8 = newData.toUTF8();
        return overwriteFileWithNewDataIfDifferent (file, utf8, strlen (utf8));
    }

    bool containsAnyNonHiddenFiles (const File& folder)
    {
        DirectoryIterator di (folder, false);

        while (di.next())
            if (! di.getFile().isHidden())
                return true;

        return false;
    }

    String unixStylePath (const String& path)
    {
        return path.replaceCharacter ('\\', '/');
    }

    String windowsStylePath (const String& path)
    {
        return path.replaceCharacter ('/', '\\');
    }

    String appendPath (const String& path, const String& subpath)
    {
        if (File::isAbsolutePath (subpath)
             || subpath.startsWithChar ('$')
             || subpath.startsWithChar ('~')
             || (CharacterFunctions::isLetter (subpath[0]) && subpath[1] == ':'))
            return subpath.replaceCharacter ('\\', '/');

        String path1 (path.replaceCharacter ('\\', '/'));
        if (! path1.endsWithChar ('/'))
            path1 << '/';

        return path1 + subpath.replaceCharacter ('\\', '/');
    }

    bool shouldPathsBeRelative (String path1, String path2)
    {
        path1 = unixStylePath (path1);
        path2 = unixStylePath (path2);

        const int len = jmin (path1.length(), path2.length());
        int commonBitLength = 0;

        for (int i = 0; i < len; ++i)
        {
            if (CharacterFunctions::toLowerCase (path1[i]) != CharacterFunctions::toLowerCase (path2[i]))
                break;

            ++commonBitLength;
        }

        return path1.substring (0, commonBitLength).removeCharacters ("/:").isNotEmpty();
    }

    //==============================================================================
    bool isJuceFolder (const File& folder)
    {
        return folder.getFileName().containsIgnoreCase ("juce")
                 && folder.getChildFile ("juce.h").exists()
                 && folder.getChildFile ("modules").exists();
    }

    static File lookInFolderForJuceFolder (const File& folder)
    {
        for (DirectoryIterator di (folder, false, "*juce*", File::findDirectories); di.next();)
        {
            if (isJuceFolder (di.getFile()))
                return di.getFile();
        }

        return File::nonexistent;
    }

    File findParentJuceFolder (const File& file)
    {
        File f (file);

        while (f.exists() && f.getParentDirectory() != f)
        {
            if (isJuceFolder (f))
                return f;

            File found = lookInFolderForJuceFolder (f);
            if (found.exists())
                return found;

            f = f.getParentDirectory();
        }

        return File::nonexistent;
    }

    File findDefaultJuceFolder()
    {
        File f = findParentJuceFolder (File::getSpecialLocation (File::currentApplicationFile));

        if (! f.exists())
            f = lookInFolderForJuceFolder (File::getSpecialLocation (File::userHomeDirectory));

        if (! f.exists())
            f = lookInFolderForJuceFolder (File::getSpecialLocation (File::userDocumentsDirectory));

        return f;
    }
}
