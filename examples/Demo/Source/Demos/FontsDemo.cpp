/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"


//==============================================================================
class FontsDemo  : public Component,
                   private ListBoxModel,
                   private Slider::Listener,
                   private Button::Listener,
                   private ComboBox::Listener
{
public:
    FontsDemo()
        : heightLabel (String(), "Height:"),
          kerningLabel (String(), "Kerning:"),
          scaleLabel  (String(), "Scale:"),
          styleLabel ("Style"),
          boldToggle ("Bold"),
          italicToggle ("Italic")
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
        heightLabel.attachToComponent (&heightSlider, true);
        scaleLabel.attachToComponent (&scaleSlider, true);
        styleLabel.attachToComponent (&styleBox, true);

        heightSlider.addListener (this);
        kerningSlider.addListener (this);
        scaleSlider.addListener (this);
        boldToggle.addListener (this);
        italicToggle.addListener (this);
        styleBox.addListener (this);

        Font::findFonts (fonts);   // Generate the list of fonts

        listBox.setRowHeight (20);
        listBox.setModel (this);   // Tell the listbox where to get its data model

        heightSlider.setRange (3.0, 150.0, 0.01);
        scaleSlider.setRange (0.2, 3.0, 0.01);
        kerningSlider.setRange (-2.0, 2.0, 0.01);

        scaleSlider.setValue (1.0);   // Set some initial values for the sliders.
        heightSlider.setValue (20.0);
        kerningSlider.setValue (0);

        // set up the layout and resizer bars..
        verticalLayout.setItemLayout (0, -0.2, -0.8, -0.35); // width of the font list must be
                                                             // between 20% and 80%, preferably 50%
        verticalLayout.setItemLayout (1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
        verticalLayout.setItemLayout (2, 150, -1.0, -0.65);  // the components on the right must be
                                                             // at least 150 pixels wide, preferably 50% of the total width

        verticalDividerBar = new StretchableLayoutResizerBar (&verticalLayout, 1, true);
        addAndMakeVisible (verticalDividerBar);

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
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (5));

        // lay out the list box and vertical divider..
        Component* vcomps[] = { &listBox, verticalDividerBar, nullptr };

        verticalLayout.layOutComponents (vcomps, 3,
                                         r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                         false,     // lay out side-by-side
                                         true);     // resize the components' heights as well as widths


        r.removeFromLeft (verticalDividerBar->getRight());

        styleBox.setBounds (r.removeFromBottom (26));
        r.removeFromBottom (8);

        const int labelWidth = 60;

        Rectangle<int> row (r.removeFromBottom (30));
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

    void buttonClicked (Button* buttonThatWasClicked) override
    {
        if (buttonThatWasClicked == &boldToggle)            refreshPreviewBoxFont();
        else if (buttonThatWasClicked == &italicToggle)     refreshPreviewBoxFont();
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

        Font font (fonts [rowNumber]);

        AttributedString s;
        s.setWordWrap (AttributedString::none);
        s.setJustification (Justification::centredLeft);
        s.append (font.getTypefaceName(), font.withPointHeight (height * 0.7f), Colours::black);
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
    Label heightLabel, kerningLabel, scaleLabel, styleLabel;
    Slider heightSlider, kerningSlider, scaleSlider;
    ToggleButton boldToggle, italicToggle;
    ComboBox styleBox;

    StretchableLayoutManager verticalLayout;
    ScopedPointer<StretchableLayoutResizerBar> verticalDividerBar;

    void refreshPreviewBoxFont()
    {
        const bool bold = boldToggle.getToggleState();
        const bool italic = italicToggle.getToggleState();
        const bool useStyle = ! (bold || italic);

        Font font (fonts [listBox.getSelectedRow()]);

        font = font.withPointHeight ((float) heightSlider.getValue())
                   .withExtraKerningFactor ((float) kerningSlider.getValue())
                   .withHorizontalScale ((float) scaleSlider.getValue());

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
        const StringArray newStyles (newFont.getAvailableStyles());

        if (newStyles != currentStyleList)
        {
            currentStyleList = newStyles;

            styleBox.clear();
            styleBox.addItemList (newStyles, 1);
            styleBox.setSelectedItemIndex (0);
        }
    }

    void comboBoxChanged (ComboBox* box) override
    {
        if (box == &styleBox)
            refreshPreviewBoxFont();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<FontsDemo> demo ("20 Graphics: Fonts");
