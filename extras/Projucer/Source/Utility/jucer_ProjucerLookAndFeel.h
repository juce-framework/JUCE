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

#pragma once


//==============================================================================
class ProjucerLookAndFeel   : public LookAndFeel_V4
{
public:
    ProjucerLookAndFeel();

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
    Font getTextButtonFont (TextButton&, int buttonHeight) override;

    void drawTextEditorOutline (Graphics&, int, int, TextEditor&) override {}
    void fillTextEditorBackground (Graphics&, int width, int height, TextEditor&) override;

    void layoutFileBrowserComponent (FileBrowserComponent&, DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*, ComboBox* currentPathBox,
                                     TextEditor* filenameBox,Button* goUpButton) override;
    void drawFileBrowserRow (Graphics&, int width, int height, const String& filename, Image* icon,
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
