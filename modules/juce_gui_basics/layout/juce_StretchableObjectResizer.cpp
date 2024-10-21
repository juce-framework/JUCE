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

StretchableObjectResizer::StretchableObjectResizer() {}
StretchableObjectResizer::~StretchableObjectResizer() {}

void StretchableObjectResizer::addItem (const double size,
                                        const double minSize, const double maxSize,
                                        const int order)
{
    // the order must be >= 0 but less than the maximum integer value.
    jassert (order >= 0 && order < std::numeric_limits<int>::max());
    jassert (maxSize >= minSize);

    Item item;
    item.size = size;
    item.minSize = minSize;
    item.maxSize = maxSize;
    item.order = order;
    items.add (item);
}

double StretchableObjectResizer::getItemSize (const int index) const noexcept
{
    return isPositiveAndBelow (index, items.size()) ? items.getReference (index).size
                                                    : 0.0;
}

void StretchableObjectResizer::resizeToFit (const double targetSize)
{
    int order = 0;

    for (;;)
    {
        double currentSize = 0;
        double minSize = 0;
        double maxSize = 0;

        int nextHighestOrder = std::numeric_limits<int>::max();

        for (int i = 0; i < items.size(); ++i)
        {
            const Item& it = items.getReference (i);
            currentSize += it.size;

            if (it.order <= order)
            {
                minSize += it.minSize;
                maxSize += it.maxSize;
            }
            else
            {
                minSize += it.size;
                maxSize += it.size;
                nextHighestOrder = jmin (nextHighestOrder, it.order);
            }
        }

        const double thisIterationTarget = jlimit (minSize, maxSize, targetSize);

        if (thisIterationTarget >= currentSize)
        {
            const double availableExtraSpace = maxSize - currentSize;
            const double targetAmountOfExtraSpace = thisIterationTarget - currentSize;
            const double scale = availableExtraSpace > 0 ? targetAmountOfExtraSpace / availableExtraSpace : 1.0;

            for (int i = 0; i < items.size(); ++i)
            {
                Item& it = items.getReference (i);

                if (it.order <= order)
                    it.size = jlimit (it.minSize, it.maxSize, it.size + (it.maxSize - it.size) * scale);
            }
        }
        else
        {
            const double amountOfSlack = currentSize - minSize;
            const double targetAmountOfSlack = thisIterationTarget - minSize;
            const double scale = targetAmountOfSlack / amountOfSlack;

            for (int i = 0; i < items.size(); ++i)
            {
                Item& it = items.getReference (i);

                if (it.order <= order)
                    it.size = jmax (it.minSize, it.minSize + (it.size - it.minSize) * scale);
            }
        }

        if (nextHighestOrder < std::numeric_limits<int>::max())
            order = nextHighestOrder;
        else
            break;
    }
}

} // namespace juce
