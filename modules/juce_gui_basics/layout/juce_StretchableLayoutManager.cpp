/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

StretchableLayoutManager::StretchableLayoutManager()
    : totalSize (0)
{
}

StretchableLayoutManager::~StretchableLayoutManager()
{
}

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
    ItemLayoutProperties* layout = getInfoFor (itemIndex);

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
    if (const ItemLayoutProperties* const layout = getInfoFor (itemIndex))
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
        if (const ItemLayoutProperties* const layout = getInfoFor (i))
            pos += layout->currentSize;

    return pos;
}

int StretchableLayoutManager::getItemCurrentAbsoluteSize (const int itemIndex) const
{
    if (const ItemLayoutProperties* const layout = getInfoFor (itemIndex))
        return layout->currentSize;

    return 0;
}

double StretchableLayoutManager::getItemCurrentRelativeSize (const int itemIndex) const
{
    if (const ItemLayoutProperties* const layout = getInfoFor (itemIndex))
        return -layout->currentSize / (double) totalSize;

    return 0;
}

void StretchableLayoutManager::setItemPosition (const int itemIndex,
                                                int newPosition)
{
    for (int i = items.size(); --i >= 0;)
    {
        const ItemLayoutProperties* const layout = items.getUnchecked(i);

        if (layout->itemIndex == itemIndex)
        {
            int realTotalSize = jmax (totalSize, getMinimumSizeOfItems (0, items.size()));
            const int minSizeAfterThisComp = getMinimumSizeOfItems (i, items.size());
            const int maxSizeAfterThisComp = getMaximumSizeOfItems (i + 1, items.size());

            newPosition = jmax (newPosition, totalSize - maxSizeAfterThisComp - layout->currentSize);
            newPosition = jmin (newPosition, realTotalSize - minSizeAfterThisComp);

            int endPos = fitComponentsIntoSpace (0, i, newPosition, 0);

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
        if (const ItemLayoutProperties* const layout = getInfoFor (i))
        {
            if (Component* const c = components[i])
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
    for (int i = items.size(); --i >= 0;)
        if (items.getUnchecked(i)->itemIndex == itemIndex)
            return items.getUnchecked(i);

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
        ItemLayoutProperties* const layout = items.getUnchecked (i);

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
            ItemLayoutProperties* const layout = items.getUnchecked (i);

            double sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            const int bestSize = jlimit (layout->currentSize,
                                         jmax (layout->currentSize,
                                               sizeToRealSize (layout->maxSize, totalSize)),
                                         roundToInt (sizeWanted * availableSpace / totalIdealSize));

            if (bestSize > layout->currentSize)
                ++numWantingMoreSpace;
        }

        // ..share out the extra space..
        for (int i = startIndex; i < endIndex; ++i)
        {
            ItemLayoutProperties* const layout = items.getUnchecked (i);

            double sizeWanted = sizeToRealSize (layout->preferredSize, totalSize);

            int bestSize = jlimit (layout->currentSize,
                                   jmax (layout->currentSize, sizeToRealSize (layout->maxSize, totalSize)),
                                   roundToInt (sizeWanted * availableSpace / totalIdealSize));

            const int extraWanted = bestSize - layout->currentSize;

            if (extraWanted > 0)
            {
                const int extraAllowed = jmin (extraWanted,
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
        ItemLayoutProperties* const layout = items.getUnchecked(i);
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
        ItemLayoutProperties* const layout = items.getUnchecked (i);

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
