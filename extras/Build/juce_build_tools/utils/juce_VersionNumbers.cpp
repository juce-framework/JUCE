/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace build_tools
{
    StringArray getVersionSegments (StringRef p)
    {
        auto segments = StringArray::fromTokens (p, ",.", "");
        segments.trim();
        segments.removeEmptyStrings();
        return segments;
    }

    int getVersionAsHexIntegerFromParts (const StringArray& segments)
    {
        auto value = (segments[0].getIntValue() << 16)
                   + (segments[1].getIntValue() << 8)
                   +  segments[2].getIntValue();

        if (segments.size() > 3)
            value = (value << 8) + segments[3].getIntValue();

        return value;
    }

    int getVersionAsHexInteger (StringRef versionString)
    {
        return getVersionAsHexIntegerFromParts (getVersionSegments (versionString));
    }

    String getVersionAsHex (StringRef versionString)
    {
        return "0x" + String::toHexString (getVersionAsHexInteger (versionString));
    }
}
}
