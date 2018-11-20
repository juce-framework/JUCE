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

struct GridTests  : public juce::UnitTest
{
    GridTests() : juce::UnitTest ("Grid class") {}

    void runTest() override
    {
        using Fr = juce::Grid::Fr;
        using Tr = juce::Grid::TrackInfo;
        using Rect = juce::Rectangle<float>;
        using Grid = juce::Grid;

        {
            Grid grid;

            grid.templateColumns.add (Tr (1_fr));
            grid.templateRows.addArray ({ Tr (20_px), Tr (1_fr) });

            grid.items.addArray ({ GridItem().withArea (1, 1),
                                   GridItem().withArea (2, 1) });

            grid.performLayout (juce::Rectangle<int> (200, 400));

            beginTest ("Layout calculation test: 1 column x 2 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f, 0.0f,  200.f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f, 20.0f, 200.f, 380.0f));

            grid.templateColumns.add (Tr (50_px));
            grid.templateRows.add (Tr (2_fr));

            grid.items.addArray ( { GridItem().withArea (1, 2),
                                    GridItem().withArea (2, 2),
                                    GridItem().withArea (3, 1),
                                    GridItem().withArea (3, 2) });

            grid.performLayout (juce::Rectangle<int> (150, 170));

            beginTest ("Layout calculation test: 2 columns x 3 rows: no gap");
            expect (grid.items[0].currentBounds == Rect (0.0f,   0.0f,  100.0f, 20.0f));
            expect (grid.items[1].currentBounds == Rect (0.0f,   20.0f, 100.0f, 50.0f));
            expect (grid.items[2].currentBounds == Rect (100.0f, 0.0f,  50.0f,  20.0f));
            expect (grid.items[3].currentBounds == Rect (100.0f, 20.0f, 50.0f,  50.0f));
            expect (grid.items[4].currentBounds == Rect (0.0f,   70.0f, 100.0f, 100.0f));
            expect (grid.items[5].currentBounds == Rect (100.0f, 70.0f, 50.0f,  100.0f));

            grid.columnGap = 20_px;
            grid.rowGap    = 10_px;

            grid.performLayout (juce::Rectangle<int> (200, 310));

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

} // namespace juce
