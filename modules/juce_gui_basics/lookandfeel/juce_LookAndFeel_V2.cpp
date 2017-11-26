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

namespace juce
{

namespace LookAndFeelHelpers
{
    static Colour createBaseColour (Colour buttonColour,
                                    bool hasKeyboardFocus,
                                    bool isMouseOverButton,
                                    bool isButtonDown) noexcept
    {
        const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
        const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

        if (isButtonDown)      return baseColour.contrasting (0.2f);
        if (isMouseOverButton) return baseColour.contrasting (0.1f);

        return baseColour;
    }

    static TextLayout layoutTooltipText (const String& text, Colour colour) noexcept
    {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 400;

        AttributedString s;
        s.setJustification (Justification::centred);
        s.append (text, Font (tooltipFontSize, Font::bold), colour);

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
        return tl;
    }
}

//==============================================================================
LookAndFeel_V2::LookAndFeel_V2()
{
    // initialise the standard set of colours..
    const uint32 textButtonColour      = 0xffbbbbff;
    const uint32 textHighlightColour   = 0x401111ee;
    const uint32 standardOutlineColour = 0xb2808080;

    static const uint32 standardColours[] =
    {
        TextButton::buttonColourId,                 textButtonColour,
        TextButton::buttonOnColourId,               0xff4444ff,
        TextButton::textColourOnId,                 0xff000000,
        TextButton::textColourOffId,                0xff000000,

        ToggleButton::textColourId,                 0xff000000,
        ToggleButton::tickColourId,                 0xff000000,
        ToggleButton::tickDisabledColourId,         0xff808080,

        TextEditor::backgroundColourId,             0xffffffff,
        TextEditor::textColourId,                   0xff000000,
        TextEditor::highlightColourId,              textHighlightColour,
        TextEditor::highlightedTextColourId,        0xff000000,
        TextEditor::outlineColourId,                0x00000000,
        TextEditor::focusedOutlineColourId,         textButtonColour,
        TextEditor::shadowColourId,                 0x38000000,

        CaretComponent::caretColourId,              0xff000000,

        Label::backgroundColourId,                  0x00000000,
        Label::textColourId,                        0xff000000,
        Label::outlineColourId,                     0x00000000,

        ScrollBar::backgroundColourId,              0x00000000,
        ScrollBar::thumbColourId,                   0xffffffff,

        TreeView::linesColourId,                    0x4c000000,
        TreeView::backgroundColourId,               0x00000000,
        TreeView::dragAndDropIndicatorColourId,     0x80ff0000,
        TreeView::selectedItemBackgroundColourId,   0x00000000,
        TreeView::oddItemsColourId,                 0x00000000,
        TreeView::evenItemsColourId,                0x00000000,

        PopupMenu::backgroundColourId,              0xffffffff,
        PopupMenu::textColourId,                    0xff000000,
        PopupMenu::headerTextColourId,              0xff000000,
        PopupMenu::highlightedTextColourId,         0xffffffff,
        PopupMenu::highlightedBackgroundColourId,   0x991111aa,

        ComboBox::buttonColourId,                   0xffbbbbff,
        ComboBox::outlineColourId,                  standardOutlineColour,
        ComboBox::textColourId,                     0xff000000,
        ComboBox::backgroundColourId,               0xffffffff,
        ComboBox::arrowColourId,                    0x99000000,

        PropertyComponent::backgroundColourId,      0x66ffffff,
        PropertyComponent::labelTextColourId,       0xff000000,

        TextPropertyComponent::backgroundColourId,  0xffffffff,
        TextPropertyComponent::textColourId,        0xff000000,
        TextPropertyComponent::outlineColourId,     standardOutlineColour,

        BooleanPropertyComponent::backgroundColourId, 0xffffffff,
        BooleanPropertyComponent::outlineColourId,  standardOutlineColour,

        ListBox::backgroundColourId,                0xffffffff,
        ListBox::outlineColourId,                   standardOutlineColour,
        ListBox::textColourId,                      0xff000000,

        Slider::backgroundColourId,                 0x00000000,
        Slider::thumbColourId,                      textButtonColour,
        Slider::trackColourId,                      0x7fffffff,
        Slider::rotarySliderFillColourId,           0x7f0000ff,
        Slider::rotarySliderOutlineColourId,        0x66000000,
        Slider::textBoxTextColourId,                0xff000000,
        Slider::textBoxBackgroundColourId,          0xffffffff,
        Slider::textBoxHighlightColourId,           textHighlightColour,
        Slider::textBoxOutlineColourId,             standardOutlineColour,

        ResizableWindow::backgroundColourId,        0xff777777,
        //DocumentWindow::textColourId,               0xff000000, // (this is deliberately not set)

        AlertWindow::backgroundColourId,            0xffededed,
        AlertWindow::textColourId,                  0xff000000,
        AlertWindow::outlineColourId,               0xff666666,

        ProgressBar::backgroundColourId,            0xffeeeeee,
        ProgressBar::foregroundColourId,            0xffaaaaee,

        TooltipWindow::backgroundColourId,          0xffeeeebb,
        TooltipWindow::textColourId,                0xff000000,
        TooltipWindow::outlineColourId,             0x4c000000,

        TabbedComponent::backgroundColourId,        0x00000000,
        TabbedComponent::outlineColourId,           0xff777777,
        TabbedButtonBar::tabOutlineColourId,        0x80000000,
        TabbedButtonBar::frontOutlineColourId,      0x90000000,

        Toolbar::backgroundColourId,                0xfff6f8f9,
        Toolbar::separatorColourId,                 0x4c000000,
        Toolbar::buttonMouseOverBackgroundColourId, 0x4c0000ff,
        Toolbar::buttonMouseDownBackgroundColourId, 0x800000ff,
        Toolbar::labelTextColourId,                 0xff000000,
        Toolbar::editingModeOutlineColourId,        0xffff0000,

        DrawableButton::textColourId,               0xff000000,
        DrawableButton::textColourOnId,             0xff000000,
        DrawableButton::backgroundColourId,         0x00000000,
        DrawableButton::backgroundOnColourId,       0xaabbbbff,

        HyperlinkButton::textColourId,              0xcc1111ee,

        GroupComponent::outlineColourId,            0x66000000,
        GroupComponent::textColourId,               0xff000000,

        BubbleComponent::backgroundColourId,        0xeeeeeebb,
        BubbleComponent::outlineColourId,           0x77000000,

        TableHeaderComponent::textColourId,         0xff000000,
        TableHeaderComponent::backgroundColourId,   0xffe8ebf9,
        TableHeaderComponent::outlineColourId,      0x33000000,
        TableHeaderComponent::highlightColourId,    0x8899aadd,

        DirectoryContentsDisplayComponent::highlightColourId,   textHighlightColour,
        DirectoryContentsDisplayComponent::textColourId,        0xff000000,

        0x1000440, /*LassoComponent::lassoFillColourId*/        0x66dddddd,
        0x1000441, /*LassoComponent::lassoOutlineColourId*/     0x99111111,

        0x1005000, /*MidiKeyboardComponent::whiteNoteColourId*/               0xffffffff,
        0x1005001, /*MidiKeyboardComponent::blackNoteColourId*/               0xff000000,
        0x1005002, /*MidiKeyboardComponent::keySeparatorLineColourId*/        0x66000000,
        0x1005003, /*MidiKeyboardComponent::mouseOverKeyOverlayColourId*/     0x80ffff00,
        0x1005004, /*MidiKeyboardComponent::keyDownOverlayColourId*/          0xffb6b600,
        0x1005005, /*MidiKeyboardComponent::textLabelColourId*/               0xff000000,
        0x1005006, /*MidiKeyboardComponent::upDownButtonBackgroundColourId*/  0xffd3d3d3,
        0x1005007, /*MidiKeyboardComponent::upDownButtonArrowColourId*/       0xff000000,
        0x1005008, /*MidiKeyboardComponent::shadowColourId*/                  0x4c000000,

        0x1004500, /*CodeEditorComponent::backgroundColourId*/                0xffffffff,
        0x1004502, /*CodeEditorComponent::highlightColourId*/                 textHighlightColour,
        0x1004503, /*CodeEditorComponent::defaultTextColourId*/               0xff000000,
        0x1004504, /*CodeEditorComponent::lineNumberBackgroundId*/            0x44999999,
        0x1004505, /*CodeEditorComponent::lineNumberTextId*/                  0x44000000,

        0x1007000, /*ColourSelector::backgroundColourId*/                     0xffe5e5e5,
        0x1007001, /*ColourSelector::labelTextColourId*/                      0xff000000,

        0x100ad00, /*KeyMappingEditorComponent::backgroundColourId*/          0x00000000,
        0x100ad01, /*KeyMappingEditorComponent::textColourId*/                0xff000000,

        FileSearchPathListComponent::backgroundColourId,        0xffffffff,

        FileChooserDialogBox::titleTextColourId,                0xff000000,
    };

    for (int i = 0; i < numElementsInArray (standardColours); i += 2)
        setColour ((int) standardColours [i], Colour ((uint32) standardColours [i + 1]));
}

LookAndFeel_V2::~LookAndFeel_V2()  {}

//==============================================================================
void LookAndFeel_V2::drawButtonBackground (Graphics& g,
                                           Button& button,
                                           const Colour& backgroundColour,
                                           bool isMouseOverButton,
                                           bool isButtonDown)
{
    const int width = button.getWidth();
    const int height = button.getHeight();

    const float outlineThickness = button.isEnabled() ? ((isButtonDown || isMouseOverButton) ? 1.2f : 0.7f) : 0.4f;
    const float halfThickness = outlineThickness * 0.5f;

    const float indentL = button.isConnectedOnLeft()   ? 0.1f : halfThickness;
    const float indentR = button.isConnectedOnRight()  ? 0.1f : halfThickness;
    const float indentT = button.isConnectedOnTop()    ? 0.1f : halfThickness;
    const float indentB = button.isConnectedOnBottom() ? 0.1f : halfThickness;

    const Colour baseColour (LookAndFeelHelpers::createBaseColour (backgroundColour,
                                                                   button.hasKeyboardFocus (true),
                                                                   isMouseOverButton, isButtonDown)
                               .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    drawGlassLozenge (g,
                      indentL,
                      indentT,
                      width - indentL - indentR,
                      height - indentT - indentB,
                      baseColour, outlineThickness, -1.0f,
                      button.isConnectedOnLeft(),
                      button.isConnectedOnRight(),
                      button.isConnectedOnTop(),
                      button.isConnectedOnBottom());
}

Font LookAndFeel_V2::getTextButtonFont (TextButton&, int buttonHeight)
{
    return Font (jmin (15.0f, buttonHeight * 0.6f));
}

int LookAndFeel_V2::getTextButtonWidthToFitText (TextButton& b, int buttonHeight)
{
    return getTextButtonFont (b, buttonHeight).getStringWidth (b.getButtonText()) + buttonHeight;
}

void LookAndFeel_V2::drawButtonText (Graphics& g, TextButton& button, bool /*isMouseOverButton*/, bool /*isButtonDown*/)
{
    Font font (getTextButtonFont (button, button.getHeight()));
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (font.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

void LookAndFeel_V2::drawTickBox (Graphics& g, Component& component,
                                  float x, float y, float w, float h,
                                  const bool ticked,
                                  const bool isEnabled,
                                  const bool isMouseOverButton,
                                  const bool isButtonDown)
{
    const float boxSize = w * 0.7f;

    drawGlassSphere (g, x, y + (h - boxSize) * 0.5f, boxSize,
                     LookAndFeelHelpers::createBaseColour (component.findColour (TextButton::buttonColourId)
                                                                    .withMultipliedAlpha (isEnabled ? 1.0f : 0.5f),
                                                           true, isMouseOverButton, isButtonDown),
                     isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColour (component.findColour (isEnabled ? ToggleButton::tickColourId
                                                     : ToggleButton::tickDisabledColourId));

        const AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                                     .translated (x, y));

        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

void LookAndFeel_V2::drawToggleButton (Graphics& g, ToggleButton& button,
                                       bool isMouseOverButton, bool isButtonDown)
{
    if (button.hasKeyboardFocus (true))
    {
        g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

    float fontSize = jmin (15.0f, button.getHeight() * 0.75f);
    const float tickWidth = fontSize * 1.1f;

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (fontSize);

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    g.drawFittedText (button.getButtonText(),
                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 5)
                                             .withTrimmedRight (2),
                      Justification::centredLeft, 10);
}

void LookAndFeel_V2::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    Font font (jmin (15.0f, button.getHeight() * 0.6f));

    const int tickWidth = jmin (24, button.getHeight());

    button.setSize (font.getStringWidth (button.getButtonText()) + tickWidth + 8,
                    button.getHeight());
}

void LookAndFeel_V2::drawDrawableButton (Graphics& g, DrawableButton& button,
                                         bool /*isMouseOverButton*/, bool /*isButtonDown*/)
{
    bool toggleState = button.getToggleState();

    g.fillAll (button.findColour (toggleState ? DrawableButton::backgroundOnColourId
                                              : DrawableButton::backgroundColourId));

    const int textH = (button.getStyle() == DrawableButton::ImageAboveTextLabel)
                        ? jmin (16, button.proportionOfHeight (0.25f))
                        : 0;

    if (textH > 0)
    {
        g.setFont ((float) textH);

        g.setColour (button.findColour (toggleState ? DrawableButton::textColourOnId
                                                    : DrawableButton::textColourId)
                        .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.4f));

        g.drawFittedText (button.getButtonText(),
                          2, button.getHeight() - textH - 1,
                          button.getWidth() - 4, textH,
                          Justification::centred, 1);
    }
}

//==============================================================================
AlertWindow* LookAndFeel_V2::createAlertWindow (const String& title, const String& message,
                                                const String& button1, const String& button2, const String& button3,
                                                AlertWindow::AlertIconType iconType,
                                                int numButtons, Component* associatedComponent)
{
    AlertWindow* aw = new AlertWindow (title, message, iconType, associatedComponent);

    if (numButtons == 1)
    {
        aw->addButton (button1, 0,
                       KeyPress (KeyPress::escapeKey),
                       KeyPress (KeyPress::returnKey));
    }
    else
    {
        const KeyPress button1ShortCut ((int) CharacterFunctions::toLowerCase (button1[0]), 0, 0);
        KeyPress button2ShortCut ((int) CharacterFunctions::toLowerCase (button2[0]), 0, 0);
        if (button1ShortCut == button2ShortCut)
            button2ShortCut = KeyPress();

        if (numButtons == 2)
        {
            aw->addButton (button1, 1, KeyPress (KeyPress::returnKey), button1ShortCut);
            aw->addButton (button2, 0, KeyPress (KeyPress::escapeKey), button2ShortCut);
        }
        else if (numButtons == 3)
        {
            aw->addButton (button1, 1, button1ShortCut);
            aw->addButton (button2, 2, button2ShortCut);
            aw->addButton (button3, 0, KeyPress (KeyPress::escapeKey));
        }
    }

    return aw;
}

void LookAndFeel_V2::drawAlertBox (Graphics& g, AlertWindow& alert,
                                   const Rectangle<int>& textArea, TextLayout& textLayout)
{
    g.fillAll (alert.findColour (AlertWindow::backgroundColourId));

    int iconSpaceUsed = 0;

    const int iconWidth = 80;
    int iconSize = jmin (iconWidth + 50, alert.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    const Rectangle<int> iconRect (iconSize / -10, iconSize / -10,
                                   iconSize, iconSize);

    if (alert.getAlertType() != AlertWindow::NoIcon)
    {
        Path icon;
        uint32 colour;
        char character;

        if (alert.getAlertType() == AlertWindow::WarningIcon)
        {
            colour = 0x55ff5555;
            character = '!';

            icon.addTriangle (iconRect.getX() + iconRect.getWidth() * 0.5f, (float) iconRect.getY(),
                              (float) iconRect.getRight(), (float) iconRect.getBottom(),
                              (float) iconRect.getX(), (float) iconRect.getBottom());

            icon = icon.createPathWithRoundedCorners (5.0f);
        }
        else
        {
            colour    = alert.getAlertType() == AlertWindow::InfoIcon ? (uint32) 0x605555ff : (uint32) 0x40b69900;
            character = alert.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';

            icon.addEllipse (iconRect.toFloat());
        }

        GlyphArrangement ga;
        ga.addFittedText (Font (iconRect.getHeight() * 0.9f, Font::bold),
                          String::charToString ((juce_wchar) (uint8) character),
                          (float) iconRect.getX(), (float) iconRect.getY(),
                          (float) iconRect.getWidth(), (float) iconRect.getHeight(),
                          Justification::centred, false);
        ga.createPath (icon);

        icon.setUsingNonZeroWinding (false);
        g.setColour (Colour (colour));
        g.fillPath (icon);

        iconSpaceUsed = iconWidth;
    }

    g.setColour (alert.findColour (AlertWindow::textColourId));

    textLayout.draw (g, Rectangle<int> (textArea.getX() + iconSpaceUsed,
                                        textArea.getY(),
                                        textArea.getWidth() - iconSpaceUsed,
                                        textArea.getHeight()).toFloat());

    g.setColour (alert.findColour (AlertWindow::outlineColourId));
    g.drawRect (0, 0, alert.getWidth(), alert.getHeight());
}

int LookAndFeel_V2::getAlertBoxWindowFlags()
{
    return ComponentPeer::windowAppearsOnTaskbar
            | ComponentPeer::windowHasDropShadow;
}

Array<int> LookAndFeel_V2::getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>& buttons)
{
    const int n = buttons.size();
    Array<int> buttonWidths;

    const int buttonHeight = getAlertWindowButtonHeight();

    for (int i = 0; i < n; ++i)
        buttonWidths.add (getTextButtonWidthToFitText (*buttons.getReference (i), buttonHeight));

    return buttonWidths;
}

int LookAndFeel_V2::getAlertWindowButtonHeight()
{
    return 28;
}

Font LookAndFeel_V2::getAlertWindowTitleFont()
{
    Font messageFont = getAlertWindowMessageFont();
    return messageFont.withHeight (messageFont.getHeight() * 1.1f).boldened();
}

Font LookAndFeel_V2::getAlertWindowMessageFont()
{
    return Font (15.0f);
}

Font LookAndFeel_V2::getAlertWindowFont()
{
    return Font (12.0f);
}

//==============================================================================
void LookAndFeel_V2::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      int width, int height,
                                      double progress, const String& textToShow)
{
    const Colour background (progressBar.findColour (ProgressBar::backgroundColourId));
    const Colour foreground (progressBar.findColour (ProgressBar::foregroundColourId));

    g.fillAll (background);

    if (progress >= 0.0f && progress < 1.0f)
    {
        drawGlassLozenge (g, 1.0f, 1.0f,
                          (float) jlimit (0.0, width - 2.0, progress * (width - 2.0)),
                          (float) (height - 2),
                          foreground,
                          0.5f, 0.0f,
                          true, true, true, true);
    }
    else
    {
        // spinning bar..
        g.setColour (foreground);

        const int stripeWidth = height * 2;
        const int position = (int) (Time::getMillisecondCounter() / 15) % stripeWidth;

        Path p;

        for (float x = (float) (- position); x < width + stripeWidth; x += stripeWidth)
            p.addQuadrilateral (x, 0.0f,
                                x + stripeWidth * 0.5f, 0.0f,
                                x, (float) height,
                                x - stripeWidth * 0.5f, (float) height);

        Image im (Image::ARGB, width, height, true);

        {
            Graphics g2 (im);
            drawGlassLozenge (g2, 1.0f, 1.0f,
                              (float) (width - 2),
                              (float) (height - 2),
                              foreground,
                              0.5f, 0.0f,
                              true, true, true, true);
        }

        g.setTiledImageFill (im, 0, 0, 0.85f);
        g.fillPath (p);
    }

    if (textToShow.isNotEmpty())
    {
        g.setColour (Colour::contrasting (background, foreground));
        g.setFont (height * 0.6f);

        g.drawText (textToShow, 0, 0, width, height, Justification::centred, false);
    }
}

void LookAndFeel_V2::drawSpinningWaitAnimation (Graphics& g, const Colour& colour, int x, int y, int w, int h)
{
    const float radius = jmin (w, h) * 0.4f;
    const float thickness = radius * 0.15f;
    Path p;
    p.addRoundedRectangle (radius * 0.4f, thickness * -0.5f,
                           radius * 0.6f, thickness,
                           thickness * 0.5f);

    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.5f;

    const uint32 animationIndex = (Time::getMillisecondCounter() / (1000 / 10)) % 12;

    for (uint32 i = 0; i < 12; ++i)
    {
        const uint32 n = (i + 12 - animationIndex) % 12;
        g.setColour (colour.withMultipliedAlpha ((n + 1) / 12.0f));

        g.fillPath (p, AffineTransform::rotation (i * (float_Pi / 6.0f))
                                       .translated (cx, cy));
    }
}

bool LookAndFeel_V2::isProgressBarOpaque (ProgressBar& progressBar)
{
    return progressBar.findColour (ProgressBar::backgroundColourId).isOpaque();
}

bool LookAndFeel_V2::areScrollbarButtonsVisible()
{
    return true;
}

void LookAndFeel_V2::drawScrollbarButton (Graphics& g, ScrollBar& scrollbar,
                                          int width, int height, int buttonDirection,
                                          bool /*isScrollbarVertical*/,
                                          bool /*isMouseOverButton*/,
                                          bool isButtonDown)
{
    Path p;

    if (buttonDirection == 0)
        p.addTriangle (width * 0.5f, height * 0.2f,
                       width * 0.1f, height * 0.7f,
                       width * 0.9f, height * 0.7f);
    else if (buttonDirection == 1)
        p.addTriangle (width * 0.8f, height * 0.5f,
                       width * 0.3f, height * 0.1f,
                       width * 0.3f, height * 0.9f);
    else if (buttonDirection == 2)
        p.addTriangle (width * 0.5f, height * 0.8f,
                       width * 0.1f, height * 0.3f,
                       width * 0.9f, height * 0.3f);
    else if (buttonDirection == 3)
        p.addTriangle (width * 0.2f, height * 0.5f,
                       width * 0.7f, height * 0.1f,
                       width * 0.7f, height * 0.9f);

    if (isButtonDown)
        g.setColour (scrollbar.findColour (ScrollBar::thumbColourId).contrasting (0.2f));
    else
        g.setColour (scrollbar.findColour (ScrollBar::thumbColourId));

    g.fillPath (p);

    g.setColour (Colour (0x80000000));
    g.strokePath (p, PathStrokeType (0.5f));
}

void LookAndFeel_V2::drawScrollbar (Graphics& g,
                                 ScrollBar& scrollbar,
                                 int x, int y,
                                 int width, int height,
                                 bool isScrollbarVertical,
                                 int thumbStartPosition,
                                 int thumbSize,
                                 bool /*isMouseOver*/,
                                 bool /*isMouseDown*/)
{
    g.fillAll (scrollbar.findColour (ScrollBar::backgroundColourId));

    Path slotPath, thumbPath;

    const float slotIndent = jmin (width, height) > 15 ? 1.0f : 0.0f;
    const float slotIndentx2 = slotIndent * 2.0f;
    const float thumbIndent = slotIndent + 1.0f;
    const float thumbIndentx2 = thumbIndent * 2.0f;

    float gx1 = 0.0f, gy1 = 0.0f, gx2 = 0.0f, gy2 = 0.0f;

    if (isScrollbarVertical)
    {
        slotPath.addRoundedRectangle (x + slotIndent,
                                      y + slotIndent,
                                      width - slotIndentx2,
                                      height - slotIndentx2,
                                      (width - slotIndentx2) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (x + thumbIndent,
                                           thumbStartPosition + thumbIndent,
                                           width - thumbIndentx2,
                                           thumbSize - thumbIndentx2,
                                           (width - thumbIndentx2) * 0.5f);
        gx1 = (float) x;
        gx2 = x + width * 0.7f;
    }
    else
    {
        slotPath.addRoundedRectangle (x + slotIndent,
                                      y + slotIndent,
                                      width - slotIndentx2,
                                      height - slotIndentx2,
                                      (height - slotIndentx2) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (thumbStartPosition + thumbIndent,
                                           y + thumbIndent,
                                           thumbSize - thumbIndentx2,
                                           height - thumbIndentx2,
                                           (height - thumbIndentx2) * 0.5f);
        gy1 = (float) y;
        gy2 = y + height * 0.7f;
    }

    const Colour thumbColour (scrollbar.findColour (ScrollBar::thumbColourId));
    Colour trackColour1, trackColour2;

    if (scrollbar.isColourSpecified (ScrollBar::trackColourId)
         || isColourSpecified (ScrollBar::trackColourId))
    {
        trackColour1 = trackColour2 = scrollbar.findColour (ScrollBar::trackColourId);
    }
    else
    {
        trackColour1 = thumbColour.overlaidWith (Colour (0x44000000));
        trackColour2 = thumbColour.overlaidWith (Colour (0x19000000));
    }

    g.setGradientFill (ColourGradient (trackColour1, gx1, gy1,
                                       trackColour2, gx2, gy2, false));
    g.fillPath (slotPath);

    if (isScrollbarVertical)
    {
        gx1 = x + width * 0.6f;
        gx2 = (float) x + width;
    }
    else
    {
        gy1 = y + height * 0.6f;
        gy2 = (float) y + height;
    }

    g.setGradientFill (ColourGradient (Colours::transparentBlack,gx1, gy1,
                       Colour (0x19000000), gx2, gy2, false));
    g.fillPath (slotPath);

    g.setColour (thumbColour);
    g.fillPath (thumbPath);

    g.setGradientFill (ColourGradient (Colour (0x10000000), gx1, gy1,
                       Colours::transparentBlack, gx2, gy2, false));

    g.saveState();

    if (isScrollbarVertical)
        g.reduceClipRegion (x + width / 2, y, width, height);
    else
        g.reduceClipRegion (x, y + height / 2, width, height);

    g.fillPath (thumbPath);
    g.restoreState();

    g.setColour (Colour (0x4c000000));
    g.strokePath (thumbPath, PathStrokeType (0.4f));
}

ImageEffectFilter* LookAndFeel_V2::getScrollbarEffect()
{
    return nullptr;
}

int LookAndFeel_V2::getMinimumScrollbarThumbSize (ScrollBar& scrollbar)
{
    return jmin (scrollbar.getWidth(), scrollbar.getHeight()) * 2;
}

int LookAndFeel_V2::getDefaultScrollbarWidth()
{
    return 18;
}

int LookAndFeel_V2::getScrollbarButtonSize (ScrollBar& scrollbar)
{
    return 2 + (scrollbar.isVertical() ? scrollbar.getWidth()
                                       : scrollbar.getHeight());
}

//==============================================================================
Path LookAndFeel_V2::getTickShape (const float height)
{
    static const unsigned char tickShapeData[] =
    {
        109,0,224,168,68,0,0,119,67,108,0,224,172,68,0,128,146,67,113,0,192,148,68,0,0,219,67,0,96,110,68,0,224,56,68,113,0,64,51,68,0,32,130,68,0,64,20,68,0,224,
        162,68,108,0,128,3,68,0,128,168,68,113,0,128,221,67,0,192,175,68,0,0,207,67,0,32,179,68,113,0,0,201,67,0,224,173,68,0,0,181,67,0,224,161,68,108,0,128,168,67,
        0,128,154,68,113,0,128,141,67,0,192,138,68,0,128,108,67,0,64,131,68,113,0,0,62,67,0,128,119,68,0,0,5,67,0,128,114,68,113,0,0,102,67,0,192,88,68,0,128,155,
        67,0,192,88,68,113,0,0,190,67,0,192,88,68,0,128,232,67,0,224,131,68,108,0,128,246,67,0,192,139,68,113,0,64,33,68,0,128,87,68,0,0,93,68,0,224,26,68,113,0,
        96,140,68,0,128,188,67,0,224,168,68,0,0,119,67,99,101
    };

    Path p;
    p.loadPathFromData (tickShapeData, sizeof (tickShapeData));
    p.scaleToFit (0, 0, height * 2.0f, height, true);
    return p;
}

Path LookAndFeel_V2::getCrossShape (const float height)
{
    static const unsigned char crossShapeData[] =
    {
        109,0,0,17,68,0,96,145,68,108,0,192,13,68,0,192,147,68,113,0,0,213,67,0,64,174,68,0,0,168,67,0,64,174,68,113,0,0,104,67,0,64,174,68,0,0,5,67,0,64,
        153,68,113,0,0,18,67,0,64,153,68,0,0,24,67,0,64,153,68,113,0,0,135,67,0,64,153,68,0,128,207,67,0,224,130,68,108,0,0,220,67,0,0,126,68,108,0,0,204,67,
        0,128,117,68,113,0,0,138,67,0,64,82,68,0,0,138,67,0,192,57,68,113,0,0,138,67,0,192,37,68,0,128,210,67,0,64,10,68,113,0,128,220,67,0,64,45,68,0,0,8,
        68,0,128,78,68,108,0,192,14,68,0,0,87,68,108,0,64,20,68,0,0,80,68,113,0,192,57,68,0,0,32,68,0,128,88,68,0,0,32,68,113,0,64,112,68,0,0,32,68,0,
        128,124,68,0,64,68,68,113,0,0,121,68,0,192,67,68,0,128,119,68,0,192,67,68,113,0,192,108,68,0,192,67,68,0,32,89,68,0,96,82,68,113,0,128,69,68,0,0,97,68,
        0,0,56,68,0,64,115,68,108,0,64,49,68,0,128,124,68,108,0,192,55,68,0,96,129,68,113,0,0,92,68,0,224,146,68,0,192,129,68,0,224,146,68,113,0,64,110,68,0,64,
        168,68,0,64,87,68,0,64,168,68,113,0,128,66,68,0,64,168,68,0,64,27,68,0,32,150,68,99,101
    };

    Path p;
    p.loadPathFromData (crossShapeData, sizeof (crossShapeData));
    p.scaleToFit (0, 0, height * 2.0f, height, true);
    return p;
}

//==============================================================================
void LookAndFeel_V2::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                               Colour /*backgroundColour*/, bool isOpen, bool /*isMouseOver*/)
{
    const int boxSize = roundToInt (jmin (16.0f, area.getWidth(), area.getHeight()) * 0.7f) | 1;

    const int x = ((int) area.getWidth()  - boxSize) / 2 + (int) area.getX();
    const int y = ((int) area.getHeight() - boxSize) / 2 + (int) area.getY();

    Rectangle<float> boxArea ((float) x, (float) y, (float) boxSize, (float) boxSize);

    g.setColour (Colour (0xe5ffffff));
    g.fillRect (boxArea);

    g.setColour (Colour (0x80000000));
    g.drawRect (boxArea);

    const float size = boxSize / 2 + 1.0f;
    const float centre = (float) (boxSize / 2);

    g.fillRect (x + (boxSize - size) * 0.5f, y + centre, size, 1.0f);

    if (! isOpen)
        g.fillRect (x + centre, y + (boxSize - size) * 0.5f, 1.0f, size);
}

bool LookAndFeel_V2::areLinesDrawnForTreeView (TreeView&)
{
    return true;
}

int LookAndFeel_V2::getTreeViewIndentSize (TreeView&)
{
    return 24;
}

//==============================================================================
void LookAndFeel_V2::drawBubble (Graphics& g, BubbleComponent& comp,
                                 const Point<float>& tip, const Rectangle<float>& body)
{
    Path p;
    p.addBubble (body.reduced (0.5f), body.getUnion (Rectangle<float> (tip.x, tip.y, 1.0f, 1.0f)),
                 tip, 5.0f, jmin (15.0f, body.getWidth() * 0.2f, body.getHeight() * 0.2f));

    g.setColour (comp.findColour (BubbleComponent::backgroundColourId));
    g.fillPath (p);

    g.setColour (comp.findColour (BubbleComponent::outlineColourId));
    g.strokePath (p, PathStrokeType (1.0f));
}


//==============================================================================
Font LookAndFeel_V2::getPopupMenuFont()
{
    return Font (17.0f);
}

void LookAndFeel_V2::getIdealPopupMenuItemSize (const String& text, const bool isSeparator,
                                                int standardMenuItemHeight, int& idealWidth, int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 2 : 10;
    }
    else
    {
        Font font (getPopupMenuFont());

        if (standardMenuItemHeight > 0 && font.getHeight() > standardMenuItemHeight / 1.3f)
            font.setHeight (standardMenuItemHeight / 1.3f);

        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundToInt (font.getHeight() * 1.3f);
        idealWidth = font.getStringWidth (text) + idealHeight * 2;
    }
}

void LookAndFeel_V2::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    g.fillAll (background);
    g.setColour (background.overlaidWith (Colour (0x2badd8e6)));

    for (int i = 0; i < height; i += 3)
        g.fillRect (0, i, width, 1);

   #if ! JUCE_MAC
    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
   #endif
}

void LookAndFeel_V2::drawPopupMenuUpDownArrow (Graphics& g, int width, int height, bool isScrollUpArrow)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    g.setGradientFill (ColourGradient (background, 0.0f, height * 0.5f,
                                       background.withAlpha (0.0f),
                                       0.0f, isScrollUpArrow ? ((float) height) : 0.0f,
                                       false));

    g.fillRect (1, 1, width - 2, height - 2);

    const float hw = width * 0.5f;
    const float arrowW = height * 0.3f;
    const float y1 = height * (isScrollUpArrow ? 0.6f : 0.3f);
    const float y2 = height * (isScrollUpArrow ? 0.3f : 0.6f);

    Path p;
    p.addTriangle (hw - arrowW, y1,
                   hw + arrowW, y1,
                   hw, y2);

    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.5f));
    g.fillPath (p);
}

void LookAndFeel_V2::drawPopupMenuItem (Graphics& g, const Rectangle<int>& area,
                                        const bool isSeparator, const bool isActive,
                                        const bool isHighlighted, const bool isTicked,
                                        const bool hasSubMenu, const String& text,
                                        const String& shortcutKeyText,
                                        const Drawable* icon, const Colour* const textColourToUse)
{
    if (isSeparator)
    {
        Rectangle<int> r (area.reduced (5, 0));
        r.removeFromTop (r.getHeight() / 2 - 1);

        g.setColour (Colour (0x33000000));
        g.fillRect (r.removeFromTop (1));

        g.setColour (Colour (0x66ffffff));
        g.fillRect (r.removeFromTop (1));
    }
    else
    {
        Colour textColour (findColour (PopupMenu::textColourId));

        if (textColourToUse != nullptr)
            textColour = *textColourToUse;

        Rectangle<int> r (area.reduced (1));

        if (isHighlighted)
        {
            g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
            g.fillRect (r);

            g.setColour (findColour (PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour (textColour);
        }

        if (! isActive)
            g.setOpacity (0.3f);

        Font font (getPopupMenuFont());

        const float maxFontHeight = area.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight)
            font.setHeight (maxFontHeight);

        g.setFont (font);

        Rectangle<float> iconArea (r.removeFromLeft ((r.getHeight() * 5) / 4).reduced (3).toFloat());

        if (icon != nullptr)
        {
            icon->drawWithin (g, iconArea, RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
        }
        else if (isTicked)
        {
            const Path tick (getTickShape (1.0f));
            g.fillPath (tick, tick.getTransformToScaleToFit (iconArea, true));
        }

        if (hasSubMenu)
        {
            const float arrowH = 0.6f * getPopupMenuFont().getAscent();

            const float x = (float) r.removeFromRight ((int) arrowH).getX();
            const float halfH = (float) r.getCentreY();

            Path p;
            p.addTriangle (x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);

            g.fillPath (p);
        }

        r.removeFromRight (3);
        g.drawFittedText (text, r, Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            Font f2 (font);
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText, r, Justification::centredRight, true);
        }
    }
}

void LookAndFeel_V2::drawPopupMenuSectionHeader (Graphics& g, const Rectangle<int>& area, const String& sectionName)
{
    g.setFont (getPopupMenuFont().boldened());
    g.setColour (findColour (PopupMenu::headerTextColourId));

    g.drawFittedText (sectionName,
                      area.getX() + 12, area.getY(), area.getWidth() - 16, (int) (area.getHeight() * 0.8f),
                      Justification::bottomLeft, 1);
}

//==============================================================================
int LookAndFeel_V2::getMenuWindowFlags()
{
    return ComponentPeer::windowHasDropShadow;
}

void LookAndFeel_V2::drawMenuBarBackground (Graphics& g, int width, int height, bool, MenuBarComponent& menuBar)
{
    const Colour baseColour (LookAndFeelHelpers::createBaseColour (menuBar.findColour (PopupMenu::backgroundColourId),
                                                                   false, false, false));

    if (menuBar.isEnabled())
        drawShinyButtonShape (g, -4.0f, 0.0f, width + 8.0f, (float) height,
                              0.0f, baseColour, 0.4f, true, true, true, true);
    else
        g.fillAll (baseColour);
}

Font LookAndFeel_V2::getMenuBarFont (MenuBarComponent& menuBar, int /*itemIndex*/, const String& /*itemText*/)
{
    return Font (menuBar.getHeight() * 0.7f);
}

int LookAndFeel_V2::getMenuBarItemWidth (MenuBarComponent& menuBar, int itemIndex, const String& itemText)
{
    return getMenuBarFont (menuBar, itemIndex, itemText)
            .getStringWidth (itemText) + menuBar.getHeight();
}

void LookAndFeel_V2::drawMenuBarItem (Graphics& g, int width, int height,
                                      int itemIndex, const String& itemText,
                                      bool isMouseOverItem, bool isMenuOpen,
                                      bool /*isMouseOverBar*/, MenuBarComponent& menuBar)
{
    if (! menuBar.isEnabled())
    {
        g.setColour (menuBar.findColour (PopupMenu::textColourId)
                            .withMultipliedAlpha (0.5f));
    }
    else if (isMenuOpen || isMouseOverItem)
    {
        g.fillAll (menuBar.findColour (PopupMenu::highlightedBackgroundColourId));
        g.setColour (menuBar.findColour (PopupMenu::highlightedTextColourId));
    }
    else
    {
        g.setColour (menuBar.findColour (PopupMenu::textColourId));
    }

    g.setFont (getMenuBarFont (menuBar, itemIndex, itemText));
    g.drawFittedText (itemText, 0, 0, width, height, Justification::centred, 1);
}

Component* LookAndFeel_V2::getParentComponentForMenuOptions (const PopupMenu::Options& options)
{
    return options.getParentComponent();
}

void LookAndFeel_V2::preparePopupMenuWindow (Component&) {}

bool LookAndFeel_V2::shouldPopupMenuScaleWithTargetComponent (const PopupMenu::Options&)    { return true; }

int LookAndFeel_V2::getPopupMenuBorderSize()    { return 2; }

//==============================================================================
void LookAndFeel_V2::fillTextEditorBackground (Graphics& g, int /*width*/, int /*height*/, TextEditor& textEditor)
{
    g.fillAll (textEditor.findColour (TextEditor::backgroundColourId));
}

void LookAndFeel_V2::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            const int border = 2;

            g.setColour (textEditor.findColour (TextEditor::focusedOutlineColourId));
            g.drawRect (0, 0, width, height, border);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId).withMultipliedAlpha (0.75f));
            drawBevel (g, 0, 0, width, height + 2, border + 2, shadowColour, shadowColour);
        }
        else
        {
            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawRect (0, 0, width, height);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId));
            drawBevel (g, 0, 0, width, height + 2, 3, shadowColour, shadowColour);
        }
    }
}

CaretComponent* LookAndFeel_V2::createCaretComponent (Component* keyFocusOwner)
{
    return new CaretComponent (keyFocusOwner);
}

//==============================================================================
void LookAndFeel_V2::drawComboBox (Graphics& g, int width, int height, const bool isButtonDown,
                                   int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box)
{
    g.fillAll (box.findColour (ComboBox::backgroundColourId));

    if (box.isEnabled() && box.hasKeyboardFocus (false))
    {
        g.setColour (box.findColour (ComboBox::buttonColourId));
        g.drawRect (0, 0, width, height, 2);
    }
    else
    {
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRect (0, 0, width, height);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    const Colour baseColour (LookAndFeelHelpers::createBaseColour (box.findColour (ComboBox::buttonColourId),
                                                                   box.hasKeyboardFocus (true),
                                                                   false, isButtonDown)
                                .withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f));

    drawGlassLozenge (g,
                      buttonX + outlineThickness, buttonY + outlineThickness,
                      buttonW - outlineThickness * 2.0f, buttonH - outlineThickness * 2.0f,
                      baseColour, outlineThickness, -1.0f,
                      true, true, true, true);

    if (box.isEnabled())
    {
        const float arrowX = 0.3f;
        const float arrowH = 0.2f;

        Path p;
        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

        g.setColour (box.findColour (ComboBox::arrowColourId));
        g.fillPath (p);
    }
}

Font LookAndFeel_V2::getComboBoxFont (ComboBox& box)
{
    return Font (jmin (15.0f, box.getHeight() * 0.85f));
}

Label* LookAndFeel_V2::createComboBoxTextBox (ComboBox&)
{
    return new Label (String(), String());
}

void LookAndFeel_V2::positionComboBoxText (ComboBox& box, Label& label)
{
    label.setBounds (1, 1,
                     box.getWidth() + 3 - box.getHeight(),
                     box.getHeight() - 2);

    label.setFont (getComboBoxFont (box));
}

//==============================================================================
Font LookAndFeel_V2::getLabelFont (Label& label)
{
    return label.getFont();
}

void LookAndFeel_V2::drawLabel (Graphics& g, Label& label)
{
    g.fillAll (label.findColour (Label::backgroundColourId));

    if (! label.isBeingEdited())
    {
        const float alpha = label.isEnabled() ? 1.0f : 0.5f;
        const Font font (getLabelFont (label));

        g.setColour (label.findColour (Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);

        Rectangle<int> textArea (label.getBorderSize().subtractedFrom (label.getLocalBounds()));

        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());

        g.setColour (label.findColour (Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (Label::outlineColourId));
    }

    g.drawRect (label.getLocalBounds());
}

//==============================================================================
void LookAndFeel_V2::drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height,
                                                 float /*sliderPos*/,
                                                 float /*minSliderPos*/,
                                                 float /*maxSliderPos*/,
                                                 const Slider::SliderStyle /*style*/, Slider& slider)
{
    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    const Colour trackColour (slider.findColour (Slider::trackColourId));
    const Colour gradCol1 (trackColour.overlaidWith (Colours::black.withAlpha (slider.isEnabled() ? 0.25f : 0.13f)));
    const Colour gradCol2 (trackColour.overlaidWith (Colour (0x14000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        const float iy = y + height * 0.5f - sliderRadius * 0.5f;
        const float ih = sliderRadius;

        g.setGradientFill (ColourGradient (gradCol1, 0.0f, iy,
                                           gradCol2, 0.0f, iy + ih, false));

        indent.addRoundedRectangle (x - sliderRadius * 0.5f, iy,
                                    width + sliderRadius, ih,
                                    5.0f);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;
        const float iw = sliderRadius;

        g.setGradientFill (ColourGradient (gradCol1, ix, 0.0f,
                                           gradCol2, ix + iw, 0.0f, false));

        indent.addRoundedRectangle (ix, y - sliderRadius * 0.5f,
                                    iw, height + sliderRadius,
                                    5.0f);
    }

    g.fillPath (indent);

    g.setColour (Colour (0x4c000000));
    g.strokePath (indent, PathStrokeType (0.5f));
}

void LookAndFeel_V2::drawLinearSliderThumb (Graphics& g, int x, int y, int width, int height,
                                            float sliderPos, float minSliderPos, float maxSliderPos,
                                            const Slider::SliderStyle style, Slider& slider)
{
    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    Colour knobColour (LookAndFeelHelpers::createBaseColour (slider.findColour (Slider::thumbColourId),
                                                             slider.hasKeyboardFocus (false) && slider.isEnabled(),
                                                             slider.isMouseOverOrDragging() && slider.isEnabled(),
                                                             slider.isMouseButtonDown() && slider.isEnabled()));

    const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

    if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
    {
        float kx, ky;

        if (style == Slider::LinearVertical)
        {
            kx = x + width * 0.5f;
            ky = sliderPos;
        }
        else
        {
            kx = sliderPos;
            ky = y + height * 0.5f;
        }

        drawGlassSphere (g,
                         kx - sliderRadius,
                         ky - sliderRadius,
                         sliderRadius * 2.0f,
                         knobColour, outlineThickness);
    }
    else
    {
        if (style == Slider::ThreeValueVertical)
        {
            drawGlassSphere (g, x + width * 0.5f - sliderRadius,
                             sliderPos - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColour, outlineThickness);
        }
        else if (style == Slider::ThreeValueHorizontal)
        {
            drawGlassSphere (g,sliderPos - sliderRadius,
                             y + height * 0.5f - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColour, outlineThickness);
        }

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            const float sr = jmin (sliderRadius, width * 0.4f);

            drawGlassPointer (g, jmax (0.0f, x + width * 0.5f - sliderRadius * 2.0f),
                              minSliderPos - sliderRadius,
                              sliderRadius * 2.0f, knobColour, outlineThickness, 1);

            drawGlassPointer (g, jmin (x + width - sliderRadius * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                              sliderRadius * 2.0f, knobColour, outlineThickness, 3);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            const float sr = jmin (sliderRadius, height * 0.4f);

            drawGlassPointer (g, minSliderPos - sr,
                              jmax (0.0f, y + height * 0.5f - sliderRadius * 2.0f),
                              sliderRadius * 2.0f, knobColour, outlineThickness, 2);

            drawGlassPointer (g, maxSliderPos - sliderRadius,
                              jmin (y + height - sliderRadius * 2.0f, y + height * 0.5f),
                              sliderRadius * 2.0f, knobColour, outlineThickness, 4);
        }
    }
}

void LookAndFeel_V2::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    g.fillAll (slider.findColour (Slider::backgroundColourId));

    if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
    {
        const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

        Colour baseColour (LookAndFeelHelpers::createBaseColour (slider.findColour (Slider::thumbColourId)
                                                                       .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f),
                                                                 false, isMouseOver,
                                                                 isMouseOver || slider.isMouseButtonDown()));

        drawShinyButtonShape (g,
                              (float) x,
                              style == Slider::LinearBarVertical ? sliderPos
                                                                 : (float) y,
                              style == Slider::LinearBarVertical ? (float) width
                                                                 : (sliderPos - x),
                              style == Slider::LinearBarVertical ? (height - sliderPos)
                                                                 : (float) height, 0.0f,
                              baseColour,
                              slider.isEnabled() ? 0.9f : 0.3f,
                              true, true, true, true);
    }
    else
    {
        drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

int LookAndFeel_V2::getSliderThumbRadius (Slider& slider)
{
    return jmin (7,
                 slider.getHeight() / 2,
                 slider.getWidth() / 2) + 2;
}

void LookAndFeel_V2::drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                                       const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider)
{
    const float radius = jmin (width / 2, height / 2) - 2.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (radius > 12.0f)
    {
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        const float thickness = 0.7f;

        {
            Path filledArc;
            filledArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, angle, thickness);
            g.fillPath (filledArc);
        }

        {
            const float innerRadius = radius * 0.2f;
            Path p;
            p.addTriangle (-innerRadius, 0.0f,
                           0.0f, -radius * thickness * 1.1f,
                           innerRadius, 0.0f);

            p.addEllipse (-innerRadius, -innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

            g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
        }

        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
        else
            g.setColour (Colour (0x80808080));

        Path outlineArc;
        outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
        outlineArc.closeSubPath();

        g.strokePath (outlineArc, PathStrokeType (slider.isEnabled() ? (isMouseOver ? 2.0f : 1.2f) : 0.3f));
    }
    else
    {
        if (slider.isEnabled())
            g.setColour (slider.findColour (Slider::rotarySliderFillColourId).withAlpha (isMouseOver ? 1.0f : 0.7f));
        else
            g.setColour (Colour (0x80808080));

        Path p;
        p.addEllipse (-0.4f * rw, -0.4f * rw, rw * 0.8f, rw * 0.8f);
        PathStrokeType (rw * 0.1f).createStrokedPath (p, p);

        p.addLineSegment (Line<float> (0.0f, 0.0f, 0.0f, -radius), rw * 0.2f);

        g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }
}

Button* LookAndFeel_V2::createSliderButton (Slider&, const bool isIncrement)
{
    return new TextButton (isIncrement ? "+" : "-", String());
}

class LookAndFeel_V2::SliderLabelComp  : public Label
{
public:
    SliderLabelComp() : Label ({}, {}) {}

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override {}
};

Label* LookAndFeel_V2::createSliderTextBox (Slider& slider)
{
    auto l = new SliderLabelComp();

    l->setJustificationType (Justification::centred);
    l->setKeyboardType (TextInputTarget::decimalKeyboard);

    l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));
    l->setColour (Label::backgroundColourId,
                  (slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                            ? Colours::transparentBlack
                            : slider.findColour (Slider::textBoxBackgroundColourId));
    l->setColour (Label::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
    l->setColour (TextEditor::textColourId, slider.findColour (Slider::textBoxTextColourId));
    l->setColour (TextEditor::backgroundColourId,
                  slider.findColour (Slider::textBoxBackgroundColourId)
                        .withAlpha ((slider.getSliderStyle() == Slider::LinearBar || slider.getSliderStyle() == Slider::LinearBarVertical)
                                        ? 0.7f : 1.0f));
    l->setColour (TextEditor::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));
    l->setColour (TextEditor::highlightColourId, slider.findColour (Slider::textBoxHighlightColourId));

    return l;
}

ImageEffectFilter* LookAndFeel_V2::getSliderEffect (Slider&)
{
    return nullptr;
}

Font LookAndFeel_V2::getSliderPopupFont (Slider&)
{
    return Font (15.0f, Font::bold);
}

int LookAndFeel_V2::getSliderPopupPlacement (Slider&)
{
    return BubbleComponent::above
            | BubbleComponent::below
            | BubbleComponent::left
            | BubbleComponent::right;
}

//==============================================================================
Slider::SliderLayout LookAndFeel_V2::getSliderLayout (Slider& slider)
{
    // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints

    int minXSpace = 0;
    int minYSpace = 0;

    Slider::TextEntryBoxPosition textBoxPos = slider.getTextBoxPosition();

    if (textBoxPos == Slider::TextBoxLeft || textBoxPos == Slider::TextBoxRight)
        minXSpace = 30;
    else
        minYSpace = 15;

    Rectangle<int> localBounds = slider.getLocalBounds();

    const int textBoxWidth = jmax (0, jmin (slider.getTextBoxWidth(),  localBounds.getWidth() - minXSpace));
    const int textBoxHeight = jmax (0, jmin (slider.getTextBoxHeight(), localBounds.getHeight() - minYSpace));

    Slider::SliderLayout layout;

    // 2. set the textBox bounds

    if (textBoxPos != Slider::NoTextBox)
    {
        if (slider.isBar())
        {
            layout.textBoxBounds = localBounds;
        }
        else
        {
            layout.textBoxBounds.setWidth (textBoxWidth);
            layout.textBoxBounds.setHeight (textBoxHeight);

            if (textBoxPos == Slider::TextBoxLeft)           layout.textBoxBounds.setX (0);
            else if (textBoxPos == Slider::TextBoxRight)     layout.textBoxBounds.setX (localBounds.getWidth() - textBoxWidth);
            else /* above or below -> centre horizontally */ layout.textBoxBounds.setX ((localBounds.getWidth() - textBoxWidth) / 2);

            if (textBoxPos == Slider::TextBoxAbove)          layout.textBoxBounds.setY (0);
            else if (textBoxPos == Slider::TextBoxBelow)     layout.textBoxBounds.setY (localBounds.getHeight() - textBoxHeight);
            else /* left or right -> centre vertically */    layout.textBoxBounds.setY ((localBounds.getHeight() - textBoxHeight) / 2);
        }
    }

    // 3. set the slider bounds

    layout.sliderBounds = localBounds;

    if (slider.isBar())
    {
        layout.sliderBounds.reduce (1, 1);   // bar border
    }
    else
    {
        if (textBoxPos == Slider::TextBoxLeft)       layout.sliderBounds.removeFromLeft (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxRight) layout.sliderBounds.removeFromRight (textBoxWidth);
        else if (textBoxPos == Slider::TextBoxAbove) layout.sliderBounds.removeFromTop (textBoxHeight);
        else if (textBoxPos == Slider::TextBoxBelow) layout.sliderBounds.removeFromBottom (textBoxHeight);

        const int thumbIndent = getSliderThumbRadius (slider);

        if (slider.isHorizontal())    layout.sliderBounds.reduce (thumbIndent, 0);
        else if (slider.isVertical()) layout.sliderBounds.reduce (0, thumbIndent);
    }

    return layout;
}

//==============================================================================
Rectangle<int> LookAndFeel_V2::getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea)
{
    const TextLayout tl (LookAndFeelHelpers::layoutTooltipText (tipText, Colours::black));

    const int w = (int) (tl.getWidth() + 14.0f);
    const int h = (int) (tl.getHeight() + 6.0f);

    return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                           screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                           w, h)
             .constrainedWithin (parentArea);
}

void LookAndFeel_V2::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    g.fillAll (findColour (TooltipWindow::backgroundColourId));

   #if ! JUCE_MAC // The mac windows already have a non-optional 1 pix outline, so don't double it here..
    g.setColour (findColour (TooltipWindow::outlineColourId));
    g.drawRect (0, 0, width, height, 1);
   #endif

    LookAndFeelHelpers::layoutTooltipText (text, findColour (TooltipWindow::textColourId))
        .draw (g, Rectangle<float> ((float) width, (float) height));
}

//==============================================================================
Button* LookAndFeel_V2::createFilenameComponentBrowseButton (const String& text)
{
    return new TextButton (text, TRANS("click to browse for a different file"));
}

void LookAndFeel_V2::layoutFilenameComponent (FilenameComponent& filenameComp,
                                              ComboBox* filenameBox, Button* browseButton)
{
    browseButton->setSize (80, filenameComp.getHeight());

    if (auto* tb = dynamic_cast<TextButton*> (browseButton))
        tb->changeWidthToFitText();

    browseButton->setTopRightPosition (filenameComp.getWidth(), 0);

    filenameBox->setBounds (0, 0, browseButton->getX(), filenameComp.getHeight());
}

//==============================================================================
void LookAndFeel_V2::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                                bool isMouseOver, bool /*isMouseDown*/,
                                                ConcertinaPanel&, Component& panel)
{
    g.fillAll (Colours::grey.withAlpha (isMouseOver ? 0.9f : 0.7f));
    g.setColour (Colours::black.withAlpha (0.5f));
    g.drawRect (area);

    g.setColour (Colours::white);
    g.setFont (Font (area.getHeight() * 0.7f).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

//==============================================================================
void LookAndFeel_V2::drawImageButton (Graphics& g, Image* image,
                                      int imageX, int imageY, int imageW, int imageH,
                                      const Colour& overlayColour,
                                      float imageOpacity,
                                      ImageButton& button)
{
    if (! button.isEnabled())
        imageOpacity *= 0.3f;

    AffineTransform t = RectanglePlacement (RectanglePlacement::stretchToFit)
                            .getTransformToFit (image->getBounds().toFloat(),
                                                Rectangle<int> (imageX, imageY, imageW, imageH).toFloat());

    if (! overlayColour.isOpaque())
    {
        g.setOpacity (imageOpacity);
        g.drawImageTransformed (*image, t, false);
    }

    if (! overlayColour.isTransparent())
    {
        g.setColour (overlayColour);
        g.drawImageTransformed (*image, t, true);
    }
}

//==============================================================================
void LookAndFeel_V2::drawCornerResizer (Graphics& g,
                                     int w, int h,
                                     bool /*isMouseOver*/,
                                     bool /*isMouseDragging*/)
{
    const float lineThickness = jmin (w, h) * 0.075f;

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.setColour (Colours::lightgrey);

        g.drawLine (w * i,
                    h + 1.0f,
                    w + 1.0f,
                    h * i,
                    lineThickness);

        g.setColour (Colours::darkgrey);

        g.drawLine (w * i + lineThickness,
                    h + 1.0f,
                    w + 1.0f,
                    h * i + lineThickness,
                    lineThickness);
    }
}

void LookAndFeel_V2::drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border)
{
    if (! border.isEmpty())
    {
        const Rectangle<int> fullSize (0, 0, w, h);
        const Rectangle<int> centreArea (border.subtractedFrom (fullSize));

        g.saveState();

        g.excludeClipRegion (centreArea);

        g.setColour (Colour (0x50000000));
        g.drawRect (fullSize);

        g.setColour (Colour (0x19000000));
        g.drawRect (centreArea.expanded (1, 1));

        g.restoreState();
    }
}

//==============================================================================
void LookAndFeel_V2::fillResizableWindowBackground (Graphics& g, int /*w*/, int /*h*/,
                                                    const BorderSize<int>& /*border*/, ResizableWindow& window)
{
    g.fillAll (window.getBackgroundColour());
}

void LookAndFeel_V2::drawResizableWindowBorder (Graphics&, int /*w*/, int /*h*/,
                                                const BorderSize<int>& /*border*/, ResizableWindow&)
{
}

void LookAndFeel_V2::drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                                 int w, int h, int titleSpaceX, int titleSpaceW,
                                                 const Image* icon, bool drawTitleTextOnLeft)
{
    if (w * h == 0)
        return;

    const bool isActive = window.isActiveWindow();

    g.setGradientFill (ColourGradient (window.getBackgroundColour(),
                                       0.0f, 0.0f,
                                       window.getBackgroundColour().contrasting (isActive ? 0.15f : 0.05f),
                                       0.0f, (float) h, false));
    g.fillAll();

    Font font (h * 0.65f, Font::bold);
    g.setFont (font);

    int textW = font.getStringWidth (window.getName());
    int iconW = 0;
    int iconH = 0;

    if (icon != nullptr)
    {
        iconH = (int) font.getHeight();
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    int textX = drawTitleTextOnLeft ? titleSpaceX
                                    : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != nullptr)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centred, false);
        textX += iconW;
        textW -= iconW;
    }

    if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
        g.setColour (window.findColour (DocumentWindow::textColourId));
    else
        g.setColour (window.getBackgroundColour().contrasting (isActive ? 0.7f : 0.4f));

    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

//==============================================================================
class LookAndFeel_V2::GlassWindowButton   : public Button
{
public:
    GlassWindowButton (const String& name, Colour col,
                       const Path& normalShape_,
                       const Path& toggledShape_) noexcept
        : Button (name),
          colour (col),
          normalShape (normalShape_),
          toggledShape (toggledShape_)
    {
    }

    //==============================================================================
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        float alpha = isMouseOverButton ? (isButtonDown ? 1.0f : 0.8f) : 0.55f;

        if (! isEnabled())
            alpha *= 0.5f;

        float x = 0, y = 0, diam;

        if (getWidth() < getHeight())
        {
            diam = (float) getWidth();
            y = (getHeight() - getWidth()) * 0.5f;
        }
        else
        {
            diam = (float) getHeight();
            y = (getWidth() - getHeight()) * 0.5f;
        }

        x += diam * 0.05f;
        y += diam * 0.05f;
        diam *= 0.9f;

        g.setGradientFill (ColourGradient (Colour::greyLevel (0.9f).withAlpha (alpha), 0, y + diam,
                                           Colour::greyLevel (0.6f).withAlpha (alpha), 0, y, false));
        g.fillEllipse (x, y, diam, diam);

        x += 2.0f;
        y += 2.0f;
        diam -= 4.0f;

        LookAndFeel_V2::drawGlassSphere (g, x, y, diam, colour.withAlpha (alpha), 1.0f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        const AffineTransform t (p.getTransformToScaleToFit (x + diam * 0.3f, y + diam * 0.3f,
                                                             diam * 0.4f, diam * 0.4f, true));

        g.setColour (Colours::black.withAlpha (alpha * 0.6f));
        g.fillPath (p, t);
    }

private:
    Colour colour;
    Path normalShape, toggledShape;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlassWindowButton)
};

Button* LookAndFeel_V2::createDocumentWindowButton (int buttonType)
{
    Path shape;
    const float crossThickness = 0.25f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.0f, 1.0f, 1.0f), crossThickness * 1.4f);
        shape.addLineSegment (Line<float> (1.0f, 0.0f, 0.0f, 1.0f), crossThickness * 1.4f);

        return new GlassWindowButton ("close", Colour (0xffdd1100), shape, shape);
    }

    if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        return new GlassWindowButton ("minimise", Colour (0xffaa8811), shape, shape);
    }

    if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (Line<float> (0.5f, 0.0f, 0.5f, 1.0f), crossThickness);
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new GlassWindowButton ("maximise", Colour (0xff119911), shape, fullscreenShape);
    }

    jassertfalse;
    return nullptr;
}

void LookAndFeel_V2::positionDocumentWindowButtons (DocumentWindow&,
                                                    int titleBarX, int titleBarY,
                                                    int titleBarW, int titleBarH,
                                                    Button* minimiseButton,
                                                    Button* maximiseButton,
                                                    Button* closeButton,
                                                    bool positionTitleBarButtonsOnLeft)
{
    const int buttonW = titleBarH - titleBarH / 8;

    int x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - buttonW / 4;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -(buttonW + buttonW / 4);
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimiseButton, maximiseButton);

    if (maximiseButton != nullptr)
    {
        maximiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != nullptr)
        minimiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
}

int LookAndFeel_V2::getDefaultMenuBarHeight()
{
    return 24;
}

//==============================================================================
DropShadower* LookAndFeel_V2::createDropShadowerForComponent (Component*)
{
    return new DropShadower (DropShadow (Colours::black.withAlpha (0.4f), 10, Point<int> (0, 2)));
}

//==============================================================================
void LookAndFeel_V2::drawStretchableLayoutResizerBar (Graphics& g, int w, int h,
                                                      bool /*isVerticalBar*/,
                                                      bool isMouseOver,
                                                      bool isMouseDragging)
{
    float alpha = 0.5f;

    if (isMouseOver || isMouseDragging)
    {
        g.fillAll (Colour (0x190000ff));
        alpha = 1.0f;
    }

    const float cx = w * 0.5f;
    const float cy = h * 0.5f;
    const float cr = jmin (w, h) * 0.4f;

    g.setGradientFill (ColourGradient (Colours::white.withAlpha (alpha), cx + cr * 0.1f, cy + cr,
                                       Colours::black.withAlpha (alpha), cx, cy - cr * 4.0f,
                                       true));

    g.fillEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);
}

//==============================================================================
void LookAndFeel_V2::drawGroupComponentOutline (Graphics& g, int width, int height,
                                                const String& text, const Justification& position,
                                                GroupComponent& group)
{
    const float textH = 15.0f;
    const float indent = 3.0f;
    const float textEdgeGap = 4.0f;
    float cs = 5.0f;

    Font f (textH);

    Path p;
    float x = indent;
    float y = f.getAscent() - 3.0f;
    float w = jmax (0.0f, width - x * 2.0f);
    float h = jmax (0.0f, height - y  - indent);
    cs = jmin (cs, w * 0.5f, h * 0.5f);
    const float cs2 = 2.0f * cs;

    float textW = text.isEmpty() ? 0 : jlimit (0.0f, jmax (0.0f, w - cs2 - textEdgeGap * 2), f.getStringWidth (text) + textEdgeGap * 2.0f);
    float textX = cs + textEdgeGap;

    if (position.testFlags (Justification::horizontallyCentred))
        textX = cs + (w - cs2 - textW) * 0.5f;
    else if (position.testFlags (Justification::right))
        textX = w - cs - textW - textEdgeGap;

    p.startNewSubPath (x + textX + textW, y);
    p.lineTo (x + w - cs, y);

    p.addArc (x + w - cs2, y, cs2, cs2, 0, float_Pi * 0.5f);
    p.lineTo (x + w, y + h - cs);

    p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, float_Pi * 0.5f, float_Pi);
    p.lineTo (x + cs, y + h);

    p.addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);
    p.lineTo (x, y + cs);

    p.addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f);
    p.lineTo (x + textX, y);

    const float alpha = group.isEnabled() ? 1.0f : 0.5f;

    g.setColour (group.findColour (GroupComponent::outlineColourId)
                    .withMultipliedAlpha (alpha));

    g.strokePath (p, PathStrokeType (2.0f));

    g.setColour (group.findColour (GroupComponent::textColourId)
                    .withMultipliedAlpha (alpha));
    g.setFont (f);
    g.drawText (text,
                roundToInt (x + textX), 0,
                roundToInt (textW),
                roundToInt (textH),
                Justification::centred, true);
}

//==============================================================================
int LookAndFeel_V2::getTabButtonOverlap (int tabDepth)
{
    return 1 + tabDepth / 3;
}

int LookAndFeel_V2::getTabButtonSpaceAroundImage()
{
    return 4;
}

int LookAndFeel_V2::getTabButtonBestWidth (TabBarButton& button, int tabDepth)
{
    int width = Font (tabDepth * 0.6f).getStringWidth (button.getButtonText().trim())
                   + getTabButtonOverlap (tabDepth) * 2;

    if (auto* extraComponent = button.getExtraComponent())
        width += button.getTabbedButtonBar().isVertical() ? extraComponent->getHeight()
                                                          : extraComponent->getWidth();

    return jlimit (tabDepth * 2, tabDepth * 8, width);
}

Rectangle<int> LookAndFeel_V2::getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp)
{
    Rectangle<int> extraComp;

    auto orientation = button.getTabbedButtonBar().getOrientation();

    if (button.getExtraComponentPlacement() == TabBarButton::beforeText)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromLeft   (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }
    else
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtBottom:
            case TabbedButtonBar::TabsAtTop:     extraComp = textArea.removeFromRight  (comp.getWidth()); break;
            case TabbedButtonBar::TabsAtLeft:    extraComp = textArea.removeFromTop    (comp.getHeight()); break;
            case TabbedButtonBar::TabsAtRight:   extraComp = textArea.removeFromBottom (comp.getHeight()); break;
            default:                             jassertfalse; break;
        }
    }

    return extraComp;
}

void LookAndFeel_V2::createTabButtonShape (TabBarButton& button, Path& p, bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    auto activeArea = button.getActiveArea();
    const float w = (float) activeArea.getWidth();
    const float h = (float) activeArea.getHeight();

    float length = w;
    float depth = h;

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    const float indent = (float) getTabButtonOverlap ((int) depth);
    const float overhang = 4.0f;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            p.startNewSubPath (w, 0.0f);
            p.lineTo (0.0f, indent);
            p.lineTo (0.0f, h - indent);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (w + overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtRight:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (w, indent);
            p.lineTo (w, h - indent);
            p.lineTo (0.0f, h);
            p.lineTo (-overhang, h + overhang);
            p.lineTo (-overhang, -overhang);
            break;

        case TabbedButtonBar::TabsAtBottom:
            p.startNewSubPath (0.0f, 0.0f);
            p.lineTo (indent, h);
            p.lineTo (w - indent, h);
            p.lineTo (w, 0.0f);
            p.lineTo (w + overhang, -overhang);
            p.lineTo (-overhang, -overhang);
            break;

        default:
            p.startNewSubPath (0.0f, h);
            p.lineTo (indent, 0.0f);
            p.lineTo (w - indent, 0.0f);
            p.lineTo (w, h);
            p.lineTo (w + overhang, h + overhang);
            p.lineTo (-overhang, h + overhang);
            break;
    }

    p.closeSubPath();

    p = p.createPathWithRoundedCorners (3.0f);
}

void LookAndFeel_V2::fillTabButtonShape (TabBarButton& button, Graphics& g, const Path& path,
                                         bool /*isMouseOver*/, bool /*isMouseDown*/)
{
    auto tabBackground = button.getTabBackgroundColour();
    const bool isFrontTab = button.isFrontTab();

    g.setColour (isFrontTab ? tabBackground
                            : tabBackground.withMultipliedAlpha (0.9f));

    g.fillPath (path);

    g.setColour (button.findColour (isFrontTab ? TabbedButtonBar::frontOutlineColourId
                                               : TabbedButtonBar::tabOutlineColourId, false)
                    .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    g.strokePath (path, PathStrokeType (isFrontTab ? 1.0f : 0.5f));
}

Font LookAndFeel_V2::getTabButtonFont (TabBarButton&, float height)
{
    return { height * 0.6f };
}

void LookAndFeel_V2::drawTabButtonText (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<float> area (button.getTextArea().toFloat());

    float length = area.getWidth();
    float depth  = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    Font font (getTabButtonFont (button, depth));
    font.setUnderline (button.hasKeyboardFocus (false));

    AffineTransform t;

    switch (button.getTabbedButtonBar().getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (float_Pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (float_Pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }

    Colour col;

    if (button.isFrontTab() && (button.isColourSpecified (TabbedButtonBar::frontTextColourId)
                                    || isColourSpecified (TabbedButtonBar::frontTextColourId)))
        col = findColour (TabbedButtonBar::frontTextColourId);
    else if (button.isColourSpecified (TabbedButtonBar::tabTextColourId)
                 || isColourSpecified (TabbedButtonBar::tabTextColourId))
        col = findColour (TabbedButtonBar::tabTextColourId);
    else
        col = button.getTabBackgroundColour().contrasting();

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

    g.setColour (col.withMultipliedAlpha (alpha));
    g.setFont (font);
    g.addTransform (t);

    g.drawFittedText (button.getButtonText().trim(),
                      0, 0, (int) length, (int) depth,
                      Justification::centred,
                      jmax (1, ((int) depth) / 12));
}

void LookAndFeel_V2::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    Path tabShape;
    createTabButtonShape (button, tabShape, isMouseOver, isMouseDown);

    const Rectangle<int> activeArea (button.getActiveArea());
    tabShape.applyTransform (AffineTransform::translation ((float) activeArea.getX(),
                                                           (float) activeArea.getY()));

    DropShadow (Colours::black.withAlpha (0.5f), 2, Point<int> (0, 1)).drawForPath (g, tabShape);

    fillTabButtonShape (button, g, tabShape, isMouseOver, isMouseDown);
    drawTabButtonText (button, g, isMouseOver, isMouseDown);
}

void LookAndFeel_V2::drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) {}

void LookAndFeel_V2::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h)
{
    const float shadowSize = 0.2f;

    Rectangle<int> shadowRect, line;
    ColourGradient gradient (Colours::black.withAlpha (bar.isEnabled() ? 0.25f : 0.15f), 0, 0,
                             Colours::transparentBlack, 0, 0, false);

    switch (bar.getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            gradient.point1.x = (float) w;
            gradient.point2.x = w * (1.0f - shadowSize);
            shadowRect.setBounds ((int) gradient.point2.x, 0, w - (int) gradient.point2.x, h);
            line.setBounds (w - 1, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtRight:
            gradient.point2.x = w * shadowSize;
            shadowRect.setBounds (0, 0, (int) gradient.point2.x, h);
            line.setBounds (0, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtTop:
            gradient.point1.y = (float) h;
            gradient.point2.y = h * (1.0f - shadowSize);
            shadowRect.setBounds (0, (int) gradient.point2.y, w, h - (int) gradient.point2.y);
            line.setBounds (0, h - 1, w, 1);
            break;

        case TabbedButtonBar::TabsAtBottom:
            gradient.point2.y = h * shadowSize;
            shadowRect.setBounds (0, 0, w, (int) gradient.point2.y);
            line.setBounds (0, 0, w, 1);
            break;

        default: break;
    }

    g.setGradientFill (gradient);
    g.fillRect (shadowRect.expanded (2, 2));

    g.setColour (Colour (0x80000000));
    g.fillRect (line);
}

Button* LookAndFeel_V2::createTabBarExtrasButton()
{
    const float thickness = 7.0f;
    const float indent = 22.0f;

    Path p;
    p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);

    DrawablePath ellipse;
    ellipse.setPath (p);
    ellipse.setFill (Colour (0x99ffffff));

    p.clear();
    p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
    p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
    p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
    p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
    p.setUsingNonZeroWinding (false);

    DrawablePath dp;
    dp.setPath (p);
    dp.setFill (Colour (0x59000000));

    DrawableComposite normalImage;
    normalImage.addAndMakeVisible (ellipse.createCopy());
    normalImage.addAndMakeVisible (dp.createCopy());

    dp.setFill (Colour (0xcc000000));

    DrawableComposite overImage;
    overImage.addAndMakeVisible (ellipse.createCopy());
    overImage.addAndMakeVisible (dp.createCopy());

    DrawableButton* db = new DrawableButton ("tabs", DrawableButton::ImageFitted);
    db->setImages (&normalImage, &overImage, nullptr);
    return db;
}


//==============================================================================
void LookAndFeel_V2::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    g.fillAll (Colours::white);

    Rectangle<int> area (header.getLocalBounds());
    area.removeFromTop (area.getHeight() / 2);

    auto backgroundColour = header.findColour (TableHeaderComponent::backgroundColourId);

    g.setGradientFill (ColourGradient (backgroundColour,
                                       0.0f, (float) area.getY(),
                                       backgroundColour.withMultipliedSaturation (.5f),
                                       0.0f, (float) area.getBottom(),
                                       false));
    g.fillRect (area);

    g.setColour (header.findColour (TableHeaderComponent::outlineColourId));
    g.fillRect (area.removeFromBottom (1));

    for (int i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

void LookAndFeel_V2::drawTableHeaderColumn (Graphics& g, TableHeaderComponent& header,
                                            const String& columnName, int /*columnId*/,
                                            int width, int height, bool isMouseOver, bool isMouseDown,
                                            int columnFlags)
{
    auto highlightColour = header.findColour (TableHeaderComponent::highlightColourId);

    if (isMouseDown)
        g.fillAll (highlightColour);
    else if (isMouseOver)
        g.fillAll (highlightColour.withMultipliedAlpha (0.625f));

    Rectangle<int> area (width, height);
    area.reduce (4, 0);

    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        Path sortArrow;
        sortArrow.addTriangle (0.0f, 0.0f,
                               0.5f, (columnFlags & TableHeaderComponent::sortedForwards) != 0 ? -0.8f : 0.8f,
                               1.0f, 0.0f);

        g.setColour (Colour (0x99000000));
        g.fillPath (sortArrow, sortArrow.getTransformToScaleToFit (area.removeFromRight (height / 2).reduced (2).toFloat(), true));
    }

    g.setColour (header.findColour (TableHeaderComponent::textColourId));
    g.setFont (Font (height * 0.5f, Font::bold));
    g.drawFittedText (columnName, area, Justification::centredLeft, 1);
}

//==============================================================================
void LookAndFeel_V2::drawLasso (Graphics& g, Component& lassoComp)
{
    const int outlineThickness = 1;

    g.fillAll (lassoComp.findColour (0x1000440 /*lassoFillColourId*/));

    g.setColour (lassoComp.findColour (0x1000441 /*lassoOutlineColourId*/));
    g.drawRect (lassoComp.getLocalBounds(), outlineThickness);
}

//==============================================================================
void LookAndFeel_V2::paintToolbarBackground (Graphics& g, int w, int h, Toolbar& toolbar)
{
    const Colour background (toolbar.findColour (Toolbar::backgroundColourId));

    g.setGradientFill (ColourGradient (background, 0.0f, 0.0f,
                                       background.darker (0.1f),
                                       toolbar.isVertical() ? w - 1.0f : 0.0f,
                                       toolbar.isVertical() ? 0.0f : h - 1.0f,
                                       false));
    g.fillAll();
}

Button* LookAndFeel_V2::createToolbarMissingItemsButton (Toolbar& /*toolbar*/)
{
    return createTabBarExtrasButton();
}

void LookAndFeel_V2::paintToolbarButtonBackground (Graphics& g, int /*width*/, int /*height*/,
                                                   bool isMouseOver, bool isMouseDown,
                                                   ToolbarItemComponent& component)
{
    if (isMouseDown)
        g.fillAll (component.findColour (Toolbar::buttonMouseDownBackgroundColourId, true));
    else if (isMouseOver)
        g.fillAll (component.findColour (Toolbar::buttonMouseOverBackgroundColourId, true));
}

void LookAndFeel_V2::paintToolbarButtonLabel (Graphics& g, int x, int y, int width, int height,
                                              const String& text, ToolbarItemComponent& component)
{
    g.setColour (component.findColour (Toolbar::labelTextColourId, true)
                    .withAlpha (component.isEnabled() ? 1.0f : 0.25f));

    const float fontHeight = jmin (14.0f, height * 0.85f);
    g.setFont (fontHeight);

    g.drawFittedText (text,
                      x, y, width, height,
                      Justification::centred,
                      jmax (1, height / (int) fontHeight));
}

//==============================================================================
void LookAndFeel_V2::drawPropertyPanelSectionHeader (Graphics& g, const String& name,
                                                     bool isOpen, int width, int height)
{
    const float buttonSize = height * 0.75f;
    const float buttonIndent = (height - buttonSize) * 0.5f;

    drawTreeviewPlusMinusBox (g, Rectangle<float> (buttonIndent, buttonIndent, buttonSize, buttonSize), Colours::white, isOpen, false);

    const int textX = (int) (buttonIndent * 2.0f + buttonSize + 2.0f);

    g.setColour (Colours::black);
    g.setFont (Font (height * 0.7f, Font::bold));
    g.drawText (name, textX, 0, width - textX - 4, height, Justification::centredLeft, true);
}

void LookAndFeel_V2::drawPropertyComponentBackground (Graphics& g, int width, int height, PropertyComponent& component)
{
    g.setColour (component.findColour (PropertyComponent::backgroundColourId));
    g.fillRect (0, 0, width, height - 1);
}

void LookAndFeel_V2::drawPropertyComponentLabel (Graphics& g, int, int height, PropertyComponent& component)
{
    g.setColour (component.findColour (PropertyComponent::labelTextColourId)
                    .withMultipliedAlpha (component.isEnabled() ? 1.0f : 0.6f));

    g.setFont (jmin (height, 24) * 0.65f);

    const Rectangle<int> r (getPropertyComponentContentPosition (component));

    g.drawFittedText (component.getName(),
                      3, r.getY(), r.getX() - 5, r.getHeight(),
                      Justification::centredLeft, 2);
}

Rectangle<int> LookAndFeel_V2::getPropertyComponentContentPosition (PropertyComponent& component)
{
    const int textW = jmin (200, component.getWidth() / 3);
    return Rectangle<int> (textW, 1, component.getWidth() - textW - 1, component.getHeight() - 3);
}

//==============================================================================
void LookAndFeel_V2::drawCallOutBoxBackground (CallOutBox& box, Graphics& g,
                                               const Path& path, Image& cachedImage)
{
    if (cachedImage.isNull())
    {
        cachedImage = Image (Image::ARGB, box.getWidth(), box.getHeight(), true);
        Graphics g2 (cachedImage);

        DropShadow (Colours::black.withAlpha (0.7f), 8, Point<int> (0, 2)).drawForPath (g2, path);
    }

    g.setColour (Colours::black);
    g.drawImageAt (cachedImage, 0, 0);

    g.setColour (Colour::greyLevel (0.23f).withAlpha (0.9f));
    g.fillPath (path);

    g.setColour (Colours::white.withAlpha (0.8f));
    g.strokePath (path, PathStrokeType (2.0f));
}

int LookAndFeel_V2::getCallOutBoxBorderSize (const CallOutBox&)
{
    return 20;
}

//==============================================================================
AttributedString LookAndFeel_V2::createFileChooserHeaderText (const String& title,
                                                           const String& instructions)
{
    AttributedString s;
    s.setJustification (Justification::centred);

    auto colour = findColour (FileChooserDialogBox::titleTextColourId);
    s.append (title + "\n\n", Font (17.0f, Font::bold), colour);
    s.append (instructions, Font (14.0f), colour);

    return s;
}

void LookAndFeel_V2::drawFileBrowserRow (Graphics& g, int width, int height,
                                         const File&, const String& filename, Image* icon,
                                         const String& fileSizeDescription,
                                         const String& fileTimeDescription,
                                         bool isDirectory, bool isItemSelected,
                                         int /*itemIndex*/, DirectoryContentsDisplayComponent& dcc)
{
    auto fileListComp = dynamic_cast<Component*> (&dcc);

    if (isItemSelected)
        g.fillAll (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::highlightColourId)
                                           : findColour (DirectoryContentsDisplayComponent::highlightColourId));

    const int x = 32;
    g.setColour (Colours::black);

    if (icon != nullptr && icon->isValid())
    {
        g.drawImageWithin (*icon, 2, 2, x - 4, height - 4,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);
    }
    else
    {
        if (auto* d = isDirectory ? getDefaultFolderImage()
                                  : getDefaultDocumentFileImage())
            d->drawWithin (g, Rectangle<float> (2.0f, 2.0f, x - 4.0f, height - 4.0f),
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);
    }

    g.setColour (fileListComp != nullptr ? fileListComp->findColour (DirectoryContentsDisplayComponent::textColourId)
                                         : findColour (DirectoryContentsDisplayComponent::textColourId));
    g.setFont (height * 0.7f);

    if (width > 450 && ! isDirectory)
    {
        const int sizeX = roundToInt (width * 0.7f);
        const int dateX = roundToInt (width * 0.8f);

        g.drawFittedText (filename,
                          x, 0, sizeX - x, height,
                          Justification::centredLeft, 1);

        g.setFont (height * 0.5f);
        g.setColour (Colours::darkgrey);

        if (! isDirectory)
        {
            g.drawFittedText (fileSizeDescription,
                              sizeX, 0, dateX - sizeX - 8, height,
                              Justification::centredRight, 1);

            g.drawFittedText (fileTimeDescription,
                              dateX, 0, width - 8 - dateX, height,
                              Justification::centredRight, 1);
        }
    }
    else
    {
        g.drawFittedText (filename,
                          x, 0, width - x, height,
                          Justification::centredLeft, 1);

    }
}

Button* LookAndFeel_V2::createFileBrowserGoUpButton()
{
    auto goUpButton = new DrawableButton ("up", DrawableButton::ImageOnButtonBackground);

    Path arrowPath;
    arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);

    DrawablePath arrowImage;
    arrowImage.setFill (Colours::black.withAlpha (0.4f));
    arrowImage.setPath (arrowPath);

    goUpButton->setImages (&arrowImage);

    return goUpButton;
}

void LookAndFeel_V2::layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                                 DirectoryContentsDisplayComponent* fileListComponent,
                                                 FilePreviewComponent* previewComp,
                                                 ComboBox* currentPathBox,
                                                 TextEditor* filenameBox,
                                                 Button* goUpButton)
{
    const int x = 8;
    auto w = browserComp.getWidth() - x - x;

    if (previewComp != nullptr)
    {
        auto previewWidth = w / 3;
        previewComp->setBounds (x + w - previewWidth, 0, previewWidth, browserComp.getHeight());

        w -= previewWidth + 4;
    }

    int y = 4;

    const int controlsHeight = 22;
    const int bottomSectionHeight = controlsHeight + 8;
    const int upButtonWidth = 50;

    currentPathBox->setBounds (x, y, w - upButtonWidth - 6, controlsHeight);
    goUpButton->setBounds (x + w - upButtonWidth, y, upButtonWidth, controlsHeight);

    y += controlsHeight + 4;

    if (auto listAsComp = dynamic_cast<Component*> (fileListComponent))
    {
        listAsComp->setBounds (x, y, w, browserComp.getHeight() - y - bottomSectionHeight);
        y = listAsComp->getBottom() + 4;
    }

    filenameBox->setBounds (x + 50, y, w - 50, controlsHeight);
}

// Pulls a drawable out of compressed ValueTree data..
static Drawable* loadDrawableFromData (const void* data, size_t numBytes)
{
    MemoryInputStream m (data, numBytes, false);
    GZIPDecompressorInputStream gz (m);
    ValueTree drawable (ValueTree::readFromStream (gz));
    return Drawable::createFromValueTree (drawable.getChild (0), nullptr);
}

const Drawable* LookAndFeel_V2::getDefaultFolderImage()
{
    if (folderImage == nullptr)
    {
        static const unsigned char drawableData[] =
        { 120,218,197,86,77,111,27,55,16,229,182,161,237,6,61,39,233,77,63,192,38,56,195,225,215,209,105,210,2,141,13,20,201,193,109,111,178,181,178,183,145,181,130,180,110,145,127,159,199,93,73,137,87,53,218,91,109,192,160,151,179,156,55,111,222,188,229,155,247,
        231,87,231,175,47,222,170,234,155,229,244,190,86,213,115,253,102,61,253,123,122,189,168,85,51,83,213,119,250,238,221,47,231,151,175,223,169,170,250,121,221,62,172,84,245,172,60,63,209,243,118,49,171,215,170,107,87,23,245,188,83,213,145,182,167,19,91,
        254,127,223,220,222,117,37,68,82,40,143,174,219,174,107,239,135,168,147,18,37,108,85,245,237,46,207,70,33,249,175,211,238,78,85,186,28,253,76,175,73,109,186,117,251,177,190,106,102,229,241,247,58,24,103,203,15,101,245,103,219,44,187,15,221,39,0,172,142,
        245,125,211,1,196,205,116,181,125,114,164,175,31,186,78,45,219,229,31,245,186,189,106,150,179,102,121,139,100,154,240,231,167,102,177,64,72,247,105,213,23,122,187,158,206,154,122,217,169,85,57,18,1,47,53,101,107,18,135,204,167,147,192,201,216,20,114,
        244,195,62,171,234,7,125,198,100,136,216,145,149,211,9,57,103,40,249,72,219,8,167,170,87,250,140,162,199,123,226,3,34,82,202,134,131,13,172,74,170,233,162,0,177,234,166,93,180,15,235,141,170,206,180,157,204,231,150,156,159,207,39,195,50,214,88,18,150,
        245,205,124,250,104,169,212,135,158,19,144,53,20,112,172,55,237,2,132,13,199,149,130,230,115,145,112,147,147,82,61,157,32,238,178,253,11,145,213,138,10,52,138,38,103,111,99,164,211,137,139,198,35,177,35,167,212,143,15,215,205,13,160,109,163,172,225,152,
        16,232,17,149,140,103,144,158,146,90,113,217,12,6,197,167,236,3,54,5,181,101,73,54,138,90,245,165,227,120,18,252,150,77,15,242,188,228,204,81,169,139,102,249,5,68,192,145,14,244,112,1,145,29,94,137,96,235,49,136,151,58,246,32,88,192,161,88,176,76,226,
        36,247,24,176,7,232,62,16,83,42,155,201,160,30,222,65,72,98,82,76,33,198,254,197,96,124,10,150,243,8,130,48,228,36,94,124,6,4,43,38,0,142,205,99,30,4,221,13,33,230,220,71,177,65,49,142,243,150,7,1,51,20,2,5,96,96,84,225,56,217,188,3,33,46,24,228,112,
        69,69,12,68,228,108,242,99,16,165,118,208,28,51,200,98,87,42,74,62,209,24,4,206,48,22,153,125,132,220,196,56,15,234,99,216,130,0,141,38,74,162,130,48,35,163,141,94,196,245,32,94,104,7,154,132,209,40,108,162,165,232,153,165,17,4,138,201,176,135,58,49,
        165,130,122,108,114,54,28,240,64,17,89,188,79,177,116,149,10,4,246,91,30,94,104,112,96,226,144,131,144,142,98,78,177,7,128,81,242,224,140,36,249,80,208,145,196,12,202,15,16,60,161,200,69,187,169,213,86,198,123,87,224,255,199,21,94,105,134,72,40,177,245,
        14,182,32,232,54,196,231,100,111,11,189,168,201,39,177,84,102,38,139,177,168,74,210,87,174,64,20,138,160,67,111,10,4,98,196,97,60,158,118,133,25,111,173,224,171,37,97,185,119,133,221,242,63,184,194,140,71,174,240,252,145,43,72,32,147,146,147,4,104,104,
        117,134,10,18,12,107,212,40,72,148,57,6,71,69,135,222,248,16,160,168,3,169,144,55,201,69,41,147,137,134,99,50,97,8,178,85,43,217,140,201,151,192,152,10,242,190,24,11,59,183,29,25,42,115,236,98,14,229,252,32,80,66,0,162,17,136,72,6,67,5,45,242,224,10,
        193,102,71,50,6,17,129,212,18,115,105,150,80,169,45,123,222,141,76,178,70,32,55,24,90,217,132,71,73,200,57,238,204,3,136,49,144,185,55,183,190,20,137,52,246,47,113,232,158,69,35,49,145,208,129,193,56,178,77,135,230,145,113,22,140,69,74,20,146,2,120,218,
        155,135,48,32,10,89,30,156,165,204,254,222,193,160,12,19,49,6,210,59,11,70,62,4,31,15,64,196,2,157,98,33,58,1,104,32,152,50,31,128,64,148,183,197,108,209,89,107,240,41,75,36,123,16,208,108,180,44,236,250,182,227,27,20,137,118,76,60,165,137,221,92,94,
        78,215,31,235,245,230,183,242,229,30,214,251,251,195,145,94,148,15,253,170,221,52,93,211,46,7,109,171,81,208,177,94,247,119,132,47,81,186,92,22,246,7,255,254,15,7,107,141,171,197,191,156,123,162,135,187,198,227,131,113,219,80,159,1,4,239,223,231,0,0 };

        folderImage = loadDrawableFromData (drawableData, sizeof (drawableData));
    }

    return folderImage;
}

const Drawable* LookAndFeel_V2::getDefaultDocumentFileImage()
{
    if (documentImage == nullptr)
    {
        static const unsigned char drawableData[] =
        { 120,218,213,88,77,115,219,54,16,37,147,208,246,228,214,75,155,246,164,123,29,12,176,216,197,199,49,105,218,94,156,153,78,114,72,219,155,108,75,137,26,89,212,200,116,59,233,175,239,3,105,201,164,68,50,158,166,233,76,196,11,69,60,173,128,197,123,139,183,
        124,241,234,217,155,103,207,207,126,204,242,7,171,233,213,44,203,31,23,47,54,211,191,166,231,203,89,182,184,204,242,147,226,195,165,219,252,125,150,229,249,207,155,242,102,157,229,143,210,227,199,197,101,121,113,115,53,91,85,89,85,174,207,102,243,42,
        203,143,10,125,58,209,233,251,171,197,219,119,85,250,173,97,151,30,157,151,85,85,94,53,168,147,132,50,226,179,252,225,246,143,174,179,44,63,254,101,90,189,203,242,34,5,127,84,172,77,118,93,109,202,247,179,55,139,203,244,248,97,161,179,63,202,197,170,
        122,93,125,192,196,242,227,226,106,81,205,54,217,197,116,125,251,228,168,56,191,169,170,108,85,174,126,159,109,202,55,139,213,229,98,245,182,249,97,254,240,167,197,114,137,5,86,31,214,245,111,175,203,37,254,230,162,92,150,55,155,180,148,249,237,39,203,
        94,215,127,58,10,213,245,39,203,234,249,102,249,87,47,203,63,129,204,49,227,252,73,225,149,145,104,131,245,254,116,34,202,82,164,16,153,179,236,108,177,234,7,49,41,237,130,144,167,17,144,15,42,104,239,93,12,35,32,99,68,9,187,24,125,7,244,77,23,36,164,
        40,56,226,61,12,107,229,130,215,100,105,24,227,89,17,246,211,105,55,140,49,218,43,207,100,245,72,28,195,70,17,230,201,118,8,243,164,139,233,95,88,23,52,152,162,54,104,48,217,237,105,15,111,91,107,253,131,160,118,34,239,69,128,54,232,135,101,121,61,203,
        110,169,181,147,2,253,159,82,48,180,229,247,167,74,193,41,141,188,35,93,241,116,18,148,113,214,120,207,113,47,19,109,16,51,182,153,193,5,59,2,10,90,69,114,218,135,48,2,50,198,43,171,189,152,81,144,88,108,85,136,78,246,64,54,42,163,35,69,30,3,121,82,38,
        98,81,98,70,64,70,139,34,111,163,167,49,144,13,202,138,179,58,220,23,52,180,186,54,104,48,79,109,208,96,198,219,19,31,220,187,118,10,6,65,237,100,222,139,5,109,80,191,30,236,151,162,135,147,142,30,68,105,182,58,6,22,84,43,229,124,148,116,97,145,55,231,
        139,11,76,228,16,37,14,48,205,145,77,134,34,176,55,152,182,200,57,99,93,204,144,145,253,65,97,229,132,72,104,63,62,71,21,140,54,186,41,226,59,84,19,63,130,15,222,235,224,185,59,104,27,226,68,101,153,241,227,177,248,29,20,136,26,8,252,178,183,241,219,
        131,137,160,209,107,109,92,79,124,16,211,184,104,93,77,130,110,124,2,65,172,67,201,60,157,88,163,2,91,99,92,216,198,55,78,69,75,190,150,119,84,98,200,71,150,109,124,36,204,227,52,8,33,229,223,68,167,173,167,131,248,137,212,226,141,19,233,160,154,248,
        144,142,195,140,137,185,59,104,15,247,119,40,126,23,69,81,200,242,110,254,123,20,49,94,112,110,245,199,111,241,167,87,36,252,101,138,132,149,22,22,38,65,134,29,182,139,24,230,192,31,144,184,133,130,72,44,131,210,142,111,147,216,30,76,123,30,113,206,242,
        150,196,157,65,129,130,76,180,194,61,34,225,160,5,228,233,160,118,34,137,26,202,115,212,29,108,72,134,243,223,90,114,226,199,226,119,80,6,245,152,197,122,217,146,184,53,24,140,210,30,21,59,80,79,124,182,202,71,207,218,112,159,72,80,53,140,109,68,2,191,
        227,217,210,78,36,94,137,88,231,82,157,8,176,61,0,122,191,19,137,3,255,13,39,183,228,20,193,151,144,119,166,79,36,40,253,156,138,72,11,181,19,137,14,46,176,217,27,180,135,251,219,31,255,235,61,148,165,96,72,122,118,23,229,81,52,135,24,250,163,183,216,
        211,43,17,217,151,136,253,116,137,28,53,188,127,92,188,221,76,47,23,169,59,90,167,144,141,239,197,86,104,141,189,60,157,80,84,142,140,4,31,154,241,122,105,132,41,107,13,201,39,86,120,24,82,114,206,198,6,96,27,227,172,36,232,168,201,36,219,24,113,62,163,
        154,101,233,143,166,203,102,26,141,206,174,179,252,89,161,39,243,249,197,121,186,38,233,246,146,211,53,1,123,56,194,231,122,143,103,179,217,60,204,167,19,147,110,41,93,173,219,123,72,89,248,35,173,16,220,50,179,111,60,181,24,88,103,156,235,7,78,248,14,
        4,119,78,162,93,60,112,35,109,16,124,126,12,17,71,67,24,1,165,142,1,181,215,248,56,6,66,235,193,137,167,61,22,30,5,3,27,101,71,64,169,25,112,216,2,63,22,169,110,43,18,200,140,129,208,160,88,44,220,208,125,65,67,171,107,131,6,243,212,6,13,102,188,61,241,
        225,189,107,165,96,16,212,78,230,189,88,208,6,245,235,214,237,235,150,62,167,110,155,106,170,53,133,192,117,193,20,84,78,74,174,98,39,92,156,8,112,21,46,80,106,12,209,207,225,228,16,113,59,225,126,87,60,133,25,209,34,36,2,99,242,52,197,48,30,75,244,247,
        212,238,246,182,173,221,185,78,215,127,167,221,162,163,221,250,152,217,146,196,222,145,100,223,235,105,108,28,250,149,212,74,224,86,2,213,118,110,119,204,224,144,208,38,214,131,200,14,214,223,120,189,230,53,1,193,70,133,154,131,56,223,16,229,48,188,14,
        201,205,213,121,71,233,68,89,15,124,103,37,53,26,11,118,176,127,169,88,166,158,219,178,117,173,83,108,75,95,55,68,186,193,53,246,146,206,127,6,63,53,78,58,228,204,155,224,113,74,91,232,221,195,240,105,215,34,29,138,64,128,183,8,130,233,71,173,56,54,101,
        99,75,186,111,65,58,28,229,145,82,19,152,12,99,180,81,130,131,75,234,229,220,247,53,231,154,79,205,185,185,155,199,249,172,38,85,253,204,76,68,95,92,204,207,255,221,75,178,227,14,187,224,224,97,202,172,173,219,12,167,130,133,9,54,135,245,92,176,29,134,
        165,110,139,141,18,16,223,29,188,183,65,207,144,106,144,151,143,128,224,176,168,110,140,32,62,56,110,219,195,54,235,20,68,209,216,34,232,21,6,41,234,157,39,211,201,107,160,230,66,225,56,153,9,101,21,37,237,150,204,14,115,208,22,221,54,216,230,33,116,
        14,65,14,44,19,8,236,73,71,246,182,110,125,224,75,132,195,214,247,163,36,51,252,84,76,124,37,212,100,88,62,183,179,76,67,217,218,242,244,229,116,243,126,182,185,254,21,105,126,208,220,239,94,229,30,21,203,244,202,117,93,94,47,170,69,185,106,246,60,219,
        3,29,23,155,250,109,237,29,170,72,175,109,119,129,127,235,9,92,20,85,185,254,72,220,147,162,121,235,219,13,44,144,225,63,241,244,165,51,0,0 };

        documentImage = loadDrawableFromData (drawableData, sizeof (drawableData));
    }

    return documentImage;
}

//==============================================================================
void LookAndFeel_V2::drawLevelMeter (Graphics& g, int width, int height, float level)
{
    g.setColour (Colours::white.withAlpha (0.7f));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) width, (float) height, 3.0f);
    g.setColour (Colours::black.withAlpha (0.2f));
    g.drawRoundedRectangle (1.0f, 1.0f, width - 2.0f, height - 2.0f, 3.0f, 1.0f);

    const int totalBlocks = 7;
    const int numBlocks = roundToInt (totalBlocks * level);
    const float w = (width - 6.0f) / (float) totalBlocks;

    for (int i = 0; i < totalBlocks; ++i)
    {
        if (i >= numBlocks)
            g.setColour (Colours::lightblue.withAlpha (0.6f));
        else
            g.setColour (i < totalBlocks - 1 ? Colours::blue.withAlpha (0.5f)
                                             : Colours::red);

        g.fillRoundedRectangle (3.0f + i * w + w * 0.1f, 3.0f, w * 0.8f, height - 6.0f, w * 0.4f);
    }
}

//==============================================================================
void LookAndFeel_V2::drawKeymapChangeButton (Graphics& g, int width, int height, Button& button, const String& keyDescription)
{
    const Colour textColour (button.findColour (0x100ad01 /*KeyMappingEditorComponent::textColourId*/, true));

    if (keyDescription.isNotEmpty())
    {
        if (button.isEnabled())
        {
            const float alpha = button.isDown() ? 0.3f : (button.isOver() ? 0.15f : 0.08f);
            g.fillAll (textColour.withAlpha (alpha));

            g.setOpacity (0.3f);
            drawBevel (g, 0, 0, width, height, 2);
        }

        g.setColour (textColour);
        g.setFont (height * 0.6f);
        g.drawFittedText (keyDescription,
                          3, 0, width - 6, height,
                          Justification::centred, 1);
    }
    else
    {
        const float thickness = 7.0f;
        const float indent = 22.0f;

        Path p;
        p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
        p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
        p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
        p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
        p.setUsingNonZeroWinding (false);

        g.setColour (textColour.withAlpha (button.isDown() ? 0.7f : (button.isOver() ? 0.5f : 0.3f)));
        g.fillPath (p, p.getTransformToScaleToFit (2.0f, 2.0f, width - 4.0f, height - 4.0f, true));
    }

    if (button.hasKeyboardFocus (false))
    {
        g.setColour (textColour.withAlpha (0.4f));
        g.drawRect (0, 0, width, height);
    }
}

//==============================================================================
void LookAndFeel_V2::drawBevel (Graphics& g, const int x, const int y, const int width, const int height,
                                const int bevelThickness, const Colour& topLeftColour, const Colour& bottomRightColour,
                                const bool useGradient, const bool sharpEdgeOnOutside)
{
    if (g.clipRegionIntersects (Rectangle<int> (x, y, width, height)))
    {
        LowLevelGraphicsContext& context = g.getInternalContext();
        context.saveState();

        for (int i = bevelThickness; --i >= 0;)
        {
            const float op = useGradient ? (sharpEdgeOnOutside ? bevelThickness - i : i) / (float) bevelThickness
                                         : 1.0f;

            context.setFill (topLeftColour.withMultipliedAlpha (op));
            context.fillRect (Rectangle<int> (x + i, y + i, width - i * 2, 1), false);
            context.setFill (topLeftColour.withMultipliedAlpha (op * 0.75f));
            context.fillRect (Rectangle<int> (x + i, y + i + 1, 1, height - i * 2 - 2), false);
            context.setFill (bottomRightColour.withMultipliedAlpha (op));
            context.fillRect (Rectangle<int> (x + i, y + height - i - 1, width - i * 2, 1), false);
            context.setFill (bottomRightColour.withMultipliedAlpha (op  * 0.75f));
            context.fillRect (Rectangle<int> (x + width - i - 1, y + i + 1, 1, height - i * 2 - 2), false);
        }

        context.restoreState();
    }
}

//==============================================================================
void LookAndFeel_V2::drawShinyButtonShape (Graphics& g,
                                        float x, float y, float w, float h,
                                        float maxCornerSize,
                                        const Colour& baseColour,
                                        const float strokeWidth,
                                        const bool flatOnLeft,
                                        const bool flatOnRight,
                                        const bool flatOnTop,
                                        const bool flatOnBottom) noexcept
{
    if (w <= strokeWidth * 1.1f || h <= strokeWidth * 1.1f)
        return;

    const float cs = jmin (maxCornerSize, w * 0.5f, h * 0.5f);

    Path outline;
    outline.addRoundedRectangle (x, y, w, h, cs, cs,
                                 ! (flatOnLeft  || flatOnTop),
                                 ! (flatOnRight || flatOnTop),
                                 ! (flatOnLeft  || flatOnBottom),
                                 ! (flatOnRight || flatOnBottom));

    ColourGradient cg (baseColour, 0.0f, y,
                       baseColour.overlaidWith (Colour (0x070000ff)), 0.0f, y + h,
                       false);

    cg.addColour (0.5,  baseColour.overlaidWith (Colour (0x33ffffff)));
    cg.addColour (0.51, baseColour.overlaidWith (Colour (0x110000ff)));

    g.setGradientFill (cg);
    g.fillPath (outline);

    g.setColour (Colour (0x80000000));
    g.strokePath (outline, PathStrokeType (strokeWidth));
}

//==============================================================================
void LookAndFeel_V2::drawGlassSphere (Graphics& g, const float x, const float y,
                                      const float diameter, const Colour& colour,
                                      const float outlineThickness) noexcept
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.addEllipse (x, y, diameter, diameter);

    {
        ColourGradient cg (Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColour (0.4, Colours::white.overlaidWith (colour));

        g.setGradientFill (cg);
        g.fillPath (p);
    }

    g.setGradientFill (ColourGradient (Colours::white, 0, y + diameter * 0.06f,
                                       Colours::transparentWhite, 0, y + diameter * 0.3f, false));
    g.fillEllipse (x + diameter * 0.2f, y + diameter * 0.05f, diameter * 0.6f, diameter * 0.4f);

    ColourGradient cg (Colours::transparentBlack,
                       x + diameter * 0.5f, y + diameter * 0.5f,
                       Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                       x, y + diameter * 0.5f, true);

    cg.addColour (0.7, Colours::transparentBlack);
    cg.addColour (0.8, Colours::black.withAlpha (0.1f * outlineThickness));

    g.setGradientFill (cg);
    g.fillPath (p);

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.drawEllipse (x, y, diameter, diameter, outlineThickness);
}

//==============================================================================
void LookAndFeel_V2::drawGlassPointer (Graphics& g,
                                       const float x, const float y, const float diameter,
                                       const Colour& colour, const float outlineThickness,
                                       const int direction) noexcept
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation (direction * (float_Pi * 0.5f), x + diameter * 0.5f, y + diameter * 0.5f));

    {
        ColourGradient cg (Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColour (0.4, Colours::white.overlaidWith (colour));

        g.setGradientFill (cg);
        g.fillPath (p);
    }

    ColourGradient cg (Colours::transparentBlack,
                       x + diameter * 0.5f, y + diameter * 0.5f,
                       Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                       x - diameter * 0.2f, y + diameter * 0.5f, true);

    cg.addColour (0.5, Colours::transparentBlack);
    cg.addColour (0.7, Colours::black.withAlpha (0.07f * outlineThickness));

    g.setGradientFill (cg);
    g.fillPath (p);

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.strokePath (p, PathStrokeType (outlineThickness));
}

//==============================================================================
void LookAndFeel_V2::drawGlassLozenge (Graphics& g,
                                       const float x, const float y, const float width, const float height,
                                       const Colour& colour, const float outlineThickness, const float cornerSize,
                                       const bool flatOnLeft,
                                       const bool flatOnRight,
                                       const bool flatOnTop,
                                       const bool flatOnBottom) noexcept
{
    if (width <= outlineThickness || height <= outlineThickness)
        return;

    const int intX = (int) x;
    const int intY = (int) y;
    const int intW = (int) width;
    const int intH = (int) height;

    const float cs = cornerSize < 0 ? jmin (width * 0.5f, height * 0.5f) : cornerSize;
    const float edgeBlurRadius = height * 0.75f + (height - cs * 2.0f);
    const int intEdge = (int) edgeBlurRadius;

    Path outline;
    outline.addRoundedRectangle (x, y, width, height, cs, cs,
                                 ! (flatOnLeft || flatOnTop),
                                 ! (flatOnRight || flatOnTop),
                                 ! (flatOnLeft || flatOnBottom),
                                 ! (flatOnRight || flatOnBottom));

    {
        ColourGradient cg (colour.darker (0.2f), 0, y,
                           colour.darker (0.2f), 0, y + height, false);

        cg.addColour (0.03, colour.withMultipliedAlpha (0.3f));
        cg.addColour (0.4, colour);
        cg.addColour (0.97, colour.withMultipliedAlpha (0.3f));

        g.setGradientFill (cg);
        g.fillPath (outline);
    }

    ColourGradient cg (Colours::transparentBlack, x + edgeBlurRadius, y + height * 0.5f,
                       colour.darker (0.2f), x, y + height * 0.5f, true);

    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.5f) / edgeBlurRadius), Colours::transparentBlack);
    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.25f) / edgeBlurRadius), colour.darker (0.2f).withMultipliedAlpha (0.3f));

    if (! (flatOnLeft || flatOnTop || flatOnBottom))
    {
        g.saveState();
        g.setGradientFill (cg);
        g.reduceClipRegion (intX, intY, intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    if (! (flatOnRight || flatOnTop || flatOnBottom))
    {
        cg.point1.setX (x + width - edgeBlurRadius);
        cg.point2.setX (x + width);

        g.saveState();
        g.setGradientFill (cg);
        g.reduceClipRegion (intX + intW - intEdge, intY, 2 + intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    {
        const float leftIndent = flatOnTop || flatOnLeft ? 0.0f : cs * 0.4f;
        const float rightIndent = flatOnTop || flatOnRight ? 0.0f : cs * 0.4f;

        Path highlight;
        highlight.addRoundedRectangle (x + leftIndent,
                                       y + cs * 0.1f,
                                       width - (leftIndent + rightIndent),
                                       height * 0.4f,
                                       cs * 0.4f,
                                       cs * 0.4f,
                                       ! (flatOnLeft || flatOnTop),
                                       ! (flatOnRight || flatOnTop),
                                       ! (flatOnLeft || flatOnBottom),
                                       ! (flatOnRight || flatOnBottom));

        g.setGradientFill (ColourGradient (colour.brighter (10.0f), 0, y + height * 0.06f,
                                           Colours::transparentWhite, 0, y + height * 0.4f, false));
        g.fillPath (highlight);
    }

    g.setColour (colour.darker().withMultipliedAlpha (1.5f));
    g.strokePath (outline, PathStrokeType (outlineThickness));
}

} // namespace juce
