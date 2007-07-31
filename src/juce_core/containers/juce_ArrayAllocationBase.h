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

#ifndef __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
#define __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__


//==============================================================================
/** The default size of chunk in which arrays increase their storage.

    Used by ArrayAllocationBase and its subclasses.
*/
const int juceDefaultArrayGranularity = 8;


//==============================================================================
/**
    Implements some basic array storage allocation functions.

    This class isn't really for public use - it's used by the other
    array classes, but might come in handy for some purposes.

    @see Array, OwnedArray, ReferenceCountedArray
*/
template <class ElementType>
class ArrayAllocationBase
{
protected:
    //==============================================================================
    /** Creates an empty array.

        @param granularity_  this is the size of increment by which the internal storage
        will be increased.
    */
    ArrayAllocationBase (const int granularity_) throw()
        : elements (0),
          numAllocated (0),
          granularity (granularity_)
    {
    }

    /** Destructor. */
    ~ArrayAllocationBase() throw()
    {
        if (elements != 0)
            juce_free (elements);
    }

    //==============================================================================
    /** Changes the amount of storage allocated.

        This will retain any data currently held in the array, and either add or
        remove extra space at the end.

        @param numElements  the number of elements that are needed
    */
    void setAllocatedSize (const int numElements) throw()
    {
        if (numAllocated != numElements)
        {
            numAllocated = numElements;

            if (numElements > 0)
            {
                if (elements == 0)
                    elements = (ElementType*) juce_malloc (sizeof (ElementType) * numElements);
                else
                    elements = (ElementType*) juce_realloc (elements, sizeof (ElementType) * numElements);
            }
            else
            {
                if (elements != 0)
                {
                    juce_free (elements);
                    elements = 0;
                }
            }
        }
    }

    /** Increases the amount of storage allocated if it is less than a given amount.

        This will retain any data currently held in the array, but will add
        extra space at the end to make sure there it's at least as big as the size
        passed in. If it's already bigger, no action is taken.

        @param minNumElements  the minimum number of elements that are needed
    */
    void ensureAllocatedSize (int minNumElements) throw()
    {
        if (minNumElements > numAllocated)
        {
            // for arrays with small granularity that get big, start
            // increasing the size in bigger jumps
            if (minNumElements > (granularity << 6))
            {
                minNumElements += (minNumElements / granularity);
                if (minNumElements > (granularity << 8))
                    minNumElements += granularity << 6;
                else
                    minNumElements += granularity << 5;
            }

            setAllocatedSize (granularity * (minNumElements / granularity + 1));
        }
    }

    //==============================================================================
    ElementType* elements;
    int numAllocated, granularity;

private:
    ArrayAllocationBase (const ArrayAllocationBase&);
    const ArrayAllocationBase& operator= (const ArrayAllocationBase&);
};


#endif   // __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
