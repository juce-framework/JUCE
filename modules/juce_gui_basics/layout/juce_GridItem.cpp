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

GridItem::Property::Property() noexcept : isAuto (true)
{
}

GridItem::Property::Property (GridItem::Keyword keyword) noexcept : isAuto (keyword == GridItem::Keyword::autoValue)
{
    jassert (keyword == GridItem::Keyword::autoValue);
}

GridItem::Property::Property (const char* lineNameToUse) noexcept : GridItem::Property (String (lineNameToUse))
{
}

GridItem::Property::Property (const String& lineNameToUse) noexcept : name (lineNameToUse), number (1)
{
}

GridItem::Property::Property (int numberToUse) noexcept : number (numberToUse)
{
}

GridItem::Property::Property (int numberToUse, const String& lineNameToUse) noexcept
    : name (lineNameToUse), number (numberToUse)
{
}

GridItem::Property::Property (Span spanToUse) noexcept
    : name (spanToUse.name), number (spanToUse.number), isSpan (true)
{
}


//==============================================================================
GridItem::Margin::Margin() noexcept : left(), right(), top(), bottom() {}

GridItem::Margin::Margin (int v) noexcept : GridItem::Margin::Margin (static_cast<float> (v)) {}

GridItem::Margin::Margin (float v) noexcept : left (v), right (v), top (v), bottom (v) {}

GridItem::Margin::Margin (float t, float r, float b, float l) noexcept : left (l), right (r), top (t), bottom (b) {}

//==============================================================================
GridItem::GridItem() noexcept = default;
GridItem::GridItem (Component& componentToUse) noexcept  : associatedComponent (&componentToUse) {}
GridItem::GridItem (Component* componentToUse) noexcept  : associatedComponent (componentToUse) {}

void GridItem::setArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd)
{
    column.start = columnStart;
    column.end = columnEnd;
    row.start = rowStart;
    row.end = rowEnd;
}

void GridItem::setArea (Property rowStart, Property columnStart)
{
    column.start = columnStart;
    row.start = rowStart;
}

void GridItem::setArea (const String& areaName)
{
    area = areaName;
}

GridItem GridItem::withArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd) const noexcept
{
    auto gi = *this;
    gi.setArea (rowStart, columnStart, rowEnd, columnEnd);
    return gi;
}

GridItem GridItem::withArea (Property rowStart, Property columnStart) const noexcept
{
    auto gi = *this;
    gi.setArea (rowStart, columnStart);
    return gi;
}

GridItem GridItem::withArea (const String& areaName) const noexcept
{
    auto gi = *this;
    gi.setArea (areaName);
    return gi;
}

GridItem GridItem::withRow (StartAndEndProperty newRow) const noexcept
{
    auto gi = *this;
    gi.row = newRow;
    return gi;
}

GridItem GridItem::withColumn (StartAndEndProperty newColumn) const noexcept
{
    auto gi = *this;
    gi.column = newColumn;
    return gi;
}

GridItem GridItem::withAlignSelf (AlignSelf newAlignSelf) const noexcept
{
    auto gi = *this;
    gi.alignSelf = newAlignSelf;
    return gi;
}

GridItem GridItem::withJustifySelf (JustifySelf newJustifySelf) const noexcept
{
    auto gi = *this;
    gi.justifySelf = newJustifySelf;
    return gi;
}

GridItem GridItem::withWidth (float newWidth) const noexcept
{
    auto gi = *this;
    gi.width = newWidth;
    return gi;
}

GridItem GridItem::withHeight (float newHeight) const noexcept
{
    auto gi = *this;
    gi.height = newHeight;
    return gi;
}

GridItem GridItem::withSize (float newWidth, float newHeight) const noexcept
{
    auto gi = *this;
    gi.width = newWidth;
    gi.height = newHeight;
    return gi;
}

GridItem GridItem::withMargin (Margin newHeight) const noexcept
{
    auto gi = *this;
    gi.margin = newHeight;
    return gi;
}

GridItem GridItem::withOrder (int newOrder) const noexcept
{
    auto gi = *this;
    gi.order = newOrder;
    return gi;
}

} // namespace juce
