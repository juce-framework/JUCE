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

namespace juce
{

/** Options that can be used to affect the layout produced by GlyphArrangement::addFittedText.

    @see GlyphArrangement::addFittedText

    @tags{Graphics}
*/
class JUCE_API GlyphArrangementOptions final
{
public:
    /** We increment the distance between the baselines of subsequent lines with this value.

        Line spacing is added below the line's descender, and doesn't affect the first line's
        baseline.

        The total distance between baselines is lineHeight * lineHeightMultiple + lineSpacing.
    */
    [[nodiscard]] GlyphArrangementOptions withLineSpacing (float x) const
    {
        return withMember (*this, &GlyphArrangementOptions::lineSpacing, x);
    }

    /** We multiply the original distance between the baselines of subsequent lines with this value.

        The line height multiple is applied to both the ascender and descender, hence it affects
        the first line's baseline.

        The total distance between baselines is lineHeight * lineHeightMultiple + lineSpacing.
    */
    [[nodiscard]] GlyphArrangementOptions withLineHeightMultiple (float x) const
    {
        return withMember (*this, &GlyphArrangementOptions::lineHeightMultiple, x);
    }

    /** @see withLineSpacing() */
    const auto& getLineSpacing() const        { return lineSpacing; }

    /** @see withLineHeightMultiple() */
    const auto& getLineHeightMultiple() const { return lineHeightMultiple; }

    /** Equality operator. */
    [[nodiscard]] bool operator== (const GlyphArrangementOptions& other) const;
    /** Inequality operator. */
    [[nodiscard]] bool operator!= (const GlyphArrangementOptions& other) const;
    /** Less-than operator. Allows GlyphArrangementOptions to be used as keys in a map. */
    [[nodiscard]] bool operator<  (const GlyphArrangementOptions& other) const;
    /** Less-than-or-equal operator. */
    [[nodiscard]] bool operator<= (const GlyphArrangementOptions& other) const;
    /** Greater-than operator. */
    [[nodiscard]] bool operator>  (const GlyphArrangementOptions& other) const;
    /** Greater-than-or-equal operator. */
    [[nodiscard]] bool operator>= (const GlyphArrangementOptions& other) const;

private:
    auto tie() const noexcept;

    float lineSpacing = 0.0f;
    float lineHeightMultiple = 1.0f;
};

} // namespace juce
