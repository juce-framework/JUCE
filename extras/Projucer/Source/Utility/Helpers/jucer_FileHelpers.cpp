/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    bool containsAnyNonHiddenFiles (const File& folder)
    {
        for (const auto& di : RangedDirectoryIterator (folder, false))
            if (! di.getFile().isHidden())
                return true;

        return false;
    }

    bool shouldPathsBeRelative (String path1, String path2)
    {
        path1 = build_tools::unixStylePath (path1);
        path2 = build_tools::unixStylePath (path2);

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
