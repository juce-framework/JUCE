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
*/
class JUCE_API  GridItem
{
public:
    enum class Keyword { autoValue };

    //==============================================================================
    /** */
    struct Span
    {
        explicit Span (int numberToUse) noexcept : number (numberToUse)
        {
            /* Span must be at least one and positive */
            jassert (numberToUse > 0);
        }

        explicit Span (int numberToUse, const juce::String& nameToUse) : Span (numberToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
            name = nameToUse;
        }

        explicit Span (const juce::String& nameToUse) : name (nameToUse)
        {
            /* Name must not be empty */
            jassert (nameToUse.isNotEmpty());
        }

        int number = 1;
        juce::String name;
    };

    //==============================================================================
    /** */
    struct Property
    {
        /** */
        Property() noexcept;

        /** */
        Property (Keyword keyword) noexcept;

        /** */
        Property (const char* lineNameToUse) noexcept;

        /** */
        Property (const juce::String& lineNameToUse) noexcept;

        /** */
        Property (int numberToUse) noexcept;

        /** */
        Property (int numberToUse, const juce::String& lineNameToUse) noexcept;

        /** */
        Property (Span spanToUse) noexcept;

    private:
        bool hasSpan() const noexcept       { return isSpan && ! isAuto; }
        bool hasAbsolute() const noexcept   { return ! (isSpan || isAuto);  }
        bool hasAuto() const noexcept       { return isAuto; }
        bool hasName() const noexcept       { return name.isNotEmpty(); }

        friend class Grid;

        juce::String name;
        int number = 1; /** Either an absolute line number or number of lines to span across. */
        bool isSpan = false;
        bool isAuto = false;
    };

    //==============================================================================
    /** */
    struct StartAndEndProperty { Property start, end; };

    //==============================================================================
    /** */
    enum class JustifySelf : int { start = 0, end, center, stretch, autoValue };
    /** */
    enum class AlignSelf   : int { start = 0, end, center, stretch, autoValue };

    /** */
    GridItem() noexcept;
    /** */
    GridItem (juce::Component& componentToUse) noexcept;
    /** */
    GridItem (juce::Component* componentToUse) noexcept;

    /** Destructor. */
    ~GridItem() noexcept;

    //==============================================================================
    /** */
    juce::Component* associatedComponent = nullptr;

    //==============================================================================
    /** */
    int order = 0;

    /** */
    JustifySelf  justifySelf = JustifySelf::autoValue;
    /** */
    AlignSelf    alignSelf   = AlignSelf::autoValue;

    /** */
    StartAndEndProperty column = { Keyword::autoValue, Keyword::autoValue };
    /** */
    StartAndEndProperty row    = { Keyword::autoValue, Keyword::autoValue };

    /** */
    juce::String area;

    //==============================================================================
    enum
    {
        useDefaultValue = -2, /* TODO: useDefaultValue should be named useAuto */
        notAssigned = -1
    };

    /* TODO: move all of this into a common class that is shared with the FlexItem */
    float width = notAssigned;
    float minWidth = 0;
    float maxWidth = notAssigned;

    float height = notAssigned;
    float minHeight = 0;
    float maxHeight = notAssigned;

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

    /** */
    Margin margin;

    /** */
    juce::Rectangle<float> currentBounds;

    /** Short-hand */
    void setArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd);

    /** Short-hand, span of 1 by default */
    void setArea (Property rowStart, Property columnStart);

    /** Short-hand */
    void setArea (const juce::String& areaName);

    /** Short-hand */
    GridItem withArea (Property rowStart, Property columnStart, Property rowEnd, Property columnEnd) const noexcept;

    /** Short-hand, span of 1 by default */
    GridItem withArea (Property rowStart, Property columnStart) const noexcept;

    /** Short-hand */
    GridItem withArea (const juce::String& areaName)  const noexcept;

    /**  */
    GridItem withRow (StartAndEndProperty row) const noexcept;

    /**  */
    GridItem withColumn (StartAndEndProperty column) const noexcept;

    /** */
    GridItem withAlignSelf (AlignSelf newAlignSelf) const noexcept;

    /** */
    GridItem withJustifySelf (JustifySelf newJustifySelf) const noexcept;

    /** */
    GridItem withWidth (float newWidth) const noexcept;

    /** */
    GridItem withHeight (float newHeight) const noexcept;

    /** */
    GridItem withSize (float newWidth, float newHeight) const noexcept;

    /** */
    GridItem withMargin (Margin newMargin) const noexcept;

    /** Returns a copy of this object with a new order. */
    GridItem withOrder (int newOrder) const noexcept;
};

} // namespace juce
