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
