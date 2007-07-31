/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_StretchableLayoutManager.h"


//==============================================================================
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

    if (layout == 0)
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
    const ItemLayoutProperties* const layout = getInfoFor (itemIndex);

    if (layout != 0)
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
    {
        const ItemLayoutProperties* const layout = getInfoFor (i);

        if (layout != 0)
            pos += layout->currentSize;
    }

    return pos;
}

int StretchableLayoutManager::getItemCurrentAbsoluteSize (const int itemIndex) const
{
    const ItemLayoutProperties* const layout = getInfoFor (itemIndex);

    if (layout != 0)
        return layout->currentSize;

    return 0;
}

double StretchableLayoutManager::getItemCurrentRelativeSize (const int itemIndex) const
{
    const ItemLayoutProperties* const layout = getInfoFor (itemIndex);

    if (layout != 0)
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
        const ItemLayoutProperties* const layout = getInfoFor (i);

        if (layout != 0)
        {
            Component* const c = components[i];

            if (c != 0)
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

    return 0;
}

int StretchableLayoutManager::fitComponentsIntoSpace (const int startIndex,
                                                      const int endIndex,
                                                      const int availableSpace,
                                                      int startPos)
{
    // calculate the total sizes
    int i;
    double totalIdealSize = 0.0;
    int totalMinimums = 0;

    for (i = startIndex; i < endIndex; ++i)
    {
        ItemLayoutProperties* const layout = items.getUnchecked (i);

        layout->currentSize = sizeToRealSize (layout->minSize, totalSize);

        totalMinimums += layout->currentSize;
        totalIdealSize += sizeToRealSize (layout->preferredSize, availableSpace);
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
        for (i = startIndex; i < endIndex; ++i)
        {
            ItemLayoutProperties* const layout = items.getUnchecked (i);

            double sizeWanted = sizeToRealSize (layout->preferredSize, availableSpace);

            const int bestSize = jlimit (layout->currentSize,
                                         jmax (layout->currentSize,
                                               sizeToRealSize (layout->maxSize, totalSize)),
                                         roundDoubleToInt (sizeWanted * availableSpace / totalIdealSize));

            if (bestSize > layout->currentSize)
                ++numWantingMoreSpace;
        }

        // ..share out the extra space..
        for (i = startIndex; i < endIndex; ++i)
        {
            ItemLayoutProperties* const layout = items.getUnchecked (i);

            double sizeWanted = sizeToRealSize (layout->preferredSize, availableSpace);

            int bestSize = jlimit (layout->currentSize,
                                   jmax (layout->currentSize, sizeToRealSize (layout->maxSize, totalSize)),
                                   roundDoubleToInt (sizeWanted * availableSpace / totalIdealSize));

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
    for (i = startIndex; i < endIndex; ++i)
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

    return roundDoubleToInt (size);
}

END_JUCE_NAMESPACE
