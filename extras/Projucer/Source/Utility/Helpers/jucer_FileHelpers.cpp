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
