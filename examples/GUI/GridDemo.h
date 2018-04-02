/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             GridDemo
 version:          1.0.0
 vendor:           juce
 website:          http://juce.com
 description:      Responsive layouts using Grid.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        GridDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct GridDemo   : public Component
{
    GridDemo()
    {
        addGridItemPanel (Colors::aquamarine, "0");
        addGridItemPanel (Colors::red,        "1");
        addGridItemPanel (Colors::blue,       "2");
        addGridItemPanel (Colors::green,      "3");
        addGridItemPanel (Colors::orange,     "4");
        addGridItemPanel (Colors::white,      "5");
        addGridItemPanel (Colors::aquamarine, "6");
        addGridItemPanel (Colors::red,        "7");
        addGridItemPanel (Colors::blue,       "8");
        addGridItemPanel (Colors::green,      "9");
        addGridItemPanel (Colors::orange,     "10");
        addGridItemPanel (Colors::white,      "11");

        setSize (750, 750);
    }

    void addGridItemPanel (Color color, const char* text)
    {
        addAndMakeVisible (items.add (new GridItemPanel (color, text)));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colors::black);
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
                               GridItem (items[8]),
                               GridItem (items[9]),
                               GridItem (items[10]),
                               GridItem (items[11])
                            });

        grid.performLayout (getLocalBounds());
    }

    //==============================================================================
    struct GridItemPanel  : public Component
    {
        GridItemPanel (Color colorToUse, const String& textToUse)
            : color (colorToUse),
              text (textToUse)
        {}

        void paint (Graphics& g) override
        {
            g.fillAll (color.withAlpha (0.5f));

            g.setColor (Colors::black);
            g.drawText (text, getLocalBounds().withSizeKeepingCenter (100, 100), Justification::centered, false);
        }

        Color color;
        String text;
    };

    OwnedArray<GridItemPanel> items;
};
