/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_LookAndFeel.h"
#include "../buttons/juce_TextButton.h"
#include "../buttons/juce_ToggleButton.h"
#include "../buttons/juce_ShapeButton.h"
#include "../buttons/juce_ArrowButton.h"
#include "../buttons/juce_DrawableButton.h"
#include "../buttons/juce_HyperlinkButton.h"
#include "../windows/juce_AlertWindow.h"
#include "../windows/juce_DocumentWindow.h"
#include "../windows/juce_ResizableWindow.h"
#include "../menus/juce_MenuBarComponent.h"
#include "../menus/juce_PopupMenu.h"
#include "../layout/juce_ScrollBar.h"
#include "../mouse/juce_LassoComponent.h"
#include "../controls/juce_Slider.h"
#include "../controls/juce_ListBox.h"
#include "../controls/juce_TableHeaderComponent.h"
#include "../controls/juce_Toolbar.h"
#include "../controls/juce_ToolbarItemComponent.h"
#include "../controls/juce_ProgressBar.h"
#include "../controls/juce_TreeView.h"
#include "../filebrowser/juce_FilenameComponent.h"
#include "../filebrowser/juce_DirectoryContentsDisplayComponent.h"
#include "../layout/juce_GroupComponent.h"
#include "../properties/juce_PropertyComponent.h"
#include "../juce_Desktop.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../../graphics/brushes/juce_GradientBrush.h"
#include "../../graphics/fonts/juce_GlyphArrangement.h"
#include "../../graphics/drawables/juce_DrawableComposite.h"
#include "../../graphics/drawables/juce_DrawablePath.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"
#include "../special/juce_MidiKeyboardComponent.h"
#include "../special/juce_ColourSelector.h"


//==============================================================================
static const Colour createBaseColour (const Colour& buttonColour,
                                      const bool hasKeyboardFocus,
                                      const bool isMouseOverButton,
                                      const bool isButtonDown) throw()
{
    const float sat = hasKeyboardFocus ? 1.3f : 0.9f;
    const Colour baseColour (buttonColour.withMultipliedSaturation (sat));

    if (isButtonDown)
        return baseColour.contrasting (0.2f);
    else if (isMouseOverButton)
        return baseColour.contrasting (0.1f);

    return baseColour;
}

//==============================================================================
LookAndFeel::LookAndFeel()
{
    /* if this fails it means you're trying to create a LookAndFeel object before
       the static Colours have been initialised. That ain't gonna work. It probably
       means that you're using a static LookAndFeel object and that your compiler has
       decided to intialise it before the Colours class.
    */
    jassert (Colours::white == Colour (0xffffffff));

    // set up the standard set of colours..
    #define textButtonColour        0xffbbbbff
    #define textHighlightColour     0x401111ee
    #define standardOutlineColour   0xb2808080

    static const int standardColours[] =
    {
        TextButton::buttonColourId,                 textButtonColour,
        TextButton::buttonOnColourId,               0xff4444ff,
        TextButton::textColourId,                   0xff000000,

        ComboBox::buttonColourId,                   0xffbbbbff,
        ComboBox::outlineColourId,                  standardOutlineColour,

        ToggleButton::textColourId,                 0xff000000,

        TextEditor::backgroundColourId,             0xffffffff,
        TextEditor::textColourId,                   0xff000000,
        TextEditor::highlightColourId,              textHighlightColour,
        TextEditor::highlightedTextColourId,        0xff000000,
        TextEditor::caretColourId,                  0xff000000,
        TextEditor::outlineColourId,                0x00000000,
        TextEditor::focusedOutlineColourId,         textButtonColour,
        TextEditor::shadowColourId,                 0x38000000,

        Label::backgroundColourId,                  0x00000000,
        Label::textColourId,                        0xff000000,
        Label::outlineColourId,                     0x00000000,

        ScrollBar::backgroundColourId,              0x00000000,
        ScrollBar::thumbColourId,                   0xffffffff,

        TreeView::linesColourId,                    0x4c000000,
        TreeView::backgroundColourId,               0x00000000,

        PopupMenu::backgroundColourId,              0xffffffff,
        PopupMenu::textColourId,                    0xff000000,
        PopupMenu::headerTextColourId,              0xff000000,
        PopupMenu::highlightedTextColourId,         0xffffffff,
        PopupMenu::highlightedBackgroundColourId,   0x991111aa,

        ComboBox::textColourId,                     0xff000000,
        ComboBox::backgroundColourId,               0xffffffff,

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

        AlertWindow::backgroundColourId,            0xffededed,
        AlertWindow::textColourId,                  0xff000000,
        AlertWindow::outlineColourId,               0xff666666,

        ProgressBar::backgroundColourId,            0xffffffff,
        ProgressBar::foregroundColourId,            0xffaaaaee,

        TooltipWindow::backgroundColourId,          0xffeeeebb,
        TooltipWindow::textColourId,                0xff000000,
        TooltipWindow::outlineColourId,             0x4c000000,

        Toolbar::backgroundColourId,                0xfff6f8f9,
        Toolbar::separatorColourId,                 0x4c000000,
        Toolbar::buttonMouseOverBackgroundColourId, 0x4c0000ff,
        Toolbar::buttonMouseDownBackgroundColourId, 0x800000ff,
        Toolbar::labelTextColourId,                 0xff000000,
        Toolbar::editingModeOutlineColourId,        0xffff0000,

        HyperlinkButton::textColourId,              0xcc1111ee,

        GroupComponent::outlineColourId,            0x66000000,
        GroupComponent::textColourId,               0xff000000,

        DirectoryContentsDisplayComponent::highlightColourId,    textHighlightColour,
        DirectoryContentsDisplayComponent::textColourId,         0xff000000,

        0x1000440, /*LassoComponent::lassoFillColourId*/         0x66dddddd,
        0x1000441, /*LassoComponent::lassoOutlineColourId*/      0x99111111,

        MidiKeyboardComponent::whiteNoteColourId,                0xffffffff,
        MidiKeyboardComponent::blackNoteColourId,                0xff000000,
        MidiKeyboardComponent::keySeparatorLineColourId,         0x66000000,
        MidiKeyboardComponent::mouseOverKeyOverlayColourId,      0x80ffff00,
        MidiKeyboardComponent::keyDownOverlayColourId,           0xffb6b600,
        MidiKeyboardComponent::textLabelColourId,                0xff000000,
        MidiKeyboardComponent::upDownButtonBackgroundColourId,   0xffd3d3d3,
        MidiKeyboardComponent::upDownButtonArrowColourId,        0xff000000,

        ColourSelector::backgroundColourId,         0xffe5e5e5,
        ColourSelector::labelTextColourId,          0xff000000
    };

    for (int i = 0; i < numElementsInArray (standardColours); i += 2)
        setColour (standardColours [i], Colour (standardColours [i + 1]));
}

LookAndFeel::~LookAndFeel()
{
}

//==============================================================================
const Colour LookAndFeel::findColour (const int colourId) const throw()
{
    const int index = colourIds.indexOf (colourId);

    if (index >= 0)
        return colours [index];

    jassertfalse
    return Colours::black;
}

void LookAndFeel::setColour (const int colourId, const Colour& colour) throw()
{
    const int index = colourIds.indexOf (colourId);

    if (index >= 0)
        colours.set (index, colour);

    colourIds.add (colourId);
    colours.add (colour);
}

//==============================================================================
static LookAndFeel* defaultLF = 0;
static LookAndFeel* currentDefaultLF = 0;

LookAndFeel& LookAndFeel::getDefaultLookAndFeel() throw()
{
    // if this happens, your app hasn't initialised itself properly.. if you're
    // trying to hack your own main() function, have a look at
    // JUCEApplication::initialiseForGUI()
    jassert (currentDefaultLF != 0);

    return *currentDefaultLF;
}

void LookAndFeel::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) throw()
{
    if (newDefaultLookAndFeel == 0)
    {
        if (defaultLF == 0)
            defaultLF = new LookAndFeel();

        newDefaultLookAndFeel = defaultLF;
    }

    currentDefaultLF = newDefaultLookAndFeel;

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        Component* const c = Desktop::getInstance().getComponent (i);

        if (c != 0)
            c->sendLookAndFeelChange();
    }
}

void LookAndFeel::clearDefaultLookAndFeel() throw()
{
    if (currentDefaultLF == defaultLF)
        currentDefaultLF = 0;

    deleteAndZero (defaultLF);
}

//==============================================================================
void LookAndFeel::drawButtonBackground (Graphics& g,
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

    const Colour baseColour (createBaseColour (backgroundColour,
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

void LookAndFeel::drawTickBox (Graphics& g,
                               Component& component,
                               int x, int y, int w, int h,
                               const bool ticked,
                               const bool isEnabled,
                               const bool isMouseOverButton,
                               const bool isButtonDown)
{
    const float boxSize = w * 0.7f;

    drawGlassSphere (g, (float) x, y + (h - boxSize) * 0.5f, boxSize,
                     createBaseColour (component.findColour (TextButton::buttonColourId)
                                                .withMultipliedAlpha (isEnabled ? 1.0f : 0.5f),
                                       true,
                                       isMouseOverButton,
                                       isButtonDown),
                     isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColour (isEnabled ? Colours::black : Colours::grey);

        const AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                         .translated ((float) x, (float) y));

        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

void LookAndFeel::drawToggleButton (Graphics& g,
                                    ToggleButton& button,
                                    bool isMouseOverButton,
                                    bool isButtonDown)
{
    if (button.hasKeyboardFocus (true))
    {
        g.setColour (button.findColour (TextEditor::focusedOutlineColourId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

    const int tickWidth = jmin (20, button.getHeight() - 4);

    drawTickBox (g, button, 4, (button.getHeight() - tickWidth) / 2,
                 tickWidth, tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColour (button.findColour (ToggleButton::textColourId));
    g.setFont (jmin (15.0f, button.getHeight() * 0.6f));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    const int textX = tickWidth + 5;

    g.drawFittedText (button.getButtonText(),
                      textX, 4,
                      button.getWidth() - textX - 2, button.getHeight() - 8,
                      Justification::centredLeft, 10);
}

void LookAndFeel::changeToggleButtonWidthToFitText (ToggleButton& button)
{
    Font font (jmin (15.0f, button.getHeight() * 0.6f));

    const int tickWidth = jmin (24, button.getHeight());

    button.setSize (font.getStringWidth (button.getButtonText()) + tickWidth + 8,
                    button.getHeight());
}

void LookAndFeel::drawAlertBox (Graphics& g,
                                AlertWindow& alert,
                                const Rectangle& textArea,
                                TextLayout& textLayout)
{
    const int iconWidth = 80;

    const Colour background (alert.findColour (AlertWindow::backgroundColourId));

    g.fillAll (background);

    int iconSpaceUsed = 0;
    Justification alignment (Justification::horizontallyCentred);

    int iconSize = jmin (iconWidth + 50, alert.getHeight() + 20);

    if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
        iconSize = jmin (iconSize, textArea.getHeight() + 50);

    const Rectangle iconRect (iconSize / -10,
                              iconSize / -10,
                              iconSize,
                              iconSize);

    if (alert.getAlertType() == AlertWindow::QuestionIcon
         || alert.getAlertType() == AlertWindow::InfoIcon)
    {
        if (alert.getAlertType() == AlertWindow::InfoIcon)
            g.setColour (background.overlaidWith (Colour (0x280000ff)));
        else
            g.setColour (background.overlaidWith (Colours::gold.darker().withAlpha (0.25f)));

        g.fillEllipse ((float) iconRect.getX(),
                       (float) iconRect.getY(),
                       (float) iconRect.getWidth(),
                       (float) iconRect.getHeight());

        g.setColour (background);
        g.setFont (iconRect.getHeight() * 0.9f, Font::bold);
        g.drawText ((alert.getAlertType() == AlertWindow::InfoIcon) ? "i"
                                                                    : "?",
                    iconRect.getX(),
                    iconRect.getY(),
                    iconRect.getWidth(),
                    iconRect.getHeight(),
                    Justification::centred, false);

        iconSpaceUsed = iconWidth;
        alignment = Justification::left;
    }
    else if (alert.getAlertType() == AlertWindow::WarningIcon)
    {
        Path p;
        p.addTriangle (iconRect.getX() + iconRect.getWidth() * 0.5f,
                       (float) iconRect.getY(),
                       (float) iconRect.getRight(),
                       (float) iconRect.getBottom(),
                       (float) iconRect.getX(),
                       (float) iconRect.getBottom());

        g.setColour (background.overlaidWith (Colour (0x33ff0000)));
        g.fillPath (p.createPathWithRoundedCorners (5.0f));

        g.setColour (background);
        g.setFont (iconRect.getHeight() * 0.9f, Font::bold);

        g.drawText (T("!"),
                    iconRect.getX(),
                    iconRect.getY(),
                    iconRect.getWidth(),
                    iconRect.getHeight() + iconRect.getHeight() / 8,
                    Justification::centred, false);

        iconSpaceUsed = iconWidth;
        alignment = Justification::left;
    }

    g.setColour (alert.findColour (AlertWindow::textColourId));

    textLayout.drawWithin (g,
                           textArea.getX() + iconSpaceUsed,
                           textArea.getY(),
                           textArea.getWidth() - iconSpaceUsed,
                           textArea.getHeight(),
                           alignment.getFlags() | Justification::top);

    g.setColour (alert.findColour (AlertWindow::outlineColourId));
    g.drawRect (0, 0, alert.getWidth(), alert.getHeight());
}

int LookAndFeel::getAlertBoxWindowFlags()
{
    return ComponentPeer::windowAppearsOnTaskbar
            | ComponentPeer::windowHasDropShadow;
}

void LookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                   int x, int y, int w, int h,
                                   float progress)
{
    const Colour background (progressBar.findColour (ProgressBar::backgroundColourId));
    g.fillAll (background);

    g.setColour (background.contrasting (0.2f));
    g.drawRect (x, y, w, h);

    drawGlassLozenge (g,
                      (float) (x + 1),
                      (float) (y + 1),
                      jlimit (0.0f, w - 2.0f, progress * (w - 2.0f)),
                      (float) (h - 2),
                      progressBar.findColour (ProgressBar::foregroundColourId),
                      0.5f,
                      0.0f,
                      true, true, true, true);
}

void LookAndFeel::drawScrollbarButton (Graphics& g,
                                       ScrollBar& scrollbar,
                                       int width, int height,
                                       int buttonDirection,
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

void LookAndFeel::drawScrollbar (Graphics& g,
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

    GradientBrush gb (thumbColour.overlaidWith (Colour (0x44000000)),
                      gx1, gy1,
                      thumbColour.overlaidWith (Colour (0x19000000)),
                      gx2, gy2, false);

    g.setBrush (&gb);
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

    GradientBrush gb2 (Colours::transparentBlack,
                       gx1, gy1,
                       Colour (0x19000000),
                       gx2, gy2, false);

    g.setBrush (&gb2);
    g.fillPath (slotPath);

    g.setColour (thumbColour);
    g.fillPath (thumbPath);

    GradientBrush gb3 (Colour (0x10000000),
                       gx1, gy1,
                       Colours::transparentBlack,
                       gx2, gy2, false);

    g.saveState();
    g.setBrush (&gb3);

    if (isScrollbarVertical)
        g.reduceClipRegion (x + width / 2, y, width, height);
    else
        g.reduceClipRegion (x, y + height / 2, width, height);

    g.fillPath (thumbPath);
    g.restoreState();

    g.setColour (Colour (0x4c000000));
    g.strokePath (thumbPath, PathStrokeType (0.4f));
}

ImageEffectFilter* LookAndFeel::getScrollbarEffect()
{
    return 0;
}

int LookAndFeel::getMinimumScrollbarThumbSize (ScrollBar& scrollbar)
{
    return jmin (scrollbar.getWidth(), scrollbar.getHeight()) * 2;
}

int LookAndFeel::getDefaultScrollbarWidth()
{
    return 18;
}

int LookAndFeel::getScrollbarButtonSize (ScrollBar& scrollbar)
{
    return 2 + (scrollbar.isVertical() ? scrollbar.getWidth()
                                       : scrollbar.getHeight());
}

//==============================================================================
const Path LookAndFeel::getTickShape (const float height)
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

const Path LookAndFeel::getCrossShape (const float height)
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
void LookAndFeel::drawTreeviewPlusMinusBox (Graphics& g, int x, int y, int w, int h, bool isPlus)
{
    const int boxSize = ((jmin (16, w, h) << 1) / 3) | 1;

    x += (w - boxSize) >> 1;
    y += (h - boxSize) >> 1;
    w = boxSize;
    h = boxSize;

    g.setColour (Colour (0xe5ffffff));
    g.fillRect (x, y, w, h);

    g.setColour (Colour (0x80000000));
    g.drawRect (x, y, w, h);

    const float size = boxSize / 2 + 1.0f;
    const float centre = (float) (boxSize / 2);

    g.fillRect (x + (w - size) * 0.5f, y + centre, size, 1.0f);

    if (isPlus)
        g.fillRect (x + centre, y + (h - size) * 0.5f, 1.0f, size);
}

//==============================================================================
void LookAndFeel::drawBubble (Graphics& g,
                              float tipX, float tipY,
                              float boxX, float boxY,
                              float boxW, float boxH)
{
    int side = 0;

    if (tipX < boxX)
        side = 1;
    else if (tipX > boxX + boxW)
        side = 3;
    else if (tipY > boxY + boxH)
        side = 2;

    const float indent = 2.0f;
    Path p;
    p.addBubble (boxX + indent,
                 boxY + indent,
                 boxW - indent * 2.0f,
                 boxH - indent * 2.0f,
                 5.0f,
                 tipX, tipY,
                 side,
                 0.5f,
                 jmin (15.0f, boxW * 0.3f, boxH * 0.3f));

    //xxx need to take comp as param for colour
    g.setColour (findColour (TooltipWindow::backgroundColourId).withAlpha (0.9f));
    g.fillPath (p);

    //xxx as above
    g.setColour (findColour (TooltipWindow::textColourId).withAlpha (0.4f));
    g.strokePath (p, PathStrokeType (1.33f));
}


//==============================================================================
const Font LookAndFeel::getPopupMenuFont()
{
    return Font (17.0f);
}

void LookAndFeel::getIdealPopupMenuItemSize (const String& text,
                                             const bool isSeparator,
                                             int standardMenuItemHeight,
                                             int& idealWidth,
                                             int& idealHeight)
{
    if (isSeparator)
    {
        idealWidth = 50;
        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight / 2 : 10;
    }
    else
    {
        Font font (getPopupMenuFont());

        idealHeight = standardMenuItemHeight > 0 ? standardMenuItemHeight : roundFloatToInt (font.getHeight() * 1.3f);
        idealWidth = font.getStringWidth (text) + idealHeight * 2;
    }
}

void LookAndFeel::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    g.fillAll (background);
    g.setColour (background.overlaidWith (Colour (0x2badd8e6)));

    for (int i = 0; i < height; i += 3)
        g.fillRect (0, i, width, 1);

    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
}

void LookAndFeel::drawPopupMenuUpDownArrow (Graphics& g,
                                            int width, int height,
                                            bool isScrollUpArrow)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    GradientBrush gb (background,
                      0.0f, height * 0.5f,
                      background.withAlpha (0.0f),
                      0.0f, isScrollUpArrow ? ((float) height) : 0.0f,
                      false);

    g.setBrush (&gb);
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

void LookAndFeel::drawPopupMenuItem (Graphics& g,
                                     int width, int height,
                                     const bool isSeparator,
                                     const bool isActive,
                                     const bool isHighlighted,
                                     const bool isTicked,
                                     const bool hasSubMenu,
                                     const String& text,
                                     const String& shortcutKeyText,
                                     Image* image,
                                     const Colour* const textColourToUse)
{
    const float halfH = height * 0.5f;

    if (isSeparator)
    {
        const float separatorIndent = 5.5f;

        g.setColour (Colour (0x33000000));
        g.drawLine (separatorIndent, halfH, width - separatorIndent, halfH);

        g.setColour (Colour (0x66ffffff));
        g.drawLine (separatorIndent, halfH + 1.0f, width - separatorIndent, halfH + 1.0f);
    }
    else
    {
        Colour textColour (findColour (PopupMenu::textColourId));

        if (textColourToUse != 0)
            textColour = *textColourToUse;

        if (isHighlighted)
        {
            g.setColour (findColour (PopupMenu::highlightedBackgroundColourId));
            g.fillRect (1, 1, width - 2, height - 2);

            g.setColour (findColour (PopupMenu::highlightedTextColourId));
        }
        else
        {
            g.setColour (textColour);
        }

        if (! isActive)
            g.setOpacity (0.3f);

        Font font (getPopupMenuFont());

        if (font.getHeight() > height / 1.3f)
            font.setHeight (height / 1.3f);

        g.setFont (font);

        const int leftBorder = (height * 5) / 4;
        const int rightBorder = 4;

        if (image != 0)
        {
            g.drawImageWithin (image,
                               2, 1, leftBorder - 4, height - 2,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
        }
        else if (isTicked)
        {
            const Path tick (getTickShape (1.0f));
            const float th = font.getAscent();
            const float ty = halfH - th * 0.5f;

            g.fillPath (tick, tick.getTransformToScaleToFit (2.0f, ty, (float) (leftBorder - 4),
                                                             th, true));
        }

        g.drawFittedText (text,
                          leftBorder, 0,
                          width - (leftBorder + rightBorder), height,
                          Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty())
        {
            Font f2 (g.getCurrentFont());
            f2.setHeight (f2.getHeight() * 0.75f);
            f2.setHorizontalScale (0.95f);
            g.setFont (f2);

            g.drawText (shortcutKeyText,
                        leftBorder,
                        0,
                        width - (leftBorder + rightBorder + 4),
                        height,
                        Justification::centredRight,
                        true);
        }

        if (hasSubMenu)
        {
            const float arrowH = 0.6f * getPopupMenuFont().getAscent();
            const float x = width - height * 0.6f;

            Path p;
            p.addTriangle (x, halfH - arrowH * 0.5f,
                           x, halfH + arrowH * 0.5f,
                           x + arrowH * 0.6f, halfH);

            g.fillPath (p);
        }
    }
}

//==============================================================================
int LookAndFeel::getMenuWindowFlags()
{
    return ComponentPeer::windowHasDropShadow;
}

void LookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height,
                                         bool, MenuBarComponent& menuBar)
{
    const Colour baseColour (createBaseColour (menuBar.findColour (PopupMenu::backgroundColourId), false, false, false));

    if (menuBar.isEnabled())
    {
        drawShinyButtonShape (g,
                              -4.0f, 0.0f,
                              width + 8.0f, (float) height,
                              0.0f,
                              baseColour,
                              0.4f,
                              true, true, true, true);
    }
    else
    {
        g.fillAll (baseColour);
    }
}

const Font LookAndFeel::getMenuBarFont (MenuBarComponent& menuBar, int /*itemIndex*/, const String& /*itemText*/)
{
    return Font (menuBar.getHeight() * 0.7f);
}

int LookAndFeel::getMenuBarItemWidth (MenuBarComponent& menuBar, int itemIndex, const String& itemText)
{
    return getMenuBarFont (menuBar, itemIndex, itemText)
            .getStringWidth (itemText) + menuBar.getHeight();
}

void LookAndFeel::drawMenuBarItem (Graphics& g,
                                   int width, int height,
                                   int itemIndex,
                                   const String& itemText,
                                   bool isMouseOverItem,
                                   bool isMenuOpen,
                                   bool /*isMouseOverBar*/,
                                   MenuBarComponent& menuBar)
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

//==============================================================================
void LookAndFeel::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
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
            g.drawBevel (0, 0, width, height + 2, border + 2, shadowColour, shadowColour);
        }
        else
        {
            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawRect (0, 0, width, height);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId));
            g.drawBevel (0, 0, width, height + 2, 3, shadowColour, shadowColour);
        }
    }
}

//==============================================================================
void LookAndFeel::drawComboBox (Graphics& g, int width, int height,
                                const bool isButtonDown,
                                int buttonX, int buttonY,
                                int buttonW, int buttonH,
                                ComboBox& box)
{
    g.fillAll (box.findColour (ComboBox::backgroundColourId));

    if (box.isEnabled() && box.hasKeyboardFocus (false))
    {
        g.setColour (box.findColour (TextButton::buttonColourId));
        g.drawRect (0, 0, width, height, 2);
    }
    else
    {
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRect (0, 0, width, height);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    const Colour baseColour (createBaseColour (box.findColour (ComboBox::buttonColourId),
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

        g.setColour (Colour (0x99000000));
        g.fillPath (p);
    }
}

const Font LookAndFeel::getComboBoxFont (ComboBox& box)
{
    const Font f (jmin (15.0f, box.getHeight() * 0.85f));
    return f;
}

//==============================================================================
void LookAndFeel::drawLinearSlider (Graphics& g,
                                    int x, int y,
                                    int width, int height,
                                    float sliderPos,
                                    float minSliderPos,
                                    float maxSliderPos,
                                    const Slider::SliderStyle style,
                                    Slider& slider)
{
    g.fillAll (slider.findColour (Slider::backgroundColourId));

    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (style == Slider::LinearBar)
    {
        Colour baseColour (createBaseColour (slider.findColour (Slider::thumbColourId)
                                                   .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f),
                                             false,
                                             isMouseOver, isMouseOver || slider.isMouseButtonDown()));

        drawShinyButtonShape (g,
                              (float) x, (float) y, sliderPos - (float) x, (float) height, 0.0f,
                              baseColour,
                              slider.isEnabled() ? 0.9f : 0.3f,
                              true, true, true, true);
    }
    else
    {
        const float sliderRadius = (float) getSliderThumbRadius (slider);

        const Colour trackColour (slider.findColour (Slider::trackColourId));
        const Colour gradCol1 (trackColour.overlaidWith (Colours::black.withAlpha (slider.isEnabled() ? 0.25f : 0.13f)));
        const Colour gradCol2 (trackColour.overlaidWith (Colour (0x14000000)));
        Path indent;

        if (slider.isHorizontal())
        {
            const float iy = y + height * 0.5f - sliderRadius * 0.5f;
            const float ih = sliderRadius;

            GradientBrush gb (gradCol1, 0.0f, iy,
                              gradCol2, 0.0f, iy + ih, false);
            g.setBrush (&gb);

            indent.addRoundedRectangle (x - sliderRadius * 0.5f, iy,
                                        width + sliderRadius, ih,
                                        5.0f);
            g.fillPath (indent);
        }
        else
        {
            const float ix = x + width * 0.5f - sliderRadius * 0.5f;
            const float iw = sliderRadius;

            GradientBrush gb (gradCol1, ix, 0.0f,
                              gradCol2, ix + iw, 0.0f, false);
            g.setBrush (&gb);

            indent.addRoundedRectangle (ix, y - sliderRadius * 0.5f,
                                        iw, height + sliderRadius,
                                        5.0f);
            g.fillPath (indent);
        }

        g.setColour (Colour (0x4c000000));
        g.strokePath (indent, PathStrokeType (0.5f));

        Colour knobColour (createBaseColour (slider.findColour (Slider::thumbColourId),
                                             slider.hasKeyboardFocus (false) && slider.isEnabled(),
                                             isMouseOver,
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
}

int LookAndFeel::getSliderThumbRadius (Slider& slider)
{
    return jmin (7,
                 slider.getHeight() / 2,
                 slider.getWidth() / 2);
}

void LookAndFeel::drawRotarySlider (Graphics& g,
                                    int x, int y,
                                    int width, int height,
                                    float sliderPos,
                                    const float rotaryStartAngle,
                                    const float rotaryEndAngle,
                                    Slider& slider)
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
            filledArc.addPieSegment (rx, ry, rw, rw,
                                    rotaryStartAngle,
                                    angle,
                                    thickness);

            g.fillPath (filledArc);
        }

        if (thickness > 0)
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
        {
            g.setColour (slider.findColour (Slider::rotarySliderOutlineColourId));
            Path outlineArc;
            outlineArc.addPieSegment (rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, thickness);
            outlineArc.closeSubPath();

            g.strokePath (outlineArc, PathStrokeType (slider.isEnabled() ? (isMouseOver ? 2.0f : 1.2f) : 0.3f));
        }
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

        p.addLineSegment (0.0f, 0.0f, 0.0f, -radius, rw * 0.2f);

        g.fillPath (p, AffineTransform::rotation (angle).translated (centreX, centreY));
    }
}

Button* LookAndFeel::createSliderButton (const bool isIncrement)
{
    return new TextButton (isIncrement ? "+" : "-", String::empty);
}

Label* LookAndFeel::createSliderTextBox (Slider& slider)
{
    Label* const l = new Label (T("n"), String::empty);

    l->setJustificationType (Justification::centred);

    l->setColour (Label::textColourId, slider.findColour (Slider::textBoxTextColourId));

    l->setColour (Label::backgroundColourId,
                  (slider.getSliderStyle() == Slider::LinearBar) ? Colours::transparentBlack
                                                                 : slider.findColour (Slider::textBoxBackgroundColourId));
    l->setColour (Label::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));

    l->setColour (TextEditor::textColourId, slider.findColour (Slider::textBoxTextColourId));

    l->setColour (TextEditor::backgroundColourId,
                  slider.findColour (Slider::textBoxBackgroundColourId)
                        .withAlpha (slider.getSliderStyle() == Slider::LinearBar ? 0.7f : 1.0f));

    l->setColour (TextEditor::outlineColourId, slider.findColour (Slider::textBoxOutlineColourId));

    return l;
}

ImageEffectFilter* LookAndFeel::getSliderEffect()
{
    return 0;
}

//==============================================================================
static const TextLayout layoutTooltipText (const String& text) throw()
{
    const float tooltipFontSize = 15.0f;
    const int maxToolTipWidth = 400;

    const Font f (tooltipFontSize, Font::bold);
    TextLayout tl (text, f);
    tl.layout (maxToolTipWidth, Justification::left, true);

    return tl;
}

void LookAndFeel::getTooltipSize (const String& tipText, int& width, int& height)
{
    const TextLayout tl (layoutTooltipText (tipText));

    width = tl.getWidth() + 14;
    height = tl.getHeight() + 10;
}

void LookAndFeel::drawTooltip (Graphics& g, const String& text, int width, int height)
{
    g.fillAll (findColour (TooltipWindow::backgroundColourId));

    const Colour textCol (findColour (TooltipWindow::textColourId));

    g.setColour (findColour (TooltipWindow::outlineColourId));
    g.drawRect (0, 0, width, height);

    const TextLayout tl (layoutTooltipText (text));

    g.setColour (findColour (TooltipWindow::textColourId));
    tl.drawWithin (g, 0, 0, width, height, Justification::centred);
}

//==============================================================================
Button* LookAndFeel::createFilenameComponentBrowseButton (const String& text)
{
    return new TextButton (text, TRANS("click to browse for a different file"));
}

void LookAndFeel::layoutFilenameComponent (FilenameComponent& filenameComp,
                                           ComboBox* filenameBox,
                                           Button* browseButton)
{
    browseButton->setSize (80, filenameComp.getHeight());

    TextButton* const tb = dynamic_cast <TextButton*> (browseButton);

    if (tb != 0)
        tb->changeWidthToFitText();

    browseButton->setTopRightPosition (filenameComp.getWidth(), 0);

    filenameBox->setBounds (0, 0, browseButton->getX(), filenameComp.getHeight());
}

//==============================================================================
void LookAndFeel::drawCornerResizer (Graphics& g,
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

void LookAndFeel::drawResizableFrame (Graphics&, int /*w*/, int /*h*/,
                                      const BorderSize& /*borders*/)
{
}

//==============================================================================
void LookAndFeel::drawResizableWindowBorder (Graphics& g, int w, int h,
                                             const BorderSize& border, ResizableWindow&)
{
    g.setColour (Colour (0x80000000));
    g.drawRect (0, 0, w, h);

    g.setColour (Colour (0x19000000));
    g.drawRect (border.getLeft() - 1,
                border.getTop() - 1,
                w + 2 - border.getLeftAndRight(),
                h + 2 - border.getTopAndBottom());
}

void LookAndFeel::drawDocumentWindowTitleBar (DocumentWindow& window,
                                              Graphics& g, int w, int h,
                                              int titleSpaceX, int titleSpaceW,
                                              const Image* icon,
                                              bool drawTitleTextOnLeft)
{
    const bool isActive = window.isActiveWindow();

    GradientBrush gb (window.getBackgroundColour(),
                      0.0f, 0.0f,
                      window.getBackgroundColour().contrasting (isActive ? 0.15f : 0.05f),
                      0.0f, (float) h, false);
    g.setBrush (&gb);
    g.fillAll();

    g.setFont (h * 0.65f, Font::bold);

    int textW = g.getCurrentFont().getStringWidth (window.getName());
    int iconW = 0;
    int iconH = 0;

    if (icon != 0)
    {
        iconH = (int) g.getCurrentFont().getHeight();
        iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
    }

    textW = jmin (titleSpaceW, textW + iconW);
    int textX = drawTitleTextOnLeft ? titleSpaceX
                                    : jmax (titleSpaceX, (w - textW) / 2);

    if (textX + textW > titleSpaceX + titleSpaceW)
        textX = titleSpaceX + titleSpaceW - textW;

    if (icon != 0)
    {
        g.setOpacity (isActive ? 1.0f : 0.6f);
        g.drawImageWithin (icon, textX, (h - iconH) / 2, iconW, iconH,
                           RectanglePlacement::centred, false);
        textX += iconW;
        textW -= iconW;
    }

    g.setColour (window.getBackgroundColour().contrasting (isActive ? 0.7f : 0.4f));
    g.drawText (window.getName(), textX, 0, textW, h, Justification::centredLeft, true);
}

//==============================================================================
class GlassWindowButton   : public Button
{
public:
    //==============================================================================
    GlassWindowButton (const String& name, const Colour& col,
                       const Path& normalShape_,
                       const Path& toggledShape_) throw()
        : Button (name),
          colour (col),
          normalShape (normalShape_),
          toggledShape (toggledShape_)
    {
    }

    ~GlassWindowButton()
    {
    }

    //==============================================================================
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
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

        GradientBrush gb1 (Colour::greyLevel (0.9f).withAlpha (alpha), 0, y + diam,
                           Colour::greyLevel (0.6f).withAlpha (alpha), 0, y, false);

        g.setBrush (&gb1);
        g.fillEllipse (x, y, diam, diam);

        x += 2.0f;
        y += 2.0f;
        diam -= 4.0f;

        LookAndFeel::drawGlassSphere (g, x, y, diam, colour.withAlpha (alpha), 1.0f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        const AffineTransform t (p.getTransformToScaleToFit (x + diam * 0.3f, y + diam * 0.3f,
                                                             diam * 0.4f, diam * 0.4f, true));

        g.setColour (Colours::black.withAlpha (alpha * 0.6f));
        g.fillPath (p, t);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Colour colour;
    Path normalShape, toggledShape;

    GlassWindowButton (const GlassWindowButton&);
    const GlassWindowButton& operator= (const GlassWindowButton&);
};

Button* LookAndFeel::createDocumentWindowButton (int buttonType)
{
    Path shape;
    const float crossThickness = 0.25f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (0.0f, 0.0f, 1.0f, 1.0f, crossThickness * 1.4f);
        shape.addLineSegment (1.0f, 0.0f, 0.0f, 1.0f, crossThickness * 1.4f);

        return new GlassWindowButton ("close", Colour (0xffdd1100), shape, shape);
    }
    else if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (0.0f, 0.5f, 1.0f, 0.5f, crossThickness);

        return new GlassWindowButton ("minimise", Colour (0xffaa8811), shape, shape);
    }
    else if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (0.5f, 0.0f, 0.5f, 1.0f, crossThickness);
        shape.addLineSegment (0.0f, 0.5f, 1.0f, 0.5f, crossThickness);

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

    jassertfalse
    return 0;
}

void LookAndFeel::positionDocumentWindowButtons (DocumentWindow&,
                                                 int titleBarX,
                                                 int titleBarY,
                                                 int titleBarW,
                                                 int titleBarH,
                                                 Button* minimiseButton,
                                                 Button* maximiseButton,
                                                 Button* closeButton,
                                                 bool positionTitleBarButtonsOnLeft)
{
    const int buttonW = titleBarH - titleBarH / 8;

    int x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - buttonW / 4;

    if (closeButton != 0)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -(buttonW + buttonW / 4);
    }

    if (positionTitleBarButtonsOnLeft)
        swapVariables (minimiseButton, maximiseButton);

    if (maximiseButton != 0)
    {
        maximiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != 0)
        minimiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
}

int LookAndFeel::getDefaultMenuBarHeight()
{
    return 24;
}

//==============================================================================
DropShadower* LookAndFeel::createDropShadowerForComponent (Component*)
{
    return new DropShadower (0.4f, 1, 5, 10);
}


//==============================================================================
void LookAndFeel::drawStretchableLayoutResizerBar (Graphics& g,
                                                   int w, int h,
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

    GradientBrush gb (Colours::white.withAlpha (alpha), cx + cr * 0.1f, cy + cr,
                      Colours::black.withAlpha (alpha), cx, cy - cr * 4.0f,
                      true);

    g.setBrush (&gb);
    g.fillEllipse (cx - cr, cy - cr, cr * 2.0f, cr * 2.0f);
}

//==============================================================================
void LookAndFeel::drawGroupComponentOutline (Graphics& g, int width, int height,
                                             const String& text,
                                             const Justification& position,
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
                roundFloatToInt (x + textX), 0,
                roundFloatToInt (textW),
                roundFloatToInt (textH),
                Justification::centred, true);
}

//==============================================================================
int LookAndFeel::getTabButtonOverlap (int tabDepth)
{
    return 1 + tabDepth / 3;
}

void LookAndFeel::createTabButtonShape (Path& p,
                                        int width, int height,
                                        int /*tabIndex*/,
                                        const String& /*text*/,
                                        Button& /*button*/,
                                        TabbedButtonBar::Orientation orientation,
                                        const bool /*isMouseOver*/,
                                        const bool /*isMouseDown*/,
                                        const bool /*isFrontTab*/)
{
    const float w = (float) width;
    const float h = (float) height;

    float length = w;
    float depth = h;

    if (orientation == TabbedButtonBar::TabsAtLeft
         || orientation == TabbedButtonBar::TabsAtRight)
    {
        swapVariables (length, depth);
    }

    const float indent = (float) getTabButtonOverlap ((int) depth);
    const float overhang = 4.0f;

    if (orientation == TabbedButtonBar::TabsAtLeft)
    {
        p.startNewSubPath (w, 0.0f);
        p.lineTo (0.0f, indent);
        p.lineTo (0.0f, h - indent);
        p.lineTo (w, h);
        p.lineTo (w + overhang, h + overhang);
        p.lineTo (w + overhang, -overhang);
    }
    else if (orientation == TabbedButtonBar::TabsAtRight)
    {
        p.startNewSubPath (0.0f, 0.0f);
        p.lineTo (w, indent);
        p.lineTo (w, h - indent);
        p.lineTo (0.0f, h);
        p.lineTo (-overhang, h + overhang);
        p.lineTo (-overhang, -overhang);
    }
    else if (orientation == TabbedButtonBar::TabsAtBottom)
    {
        p.startNewSubPath (0.0f, 0.0f);
        p.lineTo (indent, h);
        p.lineTo (w - indent, h);
        p.lineTo (w, 0.0f);
        p.lineTo (w + overhang, -overhang);
        p.lineTo (-overhang, -overhang);
    }
    else
    {
        p.startNewSubPath (0.0f, h);
        p.lineTo (indent, 0.0f);
        p.lineTo (w - indent, 0.0f);
        p.lineTo (w, h);
        p.lineTo (w + overhang, h + overhang);
        p.lineTo (-overhang, h + overhang);
    }

    p.closeSubPath();

    p = p.createPathWithRoundedCorners (3.0f);
}

void LookAndFeel::fillTabButtonShape (Graphics& g,
                                      const Path& path,
                                      const Colour& preferredColour,
                                      int /*tabIndex*/,
                                      const String& /*text*/,
                                      Button& button,
                                      TabbedButtonBar::Orientation /*orientation*/,
                                      const bool /*isMouseOver*/,
                                      const bool /*isMouseDown*/,
                                      const bool isFrontTab)
{
    g.setColour (isFrontTab ? preferredColour
                            : preferredColour.withMultipliedAlpha (0.9f));

    g.fillPath (path);

    g.setColour (Colours::black.withAlpha (button.isEnabled() ? 0.5f : 0.25f));
    g.strokePath (path, PathStrokeType (isFrontTab ? 1.0f : 0.5f));
}

void LookAndFeel::drawTabButtonText (Graphics& g,
                                     int x, int y, int w, int h,
                                     const Colour& preferredBackgroundColour,
                                     int /*tabIndex*/,
                                     const String& text,
                                     Button& button,
                                     TabbedButtonBar::Orientation orientation,
                                     const bool isMouseOver,
                                     const bool isMouseDown,
                                     const bool /*isFrontTab*/)
{
    int length = w;
    int depth = h;

    if (orientation == TabbedButtonBar::TabsAtLeft
         || orientation == TabbedButtonBar::TabsAtRight)
    {
        swapVariables (length, depth);
    }

    Font font (depth * 0.6f);
    font.setUnderline (button.hasKeyboardFocus (false));

    GlyphArrangement textLayout;
    textLayout.addFittedText (font, text.trim(),
                              0.0f, 0.0f, (float) length, (float) depth,
                              Justification::centred,
                              jmax (1, depth / 12));

    AffineTransform transform;

    if (orientation == TabbedButtonBar::TabsAtLeft)
    {
        transform = transform.rotated (float_Pi * -0.5f)
                             .translated ((float) x, (float) (y + h));
    }
    else if (orientation  == TabbedButtonBar::TabsAtRight)
    {
        transform = transform.rotated (float_Pi * 0.5f)
                             .translated ((float) (x + w), (float) y);
    }
    else
    {
        transform = transform.translated ((float) x, (float) y);
    }

    g.setColour (preferredBackgroundColour.contrasting());

    if (! (isMouseOver || isMouseDown))
        g.setOpacity (0.8f);

    if (! button.isEnabled())
        g.setOpacity (0.3f);

    textLayout.draw (g, transform);
}

int LookAndFeel::getTabButtonBestWidth (int /*tabIndex*/,
                                        const String& text,
                                        int tabDepth,
                                        Button&)
{
    Font f (tabDepth * 0.6f);
    return f.getStringWidth (text.trim()) + getTabButtonOverlap (tabDepth) * 2;
}

void LookAndFeel::drawTabButton (Graphics& g,
                                 int w, int h,
                                 const Colour& preferredColour,
                                 int tabIndex,
                                 const String& text,
                                 Button& button,
                                 TabbedButtonBar::Orientation orientation,
                                 const bool isMouseOver,
                                 const bool isMouseDown,
                                 const bool isFrontTab)
{
    int length = w;
    int depth = h;

    if (orientation == TabbedButtonBar::TabsAtLeft
            || orientation == TabbedButtonBar::TabsAtRight)
    {
        swapVariables (length, depth);
    }

    Path tabShape;

    createTabButtonShape (tabShape, w, h,
                          tabIndex, text, button, orientation,
                          isMouseOver, isMouseDown, isFrontTab);

    fillTabButtonShape (g, tabShape, preferredColour,
                        tabIndex, text, button, orientation,
                        isMouseOver, isMouseDown, isFrontTab);

    const int indent = getTabButtonOverlap (depth);
    int x = 0, y = 0;

    if (orientation == TabbedButtonBar::TabsAtLeft
         || orientation == TabbedButtonBar::TabsAtRight)
    {
        y += indent;
        h -= indent * 2;
    }
    else
    {
        x += indent;
        w -= indent * 2;
    }

    drawTabButtonText (g, x, y, w, h, preferredColour,
                       tabIndex, text, button, orientation,
                       isMouseOver, isMouseDown, isFrontTab);
}

void LookAndFeel::drawTabAreaBehindFrontButton (Graphics& g,
                                                int w, int h,
                                                TabbedButtonBar& tabBar,
                                                TabbedButtonBar::Orientation orientation)
{
    const float shadowSize = 0.2f;

    float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
    Rectangle shadowRect;

    if (orientation == TabbedButtonBar::TabsAtLeft)
    {
        x1 = (float) w;
        x2 = w * (1.0f - shadowSize);
        shadowRect.setBounds ((int) x2, 0, w - (int) x2, h);
    }
    else if (orientation == TabbedButtonBar::TabsAtRight)
    {
        x2 = w * shadowSize;
        shadowRect.setBounds (0, 0, (int) x2, h);
    }
    else if (orientation == TabbedButtonBar::TabsAtBottom)
    {
        y2 = h * shadowSize;
        shadowRect.setBounds (0, 0, w, (int) y2);
    }
    else
    {
        y1 = (float) h;
        y2 = h * (1.0f - shadowSize);
        shadowRect.setBounds (0, (int) y2, w, h - (int) y2);
    }

    GradientBrush gb (Colours::black.withAlpha (tabBar.isEnabled() ? 0.3f : 0.15f), x1, y1,
                      Colours::transparentBlack, x2, y2,
                      false);

    g.setBrush (&gb);
    shadowRect.expand (2, 2);
    g.fillRect (shadowRect);

    g.setColour (Colour (0x80000000));

    if (orientation == TabbedButtonBar::TabsAtLeft)
    {
        g.fillRect (w - 1, 0, 1, h);
    }
    else if (orientation == TabbedButtonBar::TabsAtRight)
    {
        g.fillRect (0, 0, 1, h);
    }
    else if (orientation == TabbedButtonBar::TabsAtBottom)
    {
        g.fillRect (0, 0, w, 1);
    }
    else
    {
        g.fillRect (0, h - 1, w, 1);
    }
}

Button* LookAndFeel::createTabBarExtrasButton()
{
    const float thickness = 7.0f;
    const float indent = 22.0f;

    Path p;
    p.addEllipse (-10.0f, -10.0f, 120.0f, 120.0f);

    DrawablePath ellipse;
    ellipse.setPath (p);
    ellipse.setSolidFill (Colour (0x99ffffff));

    p.clear();
    p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
    p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
    p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
    p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
    p.setUsingNonZeroWinding (false);

    DrawablePath dp;
    dp.setPath (p);
    dp.setSolidFill (Colour (0x59000000));

    DrawableComposite normalImage;
    normalImage.insertDrawable (ellipse);
    normalImage.insertDrawable (dp);

    dp.setSolidFill (Colour (0xcc000000));

    DrawableComposite overImage;
    overImage.insertDrawable (ellipse);
    overImage.insertDrawable (dp);

    DrawableButton* db = new DrawableButton (T("tabs"), DrawableButton::ImageFitted);
    db->setImages (&normalImage, &overImage, 0);
    return db;
}


//==============================================================================
void LookAndFeel::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    g.fillAll (Colours::white);

    const int w = header.getWidth();
    const int h = header.getHeight();

    GradientBrush gb (Colour (0xffe8ebf9), 0.0f, h * 0.5f,
                      Colour (0xfff6f8f9), 0.0f, h - 1.0f,
                      false);

    g.setBrush (&gb);
    g.fillRect (0, h / 2, w, h);

    g.setColour (Colour (0x33000000));
    g.fillRect (0, h - 1, w, 1);

    for (int i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).getRight() - 1, 0, 1, h - 1);
}

void LookAndFeel::drawTableHeaderColumn (Graphics& g, const String& columnName, int /*columnId*/,
                                         int width, int height,
                                         bool isMouseOver, bool isMouseDown,
                                         int columnFlags)
{
    if (isMouseDown)
        g.fillAll (Colour (0x8899aadd));
    else if (isMouseOver)
        g.fillAll (Colour (0x5599aadd));

    int rightOfText = width - 4;

    if ((columnFlags & (TableHeaderComponent::sortedForwards | TableHeaderComponent::sortedBackwards)) != 0)
    {
        const float top = height * ((columnFlags & TableHeaderComponent::sortedForwards) != 0 ? 0.35f : (1.0f - 0.35f));
        const float bottom = height - top;

        const float w = height * 0.5f;
        const float x = rightOfText - (w * 1.25f);
        rightOfText = (int) x;

        Path sortArrow;
        sortArrow.addTriangle (x, bottom, x + w * 0.5f, top, x + w, bottom);

        g.setColour (Colour (0x99000000));
        g.fillPath (sortArrow);
    }

    g.setColour (Colours::black);
    g.setFont (height * 0.5f, Font::bold);
    const int textX = 4;
    g.drawFittedText (columnName, textX, 0, rightOfText - textX, height, Justification::centredLeft, 1);
}


//==============================================================================
void LookAndFeel::paintToolbarBackground (Graphics& g, int w, int h, Toolbar& toolbar)
{
    const Colour background (toolbar.findColour (Toolbar::backgroundColourId));

    GradientBrush gb (background, 0.0f, 0.0f,
                      background.darker (0.1f),
                      toolbar.isVertical() ? w - 1.0f : 0.0f,
                      toolbar.isVertical() ? 0.0f : h - 1.0f,
                      false);

    g.setBrush (&gb);
    g.fillAll();
}

Button* LookAndFeel::createToolbarMissingItemsButton (Toolbar& /*toolbar*/)
{
    return createTabBarExtrasButton();
}

void LookAndFeel::paintToolbarButtonBackground (Graphics& g, int /*width*/, int /*height*/,
                                                bool isMouseOver, bool isMouseDown,
                                                ToolbarItemComponent& component)
{
    if (isMouseDown)
        g.fillAll (component.findColour (Toolbar::buttonMouseDownBackgroundColourId, true));
    else if (isMouseOver)
        g.fillAll (component.findColour (Toolbar::buttonMouseOverBackgroundColourId, true));
}

void LookAndFeel::paintToolbarButtonLabel (Graphics& g, int x, int y, int width, int height,
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
void LookAndFeel::drawPropertyPanelSectionHeader (Graphics& g, const String& name,
                                                  bool isOpen, int width, int height)
{
    const int buttonSize = (height * 3) / 4;
    const int buttonIndent = (height - buttonSize) / 2;

    drawTreeviewPlusMinusBox (g, buttonIndent, buttonIndent, buttonSize, buttonSize, ! isOpen);

    const int textX = buttonIndent * 2 + buttonSize + 2;

    g.setColour (Colours::black);
    g.setFont (height * 0.7f, Font::bold);
    g.drawText (name, textX, 0, width - textX - 4, height, Justification::centredLeft, true);
}

void LookAndFeel::drawPropertyComponentBackground (Graphics& g, int width, int height,
                                                   PropertyComponent&)
{
    g.setColour (Colour (0x66ffffff));
    g.fillRect (0, 0, width, height - 1);
}

void LookAndFeel::drawPropertyComponentLabel (Graphics& g, int, int height,
                                              PropertyComponent& component)
{
    g.setColour (Colours::black);

    if (! component.isEnabled())
        g.setOpacity (g.getCurrentColour().getAlpha() * 0.6f);

    g.setFont (jmin (height, 24) * 0.65f);

    const Rectangle r (getPropertyComponentContentPosition (component));

    g.drawFittedText (component.getName(),
                      3, r.getY(), r.getX() - 5, r.getHeight(),
                      Justification::centredLeft, 2);
}

const Rectangle LookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    return Rectangle (component.getWidth() / 3, 1,
                      component.getWidth() - component.getWidth() / 3 - 1, component.getHeight() - 3);
}

//==============================================================================
void LookAndFeel::createFileChooserHeaderText (const String& title,
                                               const String& instructions,
                                               GlyphArrangement& text,
                                               int width)
{
    text.clear();

    text.addJustifiedText (Font (17.0f, Font::bold), title,
                           8.0f, 22.0f, width - 16.0f,
                           Justification::centred);

    text.addJustifiedText (Font (14.0f), instructions,
                           8.0f, 24.0f + 16.0f, width - 16.0f,
                           Justification::centred);
}

void LookAndFeel::drawFileBrowserRow (Graphics& g, int width, int height,
                                      const String& filename, Image* icon,
                                      const String& fileSizeDescription,
                                      const String& fileTimeDescription,
                                      const bool isDirectory,
                                      const bool isItemSelected)
{
    if (isItemSelected)
        g.fillAll (findColour (DirectoryContentsDisplayComponent::highlightColourId));

    g.setColour (findColour (DirectoryContentsDisplayComponent::textColourId));
    g.setFont (height * 0.7f);

    Image* im = icon;
    Image* toRelease = 0;

    if (im == 0)
    {
        toRelease = im = (isDirectory ? getDefaultFolderImage()
                                      : getDefaultDocumentFileImage());
    }

    const int x = 32;

    if (im != 0)
    {
        g.drawImageWithin (im, 2, 2, x - 4, height - 4,
                           RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize,
                           false);

        ImageCache::release (toRelease);
    }

    if (width > 450 && ! isDirectory)
    {
        const int sizeX = roundFloatToInt (width * 0.7f);
        const int dateX = roundFloatToInt (width * 0.8f);

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

Image* LookAndFeel::getDefaultFolderImage()
{
    static const unsigned char foldericon_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,32,0,0,0,28,8,6,0,0,0,0,194,189,34,0,0,0,4,103,65,77,65,0,0,175,200,55,5,
        138,233,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,9,46,73,68,65,84,120,218,98,252,255,255,63,3,50,240,41,95,192,
        197,205,198,32,202,204,202,33,241,254,235,47,133,47,191,24,180,213,164,133,152,69,24,222,44,234,42,77,188,245,31,170,129,145,145,145,1,29,128,164,226,91,86,113,252,248,207,200,171,37,39,204,239,170,43,
        254,206,218,88,231,61,62,61,0,1,196,2,149,96,116,200,158,102,194,202,201,227,197,193,206,166,194,204,193,33,195,202,204,38,42,197,197,42,196,193,202,33,240,241,231,15,134,151,95,127,9,2,149,22,0,241,47,
        152,230,128,134,245,204,63,191,188,103,83,144,16,16,228,229,102,151,76,239,217,32,199,204,198,169,205,254,159,65,245,203,79,6,169,131,151,30,47,1,42,91,10,196,127,208,236,101,76,235,90,43,101,160,40,242,
        19,32,128,64,78,98,52,12,41,149,145,215,52,89,162,38,35,107,39,196,203,203,192,206,194,206,192,197,198,202,192,203,197,198,192,205,193,206,240,252,227,103,134,139,55,175,191,127,243,242,78,219,187,207,
        63,215,255,98,23,48,228,227,96,83,98,102,102,85,225,224,228,80,20,224,230,86,226,225,228,150,103,101,97,101,230,227,228,96,224,0,234,191,243,252,5,195,222,19,199,38,191,127,112,161,83,66,199,86,141,131,
        149,69,146,133,153,69,137,149,133,89,157,141,131,77,83,140,143,243,219,255,31,159,123,0,2,136,69,90,207,129,157,71,68,42,66,71,73,209,210,81,91,27,24,142,140,12,127,255,253,103,0,185,236,31,3,144,6,50,
        148,68,216,25,216,24,117,4,239,11,243,214,49,50,51,84,178,48,114,240,112,177,114,177,240,115,113,49,241,112,112,48,176,179,178,51,176,48,49,3,85,255,99,248,253,247,15,195,247,159,191,25,30,191,126,253,
        71,74,76,200,66,75,197,119,138,168,144,160,150,168,0,183,160,152,32,15,175,188,184,32,199,175,191,127,25,214,31,184,120,247,236,209,253,159,0,2,136,133,95,70,93,74,88,80,196,83,69,66,130,149,9,104,219,
        151,31,191,193,150,194,146,6,136,102,102,98,100,16,227,231,103,16,23,210,230,101,101,102,100,248,255,143,137,225,223,63,6,6,22,102,38,134,239,191,126,49,220,123,241,134,225,227,247,175,64,7,252,101,96,
        97,249,207,192,193,198,200,160,171,34,192,108,165,235,104,42,204,207,101,42,194,199,197,192,199,201,198,192,197,193,202,192,198,202,194,176,247,194,3,134,155,183,110,61,188,127,124,221,19,128,0,92,146,
        49,14,64,64,16,69,63,153,85,16,52,18,74,71,112,6,87,119,0,165,160,86,138,32,172,216,29,49,182,84,253,169,94,94,230,127,17,87,133,34,146,174,3,88,126,240,219,164,147,113,31,145,244,152,112,179,211,130,
        34,31,203,113,162,233,6,36,49,163,174,74,124,140,60,141,144,165,161,220,228,25,3,24,105,255,17,168,101,1,139,245,188,93,104,251,73,239,235,50,90,189,111,175,0,98,249,254,254,249,175,239,223,190,126,6,
        5,27,19,47,90,170,102,0,249,158,129,129,141,133,25,228,20,6,38,38,72,74,7,185,243,243,247,239,12,23,31,60,98,228,231,253,207,144,227,107,206,32,202,199,193,240,249,251,127,134,95,191,255,49,124,249,250,
        159,225,237,239,95,12,63,127,1,35,229,31,194,71,32,71,63,123,251,245,223,197,27,183,159,189,187,178,103,61,80,232,59,64,0,177,48,252,5,134,225,255,191,223,126,254,250,13,182,132,1,41,167,176,3,53,128,
        188,254,226,253,103,96,212,252,96,120,247,249,203,255,79,223,191,254,255,250,235,199,191,239,63,191,255,87,145,17,100,73,116,181,100,252,249,243,63,195,149,123,223,193,14,132,101,55,96,52,3,125,255,15,
        204,254,15,132,160,232,253,13,20,124,248,226,227,223,23,207,30,221,120,119,255,226,109,160,210,31,0,1,196,242,231,219,135,175,140,255,126,190,7,197,37,35,19,34,216,65,248,211,143,111,255,79,223,121,240,
        255,211,183,79,76,220,156,172,12,236,204,140,140,252,124,28,140,250,226,82,140,106,82,34,140,124,156,156,12,175,222,253,1,90,4,137,162,63,127,33,161,6,178,242,215,239,255,224,160,255,15,198,12,64,7,48,
        128,211,200,253,151,111,254,254,248,240,236,44,80,217,71,80,246,4,8,32,160,31,255,255,100,102,248,243,238,199,159,63,16,221,16,19,128,248,31,195,181,199,207,254,255,253,247,133,49,212,78,27,104,8,11,40,
        94,25,184,216,89,129,108,38,70,144,242,183,31,17,105,230,63,148,248,15,97,49,252,248,249,15,20,85,72,105,9,148,187,254,49,220,127,254,242,207,243,75,135,14,128,130,31,84,64,1,4,16,203,247,143,175,127,
        48,253,254,246,234,7,48,206,96,137,13,4,64,65,248,234,195,7,6,7,3,57,70,33,46,97,134,111,63,254,50,252,5,250,244,51,216,103,255,192,185,0,150,91,80,44,135,242,127,253,129,164,23,24,96,102,250,207,112,
        255,213,219,255,247,31,63,188,251,246,201,173,199,176,2,13,32,128,88,62,188,121,241,243,211,231,207,31,126,2,147,236,63,168,6,144,193,223,190,255,254,207,198,198,192,40,35,44,206,240,252,205,79,6,132,
        223,24,224,150,32,251,28,25,128,211,29,19,170,24,51,48,88,111,61,127,206,248,254,245,179,139,192,18,247,219,239,239,95,192,249,9,32,128,88,126,124,249,248,231,203,183,111,159,128,33,240,15,24,68,160,180,
        2,204,223,140,12,111,63,127,102,16,228,229,4,6,53,35,195,31,176,119,25,112,3,70,84,55,0,203,50,112,33,134,108,249,103,160,7,159,189,126,253,235,235,227,203,7,255,255,251,247,13,86,63,0,4,16,168,46,248,
        199,250,231,243,235,159,191,126,254,248,245,251,47,23,11,51,51,48,184,152,24,94,127,250,248,95,68,136,151,241,243,55,96,208,51,160,218,255,31,139,27,144,197,254,98,201,202,79,223,124,96,120,245,232,250,
        185,119,143,174,95,250,243,243,219,119,152,60,64,0,129,2,234,223,183,215,15,95,48,254,255,253,3,146,109,192,229,5,195,135,47,159,25,248,184,121,24,126,0,227,29,88,240,49,252,101,36,14,255,1,90,249,7,156,
        222,17,24,24,164,12,207,223,189,99,248,250,252,230,97,96,229,245,2,104,231,111,152,3,0,2,8,228,128,191,15,239,220,120,255,255,223,159,47,160,116,0,42,44,222,124,250,244,239,207,255,63,12,236,108,236,64,
        67,65,81,0,52,244,63,113,248,47,52,10,96,14,98,2,230,191,119,223,127,48,60,121,254,248,235,151,55,207,46,1,163,252,35,114,128,1,4,16,40,10,254,191,121,249,252,199,175,159,63,191,254,2,230,45,118,22,22,
        134,219,207,94,252,231,224,100,103,250,247,15,148,32,64,85,12,34,14,254,227,72,6,255,225,9,240,63,138,26,46,96,214,189,249,244,37,195,139,167,143,30,124,253,246,253,9,40,245,255,71,202,30,0,1,196,2,226,
        0,243,232,159,239,63,127,124,253,11,202,94,64,169,23,31,62,50,138,137,242,49,50,0,211,195,223,255,80,7,252,199,159,6,224,137,145,9,146,231,153,160,165,218,23,96,29,240,244,237,59,134,111,175,31,95,250,
        252,230,241,83,244,182,1,64,0,177,192,28,14,76,132,31,128,169,19,88,220,126,253,207,206,198,196,32,38,36,0,244,61,11,176,148,251,139,145,3,208,29,0,178,16,82,228,66,42,174,223,192,26,8,152,162,25,222,
        125,248,200,240,242,253,39,134,151,79,238,126,254,242,242,238,177,15,47,30,190,5,215,242,72,0,32,128,224,14,96,254,255,231,61,168,92,123,241,254,253,127,1,62,78,6,78,110,78,134,223,64,195,254,50,98,183,
        24,36,12,202,179,224,202,9,88,228,253,132,90,250,246,211,71,134,55,175,94,254,122,255,250,249,247,15,175,159,126,249,251,237,195,135,95,175,110,31,122,117,251,244,49,160,150,111,255,209,218,128,0,1,152,
        44,183,21,0,65,32,136,110,247,254,255,243,122,9,187,64,105,174,74,22,138,25,173,80,208,194,188,238,156,151,217,217,15,32,182,197,37,83,201,4,31,243,178,169,232,242,214,224,223,252,103,175,35,85,1,41,129,
        228,148,142,8,214,30,32,149,6,161,204,109,182,53,236,184,156,78,142,147,195,153,89,35,198,3,87,166,249,220,227,198,59,218,48,252,223,185,111,30,1,132,228,128,127,31,222,124,248,248,27,24,152,28,60,220,
        220,12,44,172,172,224,224,103,5,102,98,144,133,160,236,244,229,231,47,134,239,223,127,49,188,121,251,158,225,241,179,103,12,31,223,189,254,251,227,221,139,55,191,62,188,120,246,235,205,189,59,207,238,
        94,58,241,228,254,109,144,101,159,128,248,51,40,9,32,97,80,217,255,15,221,1,0,1,4,143,130,207,159,191,126,252,246,234,213,111,94,126,94,118,73,94,9,198,127,64,223,126,252,246,147,225,243,215,239,12,223,
        128,229,198,251,15,239,24,62,189,126,249,227,203,171,135,47,63,189,122,252,228,235,155,199,247,95,63,188,118,227,197,227,123,247,127,255,250,249,30,104,198,7,32,126,11,181,252,7,212,183,160,4,247,7,155,
        197,48,0,16,64,112,7,60,121,241,238,189,16,207,15,134,63,63,216,25,95,125,248,198,112,227,241,27,134,15,239,223,50,124,126,245,228,253,143,55,143,158,191,123,116,237,226,171,135,55,175,126,253,252,225,
        229,183,47,159,95,254,253,245,227,253,175,159,223,223,193,124,7,181,20,84,105,252,70,143,103,124,0,32,128,224,14,224,102,253,251,81,144,253,223,235,167,207,30,254,124,127,231,252,155,143,175,159,188,250,
        246,254,249,125,96,60,62,248,250,233,253,147,119,207,238,221,6,150,214,175,129,106,191,130,18,19,146,133,120,125,72,8,0,4,16,34,27,190,121,112,251,3,211,159,69,143,110,223,229,120,255,232,230,221,215,
        79,239,62,4,102,203,207,72,241,9,11,218,63,72,89,137,20,207,98,100,93,16,0,8,32,70,144,1,64,14,168,209,199,7,196,194,160,166,27,212,135,95,96,65,10,173,95,254,34,219,6,51,128,88,7,96,235,21,129,0,64,0,
        193,28,192,8,174,53,33,152,1,155,133,184,12,196,165,4,151,133,232,0,32,192,0,151,97,210,163,246,134,208,52,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

    return ImageCache::getFromMemory (foldericon_png, sizeof (foldericon_png));
}

Image* LookAndFeel::getDefaultDocumentFileImage()
{
    static const unsigned char fileicon_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,32,0,0,0,32,8,6,0,0,0,115,122,122,244,0,0,0,4,103,65,77,65,0,0,175,200,55,5,
        138,233,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,4,99,73,68,65,84,120,218,98,252,255,255,63,3,12,48,50,50,50,1,
        169,127,200,98,148,2,160,153,204,64,243,254,226,146,7,8,32,22,52,203,255,107,233,233,91,76,93,176,184,232,239,239,95,127,24,40,112,8,19,51,203,255,179,23,175,108,1,90,190,28,104,54,43,80,232,207,127,44,
        62,3,8,32,6,144,24,84,156,25,132,189,252,3,146,255,83,9,220,127,254,242,134,162,138,170,10,208,92,144,3,152,97,118,33,99,128,0,98,66,114,11,200,1,92,255,254,252,225,32,215,215,32,127,64,240,127,80,60,
        50,40,72,136,169,47,95,179,118,130,136,148,140,0,40,80,128,33,193,136,174,7,32,128,144,29,192,8,117,41,59,209,22,66,241,191,255,16,12,244,19,195,63,48,134,240,255,0,9,115,125,93,239,252,130,130,108,168,
        249,44,232,102,0,4,16,19,22,62,51,33,11,255,195,44,4,211,255,25,96,16,33,6,117,24,56,226,25,24,202,139,10,75,226,51,115,66,160,105,13,197,17,0,1,196,68,172,79,255,33,91,206,192,192,128,176,22,17,10,200,
        234,32,161,240,31,24,10,255,24,152,153,153,184,39,244,247,117,107,234,234,105,131,66,1,154,224,193,0,32,128,240,58,0,22,180,255,144,18,13,40,136,33,113,140,36,255,15,17,26,48,12,81,15,145,255,254,251,
        31,131,0,59,171,84,81,73,105,33,208,216,191,200,161,12,16,64,44,248,131,251,63,10,31,198,253,143,38,6,83,7,11,33,228,232,2,123,4,202,226,228,96,151,132,166,49,144,35,126,131,196,0,2,136,5,103,60,51,252,
        71,49,12,213,130,255,168,226,232,150,254,255,15,143,6,80,202,3,133,16,200,198,63,127,193,229,17,39,16,127,135,217,7,16,64,88,67,0,28,143,255,25,225,46,135,249,18,155,133,240,178,4,205,145,8,62,52,186,
        32,234,152,160,118,194,179,35,64,0,177,96,11,123,144,236,95,104,92,162,228,113,36,11,81,125,140,112,56,186,131,96,226,176,172,137,148,229,193,0,32,128,88,112,167,248,255,112,223,48,34,165,110,6,124,190,
        253,143,61,106,192,9,19,73,28,25,0,4,16,206,40,248,251,15,45,104,209,130,21,51,222,145,18,238,127,180,68,8,244,250,95,164,16,66,6,0,1,196,130,45,253,195,12,250,135,53,206,255,195,131,18,213,98,236,81,
        243,31,154,11,144,115,8,50,0,8,32,156,81,0,203,227,12,80,223,98,230,4,68,72,96,38,78,84,11,65,9,250,47,146,3,145,1,64,0,97,117,192,95,112,34,68,138,130,255,176,224,251,143,226,51,6,6,68,29,192,136,20,
        77,200,69,54,35,3,36,49,255,69,77,132,112,0,16,64,44,56,139,94,36,7,96,102,59,164,108,249,31,181,82,98,64,203,174,255,144,234,142,127,88,146,33,64,0,97,205,134,240,120,67,75,76,136,224,198,140,22,6,44,
        142,66,201,41,255,177,231,2,128,0,194,25,5,255,254,161,134,192,127,6,28,229,0,129,242,1,150,56,33,81,138,209,28,96,0,8,32,172,81,0,78,3,104,190,68,182,224,31,146,197,224,56,6,146,140,176,202,135,17,169,
        96,130,40,64,56,0,139,93,0,1,132,61,10,64,248,31,106,156,162,199,55,204,65,255,144,178,38,74,84,252,71,51,239,63,246,68,8,16,64,44,216,74,1,88,217,13,203,191,32,1,80,58,7,133,224,127,6,68,114,6,241,65,
        81,197,8,101,255,71,114,33,92,237,127,228,52,128,233,2,128,0,98,193,149,3,64,117,193,255,127,255,81,75,191,127,168,5,18,136,255,31,45,161,49,32,151,134,72,252,127,12,216,203,98,128,0,98,193,210,144,135,
        248,30,201,242,127,208,252,140,145,27,160,113,206,136,148,197,192,121,159,17,53,184,225,149,17,22,23,0,4,16,11,182,150,237,63,168,207,96,142,248,143,163,72,6,203,253,67,13,61,6,104,14,66,46,17,254,65,
        19,40,182,16,0,8,32,22,108,109,235,255,176,234,24,35,79,255,199,222,30,64,81,135,90,35,194,211,4,142,92,0,16,64,88,29,0,107,7,254,251,247,31,53,78,241,54,207,80,29,135,209,96,249,143,189,46,0,8,32,116,
        7,252,101,102,103,103,228,103,99,96,248,193,198,137,53,248,49,125,204,128,225,227,255,88,18,54,47,176,25,202,205,195,205,6,109,11,194,149,0,4,16,35,204,85,208,254,27,159,128,176,176,142,166,182,142,21,
        48,4,248,129,41,143,13,217,16,70,52,95,147,0,254,0,187,69,95,223,188,122,125,235,206,141,107,7,129,252,247,64,123,193,237,66,128,0,66,118,0,168,189,198,3,196,252,32,135,64,105,54,228,230,19,185,29,100,
        168,175,191,0,241,7,32,254,4,196,159,129,246,254,2,73,2,4,16,11,90,72,125,135,210,63,161,138,153,169,212,75,255,15,117,196,15,40,134,119,215,1,2,12,0,187,0,132,247,216,161,197,124,0,0,0,0,73,69,78,68,
        174,66,96,130,0,0};

    return ImageCache::getFromMemory (fileicon_png, sizeof (fileicon_png));
}

//==============================================================================
static void createRoundedPath (Path& p,
                               const float x, const float y,
                               const float w, const float h,
                               const float cs,
                               const bool curveTopLeft, const bool curveTopRight,
                               const bool curveBottomLeft, const bool curveBottomRight) throw()
{
    const float cs2 = 2.0f * cs;

    if (curveTopLeft)
    {
        p.startNewSubPath (x, y + cs);
        p.addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f);
    }
    else
    {
        p.startNewSubPath (x, y);
    }

    if (curveTopRight)
    {
        p.lineTo (x + w - cs, y);
        p.addArc (x + w - cs2, y, cs2, cs2, 0.0f, float_Pi * 0.5f);
    }
    else
    {
        p.lineTo (x + w, y);
    }

    if (curveBottomRight)
    {
        p.lineTo (x + w, y + h - cs);
        p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, float_Pi * 0.5f, float_Pi);
    }
    else
    {
        p.lineTo (x + w, y + h);
    }

    if (curveBottomLeft)
    {
        p.lineTo (x + cs, y + h);
        p.addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);
    }
    else
    {
        p.lineTo (x, y + h);
    }

    p.closeSubPath();
}

//==============================================================================
void LookAndFeel::drawShinyButtonShape (Graphics& g,
                                        float x, float y, float w, float h,
                                        float maxCornerSize,
                                        const Colour& baseColour,
                                        const float strokeWidth,
                                        const bool flatOnLeft,
                                        const bool flatOnRight,
                                        const bool flatOnTop,
                                        const bool flatOnBottom) throw()
{
    if (w <= strokeWidth * 1.1f || h <= strokeWidth * 1.1f)
        return;

    const float cs = jmin (maxCornerSize, w * 0.5f, h * 0.5f);

    Path outline;
    createRoundedPath (outline, x, y, w, h, cs,
                        ! (flatOnLeft || flatOnTop),
                        ! (flatOnRight || flatOnTop),
                        ! (flatOnLeft || flatOnBottom),
                        ! (flatOnRight || flatOnBottom));

    ColourGradient cg (baseColour, 0.0f, y,
                       baseColour.overlaidWith (Colour (0x070000ff)), 0.0f, y + h,
                       false);

    cg.addColour (0.5, baseColour.overlaidWith (Colour (0x33ffffff)));
    cg.addColour (0.51, baseColour.overlaidWith (Colour (0x110000ff)));

    GradientBrush gb (cg);
    g.setBrush (&gb);
    g.fillPath (outline);

    g.setColour (Colour (0x80000000));
    g.strokePath (outline, PathStrokeType (strokeWidth));
}

//==============================================================================
void LookAndFeel::drawGlassSphere (Graphics& g,
                                   const float x, const float y,
                                   const float diameter,
                                   const Colour& colour,
                                   const float outlineThickness) throw()
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.addEllipse (x, y, diameter, diameter);

    {
        ColourGradient cg (Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColour (0.4, Colours::white.overlaidWith (colour));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    {
        GradientBrush gb (Colours::white, 0, y + diameter * 0.06f,
                          Colours::transparentWhite, 0, y + diameter * 0.3f, false);

        g.setBrush (&gb);
        g.fillEllipse (x + diameter * 0.2f, y + diameter * 0.05f, diameter * 0.6f, diameter * 0.4f);
    }

    {
        ColourGradient cg (Colours::transparentBlack,
                           x + diameter * 0.5f, y + diameter * 0.5f,
                           Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                           x, y + diameter * 0.5f, true);

        cg.addColour (0.7, Colours::transparentBlack);
        cg.addColour (0.8, Colours::black.withAlpha (0.1f * outlineThickness));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.drawEllipse (x, y, diameter, diameter, outlineThickness);
}

//==============================================================================
void LookAndFeel::drawGlassPointer (Graphics& g,
                                    const float x, const float y,
                                    const float diameter,
                                    const Colour& colour, const float outlineThickness,
                                    const int direction) throw()
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

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    {
        ColourGradient cg (Colours::transparentBlack,
                           x + diameter * 0.5f, y + diameter * 0.5f,
                           Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                           x - diameter * 0.2f, y + diameter * 0.5f, true);

        cg.addColour (0.5, Colours::transparentBlack);
        cg.addColour (0.7, Colours::black.withAlpha (0.07f * outlineThickness));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.strokePath (p, PathStrokeType (outlineThickness));
}

//==============================================================================
void LookAndFeel::drawGlassLozenge (Graphics& g,
                                    const float x, const float y,
                                    const float width, const float height,
                                    const Colour& colour,
                                    const float outlineThickness,
                                    const float cornerSize,
                                    const bool flatOnLeft,
                                    const bool flatOnRight,
                                    const bool flatOnTop,
                                    const bool flatOnBottom) throw()
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
    createRoundedPath (outline, x, y, width, height, cs,
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

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (outline);
    }

    ColourGradient cg (Colours::transparentBlack, x + edgeBlurRadius, y + height * 0.5f,
                       colour.darker (0.2f), x, y + height * 0.5f, true);

    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.5f) / edgeBlurRadius), Colours::transparentBlack);
    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.25f) / edgeBlurRadius), colour.darker (0.2f).withMultipliedAlpha (0.3f));

    if (! (flatOnLeft || flatOnTop || flatOnBottom))
    {
        GradientBrush gb (cg);

        g.saveState();
        g.setBrush (&gb);
        g.reduceClipRegion (intX, intY, intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    if (! (flatOnRight || flatOnTop || flatOnBottom))
    {
        cg.x1 = x + width - edgeBlurRadius;
        cg.x2 = x + width;
        GradientBrush gb (cg);

        g.saveState();
        g.setBrush (&gb);
        g.reduceClipRegion (intX + intW - intEdge, intY, 2 + intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    {
        const float leftIndent = flatOnLeft ? 0.0f : cs * 0.4f;
        const float rightIndent = flatOnRight ? 0.0f : cs * 0.4f;

        Path highlight;
        createRoundedPath (highlight,
                           x + leftIndent,
                           y + cs * 0.1f,
                           width - (leftIndent + rightIndent),
                           height * 0.4f, cs * 0.4f,
                           ! (flatOnLeft || flatOnTop),
                           ! (flatOnRight || flatOnTop),
                           ! (flatOnLeft || flatOnBottom),
                           ! (flatOnRight || flatOnBottom));

        GradientBrush gb (colour.brighter (10.0f), 0, y + height * 0.06f,
                          Colours::transparentWhite, 0, y + height * 0.4f, false);

        g.setBrush (&gb);
        g.fillPath (highlight);
    }

    g.setColour (colour.darker().withMultipliedAlpha (1.5f));
    g.strokePath (outline, PathStrokeType (outlineThickness));
}

END_JUCE_NAMESPACE
