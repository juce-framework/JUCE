/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"


//==============================================================================
class FontsAndTextDemo  : public Component,
                          public ListBoxModel,
                          public ButtonListener,
                          public SliderListener
{
    OwnedArray <Font> fonts;
    ListBox* listBox;
    TextEditor* textBox;
    ToggleButton* boldButton;
    ToggleButton* italicButton;
    Slider* sizeSlider;
    Slider* kerningSlider;
    Slider* horizontalScaleSlider;

    StretchableLayoutManager verticalLayout;
    StretchableLayoutManager horizontalLayout;

    StretchableLayoutResizerBar* verticalDividerBar;

public:
    //==============================================================================
    FontsAndTextDemo()
    {
        setName (T("Fonts"));

        Font::findFonts (fonts);

        addAndMakeVisible (listBox = new ListBox (T("fonts"), this));
        listBox->setRowHeight (28);

        addAndMakeVisible (textBox = new TextEditor());

        textBox->setColour (TextEditor::backgroundColourId, Colours::white);
        textBox->setColour (TextEditor::outlineColourId, Colours::black.withAlpha (0.5f));

        textBox->setMultiLine (true, true);
        textBox->setReturnKeyStartsNewLine (true);
        textBox->setText (T("The Quick Brown Fox Jumps Over The Lazy Dog\n\nAa Bb Cc Dd Ee Ff Gg Hh Ii Jj Kk Ll Mm Nn Oo Pp Qq Rr Ss Tt Uu Vv Ww Xx Yy Zz 0123456789"));

        addAndMakeVisible (boldButton = new ToggleButton (T("bold")));
        boldButton->addButtonListener (this);

        addAndMakeVisible (italicButton = new ToggleButton (T("italic")));
        italicButton->addButtonListener (this);

        addAndMakeVisible (sizeSlider = new Slider ("Size"));
        sizeSlider->setRange (3.0, 150.0, 0.1);
        sizeSlider->setValue (20.0);
        sizeSlider->addListener (this);
        (new Label (String::empty, sizeSlider->getName()))->attachToComponent (sizeSlider, true);

        addAndMakeVisible (kerningSlider = new Slider ("Kerning"));
        kerningSlider->setRange (-1.0, 1.0, 0.01);
        kerningSlider->setValue (0.0);
        kerningSlider->addListener (this);
        (new Label (String::empty, kerningSlider->getName()))->attachToComponent (kerningSlider, true);

        addAndMakeVisible (horizontalScaleSlider = new Slider ("Stretch"));
        horizontalScaleSlider->setRange (0.1, 4.0, 0.01);
        horizontalScaleSlider->setValue (1.0);
        horizontalScaleSlider->addListener (this);
        (new Label (String::empty, horizontalScaleSlider->getName()))->attachToComponent (horizontalScaleSlider, true);

        for (int i = 0; i < fonts.size(); ++i)
        {
            if (fonts[i]->getTypefaceName().startsWithIgnoreCase (T("Arial")))
            {
                listBox->selectRow (i);
                break;
            }
        }

        listBox->setColour (ListBox::outlineColourId, Colours::black.withAlpha (0.5f));
        listBox->setOutlineThickness (1);

        // set up the layout and resizer bars..

        verticalLayout.setItemLayout (0, -0.2, -0.8, -0.5);  // width of the font list must be
                                                             // between 20% and 80%, preferably 50%
        verticalLayout.setItemLayout (1, 8, 8, 8);           // the vertical divider drag-bar thing is always 8 pixels wide
        verticalLayout.setItemLayout (2, 150, -1.0, -0.5);   // the components on the right must be
                                                             // at least 150 pixels wide, preferably 50% of the total width

        verticalDividerBar = new StretchableLayoutResizerBar (&verticalLayout, 1, true);
        addAndMakeVisible (verticalDividerBar);

        horizontalLayout.setItemLayout (0, -0.2, -1.0, -0.4);  // height of the font text box must be
                                                               // between 20% and 100%, preferably 40%
        horizontalLayout.setItemLayout (1, 8, 8, 8);           // the horizontal divider drag-bar thing is always 8 pixels high
        horizontalLayout.setItemLayout (2, 2, 5, 5);           // a gap between the controls
        horizontalLayout.setItemLayout (3, 15, 20, 20);        // the italic button would like to be 20 pixels high
        horizontalLayout.setItemLayout (4, 2, 5, 5);           // a gap between the controls
        horizontalLayout.setItemLayout (5, 15, 20, 20);        // the bold button would like to be 20 pixels high
        horizontalLayout.setItemLayout (6, 2, 5, 5);           // a gap between the controls
        horizontalLayout.setItemLayout (7, 15, 20, 20);        // the italic button would like to be 20 pixels high
        horizontalLayout.setItemLayout (8, 2, 5, 5);           // a gap between the controls
        horizontalLayout.setItemLayout (9, 15, 20, 20);        // the copy code button would like to be 20 pixels high
        horizontalLayout.setItemLayout (10, 5, -1.0, 5);        // add a gap at the bottom that will fill up any
                                                               // space left over - this will stop the
                                                               // sliders from always sticking to the
                                                               // bottom of the window
    }

    ~FontsAndTextDemo()
    {
        deleteAllChildren();
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
        textBox->setBounds (x, 0, getWidth() - x, getHeight() - 110);
        x += 70;
        sizeSlider->setBounds (x, getHeight() - 106, getWidth() - x, 22);
        kerningSlider->setBounds (x, getHeight() - 82, getWidth() - x, 22);
        horizontalScaleSlider->setBounds (x, getHeight() - 58, getWidth() - x, 22);
        boldButton->setBounds (x, getHeight() - 34, (getWidth() - x) / 2, 22);
        italicButton->setBounds (x + (getWidth() - x) / 2, getHeight() - 34, (getWidth() - x) / 2, 22);
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

        if (fonts [rowNumber] != 0)
        {
            Font font (*fonts [rowNumber]);
            font.setHeight (height * 0.7f);

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
    }

    void updatePreviewBoxText()
    {
        Font* f = fonts [listBox->getSelectedRow()];

        if (f != 0)
        {
            Font font (*f);

            font.setHeight ((float) sizeSlider->getValue());
            font.setBold (boldButton->getToggleState());
            font.setItalic (italicButton->getToggleState());
            font.setExtraKerningFactor ((float) kerningSlider->getValue());
            font.setHorizontalScale ((float) horizontalScaleSlider->getValue());

            textBox->applyFontToAllText (font);
        }
    }

    void selectedRowsChanged (int lastRowselected)
    {
        updatePreviewBoxText();
    }

    void buttonClicked (Button* button)
    {
        updatePreviewBoxText();
    }

    void sliderValueChanged (Slider*)
    {
        // (this is called when the size slider is moved)
        updatePreviewBoxText();
    }
};


//==============================================================================
Component* createFontsAndTextDemo()
{
    return new FontsAndTextDemo();
}
