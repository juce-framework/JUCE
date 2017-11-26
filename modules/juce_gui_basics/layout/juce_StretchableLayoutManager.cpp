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

StretchableLayoutManager::StretchableLayoutManager() {}
StretchableLayoutManager::~StretchableLayoutManager() {}

//==============================================================================
void StretchableLayoutManager::clearAllItems()
{
    items.clear();
    totalSize = 0;
}

void StretchableLayoutManager::setItemLayout (const int itemIndex,
                                              const double minimumSize,
                                              const double maximumSize,
                                              const double preferredSize)
{
    auto* layout = getInfoFor (itemIndex);

    if (layout == nullptr)
    {
        layout = new ItemLayoutProperties();
        layout->itemIndex = itemIndex;

        int i;
        for (i = 0; i < items.size(); ++i)
            if (items.getUnchecked (i)->itemIndex > itemIndex)
                break;

        items.insert (i, layout);
    }

    layout->minSize = minimumSize;
    layout->maxSize = maximumSize;
    layout->preferredSize = preferredSize;
    layout->currentSize = 0;
}

bool StretchableLayoutManager::getItemLayout (const int itemIndex,
                                              double& minimumSize,
                                              double& maximumSize,
                                              double& preferredSize) const
{
    if (auto* layout = getInfoFor (itemIndex))
    {
        minimumSize = layout->minSize;
        maximumSize = layout->maxSize;
        preferredSize = layout->preferredSize;
        return true;
    }

    return false;
}

//==============================================================================
void StretchableLayoutManager::setTotalSize (const int newTotalSize)
{
    totalSize = newTotalSize;

    fitComponentsIntoSpace (0, items.size(), totalSize, 0);
}

int StretchableLayoutManager::getItemCurrentPosition (const int itemIndex) const
{
    int pos = 0;

    for (int i = 0; i < itemIndex; ++i)
        if (auto* layout = getInfoFor (i))
            pos += layout->currentSize;

    return pos;
}

int StretchableLayoutManager::getItemCurrentAbsoluteSize (const int itemIndex) const
{
    if (auto* layout = getInfoFor (itemIndex))
        return layout->currentSize;

    return 0;
}

double StretchableLayoutManager::getItemCurrentRelativeSize (const int itemIndex) const
{
    if (auto* layout = getInfoFor (itemIndex))
        return -layout->currentSize / (double) totalSize;

    return 0;
}

void StretchableLayoutManager::setItemPosition (const int itemIndex,
                                                int newPosition)
{
    for (int i = items.size(); --i >= 0;)
    {
        auto* layout = items.getUnchecked(i);

        if (layout->itemIndex == itemIndex)
        {
            auto realTotalSize = jmax (totalSize, getMinimumSizeOfItems (0, items.size()));
            auto minSizeAfterThisComp = getMinimumSizeOfItems (i, items.size());
            auto maxSizeAfterThisComp = getMaximumSizeOfItems (i + 1, items.size());

            newPosition = jmax (newPosition, totalSize - maxSizeAfterThisComp - layout->currentSize);
            newPosition = jmin (newPosition, realTotalSize - minSizeAfterThisComp);

            auto endPos = fitComponentsIntoSpace (0, i, newPosition, 0);

            endPos += layout->currentSize;

            fitComponentsIntoSpace (i + 1, items.size(), totalSize - endPos, endPos);
            updatePrefSizesToMatchCurrentPositions();
            break;
        }
    }
}

//==============================================================================
void StretchableLayoutManager::layOutComponents (Component** const components,
                                                 int numComponents,
                                                 int x, int y, int w, int h,
                                                 const bool vertically,
                                                 const bool resizeOtherDimension)
{
    setTotalSize (vertically ? h : w);
    int pos = vertically ? y : x;

    for (int i = 0; i < numComponents; ++i)
    {
        if (auto* layout = getInfoFor (i))
        {
            if (auto* c = components[i])
            {
                if (i == numComponents - 1)
                {
                    // if it's the last item, crop it to exactly fit the available space..
                    if (resizeOtherDimension)
                    {
                        if (vertically)
                            c->setBounds (x, pos, w, jmax (layout->currentSize, h - pos));
                        else
                            c->setBounds (pos, y, jmax (layout->currentSize, w - pos), h);
                    }
                    else
                    {
                        if (vertically)
                            c->setBounds (c->getX(), pos, c->getWidth(), jmax (layout->currentSize, h - pos));
                        else
                            c->setBounds (pos, c->getY(), jmax (layout->currentSize, w - pos), c->getHeight());
                    }
                }
                else
                {
                    if (resizeOtherDimension)
                    {
                        if (vertically)
                            c->setBounds (x, pos, w, layout->currentSize);
                        else
                            c->setBounds (pos, y, layout->currentSize, h);
                    }
                    else
                    {
                        if (vertically)
                            c->setBounds (c->getX(), pos, c->getWidth(), layout->currentSize);
                        else
                            c->setBounds (pos, c->getY(), layout->currentSize, c->getHeight());
                    }
                }
            }

            pos += layout->currentSize;
        }
    }
}


//==============================================================================
StretchableLayoutManager::ItemLayoutProperties* StretchableLayoutManager::getInfoFor (const int itemIndex) const
{
    for (auto* i : items)
        if (i->itemIndex == itemIndex)
            return i;

    return nullptr;
}

int StretchableLayoutManager::fitComponentsIntoSpace (const int startIndex,
                                                      const int endIndex,
                                                      const int availableSpace,
                                                      int startPos)
{
    // calculate the total sizes
    double totalIdealSize = 0.0;
    int totalMinimums = 0;

    for (int i = startIndex; i < endIndex; ++i)
    {
        auto* layout = items.getUnchecked (i);

        layout->currentSize = sizeToRealSize (layout->minSize, totalSize);

        totalMinimums += layout->currentSize;
        totalIdealSize += sizeToRealSize (layout->preferredSize, totalSize);
   }

    if (totalIdealSize <= 0)
        totalIdealSize = 1.0;

    // now calc the best sizes..
    int extraSpace = availableSpace - totalMinimums;

    while (extraSpace > 0)
    {
        int numWantingMoreSpace = 0;
        int numHavingTakenExtraSpace = 0;

        // first figure out how many comps want a slice of the extra space..
        for (int i = startIndex; i < endIndex; ++i)
        {
            auto* layout = items.getUnchecked (i);

            auto sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            auto bestSize = jlimit (layout->currentSize,
                                    jmax (layout->currentSize,
                                          sizeToRealSize (layout->maxSize, totalSize)),
                                    roundToInt (sizeWanted * availableSpace / totalIdealSize));

            if (bestSize > layout->currentSize)
                ++numWantingMoreSpace;
        }

        // ..share out the extra space..
        for (int i = startIndex; i < endIndex; ++i)
        {
            auto* layout = items.getUnchecked (i);

            auto sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            auto bestSize = jlimit (layout->currentSize,
                                    jmax (layout->currentSize, sizeToRealSize (layout->maxSize, totalSize)),
                                    roundToInt (sizeWanted * availableSpace / totalIdealSize));

            auto extraWanted = bestSize - layout->currentSize;

            if (extraWanted > 0)
            {
                auto extraAllowed = jmin (extraWanted,
                                          extraSpace / jmax (1, numWantingMoreSpace));

                if (extraAllowed > 0)
                {
                    ++numHavingTakenExtraSpace;
                    --numWantingMoreSpace;

                    layout->currentSize += extraAllowed;
                    extraSpace -= extraAllowed;
                }
            }
        }

        if (numHavingTakenExtraSpace <= 0)
            break;
    }

    // ..and calculate the end position
    for (int i = startIndex; i < endIndex; ++i)
    {
        auto* layout = items.getUnchecked(i);
        startPos += layout->currentSize;
    }

    return startPos;
}

int StretchableLayoutManager::getMinimumSizeOfItems (const int startIndex,
                                                     const int endIndex) const
{
    int totalMinimums = 0;

    for (int i = startIndex; i < endIndex; ++i)
        totalMinimums += sizeToRealSize (items.getUnchecked (i)->minSize, totalSize);

    return totalMinimums;
}

int StretchableLayoutManager::getMaximumSizeOfItems (const int startIndex, const int endIndex) const
{
    int totalMaximums = 0;

    for (int i = startIndex; i < endIndex; ++i)
        totalMaximums += sizeToRealSize (items.getUnchecked (i)->maxSize, totalSize);

    return totalMaximums;
}

void StretchableLayoutManager::updatePrefSizesToMatchCurrentPositions()
{
    for (int i = 0; i < items.size(); ++i)
    {
        auto* layout = items.getUnchecked (i);

        layout->preferredSize
            = (layout->preferredSize < 0) ? getItemCurrentRelativeSize (i)
                                          : getItemCurrentAbsoluteSize (i);
    }
}

int StretchableLayoutManager::sizeToRealSize (double size, int totalSpace)
{
    if (size < 0)
        size *= -totalSpace;

    return roundToInt (size);
}

} // namespace juce
