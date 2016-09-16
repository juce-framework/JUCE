/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

struct SourceCodeRange
{
    SourceCodeRange() = default;

    SourceCodeRange (const String& f, int start, int end)
        : file (f), range (start, end)
    {
       #if JUCE_WINDOWS
        file = file.replaceCharacter ('/', '\\');
       #endif
    }

    SourceCodeRange (const String& s)
    {
        String::CharPointerType colon1 (nullptr), colon2 (nullptr);

        for (auto p = s.getCharPointer(); ! p.isEmpty(); ++p)
        {
            if (*p == ':')
            {
                colon1 = colon2;
                colon2 = p;
            }
        }

        if (colon1.getAddress() != nullptr && colon2.getAddress() != nullptr)
        {
            file = String (s.getCharPointer(), colon1);
            range = Range<int> ((colon1 + 1).getIntValue32(),
                                (colon2 + 1).getIntValue32());
        }
    }

    String file;
    Range<int> range;

    bool isValid() const noexcept   { return file.isNotEmpty() && range != Range<int>(); }

    void nudge (const String& changedFile, const int insertPoint, const int delta) noexcept
    {
        if (range.getEnd() >= insertPoint && file == changedFile)
        {
            const int newEnd = range.getEnd() + delta;
            int newStart = range.getStart();
            if (newStart > insertPoint)
                newStart += delta;

            range = Range<int> (newStart, newEnd);
        }
    }

    void fileContentChanged (const String& changedFile) noexcept
    {
        if (file == changedFile)
            range = Range<int>();
    }

    String toString() const
    {
        if (file.isEmpty() && range.isEmpty())
            return String();

        return file + ":" + String (range.getStart()) + ":" + String (range.getEnd());
    }

    void writeToValueTree (ValueTree& v, const Identifier& prop) const
    {
        const String s (toString());

        if (s.isNotEmpty())
            v.setProperty (prop, s, nullptr);
    }

    bool operator== (const SourceCodeRange& other) const noexcept    { return range == other.range && file == other.file; }
    bool operator!= (const SourceCodeRange& other) const noexcept    { return ! operator== (other); }
};
