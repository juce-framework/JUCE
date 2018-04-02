/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_ProjucerLookAndFeel.h"
#include "../../Application/jucer_Application.h"

//==============================================================================
ProjucerLookAndFeel::ProjucerLookAndFeel()
{
    setupColors();
}

ProjucerLookAndFeel::~ProjucerLookAndFeel() {}

void ProjucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const auto area = button.getActiveArea();
    auto backgroundColor = findColor (button.isFrontTab() ? secondaryBackgroundColorId
                                                            : inactiveTabBackgroundColorId);
    auto iconColor = findColor (button.isFrontTab() ? activeTabIconColorId
                                                      : inactiveTabIconColorId);

    g.setColor (backgroundColor);
    g.fillRect (area);

    const auto alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

   #ifndef BUILDING_JUCE_COMPILEENGINE
    if (button.getName() == "Project")
    {
        auto icon = Icon (getIcons().closedFolder, iconColor.withMultipliedAlpha (alpha));
        icon.draw (g, button.getTextArea().reduced (8, 8).toFloat(), false);
    }
    else if (button.getName() == "Build")
    {
        auto icon = Icon (getIcons().buildTab, iconColor.withMultipliedAlpha (alpha));
        icon.draw (g, button.getTextArea().reduced (8, 8).toFloat(), false);
    }
    else
   #endif
    {
        auto textColor = findColor (defaultTextColorId).withMultipliedAlpha (alpha);

        TextLayout textLayout;
        LookAndFeel_V3::createTabTextLayout (button, (float) area.getWidth(), (float) area.getHeight(), textColor, textLayout);

        textLayout.draw (g, button.getTextArea().toFloat());
    }
}

int ProjucerLookAndFeel::getTabButtonBestWidth (TabBarButton& button, int)
{
    if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
        return bar->getWidth() / bar->getNumTabs();

    return 120;
}

void ProjucerLookAndFeel::drawPropertyComponentLabel (Graphics& g, int width, int height, PropertyComponent& component)
{
    ignoreUnused (width);

    g.setColor (component.findColor (defaultTextColorId)
                          .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    auto textWidth = getTextWidthForPropertyComponent (&component);

    g.setFont (getPropertyComponentFont());
    g.drawFittedText (component.getName(), 0, 0, textWidth - 5, height, Justification::centeredLeft, 5, 1.0f);
}

Rectangle<int> ProjucerLookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    const auto textW = getTextWidthForPropertyComponent (&component);
    return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
}

void ProjucerLookAndFeel::drawButtonBackground (Graphics& g,
                                                Button& button,
                                                const Color& backgroundColor,
                                                bool isMouseOverButton,
                                                bool isButtonDown)
{
    const auto cornerSize = button.findParentComponentOfClass<PropertyComponent>() != nullptr ? 0.0f : 3.0f;
    const auto bounds = button.getLocalBounds().toFloat();

    auto baseColor = backgroundColor.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (isButtonDown || isMouseOverButton)
        baseColor = baseColor.contrasting (isButtonDown ? 0.2f : 0.05f);

    g.setColor (baseColor);

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

    g.setColor (button.findColor (button.getToggleState() ? TextButton::textColorOnId
                                                            : TextButton::textColorOffId)
                                   .withMultipliedAlpha (button.isEnabled() ? 1.0f
                                                                            : 0.5f));

    auto xIndent = jmin (8, button.getWidth() / 10);
    auto yIndent = jmin (3,  button.getHeight() / 6);

    auto textBounds = button.getLocalBounds().reduced (xIndent, yIndent);

    g.drawFittedText (button.getButtonText(), textBounds, Justification::centered, 3, 1.0f);
}

void ProjucerLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown)
{
    ignoreUnused (isMouseOverButton, isButtonDown);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    bool isTextEmpty = button.getButtonText().isEmpty();
    bool isPropertyComponentChild = (dynamic_cast<BooleanPropertyComponent*> (button.getParentComponent()) != nullptr);

    auto bounds = button.getLocalBounds();

    auto sideLength = isPropertyComponentChild ? 25 : bounds.getHeight();

    auto rectBounds = isTextEmpty ? bounds
                                  : bounds.removeFromLeft (jmin (sideLength, bounds.getWidth() / 3));

    rectBounds = rectBounds.withSizeKeepingCenter (sideLength, sideLength).reduced (4);

    g.setColor (button.findColor (ToggleButton::tickDisabledColorId));
    g.drawRoundedRectangle (rectBounds.toFloat(), 2.0f, 1.0f);

    if (button.getToggleState())
    {
        g.setColor (button.findColor (ToggleButton::tickColorId));
        const auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (rectBounds.reduced (2).toFloat(), false));
    }

    if (! isTextEmpty)
    {
        bounds.removeFromLeft (5);

        const auto fontSize = jmin (15.0f, button.getHeight() * 0.75f);

        g.setFont (fontSize);
        g.setColor (isPropertyComponentChild ? findColor (widgetTextColorId)
                                              : button.findColor (ToggleButton::textColorId));

        g.drawFittedText (button.getButtonText(), bounds, Justification::centeredLeft, 2);
    }
}

void ProjucerLookAndFeel::fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor)
{
    g.setColor (textEditor.findColor (TextEditor::backgroundColorId));
    g.fillRect (0, 0, width, height);

    g.setColor (textEditor.findColor (TextEditor::outlineColorId));
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
    currentPathBox->setColor (ComboBox::backgroundColorId,    findColor (backgroundColorId));
    currentPathBox->setColor (ComboBox::textColorId,          findColor (defaultTextColorId));
    currentPathBox->setColor (ComboBox::arrowColorId,         findColor (defaultTextColorId));

    topSlice.removeFromLeft (6);
    goUpButton->setBounds (topSlice);

    bottomSlice.removeFromLeft (50);
    filenameBox->setBounds (bottomSlice);
    filenameBox->setColor (TextEditor::backgroundColorId, findColor (backgroundColorId));
    filenameBox->setColor (TextEditor::textColorId,       findColor (defaultTextColorId));
    filenameBox->setColor (TextEditor::outlineColorId,    findColor (defaultTextColorId));
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
        fileListComp->setColor (DirectoryContentsDisplayComponent::textColorId,
                                 findColor (isItemSelected ? defaultHighlightedTextColorId : defaultTextColorId));

        fileListComp->setColor (DirectoryContentsDisplayComponent::highlightColorId,
                                 findColor (defaultHighlightColorId).withAlpha (0.75f));
    }


    LookAndFeel_V2::drawFileBrowserRow (g, width, height, file, filename, icon,
                                        fileSizeDescription, fileTimeDescription,
                                        isDirectory, isItemSelected, itemIndex, dcc);
}

void ProjucerLookAndFeel::drawCallOutBoxBackground (CallOutBox&, Graphics& g, const Path& path, Image&)
{
    g.setColor (findColor (secondaryBackgroundColorId));
    g.fillPath (path);

    g.setColor (findColor (userButtonBackgroundColorId));
    g.strokePath (path, PathStrokeType (2.0f));
}

void ProjucerLookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height,
                                                 bool, MenuBarComponent& menuBar)
{
    const auto color = menuBar.findColor (backgroundColorId).withAlpha (0.75f);

    Rectangle<int> r (width, height);

    g.setColor (color.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColorGradient (color, 0, 0, color.darker (0.2f), 0, (float)height, false));
    g.fillRect (r);
}

void ProjucerLookAndFeel::drawMenuBarItem (Graphics& g, int width, int height,
                                           int itemIndex, const String& itemText,
                                           bool isMouseOverItem, bool isMenuOpen,
                                           bool /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColor (menuBar.findColor (defaultTextColorId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColor (defaultHighlightColorId).withAlpha (0.75f));
        g.setColor (menuBar.findColor (defaultHighlightedTextColorId));
    }
    else
    {
        g.setColor (menuBar.findColor (defaultTextColorId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centered, 1);
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
        box.setColor (ComboBox::textColorId, findColor (widgetTextColorId));

        g.setColor (findColor (widgetBackgroundColorId));
        g.fillRect (boxBounds);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).reduced (0, 2).toFloat();
        g.setColor (Colors::black);
        g.fillPath (getChoiceComponentArrowPath (arrowZone));
    }
    else
    {
        g.setColor (box.findColor (ComboBox::outlineColorId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        auto arrowZone = boxBounds.removeFromRight (boxBounds.getHeight()).toFloat();
        g.setColor (box.findColor (ComboBox::arrowColorId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.fillPath (getArrowPath (arrowZone, 2, true, Justification::centered));

    }
}

void ProjucerLookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                                    Color, bool isOpen, bool /**isMouseOver*/)
{
    g.strokePath (getArrowPath (area, isOpen ? 2 : 1, false, Justification::centeredRight), PathStrokeType (2.0f));
}

void ProjucerLookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                           int width, int height, double progress, const String& textToShow)
{
    ignoreUnused (width, height, progress);

    const auto background = progressBar.findColor (ProgressBar::backgroundColorId);
    const auto foreground = progressBar.findColor (defaultButtonBackgroundColorId);

    const auto sideLength = jmin (width, height);

    auto barBounds = progressBar.getLocalBounds().withSizeKeepingCenter (sideLength, sideLength).reduced (1).toFloat();

    auto rotationInDegrees  = static_cast<float> ((Time::getMillisecondCounter() / 10) % 360);
    auto normalizedRotation = rotationInDegrees / 360.0f;

    const auto rotationOffset = 22.5f;
    const auto maxRotation    = 315.0f;

    auto startInDegrees = rotationInDegrees;
    auto endInDegrees   = startInDegrees + rotationOffset;

    if (normalizedRotation >= 0.25f && normalizedRotation < 0.5f)
    {
        const auto rescaledRotation = (normalizedRotation * 4.0f) - 1.0f;
        endInDegrees = startInDegrees + rotationOffset + (maxRotation * rescaledRotation);
    }
    else if (normalizedRotation >= 0.5f && normalizedRotation <= 1.0f)
    {
        endInDegrees = startInDegrees + rotationOffset + maxRotation;
        const auto rescaledRotation = 1.0f - ((normalizedRotation * 2.0f) - 1.0f);
        startInDegrees = endInDegrees - rotationOffset - (maxRotation * rescaledRotation);
    }

    g.setColor (background);
    Path arcPath2;
    arcPath2.addCenteredArc (barBounds.getCenterX(),
                            barBounds.getCenterY(),
                            barBounds.getWidth() * 0.5f,
                            barBounds.getHeight() * 0.5f, 0.0f,
                            0.0f,
                            MathConstants<float>::twoPi,
                            true);
    g.strokePath (arcPath2, PathStrokeType (2.0f));

    g.setColor (foreground);
    Path arcPath;
    arcPath.addCenteredArc (barBounds.getCenterX(),
                           barBounds.getCenterY(),
                           barBounds.getWidth() * 0.5f,
                           barBounds.getHeight() * 0.5f,
                           0.0f,
                           degreesToRadians (startInDegrees),
                           degreesToRadians (endInDegrees),
                           true);

    arcPath.applyTransform (AffineTransform::rotation (normalizedRotation * MathConstants<float>::pi * 2.25f,
                                                       barBounds.getCenterX(), barBounds.getCenterY()));
    g.strokePath (arcPath, PathStrokeType (2.0f));

    if (textToShow.isNotEmpty())
    {
        g.setColor (progressBar.findColor (TextButton::textColorOffId));
        g.setFont (Font (12.0f, 2));
        g.drawText (textToShow, barBounds, Justification::centered, false);
    }
}

//==============================================================================
Path ProjucerLookAndFeel::getArrowPath (Rectangle<float> arrowZone, const int direction,
                                        bool filled, const Justification justification)
{
    auto w = jmin (arrowZone.getWidth(),  (direction == 0 || direction == 2) ? 8.0f : filled ? 5.0f : 8.0f);
    auto h = jmin (arrowZone.getHeight(), (direction == 0 || direction == 2) ? 5.0f : filled ? 8.0f : 5.0f);

    if (justification == Justification::centered)
    {
        arrowZone.reduce ((arrowZone.getWidth() - w) / 2, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centeredRight)
    {
        arrowZone.removeFromLeft (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else if (justification == Justification::centeredLeft)
    {
        arrowZone.removeFromRight (arrowZone.getWidth() - w);
        arrowZone.reduce (0, (arrowZone.getHeight() - h) / 2);
    }
    else
    {
        jassertfalse; // currently only supports centered justifications
    }

    Path path;
    path.startNewSubPath (arrowZone.getX(), arrowZone.getBottom());
    path.lineTo (arrowZone.getCenterX(), arrowZone.getY());
    path.lineTo (arrowZone.getRight(), arrowZone.getBottom());

    if (filled)
        path.closeSubPath();

    path.applyTransform (AffineTransform::rotation (direction * MathConstants<float>::halfPi,
                                                    arrowZone.getCenterX(), arrowZone.getCenterY()));

    return path;
}

Path ProjucerLookAndFeel::getChoiceComponentArrowPath (Rectangle<float> arrowZone)
{
    auto topBounds = arrowZone.removeFromTop (arrowZone.getHeight() * 0.5f);
    auto bottomBounds = arrowZone;

    auto topArrow = getArrowPath (topBounds, 0, true, Justification::centered);
    auto bottomArrow = getArrowPath (bottomBounds, 2, true, Justification::centered);

    topArrow.addPath (bottomArrow);

    return topArrow;
}

//==============================================================================
void ProjucerLookAndFeel::setupColors()
{
    auto& colorScheme = getCurrentColorScheme();

    if (colorScheme == getDarkColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xff323e44));
        setColor (secondaryBackgroundColorId,          Color (0xff263238));
        setColor (defaultTextColorId,                  Colors::white);
        setColor (widgetTextColorId,                   Colors::white);
        setColor (defaultButtonBackgroundColorId,      Color (0xffa45c94));
        setColor (secondaryButtonBackgroundColorId,    Colors::black);
        setColor (userButtonBackgroundColorId,         Color (0xffa45c94));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Color (0xffe0ec65));
        setColor (defaultHighlightedTextColorId,       Colors::black);
        setColor (codeEditorLineNumberColorId,         Color (0xffaaaaaa));
        setColor (activeTabIconColorId,                Colors::white);
        setColor (inactiveTabBackgroundColorId,        Color (0xff181f22));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Colors::black);
        setColor (widgetBackgroundColorId,             Color (0xff495358));
        setColor (secondaryWidgetBackgroundColorId,    Color (0xff303b41));

        colorScheme.setUIColor (LookAndFeel_V4::ColorScheme::UIColor::defaultFill, Color (0xffa45c94));
    }
    else if (colorScheme == getGrayColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xff505050));
        setColor (secondaryBackgroundColorId,          Color (0xff424241));
        setColor (defaultTextColorId,                  Colors::white);
        setColor (widgetTextColorId,                   Colors::black);
        setColor (defaultButtonBackgroundColorId,      Color (0xff26ba90));
        setColor (secondaryButtonBackgroundColorId,    Colors::black);
        setColor (userButtonBackgroundColorId,         Color (0xff26ba90));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Color (0xffe0ec65));
        setColor (defaultHighlightedTextColorId,       Colors::black);
        setColor (codeEditorLineNumberColorId,         Color (0xffaaaaaa));
        setColor (activeTabIconColorId,                Colors::white);
        setColor (inactiveTabBackgroundColorId,        Color (0xff373737));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Colors::black);
        setColor (widgetBackgroundColorId,             Colors::white);
        setColor (secondaryWidgetBackgroundColorId,    Color (0xffdddddd));
    }
    else if (colorScheme == getLightColorScheme())
    {
        setColor (backgroundColorId,                   Color (0xffefefef));
        setColor (secondaryBackgroundColorId,          Color (0xfff9f9f9));
        setColor (defaultTextColorId,                  Colors::black);
        setColor (widgetTextColorId,                   Colors::black);
        setColor (defaultButtonBackgroundColorId,      Color (0xff42a2c8));
        setColor (secondaryButtonBackgroundColorId,    Color (0xffa1c677));
        setColor (userButtonBackgroundColorId,         Color (0xff42a2c8));
        setColor (defaultIconColorId,                  Colors::white);
        setColor (treeIconColorId,                     Color (0xffa9a9a9));
        setColor (defaultHighlightColorId,             Color (0xffffd05b));
        setColor (defaultHighlightedTextColorId,       Color (0xff585656));
        setColor (codeEditorLineNumberColorId,         Color (0xff888888));
        setColor (activeTabIconColorId,                Color (0xff42a2c8));
        setColor (inactiveTabBackgroundColorId,        Color (0xffd5d5d5));
        setColor (inactiveTabIconColorId,              Color (0xffa9a9a9));
        setColor (contentHeaderBackgroundColorId,      Color (0xff42a2c8));
        setColor (widgetBackgroundColorId,             Colors::white);
        setColor (secondaryWidgetBackgroundColorId,    Color (0xfff4f4f4));
    }

    setColor (Label::textColorId,                             findColor (defaultTextColorId));
    setColor (Label::textWhenEditingColorId,                  findColor (widgetTextColorId));
    setColor (TextEditor::highlightColorId,                   findColor (defaultHighlightColorId).withAlpha (0.75f));
    setColor (TextEditor::highlightedTextColorId,             findColor (defaultHighlightedTextColorId));
    setColor (TextEditor::outlineColorId,                     Colors::transparentBlack);
    setColor (TextEditor::focusedOutlineColorId,              Colors::transparentBlack);
    setColor (TextEditor::backgroundColorId,                  findColor (widgetBackgroundColorId));
    setColor (TextEditor::textColorId,                        findColor (widgetTextColorId));
    setColor (TextButton::buttonColorId,                      findColor (defaultButtonBackgroundColorId));
    setColor (ScrollBar::ColorIds::thumbColorId,             Color (0xffd0d8e0));
    setColor (TextPropertyComponent::outlineColorId,          Colors::transparentBlack);
    setColor (TextPropertyComponent::backgroundColorId,       findColor (widgetBackgroundColorId));
    setColor (TextPropertyComponent::textColorId,             findColor (widgetTextColorId));
    setColor (BooleanPropertyComponent::outlineColorId,       Colors::transparentBlack);
    setColor (BooleanPropertyComponent::backgroundColorId,    findColor (widgetBackgroundColorId));
    setColor (ToggleButton::tickDisabledColorId,              Color (0xffa9a9a9));
    setColor (ToggleButton::tickColorId,                      findColor (defaultButtonBackgroundColorId).withMultipliedBrightness(1.3f));
    setColor (CodeEditorComponent::backgroundColorId,         findColor (secondaryBackgroundColorId));
    setColor (CodeEditorComponent::lineNumberTextId,           findColor (codeEditorLineNumberColorId));
    setColor (CodeEditorComponent::lineNumberBackgroundId,     findColor (backgroundColorId));
    setColor (CodeEditorComponent::highlightColorId,          findColor (defaultHighlightColorId).withAlpha (0.5f));
    setColor (CaretComponent::caretColorId,                   findColor (defaultButtonBackgroundColorId));
    setColor (TreeView::selectedItemBackgroundColorId,        findColor (defaultHighlightColorId));
    setColor (PopupMenu::highlightedBackgroundColorId,        findColor (defaultHighlightColorId).withAlpha (0.75f));
    setColor (PopupMenu::highlightedTextColorId,              findColor (defaultHighlightedTextColorId));
    setColor (0x1000440, /*LassoComponent::lassoFillColorId*/ findColor (defaultHighlightColorId).withAlpha (0.3f));
}
