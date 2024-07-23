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

struct FlexBoxLayoutCalculation
{
    using Coord = double;

    enum class Axis { main, cross };

    FlexBoxLayoutCalculation (FlexBox& fb, Coord w, Coord h)
        : owner (fb), parentWidth (w), parentHeight (h), numItems (owner.items.size()),
          isRowDirection (fb.flexDirection == FlexBox::Direction::row
                       || fb.flexDirection == FlexBox::Direction::rowReverse),
          containerLineLength (getContainerSize (Axis::main))
    {
        lineItems.calloc (numItems * numItems);
        lineInfo.calloc (numItems);
    }

    struct ItemWithState
    {
        ItemWithState (FlexItem& source) noexcept   : item (&source) {}

        FlexItem* item;
        Coord lockedWidth = 0, lockedHeight = 0;
        Coord lockedMarginLeft = 0, lockedMarginRight = 0, lockedMarginTop = 0, lockedMarginBottom = 0;
        Coord preferredWidth = 0, preferredHeight = 0;
        bool locked = false;

        void resetItemLockedSize() noexcept
        {
            lockedWidth        = preferredWidth;
            lockedHeight       = preferredHeight;
            lockedMarginLeft   = getValueOrZeroIfAuto (item->margin.left);
            lockedMarginRight  = getValueOrZeroIfAuto (item->margin.right);
            lockedMarginTop    = getValueOrZeroIfAuto (item->margin.top);
            lockedMarginBottom = getValueOrZeroIfAuto (item->margin.bottom);
        }
    };

    struct RowInfo
    {
        int numItems;
        Coord crossSize, lineY, totalLength;
    };

    FlexBox& owner;
    const Coord parentWidth, parentHeight;
    const int numItems;
    const bool isRowDirection;
    const Coord containerLineLength;

    int numberOfRows = 1;
    Coord containerCrossLength = 0;

    HeapBlock<ItemWithState*> lineItems;
    HeapBlock<RowInfo> lineInfo;
    Array<ItemWithState> itemStates;

    ItemWithState& getItem (int x, int y) const noexcept     { return *lineItems[y * numItems + x]; }

    static bool isAuto (Coord value) noexcept
    {
        return exactlyEqual (value, static_cast<Coord> (FlexItem::autoValue));
    }

    static bool isAssigned (Coord value) noexcept
    {
        return ! exactlyEqual (value, static_cast<Coord> (FlexItem::notAssigned));
    }

    static Coord getValueOrZeroIfAuto (Coord value) noexcept { return isAuto (value) ? Coord() : value; }

    //==============================================================================
    bool isSingleLine() const { return owner.flexWrap == FlexBox::Wrap::noWrap; }

    template <typename Value>
    Value& pickForAxis (Axis axis, Value& x, Value& y) const
    {
        return (isRowDirection ? axis == Axis::main : axis == Axis::cross) ? x : y;
    }

    auto& getStartMargin (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.item->margin.left, item.item->margin.top);
    }

    auto& getEndMargin (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.item->margin.right, item.item->margin.bottom);
    }

    auto& getStartLockedMargin (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.lockedMarginLeft, item.lockedMarginTop);
    }

    auto& getEndLockedMargin (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.lockedMarginRight, item.lockedMarginBottom);
    }

    auto& getLockedSize (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.lockedWidth, item.lockedHeight);
    }

    auto& getPreferredSize (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.preferredWidth, item.preferredHeight);
    }

    Coord getContainerSize (Axis axis) const
    {
        return pickForAxis (axis, parentWidth, parentHeight);
    }

    auto& getItemSize (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.item->width, item.item->height);
    }

    auto& getMinSize (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.item->minWidth, item.item->minHeight);
    }

    auto& getMaxSize (Axis axis, ItemWithState& item) const
    {
        return pickForAxis (axis, item.item->maxWidth, item.item->maxHeight);
    }

    //==============================================================================
    void createStates()
    {
        itemStates.ensureStorageAllocated (numItems);

        for (auto& item : owner.items)
            itemStates.add (item);

        std::stable_sort (itemStates.begin(), itemStates.end(),
                          [] (const ItemWithState& i1, const ItemWithState& i2)  { return i1.item->order < i2.item->order; });

        for (auto& item : itemStates)
        {
            for (auto& axis : { Axis::main, Axis::cross })
                getPreferredSize (axis, item) = computePreferredSize (axis, item);
        }
    }

    void initialiseItems() noexcept
    {
        if (isSingleLine())  // for single-line, all items go in line 1
        {
            lineInfo[0].numItems = numItems;
            int i = 0;

            for (auto& item : itemStates)
            {
                item.resetItemLockedSize();
                lineItems[i++] = &item;
            }
        }
        else // if multi-line, group the flexbox items into multiple lines
        {
            auto currentLength = containerLineLength;
            int column = 0, row = 0;
            bool firstRow = true;

            for (auto& item : itemStates)
            {
                item.resetItemLockedSize();

                const auto flexitemLength = getItemMainSize (item);

                if (flexitemLength > currentLength)
                {
                    if (! firstRow)
                        row++;

                    if (row >= numItems)
                        break;

                    column = 0;
                    currentLength = containerLineLength;
                    numberOfRows = jmax (numberOfRows, row + 1);
                }

                currentLength -= flexitemLength;
                lineItems[row * numItems + column] = &item;
                ++column;
                lineInfo[row].numItems = jmax (lineInfo[row].numItems, column);
                firstRow = false;
            }
        }
    }

    void resolveFlexibleLengths() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            resetRowItems (row);

            for (int maxLoops = numItems; --maxLoops >= 0;)
            {
                resetUnlockedRowItems (row);

                if (layoutRowItems (row))
                    break;
            }
        }
    }

    void resolveAutoMarginsOnMainAxis() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            Coord allFlexGrow = 0;
            const auto numColumns = lineInfo[row].numItems;
            const auto remainingLength = containerLineLength - lineInfo[row].totalLength;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (isAuto (getStartMargin (Axis::main, item))) ++allFlexGrow;
                if (isAuto (getEndMargin   (Axis::main, item))) ++allFlexGrow;
            }

            const auto changeUnitWidth = remainingLength / allFlexGrow;

            if (changeUnitWidth > 0)
            {
                for (int column = 0; column < numColumns; ++column)
                {
                    auto& item = getItem (column, row);

                    if (isAuto (getStartMargin (Axis::main, item)))
                        getStartLockedMargin (Axis::main, item) = changeUnitWidth;

                    if (isAuto (getEndMargin (Axis::main, item)))
                        getEndLockedMargin   (Axis::main, item) = changeUnitWidth;
                }
            }
        }
    }

    void calculateCrossSizesByLine() noexcept
    {
        // https://www.w3.org/TR/css-flexbox-1/#algo-cross-line
        // If the flex container is single-line and has a definite cross size, the cross size of the
        // flex line is the flex container’s inner cross size.
        if (isSingleLine())
        {
            lineInfo[0].crossSize = getContainerSize (Axis::cross);
        }
        else
        {
            for (int row = 0; row < numberOfRows; ++row)
            {
                Coord maxSize = 0;
                const auto numColumns = lineInfo[row].numItems;

                for (int column = 0; column < numColumns; ++column)
                    maxSize = jmax (maxSize, getItemCrossSize (getItem (column, row)));

                lineInfo[row].crossSize = maxSize;
            }
        }
    }

    void calculateCrossSizeOfAllItems() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (isAssigned (item.item->maxHeight) && item.lockedHeight > item.item->maxHeight)
                    item.lockedHeight = item.item->maxHeight;

                if (isAssigned (item.item->maxWidth) && item.lockedWidth > item.item->maxWidth)
                    item.lockedWidth = item.item->maxWidth;
            }
        }
    }

    void alignLinesPerAlignContent() noexcept
    {
        containerCrossLength = getContainerSize (Axis::cross);

        if (owner.alignContent == FlexBox::AlignContent::flexStart)
        {
            for (int row = 0; row < numberOfRows; ++row)
                for (int row2 = row; row2 < numberOfRows; ++row2)
                    lineInfo[row].lineY = row == 0 ? 0 : lineInfo[row - 1].lineY + lineInfo[row - 1].crossSize;
        }
        else if (owner.alignContent == FlexBox::AlignContent::flexEnd)
        {
            for (int row = 0; row < numberOfRows; ++row)
            {
                Coord crossHeights = 0;

                for (int row2 = row; row2 < numberOfRows; ++row2)
                    crossHeights += lineInfo[row2].crossSize;

                lineInfo[row].lineY = containerCrossLength - crossHeights;
            }
        }
        else
        {
            Coord totalHeight = 0;

            for (int row = 0; row < numberOfRows; ++row)
                totalHeight += lineInfo[row].crossSize;

            if (owner.alignContent == FlexBox::AlignContent::stretch)
            {
                const auto difference = jmax (Coord(), (containerCrossLength - totalHeight) / numberOfRows);

                for (int row = 0; row < numberOfRows; ++row)
                {
                    lineInfo[row].crossSize += difference;
                    lineInfo[row].lineY = row == 0 ? 0 : lineInfo[row - 1].lineY + lineInfo[row - 1].crossSize;
                }
            }
            else if (owner.alignContent == FlexBox::AlignContent::center)
            {
                const auto additionalength = (containerCrossLength - totalHeight) / 2;

                for (int row = 0; row < numberOfRows; ++row)
                    lineInfo[row].lineY = row == 0 ? additionalength : lineInfo[row - 1].lineY + lineInfo[row - 1].crossSize;
            }
            else if (owner.alignContent == FlexBox::AlignContent::spaceBetween)
            {
                const auto additionalength = numberOfRows <= 1 ? Coord() : jmax (Coord(), (containerCrossLength - totalHeight)
                                                                                            / static_cast<Coord> (numberOfRows - 1));
                lineInfo[0].lineY = 0;

                for (int row = 1; row < numberOfRows; ++row)
                    lineInfo[row].lineY += additionalength + lineInfo[row - 1].lineY + lineInfo[row - 1].crossSize;
            }
            else if (owner.alignContent == FlexBox::AlignContent::spaceAround)
            {
                const auto additionalength = numberOfRows <= 1 ? Coord() : jmax (Coord(), (containerCrossLength - totalHeight)
                                                                                            / static_cast<Coord> (2 + (2 * (numberOfRows - 1))));

                lineInfo[0].lineY = additionalength;

                for (int row = 1; row < numberOfRows; ++row)
                    lineInfo[row].lineY += (2 * additionalength) + lineInfo[row - 1].lineY + lineInfo[row - 1].crossSize;
            }
        }
    }

    void resolveAutoMarginsOnCrossAxis() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;
            const auto crossSizeForLine = lineInfo[row].crossSize;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                getStartLockedMargin (Axis::cross, item) = [&]
                {
                    if (isAuto (getStartMargin (Axis::cross, item)) && isAuto (getEndMargin (Axis::cross, item)))
                        return (crossSizeForLine - getLockedSize (Axis::cross, item)) / 2;

                    if (isAuto (getStartMargin (Axis::cross, item)))
                        return crossSizeForLine - getLockedSize (Axis::cross, item) - getEndMargin (Axis::cross, item);

                    return getStartLockedMargin (Axis::cross, item);
                }();
            }
        }
    }

    // Align all flex items along the cross-axis per align-self, if neither of the item’s cross-axis margins are auto.
    void alignItemsInCrossAxisInLinesPerAlignSelf() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;
            const auto lineSize = lineInfo[row].crossSize;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (isAuto (getStartMargin (Axis::cross, item)) || isAuto (getEndMargin (Axis::cross, item)))
                    continue;

                const auto alignment = [&]
                {
                    switch (item.item->alignSelf)
                    {
                        case FlexItem::AlignSelf::stretch:      return FlexBox::AlignItems::stretch;
                        case FlexItem::AlignSelf::flexStart:    return FlexBox::AlignItems::flexStart;
                        case FlexItem::AlignSelf::flexEnd:      return FlexBox::AlignItems::flexEnd;
                        case FlexItem::AlignSelf::center:       return FlexBox::AlignItems::center;
                        case FlexItem::AlignSelf::autoAlign:    break;
                    }

                    return owner.alignItems;
                }();

                getStartLockedMargin (Axis::cross, item) = [&]
                {
                    switch (alignment)
                    {
                        // https://www.w3.org/TR/css-flexbox-1/#valdef-align-items-flex-start
                        // The cross-start margin edge of the flex item is placed flush with the
                        // cross-start edge of the line.
                        case FlexBox::AlignItems::flexStart:
                            return (Coord) getStartMargin (Axis::cross, item);

                        // https://www.w3.org/TR/css-flexbox-1/#valdef-align-items-flex-end
                        // The cross-end margin edge of the flex item is placed flush with the cross-end
                        // edge of the line.
                        case FlexBox::AlignItems::flexEnd:
                            return lineSize - getLockedSize (Axis::cross, item) - getEndMargin (Axis::cross, item);

                        // https://www.w3.org/TR/css-flexbox-1/#valdef-align-items-center
                        // The flex item’s margin box is centered in the cross axis within the line.
                        case FlexBox::AlignItems::center:
                            return getStartMargin (Axis::cross, item) + (lineSize - getLockedSize (Axis::cross, item) - getStartMargin (Axis::cross, item) - getEndMargin (Axis::cross, item)) / 2;

                        // https://www.w3.org/TR/css-flexbox-1/#valdef-align-items-stretch
                        case FlexBox::AlignItems::stretch:
                            return (Coord) getStartMargin (Axis::cross, item);
                    }

                    jassertfalse;
                    return 0.0;
                }();

                if (alignment == FlexBox::AlignItems::stretch)
                {
                    auto newSize = isAssigned (getItemSize (Axis::cross, item)) ? computePreferredSize (Axis::cross, item)
                                                                                : lineSize - getStartMargin (Axis::cross, item) - getEndMargin (Axis::cross, item);

                    if (isAssigned (getMaxSize (Axis::cross, item)))
                        newSize = jmin (newSize, (Coord) getMaxSize (Axis::cross, item));

                    if (isAssigned (getMinSize (Axis::cross, item)))
                        newSize = jmax (newSize, (Coord) getMinSize (Axis::cross, item));

                    getLockedSize (Axis::cross, item) = newSize;
                }
            }
        }
    }

    void alignItemsByJustifyContent() noexcept
    {
        Coord additionalMarginRight = 0, additionalMarginLeft = 0;

        recalculateTotalItemLengthPerLineArray();

        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;
            Coord x = 0;

            if (owner.justifyContent == FlexBox::JustifyContent::flexEnd)
            {
                x = containerLineLength - lineInfo[row].totalLength;
            }
            else if (owner.justifyContent == FlexBox::JustifyContent::center)
            {
                x = (containerLineLength - lineInfo[row].totalLength) / 2;
            }
            else if (owner.justifyContent == FlexBox::JustifyContent::spaceBetween)
            {
                additionalMarginRight
                    = jmax (Coord(), (containerLineLength - lineInfo[row].totalLength) / jmax (1, numColumns - 1));
            }
            else if (owner.justifyContent == FlexBox::JustifyContent::spaceAround)
            {
                additionalMarginLeft = additionalMarginRight
                    = jmax (Coord(), (containerLineLength - lineInfo[row].totalLength) / jmax (1, 2 * numColumns));
            }

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                getStartLockedMargin (Axis::main, item) += additionalMarginLeft;
                getEndLockedMargin   (Axis::main, item) += additionalMarginRight;

                item.item->currentBounds.setPosition (isRowDirection ? (float) (x + item.lockedMarginLeft)
                                                                     : (float) item.lockedMarginLeft,
                                                      isRowDirection ? (float) item.lockedMarginTop
                                                                     : (float) (x + item.lockedMarginTop));

                x += getItemMainSize (item);
            }
        }
    }

    void layoutAllItems() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto lineY = lineInfo[row].lineY;
            const auto numColumns = lineInfo[row].numItems;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (isRowDirection)
                    item.item->currentBounds.setY ((float) (lineY + item.lockedMarginTop));
                else
                    item.item->currentBounds.setX ((float) (lineY + item.lockedMarginLeft));

                item.item->currentBounds.setSize ((float) item.lockedWidth,
                                                  (float) item.lockedHeight);
            }
        }

        reverseLocations();
        reverseWrap();
    }

private:
    void resetRowItems (const int row) noexcept
    {
        const auto numColumns = lineInfo[row].numItems;

        for (int column = 0; column < numColumns; ++column)
            resetItem (getItem (column, row));
    }

    void resetUnlockedRowItems (const int row) noexcept
    {
        const auto numColumns = lineInfo[row].numItems;

        for (int column = 0; column < numColumns; ++column)
        {
            auto& item = getItem (column, row);

            if (! item.locked)
                resetItem (item);
        }
    }

    void resetItem (ItemWithState& item) noexcept
    {
        item.locked = false;

        for (auto& axis : { Axis::main, Axis::cross })
            getLockedSize (axis, item) = computePreferredSize (axis, item);
    }

    bool layoutRowItems (const int row) noexcept
    {
        const auto numColumns = lineInfo[row].numItems;
        auto flexContainerLength = containerLineLength;
        Coord totalItemsLength = 0, totalFlexGrow = 0, totalFlexShrink = 0;

        for (int column = 0; column < numColumns; ++column)
        {
            const auto& item = getItem (column, row);

            if (item.locked)
            {
                flexContainerLength -= getItemMainSize (item);
            }
            else
            {
                totalItemsLength += getItemMainSize (item);
                totalFlexGrow   += item.item->flexGrow;
                totalFlexShrink += item.item->flexShrink;
            }
        }

        Coord changeUnit = 0;
        const auto difference = flexContainerLength - totalItemsLength;
        const bool positiveFlexibility = difference > 0;

        if (positiveFlexibility)
        {
            if (! approximatelyEqual (totalFlexGrow, 0.0))
                changeUnit = difference / totalFlexGrow;
        }
        else
        {
            if (! approximatelyEqual (totalFlexShrink, 0.0))
                changeUnit = difference / totalFlexShrink;
        }

        bool ok = true;

        for (int column = 0; column < numColumns; ++column)
        {
            auto& item = getItem (column, row);

            if (! item.locked)
                if (! addToItemLength (item, (positiveFlexibility ? item.item->flexGrow
                                                                  : item.item->flexShrink) * changeUnit, row))
                    ok = false;
        }

        return ok;
    }

    void recalculateTotalItemLengthPerLineArray() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            lineInfo[row].totalLength = 0;
            const auto numColumns = lineInfo[row].numItems;

            for (int column = 0; column < numColumns; ++column)
                lineInfo[row].totalLength += getItemMainSize (getItem (column, row));
        }
    }

    void reverseLocations() noexcept
    {
        if (owner.flexDirection == FlexBox::Direction::rowReverse)
        {
            for (auto& item : owner.items)
                item.currentBounds.setX ((float) (containerLineLength - item.currentBounds.getRight()));
        }
        else if (owner.flexDirection == FlexBox::Direction::columnReverse)
        {
            for (auto& item : owner.items)
                item.currentBounds.setY ((float) (containerLineLength - item.currentBounds.getBottom()));
        }
    }

    void reverseWrap() noexcept
    {
        if (owner.flexWrap == FlexBox::Wrap::wrapReverse)
        {
            if (isRowDirection)
            {
                for (auto& item : owner.items)
                    item.currentBounds.setY ((float) (containerCrossLength - item.currentBounds.getBottom()));
            }
            else
            {
                for (auto& item : owner.items)
                    item.currentBounds.setX ((float) (containerCrossLength - item.currentBounds.getRight()));
            }
        }
    }

    Coord getItemMainSize (const ItemWithState& item) const noexcept
    {
        return isRowDirection ? item.lockedWidth  + item.lockedMarginLeft + item.lockedMarginRight
                              : item.lockedHeight + item.lockedMarginTop  + item.lockedMarginBottom;
    }

    Coord getItemCrossSize (const ItemWithState& item) const noexcept
    {
        return isRowDirection ? item.lockedHeight + item.lockedMarginTop  + item.lockedMarginBottom
                              : item.lockedWidth  + item.lockedMarginLeft + item.lockedMarginRight;
    }

    bool addToItemLength (ItemWithState& item, const Coord length, int row) const noexcept
    {
        bool ok = false;

        const auto prefSize = computePreferredSize (Axis::main, item);

        const auto pickForMainAxis = [this] (auto& a, auto& b) -> auto& { return pickForAxis (Axis::main, a, b); };

        if (isAssigned (pickForMainAxis (item.item->maxWidth, item.item->maxHeight))
            && pickForMainAxis (item.item->maxWidth, item.item->maxHeight) < prefSize + length)
        {
            pickForMainAxis (item.lockedWidth, item.lockedHeight) = pickForMainAxis (item.item->maxWidth, item.item->maxHeight);
            item.locked = true;
        }
        else if (isAssigned (prefSize) && pickForMainAxis (item.item->minWidth, item.item->minHeight) > prefSize + length)
        {
            pickForMainAxis (item.lockedWidth, item.lockedHeight) = pickForMainAxis (item.item->minWidth, item.item->minHeight);
            item.locked = true;
        }
        else
        {
            ok = true;
            pickForMainAxis (item.lockedWidth, item.lockedHeight) = prefSize + length;
        }

        lineInfo[row].totalLength += pickForMainAxis (item.lockedWidth, item.lockedHeight)
                                     + pickForMainAxis (item.lockedMarginLeft, item.lockedMarginTop)
                                     + pickForMainAxis (item.lockedMarginRight, item.lockedMarginBottom);

        return ok;
    }

    Coord computePreferredSize (Axis axis, ItemWithState& itemWithState) const noexcept
    {
        const auto& item = *itemWithState.item;

        auto preferredSize = (item.flexBasis > 0 && axis == Axis::main) ? item.flexBasis
                                                                        : (isAssigned (getItemSize (axis, itemWithState)) ? getItemSize (axis, itemWithState)
                                                                                                                          : getMinSize (axis, itemWithState));

        const auto minSize = getMinSize (axis, itemWithState);

        if (isAssigned (minSize) && preferredSize < minSize)
            return minSize;

        const auto maxSize = getMaxSize (axis, itemWithState);

        if (isAssigned (maxSize) && maxSize < preferredSize)
            return maxSize;

        return preferredSize;
    }
};

//==============================================================================
FlexBox::FlexBox (JustifyContent jc) noexcept  : justifyContent (jc) {}

FlexBox::FlexBox (Direction d, Wrap w, AlignContent ac, AlignItems ai, JustifyContent jc) noexcept
    : flexDirection (d), flexWrap (w), alignContent (ac), alignItems (ai), justifyContent (jc)
{
}

void FlexBox::performLayout (Rectangle<float> targetArea)
{
    if (! items.isEmpty())
    {
        FlexBoxLayoutCalculation layout (*this, targetArea.getWidth(), targetArea.getHeight());

        layout.createStates();
        layout.initialiseItems();
        layout.resolveFlexibleLengths();
        layout.resolveAutoMarginsOnMainAxis();
        layout.calculateCrossSizesByLine();
        layout.calculateCrossSizeOfAllItems();
        layout.alignLinesPerAlignContent();
        layout.resolveAutoMarginsOnCrossAxis();
        layout.alignItemsInCrossAxisInLinesPerAlignSelf();
        layout.alignItemsByJustifyContent();
        layout.layoutAllItems();

        for (auto& item : items)
        {
            item.currentBounds += targetArea.getPosition();

            if (auto* comp = item.associatedComponent)
                comp->setBounds (Rectangle<int>::leftTopRightBottom ((int) item.currentBounds.getX(),
                                                                     (int) item.currentBounds.getY(),
                                                                     (int) item.currentBounds.getRight(),
                                                                     (int) item.currentBounds.getBottom()));

            if (auto* box = item.associatedFlexBox)
                box->performLayout (item.currentBounds);
        }
    }
}

void FlexBox::performLayout (Rectangle<int> targetArea)
{
    performLayout (targetArea.toFloat());
}

//==============================================================================
FlexItem::FlexItem() noexcept {}
FlexItem::FlexItem (float w, float h) noexcept                  : currentBounds (w, h), minWidth (w), minHeight (h) {}
FlexItem::FlexItem (float w, float h, Component& c) noexcept    : FlexItem (w, h)  { associatedComponent = &c; }
FlexItem::FlexItem (float w, float h, FlexBox& fb) noexcept     : FlexItem (w, h)  { associatedFlexBox = &fb; }
FlexItem::FlexItem (Component& c) noexcept                      : associatedComponent (&c) {}
FlexItem::FlexItem (FlexBox& fb) noexcept                       : associatedFlexBox (&fb) {}

FlexItem::Margin::Margin() noexcept                                     : left(), right(), top(), bottom() {}
FlexItem::Margin::Margin (float v) noexcept                             : left (v), right (v), top (v), bottom (v) {}
FlexItem::Margin::Margin (float t, float r, float b, float l) noexcept  : left (l), right (r), top (t), bottom (b) {}

//==============================================================================
FlexItem FlexItem::withFlex (float newFlexGrow) const noexcept
{
    auto fi = *this;
    fi.flexGrow = newFlexGrow;
    return fi;
}

FlexItem FlexItem::withFlex (float newFlexGrow, float newFlexShrink) const noexcept
{
    auto fi = withFlex (newFlexGrow);
    fi.flexShrink = newFlexShrink;
    return fi;
}

FlexItem FlexItem::withFlex (float newFlexGrow, float newFlexShrink, float newFlexBasis) const noexcept
{
    auto fi = withFlex (newFlexGrow, newFlexShrink);
    fi.flexBasis = newFlexBasis;
    return fi;
}

FlexItem FlexItem::withWidth (float newWidth) const noexcept         { auto fi = *this; fi.width = newWidth; return fi; }
FlexItem FlexItem::withMinWidth (float newMinWidth) const noexcept   { auto fi = *this; fi.minWidth = newMinWidth; return fi; }
FlexItem FlexItem::withMaxWidth (float newMaxWidth) const noexcept   { auto fi = *this; fi.maxWidth = newMaxWidth; return fi; }

FlexItem FlexItem::withMinHeight (float newMinHeight) const noexcept { auto fi = *this; fi.minHeight = newMinHeight; return fi; }
FlexItem FlexItem::withMaxHeight (float newMaxHeight) const noexcept { auto fi = *this; fi.maxHeight = newMaxHeight; return fi; }
FlexItem FlexItem::withHeight (float newHeight) const noexcept       { auto fi = *this; fi.height = newHeight; return fi; }

FlexItem FlexItem::withMargin (Margin m) const noexcept              { auto fi = *this; fi.margin = m; return fi; }
FlexItem FlexItem::withOrder (int newOrder) const noexcept           { auto fi = *this; fi.order = newOrder; return fi; }
FlexItem FlexItem::withAlignSelf (AlignSelf a) const noexcept        { auto fi = *this; fi.alignSelf = a; return fi; }

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class FlexBoxTests final : public UnitTest
{
public:
    FlexBoxTests() : UnitTest ("FlexBox", UnitTestCategories::gui) {}

    void runTest() override
    {
        using AlignSelf = FlexItem::AlignSelf;
        using Direction = FlexBox::Direction;

        const Rectangle<float> rect (10.0f, 20.0f, 300.0f, 200.0f);
        const auto doLayout = [&rect] (Direction direction, Array<FlexItem> items)
        {
            juce::FlexBox flex;
            flex.flexDirection = direction;
            flex.items = std::move (items);
            flex.performLayout (rect);
            return flex;
        };

        beginTest ("flex item with mostly auto properties");
        {
            const auto test = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem{}.withAlignSelf (alignment) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            test (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), 0.0f, rect.getHeight() });
            test (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), 0.0f, rect.getHeight() });
            test (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom(), 0.0f, 0.0f });
            test (Direction::row, AlignSelf::center,    { rect.getX(), rect.getCentreY(), 0.0f, 0.0f });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), rect.getWidth(), 0.0f });
            test (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), rect.getWidth(), 0.0f });
            test (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight(), rect.getY(), 0.0f, 0.0f });
            test (Direction::column, AlignSelf::center,    { rect.getCentreX(), rect.getY(), 0.0f, 0.0f });
        }

        beginTest ("flex item with specified width and height");
        {
            constexpr auto w = 50.0f;
            constexpr auto h = 60.0f;
            const auto test = [&] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withWidth (w)
                                                                         .withHeight (h) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            test (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), w, h });
            test (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), w, h });
            test (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), w, h });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom() - h, w, h });
            test (Direction::row, AlignSelf::center,    { rect.getX(), rect.getY() + (rect.getHeight() - h) * 0.5f, w, h });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), w, h });
            test (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), w, h });
            test (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), w, h });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight() - w, rect.getY(), w, h });
            test (Direction::column, AlignSelf::center,    { rect.getX() + (rect.getWidth() - w) * 0.5f, rect.getY(), w, h });
        }

        beginTest ("flex item with oversized width and height");
        {
            const auto w = rect.getWidth() * 2;
            const auto h = rect.getHeight() * 2;
            const auto test = [this, &doLayout, &w, &h] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withWidth (w)
                                                                         .withHeight (h) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            const Rectangle<float> baseRow (rect.getX(), rect.getY(), rect.getWidth(), h);
            test (Direction::row, AlignSelf::autoAlign, baseRow);
            test (Direction::row, AlignSelf::stretch,   baseRow);
            test (Direction::row, AlignSelf::flexStart, baseRow);
            test (Direction::row, AlignSelf::flexEnd,   baseRow.withBottomY (rect.getBottom()));
            test (Direction::row, AlignSelf::center,    baseRow.withCentre (rect.getCentre()));

            const Rectangle<float> baseColumn (rect.getX(), rect.getY(), w, rect.getHeight());
            test (Direction::column, AlignSelf::autoAlign, baseColumn);
            test (Direction::column, AlignSelf::stretch,   baseColumn);
            test (Direction::column, AlignSelf::flexStart, baseColumn);
            test (Direction::column, AlignSelf::flexEnd,   baseColumn.withRightX (rect.getRight()));
            test (Direction::column, AlignSelf::center,    baseColumn.withCentre (rect.getCentre()));
        }

        beginTest ("flex item with minimum width and height");
        {
            constexpr auto w = 50.0f;
            constexpr auto h = 60.0f;
            const auto test = [&] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMinWidth (w)
                                                                         .withMinHeight (h) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            test (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), w, rect.getHeight() });
            test (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), w, rect.getHeight() });
            test (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), w, h });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom() - h, w, h });
            test (Direction::row, AlignSelf::center,    { rect.getX(), rect.getY() + (rect.getHeight() - h) * 0.5f, w, h });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), rect.getWidth(), h });
            test (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), rect.getWidth(), h });
            test (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), w, h });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight() - w, rect.getY(), w, h });
            test (Direction::column, AlignSelf::center,    { rect.getX() + (rect.getWidth() - w) * 0.5f, rect.getY(), w, h });
        }

        beginTest ("flex item with maximum width and height");
        {
            constexpr auto w = 50.0f;
            constexpr auto h = 60.0f;
            const auto test = [&] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMaxWidth (w)
                                                                         .withMaxHeight (h) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            test (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), 0.0f, h });
            test (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), 0.0f, h });
            test (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom(), 0.0f, 0.0f });
            test (Direction::row, AlignSelf::center,    { rect.getX(), rect.getCentreY(), 0.0f, 0.0f });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), w, 0.0f });
            test (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), w, 0.0f });
            test (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight(), rect.getY(), 0.0f, 0.0f });
            test (Direction::column, AlignSelf::center,    { rect.getCentreX(), rect.getY(), 0.0f, 0.0f });
        }

        beginTest ("flex item with specified flex");
        {
            const auto test = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment).withFlex (1.0f) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            test (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight() });
            test (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight() });
            test (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), rect.getWidth(), 0.0f });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom(), rect.getWidth(), 0.0f });
            test (Direction::row, AlignSelf::center,    { rect.getX(), rect.getCentreY(), rect.getWidth(), 0.0f });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight() });
            test (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight() });
            test (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, rect.getHeight() });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight(), rect.getY(), 0.0f, rect.getHeight() });
            test (Direction::column, AlignSelf::center,    { rect.getCentreX(), rect.getY(), 0.0f, rect.getHeight() });
        }

        beginTest ("flex item with margin");
        {
            const FlexItem::Margin margin (10.0f, 20.0f, 30.0f, 40.0f);

            const auto test = [this, &doLayout, &margin] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment).withMargin (margin) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            const auto remainingHeight = rect.getHeight() - margin.top - margin.bottom;
            const auto remainingWidth = rect.getWidth() - margin.left - margin.right;

            test (Direction::row, AlignSelf::autoAlign, { rect.getX() + margin.left, rect.getY() + margin.top, 0.0f, remainingHeight });
            test (Direction::row, AlignSelf::stretch,   { rect.getX() + margin.left, rect.getY() + margin.top, 0.0f, remainingHeight });
            test (Direction::row, AlignSelf::flexStart, { rect.getX() + margin.left, rect.getY() + margin.top, 0.0f, 0.0f });
            test (Direction::row, AlignSelf::flexEnd,   { rect.getX() + margin.left, rect.getBottom() - margin.bottom, 0.0f, 0.0f });
            test (Direction::row, AlignSelf::center,    { rect.getX() + margin.left, rect.getY() + margin.top + remainingHeight * 0.5f, 0.0f, 0.0f });

            test (Direction::column, AlignSelf::autoAlign, { rect.getX() + margin.left, rect.getY() + margin.top, remainingWidth, 0.0f });
            test (Direction::column, AlignSelf::stretch,   { rect.getX() + margin.left, rect.getY() + margin.top, remainingWidth, 0.0f });
            test (Direction::column, AlignSelf::flexStart, { rect.getX() + margin.left, rect.getY() + margin.top, 0.0f, 0.0f });
            test (Direction::column, AlignSelf::flexEnd,   { rect.getRight() - margin.right, rect.getY() + margin.top, 0.0f, 0.0f });
            test (Direction::column, AlignSelf::center,    { rect.getX() + margin.left + remainingWidth * 0.5f, rect.getY() + margin.top, 0.0f, 0.0f });
        }

        const AlignSelf alignments[] { AlignSelf::autoAlign,
                                       AlignSelf::stretch,
                                       AlignSelf::flexStart,
                                       AlignSelf::flexEnd,
                                       AlignSelf::center };

        beginTest ("flex item with auto margin");
        {
            for (const auto& alignment : alignments)
            {
                for (const auto& direction : { Direction::row, Direction::column })
                {
                    const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                             .withMargin ((float) FlexItem::autoValue) });
                    expect (flex.items.getFirst().currentBounds == Rectangle<float> (rect.getCentre(), rect.getCentre()));
                }
            }

            const auto testTop = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMargin ({ (float) FlexItem::autoValue, 0.0f, 0.0f, 0.0f }) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            for (const auto& alignment : alignments)
                testTop (Direction::row, alignment, { rect.getX(), rect.getBottom(), 0.0f, 0.0f });

            testTop (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getBottom(), rect.getWidth(), 0.0f });
            testTop (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getBottom(), rect.getWidth(), 0.0f });
            testTop (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getBottom(), 0.0f, 0.0f });
            testTop (Direction::column, AlignSelf::flexEnd,   { rect.getRight(), rect.getBottom(), 0.0f, 0.0f });
            testTop (Direction::column, AlignSelf::center,    { rect.getCentreX(), rect.getBottom(), 0.0f, 0.0f });

            const auto testBottom = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMargin ({ 0.0f, 0.0f, (float) FlexItem::autoValue, 0.0f }) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            for (const auto& alignment : alignments)
                testBottom (Direction::row, alignment, { rect.getX(), rect.getY(), 0.0f, 0.0f });

            testBottom (Direction::column, AlignSelf::autoAlign, { rect.getX(), rect.getY(), rect.getWidth(), 0.0f });
            testBottom (Direction::column, AlignSelf::stretch,   { rect.getX(), rect.getY(), rect.getWidth(), 0.0f });
            testBottom (Direction::column, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            testBottom (Direction::column, AlignSelf::flexEnd,   { rect.getRight(), rect.getY(), 0.0f, 0.0f });
            testBottom (Direction::column, AlignSelf::center,    { rect.getCentreX(), rect.getY(), 0.0f, 0.0f });

            const auto testLeft = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMargin ({ 0.0f, 0.0f, 0.0f, (float) FlexItem::autoValue }) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            testLeft (Direction::row, AlignSelf::autoAlign, { rect.getRight(), rect.getY(), 0.0f, rect.getHeight() });
            testLeft (Direction::row, AlignSelf::stretch,   { rect.getRight(), rect.getY(), 0.0f, rect.getHeight() });
            testLeft (Direction::row, AlignSelf::flexStart, { rect.getRight(), rect.getY(), 0.0f, 0.0f });
            testLeft (Direction::row, AlignSelf::flexEnd,   { rect.getRight(), rect.getBottom(), 0.0f, 0.0f });
            testLeft (Direction::row, AlignSelf::center,    { rect.getRight(), rect.getCentreY(), 0.0f, 0.0f });

            for (const auto& alignment : alignments)
                testLeft (Direction::column, alignment, { rect.getRight(), rect.getY(), 0.0f, 0.0f });

            const auto testRight = [this, &doLayout] (Direction direction, AlignSelf alignment, Rectangle<float> expectedBounds)
            {
                const auto flex = doLayout (direction, { juce::FlexItem().withAlignSelf (alignment)
                                                                         .withMargin ({ 0.0f, (float) FlexItem::autoValue, 0.0f, 0.0f }) });
                expect (flex.items.getFirst().currentBounds == expectedBounds);
            };

            testRight (Direction::row, AlignSelf::autoAlign, { rect.getX(), rect.getY(), 0.0f, rect.getHeight() });
            testRight (Direction::row, AlignSelf::stretch,   { rect.getX(), rect.getY(), 0.0f, rect.getHeight() });
            testRight (Direction::row, AlignSelf::flexStart, { rect.getX(), rect.getY(), 0.0f, 0.0f });
            testRight (Direction::row, AlignSelf::flexEnd,   { rect.getX(), rect.getBottom(), 0.0f, 0.0f });
            testRight (Direction::row, AlignSelf::center,    { rect.getX(), rect.getCentreY(), 0.0f, 0.0f });

            for (const auto& alignment : alignments)
                testRight (Direction::column, alignment, { rect.getX(), rect.getY(), 0.0f, 0.0f });
        }

        beginTest ("in a multiline layout, items too large to fit on the main axis are given a line to themselves");
        {
            const auto spacer = 10.0f;

            for (const auto alignment : alignments)
            {
                juce::FlexBox flex;
                flex.flexWrap = FlexBox::Wrap::wrap;
                flex.items = { FlexItem().withAlignSelf (alignment)
                                         .withWidth (spacer)
                                         .withHeight (spacer),
                               FlexItem().withAlignSelf (alignment)
                                         .withWidth (rect.getWidth() * 2)
                                         .withHeight (rect.getHeight()),
                               FlexItem().withAlignSelf (alignment)
                                         .withWidth (spacer)
                                         .withHeight (spacer) };
                flex.performLayout (rect);

                expect (flex.items[0].currentBounds == Rectangle<float> (rect.getX(), rect.getY(), spacer, spacer));
                expect (flex.items[1].currentBounds == Rectangle<float> (rect.getX(), rect.getY() + spacer, rect.getWidth(), rect.getHeight()));
                expect (flex.items[2].currentBounds == Rectangle<float> (rect.getX(), rect.getBottom() + spacer, 10.0f, 10.0f));
            }
        }
    }
};

static FlexBoxTests flexBoxTests;

#endif

} // namespace juce
