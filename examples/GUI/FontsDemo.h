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

 name:             FontsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays different font styles and types.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        FontsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class FontsDemo  : public Component,
                   private ListBoxModel,
                   private Slider::Listener
{
public:
    FontsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (listBox);
        addAndMakeVisible (demoTextBox);
        addAndMakeVisible (heightSlider);
        addAndMakeVisible (heightLabel);
        addAndMakeVisible (kerningLabel);
        addAndMakeVisible (kerningSlider);
        addAndMakeVisible (scaleLabel);
        addAndMakeVisible (scaleSlider);
        addAndMakeVisible (boldToggle);
        addAndMakeVisible (italicToggle);
        addAndMakeVisible (styleBox);

        kerningLabel.attachToComponent (&kerningSlider, true);
        heightLabel .attachToComponent (&heightSlider,  true);
        scaleLabel  .attachToComponent (&scaleSlider,   true);
        styleLabel  .attachToComponent (&styleBox,      true);

        heightSlider .addListener (this);
        kerningSlider.addListener (this);
        scaleSlider  .addListener (this);

        boldToggle  .onClick  = [this] { refreshPreviewBoxFont(); };
        italicToggle.onClick  = [this] { refreshPreviewBoxFont(); };
        styleBox    .onChange = [this] { refreshPreviewBoxFont(); };

        Font::findFonts (fonts);   // Generate the list of fonts

        listBox.setRowHeight (20);
        listBox.setModel (this);   // Tell the listbox where to get its data model
        listBox.setColour (ListBox::textColourId, Colours::black);
        listBox.setColour (ListBox::backgroundColourId, Colours::white);

        heightSlider .setRange (3.0, 150.0, 0.01);
        scaleSlider  .setRange (0.2, 3.0, 0.01);
        kerningSlider.setRange (-2.0, 2.0, 0.01);

        scaleSlider  .setValue (1.0);   // Set some initial values for the sliders.
        heightSlider .setValue (20.0);
        kerningSlider.setValue (0);

        // set up the layout and resizer bars..
        verticalLayout.setItemLayout (0, -0.2, -0.8, -0.35); // width of the font list must be
                                                             // between 20% and 80%, preferably 50%
        verticalLayout.setItemLayout (1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
        verticalLayout.setItemLayout (2, 150, -1.0, -0.65);  // the components on the right must be
                                                             // at least 150 pixels wide, preferably 50% of the total width

        verticalDividerBar.reset (new StretchableLayoutResizerBar (&verticalLayout, 1, true));
        addAndMakeVisible (verticalDividerBar.get());

        // ..and pick a random font to select intially
        listBox.selectRow (Random::getSystemRandom().nextInt (fonts.size()));

        demoTextBox.setMultiLine (true);
        demoTextBox.setReturnKeyStartsNewLine (true);
        demoTextBox.setText ("Aa Bb Cc Dd Ee Ff Gg Hh Ii\n"
                             "Jj Kk Ll Mm Nn Oo Pp Qq Rr\n"
                             "Ss Tt Uu Vv Ww Xx Yy Zz\n"
                             "0123456789\n\n"
                             "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt "
                             "ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco "
                             "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in "
                             "voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
                             "non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

        demoTextBox.setCaretPosition (0);
        demoTextBox.setColour (TextEditor::textColourId, Colours::black);
        demoTextBox.setColour (TextEditor::backgroundColourId, Colours::white);

        setSize (750, 750);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (5);

        // lay out the list box and vertical divider..
        Component* vcomps[] = { &listBox, verticalDividerBar.get(), nullptr };

        verticalLayout.layOutComponents (vcomps, 3,
                                         r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                         false,     // lay out side-by-side
                                         true);     // resize the components' heights as well as widths


        r.removeFromLeft (verticalDividerBar->getRight());

        styleBox.setBounds (r.removeFromBottom (26));
        r.removeFromBottom (8);

        int labelWidth = 60;

        auto row = r.removeFromBottom (30);
        row.removeFromLeft (labelWidth);
        boldToggle.setBounds (row.removeFromLeft (row.getWidth() / 2));
        italicToggle.setBounds (row);

        r.removeFromBottom (8);
        scaleSlider.setBounds (r.removeFromBottom (30).withTrimmedLeft (labelWidth));
        r.removeFromBottom (8);
        kerningSlider.setBounds (r.removeFromBottom (30).withTrimmedLeft (labelWidth));
        r.removeFromBottom (8);
        heightSlider.setBounds (r.removeFromBottom (30).withTrimmedLeft (labelWidth));
        r.removeFromBottom (8);
        demoTextBox.setBounds (r);
    }

    void sliderValueChanged (Slider* sliderThatWasMoved) override
    {
        if (sliderThatWasMoved == &heightSlider)            refreshPreviewBoxFont();
        else if (sliderThatWasMoved == &kerningSlider)      refreshPreviewBoxFont();
        else if (sliderThatWasMoved == &scaleSlider)        refreshPreviewBoxFont();
    }

    // The following methods implement the ListBoxModel virtual methods:
    int getNumRows() override
    {
        return fonts.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);

        auto font = fonts[rowNumber];

        AttributedString s;
        s.setWordWrap (AttributedString::none);
        s.setJustification (Justification::centredLeft);
        s.append (font.getTypefaceName(), font.withHeight (height * 0.7f), Colours::black);
        s.append ("   " + font.getTypefaceName(), Font (height * 0.5f, Font::italic), Colours::grey);

        s.draw (g, Rectangle<int> (width, height).expanded (-4, 50).toFloat());
    }

    void selectedRowsChanged (int /*lastRowselected*/) override
    {
        refreshPreviewBoxFont();
    }

private:
    Array<Font> fonts;
    StringArray currentStyleList;

    ListBox listBox;
    TextEditor demoTextBox;

    Label heightLabel   { {}, "Height:" },
          kerningLabel  { {}, "Kerning:" },
          scaleLabel    { "Scale:" },
          styleLabel    { "Style" };

    ToggleButton boldToggle   { "Bold" },
                 italicToggle { "Italic" };

    Slider heightSlider, kerningSlider, scaleSlider;
    ComboBox styleBox;

    StretchableLayoutManager verticalLayout;
    std::unique_ptr<StretchableLayoutResizerBar> verticalDividerBar;

    //==============================================================================
    void refreshPreviewBoxFont()
    {
        auto bold   = boldToggle  .getToggleState();
        auto italic = italicToggle.getToggleState();
        auto useStyle = ! (bold || italic);

        auto font = fonts[listBox.getSelectedRow()];

        font = font.withPointHeight        ((float) heightSlider .getValue())
                   .withExtraKerningFactor ((float) kerningSlider.getValue())
                   .withHorizontalScale    ((float) scaleSlider  .getValue());

        if (bold)    font = font.boldened();
        if (italic)  font = font.italicised();

        updateStylesList (font);

        styleBox.setEnabled (useStyle);

        if (useStyle)
            font = font.withTypefaceStyle (styleBox.getText());

        demoTextBox.applyFontToAllText (font);
    }

    void updateStylesList (const Font& newFont)
    {
        auto newStyles = newFont.getAvailableStyles();

        if (newStyles != currentStyleList)
        {
            currentStyleList = newStyles;

            styleBox.clear();
            styleBox.addItemList (newStyles, 1);
            styleBox.setSelectedItemIndex (0);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontsDemo)
};
