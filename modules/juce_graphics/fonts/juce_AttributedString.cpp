/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace
{
    int getLength (const Array<AttributedString::Attribute>& atts) noexcept
    {
        return atts.size() != 0 ? atts.getReference (atts.size() - 1).range.getEnd() : 0;
    }

    void splitAttributeRanges (Array<AttributedString::Attribute>& atts, int position)
    {
        for (int i = atts.size(); --i >= 0;)
        {
            const auto& att = atts.getUnchecked (i);
            auto offset = position - att.range.getStart();

            if (offset >= 0)
            {
                if (offset > 0 && position < att.range.getEnd())
                {
                    atts.insert (i + 1, AttributedString::Attribute (att));
                    atts.getReference (i).range.setEnd (position);
                    atts.getReference (i + 1).range.setStart (position);
                }

                break;
            }
        }
    }

    Range<int> splitAttributeRanges (Array<AttributedString::Attribute>& atts, Range<int> newRange)
    {
        newRange = newRange.getIntersectionWith ({ 0, getLength (atts) });

        if (! newRange.isEmpty())
        {
            splitAttributeRanges (atts, newRange.getStart());
            splitAttributeRanges (atts, newRange.getEnd());
        }

        return newRange;
    }

    void mergeAdjacentRanges (Array<AttributedString::Attribute>& atts)
    {
        for (int i = atts.size() - 1; --i >= 0;)
        {
            auto& a1 = atts.getReference (i);
            auto& a2 = atts.getReference (i + 1);

            if (a1.colour == a2.colour && a1.font == a2.font)
            {
                a1.range.setEnd (a2.range.getEnd());
                atts.remove (i + 1);

                if (i < atts.size() - 1)
                    ++i;
            }
        }
    }

    void appendRange (Array<AttributedString::Attribute>& atts,
                      int length, const Font* f, const Colour* c)
    {
        if (atts.size() == 0)
        {
            atts.add ({ Range<int> (0, length), f != nullptr ? *f : Font(), c != nullptr ? *c : Colour (0xff000000) });
        }
        else
        {
            auto start = getLength (atts);
            atts.add ({ Range<int> (start, start + length),
                        f != nullptr ? *f : atts.getReference (atts.size() - 1).font,
                        c != nullptr ? *c : atts.getReference (atts.size() - 1).colour });

            mergeAdjacentRanges (atts);
        }
    }

    void applyFontAndColour (Array<AttributedString::Attribute>& atts,
                             Range<int> range, const Font* f, const Colour* c)
    {
        range = splitAttributeRanges (atts, range);

        for (auto& att : atts)
        {
            if (range.getStart() < att.range.getEnd())
            {
                if (range.getEnd() <= att.range.getStart())
                    break;

                if (c != nullptr) att.colour = *c;
                if (f != nullptr) att.font = *f;
            }
        }

        mergeAdjacentRanges (atts);
    }

    void truncate (Array<AttributedString::Attribute>& atts, int newLength)
    {
        splitAttributeRanges (atts, newLength);

        for (int i = atts.size(); --i >= 0;)
            if (atts.getReference (i).range.getStart() >= newLength)
                atts.remove (i);
    }
}

//==============================================================================
AttributedString::Attribute::Attribute (Attribute&& other) noexcept
    : range (other.range),
      font (std::move (other.font)),
      colour (other.colour)
{
}

AttributedString::Attribute& AttributedString::Attribute::operator= (Attribute&& other) noexcept
{
    range = other.range;
    font = std::move (other.font);
    colour = other.colour;
    return *this;
}

AttributedString::Attribute::Attribute (Range<int> r, const Font& f, Colour c) noexcept
    : range (r), font (f), colour (c)
{
}

//==============================================================================
AttributedString::AttributedString (AttributedString&& other) noexcept
    : text (std::move (other.text)),
      lineSpacing (other.lineSpacing),
      justification (other.justification),
      wordWrap (other.wordWrap),
      readingDirection (other.readingDirection),
      attributes (std::move (other.attributes))
{
}

AttributedString& AttributedString::operator= (AttributedString&& other) noexcept
{
    text = std::move (other.text);
    lineSpacing = other.lineSpacing;
    justification = other.justification;
    wordWrap = other.wordWrap;
    readingDirection = other.readingDirection;
    attributes = std::move (other.attributes);

    return *this;
}

void AttributedString::setText (const String& newText)
{
    auto newLength = newText.length();
    auto oldLength = getLength (attributes);

    if (newLength > oldLength)
        appendRange (attributes, newLength - oldLength, nullptr, nullptr);
    else if (newLength < oldLength)
        truncate (attributes, newLength);

    text = newText;
}

void AttributedString::append (const String& textToAppend)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), nullptr, nullptr);
}

void AttributedString::append (const String& textToAppend, const Font& font)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), &font, nullptr);
}

void AttributedString::append (const String& textToAppend, Colour colour)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), nullptr, &colour);
}

void AttributedString::append (const String& textToAppend, const Font& font, Colour colour)
{
    text += textToAppend;
    appendRange (attributes, textToAppend.length(), &font, &colour);
}

void AttributedString::append (const AttributedString& other)
{
    auto originalLength = getLength (attributes);
    auto originalNumAtts = attributes.size();
    text += other.text;
    attributes.addArray (other.attributes);

    for (auto i = originalNumAtts; i < attributes.size(); ++i)
        attributes.getReference (i).range += originalLength;

    mergeAdjacentRanges (attributes);
}

void AttributedString::clear()
{
    text.clear();
    attributes.clear();
}

void AttributedString::setJustification (Justification newJustification) noexcept
{
    justification = newJustification;
}

void AttributedString::setWordWrap (WordWrap newWordWrap) noexcept
{
    wordWrap = newWordWrap;
}

void AttributedString::setReadingDirection (ReadingDirection newReadingDirection) noexcept
{
    readingDirection = newReadingDirection;
}

void AttributedString::setLineSpacing (const float newLineSpacing) noexcept
{
    lineSpacing = newLineSpacing;
}

void AttributedString::setColour (Range<int> range, Colour colour)
{
    applyFontAndColour (attributes, range, nullptr, &colour);
}

void AttributedString::setFont (Range<int> range, const Font& font)
{
    applyFontAndColour (attributes, range, &font, nullptr);
}

void AttributedString::setColour (Colour colour)
{
    setColour ({ 0, getLength (attributes) }, colour);
}

void AttributedString::setFont (const Font& font)
{
    setFont ({ 0, getLength (attributes) }, font);
}

void AttributedString::draw (Graphics& g, const Rectangle<float>& area) const
{
    if (text.isNotEmpty() && g.clipRegionIntersects (area.getSmallestIntegerContainer()))
    {
        jassert (text.length() == getLength (attributes));

        if (! g.getInternalContext().drawTextLayout (*this, area))
        {
            TextLayout layout;
            layout.createLayout (*this, area.getWidth());
            layout.draw (g, area);
        }
    }
}

} // namespace juce
