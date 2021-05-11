/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

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
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

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
        addAndMakeVisible (horizontalJustificationLabel);
        addAndMakeVisible (verticalJustificationLabel);
        addAndMakeVisible (scaleSlider);
        addAndMakeVisible (boldToggle);
        addAndMakeVisible (italicToggle);
        addAndMakeVisible (underlineToggle);
        addAndMakeVisible (styleBox);
        addAndMakeVisible (horizontalJustificationBox);
        addAndMakeVisible (verticalJustificationBox);
        addAndMakeVisible (resetButton);

        kerningLabel                .attachToComponent (&kerningSlider,              true);
        heightLabel                 .attachToComponent (&heightSlider,               true);
        scaleLabel                  .attachToComponent (&scaleSlider,                true);
        styleLabel                  .attachToComponent (&styleBox,                   true);
        horizontalJustificationLabel.attachToComponent (&horizontalJustificationBox, true);
        verticalJustificationLabel  .attachToComponent (&verticalJustificationBox,   true);

        heightSlider .addListener (this);
        kerningSlider.addListener (this);
        scaleSlider  .addListener (this);

        boldToggle     .onClick  = [this] { refreshPreviewBoxFont(); };
        italicToggle   .onClick  = [this] { refreshPreviewBoxFont(); };
        underlineToggle.onClick  = [this] { refreshPreviewBoxFont(); };
        styleBox       .onChange = [this] { refreshPreviewBoxFont(); };

        Font::findFonts (fonts);   // Generate the list of fonts

        listBox.setTitle ("Fonts");
        listBox.setRowHeight (20);
        listBox.setModel (this);   // Tell the listbox where to get its data model
        listBox.setColour (ListBox::textColourId, Colours::black);
        listBox.setColour (ListBox::backgroundColourId, Colours::white);

        heightSlider .setRange (3.0, 150.0, 0.01);
        scaleSlider  .setRange (0.2, 3.0, 0.01);
        kerningSlider.setRange (-2.0, 2.0, 0.01);

        // set up the layout and resizer bars..
        verticalLayout.setItemLayout (0, -0.2, -0.8, -0.35); // width of the font list must be
                                                             // between 20% and 80%, preferably 50%
        verticalLayout.setItemLayout (1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
        verticalLayout.setItemLayout (2, 150, -1.0, -0.65);  // the components on the right must be
                                                             // at least 150 pixels wide, preferably 50% of the total width

        verticalDividerBar.reset (new StretchableLayoutResizerBar (&verticalLayout, 1, true));
        addAndMakeVisible (verticalDividerBar.get());

        // ..and pick a random font to select initially
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

        demoTextBox.setWhitespaceUnderlined (false);

        resetButton.onClick = [this] { resetToDefaultParameters(); };

        setupJustificationOptions();
        resetToDefaultParameters();

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

        resetButton.setBounds (r.removeFromBottom (30).reduced (jmax (20, r.getWidth() / 5), 0));
        r.removeFromBottom (8);

        const int labelWidth = 60;

        auto styleArea = r.removeFromBottom (26);
        styleArea.removeFromLeft (labelWidth);
        styleBox.setBounds (styleArea);
        r.removeFromBottom (8);

        auto row = r.removeFromBottom (30);
        row.removeFromLeft (labelWidth);
        auto toggleWidth = row.getWidth() / 3;
        boldToggle     .setBounds (row.removeFromLeft (toggleWidth));
        italicToggle   .setBounds (row.removeFromLeft (toggleWidth));
        underlineToggle.setBounds (row);

        r.removeFromBottom (8);
        horizontalJustificationBox.setBounds (r.removeFromBottom (30).withTrimmedLeft (labelWidth * 3));
        r.removeFromBottom (8);
        verticalJustificationBox.setBounds (r.removeFromBottom (30).withTrimmedLeft (labelWidth * 3));
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
        if      (sliderThatWasMoved == &heightSlider)   refreshPreviewBoxFont();
        else if (sliderThatWasMoved == &kerningSlider)  refreshPreviewBoxFont();
        else if (sliderThatWasMoved == &scaleSlider)    refreshPreviewBoxFont();
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
        s.append (getNameForRow (rowNumber), font.withHeight ((float) height * 0.7f), Colours::black);
        s.append ("   " + font.getTypefaceName(), Font ((float) height * 0.5f, Font::italic), Colours::grey);

        s.draw (g, Rectangle<int> (width, height).expanded (-4, 50).toFloat());
    }

    String getNameForRow (int rowNumber) override
    {
        return fonts[rowNumber].getTypefaceName();
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

    const double defaultScale = 1.0, defaultHeight = 20.0, defaultKerning = 0.0;
    const bool defaultBold = false, defaultItalic = false, defaultUnderlined = false;
    const int defaultStyle = 0, defaultHorizontalJustification = 0, defaultVerticalJustification = 0;

    Label heightLabel  { {}, "Height:" },
          kerningLabel { {}, "Kerning:" },
          scaleLabel   { {}, "Scale:" },
          styleLabel   { {}, "Style:" },
          horizontalJustificationLabel { {}, "Justification (horizontal):" },
          verticalJustificationLabel   { {}, "Justification (vertical):" };

    ToggleButton boldToggle      { "Bold" },
                 italicToggle    { "Italic" },
                 underlineToggle { "Underlined" };

    TextButton resetButton { "Reset" };

    Slider heightSlider, kerningSlider, scaleSlider;
    ComboBox styleBox, horizontalJustificationBox, verticalJustificationBox;

    StretchableLayoutManager verticalLayout;
    std::unique_ptr<StretchableLayoutResizerBar> verticalDividerBar;

    StringArray horizontalJustificationStrings { "Left", "Centred", "Right" },
                verticalJustificationStrings   { "Top",  "Centred", "Bottom" };

    Array<int>  horizontalJustificationFlags { Justification::left, Justification::horizontallyCentred, Justification::right },
                verticalJustificationFlags   { Justification::top,  Justification::verticallyCentred, Justification::bottom};

    //==============================================================================
    void resetToDefaultParameters()
    {
        scaleSlider  .setValue (defaultScale);
        heightSlider .setValue (defaultHeight);
        kerningSlider.setValue (defaultKerning);

        boldToggle     .setToggleState (defaultBold,       sendNotificationSync);
        italicToggle   .setToggleState (defaultItalic,     sendNotificationSync);
        underlineToggle.setToggleState (defaultUnderlined, sendNotificationSync);

        styleBox.setSelectedItemIndex (defaultStyle);
        horizontalJustificationBox.setSelectedItemIndex (defaultHorizontalJustification);
        verticalJustificationBox  .setSelectedItemIndex (defaultVerticalJustification);
    }

    void setupJustificationOptions()
    {
        horizontalJustificationBox.addItemList (horizontalJustificationStrings, 1);
        verticalJustificationBox  .addItemList (verticalJustificationStrings, 1);

        auto updateJustification = [this]()
        {
            auto horizontalIndex = horizontalJustificationBox.getSelectedItemIndex();
            auto verticalIndex   = verticalJustificationBox.getSelectedItemIndex();

            auto horizontalJustification = horizontalJustificationFlags[horizontalIndex];
            auto verticalJustification   = verticalJustificationFlags[verticalIndex];

            demoTextBox.setJustification (horizontalJustification | verticalJustification);
        };

        horizontalJustificationBox.onChange = updateJustification;
        verticalJustificationBox  .onChange = updateJustification;
    }

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

        font.setUnderline (underlineToggle.getToggleState());

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
            styleBox.setSelectedItemIndex (defaultStyle);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontsDemo)
};
