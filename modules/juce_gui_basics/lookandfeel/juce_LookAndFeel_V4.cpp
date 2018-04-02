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

namespace juce
{

Color LookAndFeel_V4::ColorScheme::getUIColor (UIColor index) const noexcept
{
    if (isPositiveAndBelow (index, numColors))
        return palette[index];

    jassertfalse;
    return {};
}

void LookAndFeel_V4::ColorScheme::setUIColor (UIColor index, Color newColor) noexcept
{
    if (isPositiveAndBelow (index, numColors))
        palette[index] = newColor;
    else
        jassertfalse;
}

bool LookAndFeel_V4::ColorScheme::operator== (const ColorScheme& other) const noexcept
{
    for (auto i = 0; i < numColors; ++i)
        if (palette[i] != other.palette[i])
            return false;

    return true;
}

bool LookAndFeel_V4::ColorScheme::operator!= (const ColorScheme& other) const noexcept
{
    return ! operator== (other);
}

//==============================================================================
LookAndFeel_V4::LookAndFeel_V4()  : currentColorScheme (getDarkColorScheme())
{
    initializeColors();
}

LookAndFeel_V4::LookAndFeel_V4 (ColorScheme scheme)  : currentColorScheme (scheme)
{
    initializeColors();
}

LookAndFeel_V4::~LookAndFeel_V4()  {}

//==============================================================================
void LookAndFeel_V4::setColorScheme (ColorScheme newColorScheme)
{
    currentColorScheme = newColorScheme;
    initializeColors();
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getDarkColorScheme()
{
    return { 0xff323e44, 0xff263238, 0xff323e44,
             0xff8e989b, 0xffffffff, 0xff42a2c8,
             0xffffffff, 0xff181f22, 0xffffffff };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getMidnightColorScheme()
{
    return { 0xff2f2f3a, 0xff191926, 0xffd0d0d0,
             0xff66667c, 0xc8ffffff, 0xffd8d8d8,
             0xffffffff, 0xff606073, 0xff000000 };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getGrayColorScheme()
{
    return { 0xff505050, 0xff424242, 0xff606060,
             0xffa6a6a6, 0xffffffff, 0xff21ba90,
             0xff000000, 0xffffffff, 0xffffffff };
}

LookAndFeel_V4::ColorScheme LookAndFeel_V4::getLightColorScheme()
{
    return { 0xffefefef, 0xffffffff, 0xffffffff,
             0xffdddddd, 0xff000000, 0xffa9a9a9,
             0xffffffff, 0xff42a2c8, 0xff000000 };
}

//==============================================================================
class LookAndFeel_V4_DocumentWindowButton   : public Button
{
public:
    LookAndFeel_V4_DocumentWindowButton (const String& name, Color c, const Path& normal, const Path& toggled)
        : Button (name), color (c), normalShape (normal), toggledShape (toggled)
    {
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto background = Colors::gray;

        if (auto* rw = findParentComponentOfClass<ResizableWindow>())
            if (auto lf = dynamic_cast<LookAndFeel_V4*> (&rw->getLookAndFeel()))
                background = lf->getCurrentColorScheme().getUIColor (LookAndFeel_V4::ColorScheme::widgetBackground);

        g.fillAll (background);

        g.setColor ((! isEnabled() || isButtonDown) ? color.withAlpha (0.6f)
                                                     : color);

        if (isMouseOverButton)
        {
            g.fillAll();
            g.setColor (background);
        }

        auto& p = getToggleState() ? toggledShape : normalShape;

        auto reducedRect = Justification (Justification::centered)
                              .appliedToRectangle (Rectangle<int> (getHeight(), getHeight()), getLocalBounds())
                              .toFloat()
                              .reduced (getHeight() * 0.3f);

        g.fillPath (p, p.getTransformToScaleToFit (reducedRect, true));
    }

private:
    Color color;
    Path normalShape, toggledShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V4_DocumentWindowButton)
};

Button* LookAndFeel_V4::createDocumentWindowButton (int buttonType)
{
    Path shape;
    auto crossThickness = 0.15f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment ({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);

        return new LookAndFeel_V4_DocumentWindowButton ("close", Color (0xff9A131D), shape, shape);
    }

    if (buttonType == DocumentWindow::minimizeButton)
    {
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        return new LookAndFeel_V4_DocumentWindowButton ("minimize", Color (0xffaa8811), shape, shape);
    }

    if (buttonType == DocumentWindow::maximizeButton)
    {
        shape.addLineSegment ({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
        shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new LookAndFeel_V4_DocumentWindowButton ("maximize", Color (0xff0A830A), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

void LookAndFeel_V4::positionDocumentWindowButtons (DocumentWindow&,
                                                    int titleBarX, int titleBarY,
                                                    int titleBarW, int titleBarH,
                                                    Button* minimizeButton,
                                                    Button* maximizeButton,
                                                    Button* closeButton,
                                                    bool positionTitleBarButtonsOnLeft)
{
    titleBarH = jmin (titleBarH, titleBarH - titleBarY);

    auto buttonW = static_cast<int> (titleBarH * 1.2);

    auto x = positionTitleBarButtonsOnLeft ? titleBarX
                                           : titleBarX + titleBarW - buttonW;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimizeButton, maximizeButton);

    if (maximizeButton != nullptr)
    {
        maximizeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimizeButton != nullptr)
        minimizeButton->setBounds (x, titleBarY, buttonW, titleBarH);
}

void LookAndFeel_V4::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                                 int w, int h, int titleSpaceX, int titleSpaceW,
                                                 const Image* icon, bool drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    auto isActive = window.isActiveWindow();

    g.setColor (getCurrentColorScheme().getUIColor (ColorScheme::widgetBackground));
    g.fillAll();

    Font font (h * 0.65f, Font::plain);
    g.setFont (font);

    auto textW = font.getStringWidth (window.getName());
    auto iconW = 0;
    auto iconH = 0;

    if (icon != nullptr)
    {
        iconH = static_cast<int> (font.getHeight());
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    auto textX = drawTitleTextOnLeft ? titleSpaceX
                                     : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centered, false);
        textX += iconW;
        textW -= iconW;
    }

    if (window.isColorSpecified (DocumentWindow::textColorId) || isColorSpecified (DocumentWindow::textColorId))
        g.setColor (window.findColor (DocumentWindow::textColorId));
    else
        g.setColor (getCurrentColorScheme().getUIColor (ColorScheme::defaultText));

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centeredLeft, true);
}

//==============================================================================
Font LookAndFeel_V4::getTextButtonFont (TextButton&, int buttonHeight)
{
    return { jmin (16.0f, buttonHeight * 0.6f) };
}

void LookAndFeel_V4::drawButtonBackground (Graphics& g,
                                           Button& button,
                                           const Color& backgroundColor,
                                           bool isMouseOverButton,
                                           bool isButtonDown)
{
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColor = backgroundColor.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

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

        g.setColor (button.findColor (ComboBox::outlineColorId));
        g.strokePath (path, PathStrokeType (1.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColor (button.findColor (ComboBox::outlineColorId));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
}

void LookAndFeel_V4::drawToggleButton (Graphics& g, ToggleButton& button,
                                       bool isMouseOverButton, bool isButtonDown)
{
    auto fontSize = jmin (15.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColor (button.findColor (ToggleButton::textColorId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
                                             .withTrimmedRight (2),
                      Justification::centeredLeft, 10);
}

void LookAndFeel_V4::drawTickBox (Graphics& g, Component& component,
                                  float x, float y, float w, float h,
                                  const bool ticked,
                                  const bool isEnabled,
                                  const bool isMouseOverButton,
                                  const bool isButtonDown)
{
    ignoreUnused (isEnabled, isMouseOverButton, isButtonDown);

    Rectangle<float> tickBounds (x, y, w, h);

    g.setColor (component.findColor (ToggleButton::tickDisabledColorId));
    g.drawRoundedRectangle (tickBounds, 4.0f, 1.0f);

    if (ticked)
    {
        g.setColor (component.findColor (ToggleButton::tickColorId));
        auto tick = getTickShape (0.75f);
        g.fillPath (tick, tick.getTransformToScaleToFit (tickBounds.reduced (4, 5).toFloat(), false));
    }
}

void LookAndFeel_V4::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    auto fontSize = jmin (15.0f, button.getHeight() * 0.75f);
    auto tickWidth = fontSize * 1.1f;

    Font font (fontSize);

    button.setSize (font.getStringWidth (button.getButtonText()) + roundToInt (tickWidth) + 14, button.getHeight());
}

//==============================================================================
AlertWindow* LookAndFeel_V4::createAlertWindow (const String& title, const String& message,
                                                const String& button1, const String& button2, const String& button3,
                                                AlertWindow::AlertIconType iconType,
                                                int numButtons, Component* associatedComponent)
{
    auto boundsOffset = 50;

    auto* aw = LookAndFeel_V2::createAlertWindow (title, message, button1, button2, button3,
                                                  iconType, numButtons, associatedComponent);

    auto bounds = aw->getBounds();
    bounds = bounds.withSizeKeepingCenter (bounds.getWidth() + boundsOffset, bounds.getHeight() + boundsOffset);
    aw->setBounds (bounds);

    for (auto* child : aw->getChildren())
        if (auto* button = dynamic_cast<TextButton*> (child))
            button->setBounds (button->getBounds() + Point<int> (25, 40));

    return aw;
}

void LookAndFeel_V4::drawAlertBox (Graphics& g, AlertWindow& alert,
                                   const Rectangle<int>& textArea, TextLayout& textLayout)
{
    auto cornerSize = 4.0f;

    g.setColor (alert.findColor (AlertWindow::outlineColorId));
    g.drawRoundedRectangle (alert.getLocalBounds().toFloat(), cornerSize, 2.0f);

    auto bounds = alert.getLocalBounds().reduced (1);
    g.reduceClipRegion (bounds);

    g.setColor (alert.findColor (AlertWindow::backgroundColorId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    auto iconSpaceUsed = 0;

    auto iconWidth = 80;
    auto iconSize = jmin (iconWidth + 50, bounds.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    Rectangle<int> iconRect (iconSize / -10, iconSize / -10,
                             iconSize, iconSize);

    if (alert.getAlertType() != AlertWindow::NoIcon)
    {
        Path icon;
        char character;
        uint32 color;

        if (alert.getAlertType() == AlertWindow::WarningIcon)
        {
            character = '!';

            icon.addTriangle (iconRect.getX() + iconRect.getWidth() * 0.5f, (float) iconRect.getY(),
                              static_cast<float> (iconRect.getRight()), static_cast<float> (iconRect.getBottom()),
                              static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getBottom()));

            icon = icon.createPathWithRoundedCorners (5.0f);
            color = 0x66ff2a00;
        }
        else
        {
            color = Color (0xff00b0b9).withAlpha (0.4f).getARGB();
            character = alert.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText ({ iconRect.getHeight() * 0.9f, Font::bold },
                          String::charToString ((juce_wchar) (uint8) character),
                          static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getY()),
                          static_cast<float> (iconRect.getWidth()), static_cast<float> (iconRect.getHeight()),
                          Justification::centered, false);
        ga.createPath (icon);

        icon.setUsingNonZeroWinding (false);
        g.setColor (Color (color));
        g.fillPath (icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColor (alert.findColor (AlertWindow::textColorId));

    Rectangle<int> alertBounds (bounds.getX() + iconSpaceUsed, 30,
                                bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

    textLayout.draw (g, alertBounds.toFloat());
}

int LookAndFeel_V4::getAlertWindowButtonHeight()    { return 40; }
Font LookAndFeel_V4::getAlertWindowTitleFont()      { return { 18.0f, Font::bold }; }
Font LookAndFeel_V4::getAlertWindowMessageFont()    { return { 16.0f }; }
Font LookAndFeel_V4::getAlertWindowFont()           { return { 14.0f }; }

//==============================================================================
void LookAndFeel_V4::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      int width, int height, double progress, const String& textToShow)
{
    if (width == height)
        drawCircularProgressBar (g, progressBar, textToShow);
    else
        drawLinearProgressBar (g, progressBar, width, height, progress, textToShow);
}

void LookAndFeel_V4::drawLinearProgressBar (Graphics& g, ProgressBar& progressBar,
                                            int width, int height,
                                            double progress, const String& textToShow)
{
    auto background = progressBar.findColor (ProgressBar::backgroundColorId);
    auto foreground = progressBar.findColor (ProgressBar::foregroundColorId);

    auto barBounds = progressBar.getLocalBounds().toFloat();

    g.setColor (background);
    g.fillRoundedRectangle (barBounds, progressBar.getHeight() * 0.5f);

    if (progress >= 0.0f && progress <= 1.0f)
    {
        Path p;
        p.addRoundedRectangle (barBounds, progressBar.getHeight() * 0.5f);
        g.reduceClipRegion (p);

        barBounds.setWidth (barBounds.getWidth() * (float) progress);
        g.setColor (foreground);
        g.fillRoundedRectangle (barBounds, progressBar.getHeight() * 0.5f);
    }
    else
    {
        // spinning bar..
        g.setColor (background);

        auto stripeWidth = height * 2;
        auto position = static_cast<int> (Time::getMillisecondCounter() / 15) % stripeWidth;

        Path p;

        for (auto x = static_cast<float> (-position); x < width + stripeWidth; x += stripeWidth)
            p.addQuadrilateral (x, 0.0f,
                                x + stripeWidth * 0.5f, 0.0f,
                                x, static_cast<float> (height),
                                x - stripeWidth * 0.5f, static_cast<float> (height));

        Image im (Image::ARGB, width, height, true);

        {
            Graphics g2 (im);
            g2.setColor (foreground);
            g2.fillRoundedRectangle (barBounds, progressBar.getHeight() * 0.5f);
        }

        g.setTiledImageFill (im, 0, 0, 0.85f);
        g.fillPath (p);
    }

    if (textToShow.isNotEmpty())
    {
        g.setColor (Color::contrasting (background, foreground));
        g.setFont (height * 0.6f);

        g.drawText (textToShow, 0, 0, width, height, Justification::centered, false);
    }
}

void LookAndFeel_V4::drawCircularProgressBar (Graphics& g, ProgressBar& progressBar, const String& progressText)
{
    auto background = progressBar.findColor (ProgressBar::backgroundColorId);
    auto foreground = progressBar.findColor (ProgressBar::foregroundColorId);

    auto barBounds = progressBar.getLocalBounds().reduced (2, 2).toFloat();

    auto rotationInDegrees  = static_cast<float> ((Time::getMillisecondCounter() / 10) % 360);
    auto normalizedRotation = rotationInDegrees / 360.0f;

    auto rotationOffset = 22.5f;
    auto maxRotation    = 315.0f;

    auto startInDegrees = rotationInDegrees;
    auto endInDegrees   = startInDegrees + rotationOffset;

    if (normalizedRotation >= 0.25f && normalizedRotation < 0.5f)
    {
        auto rescaledRotation = (normalizedRotation * 4.0f) - 1.0f;
        endInDegrees = startInDegrees + rotationOffset + (maxRotation * rescaledRotation);
    }
    else if (normalizedRotation >= 0.5f && normalizedRotation <= 1.0f)
    {
        endInDegrees = startInDegrees + rotationOffset + maxRotation;
        auto rescaledRotation = 1.0f - ((normalizedRotation * 2.0f) - 1.0f);
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
    g.strokePath (arcPath2, PathStrokeType (4.0f));

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

    arcPath.applyTransform (AffineTransform::rotation (normalizedRotation * MathConstants<float>::pi * 2.25f, barBounds.getCenterX(), barBounds.getCenterY()));
    g.strokePath (arcPath, PathStrokeType (4.0f));

    if (progressText.isNotEmpty())
    {
        g.setColor (progressBar.findColor (TextButton::textColorOffId));
        g.setFont ({ 12.0f, Font::italic });
        g.drawText (progressText, barBounds, Justification::centered, false);
    }
}

//==============================================================================
int LookAndFeel_V4::getDefaultScrollbarWidth()
{
    return 8;
}

void LookAndFeel_V4::drawScrollbar (Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height,
                                    bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown)
{
    ignoreUnused (isMouseDown);

    Rectangle<int> thumbBounds;

    if (isScrollbarVertical)
        thumbBounds = { x, thumbStartPosition, width, thumbSize };
    else
        thumbBounds = { thumbStartPosition, y, thumbSize, height };

    auto c = scrollbar.findColor (ScrollBar::ColorIds::thumbColorId);
    g.setColor (isMouseOver ? c.brighter (0.25f) : c);
    g.fillRoundedRectangle (thumbBounds.reduced (1).toFloat(), 4.0f);
}

//==============================================================================
Path LookAndFeel_V4::getTickShape (float height)
{
    static const unsigned char pathData[] = { 110,109,32,210,202,64,126,183,148,64,108,39,244,247,64,245,76,124,64,108,178,131,27,65,246,76,252,64,108,175,242,4,65,246,76,252,
        64,108,236,5,68,65,0,0,160,180,108,240,150,90,65,21,136,52,63,108,48,59,16,65,0,0,32,65,108,32,210,202,64,126,183,148,64, 99,101,0,0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

Path LookAndFeel_V4::getCrossShape (float height)
{
    static const unsigned char pathData[] = { 110,109,51,51,255,66,0,0,0,0,108,205,204,13,67,51,51,99,65,108,0,0,170,66,205,204,141,66,108,51,179,13,67,52,51,255,66,108,0,0,255,
        66,205,204,13,67,108,205,204,141,66,0,0,170,66,108,52,51,99,65,51,179,13,67,108,0,0,0,0,51,51,255,66,108,205,204,98,66, 204,204,141,66,108,0,0,0,0,51,51,99,65,108,51,51,
        99,65,0,0,0,0,108,205,204,141,66,205,204,98,66,108,51,51,255,66,0,0,0,0,99,101,0,0 };

    Path path;
    path.loadPathFromData (pathData, sizeof (pathData));
    path.scaleToFit (0, 0, height * 2.0f, height, true);

    return path;
}

//==============================================================================
void LookAndFeel_V4::fillTextEditorBackground (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) != nullptr)
    {
        g.setColor (textEditor.findColor (TextEditor::backgroundColorId));
        g.fillRect (0, 0, width, height);

        g.setColor (textEditor.findColor (TextEditor::outlineColorId));
        g.drawHorizontalLine (height - 1, 0.0f, static_cast<float> (width));
    }
    else
    {
        LookAndFeel_V2::fillTextEditorBackground (g, width, height, textEditor);
    }
}

void LookAndFeel_V4::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (dynamic_cast<AlertWindow*> (textEditor.getParentComponent()) == nullptr)
    {
        if (textEditor.isEnabled())
        {
            if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
            {
                g.setColor (textEditor.findColor (TextEditor::focusedOutlineColorId));
                g.drawRect (0, 0, width, height, 2);
            }
            else
            {
                g.setColor (textEditor.findColor (TextEditor::outlineColorId));
                g.drawRect (0, 0, width, height);
            }
        }
    }
}

//==============================================================================
Button* LookAndFeel_V4::createFileBrowserGoUpButton()
{
    auto* goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

    Path arrowPath;
    arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);

    DrawablePath arrowImage;
    arrowImage.setFill (goUpButton->findColor (TextButton::textColorOffId));
    arrowImage.setPath (arrowPath);

    goUpButton->setImages (&arrowImage);

    return goUpButton;
}

void LookAndFeel_V4::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                 DirectoryContentsDisplayComponent* fileListComponent,
                                                 FilePreviewComponent* previewComp,
                                                 ComboBox* currentPathBox,
                                                 TextEditor* filenameBox,
                                                 Button* goUpButton)
{
    auto sectionHeight = 22;
    auto buttonWidth = 50;

    auto b = browserComp.getLocalBounds().reduced (20, 5);

    auto topSlice    = b.removeFromTop (sectionHeight);
    auto bottomSlice = b.removeFromBottom (sectionHeight);

    currentPathBox->setBounds (topSlice.removeFromLeft (topSlice.getWidth() - buttonWidth));

    topSlice.removeFromLeft (6);
    goUpButton->setBounds (topSlice);

    bottomSlice.removeFromLeft (20);
    filenameBox->setBounds (bottomSlice);

    if (previewComp != nullptr)
        previewComp->setBounds (b.removeFromRight (b.getWidth() / 3));

    if (auto* listAsComp = dynamic_cast<Component*> (fileListComponent))
        listAsComp->setBounds (b.reduced (0, 10));
}

void LookAndFeel_V4::drawFileBrowserRow (Graphics& g, int width, int height,
                                         const File& file, const String& filename, Image* icon,
                                         const String& fileSizeDescription,
                                         const String& fileTimeDescription,
                                         bool isDirectory, bool isItemSelected,
                                         int itemIndex, DirectoryContentsDisplayComponent& dcc)
{
    LookAndFeel_V2::drawFileBrowserRow (g, width, height, file, filename, icon,
                                        fileSizeDescription, fileTimeDescription,
                                        isDirectory, isItemSelected, itemIndex, dcc);
}

//==============================================================================
void LookAndFeel_V4::drawPopupMenuItem (Graphics& g, const Rectangle<int>& area,
                                        const bool isSeparator, const bool isActive,
                                        const bool isHighlighted, const bool isTicked,
                                        const bool hasSubMenu, const String& text,
                                        const String& shortcutKeyText,
                                        const Drawable* icon, const Color* const textColorToUse)
{
    if (isSeparator)
    {
        auto r  = area.reduced (5, 0);
        r.removeFromTop (roundToInt ((r.getHeight() * 0.5f) - 0.5f));

        g.setColor (findColor (PopupMenu::textColorId).withAlpha (0.3f));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        auto textColor = (textColorToUse == nullptr ? findColor (PopupMenu::textColorId)
                                                      : *textColorToUse);

        auto r  = area.reduced (1);

        if (isHighlighted && isActive)
        {
            g.setColor (findColor (PopupMenu::highlightedBackgroundColorId));
            g.fillRect (r);

            g.setColor (findColor (PopupMenu::highlightedTextColorId));
        }
        else
        {
            g.setColor (textColor.withMultipliedAlpha (isActive ? 1.0f : 0.5f));
        }

        r.reduce (jmin (5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        auto maxFontHeight = r.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);

        g.setFont (font);

        auto iconArea = r.removeFromLeft (roundToInt (maxFontHeight)).toFloat();

        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, RectanglePlacement::centered | RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft (roundToInt (maxFontHeight * 0.5f));
        }
        else if (isTicked)
        {
            auto tick = getTickShape (1.0f);
            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea.reduced (iconArea.getWidth() / 5, 0).toFloat(), true));
        }

        if (hasSubMenu)
        {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = static_cast<float> (r.removeFromRight ((int) arrowH).getX());
            auto halfH = static_cast<float> (r.getCenterY());

            Path path;
            path.startNewSubPath (x, halfH - arrowH * 0.5f);
            path.lineTo (x + arrowH * 0.6f, halfH);
            path.lineTo (x, halfH + arrowH * 0.5f);

            g.strokePath (path, PathStrokeType (2.0f));
        }

        r.removeFromRight (3);
        g.drawFittedText (text, r, Justification::centeredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            auto f2 = font;
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText, r, Justification::centeredRight, true);
        }
    }
}

void LookAndFeel_V4::getIdealPopupMenuItemSize (const String& text, const bool isSeparator,
                                                int standardMenuItemHeight, int& idealWidth, int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 10 : 10;
    }
    else
    {
        auto font = getPopupMenuFont();

        if (standardMenuItemHeight > 0 && font.getHeight() > standardMenuItemHeight / 1.3f)
            font.setHeight (standardMenuItemHeight / 1.3f);

        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt (font.getHeight() * 1.3f);
        idealWidth = font.getStringWidth (text) + idealHeight * 2;
    }
}

void LookAndFeel_V4::drawMenuBarBackground (Graphics& g, int width, int height,
                                            bool, MenuBarComponent& menuBar)
{
    auto color = menuBar.findColor (TextButton::buttonColorId).withAlpha (0.4f);

    Rectangle<int> r (width, height);

    g.setColor (color.contrasting (0.15f));
    g.fillRect  (r.removeFromTop (1));
    g.fillRect  (r.removeFromBottom (1));

    g.setGradientFill (ColorGradient::vertical (color, 0, color.darker (0.2f), (float) height));
    g.fillRect (r);
}

void LookAndFeel_V4::drawMenuBarItem (Graphics& g, int width, int height,
                                      int itemIndex, const String& itemText,
                                      bool isMouseOverItem, bool isMenuOpen,
                                      bool /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColor (menuBar.findColor (TextButton::textColorOffId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll   (menuBar.findColor (TextButton::buttonOnColorId));
        g.setColor (menuBar.findColor (TextButton::textColorOnId));
    }
    else
    {
        g.setColor (menuBar.findColor (TextButton::textColorOffId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centered, 1);
}

//==============================================================================
void LookAndFeel_V4::drawComboBox (Graphics& g, int width, int height, bool,
                                   int, int, int, int, ComboBox& box)
{
    auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
    Rectangle<int> boxBounds (0, 0, width, height);

    g.setColor (box.findColor (ComboBox::backgroundColorId));
    g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

    g.setColor (box.findColor (ComboBox::outlineColorId));
    g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    Rectangle<int> arrowZone (width - 30, 0, 20, height);
    Path path;
    path.startNewSubPath (arrowZone.getX() + 3.0f, arrowZone.getCenterY() - 2.0f);
    path.lineTo (static_cast<float> (arrowZone.getCenterX()), arrowZone.getCenterY() + 3.0f);
    path.lineTo (arrowZone.getRight() - 3.0f, arrowZone.getCenterY() - 2.0f);

    g.setColor (box.findColor (ComboBox::arrowColorId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
    g.strokePath (path, PathStrokeType (2.0f));
}

Font LookAndFeel_V4::getComboBoxFont (ComboBox& box)
{
    return { jmin (16.0f, box.getHeight() * 0.85f) };
}

void LookAndFeel_V4::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (1, 1,
                     box.getWidth() - 30,
                     box.getHeight() - 2);

    label.setFont (getComboBoxFont (box));
}

//==============================================================================
int LookAndFeel_V4::getSliderThumbRadius (Slider& slider)
{
    return jmin (12, slider.isHorizontal() ? static_cast<int> (slider.getHeight() * 0.5f)
                                           : static_cast<int> (slider.getWidth()  * 0.5f));
}

void LookAndFeel_V4::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    if (slider.isBar())
    {
        g.setColor (slider.findColor (Slider::trackColorId));
        g.fillRect (slider.isHorizontal() ? Rectangle<float> (static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                                          : Rectangle<float> (x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
    }
    else
    {
        auto isTwoVal   = (style == Slider::SliderStyle::TwoValueVertical   || style == Slider::SliderStyle::TwoValueHorizontal);
        auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

        auto trackWidth = jmin (6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

        Point<float> startPoint (slider.isHorizontal() ? x : x + width * 0.5f,
                                 slider.isHorizontal() ? y + height * 0.5f : height + y);

        Point<float> endPoint (slider.isHorizontal() ? width + x : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : y);

        Path backgroundTrack;
        backgroundTrack.startNewSubPath (startPoint);
        backgroundTrack.lineTo (endPoint);
        g.setColor (slider.findColor (Slider::backgroundColorId));
        g.strokePath (backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        Path valueTrack;
        Point<float> minPoint, maxPoint, thumbPoint;

        if (isTwoVal || isThreeVal)
        {
            minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                         slider.isHorizontal() ? height * 0.5f : minSliderPos };

            if (isThreeVal)
                thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
                               slider.isHorizontal() ? height * 0.5f : sliderPos };

            maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                         slider.isHorizontal() ? height * 0.5f : maxSliderPos };
        }
        else
        {
            auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
            auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

            minPoint = startPoint;
            maxPoint = { kx, ky };
        }

        auto thumbWidth = getSliderThumbRadius (slider);

        valueTrack.startNewSubPath (minPoint);
        valueTrack.lineTo (isThreeVal ? thumbPoint : maxPoint);
        g.setColor (slider.findColor (Slider::trackColorId));
        g.strokePath (valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

        if (! isTwoVal)
        {
            g.setColor (slider.findColor (Slider::thumbColorId));
            g.fillEllipse (Rectangle<float> (static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCenter (isThreeVal ? thumbPoint : maxPoint));
        }

        if (isTwoVal || isThreeVal)
        {
            auto sr = jmin (trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
            auto pointerColor = slider.findColor (Slider::thumbColorId);

            if (slider.isHorizontal())
            {
                drawPointer (g, minSliderPos - sr,
                             jmax (0.0f, y + height * 0.5f - trackWidth * 2.0f),
                             trackWidth * 2.0f, pointerColor, 2);

                drawPointer (g, maxSliderPos - trackWidth,
                             jmin (y + height - trackWidth * 2.0f, y + height * 0.5f),
                             trackWidth * 2.0f, pointerColor, 4);
            }
            else
            {
                drawPointer (g, jmax (0.0f, x + width * 0.5f - trackWidth * 2.0f),
                             minSliderPos - trackWidth,
                             trackWidth * 2.0f, pointerColor, 1);

                drawPointer (g, jmin (x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                             trackWidth * 2.0f, pointerColor, 3);
            }
        }
    }
}

void LookAndFeel_V4::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    auto outline = slider.findColor (Slider::rotarySliderOutlineColorId);
    auto fill    = slider.findColor (Slider::rotarySliderFillColorId);

    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (10);

    auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    Path backgroundArc;
    backgroundArc.addCenteredArc (bounds.getCenterX(),
                                 bounds.getCenterY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColor (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCenteredArc (bounds.getCenterX(),
                                bounds.getCenterY(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotaryStartAngle,
                                toAngle,
                                true);

        g.setColor (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    auto thumbWidth = lineW * 2.0f;
    Point<float> thumbPoint (bounds.getCenterX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi),
                             bounds.getCenterY() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));

    g.setColor (slider.findColor (Slider::thumbColorId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCenter (thumbPoint));
}

void LookAndFeel_V4::drawPointer (Graphics& g, const float x, const float y, const float diameter,
                                  const Color& color, const int direction) noexcept
{
    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation (direction * MathConstants<float>::halfPi,
                                                 x + diameter * 0.5f, y + diameter * 0.5f));
    g.setColor (color);
    g.fillPath (p);
}

Label* LookAndFeel_V4::createSliderTextBox (Slider& slider)
{
    auto* l = LookAndFeel_V2::createSliderTextBox (slider);

    if (getCurrentColorScheme() == LookAndFeel_V4::getGrayColorScheme() && (slider.getSliderStyle() == Slider::LinearBar
                                                                               || slider.getSliderStyle() == Slider::LinearBarVertical))
    {
        l->setColor (Label::textColorId, Colors::black.withAlpha (0.7f));
    }

    return l;
}

//==============================================================================
void LookAndFeel_V4::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    Rectangle<int> bounds (width, height);
    auto cornerSize = 5.0f;

    g.setColor (findColor (TooltipWindow::backgroundColorId));
    g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

    g.setColor (findColor (TooltipWindow::outlineColorId));
    g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

    LookAndFeelHelpers::layoutTooltipText (text, findColor (TooltipWindow::textColorId))
                       .draw (g, { static_cast<float> (width), static_cast<float> (height) });
}

//==============================================================================
void LookAndFeel_V4::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                                bool isMouseOver, bool /*isMouseDown*/,
                                                ConcertinaPanel& concertina, Component& panel)
{
    auto bounds = area.toFloat().reduced (0.5f);
    auto cornerSize = 4.0f;
    auto isTopPanel = (concertina.getPanel (0) == &panel);

    Path p;
    p.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                           cornerSize, cornerSize, isTopPanel, isTopPanel, false, false);

    auto bkg = Colors::gray;

    g.setGradientFill (ColorGradient::vertical (Colors::white.withAlpha (isMouseOver ? 0.4f : 0.2f), static_cast<float> (area.getY()),
                                                 Colors::darkgray.withAlpha (0.1f), static_cast<float> (area.getBottom())));
    g.fillPath (p);
}

//==============================================================================
void LookAndFeel_V4::drawLevelMeter (Graphics& g, int width, int height, float level)
{
    auto outerCornerSize = 3.0f;
    auto outerBorderWidth = 2.0f;
    auto totalBlocks = 7;
    auto spacingFraction = 0.03f;

    g.setColor (findColor (ResizableWindow::backgroundColorId));
    g.fillRoundedRectangle (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height), outerCornerSize);

    auto doubleOuterBorderWidth = 2.0f * outerBorderWidth;
    auto numBlocks = roundToInt (totalBlocks * level);

    auto blockWidth = (width - doubleOuterBorderWidth) / static_cast<float> (totalBlocks);
    auto blockHeight = height - doubleOuterBorderWidth;

    auto blockRectWidth = (1.0f - 2.0f * spacingFraction) * blockWidth;
    auto blockRectSpacing = spacingFraction * blockWidth;

    auto blockCornerSize = 0.1f * blockWidth;

    auto c = findColor (Slider::thumbColorId);

    for (auto i = 0; i < totalBlocks; ++i)
    {
        if (i >= numBlocks)
            g.setColor (c.withAlpha (0.5f));
        else
            g.setColor (i < totalBlocks - 1 ? c : Colors::red);

        g.fillRoundedRectangle (outerBorderWidth + (i * blockWidth) + blockRectSpacing,
                                outerBorderWidth,
                                blockRectWidth,
                                blockHeight,
                                blockCornerSize);
    }
}

//==============================================================================
void LookAndFeel_V4::paintToolbarBackground (Graphics& g, int w, int h, Toolbar& toolbar)
{
    auto background = toolbar.findColor (Toolbar::backgroundColorId);

    g.setGradientFill ({ background, 0.0f, 0.0f,
                         background.darker (0.2f),
                         toolbar.isVertical() ? w - 1.0f : 0.0f,
                         toolbar.isVertical() ? 0.0f : h - 1.0f,
                         false });
    g.fillAll();
}

void LookAndFeel_V4::paintToolbarButtonLabel (Graphics& g, int x, int y, int width, int height,
                                              const String& text, ToolbarItemComponent& component)
{
    auto baseTextColor = component.findParentComponentOfClass<PopupMenu::CustomComponent>() != nullptr
                              ? component.findColor (PopupMenu::textColorId)
                              : component.findColor (Toolbar::labelTextColorId);

    g.setColor (baseTextColor.withAlpha (component.isEnabled() ? 1.0f : 0.25f));

    auto fontHeight = jmin (14.0f, height * 0.85f);
    g.setFont (fontHeight);

    g.drawFittedText (text,
                      x, y, width, height,
                      Justification::centered,
                      jmax (1, height / (int) fontHeight));
}

//==============================================================================
void LookAndFeel_V4::drawPropertyPanelSectionHeader (Graphics& g, const String& name,
                                                     bool isOpen, int width, int height)
{
    auto buttonSize = height * 0.75f;
    auto buttonIndent = (height - buttonSize) * 0.5f;

    drawTreeviewPlusMinusBox (g, { buttonIndent, buttonIndent, buttonSize, buttonSize },
                              findColor (ResizableWindow::backgroundColorId), isOpen, false);

    auto textX = static_cast<int> ((buttonIndent * 2.0f + buttonSize + 2.0f));

    g.setColor (findColor (PropertyComponent::labelTextColorId));

    g.setFont ({ height * 0.7f, Font::bold });
    g.drawText (name, textX, 0, width - textX - 4, height, Justification::centeredLeft, true);
}

void LookAndFeel_V4::drawPropertyComponentBackground (Graphics& g, int width, int height, PropertyComponent& component)
{
    g.setColor (component.findColor (PropertyComponent::backgroundColorId));
    g.fillRect  (0, 0, width, height - 1);
}

void LookAndFeel_V4::drawPropertyComponentLabel (Graphics& g, int width, int height, PropertyComponent& component)
{
    ignoreUnused (width);

    auto indent = getPropertyComponentIndent (component);

    g.setColor (component.findColor (PropertyComponent::labelTextColorId)
                          .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    g.setFont (jmin (height, 24) * 0.65f);

    auto r = getPropertyComponentContentPosition (component);

    g.drawFittedText (component.getName(),
                      indent, r.getY(), r.getX() - 5, r.getHeight(),
                      Justification::centeredLeft, 2);
}

int LookAndFeel_V4::getPropertyComponentIndent (PropertyComponent& component)
{
    return jmin (10, component.getWidth() / 10);
}

Rectangle<int> LookAndFeel_V4::getPropertyComponentContentPosition (PropertyComponent& component)
{
    auto textW = jmin (200, component.getWidth() / 2);
    return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
}

//==============================================================================
void LookAndFeel_V4::drawCallOutBoxBackground (CallOutBox& box, Graphics& g,
                                               const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = { Image::ARGB, box.getWidth(), box.getHeight(), true };
        Graphics g2 (cachedImage);

        DropShadow (Colors::black.withAlpha (0.7f), 8, { 0, 2 }).drawForPath (g2, path);
    }

    g.setColor (Colors::black);
    g.drawImageAt (cachedImage, 0, 0);

    g.setColor (currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.8f));
    g.fillPath (path);

    g.setColor (currentColorScheme.getUIColor (ColorScheme::UIColor::outline).withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (2.0f));
}

//==============================================================================
void LookAndFeel_V4::drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/,
                                      bool isMouseOver, bool isMouseDragging)
{
    if (isMouseOver || isMouseDragging)
        g.fillAll (currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.5f));
}

//==============================================================================
void LookAndFeel_V4::initializeColors()
{
    const uint32 transparent = 0x00000000;

    const uint32 colorsToUse[] =
    {
        TextButton::buttonColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextButton::buttonOnColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        TextButton::textColorOnId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TextButton::textColorOffId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        ToggleButton::textColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ToggleButton::tickColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ToggleButton::tickDisabledColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).withAlpha (0.5f).getARGB(),

        TextEditor::backgroundColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextEditor::textColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        TextEditor::highlightColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        TextEditor::highlightedTextColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TextEditor::outlineColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TextEditor::focusedOutlineColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TextEditor::shadowColorId,                 transparent,

        CaretComponent::caretColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),

        Label::backgroundColorId,                  transparent,
        Label::textColorId,                        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Label::outlineColorId,                     transparent,
        Label::textWhenEditingColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        ScrollBar::backgroundColorId,              transparent,
        ScrollBar::thumbColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        ScrollBar::trackColorId,                   transparent,

        TreeView::linesColorId,                    transparent,
        TreeView::backgroundColorId,               transparent,
        TreeView::dragAndDropIndicatorColorId,     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TreeView::selectedItemBackgroundColorId,   transparent,
        TreeView::oddItemsColorId,                 transparent,
        TreeView::evenItemsColorId,                transparent,

        PopupMenu::backgroundColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        PopupMenu::textColorId,                    currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        PopupMenu::headerTextColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        PopupMenu::highlightedTextColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        PopupMenu::highlightedBackgroundColorId,   currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        ComboBox::buttonColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ComboBox::outlineColorId,                  currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ComboBox::textColorId,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ComboBox::backgroundColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ComboBox::arrowColorId,                    currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        ComboBox::focusedOutlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        PropertyComponent::backgroundColorId,      currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        PropertyComponent::labelTextColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        TextPropertyComponent::backgroundColorId,  currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        TextPropertyComponent::textColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        TextPropertyComponent::outlineColorId,     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        BooleanPropertyComponent::backgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        BooleanPropertyComponent::outlineColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ListBox::backgroundColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ListBox::outlineColorId,                   currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        ListBox::textColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        Slider::backgroundColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        Slider::thumbColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        Slider::trackColorId,                      currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        Slider::rotarySliderFillColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        Slider::rotarySliderOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        Slider::textBoxTextColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Slider::textBoxBackgroundColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.0f).getARGB(),
        Slider::textBoxHighlightColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        Slider::textBoxOutlineColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ResizableWindow::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::windowBackground).getARGB(),

        DocumentWindow::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        AlertWindow::backgroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        AlertWindow::textColorId,                  currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        AlertWindow::outlineColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        ProgressBar::backgroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        ProgressBar::foregroundColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        TooltipWindow::backgroundColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        TooltipWindow::textColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        TooltipWindow::outlineColorId,             transparent,

        TabbedComponent::backgroundColorId,        transparent,
        TabbedComponent::outlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        TabbedButtonBar::tabOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::outline).withAlpha (0.5f).getARGB(),
        TabbedButtonBar::frontOutlineColorId,      currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        Toolbar::backgroundColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).withAlpha (0.4f).getARGB(),
        Toolbar::separatorColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        Toolbar::buttonMouseOverBackgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).contrasting (0.2f).getARGB(),
        Toolbar::buttonMouseDownBackgroundColorId, currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).contrasting (0.5f).getARGB(),
        Toolbar::labelTextColorId,                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        Toolbar::editingModeOutlineColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        DrawableButton::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        DrawableButton::textColorOnId,             currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),
        DrawableButton::backgroundColorId,         transparent,
        DrawableButton::backgroundOnColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),

        HyperlinkButton::textColorId,              currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).interpolatedWith (Colors::blue, 0.4f).getARGB(),

        GroupComponent::outlineColorId,            currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),
        GroupComponent::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        BubbleComponent::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        BubbleComponent::outlineColorId,           currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        DirectoryContentsDisplayComponent::highlightColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).getARGB(),
        DirectoryContentsDisplayComponent::textColorId,               currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        DirectoryContentsDisplayComponent::highlightedTextColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedText).getARGB(),

        0x1000440, /*LassoComponent::lassoFillColorId*/        currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        0x1000441, /*LassoComponent::lassoOutlineColorId*/     currentColorScheme.getUIColor (ColorScheme::UIColor::outline).getARGB(),

        0x1005000, /*MidiKeyboardComponent::whiteNoteColorId*/               0xffffffff,
        0x1005001, /*MidiKeyboardComponent::blackNoteColorId*/               0xff000000,
        0x1005002, /*MidiKeyboardComponent::keySeparatorLineColorId*/        0x66000000,
        0x1005003, /*MidiKeyboardComponent::mouseOverKeyOverlayColorId*/     0x80ffff00,
        0x1005004, /*MidiKeyboardComponent::keyDownOverlayColorId*/          0xffb6b600,
        0x1005005, /*MidiKeyboardComponent::textLabelColorId*/               0xff000000,
        0x1005006, /*MidiKeyboardComponent::upDownButtonBackgroundColorId*/  0xffd3d3d3,
        0x1005007, /*MidiKeyboardComponent::upDownButtonArrowColorId*/       0xff000000,
        0x1005008, /*MidiKeyboardComponent::shadowColorId*/                  0x4c000000,

        0x1004500, /*CodeEditorComponent::backgroundColorId*/                currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x1004502, /*CodeEditorComponent::highlightColorId*/                 currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).withAlpha (0.4f).getARGB(),
        0x1004503, /*CodeEditorComponent::defaultTextColorId*/               currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        0x1004504, /*CodeEditorComponent::lineNumberBackgroundId*/            currentColorScheme.getUIColor (ColorScheme::UIColor::highlightedFill).withAlpha (0.5f).getARGB(),
        0x1004505, /*CodeEditorComponent::lineNumberTextId*/                  currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),

        0x1007000, /*ColorSelector::backgroundColorId*/                     currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x1007001, /*ColorSelector::labelTextColorId*/                      currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        0x100ad00, /*KeyMappingEditorComponent::backgroundColorId*/          currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        0x100ad01, /*KeyMappingEditorComponent::textColorId*/                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        FileSearchPathListComponent::backgroundColorId,        currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),

        FileChooserDialogBox::titleTextColorId,                currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),

        SidePanel::backgroundColor,                            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).getARGB(),
        SidePanel::titleTextColor,                             currentColorScheme.getUIColor (ColorScheme::UIColor::defaultText).getARGB(),
        SidePanel::shadowBaseColor,                            currentColorScheme.getUIColor (ColorScheme::UIColor::widgetBackground).darker().getARGB(),
        SidePanel::dismissButtonNormalColor,                   currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).getARGB(),
        SidePanel::dismissButtonOverColor,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).darker().getARGB(),
        SidePanel::dismissButtonDownColor,                     currentColorScheme.getUIColor (ColorScheme::UIColor::defaultFill).brighter().getARGB(),

        FileBrowserComponent::currentPathBoxBackgroundColorId,    currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        FileBrowserComponent::currentPathBoxTextColorId,          currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        FileBrowserComponent::currentPathBoxArrowColorId,         currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
        FileBrowserComponent::filenameBoxBackgroundColorId,       currentColorScheme.getUIColor (ColorScheme::UIColor::menuBackground).getARGB(),
        FileBrowserComponent::filenameBoxTextColorId,             currentColorScheme.getUIColor (ColorScheme::UIColor::menuText).getARGB(),
    };

    for (int i = 0; i < numElementsInArray (colorsToUse); i += 2)
        setColor ((int) colorsToUse [i], Color ((uint32) colorsToUse [i + 1]));
}

} // namespace juce
