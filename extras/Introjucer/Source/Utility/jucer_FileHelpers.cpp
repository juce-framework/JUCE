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
    static int64 calculateMemoryHashCode (const void* data, const int numBytes)
    {
        int64 t = 0;

        for (int i = 0; i < numBytes; ++i)
            t = t * 65599 + static_cast <const uint8*> (data)[i];

        return t;
    }

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
        if (file.getSize() == numBytes
              && calculateMemoryHashCode (data, numBytes) == calculateFileHashCode (file))
            return true;

        if (file.exists())
            return file.replaceWithData (data, numBytes);
        else
            return file.appendData (data, numBytes);
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

    String unixStylePath (const String& path)       { return path.replaceCharacter ('\\', '/'); }
    String windowsStylePath (const String& path)    { return path.replaceCharacter ('/', '\\'); }

    String currentOSStylePath (const String& path)
    {
       #if JUCE_WINDOWS
        return windowsStylePath (path);
       #else
        return unixStylePath (path);
       #endif
    }

    bool isAbsolutePath (const String& path)
    {
        return File::isAbsolutePath (path)
                || path.startsWithChar ('/') // (needed because File::isAbsolutePath will ignore forward-slashes on Windows)
                || path.startsWithChar ('$')
                || path.startsWithChar ('~')
                || (CharacterFunctions::isLetter (path[0]) && path[1] == ':')
                || path.startsWithIgnoreCase ("smb:");
    }

    String appendPath (const String& path, const String& subpath)
    {
        if (isAbsolutePath (subpath))
            return unixStylePath (subpath);

        String path1 (unixStylePath (path));
        if (! path1.endsWithChar ('/'))
            path1 << '/';

        return path1 + unixStylePath (subpath);
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

    String getRelativePathFrom (const File& file, const File& sourceFolder)
    {
       #if ! JUCE_WINDOWS
        // On a non-windows machine, we can't know if a drive-letter path may be relative or not.
        if (CharacterFunctions::isLetter (file.getFullPathName()[0]) && file.getFullPathName()[1] == ':')
            return file.getFullPathName();
       #endif

        return file.getRelativePathFrom (sourceFolder);
    }
}
