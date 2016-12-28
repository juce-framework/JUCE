/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TEXTDIFF_H_INCLUDED
#define JUCE_TEXTDIFF_H_INCLUDED


/**
    Calculates and applies a sequence of changes to convert one text string into
    another.

    Once created, the TextDiff object contains an array of change objects, where
    each change can be either an insertion or a deletion. When applied in order
    to the original string, these changes will convert it to the target string.
*/
class JUCE_API TextDiff
{
public:
    /** Creates a set of diffs for converting the original string into the target. */
    TextDiff (const String& original,
              const String& target);

    /** Applies this sequence of changes to the original string, producing the
        target string that was specified when generating them.

        Obviously it only makes sense to call this function with the string that
        was originally passed to the constructor. Any other input will produce an
        undefined result.
    */
    String appliedTo (String text) const;

    /** Describes a change, which can be either an insertion or deletion. */
    struct Change
    {
        String insertedText; /**< If this change is a deletion, this string will be empty; otherwise,
                                  it'll be the text that should be inserted at the index specified by start. */
        int start;    /**< Specifies the character index in a string at which text should be inserted or deleted. */
        int length;   /**< If this change is a deletion, this specifies the number of characters to delete. For an
                           insertion, this is the length of the new text being inserted. */

        /** Returns true if this change is a deletion, or false for an insertion. */
        bool isDeletion() const noexcept;

        /** Returns the result of applying this change to a string. */
        String appliedTo (const String& original) const noexcept;
    };

    /** The list of changes required to perform the transformation.
        Applying each of these, in order, to the original string will produce the target.
    */
    Array<Change> changes;
};


#endif   // JUCE_TEXTDIFF_H_INCLUDED
