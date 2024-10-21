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

//==============================================================================
/**
    A utility class for fitting a set of objects whose sizes can vary between
    a minimum and maximum size, into a space.

    This is a trickier algorithm than it would first seem, so I've put it in this
    class to allow it to be shared by various bits of code.

    To use it, create one of these objects, call addItem() to add the list of items
    you need, then call resizeToFit(), which will change all their sizes. You can
    then retrieve the new sizes with getItemSize() and getNumItems().

    It's currently used by the TableHeaderComponent for stretching out the table
    headings to fill the table's width.

    @tags{GUI}
*/
class StretchableObjectResizer
{
public:
    //==============================================================================
    /** Creates an empty object resizer. */
    StretchableObjectResizer();

    /** Destructor. */
    ~StretchableObjectResizer();

    //==============================================================================
    /** Adds an item to the list.

        The order parameter lets you specify groups of items that are resized first when some
        space needs to be found. Those items with an order of 0 will be the first ones to be
        resized, and if that doesn't provide enough space to meet the requirements, the algorithm
        will then try resizing the items with an order of 1, then 2, and so on.
    */
    void addItem (double currentSize,
                  double minSize,
                  double maxSize,
                  int order = 0);

    /** Resizes all the items to fit this amount of space.

        This will attempt to fit them in without exceeding each item's minimum and
        maximum sizes. In cases where none of the items can be expanded or enlarged any
        further, the final size may be greater or less than the size passed in.

        After calling this method, you can retrieve the new sizes with the getItemSize()
        method.
    */
    void resizeToFit (double targetSize);

    /** Returns the number of items that have been added. */
    int getNumItems() const noexcept                        { return items.size(); }

    /** Returns the size of one of the items. */
    double getItemSize (int index) const noexcept;


private:
    //==============================================================================
    struct Item
    {
        double size;
        double minSize;
        double maxSize;
        int order;
    };

    Array<Item> items;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StretchableObjectResizer)
};

} // namespace juce
