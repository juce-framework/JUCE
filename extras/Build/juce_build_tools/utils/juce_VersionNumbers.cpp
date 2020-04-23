/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace build_tools
{
    StringArray getVersionSegments (juce::StringRef p)
    {
        auto segments = StringArray::fromTokens (p, ",.", "");
        segments.trim();
        segments.removeEmptyStrings();
        return segments;
    }

    int getVersionAsHexInteger (juce::StringRef versionString)
    {
        auto segments = getVersionSegments (versionString);

        auto value = (segments[0].getIntValue() << 16)
                     + (segments[1].getIntValue() << 8)
                     +  segments[2].getIntValue();

        if (segments.size() > 3)
            value = (value << 8) + segments[3].getIntValue();

        return value;
    }

    String getVersionAsHex (juce::StringRef versionString)
    {
        return "0x" + String::toHexString (getVersionAsHexInteger (versionString));
    }
}
}
