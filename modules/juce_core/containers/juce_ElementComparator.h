/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_ELEMENTCOMPARATOR_H_INCLUDED
#define JUCE_ELEMENTCOMPARATOR_H_INCLUDED


//==============================================================================
/**
    Sorts a range of elements in an array.

    The comparator object that is passed-in must define a public method with the following
    signature:
    @code
    int compareElements (ElementType first, ElementType second);
    @endcode

    ..and this method must return:
      - a value of < 0 if the first comes before the second
      - a value of 0 if the two objects are equivalent
      - a value of > 0 if the second comes before the first

    To improve performance, the compareElements() method can be declared as static or const.

    @param comparator       an object which defines a compareElements() method
    @param array            the array to sort
    @param firstElement     the index of the first element of the range to be sorted
    @param lastElement      the index of the last element in the range that needs
                            sorting (this is inclusive)
    @param retainOrderOfEquivalentItems     if true, the order of items that the
                            comparator deems the same will be maintained - this will be
                            a slower algorithm than if they are allowed to be moved around.

    @see sortArrayRetainingOrder
*/
template <class ElementType, class ElementComparator>
static void sortArray (ElementComparator& comparator,
                       ElementType* const array,
                       int firstElement,
                       int lastElement,
                       const bool retainOrderOfEquivalentItems)
{
    (void) comparator;  // if you pass in an object with a static compareElements() method, this
                        // avoids getting warning messages about the parameter being unused

    if (lastElement > firstElement)
    {
        if (retainOrderOfEquivalentItems)
        {
            for (int i = firstElement; i < lastElement; ++i)
            {
                if (comparator.compareElements (array[i], array [i + 1]) > 0)
                {
                    std::swap (array[i], array[i + 1]);

                    if (i > firstElement)
                        i -= 2;
                }
            }
        }
        else
        {
            int fromStack[30], toStack[30];
            int stackIndex = 0;

            for (;;)
            {
                const int size = (lastElement - firstElement) + 1;

                if (size <= 8)
                {
                    int j = lastElement;
                    int maxIndex;

                    while (j > firstElement)
                    {
                        maxIndex = firstElement;
                        for (int k = firstElement + 1; k <= j; ++k)
                            if (comparator.compareElements (array[k], array [maxIndex]) > 0)
                                maxIndex = k;

                        std::swap (array[j], array[maxIndex]);
                        --j;
                    }
                }
                else
                {
                    const int mid = firstElement + (size >> 1);
                    std::swap (array[mid], array[firstElement]);

                    int i = firstElement;
                    int j = lastElement + 1;

                    for (;;)
                    {
                        while (++i <= lastElement
                                && comparator.compareElements (array[i], array [firstElement]) <= 0)
                        {}

                        while (--j > firstElement
                                && comparator.compareElements (array[j], array [firstElement]) >= 0)
                        {}

                        if (j < i)
                            break;

                        std::swap (array[i], array[j]);
                    }

                    std::swap (array[j], array[firstElement]);

                    if (j - 1 - firstElement >= lastElement - i)
                    {
                        if (firstElement + 1 < j)
                        {
                            fromStack [stackIndex] = firstElement;
                            toStack [stackIndex] = j - 1;
                            ++stackIndex;
                        }

                        if (i < lastElement)
                        {
                            firstElement = i;
                            continue;
                        }
                    }
                    else
                    {
                        if (i < lastElement)
                        {
                            fromStack [stackIndex] = i;
                            toStack [stackIndex] = lastElement;
                            ++stackIndex;
                        }

                        if (firstElement + 1 < j)
                        {
                            lastElement = j - 1;
                            continue;
                        }
                    }
                }

                if (--stackIndex < 0)
                    break;

                jassert (stackIndex < numElementsInArray (fromStack));

                firstElement = fromStack [stackIndex];
                lastElement = toStack [stackIndex];
            }
        }
    }
}


//==============================================================================
/**
    Searches a sorted array of elements, looking for the index at which a specified value
    should be inserted for it to be in the correct order.

    The comparator object that is passed-in must define a public method with the following
    signature:
    @code
    int compareElements (ElementType first, ElementType second);
    @endcode

    ..and this method must return:
      - a value of < 0 if the first comes before the second
      - a value of 0 if the two objects are equivalent
      - a value of > 0 if the second comes before the first

    To improve performance, the compareElements() method can be declared as static or const.

    @param comparator       an object which defines a compareElements() method
    @param array            the array to search
    @param newElement       the value that is going to be inserted
    @param firstElement     the index of the first element to search
    @param lastElement      the index of the last element in the range (this is non-inclusive)
*/
template <class ElementType, class ElementComparator>
static int findInsertIndexInSortedArray (ElementComparator& comparator,
                                         ElementType* const array,
                                         const ElementType newElement,
                                         int firstElement,
                                         int lastElement)
{
    jassert (firstElement <= lastElement);

    (void) comparator;  // if you pass in an object with a static compareElements() method, this
                        // avoids getting warning messages about the parameter being unused

    while (firstElement < lastElement)
    {
        if (comparator.compareElements (newElement, array [firstElement]) == 0)
        {
            ++firstElement;
            break;
        }
        else
        {
            const int halfway = (firstElement + lastElement) >> 1;

            if (halfway == firstElement)
            {
                if (comparator.compareElements (newElement, array [halfway]) >= 0)
                    ++firstElement;

                break;
            }
            else if (comparator.compareElements (newElement, array [halfway]) >= 0)
            {
                firstElement = halfway;
            }
            else
            {
                lastElement = halfway;
            }
        }
    }

    return firstElement;
}

//==============================================================================
/**
    A simple ElementComparator class that can be used to sort an array of
    objects that support the '<' operator.

    This will work for primitive types and objects that implement operator<().

    Example: @code
    Array <int> myArray;
    DefaultElementComparator<int> sorter;
    myArray.sort (sorter);
    @endcode

    @see ElementComparator
*/
template <class ElementType>
class DefaultElementComparator
{
private:
    typedef PARAMETER_TYPE (ElementType) ParameterType;

public:
    static int compareElements (ParameterType first, ParameterType second)
    {
        return (first < second) ? -1 : ((second < first) ? 1 : 0);
    }
};


#endif   // JUCE_ELEMENTCOMPARATOR_H_INCLUDED
