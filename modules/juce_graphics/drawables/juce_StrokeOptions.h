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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
/** The set of SVG stroke attributes. */
class JUCE_API  StrokeOptions
{
public:
    /** Returns a copy of these attributes with a new colour. */
    [[nodiscard]] StrokeOptions withColour (Colour x) const
    {
        return withMember (*this, &StrokeOptions::colour, x);
    }

    /** Returns a copy of these attributes with a new dash array. */
    [[nodiscard]] StrokeOptions withDashArray (Span<const float> x) const
    {
        std::vector<float> arr (x.begin(), x.end());
        return withMember (*this, &StrokeOptions::dashArray, std::move (arr));
    }

    /** Returns a copy of these attributes with a new dash offset. */
    [[nodiscard]] StrokeOptions withDashOffset (float x) const
    {
        return withMember (*this, &StrokeOptions::dashOffset, x);
    }

    /** Returns a copy of these attributes with a new line cap style. */
    [[nodiscard]] StrokeOptions withLineCap (PathStrokeType::EndCapStyle x) const
    {
        return withMember (*this, &StrokeOptions::lineCap, x);
    }

    /** Returns a copy of these attributes with a new line join style. */
    [[nodiscard]] StrokeOptions withLineJoin (PathStrokeType::JointStyle x) const
    {
        return withMember (*this, &StrokeOptions::lineJoin, x);
    }

    /** Returns a copy of these attributes with a new miter limit. */
    [[nodiscard]] StrokeOptions withMiterLimit (float x) const
    {
        return withMember (*this, &StrokeOptions::miterLimit, x);
    }

    /** Returns a copy of these attributes with a new stroke width. */
    [[nodiscard]] StrokeOptions withWidth (float x) const
    {
        return withMember (*this, &StrokeOptions::width, x);
    }

    /** @see withColour() */
    const auto& getColour() const          { return colour; }

    /** @see withDashArray() */
    Span<const float> getDashArray() const { return dashArray; }

    /** @see withDashOffset() */
    const auto& getDashOffset() const      { return dashOffset; }

    /** @see withLineCap() */
    const auto& getLineCap() const         { return lineCap; }

    /** @see withLineJoin() */
    const auto& getLineJoin() const        { return lineJoin; }

    /** @see withMiterLimit() */
    const auto& getMiterLimit() const      { return miterLimit; }

    /** @see withWidth() */
    const auto& getWidth() const           { return width; }

    /** Creates a PathStrokeType object based on the current attributes. */
    PathStrokeType createStrokeType() const
    {
        PathStrokeType type { width, lineJoin, lineCap };
        type.setMiterLimit (miterLimit);
        return type;
    }

    /** Equality operator. */
    bool operator== (const StrokeOptions& other) const noexcept;

    /** Inequality operator. */
    bool operator!= (const StrokeOptions& other) const noexcept   { return ! operator== (other); }

private:
    Colour colour = Colours::black;  // A combination of the 'stroke' and 'opacity' attributes.
    std::vector<float> dashArray;
    float dashOffset{};
    PathStrokeType::EndCapStyle lineCap = PathStrokeType::EndCapStyle::butt;
    PathStrokeType::JointStyle lineJoin = PathStrokeType::JointStyle::mitered;
    float miterLimit = 4.0f;
    float width = 1.0f;
};

} // namespace juce
