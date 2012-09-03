/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

AttributedString::Attribute::Attribute (const Range<int>& range_, const Colour& colour_)
    : range (range_), colour (new Colour (colour_))
{
}

AttributedString::Attribute::Attribute (const Range<int>& range_, const Font& font_)
    : range (range_), font (new Font (font_))
{
}

AttributedString::Attribute::Attribute (const Attribute& other)
    : range (other.range),
      font (other.font.createCopy()),
      colour (other.colour.createCopy())
{
}

AttributedString::Attribute::Attribute (const Attribute& other, const int offset)
    : range (other.range + offset),
      font (other.font.createCopy()),
      colour (other.colour.createCopy())
{
}

AttributedString::Attribute::~Attribute() {}

//==============================================================================
AttributedString::AttributedString()
    : lineSpacing (0.0f),
      justification (Justification::left),
      wordWrap (AttributedString::byWord),
      readingDirection (AttributedString::natural)
{
}

AttributedString::AttributedString (const String& newString)
    : text (newString),
      lineSpacing (0.0f),
      justification (Justification::left),
      wordWrap (AttributedString::byWord),
      readingDirection (AttributedString::natural)
{
}

AttributedString::AttributedString (const AttributedString& other)
    : text (other.text),
      lineSpacing (other.lineSpacing),
      justification (other.justification),
      wordWrap (other.wordWrap),
      readingDirection (other.readingDirection)
{
    attributes.addCopiesOf (other.attributes);
}

AttributedString& AttributedString::operator= (const AttributedString& other)
{
    if (this != &other)
    {
        text = other.text;
        lineSpacing = other.lineSpacing;
        justification = other.justification;
        wordWrap = other.wordWrap;
        readingDirection = other.readingDirection;
        attributes.clear();
        attributes.addCopiesOf (other.attributes);
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
AttributedString::AttributedString (AttributedString&& other) noexcept
    : text (static_cast <String&&> (other.text)),
      lineSpacing (other.lineSpacing),
      justification (other.justification),
      wordWrap (other.wordWrap),
      readingDirection (other.readingDirection),
      attributes (static_cast <OwnedArray<Attribute>&&> (other.attributes))
{
}

AttributedString& AttributedString::operator= (AttributedString&& other) noexcept
{
    text = static_cast <String&&> (other.text);
    lineSpacing = other.lineSpacing;
    justification = other.justification;
    wordWrap = other.wordWrap;
    readingDirection = other.readingDirection;
    attributes = static_cast <OwnedArray<Attribute>&&> (other.attributes);
    return *this;
}
#endif

AttributedString::~AttributedString() {}

void AttributedString::setText (const String& other)
{
    text = other;
}

void AttributedString::append (const String& textToAppend)
{
    text += textToAppend;
}

void AttributedString::append (const String& textToAppend, const Font& font)
{
    const int oldLength = text.length();
    const int newLength = textToAppend.length();

    text += textToAppend;
    setFont (Range<int> (oldLength, oldLength + newLength), font);
}

void AttributedString::append (const String& textToAppend, const Colour& colour)
{
    const int oldLength = text.length();
    const int newLength = textToAppend.length();

    text += textToAppend;
    setColour (Range<int> (oldLength, oldLength + newLength), colour);
}

void AttributedString::append (const String& textToAppend, const Font& font, const Colour& colour)
{
    const int oldLength = text.length();
    const int newLength = textToAppend.length();

    text += textToAppend;
    setFont (Range<int> (oldLength, oldLength + newLength), font);
    setColour (Range<int> (oldLength, oldLength + newLength), colour);
}

void AttributedString::append (const AttributedString& other)
{
    const int originalLength = text.length();
    text += other.text;

    for (int i = 0; i < other.attributes.size(); ++i)
        attributes.add (new Attribute (*other.attributes.getUnchecked(i), originalLength));
}

void AttributedString::clear()
{
    text = String::empty;
    attributes.clear();
}

void AttributedString::setJustification (const Justification& newJustification) noexcept
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

void AttributedString::setColour (const Range<int>& range, const Colour& colour)
{
    attributes.add (new Attribute (range, colour));
}

void AttributedString::setColour (const Colour& colour)
{
    for (int i = attributes.size(); --i >= 0;)
        if (attributes.getUnchecked(i)->getColour() != nullptr)
            attributes.remove (i);

    setColour (Range<int> (0, text.length()), colour);
}

void AttributedString::setFont (const Range<int>& range, const Font& font)
{
    attributes.add (new Attribute (range, font));
}

void AttributedString::setFont (const Font& font)
{
    for (int i = attributes.size(); --i >= 0;)
        if (attributes.getUnchecked(i)->getFont() != nullptr)
            attributes.remove (i);

    setFont (Range<int> (0, text.length()), font);
}

void AttributedString::draw (Graphics& g, const Rectangle<float>& area) const
{
    if (text.isNotEmpty() && g.clipRegionIntersects (area.getSmallestIntegerContainer()))
    {
        if (! g.getInternalContext().drawTextLayout (*this, area))
        {
            TextLayout layout;
            layout.createLayout (*this, area.getWidth());
            layout.draw (g, area);
        }
    }
}
