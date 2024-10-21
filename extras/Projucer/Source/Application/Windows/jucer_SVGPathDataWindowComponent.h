/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class SVGPathDataComponent final : public Component,
                                   public FileDragAndDropTarget

{
public:
    SVGPathDataComponent()
    {
        desc.setJustificationType (Justification::centred);
        addAndMakeVisible (desc);

        userText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        userText.setMultiLine (true, true);
        userText.setReturnKeyStartsNewLine (true);
        addAndMakeVisible (userText);
        userText.onTextChange = [this] { update(); };
        userText.onEscapeKey  = [this] { getTopLevelComponent()->exitModalState (0); };

        resultText.setFont (getAppSettings().appearance.getCodeFont().withHeight (13.0f));
        resultText.setMultiLine (true, true);
        resultText.setReadOnly (true);
        resultText.setSelectAllWhenFocused (true);
        addAndMakeVisible (resultText);

        userText.setText (getLastText());

        addAndMakeVisible (copyButton);
        copyButton.onClick = [this] { SystemClipboard::copyTextToClipboard (resultText.getText()); };

        addAndMakeVisible (closeSubPathButton);
        closeSubPathButton.onClick = [this] { update(); };
        closeSubPathButton.setToggleState (true, NotificationType::dontSendNotification);

        addAndMakeVisible (fillPathButton);
        fillPathButton.onClick = [this] { update(); };
        fillPathButton.setToggleState (true, NotificationType::dontSendNotification);
    }

    void update()
    {
        getLastText() = userText.getText();
        auto text = getLastText().trim().unquoted().trim();

        path = Drawable::parseSVGPath (text);

        if (path.isEmpty())
            path = pathFromPoints (text);

        String result = "No path generated.. Not a valid SVG path string?";

        if (! path.isEmpty())
        {
            MemoryOutputStream data;
            path.writePathToStream (data);

            MemoryOutputStream out;

            out << "static const unsigned char pathData[] = ";
            build_tools::writeDataAsCppLiteral (data.getMemoryBlock(), out, false, true);
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
        auto r = getLocalBounds().reduced (8);

        auto bottomSection = r.removeFromBottom (30);
        copyButton.setBounds (bottomSection.removeFromLeft (50));
        bottomSection.removeFromLeft (25);
        fillPathButton.setBounds (bottomSection.removeFromLeft (bottomSection.getWidth() / 2));
        closeSubPathButton.setBounds (bottomSection);

        r.removeFromBottom (5);
        desc.setBounds (r.removeFromTop (44));
        r.removeFromTop (8);
        userText.setBounds (r.removeFromTop (r.getHeight() / 2));
        r.removeFromTop (8);
        previewPathArea = r.removeFromRight (r.getHeight());
        resultText.setBounds (r);
    }

    void paint (Graphics& g) override
    {
        if (dragOver)
        {
            g.setColour (findColour (secondaryBackgroundColourId).brighter());
            g.fillAll();
        }

        g.setColour (findColour (defaultTextColourId));
        path.applyTransform (path.getTransformToScaleToFit (previewPathArea.reduced (4).toFloat(), true));

        if (fillPathButton.getToggleState())
            g.fillPath (path);
        else
            g.strokePath (path, PathStrokeType (2.0f));
    }

    void lookAndFeelChanged() override
    {
        userText.applyFontToAllText (userText.getFont());
        resultText.applyFontToAllText (resultText.getFont());
    }

    bool isInterestedInFileDrag (const StringArray& files) override
    {
        return files.size() == 1
                && File (files[0]).hasFileExtension  ("svg");
    }

    void fileDragEnter (const StringArray&, int, int) override
    {
        dragOver = true;
        repaint();
    }

    void fileDragExit (const StringArray&) override
    {
        dragOver = false;
        repaint();
    }

    void filesDropped (const StringArray& files, int, int) override
    {
        dragOver = false;
        repaint();

        if (auto element = parseXML (File (files[0])))
        {
            if (auto* ePath = element->getChildByName ("path"))
                userText.setText (ePath->getStringAttribute ("d"), true);
            else if (auto* ePolygon = element->getChildByName ("polygon"))
                userText.setText (ePolygon->getStringAttribute ("points"), true);
        }
    }

    Path pathFromPoints (String pointsText)
    {
        auto points = StringArray::fromTokens (pointsText, " ,", "");
        points.removeEmptyStrings();

        jassert (points.size() % 2 == 0);

        Path p;

        for (int i = 0; i < points.size() / 2; i++)
        {
            auto x = points[i * 2].getFloatValue();
            auto y = points[i * 2 + 1].getFloatValue();

            if (i == 0)
                p.startNewSubPath ({ x, y });
            else
                p.lineTo ({ x, y });
        }

        if (closeSubPathButton.getToggleState())
            p.closeSubPath();

        return p;
    }

private:
    Label desc { {}, "Paste an SVG path string into the top box, and it'll be converted to some C++ "
                     "code that will load it as a Path object.." };
    TextButton copyButton { "Copy" };
    TextEditor userText, resultText;

    ToggleButton closeSubPathButton { "Close sub-path" };
    ToggleButton fillPathButton     { "Fill path" };

    Rectangle<int> previewPathArea;
    Path path;
    bool dragOver = false;

    String& getLastText()
    {
        static String t;
        return t;
    }
};
