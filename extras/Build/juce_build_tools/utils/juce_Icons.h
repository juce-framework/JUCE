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
    struct Icons
    {
        std::unique_ptr<Drawable> small;
        std::unique_ptr<Drawable> big;
    };

    Array<Drawable*> asArray (const Icons&);
    void writeMacIcon (const Icons&, const File&);
    void writeWinIcon (const Icons&, const File&);

    Image getBestIconForSize (const Icons& icons,
                              int size,
                              bool returnNullIfNothingBigEnough);
    Image rescaleImageForIcon (Drawable& d, const int size);

    RelativePath createXcassetsFolderFromIcons (const Icons& icons,
                                                const File& targetFolder,
                                                String projectFilenameRootString);
}
}
