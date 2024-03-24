/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             GridDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Responsive layouts using Grid.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        GridDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct GridDemo final : public Component
{
    GridDemo()
    {
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

        setSize (750, 750);
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
