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

#pragma once


namespace juce
{

class FontListComponent final : public Component,
                                private ListBoxModel
{
public:
    FontListComponent()
    {
        listBox.setTitle ("Fonts");
        listBox.setRowHeight (20);
        listBox.setColour (ListBox::textColourId, Colours::black);
        listBox.setColour (ListBox::backgroundColourId, Colours::white);
        listBox.setModel (this);

        addAndMakeVisible (listBox);
    }

    std::function<void()> onFontSelected;

    template <typename Filter>
    void rescanAndRemoveIf (Filter&& filter)
    {
        fonts.clear();
        Font::findFonts (fonts);

        if (filter != nullptr)
            fonts.removeIf (std::move (filter));

        listBox.updateContent();
    }

    void rescan()
    {
        rescanAndRemoveIf ([] (const Font&) { return false; });
        listBox.updateContent();
    }

    void selectRow (int row)
    {
        listBox.selectRow (row);
    }

    int getNumFonts() const
    {
        return fonts.size();
    }

    int getSelectedRow() const
    {
        return listBox.getSelectedRow();
    }

    [[nodiscard]] Typeface::Ptr getFaceForRow (int rowNumber) const
    {
        return fonts.getReference (rowNumber).getTypefacePtr();
    }

    [[nodiscard]] Font getFontForRow (int rowNumber) const
    {
        return fonts.getReference (rowNumber);
    }

    void resized() override
    {
        listBox.setBounds (getLocalBounds());
    }

private:
    int getNumRows() override
    {
        return fonts.size();
    }

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width,
                           int height,
                           bool rowIsSelected) override
    {
        if (rowNumber >= getNumRows())
            return;

        if (rowIsSelected)
            g.fillAll (Colours::lightblue);

        const Font options { FontOptions { getFaceForRow (rowNumber) } };

        AttributedString s;
        s.setWordWrap (AttributedString::none);
        s.setJustification (Justification::centredLeft);
        s.append (getNameForRow (rowNumber),
                  options.withPointHeight ((float) height * 0.7f),
                  Colours::black);

        s.append ("   " + getNameForRow (rowNumber),
                  FontOptions{}.withPointHeight ((float) height * 0.5f).withStyle ("Italic"),
                  Colours::grey);

        s.draw (g, Rectangle (width, height).expanded (-4, 50).toFloat());
    }

    void selectedRowsChanged (int) override
    {
        NullCheckedInvocation::invoke (onFontSelected);
    }

    String getNameForRow (int rowNumber) override
    {
        return fonts.getReference (rowNumber).getTypefaceName();
    }

    ListBox listBox;
    Array<Font> fonts;
};

} // namespace juce
