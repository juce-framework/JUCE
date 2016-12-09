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

class SVGPathDataComponent  : public Component,
                              private TextEditorListener
{
public:
    SVGPathDataComponent()
        : desc (String(),
                "Paste an SVG path string into the top box, and it'll be converted to some C++ "
                "code that will load it as a Path object..")
    {
        desc.setJustificationType (Justification::centred);
        desc.setColour (Label::textColourId, Colours::white);
        addAndMakeVisible (desc);

        const Colour bkgd (Colours::white.withAlpha (0.6f));

        userText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        userText.setMultiLine (true, true);
        userText.setReturnKeyStartsNewLine (true);
        userText.setColour (TextEditor::backgroundColourId, bkgd);
        addAndMakeVisible (userText);
        userText.addListener (this);

        resultText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        resultText.setMultiLine (true, true);
        resultText.setColour (TextEditor::backgroundColourId, bkgd);
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
        g.setColour (Colours::white);
        g.fillPath (path, path.getTransformToScaleToFit (previewPathArea.reduced (4).toFloat(), true));
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
