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
    /** */
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
    /** */
    enum class JustifyItems : int   { start = 0, end, center, stretch };
    /** */
    enum class AlignItems : int     { start = 0, end, center, stretch };
    /** */
    enum class JustifyContent       { start, end, center, stretch, spaceAround, spaceBetween, spaceEvenly };
    /** */
    enum class AlignContent         { start, end, center, stretch, spaceAround, spaceBetween, spaceEvenly };
    /** */
    enum class AutoFlow             { row, column, rowDense, columnDense };


    //==============================================================================
    /** */
    Grid() noexcept;

    /** Destructor */
    ~Grid() noexcept;

    //==============================================================================
    /** */
    JustifyItems   justifyItems   = JustifyItems::stretch;
    /** */
    AlignItems     alignItems     = AlignItems::stretch;
    /** */
    JustifyContent justifyContent = JustifyContent::stretch;
    /** */
    AlignContent   alignContent   = AlignContent::stretch;
    /** */
    AutoFlow       autoFlow       = AutoFlow::row;


    //==============================================================================
    /** */
    juce::Array<TrackInfo> templateColumns;

    /** */
    juce::Array<TrackInfo> templateRows;

    /** Template areas */
    juce::StringArray templateAreas;

    /** */
    TrackInfo autoRows;

    /** */
    TrackInfo autoColumns;

    /** */
    Px columnGap { 0 };
    /** */
    Px rowGap { 0 };

    /** */
    void setGap (Px sizeInPixels) noexcept          { rowGap = columnGap = sizeInPixels; }

    //==============================================================================
    /** */
    juce::Array<GridItem> items;

    //==============================================================================
    /** */
    void performLayout (juce::Rectangle<int>);

    //==============================================================================
    /** */
    int getNumberOfColumns() const noexcept         { return templateColumns.size(); }
    /** */
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
