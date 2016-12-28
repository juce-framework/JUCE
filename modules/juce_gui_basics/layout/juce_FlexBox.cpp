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

struct FlexBoxLayoutCalculation
{
    using Coord = double;

    FlexBoxLayoutCalculation (const FlexBox& fb, Coord w, Coord h)
       : owner (fb), parentWidth (w), parentHeight (h), numItems (owner.items.size()),
         isRowDirection (fb.flexDirection == FlexBox::Direction::row
                          || fb.flexDirection == FlexBox::Direction::rowReverse),
         containerLineLength (isRowDirection ? parentWidth : parentHeight)
    {
        lineItems.calloc ((size_t) (numItems * numItems));
        lineInfo.calloc ((size_t) numItems);
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

        void setWidthChecked (Coord newWidth) noexcept
        {
            if (isAssigned (item->maxWidth))  newWidth = jmin (newWidth, static_cast<Coord> (item->maxWidth));
            if (isAssigned (item->minWidth))  newWidth = jmax (newWidth, static_cast<Coord> (item->minWidth));

            lockedWidth = newWidth;
        }

        void setHeightChecked (Coord newHeight) noexcept
        {
            if (isAssigned (item->maxHeight))  newHeight = jmin (newHeight, (Coord) item->maxHeight);
            if (isAssigned (item->minHeight))  newHeight = jmax (newHeight, (Coord) item->minHeight);

            lockedHeight = newHeight;
        }
    };

    struct RowInfo
    {
        int numItems;
        Coord crossSize, lineY, totalLength;
    };

    const FlexBox& owner;
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

    static bool isAuto (Coord value) noexcept                { return value == FlexItem::autoValue; }
    static bool isAssigned (Coord value) noexcept            { return value != FlexItem::notAssigned; }
    static Coord getValueOrZeroIfAuto (Coord value) noexcept { return isAuto (value) ? Coord() : value; }

    //==============================================================================
    void createStates()
    {
        itemStates.ensureStorageAllocated (numItems);

        for (auto& item : owner.items)
            itemStates.add (item);

        itemStates.sort (*this, true);

        for (auto& item : itemStates)
        {
            item.preferredWidth  = getPreferredWidth  (item);
            item.preferredHeight = getPreferredHeight (item);
        }
    }

    void initialiseItems() noexcept
    {
        if (owner.flexWrap == FlexBox::Wrap::noWrap)  // for single-line, all items go in line 1
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

                const auto flexitemLength = getItemLength (item);

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

                if (isRowDirection)
                {
                    if (isAuto (item.item->margin.left))    ++allFlexGrow;
                    if (isAuto (item.item->margin.right))   ++allFlexGrow;
                }
                else
                {
                    if (isAuto (item.item->margin.top))     ++allFlexGrow;
                    if (isAuto (item.item->margin.bottom))  ++allFlexGrow;
                }
            }

            auto changeUnitWidth = remainingLength / allFlexGrow;

            if (changeUnitWidth > 0)
            {
                for (int column = 0; column < numColumns; ++column)
                {
                    auto& item = getItem (column, row);

                    if (isRowDirection)
                    {
                        if (isAuto (item.item->margin.left))    item.lockedMarginLeft  = changeUnitWidth;
                        if (isAuto (item.item->margin.right))   item.lockedMarginRight = changeUnitWidth;
                    }
                    else
                    {
                        if (isAuto (item.item->margin.top))     item.lockedMarginTop    = changeUnitWidth;
                        if (isAuto (item.item->margin.bottom))  item.lockedMarginBottom = changeUnitWidth;
                    }
                }
            }
        }
    }

    void calculateCrossSizesByLine() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            Coord maxSize = 0;
            const auto numColumns = lineInfo[row].numItems;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                maxSize = jmax (maxSize, isRowDirection ? item.lockedHeight + item.lockedMarginTop  + item.lockedMarginBottom
                                                        : item.lockedWidth  + item.lockedMarginLeft + item.lockedMarginRight);
            }

            lineInfo[row].crossSize = maxSize;
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
        containerCrossLength = isRowDirection ? parentHeight : parentWidth;

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

                if (isRowDirection)
                {
                    if (isAuto (item.item->margin.top) && isAuto (item.item->margin.bottom))
                        item.lockedMarginTop = (crossSizeForLine - item.lockedHeight) / 2;
                    else if (isAuto (item.item->margin.top))
                        item.lockedMarginTop = crossSizeForLine - item.lockedHeight - item.item->margin.bottom;
                }
                else if (isAuto (item.item->margin.left) && isAuto (item.item->margin.right))
                {
                    item.lockedMarginLeft = jmax (Coord(), (crossSizeForLine - item.lockedWidth) / 2);
                }
                else if (isAuto (item.item->margin.top))
                {
                    item.lockedMarginLeft = jmax (Coord(), crossSizeForLine - item.lockedHeight - item.item->margin.bottom);
                }
            }
        }
    }

    void alignItemsInCrossAxisInLinesPerAlignItems() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;
            const auto lineSize = lineInfo[row].crossSize;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (item.item->alignSelf == FlexItem::AlignSelf::autoAlign)
                {
                    if (owner.alignItems == FlexBox::AlignItems::stretch)
                    {
                        item.lockedMarginTop = item.item->margin.top;

                        if (isRowDirection)
                            item.setHeightChecked (lineSize - item.item->margin.top - item.item->margin.bottom);
                    }
                    else if (owner.alignItems == FlexBox::AlignItems::flexStart)
                    {
                        item.lockedMarginTop = item.item->margin.top;
                    }
                    else if (owner.alignItems == FlexBox::AlignItems::flexEnd)
                    {
                        item.lockedMarginTop = lineSize - item.lockedHeight - item.item->margin.bottom;
                    }
                    else if (owner.alignItems == FlexBox::AlignItems::center)
                    {
                        item.lockedMarginTop = (lineSize - item.lockedHeight - item.item->margin.top - item.item->margin.bottom) / 2;
                    }
                }
            }
        }
    }

    void alignLinesPerAlignSelf() noexcept
    {
        for (int row = 0; row < numberOfRows; ++row)
        {
            const auto numColumns = lineInfo[row].numItems;
            const auto lineSize = lineInfo[row].crossSize;

            for (int column = 0; column < numColumns; ++column)
            {
                auto& item = getItem (column, row);

                if (! isAuto (item.item->margin.top))
                {
                    if (item.item->alignSelf == FlexItem::AlignSelf::flexStart)
                    {
                        if (isRowDirection)
                            item.lockedMarginTop = item.item->margin.top;
                        else
                            item.lockedMarginLeft = item.item->margin.left;
                    }
                    else if (item.item->alignSelf == FlexItem::AlignSelf::flexEnd)
                    {
                        if (isRowDirection)
                            item.lockedMarginTop = lineSize - item.lockedHeight - item.item->margin.bottom;
                        else
                            item.lockedMarginLeft = lineSize - item.lockedWidth - item.item->margin.right;
                    }
                    else if (item.item->alignSelf == FlexItem::AlignSelf::center)
                    {
                        if (isRowDirection)
                            item.lockedMarginTop = item.item->margin.top + (lineSize - item.lockedHeight - item.item->margin.top - item.item->margin.bottom) / 2;
                        else
                            item.lockedMarginLeft = item.item->margin.left + (lineSize - item.lockedWidth - item.item->margin.left - item.item->margin.right) / 2;
                    }
                    else if (item.item->alignSelf == FlexItem::AlignSelf::stretch)
                    {
                        item.lockedMarginTop  = item.item->margin.top;
                        item.lockedMarginLeft = item.item->margin.left;

                        if (isRowDirection)
                            item.setHeightChecked (isAssigned (item.item->height) ? getPreferredHeight (item)
                                                                                  : lineSize - item.item->margin.top - item.item->margin.bottom);
                        else
                            item.setWidthChecked (isAssigned (item.item->width) ? getPreferredWidth (item)
                                                                                : lineSize - item.item->margin.left - item.item->margin.right);
                    }
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

                if (isRowDirection)
                {
                    item.lockedMarginLeft  += additionalMarginLeft;
                    item.lockedMarginRight += additionalMarginRight;
                    item.item->currentBounds.setPosition ((float) (x + item.lockedMarginLeft), (float) item.lockedMarginTop);
                    x += item.lockedWidth + item.lockedMarginLeft + item.lockedMarginRight;
                }
                else
                {
                    item.lockedMarginTop    += additionalMarginLeft;
                    item.lockedMarginBottom += additionalMarginRight;
                    item.item->currentBounds.setPosition ((float) item.lockedMarginLeft, (float) (x + item.lockedMarginTop));
                    x += item.lockedHeight + item.lockedMarginTop + item.lockedMarginBottom;
                }
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

    static int compareElements (const ItemWithState& i1, const ItemWithState& i2) noexcept
    {
        return i1.item->order < i2.item->order ? -1 : (i2.item->order < i1.item->order ? 1 : 0);
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
        item.lockedWidth  = getPreferredWidth (item);
        item.lockedHeight = getPreferredHeight (item);
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
                flexContainerLength -= getItemLength (item);
            }
            else
            {
                totalItemsLength += getItemLength (item);
                totalFlexGrow   += item.item->flexGrow;
                totalFlexShrink += item.item->flexShrink;
            }
        }

        Coord changeUnit = 0;
        const auto difference = flexContainerLength - totalItemsLength;
        const bool positiveFlexibility = difference > 0;

        if (positiveFlexibility)
        {
            if (totalFlexGrow != 0)
                changeUnit = difference / totalFlexGrow;
        }
        else
        {
            if (totalFlexShrink != 0)
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
            {
                const auto& item = getItem (column, row);

                lineInfo[row].totalLength += isRowDirection ? item.lockedWidth + item.lockedMarginLeft + item.lockedMarginRight
                                                            : item.lockedHeight + item.lockedMarginTop + item.lockedMarginBottom;
            }
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

    Coord getItemLength (const ItemWithState& item) const noexcept
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

        if (isRowDirection)
        {
            const auto prefWidth = getPreferredWidth (item);

            if (isAssigned (item.item->maxWidth) && item.item->maxWidth < prefWidth + length)
            {
                item.lockedWidth = item.item->maxWidth;
                item.locked = true;
            }
            else if (isAssigned (prefWidth) && item.item->minWidth > prefWidth + length)
            {
                item.lockedWidth = item.item->minWidth;
                item.locked = true;
            }
            else
            {
                ok = true;
                item.lockedWidth = prefWidth + length;
            }

            lineInfo[row].totalLength += item.lockedWidth + item.lockedMarginLeft + item.lockedMarginRight;
        }
        else
        {
            const auto prefHeight = getPreferredHeight (item);

            if (isAssigned (item.item->maxHeight) && item.item->maxHeight < prefHeight + length)
            {
                item.lockedHeight = item.item->maxHeight;
                item.locked = true;
            }
            else if (isAssigned (prefHeight) && item.item->minHeight > prefHeight + length)
            {
                item.lockedHeight = item.item->minHeight;
                item.locked = true;
            }
            else
            {
                ok = true;
                item.lockedHeight = prefHeight + length;
            }

            lineInfo[row].totalLength += item.lockedHeight + item.lockedMarginTop + item.lockedMarginBottom;
        }

        return ok;
    }

    Coord getPreferredWidth (const ItemWithState& itemWithState) const noexcept
    {
        const auto& item = *itemWithState.item;
        auto preferredWidth = (item.flexBasis > 0 && isRowDirection)
                                 ? item.flexBasis
                                 : (isAssigned (item.width) ? item.width : item.minWidth);

        if (isAssigned (item.minWidth) && preferredWidth < item.minWidth)  return item.minWidth;
        if (isAssigned (item.maxWidth) && preferredWidth > item.maxWidth)  return item.maxWidth;

        return preferredWidth;
    }

    Coord getPreferredHeight (const ItemWithState& itemWithState) const noexcept
    {
        const auto& item = *itemWithState.item;
        auto preferredHeight = (item.flexBasis > 0 && ! isRowDirection)
                                 ? item.flexBasis
                                 : (isAssigned (item.height) ? item.height : item.minHeight);

        if (isAssigned (item.minHeight) && preferredHeight < item.minHeight)  return item.minHeight;
        if (isAssigned (item.maxHeight) && preferredHeight > item.maxHeight)  return item.maxHeight;

        return preferredHeight;
    }
};

//==============================================================================
FlexBox::FlexBox() noexcept {}
FlexBox::~FlexBox() noexcept {}

FlexBox::FlexBox (JustifyContent jc) noexcept  : justifyContent (jc) {}

FlexBox::FlexBox (Direction d, Wrap w, AlignContent ac, AlignItems ai, JustifyContent jc) noexcept
    : flexDirection (d), flexWrap (w), alignContent (ac), alignItems (ai), justifyContent (jc)
{}

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
        layout.alignItemsInCrossAxisInLinesPerAlignItems();
        layout.alignLinesPerAlignSelf();
        layout.alignItemsByJustifyContent();
        layout.layoutAllItems();

        for (auto& item : items)
        {
            item.currentBounds += targetArea.getPosition();

            if (auto comp = item.associatedComponent)
                comp->setBounds (item.currentBounds.getSmallestIntegerContainer());

            if (auto box = item.associatedFlexBox)
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

FlexItem FlexItem::withMinHeight (float newMinHeight) const noexcept { auto fi = *this; fi.minHeight = newMinHeight; return fi; };
FlexItem FlexItem::withMaxHeight (float newMaxHeight) const noexcept { auto fi = *this; fi.maxHeight = newMaxHeight; return fi; };
FlexItem FlexItem::withHeight (float newHeight) const noexcept       { auto fi = *this; fi.height = newHeight; return fi; }

FlexItem FlexItem::withMargin (Margin m) const noexcept              { auto fi = *this; fi.margin = m; return fi; }
FlexItem FlexItem::withOrder (int newOrder) const noexcept           { auto fi = *this; fi.order = newOrder; return fi; }
FlexItem FlexItem::withAlignSelf (AlignSelf a) const noexcept        { auto fi = *this; fi.alignSelf = a; return fi; }
