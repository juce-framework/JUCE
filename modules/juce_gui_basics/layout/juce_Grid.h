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
    Container that handles geometry for grid layouts (fixed columns and rows) using a set of declarative rules.

    Implemented from the `CSS Grid Layout` specification as described at:
    https://css-tricks.com/snippets/css/complete-guide-grid/

    @see GridItem

    @tags{GUI}
*/
class JUCE_API  Grid  final
{
public:
    //==============================================================================
    /** A size in pixels */
    struct Px  final
    {
        explicit Px (float p) : pixels (static_cast<long double>(p)) { /*sta (p >= 0.0f);*/ }
        explicit Px (int p)   : pixels (static_cast<long double>(p)) { /*sta (p >= 0.0f);*/ }
        explicit constexpr Px (long double p)        : pixels (p) {}
        explicit constexpr Px (unsigned long long p) : pixels (static_cast<long double>(p)) {}

        long double pixels;
    };

    /** A fractional ratio integer */
    struct Fr  final
    {
        explicit Fr (int f) : fraction (static_cast<unsigned long long> (f)) {}
        explicit constexpr Fr (unsigned long long p) : fraction (p) {}

        unsigned long long fraction;
    };

    //==============================================================================
    /** Represents a track. */
    struct TrackInfo  final
    {
        /** Creates a track with auto dimension. */
        TrackInfo() noexcept;
        /** */
        TrackInfo (Px sizeInPixels) noexcept;
        /** */
        TrackInfo (Fr fractionOfFreeSpace) noexcept;

        /** */
        TrackInfo (Px sizeInPixels, const juce::String& endLineNameToUse) noexcept;
        /** */
        TrackInfo (Fr fractionOfFreeSpace, const juce::String& endLineNameToUse) noexcept;

        /** */
        TrackInfo (const juce::String& startLineNameToUse, Px sizeInPixels) noexcept;
        /** */
        TrackInfo (const juce::String& startLineNameToUse, Fr fractionOfFreeSpace) noexcept;

        /** */
        TrackInfo (const juce::String& startLineNameToUse, Px sizeInPixels, const juce::String& endLineNameToUse) noexcept;
        /** */
        TrackInfo (const juce::String& startLineNameToUse, Fr fractionOfFreeSpace, const juce::String& endLineNameToUse) noexcept;

    private:
        friend class Grid;
        friend class GridItem;

        float size = 0; // Either a fraction or an absolute size in pixels
        bool isFraction = false;
        bool hasKeyword = false;

        juce::String startLineName, endLineName;
    };

    //==============================================================================
    /** Possible values for the justifyItems property. */
    enum class JustifyItems : int
    {
        start = 0,                /**< Content inside the item is justified towards the left. */
        end,                      /**< Content inside the item is justified towards the right. */
        center,                   /**< Content inside the item is justified towards the center. */
        stretch                   /**< Content inside the item is stretched from left to right. */
    };

    /** Possible values for the alignItems property. */
    enum class AlignItems : int
    {
        start = 0,                /**< Content inside the item is aligned towards the top. */
        end,                      /**< Content inside the item is aligned towards the bottom. */
        center,                   /**< Content inside the item is aligned towards the center. */
        stretch                   /**< Content inside the item is stretched from top to bottom. */
    };

    /** Possible values for the justifyContent property. */
    enum class JustifyContent
    {
        start,                    /**< Items are justified towards the left of the container. */
        end,                      /**< Items are justified towards the right of the container. */
        center,                   /**< Items are justified towards the center of the container. */
        stretch,                  /**< Items are stretched from left to right of the container. */
        spaceAround,              /**< Items are evenly spaced along the row with spaces between them. */
        spaceBetween,             /**< Items are evenly spaced along the row with spaces around them. */
        spaceEvenly               /**< Items are evenly spaced along the row with even amount of spaces between them. */
    };

    /** Possible values for the alignContent property. */
    enum class AlignContent
    {
        start,                    /**< Items are aligned towards the top of the container. */
        end,                      /**< Items are aligned towards the bottom of the container. */
        center,                   /**< Items are aligned towards the center of the container. */
        stretch,                  /**< Items are stretched from top to bottom of the container. */
        spaceAround,              /**< Items are evenly spaced along the column with spaces between them. */
        spaceBetween,             /**< Items are evenly spaced along the column with spaces around them. */
        spaceEvenly               /**< Items are evenly spaced along the column with even amount of spaces between them. */
    };

    /** Possible values for the autoFlow property. */
    enum class AutoFlow
    {
        row,                      /**< Fills the grid by adding rows of items. */
        column,                   /**< Fills the grid by adding columns of items. */
        rowDense,                 /**< Fills the grid by adding rows of items and attempts to fill in gaps. */
        columnDense               /**< Fills the grid by adding columns of items and attempts to fill in gaps. */
    };


    //==============================================================================
    /** Creates an empty Grid container with default parameters. */
    Grid() noexcept;

    /** Destructor */
    ~Grid() noexcept;

    //==============================================================================
    /** Specifies the alignment of content inside the items along the rows. */
    JustifyItems   justifyItems   = JustifyItems::stretch;

    /** Specifies the alignment of content inside the items along the columns. */
    AlignItems     alignItems     = AlignItems::stretch;

    /** Specifies the alignment of items along the rows. */
    JustifyContent justifyContent = JustifyContent::stretch;

    /** Specifies the alignment of items along the columns. */
    AlignContent   alignContent   = AlignContent::stretch;

    /** Specifies how the auto-placement algorithm places items. */
    AutoFlow       autoFlow       = AutoFlow::row;


    //==============================================================================
    /** The set of column tracks to lay out. */
    juce::Array<TrackInfo> templateColumns;

    /** The set of row tracks to lay out. */
    juce::Array<TrackInfo> templateRows;

    /** Template areas */
    juce::StringArray templateAreas;

    /** The row track for auto dimension. */
    TrackInfo autoRows;

    /** The column track for auto dimension. */
    TrackInfo autoColumns;

    /** The gap in pixels between columns. */
    Px columnGap { 0 };
    /** The gap in pixels between rows. */
    Px rowGap { 0 };

    /** Sets the gap between rows and columns in pixels. */
    void setGap (Px sizeInPixels) noexcept          { rowGap = columnGap = sizeInPixels; }

    //==============================================================================
    /** The set of items to lay-out. */
    juce::Array<GridItem> items;

    //==============================================================================
    /** Lays-out the grid's items within the given rectangle. */
    void performLayout (juce::Rectangle<int>);

    //==============================================================================
    /** Returns the number of columns. */
    int getNumberOfColumns() const noexcept         { return templateColumns.size(); }
    /** Returns the number of rows. */
    int getNumberOfRows() const noexcept            { return templateRows.size(); }

private:
    //==============================================================================
    struct SizeCalculation;
    struct PlacementHelpers;
    struct AutoPlacement;
    struct BoxAlignment;
};

constexpr Grid::Px operator"" _px (long double px)          { return Grid::Px { px }; }
constexpr Grid::Px operator"" _px (unsigned long long px)   { return Grid::Px { px }; }
constexpr Grid::Fr operator"" _fr (unsigned long long fr)   { return Grid::Fr { fr }; }

} // namespace juce
