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
class ProjucerLookAndFeel   : public LookAndFeel_V4
{
public:
    ProjucerLookAndFeel();
    ~ProjucerLookAndFeel() override;

    void drawTabButton (TabBarButton& button, Graphics&, bool isMouseOver, bool isMouseDown) override;
    int getTabButtonBestWidth (TabBarButton&, int tabDepth) override;
    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int, int) override {}

    void drawPropertyComponentBackground (Graphics&, int, int, PropertyComponent&) override {}
    void drawPropertyComponentLabel (Graphics&, int width, int height, PropertyComponent&) override;
    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&) override;

    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;
    void drawButtonText (Graphics&, TextButton&, bool isMouseOverButton, bool isButtonDown) override;
    void drawToggleButton (Graphics&, ToggleButton&, bool isMouseOverButton, bool isButtonDown) override;

    void drawTextEditorOutline (Graphics&, int, int, TextEditor&) override {}
    void fillTextEditorBackground (Graphics&, int width, int height, TextEditor&) override;

    void layoutFileBrowserComponent (FileBrowserComponent&, DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*, ComboBox* currentPathBox,
                                     TextEditor* filenameBox,Button* goUpButton) override;
    void drawFileBrowserRow (Graphics&, int width, int height, const File&, const String& filename, Image* icon,
                             const String& fileSizeDescription, const String& fileTimeDescription,
                             bool isDirectory, bool isItemSelected, int itemIndex, DirectoryContentsDisplayComponent&) override;

    void drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) override;

    void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;

    void drawMenuBarItem (Graphics&, int width, int height,
                          int itemIndex, const String& itemText,
                          bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                          MenuBarComponent&) override;

    void drawResizableFrame (Graphics&, int w, int h, const BorderSize<int>&) override;

    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       ComboBox&) override;

    void drawTreeviewPlusMinusBox (Graphics&, const Rectangle<float>& area,
                                   Colour backgroundColour, bool isItemOpen, bool isMouseOver) override;

    void drawProgressBar (Graphics&, ProgressBar&, int width, int height, double progress, const String& textToShow) override;

    //==============================================================================
    static Path getArrowPath (Rectangle<float> arrowZone, const int direction,
                              const bool filled, const Justification justification);
    static Path getChoiceComponentArrowPath (Rectangle<float> arrowZone);

    static Font getPropertyComponentFont()                                 { return { 14.0f, Font::FontStyleFlags::bold }; }
    static int getTextWidthForPropertyComponent (PropertyComponent* pp)    { return jmin (200, pp->getWidth() / 2); }

    //==============================================================================
    void setupColours();

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerLookAndFeel)
};
