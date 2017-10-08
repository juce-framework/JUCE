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

#pragma once


//==============================================================================
class SVGPathDataComponent  : public Component,
                              private TextEditor::Listener
{
public:
    SVGPathDataComponent()
        : desc (String(),
                "Paste an SVG path string into the top box, and it'll be converted to some C++ "
                "code that will load it as a Path object..")
    {
        desc.setJustificationType (Justification::centred);
        addAndMakeVisible (desc);

        userText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        userText.setMultiLine (true, true);
        userText.setReturnKeyStartsNewLine (true);
        addAndMakeVisible (userText);
        userText.addListener (this);

        resultText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        resultText.setMultiLine (true, true);
        resultText.setReadOnly (true);
        resultText.setSelectAllWhenFocused (true);
        addAndMakeVisible (resultText);

        userText.setText (getLastText());
    }

    void textEditorTextChanged (TextEditor&) override
    {
        update();
    }

    void textEditorEscapeKeyPressed (TextEditor&) override
    {
        getTopLevelComponent()->exitModalState (0);
    }

    void update()
    {
        getLastText() = userText.getText();

        path = Drawable::parseSVGPath (getLastText().trim().unquoted().trim());

        String result = "No path generated.. Not a valid SVG path string?";

        if (! path.isEmpty())
        {
            MemoryOutputStream data;
            path.writePathToStream (data);

            MemoryOutputStream out;

            out << "static const unsigned char pathData[] = ";
            CodeHelpers::writeDataAsCppLiteral (data.getMemoryBlock(), out, false, true);
            out << newLine
                << newLine
                << "Path path;" << newLine
                << "path.loadPathFromData (pathData, sizeof (pathData));" << newLine;

            result = out.toString();
        }

        resultText.setText (result, false);
        repaint (previewPathArea);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (8));
        desc.setBounds (r.removeFromTop (44));
        r.removeFromTop (8);
        userText.setBounds (r.removeFromTop (r.getHeight() / 2));
        r.removeFromTop (8);
        previewPathArea = r.removeFromRight (r.getHeight());
        resultText.setBounds (r);
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (secondaryBackgroundColourId));

        g.setColour (findColour (defaultTextColourId));
        g.fillPath (path, path.getTransformToScaleToFit (previewPathArea.reduced (4).toFloat(), true));
    }

    void lookAndFeelChanged() override
    {
        userText.applyFontToAllText (userText.getFont());
        resultText.applyFontToAllText (resultText.getFont());
    }

private:
    Label desc;
    TextEditor userText, resultText;
    Rectangle<int> previewPathArea;
    Path path;

    String& getLastText()
    {
        static String t;
        return t;
    }
};
