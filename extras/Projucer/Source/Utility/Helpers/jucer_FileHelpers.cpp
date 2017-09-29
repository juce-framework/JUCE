/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_CodeHelpers.h"

//==============================================================================
namespace FileHelpers
{
    static int64 calculateMemoryHashCode (const void* data, const size_t numBytes)
    {
        int64 t = 0;

        for (size_t i = 0; i < numBytes; ++i)
            t = t * 65599 + static_cast<const uint8*> (data)[i];

        return t;
    }

    int64 calculateStreamHashCode (InputStream& in)
    {
        int64 t = 0;

        const int bufferSize = 4096;
        HeapBlock<uint8> buffer;
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
        ScopedPointer<FileInputStream> stream (file.createInputStream());
        return stream != nullptr ? calculateStreamHashCode (*stream) : 0;
    }

    bool overwriteFileWithNewDataIfDifferent (const File& file, const void* data, size_t numBytes)
    {
        if (file.getSize() == (int64) numBytes
              && calculateMemoryHashCode (data, numBytes) == calculateFileHashCode (file))
            return true;

        if (file.exists())
            return file.replaceWithData (data, numBytes);

        return file.getParentDirectory().createDirectory() && file.appendData (data, numBytes);
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

    // removes "/../" bits from the middle of the path
    String simplifyPath (String::CharPointerType p)
    {
       #if JUCE_WINDOWS
        if (CharacterFunctions::indexOf (p, CharPointer_ASCII ("/../")) >= 0
             || CharacterFunctions::indexOf (p, CharPointer_ASCII ("\\..\\")) >= 0)
       #else
        if (CharacterFunctions::indexOf (p, CharPointer_ASCII ("/../")) >= 0)
       #endif
        {
            StringArray toks;

           #if JUCE_WINDOWS
            toks.addTokens (p, "\\/", StringRef());
           #else
            toks.addTokens (p, "/", StringRef());
           #endif

            while (toks[0] == ".")
                toks.remove (0);

            for (int i = 1; i < toks.size(); ++i)
            {
                if (toks[i] == ".." && toks [i - 1] != "..")
                {
                    toks.removeRange (i - 1, 2);
                    i = jmax (0, i - 2);
                }
            }

            return toks.joinIntoString ("/");
        }

        return p;
    }

    String simplifyPath (const String& path)
    {
       #if JUCE_WINDOWS
        if (path.contains ("\\..\\") || path.contains ("/../"))
       #else
        if (path.contains ("/../"))
       #endif
            return simplifyPath (path.getCharPointer());

        return path;
    }
}
