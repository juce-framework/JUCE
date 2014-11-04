/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

struct TextDiffHelpers
{
    enum { minLengthToMatch = 3 };

    struct StringRegion
    {
        StringRegion (const String& s) noexcept
            : text (s.getCharPointer()), start (0), length (s.length()) {}

        StringRegion (const String::CharPointerType t, int s, int len)  noexcept
            : text (t), start (s), length (len) {}

        String::CharPointerType text;
        int start, length;
    };

    static void addInsertion (TextDiff& td, const String::CharPointerType text, int index, int length)
    {
        TextDiff::Change c;
        c.insertedText = String (text, (size_t) length);
        c.start = index;
        c.length = length;
        td.changes.add (c);
    }

    static void addDeletion (TextDiff& td, int index, int length)
    {
        TextDiff::Change c;
        c.start = index;
        c.length = length;
        td.changes.add (c);
    }

    static void diffSkippingCommonStart (TextDiff& td, const StringRegion& a, const StringRegion& b)
    {
        String::CharPointerType sa (a.text);
        String::CharPointerType sb (b.text);
        const int maxLen = jmax (a.length, b.length);

        for (int i = 0; i < maxLen; ++i, ++sa, ++sb)
        {
            if (*sa != *sb)
            {
                diffRecursively (td, StringRegion (sa, a.start + i, a.length - i),
                                     StringRegion (sb, b.start + i, b.length - i));
                break;
            }
        }
    }

    static void diffRecursively (TextDiff& td, const StringRegion& a, const StringRegion& b)
    {
        int indexA, indexB;
        const int len = findLongestCommonSubstring (a.text, a.length,
                                                    b.text, b.length,
                                                    indexA, indexB);

        if (len >= minLengthToMatch)
        {
            if (indexA > 0 && indexB > 0)
                diffSkippingCommonStart (td, StringRegion (a.text, a.start, indexA),
                                             StringRegion (b.text, b.start, indexB));
            else if (indexA > 0)
                addDeletion (td, b.start, indexA);
            else if (indexB > 0)
                addInsertion (td, b.text, b.start, indexB);

            diffRecursively (td, StringRegion (a.text + indexA + len, a.start + indexA + len, a.length - indexA - len),
                                 StringRegion (b.text + indexB + len, b.start + indexB + len, b.length - indexB - len));
        }
        else
        {
            if (a.length > 0)   addDeletion (td, b.start, a.length);
            if (b.length > 0)   addInsertion (td, b.text, b.start, b.length);
        }
    }

    static int findLongestCommonSubstring (String::CharPointerType a, const int lenA,
                                           const String::CharPointerType b, const int lenB,
                                           int& indexInA, int& indexInB)
    {
        if (lenA == 0 || lenB == 0)
            return 0;

        HeapBlock<int> lines;
        lines.calloc (2 + 2 * (size_t) lenB);

        int* l0 = lines;
        int* l1 = l0 + lenB + 1;

        int loopsWithoutImprovement = 0;
        int bestLength = 0;
        indexInA = indexInB = 0;

        for (int i = 0; i < lenA; ++i)
        {
            const juce_wchar ca = a.getAndAdvance();
            String::CharPointerType b2 (b);

            for (int j = 0; j < lenB; ++j)
            {
                if (ca != b2.getAndAdvance())
                {
                    l1[j + 1] = 0;
                }
                else
                {
                    const int len = l0[j] + 1;
                    l1[j + 1] = len;

                    if (len > bestLength)
                    {
                        loopsWithoutImprovement = 0;
                        bestLength = len;
                        indexInA = i;
                        indexInB = j;
                    }
                }
            }

            if (++loopsWithoutImprovement > 100)
                break;

            std::swap (l0, l1);
        }

        indexInA -= bestLength - 1;
        indexInB -= bestLength - 1;
        return bestLength;
    }
};

TextDiff::TextDiff (const String& original, const String& target)
{
    TextDiffHelpers::diffSkippingCommonStart (*this, original, target);
}

String TextDiff::appliedTo (String text) const
{
    for (int i = 0; i < changes.size(); ++i)
        text = changes.getReference(i).appliedTo (text);

    return text;
}

bool TextDiff::Change::isDeletion() const noexcept
{
    return insertedText.isEmpty();
}

String TextDiff::Change::appliedTo (const String& text) const noexcept
{
    return text.substring (0, start) + (isDeletion() ? text.substring (start + length)
                                                     : (insertedText + text.substring (start)));
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class DiffTests  : public UnitTest
{
public:
    DiffTests() : UnitTest ("TextDiff class") {}

    static String createString (Random& r)
    {
        juce_wchar buffer[50] = { 0 };

        for (int i = r.nextInt (49); --i >= 0;)
        {
            if (r.nextInt (10) == 0)
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (juce_wchar) ('a' + r.nextInt (3));
        }

        return CharPointer_UTF32 (buffer);
    }

    void testDiff (const String& a, const String& b)
    {
        TextDiff diff (a, b);
        const String result (diff.appliedTo (a));
        expectEquals (result, b);
    }

    void runTest()
    {
        beginTest ("TextDiff");

        Random r = getRandom();

        testDiff (String::empty, String::empty);
        testDiff ("x", String::empty);
        testDiff (String::empty, "x");
        testDiff ("x", "x");
        testDiff ("x", "y");
        testDiff ("xxx", "x");
        testDiff ("x", "xxx");

        for (int i = 5000; --i >= 0;)
        {
            String s (createString (r));
            testDiff (s, createString (r));
            testDiff (s + createString (r), s + createString (r));
        }
    }
};

static DiffTests diffTests;

#endif
