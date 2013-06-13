/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"


//==============================================================================
class FontsAndTextDemo  : public Component,
                          public ListBoxModel,
                          public ComboBoxListener,
                          public SliderListener
{
public:
    //==============================================================================
    FontsAndTextDemo()
        : sizeLabel (String::empty, "Size:"),
          kerningLabel (String::empty, "Kerning:"),
          horizontalScaleLabel (String::empty, "Scale:"),
          styleLabel (String::empty, "Style:")
    {
        setName ("Fonts");

        Font::findFonts (fonts);

        addAndMakeVisible (listBox = new ListBox ("fonts", this));
        listBox->setRowHeight (28);

        addAndMakeVisible (&textBox);

        textBox.setColour (TextEditor::backgroundColourId, Colours::white);
        textBox.setColour (TextEditor::outlineColourId, Colours::black.withAlpha (0.5f));

        textBox.setMultiLine (true, true);
        textBox.setReturnKeyStartsNewLine (true);
        textBox.setText ("The Quick Brown Fox Jumps Over The Lazy Dog\n\n"
                         "Aa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz 0123456789");

        addAndMakeVisible (&fontStylesComboBox);
        fontStylesComboBox.addListener (this);
        styleLabel.attachToComponent (&fontStylesComboBox, true);

        addAndMakeVisible (&sizeSlider);
        sizeSlider.setRange (3.0, 150.0, 0.1);
        sizeSlider.setValue (20.0);
        sizeSlider.addListener (this);
        sizeLabel.attachToComponent (&sizeSlider, true);

        addAndMakeVisible (&kerningSlider);
        kerningSlider.setRange (-1.0, 1.0, 0.01);
        kerningSlider.setValue (0.0);
        kerningSlider.addListener (this);
        kerningLabel.attachToComponent (&kerningSlider, true);

        addAndMakeVisible (&horizontalScaleSlider);
        horizontalScaleSlider.setRange (0.1, 4.0, 0.01);
        horizontalScaleSlider.setValue (1.0);
        horizontalScaleSlider.addListener (this);
        horizontalScaleLabel.attachToComponent (&horizontalScaleSlider, true);

        listBox->setColour (ListBox::outlineColourId, Colours::black.withAlpha (0.5f));
        listBox->setOutlineThickness (1);
        listBox->selectRow (Random::getSystemRandom().nextInt (fonts.size()));

        // set up the layout and resizer bars..

        verticalLayout.setItemLayout (0, -0.2, -0.8, -0.5);  // width of the font list must be
                                                             // between 20% and 80%, preferably 50%
        verticalLayout.setItemLayout (1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
        verticalLayout.setItemLayout (2, 150, -1.0, -0.5);   // the components on the right must be
                                                             // at least 150 pixels wide, preferably 50% of the total width

        verticalDividerBar = new StretchableLayoutResizerBar (&verticalLayout, 1, true);
        addAndMakeVisible (verticalDividerBar);
    }

    void resized()
    {
        // lay out the list box and vertical divider..
        Component* vcomps[] = { listBox, verticalDividerBar, 0 };

        verticalLayout.layOutComponents (vcomps, 3,
                                         4, 4, getWidth() - 8, getHeight() - 8,
                                         false,     // lay out side-by-side
                                         true);     // resize the components' heights as well as widths

        // now lay out the text box and the controls below it..
        int x = verticalLayout.getItemCurrentPosition (2) + 4;
        textBox.setBounds (x, 0, getWidth() - x, getHeight() - 110);
        x += 70;
        sizeSlider.setBounds (x, getHeight() - 106, getWidth() - x, 22);
        kerningSlider.setBounds (x, getHeight() - 82, getWidth() - x, 22);
        horizontalScaleSlider.setBounds (x, getHeight() - 58, getWidth() - x, 22);
        fontStylesComboBox.setBounds (x, getHeight() - 34, (getWidth() - x) / 2, 22);
    }

    // implements the ListBoxModel method
    int getNumRows()
    {
        return fonts.size();
    }

    // implements the ListBoxModel method
    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);

        Font font = fonts[rowNumber].withPointHeight (height * 0.6f);

        g.setFont (font);
        g.setColour (Colours::black);
        g.drawText (font.getTypefaceName(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);

        int x = jmax (0, font.getStringWidth (font.getTypefaceName())) + 12;
        g.setFont (Font (11.0f, Font::italic));
        g.setColour (Colours::grey);
        g.drawText (font.getTypefaceName(),
                    x, 0, width - x - 2, height,
                    Justification::centredLeft, true);
    }

    void updatePreviewBoxText()
    {
        Font font (fonts [listBox->getSelectedRow()]);

        font.setHeight ((float) sizeSlider.getValue());
        font.setExtraKerningFactor ((float) kerningSlider.getValue());
        font.setHorizontalScale ((float) horizontalScaleSlider.getValue());

        updateStylesList (font);
        font.setTypefaceStyle (fontStylesComboBox.getText());

        textBox.applyFontToAllText (font);
    }

    void updateStylesList (const Font& newFont)
    {
        const StringArray newStyles (newFont.getAvailableStyles());

        if (newStyles != currentStyleList)
        {
            currentStyleList = newStyles;

            fontStylesComboBox.clear();

            for (int i = 0; i < newStyles.size(); ++i)
                fontStylesComboBox.addItem (newStyles[i], i + 1);

            fontStylesComboBox.setSelectedItemIndex (0);
        }
    }

    void selectedRowsChanged (int /*lastRowselected*/)
    {
        updatePreviewBoxText();
    }

    void buttonClicked (Button*)
    {
        updatePreviewBoxText();
    }

    void sliderValueChanged (Slider*)
    {
        updatePreviewBoxText();
    }

    void comboBoxChanged (ComboBox*)
    {
        updatePreviewBoxText();
    }

private:
    Array<Font> fonts;
    StringArray currentStyleList;

    ScopedPointer<ListBox> listBox;
    TextEditor textBox;
    ComboBox fontStylesComboBox;
    Slider sizeSlider, kerningSlider, horizontalScaleSlider;
    Label sizeLabel, kerningLabel, horizontalScaleLabel, styleLabel;

    StretchableLayoutManager verticalLayout;
    ScopedPointer<StretchableLayoutResizerBar> verticalDividerBar;
};


//==============================================================================
Component* createFontsAndTextDemo()
{
    return new FontsAndTextDemo();
}
