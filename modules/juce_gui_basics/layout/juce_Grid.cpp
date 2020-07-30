/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

struct Grid::SizeCalculation
{
    static float getTotalAbsoluteSize (const Array<Grid::TrackInfo>& tracks, Px gapSize) noexcept
    {
        float totalCellSize = 0.0f;

        for (const auto& trackInfo : tracks)
            if (! trackInfo.isFractional() || trackInfo.isAuto())
                totalCellSize += trackInfo.getSize();

        float totalGap = tracks.size() > 1 ? static_cast<float> ((tracks.size() - 1) * gapSize.pixels)
                                           : 0.0f;

        return totalCellSize + totalGap;
    }

    static float getRelativeUnitSize (float size, float totalAbsolute, const Array<Grid::TrackInfo>& tracks) noexcept
    {
        const float totalRelative = jlimit (0.0f, size, size - totalAbsolute);
        float factorsSum = 0.0f;

        for (const auto& trackInfo : tracks)
            if (trackInfo.isFractional())
                factorsSum += trackInfo.getSize();

        jassert (factorsSum != 0.0f);
        return totalRelative / factorsSum;
    }

    //==============================================================================
    static float getTotalAbsoluteHeight (const Array<Grid::TrackInfo>& rowTracks, Px rowGap)
    {
        return getTotalAbsoluteSize (rowTracks, rowGap);
    }

    static float getTotalAbsoluteWidth (const Array<Grid::TrackInfo>& columnTracks, Px columnGap)
    {
        return getTotalAbsoluteSize (columnTracks, columnGap);
    }

    static float getRelativeWidthUnit (float gridWidth, Px columnGap, const Array<Grid::TrackInfo>& columnTracks)
    {
        return getRelativeUnitSize (gridWidth, getTotalAbsoluteWidth (columnTracks, columnGap), columnTracks);
    }

    static float getRelativeHeightUnit (float gridHeight, Px rowGap, const Array<Grid::TrackInfo>& rowTracks)
    {
        return getRelativeUnitSize (gridHeight, getTotalAbsoluteHeight (rowTracks, rowGap), rowTracks);
    }

    //==============================================================================
    static bool hasAnyFractions (const Array<Grid::TrackInfo>& tracks)
    {
        for (auto& t : tracks)
            if (t.isFractional())
                return true;

        return false;
    }

    void computeSizes (float gridWidth, float gridHeight,
                       Px columnGapToUse, Px rowGapToUse,
                       const Array<Grid::TrackInfo>& columnTracks,
                       const Array<Grid::TrackInfo>& rowTracks)
    {
        if (hasAnyFractions (columnTracks))
            relativeWidthUnit = getRelativeWidthUnit (gridWidth, columnGapToUse, columnTracks);
        else
            remainingWidth = gridWidth - getTotalAbsoluteSize (columnTracks, columnGapToUse);

        if (hasAnyFractions (rowTracks))
            relativeHeightUnit = getRelativeHeightUnit (gridHeight, rowGapToUse, rowTracks);
        else
            remainingHeight = gridHeight - getTotalAbsoluteSize (rowTracks, rowGapToUse);
    }

    float relativeWidthUnit  = 0.0f;
    float relativeHeightUnit = 0.0f;
    float remainingWidth     = 0.0f;
    float remainingHeight    = 0.0f;
};

//==============================================================================
struct Grid::PlacementHelpers
{
    enum { invalid = -999999 };
    static constexpr auto emptyAreaCharacter = ".";

    //==============================================================================
    struct LineRange { int start, end; };
    struct LineArea  { LineRange column, row; };
    struct LineInfo  { StringArray lineNames; };

    struct NamedArea
    {
        String name;
        LineArea lines;
    };

    //==============================================================================
    static Array<LineInfo> getArrayOfLinesFromTracks (const Array<Grid::TrackInfo>& tracks)
    {
        // fill line info array
        Array<LineInfo> lines;

        for (int i = 1; i <= tracks.size(); ++i)
        {
            const auto& currentTrack = tracks.getReference (i - 1);

            if (i == 1) // start line
            {
                LineInfo li;
                li.lineNames.add (currentTrack.getStartLineName());
                lines.add (li);
            }

            if (i > 1 && i <= tracks.size()) // two lines in between tracks
            {
                const auto& prevTrack = tracks.getReference (i - 2);

                LineInfo li;
                li.lineNames.add (prevTrack.getEndLineName());
                li.lineNames.add (currentTrack.getStartLineName());

                lines.add (li);
            }

            if (i == tracks.size()) // end line
            {
                LineInfo li;
                li.lineNames.add (currentTrack.getEndLineName());
                lines.add (li);
            }
        }

        jassert (lines.size() == tracks.size() + 1);

        return lines;
    }

    //==============================================================================
    static int deduceAbsoluteLineNumberFromLineName (GridItem::Property prop,
                                                     const Array<Grid::TrackInfo>& tracks)
    {
        jassert (prop.hasAbsolute());

        const auto lines = getArrayOfLinesFromTracks (tracks);
        int count = 0;

        for (int i = 0; i < lines.size(); i++)
        {
            for (const auto& name : lines.getReference (i).lineNames)
            {
                if (prop.getName() == name)
                {
                    ++count;
                    break;
                }
            }

            if (count == prop.getNumber())
                return i + 1;
        }

        jassertfalse;
        return count;
    }

    static int deduceAbsoluteLineNumber (GridItem::Property prop,
                                         const Array<Grid::TrackInfo>& tracks)
    {
        jassert (prop.hasAbsolute());

        if (prop.hasName())
            return deduceAbsoluteLineNumberFromLineName (prop, tracks);

        return prop.getNumber() > 0 ? prop.getNumber() : tracks.size() + 2 + prop.getNumber();
    }

    static int deduceAbsoluteLineNumberFromNamedSpan (int startLineNumber,
                                                      GridItem::Property propertyWithSpan,
                                                      const Array<Grid::TrackInfo>& tracks)
    {
        jassert (propertyWithSpan.hasSpan());

        const auto lines = getArrayOfLinesFromTracks (tracks);
        int count = 0;

        for (int i = startLineNumber; i < lines.size(); i++)
        {
            for (const auto& name : lines.getReference (i).lineNames)
            {
                if (propertyWithSpan.getName() == name)
                {
                    ++count;
                    break;
                }
            }

            if (count == propertyWithSpan.getNumber())
                return i + 1;
        }

        jassertfalse;
        return count;
    }

    static int deduceAbsoluteLineNumberBasedOnSpan (int startLineNumber,
                                                    GridItem::Property propertyWithSpan,
                                                    const Array<Grid::TrackInfo>& tracks)
    {
        jassert (propertyWithSpan.hasSpan());

        if (propertyWithSpan.hasName())
            return deduceAbsoluteLineNumberFromNamedSpan (startLineNumber, propertyWithSpan, tracks);

        return startLineNumber + propertyWithSpan.getNumber();
    }

    //==============================================================================
    static LineRange deduceLineRange (GridItem::StartAndEndProperty prop, const Array<Grid::TrackInfo>& tracks)
    {
        LineRange s;

        jassert (! (prop.start.hasAuto() && prop.end.hasAuto()));

        if (prop.start.hasAbsolute() && prop.end.hasAuto())
        {
            prop.end = GridItem::Span (1);
        }
        else if (prop.start.hasAuto() && prop.end.hasAbsolute())
        {
            prop.start = GridItem::Span (1);
        }

        if (prop.start.hasAbsolute() && prop.end.hasAbsolute())
        {
            s.start = deduceAbsoluteLineNumber (prop.start, tracks);
            s.end   = deduceAbsoluteLineNumber (prop.end, tracks);
        }
        else if (prop.start.hasAbsolute() && prop.end.hasSpan())
        {
            s.start = deduceAbsoluteLineNumber (prop.start, tracks);
            s.end   = deduceAbsoluteLineNumberBasedOnSpan (s.start, prop.end, tracks);
        }
        else if (prop.start.hasSpan() && prop.end.hasAbsolute())
        {
            s.start = deduceAbsoluteLineNumber (prop.end, tracks);
            s.end   = deduceAbsoluteLineNumberBasedOnSpan (s.start, prop.start, tracks);
        }
        else
        {
            // Can't have an item with spans on both start and end.
            jassertfalse;
            s.start = s.end = {};
        }

        // swap if start overtakes end
        if (s.start > s.end)
            std::swap (s.start, s.end);
        else if (s.start == s.end)
            s.end = s.start + 1;

        return s;
    }

    static LineArea deduceLineArea (const GridItem& item,
                                    const Grid& grid,
                                    const std::map<String, LineArea>& namedAreas)
    {
        if (item.area.isNotEmpty() && ! grid.templateAreas.isEmpty())
        {
            // Must be a named area!
            jassert (namedAreas.count (item.area) != 0);

            return namedAreas.at (item.area);
        }

        return { deduceLineRange (item.column, grid.templateColumns),
                 deduceLineRange (item.row,    grid.templateRows) };
    }

    //==============================================================================
    static Array<StringArray> parseAreasProperty (const StringArray& areasStrings)
    {
        Array<StringArray> strings;

        for (const auto& areaString : areasStrings)
            strings.add (StringArray::fromTokens (areaString, false));

        if (strings.size() > 0)
        {
            for (auto s : strings)
            {
                jassert (s.size() == strings[0].size()); // all rows must have the same number of columns
            }
        }

        return strings;
    }

    static NamedArea findArea (Array<StringArray>& stringsArrays)
    {
        NamedArea area;

        for (auto& stringArray : stringsArrays)
        {
            for (auto& string : stringArray)
            {
                // find anchor
                if (area.name.isEmpty())
                {
                    if (string != emptyAreaCharacter)
                    {
                        area.name = string;
                        area.lines.row.start = stringsArrays.indexOf (stringArray) + 1; // non-zero indexed;
                        area.lines.column.start = stringArray.indexOf (string) + 1; // non-zero indexed;

                        area.lines.row.end = stringsArrays.indexOf (stringArray) + 2;
                        area.lines.column.end = stringArray.indexOf (string) + 2;

                        // mark as visited
                        string = emptyAreaCharacter;
                    }
                }
                else
                {
                    if (string == area.name)
                    {
                        area.lines.row.end = stringsArrays.indexOf (stringArray) + 2;
                        area.lines.column.end = stringArray.indexOf (string) + 2;

                        // mark as visited
                        string = emptyAreaCharacter;
                    }
                }
            }
        }

        return area;
    }

    //==============================================================================
    static std::map<String, LineArea> deduceNamedAreas (const StringArray& areasStrings)
    {
        auto stringsArrays = parseAreasProperty (areasStrings);

        std::map<String, LineArea> areas;

        for (auto area = findArea (stringsArrays); area.name.isNotEmpty(); area = findArea (stringsArrays))
        {
            if (areas.count (area.name) == 0)
                areas[area.name] = area.lines;
            else
                // Make sure your template-areas property only has one area with the same name and is well-formed
                jassertfalse;
        }

        return areas;
    }

    //==============================================================================
    static float getCoord (int trackNumber, float relativeUnit, Px gap, const Array<Grid::TrackInfo>& tracks)
    {
        float c = 0;

        for (const auto* it = tracks.begin(); it != tracks.begin() + trackNumber - 1; ++it)
            c += it->getAbsoluteSize (relativeUnit) + static_cast<float> (gap.pixels);

        return c;
    }

    static Rectangle<float> getCellBounds (int columnNumber, int rowNumber,
                                                 const Array<Grid::TrackInfo>& columnTracks,
                                                 const Array<Grid::TrackInfo>& rowTracks,
                                                 Grid::SizeCalculation calculation,
                                                 Px columnGap, Px rowGap)
    {
        jassert (columnNumber >= 1 && columnNumber <= columnTracks.size());
        jassert (rowNumber >= 1 && rowNumber <= rowTracks.size());

        const auto x = getCoord (columnNumber, calculation.relativeWidthUnit, columnGap, columnTracks);
        const auto y = getCoord (rowNumber, calculation.relativeHeightUnit, rowGap, rowTracks);

        const auto& columnTrackInfo = columnTracks.getReference (columnNumber - 1);
        const float width = columnTrackInfo.getAbsoluteSize (calculation.relativeWidthUnit);

        const auto& rowTrackInfo = rowTracks.getReference (rowNumber - 1);
        const float height = rowTrackInfo.getAbsoluteSize (calculation.relativeHeightUnit);

        return { x, y, width, height };
    }

    static Rectangle<float> alignCell (Rectangle<float> area,
                                             int columnNumber, int rowNumber,
                                             int numberOfColumns, int numberOfRows,
                                             Grid::SizeCalculation calculation,
                                             Grid::AlignContent alignContent,
                                             Grid::JustifyContent justifyContent)
    {
        if (alignContent == Grid::AlignContent::end)
            area.setY (area.getY() + calculation.remainingHeight);

        if (justifyContent == Grid::JustifyContent::end)
            area.setX (area.getX() + calculation.remainingWidth);

        if (alignContent == Grid::AlignContent::center)
            area.setY (area.getY() + calculation.remainingHeight / 2);

        if (justifyContent == Grid::JustifyContent::center)
            area.setX (area.getX() + calculation.remainingWidth / 2);

        if (alignContent == Grid::AlignContent::spaceBetween)
        {
            const auto shift = ((float) (rowNumber - 1) * (calculation.remainingHeight / float(numberOfRows - 1)));
            area.setY (area.getY() + shift);
        }

        if (justifyContent == Grid::JustifyContent::spaceBetween)
        {
            const auto shift = ((float) (columnNumber - 1) * (calculation.remainingWidth / float(numberOfColumns - 1)));
            area.setX (area.getX() + shift);
        }

        if (alignContent == Grid::AlignContent::spaceEvenly)
        {
            const auto shift = ((float) rowNumber * (calculation.remainingHeight / float(numberOfRows + 1)));
            area.setY (area.getY() + shift);
        }

        if (justifyContent == Grid::JustifyContent::spaceEvenly)
        {
            const auto shift = ((float) columnNumber * (calculation.remainingWidth / float(numberOfColumns + 1)));
            area.setX (area.getX() + shift);
        }

        if (alignContent == Grid::AlignContent::spaceAround)
        {
            const auto inbetweenShift = calculation.remainingHeight / float(numberOfRows);
            const auto sidesShift = inbetweenShift / 2;
            auto shift = (float) (rowNumber - 1) * inbetweenShift + sidesShift;

            area.setY (area.getY() + shift);
        }

        if (justifyContent == Grid::JustifyContent::spaceAround)
        {
            const auto inbetweenShift = calculation.remainingWidth / float(numberOfColumns);
            const auto sidesShift = inbetweenShift / 2;
            auto shift = (float) (columnNumber - 1) * inbetweenShift + sidesShift;

            area.setX (area.getX() + shift);
        }

        return area;
    }

    static Rectangle<float> getAreaBounds (int columnLineNumberStart, int columnLineNumberEnd,
                                                 int rowLineNumberStart, int rowLineNumberEnd,
                                                 const Array<Grid::TrackInfo>& columnTracks,
                                                 const Array<Grid::TrackInfo>& rowTracks,
                                                 Grid::SizeCalculation calculation,
                                                 Grid::AlignContent alignContent,
                                                 Grid::JustifyContent justifyContent,
                                                 Px columnGap, Px rowGap)
    {
        auto startCell = getCellBounds (columnLineNumberStart, rowLineNumberStart,
                                        columnTracks, rowTracks,
                                        calculation,
                                        columnGap, rowGap);

        auto endCell = getCellBounds (columnLineNumberEnd - 1, rowLineNumberEnd - 1,
                                      columnTracks, rowTracks,
                                      calculation,
                                      columnGap, rowGap);

        startCell = alignCell (startCell,
                               columnLineNumberStart, rowLineNumberStart,
                               columnTracks.size(), rowTracks.size(),
                               calculation,
                               alignContent,
                               justifyContent);

        endCell = alignCell (endCell,
                             columnLineNumberEnd - 1, rowLineNumberEnd - 1,
                             columnTracks.size(), rowTracks.size(),
                             calculation,
                             alignContent,
                             justifyContent);

        auto horizontalRange = startCell.getHorizontalRange().getUnionWith (endCell.getHorizontalRange());
        auto verticalRange = startCell.getVerticalRange().getUnionWith (endCell.getVerticalRange());
        return { horizontalRange.getStart(),  verticalRange.getStart(),
                 horizontalRange.getLength(), verticalRange.getLength() };
    }
};

//==============================================================================
struct Grid::AutoPlacement
{
    using ItemPlacementArray = Array<std::pair<GridItem*, Grid::PlacementHelpers::LineArea>>;

    //==============================================================================
    struct OccupancyPlane
    {
        struct Cell { int column, row; };

        OccupancyPlane (int highestColumnToUse, int highestRowToUse, bool isColumnFirst)
            : highestCrossDimension (isColumnFirst ? highestRowToUse : highestColumnToUse),
              columnFirst (isColumnFirst)
        {}

        Grid::PlacementHelpers::LineArea setCell (Cell cell, int columnSpan, int rowSpan)
        {
            for (int i = 0; i < columnSpan; i++)
                for (int j = 0; j < rowSpan; j++)
                    setCell (cell.column + i, cell.row + j);

            return { { cell.column, cell.column + columnSpan }, { cell.row, cell.row + rowSpan } };
        }

        Grid::PlacementHelpers::LineArea setCell (Cell start, Cell end)
        {
            return setCell (start, std::abs (end.column - start.column),
                                   std::abs (end.row - start.row));
        }

        Cell nextAvailable (Cell referenceCell, int columnSpan, int rowSpan)
        {
            while (isOccupied (referenceCell, columnSpan, rowSpan) || isOutOfBounds (referenceCell, columnSpan, rowSpan))
                referenceCell = advance (referenceCell);

            return referenceCell;
        }

        Cell nextAvailableOnRow (Cell referenceCell, int columnSpan, int rowSpan, int rowNumber)
        {
            if (columnFirst && (rowNumber + rowSpan) > highestCrossDimension)
                highestCrossDimension = rowNumber + rowSpan;

            while (isOccupied (referenceCell, columnSpan, rowSpan)
                   || (referenceCell.row != rowNumber))
                referenceCell = advance (referenceCell);

            return referenceCell;
        }

        Cell nextAvailableOnColumn (Cell referenceCell, int columnSpan, int rowSpan, int columnNumber)
        {
            if (! columnFirst && (columnNumber + columnSpan) > highestCrossDimension)
                highestCrossDimension = columnNumber + columnSpan;

            while (isOccupied (referenceCell, columnSpan, rowSpan)
                   || (referenceCell.column != columnNumber))
                referenceCell = advance (referenceCell);

            return referenceCell;
        }

    private:
        struct SortableCell
        {
            int column, row;
            bool columnFirst;

            bool operator< (const SortableCell& other) const
            {
                if (columnFirst)
                {
                    if (row == other.row)
                        return column < other.column;

                    return row < other.row;
                }

                if (row == other.row)
                    return column < other.column;

                return row < other.row;
            }
        };

        void setCell (int column, int row)
        {
            occupiedCells.insert ({ column, row, columnFirst });
        }

        bool isOccupied (Cell cell) const
        {
            return occupiedCells.count ({ cell.column, cell.row, columnFirst }) > 0;
        }

        bool isOccupied (Cell cell, int columnSpan, int rowSpan) const
        {
            for (int i = 0; i < columnSpan; i++)
                for (int j = 0; j < rowSpan; j++)
                    if (isOccupied ({ cell.column + i, cell.row + j }))
                        return true;

            return false;
        }

        bool isOutOfBounds (Cell cell, int columnSpan, int rowSpan) const
        {
            const auto crossSpan = columnFirst ? rowSpan : columnSpan;

            return (getCrossDimension (cell) + crossSpan) > getHighestCrossDimension();
        }

        int getHighestCrossDimension() const
        {
            Cell cell { 1, 1 };

            if (occupiedCells.size() > 0)
                cell = { occupiedCells.crbegin()->column, occupiedCells.crbegin()->row };

            return std::max (getCrossDimension (cell), highestCrossDimension);
        }

        Cell advance (Cell cell) const
        {
            if ((getCrossDimension (cell) + 1) >= getHighestCrossDimension())
                return fromDimensions (getMainDimension (cell) + 1, 1);

            return fromDimensions (getMainDimension (cell), getCrossDimension (cell) + 1);
        }

        int getMainDimension (Cell cell) const   { return columnFirst ? cell.column : cell.row; }
        int getCrossDimension (Cell cell) const  { return columnFirst ? cell.row : cell.column; }

        Cell fromDimensions (int mainDimension, int crossDimension) const
        {
            if (columnFirst)
                return { mainDimension, crossDimension };

            return { crossDimension, mainDimension };
        }

        int highestCrossDimension;
        bool columnFirst;
        std::set<SortableCell> occupiedCells;
    };

    //==============================================================================
    static bool isFixed (GridItem::StartAndEndProperty prop)
    {
        return prop.start.hasName() || prop.start.hasAbsolute() || prop.end.hasName() || prop.end.hasAbsolute();
    }

    static bool hasFullyFixedPlacement (const GridItem& item)
    {
        if (item.area.isNotEmpty())
            return true;

        if (isFixed (item.column) && isFixed (item.row))
            return true;

        return false;
    }

    static bool hasPartialFixedPlacement (const GridItem& item)
    {
        if (item.area.isNotEmpty())
            return false;

        if (isFixed (item.column) ^ isFixed (item.row))
            return true;

        return false;
    }

    static bool hasAutoPlacement (const GridItem& item)
    {
        return ! hasFullyFixedPlacement (item) && ! hasPartialFixedPlacement (item);
    }

    //==============================================================================
    static bool hasDenseAutoFlow (Grid::AutoFlow autoFlow)
    {
        return autoFlow == Grid::AutoFlow::columnDense
            || autoFlow == Grid::AutoFlow::rowDense;
    }

    static bool isColumnAutoFlow (Grid::AutoFlow autoFlow)
    {
        return autoFlow == Grid::AutoFlow::column
            || autoFlow == Grid::AutoFlow::columnDense;
    }

    //==============================================================================
    static int getSpanFromAuto (GridItem::StartAndEndProperty prop)
    {
        if (prop.end.hasSpan())
            return prop.end.getNumber();

        if (prop.start.hasSpan())
            return prop.start.getNumber();

        return 1;
    }

    //==============================================================================
    ItemPlacementArray deduceAllItems (Grid& grid) const
    {
        const auto namedAreas = Grid::PlacementHelpers::deduceNamedAreas (grid.templateAreas);

        OccupancyPlane plane (jmax (grid.templateColumns.size() + 1, 2),
                              jmax (grid.templateRows.size() + 1, 2),
                              isColumnAutoFlow (grid.autoFlow));

        ItemPlacementArray itemPlacementArray;
        Array<GridItem*> sortedItems;

        for (auto& item : grid.items)
            sortedItems.add (&item);

        std::stable_sort (sortedItems.begin(), sortedItems.end(),
                          [] (const GridItem* i1, const GridItem* i2)  { return i1->order < i2->order; });

        // place fixed items first
        for (auto* item : sortedItems)
        {
            if (hasFullyFixedPlacement (*item))
            {
                const auto a = Grid::PlacementHelpers::deduceLineArea (*item, grid, namedAreas);
                plane.setCell ({ a.column.start, a.row.start }, { a.column.end, a.row.end });
                itemPlacementArray.add ({ item, a });
            }
        }

        OccupancyPlane::Cell lastInsertionCell = { 1, 1 };

        for (auto* item : sortedItems)
        {
            if (hasPartialFixedPlacement (*item))
            {
                if (isFixed (item->column))
                {
                    const auto p = Grid::PlacementHelpers::deduceLineRange (item->column, grid.templateColumns);
                    const auto columnSpan = std::abs (p.start - p.end);
                    const auto rowSpan = getSpanFromAuto (item->row);

                    const auto insertionCell = hasDenseAutoFlow (grid.autoFlow) ? OccupancyPlane::Cell { p.start, 1 }
                                                                                : lastInsertionCell;
                    const auto nextAvailableCell = plane.nextAvailableOnColumn (insertionCell, columnSpan, rowSpan, p.start);
                    const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);
                    lastInsertionCell = nextAvailableCell;

                    itemPlacementArray.add ({ item, lineArea });
                }
                else if (isFixed (item->row))
                {
                    const auto p = Grid::PlacementHelpers::deduceLineRange (item->row, grid.templateRows);
                    const auto columnSpan = getSpanFromAuto (item->column);
                    const auto rowSpan = std::abs (p.start - p.end);

                    const auto insertionCell = hasDenseAutoFlow (grid.autoFlow) ? OccupancyPlane::Cell { 1, p.start }
                                                                                : lastInsertionCell;

                    const auto nextAvailableCell = plane.nextAvailableOnRow (insertionCell, columnSpan, rowSpan, p.start);
                    const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);

                    lastInsertionCell = nextAvailableCell;

                    itemPlacementArray.add ({ item, lineArea });
                }
            }
        }

        lastInsertionCell = { 1, 1 };

        for (auto* item : sortedItems)
        {
            if (hasAutoPlacement (*item))
            {
                const auto columnSpan = getSpanFromAuto (item->column);
                const auto rowSpan = getSpanFromAuto (item->row);

                const auto nextAvailableCell = plane.nextAvailable (lastInsertionCell, columnSpan, rowSpan);
                const auto lineArea = plane.setCell (nextAvailableCell, columnSpan, rowSpan);

                if (! hasDenseAutoFlow (grid.autoFlow))
                    lastInsertionCell = nextAvailableCell;

                itemPlacementArray.add ({ item,  lineArea });
            }
        }

        return itemPlacementArray;
    }

    //==============================================================================
    static std::pair<int, int> getHighestEndLinesNumbers (const ItemPlacementArray& items)
    {
        int columnEndLine = 1;
        int rowEndLine = 1;

        for (auto& item : items)
        {
            const auto p = item.second;
            columnEndLine = std::max (p.column.end, columnEndLine);
            rowEndLine = std::max (p.row.end, rowEndLine);
        }

        return { columnEndLine, rowEndLine };
    }

    static std::pair<Array<TrackInfo>, Array<TrackInfo>> createImplicitTracks (const Grid& grid,
                                                                                           const ItemPlacementArray& items)
    {
        const auto columnAndRowLineEnds = getHighestEndLinesNumbers (items);

        Array<TrackInfo> implicitColumnTracks, implicitRowTracks;

        for (int i = grid.templateColumns.size() + 1; i < columnAndRowLineEnds.first; i++)
            implicitColumnTracks.add (grid.autoColumns);

        for (int i = grid.templateRows.size() + 1; i < columnAndRowLineEnds.second; i++)
            implicitRowTracks.add (grid.autoRows);

        return { implicitColumnTracks, implicitRowTracks };
    }

    //==============================================================================
    static void applySizeForAutoTracks (Array<Grid::TrackInfo>& columns,
                                        Array<Grid::TrackInfo>& rows,
                                        const ItemPlacementArray& itemPlacementArray)
    {
        auto isSpan = [] (Grid::PlacementHelpers::LineRange r) -> bool { return std::abs (r.end - r.start) > 1; };

        auto getHighestItemOnRow = [isSpan] (int rowNumber, const ItemPlacementArray& itemPlacementArrayToUse) -> float
        {
            float highestRowSize = 0.0f;

            for (const auto& i : itemPlacementArrayToUse)
                if (! isSpan (i.second.row) && i.second.row.start == rowNumber)
                    highestRowSize = std::max (highestRowSize, i.first->height + i.first->margin.top + i.first->margin.bottom);

            return highestRowSize;
        };

        auto getHighestItemOnColumn = [isSpan] (int rowNumber, const ItemPlacementArray& itemPlacementArrayToUse) -> float
        {
            float highestColumnSize = 0.0f;
            for (const auto& i : itemPlacementArrayToUse)
                if (! isSpan (i.second.column) && i.second.column.start == rowNumber)
                    highestColumnSize = std::max (highestColumnSize, i.first->width + i.first->margin.left + i.first->margin.right);

            return highestColumnSize;
        };

        for (int i = 0; i < rows.size(); i++)
            if (rows.getReference (i).isAuto())
                rows.getReference (i).size = getHighestItemOnRow (i + 1, itemPlacementArray);

        for (int i = 0; i < columns.size(); i++)
            if (columns.getReference (i).isAuto())
                columns.getReference (i).size = getHighestItemOnColumn (i + 1, itemPlacementArray);
    }
};

//==============================================================================
struct Grid::BoxAlignment
{
    static Rectangle<float> alignItem (const GridItem& item,
                                             const Grid& grid,
                                             Rectangle<float> area)
    {
        // if item align is auto, inherit value from grid
        Grid::AlignItems alignType = Grid::AlignItems::start;
        Grid::JustifyItems justifyType = Grid::JustifyItems::start;

        if (item.alignSelf == GridItem::AlignSelf::autoValue)
            alignType = grid.alignItems;
        else
            alignType = static_cast<Grid::AlignItems> (item.alignSelf);

        if (item.justifySelf == GridItem::JustifySelf::autoValue)
            justifyType = grid.justifyItems;
        else
            justifyType = static_cast<Grid::JustifyItems> (item.justifySelf);

        // subtract margin from area
        area = BorderSize<float> (item.margin.top, item.margin.left, item.margin.bottom, item.margin.right)
                  .subtractedFrom (area);

        // align and justify
        auto r = area;

        if (item.width     != (float) GridItem::notAssigned)  r.setWidth  (item.width);
        if (item.height    != (float) GridItem::notAssigned)  r.setHeight (item.height);
        if (item.maxWidth  != (float) GridItem::notAssigned)  r.setWidth  (jmin (item.maxWidth,  r.getWidth()));
        if (item.minWidth  > 0.0f)                            r.setWidth  (jmax (item.minWidth,  r.getWidth()));
        if (item.maxHeight != (float) GridItem::notAssigned)  r.setHeight (jmin (item.maxHeight, r.getHeight()));
        if (item.minHeight > 0.0f)                            r.setHeight (jmax (item.minHeight, r.getHeight()));

        if (alignType == Grid::AlignItems::start && justifyType == Grid::JustifyItems::start)
            return r;

        if (alignType   == Grid::AlignItems::end)       r.setY (r.getY() + (area.getHeight() - r.getHeight()));
        if (justifyType == Grid::JustifyItems::end)     r.setX (r.getX() + (area.getWidth()  - r.getWidth()));
        if (alignType   == Grid::AlignItems::center)    r.setCentre (r.getCentreX(),    area.getCentreY());
        if (justifyType == Grid::JustifyItems::center)  r.setCentre (area.getCentreX(), r.getCentreY());

        return r;
    }
};

//==============================================================================
Grid::TrackInfo::TrackInfo() noexcept : hasKeyword (true) {}

Grid::TrackInfo::TrackInfo (Px sizeInPixels) noexcept : size (static_cast<float> (sizeInPixels.pixels)), isFraction (false) {}

Grid::TrackInfo::TrackInfo (Fr fractionOfFreeSpace) noexcept : size ((float)fractionOfFreeSpace.fraction), isFraction (true) {}

Grid::TrackInfo::TrackInfo (Px sizeInPixels, const String& endLineNameToUse) noexcept : Grid::TrackInfo (sizeInPixels)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (Fr fractionOfFreeSpace, const String& endLineNameToUse) noexcept : Grid::TrackInfo (fractionOfFreeSpace)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const String& startLineNameToUse, Px sizeInPixels) noexcept : Grid::TrackInfo (sizeInPixels)
{
    startLineName = startLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const String& startLineNameToUse, Fr fractionOfFreeSpace) noexcept : Grid::TrackInfo (fractionOfFreeSpace)
{
    startLineName = startLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const String& startLineNameToUse, Px sizeInPixels, const String& endLineNameToUse) noexcept
  : Grid::TrackInfo (startLineNameToUse, sizeInPixels)
{
    endLineName = endLineNameToUse;
}

Grid::TrackInfo::TrackInfo (const String& startLineNameToUse, Fr fractionOfFreeSpace, const String& endLineNameToUse) noexcept
  : Grid::TrackInfo (startLineNameToUse, fractionOfFreeSpace)
{
    endLineName = endLineNameToUse;
}

float Grid::TrackInfo::getAbsoluteSize (float relativeFractionalUnit) const
{
    if (isFractional())
        return size * relativeFractionalUnit;
    else
        return size;
}

//==============================================================================
Grid::Grid() noexcept {}
Grid::~Grid() noexcept {}

//==============================================================================
void Grid::performLayout (Rectangle<int> targetArea)
{
    const auto itemsAndAreas = Grid::AutoPlacement().deduceAllItems (*this);

    const auto implicitTracks = Grid::AutoPlacement::createImplicitTracks (*this, itemsAndAreas);
    auto columnTracks = templateColumns;
    auto rowTracks = templateRows;
    columnTracks.addArray (implicitTracks.first);
    rowTracks.addArray (implicitTracks.second);

    Grid::AutoPlacement::applySizeForAutoTracks (columnTracks, rowTracks, itemsAndAreas);

    Grid::SizeCalculation calculation;
    calculation.computeSizes (targetArea.toFloat().getWidth(),
                              targetArea.toFloat().getHeight(),
                              columnGap,
                              rowGap,
                              columnTracks,
                              rowTracks);

    for (auto& itemAndArea : itemsAndAreas)
    {
        const auto a = itemAndArea.second;
        const auto areaBounds = Grid::PlacementHelpers::getAreaBounds (a.column.start, a.column.end,
                                                                       a.row.start, a.row.end,
                                                                       columnTracks,
                                                                       rowTracks,
                                                                       calculation,
                                                                       alignContent,
                                                                       justifyContent,
                                                                       columnGap,
                                                                       rowGap);

        auto* item = itemAndArea.first;
        item->currentBounds = Grid::BoxAlignment::alignItem (*item, *this, areaBounds)
                                + targetArea.toFloat().getPosition();

        if (auto* c = item->associatedComponent)
            c->setBounds (item->currentBounds.toNearestIntEdges());
    }
}

//==============================================================================
#if JUCE_UNIT_TESTS

struct GridTests  : public UnitTest
{
    GridTests()
        : UnitTest ("Grid", UnitTestCategories::gui)
    {}

    void runTest() override
    {
        using Fr = Grid::Fr;
        using Tr = Grid::TrackInfo;
        using Rect = Rectangle<float>;

        {
            Grid grid;

            grid.templateColumns.add (Tr (1_fr));
            grid.templateRows.addArray ({ Tr (20_px), Tr (1_fr) });

            grid.items.addArray ({ GridItem().withArea (1, 1),
                                   GridItem().withArea (2, 1) });

            grid.performLayout (Rectangle<int> (200, 400));

            beginTest ("Layout calculation test: 1 column x 2 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  200.f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 20.0f, 200.f, 380.0f));

            grid.templateColumns.add (Tr (50_px));
            grid.templateRows.add (Tr (2_fr));

            grid.items.addArray ( { GridItem().withArea (1, 2),
                                    GridItem().withArea (2, 2),
                                    GridItem().withArea (3, 1),
                                    GridItem().withArea (3, 2) });

            grid.performLayout (Rectangle<int> (150, 170));

            beginTest ("Layout calculation test: 2 columns x 3 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f,   0.0f,  100.0f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f,   20.0f, 100.0f, 50.0f));
            expect (grid.items[2].currentBounds == Rect (100.0f, 0.0f,  50.0f,  20.0f));
            expect (grid.items[3].currentBounds == Rect (100.0f, 20.0f, 50.0f,  50.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f,   70.0f, 100.0f, 100.0f));
            expect (grid.items[5].currentBounds == Rect (100.0f, 70.0f, 50.0f,  100.0f));

            grid.columnGap = 20_px;
            grid.rowGap    = 10_px;

            grid.performLayout (Rectangle<int> (200, 310));

            beginTest ("Layout calculation test: 2 columns x 3 rows: rowGap of 10 and columnGap of 20");
            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f, 130.0f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 30.0f, 130.0f, 90.0f));
            expect (grid.items[2].currentBounds == Rect (150.0f, 0.0f, 50.0f, 20.0f));
            expect (grid.items[3].currentBounds == Rect (150.0f, 30.0f, 50.0f, 90.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f, 130.0f, 130.0f, 180.0f));
            expect (grid.items[5].currentBounds == Rect (150.0f, 130.0f, 50.0f,  180.0f));
        }

        {
            Grid grid;

            grid.templateColumns.addArray ({ Tr ("first", 20_px, "in"), Tr ("in", 1_fr, "in"), Tr (20_px, "last") });
            grid.templateRows.addArray ({ Tr (1_fr),
                                          Tr (20_px)});

            {
                beginTest ("Grid items placement tests: integer and custom ident, counting forward");

                GridItem i1, i2, i3, i4, i5;
                i1.column = { 1, 4 };
                i1.row    = { 1, 2 };

                i2.column = { 1, 3 };
                i2.row    = { 1, 3 };

                i3.column = { "first", "in" };
                i3.row    = { 2, 3 };

                i4.column = { "first", { 2, "in" } };
                i4.row    = { 1, 2 };

                i5.column = { "first", "last" };
                i5.row    = { 1, 2 };

                grid.items.addArray ({ i1, i2, i3, i4, i5 });

                grid.performLayout ({ 140, 100 });

                expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
                expect (grid.items[1].currentBounds == Rect (0.0f, 0.0f,  120.0f, 100.0f));
                expect (grid.items[2].currentBounds == Rect (0.0f, 80.0f, 20.0f,  20.0f));
                expect (grid.items[3].currentBounds == Rect (0.0f, 0.0f,  120.0f, 80.0f));
                expect (grid.items[4].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
            }
        }

        {
            Grid grid;

            grid.templateColumns.addArray ({ Tr ("first", 20_px, "in"), Tr ("in", 1_fr, "in"), Tr (20_px, "last") });
            grid.templateRows.addArray ({ Tr (1_fr),
                                          Tr (20_px)});

            beginTest ("Grid items placement tests: integer and custom ident, counting forward, reversed end and start");

            GridItem i1, i2, i3, i4, i5;
            i1.column = { 4, 1 };
            i1.row    = { 2, 1 };

            i2.column = { 3, 1 };
            i2.row    = { 3, 1 };

            i3.column = { "in", "first" };
            i3.row    = { 3, 2 };

            i4.column = { "first", { 2, "in" } };
            i4.row    = { 1, 2 };

            i5.column = { "last", "first" };
            i5.row    = { 1, 2 };

            grid.items.addArray ({ i1, i2, i3, i4, i5 });

            grid.performLayout ({ 140, 100 });

            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 0.0f,  120.0f, 100.0f));
            expect (grid.items[2].currentBounds == Rect (0.0f, 80.0f, 20.0f,  20.0f));
            expect (grid.items[3].currentBounds == Rect (0.0f, 0.0f,  120.0f, 80.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f, 0.0f,  140.0f, 80.0f));
        }

        {
            beginTest ("Grid items placement tests: areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (Fr (1_fr)), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.templateAreas = { "header header header header",
                                   "main main . sidebar",
                                   "footer footer footer footer" };

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                });

            grid.performLayout ({ 300, 150 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   300.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (250.f, 50.f,  50.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 300.f, 50.f));
        }

        {
            beginTest ("Grid implicit rows and columns: triggered by areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (1_fr), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.autoRows = Tr (30_px);
            grid.autoColumns = Tr (30_px);

            grid.templateAreas = { "header header header header header",
                                   "main main . sidebar sidebar",
                                   "footer footer footer footer footer",
                                   "sub sub sub sub sub"};

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                   GridItem().withArea ("sub"),
                                });

            grid.performLayout ({ 330, 180 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   330.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (250.f, 50.f,  80.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 330.f, 50.f));
            expect (grid.items[4].currentBounds == Rect (0.f,   150.f, 330.f, 30.f));
        }

        {
            beginTest ("Grid implicit rows and columns: triggered by areas");

            Grid grid;

            grid.templateColumns =       { Tr (50_px), Tr (100_px), Tr (1_fr), Tr (50_px) };
            grid.templateRows = { Tr (50_px),
                                  Tr (1_fr),
                                  Tr (50_px) };

            grid.autoRows = Tr (1_fr);
            grid.autoColumns = Tr (1_fr);

            grid.templateAreas = { "header header header header",
                                   "main main . sidebar",
                                   "footer footer footer footer" };

            grid.items.addArray ({ GridItem().withArea ("header"),
                                   GridItem().withArea ("main"),
                                   GridItem().withArea ("sidebar"),
                                   GridItem().withArea ("footer"),
                                   GridItem().withArea (4, 5, 6, 7)
                                });

            grid.performLayout ({ 350, 250 });

            expect (grid.items[0].currentBounds == Rect (0.f,   0.f,   250.f, 50.f));
            expect (grid.items[1].currentBounds == Rect (0.f,   50.f,  150.f, 50.f));
            expect (grid.items[2].currentBounds == Rect (200.f, 50.f,  50.f,  50.f));
            expect (grid.items[3].currentBounds == Rect (0.f,   100.f, 250.f, 50.f));
            expect (grid.items[4].currentBounds == Rect (250.f, 150.f, 100.f, 100.f));
        }

    }
};

static GridTests gridUnitTests;

#endif

} // namespace juce
