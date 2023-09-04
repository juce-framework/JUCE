/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_ProjucerLookAndFeel.h"

#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
ProjucerLookAndFeel::ProjucerLookAndFeel()
{
    setupColours();
}

ProjucerLookAndFeel::~ProjucerLookAndFeel() {}

void ProjucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const auto area = button.getActiveArea();
    auto backgroundColour = findColour (button.isFrontTab() ? secondaryBackgroundColourId
                                                            : inactiveTabBackgroundColourId);

    g.setColour (backgroundColour);
    g.fillRect (area);

    const auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    auto textColour = findColour (defaultTextColourId).withMultipliedAlpha (alpha);

    auto iconColour = findColour (button.isFrontTab() ? activeTabIconColourId
                                                      : inactiveTabIconColourId);

    auto isProjectTab = button.getName() == ProjectContentComponent::getProjectTabName();

    if (isProjectTab)
    {
        auto icon = Icon (getIcons().closedFolder,
                          iconColour.withMultipliedAlpha (alpha));

        auto isSingleTab = (button.getTabbedButtonBar().getNumTabs() == 1);

        if (isSingleTab)
        {
            auto activeArea = button.getActiveArea().reduced (5);

            activeArea.removeFromLeft (15);
            icon.draw (g, activeArea.removeFromLeft (activeArea.getHeight()).toFloat(), false);
            activeArea.removeFromLeft (10);

            g.setColour (textColour);
            g.drawFittedText (ProjectContentComponent::getProjectTabName(),
                              activeArea, Justification::centredLeft, 1);
        }
        else
        {
            icon.draw (g, button.getTextArea().reduced (8, 8).toFloat(), false);
        }
    }
    else
    {
        TextLayout textLayout;
        LookAndFeel_V3::createTabTextLayout (button, (float) area.getWidth(), (float) area.getHeight(), textColour, textLayout);

        textLayout.draw (g, button.getTextArea().toFloat());
    }
}

int ProjucerLookAndFeel::getTabButtonBestWidth (TabBarButton& button, int)
{
    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
        return bar->getWidth() / bar->getNumTabs();

    return 120;
}

void ProjucerLookAndFeel::drawPropertyComponentLabel (Graphics& g, int, int height, PropertyComponent& component)
{
    g.setColour (component.findColour (defaultTextColourId)
                          .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    auto textWidth = getTextWidthForPropertyComponent (component);

    g.setFont (getPropertyComponentFont());
    g.drawFittedText (component.getName(), 0, 0, textWidth, height, Justification::centredLeft, 5, 1.0f);
}

Rectangle<int> ProjucerLookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    const auto paddedTextW = getTextWidthForPropertyComponent (component) + 5;
    return { paddedTextW , 0, component.getWidth() - paddedTextW, component.getHeight() - 1 };
}

void ProjucerLookAndFeel::drawButtonBackground (Graphics& g,
                                                Button& button,
                                                const Colour& backgroundColour,
                                                bool isMouseOverButton,
                                                bool isButtonDown)
{
    const auto cornerSize = button.findParentComponentOfClass<PropertyComponent>() != nullptr ? 0.0f : 3.0f;
    const auto bounds = button.getLocalBounds().toFloat();

    auto baseColour = backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (isButtonDown || isMouseOverButton)
        baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.05f);

    g.setColour (baseColour);

    if (button.isConnectedOnLeft() || button.isConnectedOnRight())
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! button.isConnectedOnLeft(),
                                  ! button.isConnectedOnRight(),
                                  ! button.isConnectedOnLeft(),
                                  ! button.isConnectedOnRight());

        g.fillPath (path);
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);
    }
}

void ProjucerLookAndFeel::drawButtonText (Graphics& g, TextButton& button, bool isMouseOverButton, bool isButtonDown)
{
    ignoreUnused (isMouseOverButton, isButtonDown);

    g.setFont (getTextButtonFont (button, button.getHeight()));

    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                                   .withMultipliedAlpha (button.isEnabled() ? 1.0f
                                                                            : 0.5f));

    auto xIndent = jmin (8, button.getWidth() / 10);
    auto yIndent = jmin (3,  button.getHeight() / 6);

    auto textBounds = button.getLocalBounds().reduced (xIndent, yIndent);

    g.drawFittedText (button.getButtonText(), textBounds, Justification::centred, 3, 1.0f);
}

void ProjucerLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown)
{
    ignoreUnused (isMouseOverButton, isButtonDown);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    bool isTextEmpty = button.getButtonText().isEmpty();
    bool isPropertyComponentChild = (dynamic_cast<BooleanPropertyComponent*> (button.getParentComponent()) != nullptr
                                     || dynamic_cast<MultiChoicePropertyComponent*> (button.getParentComponent()) != nullptr);

    auto bounds = button.getLocalBounds();

    auto sideLength = isPropertyComponentChild ? 25 : bounds.getHeight();

    auto rectBounds = isTextEmpty ? bounds
                                  : bounds.removeFromLeft (jmin (sideLength, bounds.getWidth() / 3));

    rectBounds = rectBounds.withSizeKeepingCentre (sideLength, sideLength).reduced (4);

    g.setColour (button.findColour (ToggleButton::tickDisabledColourId));
    g.drawRoundedRectangle (rectBounds.toFloat(), 2.0f, 1.0f);

    if (button.getToggleState())
    {
        g.setColour (button.findColour (ToggleButton::tickColourId));
        const auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (rectBounds.reduced (2).toFloat(), false));
    }

    if (! isTextEmpty)
    {
        bounds.removeFromLeft (5);

        const auto fontSize = jmin (15.0f, (float) button.getHeight() * 0.75f);

        g.setFont (fontSize);
        g.setColour (isPropertyComponentChild ? findColour (widgetTextColourId)
                                              : button.findColour (ToggleButton::textColourId));

        g.drawFittedText (button.getButtonText(), bounds, Justification::centredLeft, 2);
    }
}

void ProjucerLookAndFeel::fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor)
{
    g.setColour (textEditor.findColour (TextEditor::backgroundColourId));
    g.fillRect (0, 0, width, height);

    g.setColour (textEditor.findColour (TextEditor::outlineColourId));
    g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
}

void ProjucerLookAndFeel::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                      DirectoryContentsDisplayComponent* fileListComponent,
                                                      FilePreviewComponent* previewComp,
                                                      ComboBox* currentPathBox,
                                                      TextEditor* filenameBox,
                                                      Button* goUpButton)
{
    const auto sectionHeight = 22;
    const auto buttonWidth = 50;

    auto b = browserComp.getLocalBounds().reduced (20, 5);

    auto topSlice    = b.removeFromTop (sectionHeight);
    auto bottomSlice = b.removeFromBottom (sectionHeight);

    currentPathBox->setBounds (topSlice.removeFromLeft (topSlice.getWidth() - buttonWidth));
    currentPathBox->setColour (ComboBox::backgroundColourId,    findColour (backgroundColourId));
    currentPathBox->setColour (ComboBox::textColourId,          findColour (defaultTextColourId));
    currentPathBox->setColour (ComboBox::arrowColourId,         findColour (defaultTextColourId));

    topSlice.removeFromLeft (6);
    goUpButton->setBounds (topSlice);

    bottomSlice.removeFromLeft (50);
    filenameBox->setBounds (bottomSlice);
    filenameBox->setColour (TextEditor::backgroundColourId, findColour (backgroundColourId));
    filenameBox->setColour (TextEditor::textColourId,       findColour (defaultTextColourId));
    filenameBox->setColour (TextEditor::outlineColourId,    findColour (defaultTextColourId));
    filenameBox->applyFontToAllText (filenameBox->getFont());

    if (previewComp != nullptr)
        previewComp->setBounds (b.removeFromRight (b.getWidth() / 3));

    if (auto listAsComp = dynamic_cast<Component*> (fileListComponent))
        listAsComp->setBounds (b.reduced (0, 10));
}

void ProjucerLookAndFeel::drawFileBrowserRow (Graphics& g, int width, int height,
                                              const File& file, const String& filename, Image* icon,
                                              const String& fileSizeDescription,
                                              const String& fileTimeDescription,
                                              bool isDirectory, bool isItemSelected,
                                              int itemIndex, DirectoryContentsDisplayComponent& dcc)
{
    if (auto fileListComp = dynamic_cast<Component*> (&dcc))
    {
        fileListComp->setColour (DirectoryContentsDisplayComponent::textColourId,
                                 findColour (isItemSelected ? defaultHighlightedTextColourId : defaultTextColourId));

        fileListComp->setColour (DirectoryContentsDisplayComponent::highlightColourId,
                                 findColour (defaultHighlightColourId).withAlpha (0.75f));
    }


    LookAndFeel_V2::drawFileBrowserRow (g, width, height, file, filename, icon,
                                        fileSizeDescription, fileTimeDescription,
                                        isDirectory, isItemSelected, itemIndex, dcc);
}

void ProjucerLookAndFeel::drawCallOutBoxBackground (CallOutBox&, Graphics& g, const Path& path, Image&)
{
    g.setColour (findColour (secondaryBackgroundColourId));
    g.fillPath (path);

    g.setColour (findColour (userButtonBackgroundColourId));
    g.strokePath (path, PathStrokeType (2.0f));
}

void ProjucerLookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height,
                                                 bool, MenuBarComponent& menuBar)
{
    const auto colour = menuBar.findColour (backgroundColourId).withAlpha (0.75f);

    Rectangle<int> r (width, height);

    g.setColour (colour.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColourGradient (colour, 0, 0, colour.darker (0.2f), 0, (float)height, false));
    g.fillRect (r);
}

void ProjucerLookAndFeel::drawMenuBarItem (Graphics& g, int width, int height,
                                           int itemIndex, const String& itemText,
                                           bool isMouseOverItem, bool isMenuOpen,
                                           bool /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColour (menuBar.findColour (defaultTextColourId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColour (defaultHighlightColourId).withAlpha (0.75f));
        g.setColour (menuBar.findColour (defaultHighlightedTextColourId));
    }
    else
    {
        g.setColour (menuBar.findColour (defaultTextColourId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centred, 1);
}

void ProjucerLookAndFeel::drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border)
{
    ignoreUnused (g, w, h, border);
}

void ProjucerLookAndFeel::drawComboBox (Graphics& g, int width, int height, bool,
                                        int, int, int, int, ComboBox& box)
{
    const auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 1.5f;
    Rectangle<int> boxBounds (0, 0, width, height);

    auto isChoiceCompChild = (box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr);

    if (isChoiceCompChild)
    {
        box.setColour (ComboBox::textColourId, findColour (widgetTextColourId));

        g.setColour (findColour (widgetBackgroundColourId));
        g.fillRect (boxBounds);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).reduced (0, 2).toFloat();
        g.setColour (Colours::black);
        g.fillPath (getChoiceComponentArrowPath (arrowZone));
    }
    else
    {
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).toFloat();
        g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.fillPath (getArrowPath (arrowZone, 2, true, Justification::centred));

    }
}

void ProjucerLookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                                    Colour, bool isOpen, bool /**isMouseOver*/)
{
    g.strokePath (getArrowPath (area, isOpen ? 2 : 1, false, Justification::centredRight), PathStrokeType (2.0f));
}

ProgressBar::Style ProjucerLookAndFeel::getDefaultProgressBarStyle (const ProgressBar&)
{
    return ProgressBar::Style::circular;
}

//==============================================================================
Path ProjucerLookAndFeel::getArrowPath (Rectangle<float> arrowZone, const int direction,
                                        bool filled, const Justification justification)
{
    auto w = jmin (arrowZone.getWidth(),  (direction == 0 || direction == 2) ? 8.0f : filled ? 5.0f : 8.0f);
    auto h = jmin (arrowZone.getHeight(), (direction == 0 || direction == 2) ? 5.0f : filled ? 8.0f : 5.0f);

    if (justification == Justification::centred)
    {
        arrowZone.reduce ((arrowZone.getWidth() - w) / 2, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centredRight)
    {
        arrowZone.removeFromLeft (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centredLeft)
    {
        arrowZone.removeFromRight (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else
    {
        jassertfalse; // currently only supports centred justifications
    }

    Path path;
    path.startNewSubPath (arrowZone.getX(), arrowZone.getBottom());
    path.lineTo (arrowZone.getCentreX(), arrowZone.getY());
    path.lineTo (arrowZone.getRight(), arrowZone.getBottom());

    if (filled)
        path.closeSubPath();

    path.applyTransform (AffineTransform::rotation ((float) direction * MathConstants<float>::halfPi,
                                                    arrowZone.getCentreX(), arrowZone.getCentreY()));

    return path;
}

Path ProjucerLookAndFeel::getChoiceComponentArrowPath (Rectangle<float> arrowZone)
{
    auto topBounds = arrowZone.removeFromTop (arrowZone.getHeight() * 0.5f);
    auto bottomBounds = arrowZone;

    auto topArrow = getArrowPath (topBounds, 0, true, Justification::centred);
    auto bottomArrow = getArrowPath (bottomBounds, 2, true, Justification::centred);

    topArrow.addPath (bottomArrow);

    return topArrow;
}

//==============================================================================
void ProjucerLookAndFeel::setupColours()
{
    auto& colourScheme = getCurrentColourScheme();

    if (colourScheme == getDarkColourScheme() || colourScheme == getProjucerDarkColourScheme())
    {
        setColour (backgroundColourId,                   Colour (0xff323e44));
        setColour (secondaryBackgroundColourId,          Colour (0xff263238));
        setColour (defaultTextColourId,                  Colours::white);
        setColour (widgetTextColourId,                   Colours::white);
        setColour (defaultButtonBackgroundColourId,      Colour (0xffa45c94));
        setColour (secondaryButtonBackgroundColourId,    Colours::black);
        setColour (userButtonBackgroundColourId,         Colour (0xffa45c94));
        setColour (defaultIconColourId,                  Colours::white);
        setColour (treeIconColourId,                     Colour (0xffa9a9a9));
        setColour (defaultHighlightColourId,             Colour (0xffe0ec65));
        setColour (defaultHighlightedTextColourId,       Colours::black);
        setColour (codeEditorLineNumberColourId,         Colour (0xffaaaaaa));
        setColour (activeTabIconColourId,                Colours::white);
        setColour (inactiveTabBackgroundColourId,        Colour (0xff181f22));
        setColour (inactiveTabIconColourId,              Colour (0xffa9a9a9));
        setColour (contentHeaderBackgroundColourId,      Colours::black);
        setColour (widgetBackgroundColourId,             Colour (0xff495358));
        setColour (secondaryWidgetBackgroundColourId,    Colour (0xff303b41));

        colourScheme = getProjucerDarkColourScheme();
    }
    else if (colourScheme == getGreyColourScheme())
    {
        setColour (backgroundColourId,                   Colour (0xff505050));
        setColour (secondaryBackgroundColourId,          Colour (0xff424241));
        setColour (defaultTextColourId,                  Colours::white);
        setColour (widgetTextColourId,                   Colours::black);
        setColour (defaultButtonBackgroundColourId,      Colour (0xff26ba90));
        setColour (secondaryButtonBackgroundColourId,    Colours::black);
        setColour (userButtonBackgroundColourId,         Colour (0xff26ba90));
        setColour (defaultIconColourId,                  Colours::white);
        setColour (treeIconColourId,                     Colour (0xffa9a9a9));
        setColour (defaultHighlightColourId,             Colour (0xffe0ec65));
        setColour (defaultHighlightedTextColourId,       Colours::black);
        setColour (codeEditorLineNumberColourId,         Colour (0xffaaaaaa));
        setColour (activeTabIconColourId,                Colours::white);
        setColour (inactiveTabBackgroundColourId,        Colour (0xff373737));
        setColour (inactiveTabIconColourId,              Colour (0xffa9a9a9));
        setColour (contentHeaderBackgroundColourId,      Colours::black);
        setColour (widgetBackgroundColourId,             Colours::white);
        setColour (secondaryWidgetBackgroundColourId,    Colour (0xffdddddd));
    }
    else if (colourScheme == getLightColourScheme())
    {
        setColour (backgroundColourId,                   Colour (0xffefefef));
        setColour (secondaryBackgroundColourId,          Colour (0xfff9f9f9));
        setColour (defaultTextColourId,                  Colours::black);
        setColour (widgetTextColourId,                   Colours::black);
        setColour (defaultButtonBackgroundColourId,      Colour (0xff42a2c8));
        setColour (secondaryButtonBackgroundColourId,    Colour (0xffa1c677));
        setColour (userButtonBackgroundColourId,         Colour (0xff42a2c8));
        setColour (defaultIconColourId,                  Colours::white);
        setColour (treeIconColourId,                     Colour (0xffa9a9a9));
        setColour (defaultHighlightColourId,             Colours::orange);
        setColour (defaultHighlightedTextColourId,       Colour (0xff585656));
        setColour (codeEditorLineNumberColourId,         Colour (0xff888888));
        setColour (activeTabIconColourId,                Colour (0xff42a2c8));
        setColour (inactiveTabBackgroundColourId,        Colour (0xffd5d5d5));
        setColour (inactiveTabIconColourId,              Colour (0xffa9a9a9));
        setColour (contentHeaderBackgroundColourId,      Colour (0xff42a2c8));
        setColour (widgetBackgroundColourId,             Colours::white);
        setColour (secondaryWidgetBackgroundColourId,    Colour (0xfff4f4f4));
    }

    setColour (Label::textColourId,                             findColour (defaultTextColourId));
    setColour (Label::textWhenEditingColourId,                  findColour (widgetTextColourId));
    setColour (TextEditor::highlightColourId,                   findColour (defaultHighlightColourId).withAlpha (0.75f));
    setColour (TextEditor::highlightedTextColourId,             findColour (defaultHighlightedTextColourId));
    setColour (TextEditor::outlineColourId,                     Colours::transparentBlack);
    setColour (TextEditor::focusedOutlineColourId,              Colours::transparentBlack);
    setColour (TextEditor::backgroundColourId,                  findColour (widgetBackgroundColourId));
    setColour (TextEditor::textColourId,                        findColour (widgetTextColourId));
    setColour (TextButton::buttonColourId,                      findColour (defaultButtonBackgroundColourId));
    setColour (ScrollBar::ColourIds::thumbColourId,             Colour (0xffd0d8e0));
    setColour (TextPropertyComponent::outlineColourId,          Colours::transparentBlack);
    setColour (TextPropertyComponent::backgroundColourId,       findColour (widgetBackgroundColourId));
    setColour (TextPropertyComponent::textColourId,             findColour (widgetTextColourId));
    setColour (BooleanPropertyComponent::outlineColourId,       Colours::transparentBlack);
    setColour (BooleanPropertyComponent::backgroundColourId,    findColour (widgetBackgroundColourId));
    setColour (ToggleButton::tickDisabledColourId,              Colour (0xffa9a9a9));
    setColour (ToggleButton::tickColourId,                      findColour (defaultButtonBackgroundColourId).withMultipliedBrightness(1.3f));
    setColour (CodeEditorComponent::backgroundColourId,         findColour (secondaryBackgroundColourId));
    setColour (CodeEditorComponent::lineNumberTextId,           findColour (codeEditorLineNumberColourId));
    setColour (CodeEditorComponent::lineNumberBackgroundId,     findColour (backgroundColourId));
    setColour (CodeEditorComponent::highlightColourId,          findColour (defaultHighlightColourId).withAlpha (0.5f));
    setColour (CaretComponent::caretColourId,                   findColour (defaultButtonBackgroundColourId));
    setColour (TreeView::selectedItemBackgroundColourId,        findColour (defaultHighlightColourId));
    setColour (PopupMenu::highlightedBackgroundColourId,        findColour (defaultHighlightColourId).withAlpha (0.75f));
    setColour (PopupMenu::highlightedTextColourId,              findColour (defaultHighlightedTextColourId));
    setColour (ProgressBar::foregroundColourId,                 findColour (defaultButtonBackgroundColourId));
    setColour (0x1000440, /*LassoComponent::lassoFillColourId*/ findColour (defaultHighlightColourId).withAlpha (0.3f));
}
