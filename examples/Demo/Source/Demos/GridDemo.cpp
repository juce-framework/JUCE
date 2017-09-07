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

#include "../JuceDemoHeader.h"

// these classes are C++11-only
#if JUCE_HAS_CONSTEXPR

struct GridDemo   : public Component
{
    GridDemo()
    {
        addGridItemPanel (Colours::aquamarine, "0");
        addGridItemPanel (Colours::aquamarine, "0");
        addGridItemPanel (Colours::red,        "1");
        addGridItemPanel (Colours::blue,       "2");
        addGridItemPanel (Colours::green,      "3");
        addGridItemPanel (Colours::orange,     "4");
        addGridItemPanel (Colours::white,      "5");
        addGridItemPanel (Colours::aquamarine, "6");
        addGridItemPanel (Colours::red,        "7");
        addGridItemPanel (Colours::blue,       "8");
        addGridItemPanel (Colours::green,      "9");
        addGridItemPanel (Colours::orange,     "10");
        addGridItemPanel (Colours::white,      "11");
    }

    void addGridItemPanel (Colour colour, const char* text)
    {
        addAndMakeVisible (items.add (new GridItemPanel (colour, text)));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
         Grid grid;

         grid.rowGap    = 20_px;
         grid.columnGap = 20_px;

         using Track = Grid::TrackInfo;

         grid.templateRows = { Track (1_fr), Track (1_fr), Track (1_fr) };

         grid.templateColumns = { Track (1_fr),
                                  Track (1_fr),
                                  Track (1_fr) };


         grid.autoColumns = Track (1_fr);
         grid.autoRows    = Track (1_fr);

         grid.autoFlow = Grid::AutoFlow::column;

         grid.items.addArray ({ GridItem (items[0]).withArea (2, 2, 4, 4),
                                GridItem (items[1]),
                                GridItem (items[2]).withArea ({}, 3),
                                GridItem (items[3]),
                                GridItem (items[4]).withArea (GridItem::Span (2), {}),
                                GridItem (items[5]),
                                GridItem (items[6]),
                                GridItem (items[7]),
                                GridItem (items[8])
                              });

         grid.performLayout (getLocalBounds());
    }

    //==============================================================================
    struct GridItemPanel  : public Component
    {
        GridItemPanel (Colour colourToUse, const String& textToUse)
            : colour (colourToUse),
              text (textToUse)
        {}

        void paint (Graphics& g) override
        {
            g.fillAll (colour.withAlpha (0.5f));

            g.setColour (Colours::black);
            g.drawText (text, getLocalBounds().withSizeKeepingCentre (100, 100), Justification::centred, false);
        }

        Colour colour;
        String text;
    };

    OwnedArray<GridItemPanel> items;
};

// This static object will register this demo type in a global list of demos..
static JuceDemoType<GridDemo> demo ("10 Components: GridDemo");

#endif // JUCE_HAS_CONSTEXPR
