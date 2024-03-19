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
class ProjucerLookAndFeel : public LookAndFeel_V4
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

    ProgressBar::Style getDefaultProgressBarStyle (const ProgressBar&) override;

    //==============================================================================
    static Path getArrowPath (Rectangle<float> arrowZone, int direction,
                              bool filled, Justification justification);
    static Path getChoiceComponentArrowPath (Rectangle<float> arrowZone);

    static Font getPropertyComponentFont()                                       { return FontOptions { 14.0f, Font::FontStyleFlags::bold }; }
    static int getTextWidthForPropertyComponent (const PropertyComponent& pc)    { return jmin (200, pc.getWidth() / 2); }

    static ColourScheme getProjucerDarkColourScheme()
    {
        return { 0xff323e44, 0xff263238, 0xff323e44,
                 0xff8e989b, 0xffffffff, 0xffa45c94,
                 0xffffffff, 0xff181f22, 0xffffffff };
    }

    //==============================================================================
    void setupColours();

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjucerLookAndFeel)
};
