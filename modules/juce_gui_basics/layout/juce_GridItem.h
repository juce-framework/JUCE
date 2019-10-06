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

/**
    Defines an item in a Grid
    @see Grid

    @tags{GUI}
*/
class JUCE_API  GridItem
{
public:
    enum class Keyword { autoValue };

    //==============================================================================
    /** Represents a span. */
    struct Span
    {
        explicit Span (int numberToUse) noexcept : number (numberToUse)
        {
            /* Span must be at least one and positive */
            jassert (numberToUse > 0);
        }

        explicit Span (int numberToUse, const String& nameToUse) : Span (numberToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
            name = nameToUse;
        }

        explicit Span (const String& nameToUse) : name (nameToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
        }

        int number = 1;
        String name;
    };

    //==============================================================================
    /** Represents a property. */
    struct Property
    {
        Property() noexcept;

        Property (Keyword keyword) noexcept;

        Property (const char* lineNameToUse) noexcept;

        Property (const String& lineNameToUse) noexcept;

        Property (int numberToUse) noexcept;

        Property (int numberToUse, const String& lineNameToUse) noexcept;

        Property (Span spanToUse) noexcept;

        bool hasSpan() const noexcept          { return isSpan && ! isAuto; }
        bool hasAbsolute() const noexcept      { return ! (isSpan || isAuto);  }
        bool hasAuto() const noexcept          { return isAuto; }
        bool hasName() const noexcept          { return name.isNotEmpty(); }
        const String& getName() const noexcept { return name; }
        int getNumber() const noexcept         { return number; }

    private:
        String name;
        int number = 1; /** Either an absolute line number or number of lines to span across. */
        bool isSpan = false;
        bool isAuto = false;
    };

    //==============================================================================
    /** Represents start and end properties. */
    struct StartAndEndProperty { Property start, end; };

    //==============================================================================
    /** Possible values for the justifySelf property. */
    enum class JustifySelf : int
    {
        start = 0,               /**< Content inside the item is justified towards the left. */
        end,                     /**< Content inside the item is justified towards the right. */
        center,                  /**< Content inside the item is justified towards the center. */
        stretch,                 /**< Content inside the item is stretched from left to right. */
        autoValue                /**< Follows the Grid container's justifyItems property. */
    };

    /** Possible values for the alignSelf property. */
    enum class AlignSelf   : int
    {
        start = 0,               /**< Content inside the item is aligned towards the top. */
        end,                     /**< Content inside the item is aligned towards the bottom. */
        center,                  /**< Content inside the item is aligned towards the center. */
        stretch,                 /**< Content inside the item is stretched from top to bottom. */
        autoValue                /**< Follows the Grid container's alignItems property. */
    };

    /** Creates an item with default parameters. */
    GridItem() noexcept;
    /** Creates an item with a given Component to use. */
    GridItem (Component& componentToUse) noexcept;
    /** Creates an item with a given Component to use. */
    GridItem (Component* componentToUse) noexcept;

    /** Destructor. */
    ~GridItem() noexcept;

    //==============================================================================
    /** If this is non-null, it represents a Component whose bounds are controlled by this item. */
    Component* associatedComponent = nullptr;

    //==============================================================================
    /** Determines the order used to lay out items in their grid container. */
    int order = 0;

    /** This is the justify-self property of the item.
        This determines the alignment of the item along the row.
    */
    JustifySelf  justifySelf = JustifySelf::autoValue;

    /** This is the align-self property of the item.
        This determines the alignment of the item along the column.
    */
    AlignSelf    alignSelf   = AlignSelf::autoValue;

    /** These are the start and end properties of the column. */
    StartAndEndProperty column = { Keyword::autoValue, Keyword::autoValue };

    /** These are the start and end properties of the row. */
    StartAndEndProperty row    = { Keyword::autoValue, Keyword::autoValue };

    /** */
    String area;

    //==============================================================================
    enum
    {
        useDefaultValue = -2, /* TODO: useDefaultValue should be named useAuto */
        notAssigned = -1
    };

    /* TODO: move all of this into a common class that is shared with the FlexItem */
    float width    = notAssigned;
    float minWidth = 0.0f;
    float maxWidth = notAssigned;

    float height    = notAssigned;
    float minHeight = 0.0f;
    float maxHeight = notAssigned;

    /** Represents a margin. */
    struct Margin
    {
        Margin() noexcept;
        Margin (int size) noexcept;
        Margin (float size) noexcept;
        Margin (float top, float right, float bottom, float left) noexcept;   /**< Creates a margin with these sizes. */

        float left;
        float right;
        float top;
        float bottom;
    };

    /** The margin to leave around this item. */
    Margin margin;

    /** The item's current bounds. */
    Rectangle<float> currentBounds;

    /** Short-hand */
    void setArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd);

    /** Short-hand, span of 1 by default */
    void setArea (Property rowStart, Property columnStart);

    /** Short-hand */
    void setArea (const String& areaName);

    /** Short-hand */
    GridItem withArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd) const noexcept;

    /** Short-hand, span of 1 by default */
    GridItem withArea (Property rowStart, Property columnStart) const noexcept;

    /** Short-hand */
    GridItem withArea (const String& areaName)  const noexcept;

    /** Returns a copy of this object with a new row property. */
    GridItem withRow (StartAndEndProperty row) const noexcept;

    /** Returns a copy of this object with a new column property. */
    GridItem withColumn (StartAndEndProperty column) const noexcept;

    /** Returns a copy of this object with a new alignSelf property. */
    GridItem withAlignSelf (AlignSelf newAlignSelf) const noexcept;

    /** Returns a copy of this object with a new justifySelf property. */
    GridItem withJustifySelf (JustifySelf newJustifySelf) const noexcept;

    /** Returns a copy of this object with a new width. */
    GridItem withWidth (float newWidth) const noexcept;

    /** Returns a copy of this object with a new height. */
    GridItem withHeight (float newHeight) const noexcept;

    /** Returns a copy of this object with a new size. */
    GridItem withSize (float newWidth, float newHeight) const noexcept;

    /** Returns a copy of this object with a new margin. */
    GridItem withMargin (Margin newMargin) const noexcept;

    /** Returns a copy of this object with a new order. */
    GridItem withOrder (int newOrder) const noexcept;
};

} // namespace juce
