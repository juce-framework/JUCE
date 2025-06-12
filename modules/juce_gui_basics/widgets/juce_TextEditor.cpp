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

namespace juce
{

//==============================================================================
struct TextEditor::InsertAction final : public UndoableAction
{
    InsertAction (TextEditor& ed, const String& newText, int insertPos,
                  const Font& newFont, Colour newColour, int oldCaret, int newCaret)
        : owner (ed),
          text (newText),
          insertIndex (insertPos),
          oldCaretPos (oldCaret),
          newCaretPos (newCaret),
          font (newFont),
          colour (newColour)
    {
    }

    bool perform() override
    {
        owner.insert (text, insertIndex, font, colour, nullptr, newCaretPos);
        return true;
    }

    bool undo() override
    {
        owner.remove ({ insertIndex, insertIndex + text.length() }, nullptr, oldCaretPos);
        return true;
    }

    int getSizeInUnits() override
    {
        return text.length() + 16;
    }

private:
    TextEditor& owner;
    const String text;
    const int insertIndex, oldCaretPos, newCaretPos;
    const Font font;
    const Colour colour;

    JUCE_DECLARE_NON_COPYABLE (InsertAction)
};

//==============================================================================
struct TextEditor::RemoveAction final : public UndoableAction
{
    RemoveAction (TextEditor& ed, Range<int> rangeToRemove, int oldCaret, int newCaret)
        : owner (ed),
          range (rangeToRemove),
          oldCaretPos (oldCaret),
          newCaretPos (newCaret)
    {
    }

    bool perform() override
    {
        owner.remove (range, nullptr, newCaretPos, &removedText);
        return true;
    }

    bool undo() override
    {
        owner.reinsert (removedText);
        owner.moveCaretTo (oldCaretPos, false);
        return true;
    }

    int getSizeInUnits() override
    {
        return std::accumulate (removedText.texts.begin(),
                                removedText.texts.end(),
                                0,
                                [] (auto sum, auto& value)
                                {
                                    return sum + (int) value.getNumBytesAsUTF8();
                                });
    }

private:
    TextEditor& owner;
    const Range<int> range;
    const int oldCaretPos, newCaretPos;
    TextEditorStorageChunks removedText;

    JUCE_DECLARE_NON_COPYABLE (RemoveAction)
};

//==============================================================================
struct TextEditor::TextHolderComponent final : public Component,
                                               public Timer,
                                               public Value::Listener
{
    TextHolderComponent (TextEditor& ed)  : owner (ed)
    {
        setWantsKeyboardFocus (false);
        setInterceptsMouseClicks (false, true);
        setMouseCursor (MouseCursor::ParentCursor);

        owner.getTextValue().addListener (this);
    }

    ~TextHolderComponent() override
    {
        owner.getTextValue().removeListener (this);
    }

    void paint (Graphics& g) override
    {
        owner.drawContent (g);
    }

    void restartTimer()
    {
        startTimer (350);
    }

    void timerCallback() override
    {
        owner.timerCallbackInt();
    }

    void valueChanged (Value&) override
    {
        owner.textWasChangedByValue();
    }

    TextEditor& owner;

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextHolderComponent)
};

//==============================================================================
struct TextEditor::TextEditorViewport final : public Viewport
{
    TextEditorViewport (TextEditor& ed) : owner (ed) {}

    void visibleAreaChanged (const Rectangle<int>&) override
    {
        if (! reentrant) // it's rare, but possible to get into a feedback loop as the viewport's scrollbars
                         // appear and disappear, causing the wrap width to change.
        {
            auto wordWrapWidth = owner.getWordWrapWidth();
            owner.updateBaseShapedTextOptions();

            if (wordWrapWidth != lastWordWrapWidth)
            {
                lastWordWrapWidth = wordWrapWidth;

                ScopedValueSetter<bool> svs (reentrant, true);
                owner.checkLayout();
            }
        }
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    TextEditor& owner;
    int lastWordWrapWidth = 0;
    bool reentrant = false;

    JUCE_DECLARE_NON_COPYABLE (TextEditorViewport)
};

//==============================================================================
namespace TextEditorDefs
{
    const int textChangeMessageId = 0x10003001;
    const int returnKeyMessageId  = 0x10003002;
    const int escapeKeyMessageId  = 0x10003003;
    const int focusLossMessageId  = 0x10003004;

    const int maxActionsPerTransaction = 100;

    static int getCharacterCategory (juce_wchar character) noexcept
    {
        return CharacterFunctions::isLetterOrDigit (character)
                    ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
    }
}

//==============================================================================
TextEditor::TextEditor (const String& name, juce_wchar passwordChar)
    : Component (name),
      passwordCharacter (passwordChar),
      textStorage { std::make_unique<TextEditorStorage>() },
      caretState { this }
{
    setMouseCursor (MouseCursor::IBeamCursor);

    viewport.reset (new TextEditorViewport (*this));
    addAndMakeVisible (viewport.get());
    viewport->setViewedComponent (textHolder = new TextHolderComponent (*this));
    viewport->setWantsKeyboardFocus (false);
    viewport->setScrollBarsShown (false, false);

    setWantsKeyboardFocus (true);
    recreateCaret();
}

TextEditor::~TextEditor()
{
    if (auto* peer = getPeer())
        peer->refreshTextInputTarget();

    textValue.removeListener (textHolder);
    textValue.referTo (Value());

    viewport.reset();
    textHolder = nullptr;
}

//==============================================================================
void TextEditor::newTransaction()
{
    lastTransactionTime = Time::getApproximateMillisecondCounter();
    undoManager.beginNewTransaction();
}

bool TextEditor::undoOrRedo (const bool shouldUndo)
{
    if (! isReadOnly())
    {
        newTransaction();

        if (shouldUndo ? undoManager.undo()
                       : undoManager.redo())
        {
            repaint();
            textChanged();
            scrollToMakeSureCursorIsVisible();

            return true;
        }
    }

    return false;
}

bool TextEditor::undo()     { return undoOrRedo (true); }
bool TextEditor::redo()     { return undoOrRedo (false); }

//==============================================================================
void TextEditor::setMultiLine (const bool shouldBeMultiLine,
                               const bool shouldWordWrap)
{
    if (multiline != shouldBeMultiLine
         || wordWrap != (shouldWordWrap && shouldBeMultiLine))
    {
        multiline = shouldBeMultiLine;
        wordWrap = shouldWordWrap && shouldBeMultiLine;
        updateBaseShapedTextOptions();

        checkLayout();

        viewport->setViewPosition (0, 0);
        resized();
        scrollToMakeSureCursorIsVisible();
    }
}

bool TextEditor::isMultiLine() const
{
    return multiline;
}

void TextEditor::setScrollbarsShown (bool shown)
{
    if (scrollbarVisible != shown)
    {
        scrollbarVisible = shown;
        checkLayout();
    }
}

void TextEditor::setReadOnly (bool shouldBeReadOnly)
{
    if (readOnly != shouldBeReadOnly)
    {
        readOnly = shouldBeReadOnly;
        enablementChanged();
        invalidateAccessibilityHandler();

        if (auto* peer = getPeer())
            peer->refreshTextInputTarget();
    }
}

void TextEditor::setClicksOutsideDismissVirtualKeyboard (bool newValue)
{
    clicksOutsideDismissVirtualKeyboard = newValue;
}

bool TextEditor::isReadOnly() const noexcept
{
    return readOnly || ! isEnabled();
}

bool TextEditor::isTextInputActive() const
{
    return ! isReadOnly() && (! clicksOutsideDismissVirtualKeyboard || globalMouseListener.lastMouseDownInEditor());
}

void TextEditor::setReturnKeyStartsNewLine (bool shouldStartNewLine)
{
    returnKeyStartsNewLine = shouldStartNewLine;
}

void TextEditor::setTabKeyUsedAsCharacter (bool shouldTabKeyBeUsed)
{
    tabKeyUsed = shouldTabKeyBeUsed;
}

void TextEditor::setPopupMenuEnabled (bool b)
{
    popupMenuEnabled = b;
}

void TextEditor::setSelectAllWhenFocused (bool b)
{
    selectAllTextWhenFocused = b;
}

void TextEditor::setJustification (Justification j)
{
    if (justification != j)
    {
        justification = j;

        resized();
        repaint();
    }
}

void TextEditor::setLineSpacing (float newLineSpacing) noexcept
{
    lineSpacing = jmax (1.0f, newLineSpacing);
    updateBaseShapedTextOptions();
}

//==============================================================================
void TextEditor::setFont (const Font& newFont)
{
    currentFont = newFont;
    scrollToMakeSureCursorIsVisible();
}

void TextEditor::applyFontToAllText (const Font& newFont, bool changeCurrentFont)
{
    if (changeCurrentFont)
        currentFont = newFont;

    textStorage->setFontForAllText (newFont);

    const auto overallColour = findColour (textColourId);
    textStorage->setColourForAllText (overallColour);

    checkLayout();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

void TextEditor::applyColourToAllText (const Colour& newColour, bool changeCurrentTextColour)
{
    textStorage->setColourForAllText (newColour);

    if (changeCurrentTextColour)
        setColour (TextEditor::textColourId, newColour);
    else
        repaint();
}

void TextEditor::lookAndFeelChanged()
{
    caret.reset();
    recreateCaret();
}

void TextEditor::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

void TextEditor::enablementChanged()
{
    recreateCaret();
    repaint();
}

void TextEditor::setCaretVisible (bool shouldCaretBeVisible)
{
    if (caretVisible != shouldCaretBeVisible)
    {
        caretVisible = shouldCaretBeVisible;
        recreateCaret();
    }
}

void TextEditor::recreateCaret()
{
    if (isCaretVisible())
    {
        if (caret == nullptr)
        {
            caret.reset (getLookAndFeel().createCaretComponent (this));
            textHolder->addChildComponent (caret.get());
            updateCaretPosition();
        }
    }
    else
    {
        caret.reset();
    }
}

void TextEditor::updateCaretPosition()
{
    if (caret != nullptr
        && getWidth() > 0 && getHeight() > 0)
    {
        caret->setCaretPosition (getCaretRectangle().translated (leftIndent,
                                                                 topIndent + roundToInt (getYOffset())) - getTextOffset());

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

TextEditor::LengthAndCharacterRestriction::LengthAndCharacterRestriction (int maxLen, const String& chars)
    : allowedCharacters (chars), maxLength (maxLen)
{
}

String TextEditor::LengthAndCharacterRestriction::filterNewText (TextEditor& ed, const String& newInput)
{
    String t (newInput);

    if (allowedCharacters.isNotEmpty())
        t = t.retainCharacters (allowedCharacters);

    if (maxLength > 0)
        t = t.substring (0, maxLength - (ed.getTotalNumChars() - ed.getHighlightedRegion().getLength()));

    return t;
}

void TextEditor::setInputFilter (InputFilter* newFilter, bool takeOwnership)
{
    inputFilter.set (newFilter, takeOwnership);
}

void TextEditor::setInputRestrictions (int maxLen, const String& chars)
{
    setInputFilter (new LengthAndCharacterRestriction (maxLen, chars), true);
}

void TextEditor::setTextToShowWhenEmpty (const String& text, Colour colourToUse)
{
    textToShowWhenEmpty = text;
    colourForTextWhenEmpty = colourToUse;
}

void TextEditor::setPasswordCharacter (juce_wchar newPasswordCharacter)
{
    if (passwordCharacter != newPasswordCharacter)
    {
        passwordCharacter = newPasswordCharacter;
        applyFontToAllText (currentFont);
        updateBaseShapedTextOptions();
    }
}

void TextEditor::setScrollBarThickness (int newThicknessPixels)
{
    viewport->setScrollBarThickness (newThicknessPixels);
}

//==============================================================================
void TextEditor::clear()
{
    clearInternal (nullptr);
    checkLayout();
    undoManager.clearUndoHistory();
    repaint();
}

void TextEditor::setText (const String& newText, bool sendTextChangeMessage)
{
    auto newLength = newText.length();

    if (newLength != getTotalNumChars() || getText() != newText)
    {
        if (! sendTextChangeMessage)
            textValue.removeListener (textHolder);

        textValue = newText;

        auto oldCursorPos = caretState.getPosition();
        auto cursorWasAtEnd = oldCursorPos >= getTotalNumChars();

        clearInternal (nullptr);
        insert (newText, 0, currentFont, findColour (textColourId), nullptr, caretState.getPosition());

        // if you're adding text with line-feeds to a single-line text editor, it
        // ain't gonna look right!
        jassert (multiline || ! newText.containsAnyOf ("\r\n"));

        if (cursorWasAtEnd && ! isMultiLine())
            oldCursorPos = getTotalNumChars();

        moveCaretTo (oldCursorPos, false);

        if (sendTextChangeMessage)
            textChanged();
        else
            textValue.addListener (textHolder);

        checkLayout();
        scrollToMakeSureCursorIsVisible();
        undoManager.clearUndoHistory();

        repaint();
    }
}

//==============================================================================
void TextEditor::updateValueFromText()
{
    if (valueTextNeedsUpdating)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }
}

Value& TextEditor::getTextValue()
{
    updateValueFromText();
    return textValue;
}

void TextEditor::textWasChangedByValue()
{
    if (textValue.getValueSource().getReferenceCount() > 1)
        setText (textValue.getValue());
}

//==============================================================================
void TextEditor::textChanged()
{
    checkLayout();

    if (listeners.size() != 0 || onTextChange != nullptr)
        postCommandMessage (TextEditorDefs::textChangeMessageId);

    if (textValue.getValueSource().getReferenceCount() > 1)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
}

void TextEditor::setSelection (Range<int> newSelection) noexcept
{
    if (newSelection != selection)
    {
        selection = newSelection;

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

void TextEditor::returnPressed()    { postCommandMessage (TextEditorDefs::returnKeyMessageId); }
void TextEditor::escapePressed()    { postCommandMessage (TextEditorDefs::escapeKeyMessageId); }

void TextEditor::addListener (Listener* l)      { listeners.add (l); }
void TextEditor::removeListener (Listener* l)   { listeners.remove (l); }

//==============================================================================
void TextEditor::timerCallbackInt()
{
    checkFocus();

    auto now = Time::getApproximateMillisecondCounter();

    if (now > lastTransactionTime + 200)
        newTransaction();
}

void TextEditor::checkFocus()
{
    if (! wasFocused && hasKeyboardFocus (false) && ! isCurrentlyBlockedByAnotherModalComponent())
        wasFocused = true;
}

void TextEditor::repaintText (Range<int> range)
{
    if (! range.isEmpty())
    {
        if (range.getEnd() >= getTotalNumChars())
        {
            textHolder->repaint();
            return;
        }

        const auto [anchor, lh] = getCursorEdge (caretState.withPosition (range.getStart())
                                                           .withPreferredEdge (Edge::trailing));

        auto y1 = std::trunc (anchor.y);
        int y2 = 0;

        if (range.getEnd() >= getTotalNumChars())
        {
            y2 = textHolder->getHeight();
        }
        else
        {
            const auto info = getCursorEdge (caretState.withPosition (range.getEnd())
                                                       .withPreferredEdge (Edge::leading));

            y2 = (int) (info.anchor.y + lh * 2.0f);
        }

        const auto offset = getYOffset();

        textHolder->repaint (0,
                             (int) std::floor (y1 + offset),
                             textHolder->getWidth(),
                             (int) std::ceil ((float) y2 - y1 + offset));
    }
}

//==============================================================================
void TextEditor::moveCaret (const int newCaretPos)
{
    const auto clamped = std::clamp (newCaretPos, 0, getTotalNumChars());

    if (clamped == getCaretPosition())
        return;

    caretState.setPosition (clamped);

    if (hasKeyboardFocus (false))
        textHolder->restartTimer();

    scrollToMakeSureCursorIsVisible();
    updateCaretPosition();

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
}

int TextEditor::getCaretPosition() const
{
    return caretState.getPosition();
}

void TextEditor::setCaretPosition (const int newIndex)
{
    moveCaretTo (newIndex, false);
}

void TextEditor::moveCaretToEnd()
{
    setCaretPosition (std::numeric_limits<int>::max());
}

void TextEditor::scrollEditorToPositionCaret (const int desiredCaretX,
                                              const int desiredCaretY)

{
    updateCaretPosition();
    auto caretRect = getCaretRectangle().translated (leftIndent, topIndent);

    auto vx = caretRect.getX() - desiredCaretX;
    auto vy = caretRect.getY() - desiredCaretY;

    if (desiredCaretX < jmax (1, proportionOfWidth (0.05f)))
        vx += desiredCaretX - proportionOfWidth (0.2f);
    else if (desiredCaretX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        vx += desiredCaretX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();

    vx = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), vx);

    if (! isMultiLine())
    {
        vy = viewport->getViewPositionY();
    }
    else
    {
        vy = jlimit (0, jmax (0, textHolder->getHeight() - viewport->getMaximumVisibleHeight()), vy);

        if (desiredCaretY < 0)
            vy = jmax (0, desiredCaretY + vy);
        else if (desiredCaretY > jmax (0, viewport->getMaximumVisibleHeight() - caretRect.getHeight()))
            vy += desiredCaretY + 2 + caretRect.getHeight() - viewport->getMaximumVisibleHeight();
    }

    viewport->setViewPosition (vx, vy);
}

Rectangle<int> TextEditor::getCaretRectangleForCharIndex (int index) const
{
    const auto [anchor, cursorHeight] = getCursorEdge (caretState.withPosition (index));
    Rectangle<float> caretRectangle { anchor.x, anchor.y, 2.0f, cursorHeight };
    return caretRectangle.getSmallestIntegerContainer() + getTextOffset();
}

Point<int> TextEditor::getTextOffset() const
{
    return { getLeftIndent() + borderSize.getLeft() - viewport->getViewPositionX(),
             roundToInt ((float) getTopIndent() + (float) borderSize.getTop() + getYOffset()) - viewport->getViewPositionY() };
}

template <typename T>
detail::RangedValues<T> TextEditor::getGlyphRanges (const detail::RangedValues<T>& textRanges) const
{
    detail::RangedValues<T> glyphRanges;
    std::vector<Range<int64>> glyphRangesStorage;

    detail::Ranges::Operations ops;

    for (const auto [range, value, paragraph] : makeIntersectingRangedValues (&textRanges,
                                                                              textStorage.get()))
    {
        paragraph->getShapedText().getGlyphRanges (range - paragraph->getRange().getStart(),
                                                   glyphRangesStorage);

        for (const auto& glyphRange : glyphRangesStorage)
        {
            glyphRanges.set (glyphRange + paragraph->getStartingGlyph(), value, ops);
            ops.clear();
        }
    }

    return glyphRanges;
}

bool TextEditor::isTextStorageHeightGreaterEqualThan (float value) const
{
    float height = 0.0;

    for (auto paragraphItem : *textStorage)
    {
        height += paragraphItem.value->getHeight();

        if (height >= value)
            return true;
    }

    return false;
}

float TextEditor::getTextStorageHeight() const
{
    const auto textHeight = std::accumulate (textStorage->begin(), textStorage->end(), 0.0f, [&] (auto acc, auto item)
    {
        return acc + item.value->getHeight();
    });

    if (! textStorage->isEmpty() && ! textStorage->back().value->getText().endsWith ("\n"))
        return textHeight;

    return textHeight + getLineSpacing() * textStorage->getLastFont().value_or (currentFont).getHeight();
}

float TextEditor::getYOffset() const
{
    const auto bottomY = getMaximumTextHeight();

    const auto juce7LineSpacingOffset = std::invoke ([&]
    {
         if (approximatelyEqual (lineSpacing, 1.0f) || textStorage->isEmpty())
             return 0.0f;

         const auto& lineMetrics = textStorage->front().value->getShapedText().getLineMetricsForGlyphRange();

         if (lineMetrics.isEmpty())
             return 0.0f;

         const auto& line = lineMetrics.front().value;

         jassert (lineSpacing >= 1.0f);
         return line.maxAscent * (1.0f / lineSpacing - 1.0f);
    });

    if (justification.testFlags (Justification::top) || isTextStorageHeightGreaterEqualThan ((float) bottomY))
        return juce7LineSpacingOffset;

    auto bottom = jmax (0.0f, (float) bottomY - getTextStorageHeight());

    if (justification.testFlags (Justification::bottom))
        return bottom;

    return bottom * 0.5f;
}

Range<int64> TextEditor::getLineRangeForIndex (int index)
{
    jassert (index >= 0);

    const auto indexInText = (int64) index;

    if (textStorage->isEmpty())
        return { indexInText, indexInText };

    if (const auto paragraph = textStorage->getParagraphContainingCodepointIndex (indexInText))
    {
        const auto& shapedText = paragraph->value->getShapedText();
        auto r = *shapedText.getLineTextRanges().find (indexInText - paragraph->range.getStart())
                 + paragraph->range.getStart();

        if (r.getEnd() != paragraph->range.getEnd())
            return r;

        constexpr juce_wchar cr = 0x0d;
        constexpr juce_wchar lf = 0x0a;

        const auto startIt = shapedText.getText().begin();
        auto endIt = shapedText.getText().end();

        for (int i = 0; i < 2; ++i)
        {
            if (endIt == startIt)
                break;

            auto newEnd = endIt - 1;

            if (*newEnd != cr && *newEnd != lf)
                break;

            r.setEnd (std::max (r.getStart(), r.getEnd() - 1));
            endIt = newEnd;
        }

        return r;
    }

    const auto& lastParagraphItem = textStorage->back();

    if (lastParagraphItem.value->getText().endsWith ("\n"))
        return Range<int64>::withStartAndLength (lastParagraphItem.range.getEnd(), 0);

    return lastParagraphItem.value->getShapedText().getLineTextRanges().getRanges().back()
               + lastParagraphItem.range.getStart();
}

TextEditor::CaretEdge TextEditor::getTextSelectionEdge (int index, Edge edge) const
{
    jassert (0 <= index && index < getTotalNumChars());
    const auto textRange = Range<int64>::withStartAndLength ((int64) index, 1);

    const auto paragraphIt = std::find_if (textStorage->begin(),
                                           textStorage->end(),
                                           [&] (const auto& p)
                                           {
                                               return p.range.contains (textRange.getStart());
                                           });

    jassert (paragraphIt != textStorage->end());

    auto& paragraph = paragraphIt->value;
    const auto& shapedText = paragraph->getShapedText();

    const auto glyphRange = std::invoke ([&]() -> Range<int64>
    {
        std::vector<Range<int64>> g;
        shapedText.getGlyphRanges (textRange - paragraph->getRange().getStart(), g);

        if (g.empty())
            return {};

        return g.front();
    });

    if (glyphRange.isEmpty())
        return getDefaultCursorEdge();

    const auto glyphsBounds = shapedText.getGlyphsBounds (glyphRange).getRectangle (0);
    const auto ltr = shapedText.isLtr (glyphRange.getStart());

    const auto anchorX = std::invoke ([&]
    {
        if (edge == Edge::leading)
            return ltr ? glyphsBounds.getX() : glyphsBounds.getRight();

        return ltr ? glyphsBounds.getRight() : glyphsBounds.getX();
    });

    const auto lineMetrics = shapedText.getLineMetricsForGlyphRange().find (glyphRange.getStart())->value;
    const auto anchorY = lineMetrics.anchor.getY();

    return { { anchorX, anchorY -lineMetrics.maxAscent + paragraph->getTop() },
             lineMetrics.maxAscent + lineMetrics.maxDescent };
}

void TextEditor::updateBaseShapedTextOptions()
{
    auto options = detail::ShapedText::Options{}.withTrailingWhitespacesShouldFit (true)
                                                .withJustification (getJustificationType().getOnlyHorizontalFlags())
                                                .withDrawLinesInFull()
                                                .withLeading (lineSpacing);

    if (wordWrap)
        options = options.withWordWrapWidth ((float) getMaximumTextWidth())
                         .withAllowBreakingInsideWord();
    else
        options = options.withAlignmentWidth ((float) getMaximumTextWidth());

    textStorage->setBaseShapedTextOptions (options, passwordCharacter);
}

static auto asInt64Range (Range<int> r)
{
    return Range<int64> { (int64) r.getStart(), (int64) r.getEnd() };
}

RectangleList<int> TextEditor::getTextBounds (Range<int> textRange) const
{
    RectangleList<int> boundingBox;

    detail::RangedValues<int> mask;
    detail::Ranges::Operations ops;
    mask.set (asInt64Range (textRange), 0, ops);

    for (auto [_1, paragraph, _2] : makeIntersectingRangedValues (textStorage.get(), &mask))
    {
        ignoreUnused (_1, _2);
        auto& shapedText = paragraph->getShapedText();

        std::vector<Range<int64>> glyphRanges;
        shapedText.getGlyphRanges (asInt64Range (textRange) - paragraph->getRange().getStart(),
                                   glyphRanges);

        for (const auto& glyphRange : glyphRanges)
            for (const auto& bounds : shapedText.getGlyphsBounds (glyphRange))
                boundingBox.add (bounds.withY (bounds.getY() + paragraph->getTop()).getSmallestIntegerContainer());
    }

    boundingBox.offsetAll (getTextOffset());
    return boundingBox;
}

//==============================================================================
// Extra space for the cursor at the right-hand-edge
constexpr int rightEdgeSpace = 2;

int TextEditor::getWordWrapWidth() const
{
    return wordWrap ? getMaximumTextWidth()
                    : std::numeric_limits<int>::max();
}

int TextEditor::getMaximumTextWidth() const
{
    return jmax (1, viewport->getMaximumVisibleWidth() - leftIndent - rightEdgeSpace);
}

int TextEditor::getMaximumTextHeight() const
{
    return jmax (1, viewport->getMaximumVisibleHeight() - topIndent);
}

void TextEditor::checkLayout()
{
    if (getWordWrapWidth() > 0)
    {
        const auto textBottom = topIndent
                                + (int) std::ceil (getYOffset() + getTextStorageHeight());

        const auto maxTextWidth = std::accumulate (textStorage->begin(), textStorage->end(), 0.0f, [&](auto pMax, auto paragraph)
                                                   {
                                                       auto& shapedText = paragraph.value->getShapedText();

                                                       const auto paragraphWidth = std::accumulate (shapedText.getLineMetricsForGlyphRange().begin(),
                                                                                                    shapedText.getLineMetricsForGlyphRange().end(),
                                                                                                    0.0f,
                                                                                                    [&] (auto lMax, auto line)
                                                                                                    {
                                                                                                        return std::max (lMax,
                                                                                                                         line.value.effectiveLineLength);
                                                                                                    });

                                                       return std::max (pMax, paragraphWidth);
                                                   });

        const auto textRight = std::max (viewport->getMaximumVisibleWidth(),
                                         (int) std::ceil (maxTextWidth) + leftIndent + rightEdgeSpace);

        textHolder->setSize (textRight, std::max (textBottom, viewport->getHeight()));
        viewport->setScrollBarsShown (scrollbarVisible && multiline && textBottom > viewport->getMaximumVisibleHeight(),
                                      scrollbarVisible && multiline && ! wordWrap && textRight > viewport->getMaximumVisibleWidth());
    }
}

int TextEditor::getTextWidth() const    { return textHolder->getWidth(); }
int TextEditor::getTextHeight() const   { return textHolder->getHeight(); }

void TextEditor::setIndents (int newLeftIndent, int newTopIndent)
{
    if (leftIndent != newLeftIndent || topIndent != newTopIndent)
    {
        leftIndent = newLeftIndent;
        topIndent  = newTopIndent;

        resized();
        repaint();
    }
}

void TextEditor::setBorder (BorderSize<int> border)
{
    borderSize = border;
    resized();
}

BorderSize<int> TextEditor::getBorder() const
{
    return borderSize;
}

void TextEditor::setScrollToShowCursor (const bool shouldScrollToShowCursor)
{
    keepCaretOnScreen = shouldScrollToShowCursor;
}

void TextEditor::scrollToMakeSureCursorIsVisible()
{
    updateCaretPosition();

    if (keepCaretOnScreen)
    {
        auto viewPos = viewport->getViewPosition();
        auto caretRect = getCaretRectangle().translated (leftIndent, topIndent) - getTextOffset();
        auto relativeCursor = caretRect.getPosition() - viewPos;

        if (relativeCursor.x < jmax (1, proportionOfWidth (0.05f)))
        {
            viewPos.x += relativeCursor.x - proportionOfWidth (0.2f);
        }
        else if (relativeCursor.x > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        {
            viewPos.x += relativeCursor.x + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
        }

        viewPos.x = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), viewPos.x);

        if (! isMultiLine())
        {
            viewPos.y = (getHeight() - textHolder->getHeight() - topIndent) / -2;
        }
        else if (relativeCursor.y < 0)
        {
            viewPos.y = jmax (0, relativeCursor.y + viewPos.y);
        }
        else if (relativeCursor.y > jmax (0, viewport->getMaximumVisibleHeight() - caretRect.getHeight()))
        {
            viewPos.y += relativeCursor.y + 2 + caretRect.getHeight() - viewport->getMaximumVisibleHeight();
        }

        viewport->setViewPosition (viewPos);
    }
}

void TextEditor::moveCaretTo (const int newPosition, const bool isSelecting)
{
    if (isSelecting)
    {
        moveCaret (newPosition);

        auto oldSelection = selection;

        if (dragType == notDragging)
        {
            if (std::abs (getCaretPosition() - selection.getStart()) < std::abs (getCaretPosition() - selection.getEnd()))
                dragType = draggingSelectionStart;
            else
                dragType = draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            if (getCaretPosition() >= selection.getEnd())
                dragType = draggingSelectionEnd;

            setSelection (Range<int>::between (getCaretPosition(), selection.getEnd()));
        }
        else
        {
            if (getCaretPosition() < selection.getStart())
                dragType = draggingSelectionStart;

            setSelection (Range<int>::between (getCaretPosition(), selection.getStart()));
        }

        repaintText (selection.getUnionWith (oldSelection));
    }
    else
    {
        dragType = notDragging;

        repaintText (selection);

        moveCaret (newPosition);
        setSelection (Range<int>::emptyRange (getCaretPosition()));
    }
}

int TextEditor::getTextIndexAt (const int x, const int y) const
{
    const auto offset = getTextOffset();

    return indexAtPosition ((float) (x - offset.x),
                            (float) (y - offset.y));
}

int TextEditor::getTextIndexAt (const Point<int> pt) const
{
    return getTextIndexAt (pt.x, pt.y);
}

int TextEditor::getCharIndexForPoint (const Point<int> point) const
{
    return getTextIndexAt (isMultiLine() ? point : getTextBounds ({ 0, getTotalNumChars() }).getBounds().getConstrainedPoint (point));
}

void TextEditor::insertTextAtCaret (const String& t)
{
    const auto filtered = inputFilter != nullptr ? inputFilter->filterNewText (*this, t) : t;
    const auto newText = isMultiLine() ? filtered.replace ("\r\n", "\n")
                                       : filtered.replaceCharacters ("\r\n", "  ");
    const auto insertIndex = selection.getStart();
    const auto newCaretPos = insertIndex + newText.length();

    remove (selection, getUndoManager(),
            newText.isNotEmpty() ? newCaretPos - 1 : newCaretPos);

    insert (newText, insertIndex, currentFont, findColour (textColourId),
            getUndoManager(), newCaretPos);

    textChanged();
}

void TextEditor::setHighlightedRegion (const Range<int>& newSelection)
{
    if (newSelection == getHighlightedRegion())
        return;

    const auto cursorAtStart = newSelection.getEnd() == getHighlightedRegion().getStart()
                            || newSelection.getEnd() == getHighlightedRegion().getEnd();
    moveCaretTo (cursorAtStart ? newSelection.getEnd() : newSelection.getStart(), false);
    moveCaretTo (cursorAtStart ? newSelection.getStart() : newSelection.getEnd(), true);
}

//==============================================================================
void TextEditor::copy()
{
    if (passwordCharacter == 0)
    {
        auto selectedText = getHighlightedText();

        if (selectedText.isNotEmpty())
            SystemClipboard::copyTextToClipboard (selectedText);
    }
}

void TextEditor::paste()
{
    if (! isReadOnly())
    {
        auto clip = SystemClipboard::getTextFromClipboard();

        if (clip.isNotEmpty())
            insertTextAtCaret (clip);
    }
}

void TextEditor::cut()
{
    if (! isReadOnly())
    {
        moveCaret (selection.getEnd());
        insertTextAtCaret (String());
    }
}

static void drawUnderline (Graphics& g,
                           Span<const detail::ShapedGlyph> glyphs,
                           Span<const Point<float>> positions,
                           const Font& font,
                           const AffineTransform& transform,
                           bool underlineWhitespace)
{
    const auto lineThickness = font.getDescent() * 0.3f;

    const auto getLeft = [&] (const auto& iter)
    {
        return *(positions.begin() + std::distance (glyphs.begin(), iter));
    };

    const auto getRight = [&] (const auto& iter)
    {
        if (iter == glyphs.end())
            return positions.back() + glyphs.back().advance;

        const auto p = *(positions.begin() + std::distance (glyphs.begin(), iter));
        return p + iter->advance;
    };

    for (auto it = glyphs.begin(), end = glyphs.end(); it != end;)
    {
        const auto adjacent = std::adjacent_find (it,
                                                  end,
                                                  [] (const auto& a, const auto& b)
                                                  {
                                                      return a.isWhitespace() != b.isWhitespace();
                                                  });

        if (! it->isWhitespace() || underlineWhitespace)
        {
            const auto left = getLeft (it);
            const auto right = getRight (adjacent);

            Path p;
            p.addRectangle (left.x, left.y + lineThickness * 2.0f, right.x - left.x, lineThickness);
            g.fillPath (p, transform);
        }

        it = adjacent + (adjacent == end ? 0 : 1);;
    }
}

struct UseClip
{
    bool clipAtBegin = false;
    bool clipAtEnd = false;
};

// Glyphs can reach beyond the anchor - advance defined rectangle. We shouldn't use a clip unless
// we need to partially paint a ligature.
static UseClip getDrawableGlyphs (Span<const detail::ShapedGlyph> glyphs,
                                  Span<const Point<float>> positions,
                                  std::vector<uint16_t>& glyphIdsOut,
                                  std::vector<Point<float>>& positionsOut)
{
    jassert (! glyphs.empty() && glyphs.size() == positions.size());

    glyphIdsOut.clear();
    positionsOut.clear();

    UseClip useClip;

    const auto& firstGlyph = glyphs.front();

    if (firstGlyph.isPlaceholderForLigature())
    {
        useClip.clipAtBegin = true;
        glyphIdsOut.push_back ((uint16_t) firstGlyph.glyphId);
        positionsOut.push_back (positions[0] - (float) firstGlyph.getDistanceFromLigature() * firstGlyph.advance);
    }

    int remainingLigaturePlaceholders = 0;

    for (const auto [index, glyph] : enumerate (glyphs, size_t{}))
    {
        if (glyph.isLigature())
            remainingLigaturePlaceholders += glyph.getNumTrailingLigaturePlaceholders();
        else
            remainingLigaturePlaceholders = std::max (0, remainingLigaturePlaceholders - 1);

        if (! glyph.isPlaceholderForLigature())
        {
            glyphIdsOut.push_back ((uint16_t) glyph.glyphId);
            positionsOut.push_back (positions[index]);
        }
   }

   useClip.clipAtEnd = remainingLigaturePlaceholders > 0;
   return useClip;
}

//==============================================================================
void TextEditor::drawContent (Graphics& g)
{
    using namespace detail;

    g.setOrigin (leftIndent, topIndent);
    float yOffset = getYOffset();

    Graphics::ScopedSaveState ss (g);

    detail::Ranges::Operations ops;

    const auto glyphColours = getGlyphRanges (textStorage->getColours());

    const auto selectedTextRanges = std::invoke ([&]
    {
        detail::RangedValues<int8_t> rv;
        rv.set (asInt64Range (selection), 1, ops);
        return getGlyphRanges (rv);
    });

    const auto textSelectionMask = std::invoke ([&]
    {
        ops.clear();

        detail::RangedValues<int8_t> rv;
        rv.set ({ 0, textStorage->getTotalNumGlyphs() }, 0, ops);

        for (const auto item : selectedTextRanges)
            rv.set (item.range, item.value, ops);

        return rv;
    });

    const auto underlining = std::invoke ([&]
    {
        ops.clear();

        detail::RangedValues<int> rv;
        rv.set ({ 0, textStorage->getTotalNumChars() }, 0, ops);

        for (const auto& underlined : underlinedSections)
            rv.set (asInt64Range (underlined), 1, ops);

        return getGlyphRanges (rv);
    });

    const auto drawSelection = [&] (Span<const detail::ShapedGlyph> glyphs,
                                    Span<const Point<float>> positions,
                                    Font font,
                                    Range<int64>,
                                    LineMetrics,
                                    int)
    {
        g.setColour (findColour (highlightColourId).withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));
        g.fillRect ({ positions.front().translated (0.0f, -font.getAscent() + yOffset),
                      positions.back().translated (glyphs.back().advance.getX(), font.getDescent() + yOffset) });
    };

    std::vector<uint16_t> glyphIds;
    std::vector<Point<float>> positionsForGlyphIds;

    const auto drawGlyphRuns = [&] (Span<const detail::ShapedGlyph> glyphs,
                                    Span<const Point<float>> positions,
                                    Font font,
                                    Range<int64>,
                                    LineMetrics,
                                    Colour colour,
                                    int isSelected,
                                    int hasTemporaryUnderlining)
    {
        auto& context = g.getInternalContext();

        if (context.getFont() != font)
            context.setFont (font);

        const auto transform = AffineTransform::translation (0.0f, yOffset);

        g.setColour (isSelected ? findColour (highlightedTextColourId) : colour);

        glyphIds.clear();
        positionsForGlyphIds.clear();

        const auto useClip = getDrawableGlyphs (glyphs, positions, glyphIds, positionsForGlyphIds);

        {
            // Graphics::ScopedSaveState doesn't restore the clipping regions
            context.saveState();
            const ScopeGuard restoreGraphicsContext { [&context] { context.restoreState(); } };

            if (useClip.clipAtBegin || useClip.clipAtEnd)
            {
                const auto componentBoundsInDrawBasis = getLocalBounds().toFloat().transformedBy (transform.inverted());

                // We don't really want to constrain the vertical clip, so we add/subtract a little extra,
                // because clipping right at the line 0 will still result in a visible clip border with
                // the below code.
                const auto clipTop    = componentBoundsInDrawBasis.getY() - 10.0f;
                const auto clipBottom = componentBoundsInDrawBasis.getBottom() + 10.0f;
                const auto clipX      = useClip.clipAtBegin ? positions.front().getX() : 0.0f;
                const auto clipRight  = useClip.clipAtEnd ? positions.back().getX() + glyphs.back().advance.getX()
                                                          : (float) getRight();

                const Rectangle<float> clipRect { { clipX, clipTop }, { clipRight, clipBottom } };
                Path clipPath;
                clipPath.addRectangle (clipRect);
                context.clipToPath (clipPath, transform);
            }

            context.drawGlyphs (glyphIds, positionsForGlyphIds, transform);
        }

        if (font.isUnderlined())
            drawUnderline (g, glyphs, positions, font,  transform, isWhitespaceUnderlined());

        if (hasTemporaryUnderlining)
        {
            const auto startX = roundToInt (positions.front().getX());
            const auto endX = roundToInt (positions.back().getX() + glyphs.back().advance.getX());
            auto baselineY = roundToInt (positions.front().getY() + 0.5f);

            Graphics::ScopedSaveState state (g);
            g.addTransform (transform);
            g.reduceClipRegion ({ startX, baselineY, endX - startX, 1 });

            g.fillCheckerBoard ({ (float) endX, (float) baselineY + 1.0f },
                                3.0f,
                                1.0f,
                                colour,
                                Colours::transparentBlack);
        }
    };

    const auto clip = std::invoke ([&]
    {
        auto c = g.getClipBounds();
        c.setY (roundToInt ((float) c.getY() - yOffset));
        return c;
    });

    for (auto [range, paragraph] : *textStorage)
    {
        ignoreUnused (range);

        const auto glyphsRange = Range<int64>::withStartAndLength (paragraph->getStartingGlyph(),
                                                                   paragraph->getNumGlyphs());

        const auto top = paragraph->getTop();
        const auto bottom = top + paragraph->getHeight();

        if ((float) clip.getY() <= bottom && top <= (float) clip.getBottom())
        {
            paragraph->getShapedText().accessTogetherWith (drawSelection,
                                                           selectedTextRanges.getIntersectionsStartingAtZeroWith (glyphsRange));

            paragraph->getShapedText().accessTogetherWith (drawGlyphRuns,
                                                           glyphColours.getIntersectionsStartingAtZeroWith (glyphsRange),
                                                           textSelectionMask.getIntersectionsStartingAtZeroWith (glyphsRange),
                                                           underlining.getIntersectionsStartingAtZeroWith (glyphsRange));
        }

        yOffset += paragraph->getHeight();
    }
}

void TextEditor::paint (Graphics& g)
{
    getLookAndFeel().fillTextEditorBackground (g, getWidth(), getHeight(), *this);
}

void TextEditor::paintOverChildren (Graphics& g)
{
    if (textToShowWhenEmpty.isNotEmpty()
         && (! hasKeyboardFocus (false))
         && getTotalNumChars() == 0)
    {
        g.setColour (colourForTextWhenEmpty);
        g.setFont (getFont());

        Rectangle<int> textBounds (leftIndent,
                                   topIndent,
                                   viewport->getWidth() - leftIndent,
                                   getHeight() - topIndent);

        if (! textBounds.isEmpty())
            g.drawText (textToShowWhenEmpty, textBounds, justification, true);
    }

    getLookAndFeel().drawTextEditorOutline (g, getWidth(), getHeight(), *this);
}

//==============================================================================
void TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const bool writable = ! isReadOnly();

    if (passwordCharacter == 0)
    {
        m.addItem (StandardApplicationCommandIDs::cut,   TRANS ("Cut"), writable);
        m.addItem (StandardApplicationCommandIDs::copy,  TRANS ("Copy"), ! selection.isEmpty());
    }

    m.addItem (StandardApplicationCommandIDs::paste,     TRANS ("Paste"), writable);
    m.addItem (StandardApplicationCommandIDs::del,       TRANS ("Delete"), writable);
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::selectAll, TRANS ("Select All"));
    m.addSeparator();

    if (getUndoManager() != nullptr)
    {
        m.addItem (StandardApplicationCommandIDs::undo, TRANS ("Undo"), undoManager.canUndo());
        m.addItem (StandardApplicationCommandIDs::redo, TRANS ("Redo"), undoManager.canRedo());
    }
}

void TextEditor::performPopupMenuAction (const int menuItemID)
{
    switch (menuItemID)
    {
        case StandardApplicationCommandIDs::cut:        cutToClipboard(); break;
        case StandardApplicationCommandIDs::copy:       copyToClipboard(); break;
        case StandardApplicationCommandIDs::paste:      pasteFromClipboard(); break;
        case StandardApplicationCommandIDs::del:        cut(); break;
        case StandardApplicationCommandIDs::selectAll:  selectAll(); break;
        case StandardApplicationCommandIDs::undo:       undo(); break;
        case StandardApplicationCommandIDs::redo:       redo(); break;
        default: break;
    }
}

//==============================================================================
void TextEditor::mouseDown (const MouseEvent& e)
{
    beginDragAutoRepeat (100);
    newTransaction();

    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            caretState.setPreferredEdge (Edge::leading);
            moveCaretTo (getTextIndexAt (e.getPosition()), e.mods.isShiftDown());

            if (auto* peer = getPeer())
                peer->closeInputMethodContext();
        }
        else
        {
            PopupMenu m;
            m.setLookAndFeel (&getLookAndFeel());
            addPopupMenuItems (m, &e);

            menuActive = true;

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMousePosition(),
                             [safeThis = SafePointer { this }] (int menuResult)
                             {
                                 if (auto* editor = safeThis.getComponent())
                                 {
                                     editor->menuActive = false;

                                     if (menuResult != 0)
                                         editor->performPopupMenuAction (menuResult);
                                 }
                             });
        }
    }
}

void TextEditor::mouseDrag (const MouseEvent& e)
{
    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            caretState.setPreferredEdge (Edge::leading);
            moveCaretTo (getTextIndexAt (e.getPosition()), true);
        }
    }
}

void TextEditor::mouseUp (const MouseEvent& e)
{
    newTransaction();
    textHolder->restartTimer();

    if (wasFocused || ! selectAllTextWhenFocused)
        if (e.mouseWasClicked() && ! (popupMenuEnabled && e.mods.isPopupMenu()))
            moveCaret (getTextIndexAt (e.getPosition()));

    wasFocused = true;
}

void TextEditor::mouseDoubleClick (const MouseEvent& e)
{
    int tokenEnd = getTextIndexAt (e.getPosition());
    int tokenStart = 0;

    if (e.getNumberOfClicks() > 3)
    {
        tokenEnd = getTotalNumChars();
    }
    else
    {
        auto t = getText();
        auto totalLength = getTotalNumChars();

        while (tokenEnd < totalLength)
        {
            auto c = t[tokenEnd];

            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (c) || c > 128)
                ++tokenEnd;
            else
                break;
        }

        tokenStart = tokenEnd;

        while (tokenStart > 0)
        {
            auto c = t[tokenStart - 1];

            // (note the slight bodge here - it's because iswalnum only checks for alphabetic chars in the current locale)
            if (CharacterFunctions::isLetterOrDigit (c) || c > 128)
                --tokenStart;
            else
                break;
        }

        if (e.getNumberOfClicks() > 2)
        {
            while (tokenEnd < totalLength)
            {
                auto c = t[tokenEnd];

                if (c != '\r' && c != '\n')
                    ++tokenEnd;
                else
                    break;
            }

            while (tokenStart > 0)
            {
                auto c = t[tokenStart - 1];

                if (c != '\r' && c != '\n')
                    --tokenStart;
                else
                    break;
            }
        }
    }

    moveCaretTo (tokenEnd, false);
    moveCaretTo (tokenStart, true);
}

void TextEditor::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! viewport->useMouseWheelMoveIfNeeded (e, wheel))
        Component::mouseWheelMove (e, wheel);
}

//==============================================================================
bool TextEditor::moveCaretWithTransaction (const int newPos, const bool selecting)
{
    newTransaction();
    moveCaretTo (newPos, selecting);

    if (auto* peer = getPeer())
        peer->closeInputMethodContext();

    return true;
}

bool TextEditor::moveCaretLeft (bool moveInWholeWordSteps, bool selecting)
{
    auto pos = getCaretPosition();

    if (moveInWholeWordSteps)
        pos = findWordBreakBefore (pos);
    else
        --pos;

    return moveCaretWithTransaction (pos, selecting);
}

bool TextEditor::moveCaretRight (bool moveInWholeWordSteps, bool selecting)
{
    auto pos = getCaretPosition();

    if (moveInWholeWordSteps)
        pos = findWordBreakAfter (pos);
    else
        ++pos;

    return moveCaretWithTransaction (pos, selecting);
}

TextEditor::Edge TextEditor::getEdgeTypeCloserToPosition (int indexInText, Point<float> pos) const
{
    const auto testCaret = caretState.withPosition (indexInText);

    const auto leading = getCursorEdge (testCaret.withPreferredEdge (Edge::leading)).anchor.getDistanceFrom (pos);
    const auto trailing = getCursorEdge (testCaret.withPreferredEdge (Edge::trailing)).anchor.getDistanceFrom (pos);

    if (leading < trailing)
        return Edge::leading;

    return Edge::trailing;
}

bool TextEditor::moveCaretUp (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();

    const auto newY = caretPos.getY() - 1.0f;

    if (newY < 0.0f)
        return moveCaretToStartOfLine (selecting);

    const Point<float> testPosition { caretPos.getX(), newY };
    const auto newIndex = indexAtPosition (testPosition.getX(), testPosition.getY());

    const auto edgeToUse = getEdgeTypeCloserToPosition (newIndex, testPosition);
    caretState.setPreferredEdge (edgeToUse);
    return moveCaretWithTransaction (newIndex, selecting);
}

bool TextEditor::moveCaretDown (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    const Point<float> testPosition { caretPos.getX(), caretPos.getBottom() + 1.0f };
    const auto newIndex = indexAtPosition (testPosition.getX(), testPosition.getY());

    const auto edgeToUse = getEdgeTypeCloserToPosition (newIndex, testPosition);
    caretState.setPreferredEdge (edgeToUse);
    return moveCaretWithTransaction (newIndex, selecting);
}

bool TextEditor::pageUp (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getY() - (float) viewport->getViewHeight()), selecting);
}

bool TextEditor::pageDown (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    const auto caretPos = (getCaretRectangle() - getTextOffset()).toFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getBottom() + (float) viewport->getViewHeight()), selecting);
}

void TextEditor::scrollByLines (int deltaLines)
{
    viewport->getVerticalScrollBar().moveScrollbarInSteps (deltaLines);
}

bool TextEditor::scrollDown()
{
    scrollByLines (-1);
    return true;
}

bool TextEditor::scrollUp()
{
    scrollByLines (1);
    return true;
}

bool TextEditor::moveCaretToTop (bool selecting)
{
    return moveCaretWithTransaction (0, selecting);
}

bool TextEditor::moveCaretToStartOfLine (bool selecting)
{
    const auto lineRange = getLineRangeForIndex (caretState.getVisualIndex());
    caretState.setPreferredEdge (Edge::leading);
    return moveCaretWithTransaction ((int) lineRange.getStart(), selecting);
}

bool TextEditor::moveCaretToEnd (bool selecting)
{
    return moveCaretWithTransaction (getTotalNumChars(), selecting);
}

bool TextEditor::moveCaretToEndOfLine (bool selecting)
{
    const auto lineRange = getLineRangeForIndex (caretState.getVisualIndex());
    caretState.setPreferredEdge (Edge::trailing);
    return moveCaretWithTransaction ((int) lineRange.getEnd(), selecting);
}

bool TextEditor::deleteBackwards (bool moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
        moveCaretTo (findWordBreakBefore (getCaretPosition()), true);
    else if (selection.isEmpty() && selection.getStart() > 0)
        setSelection ({ selection.getEnd() - 1, selection.getEnd() });

    cut();
    return true;
}

bool TextEditor::deleteForwards (bool /*moveInWholeWordSteps*/)
{
    if (selection.isEmpty() && selection.getStart() < getTotalNumChars())
        setSelection ({ selection.getStart(), selection.getStart() + 1 });

    cut();
    return true;
}

bool TextEditor::copyToClipboard()
{
    newTransaction();
    copy();
    return true;
}

bool TextEditor::cutToClipboard()
{
    newTransaction();
    copy();
    cut();
    return true;
}

bool TextEditor::pasteFromClipboard()
{
    newTransaction();
    paste();
    return true;
}

bool TextEditor::selectAll()
{
    newTransaction();
    moveCaretTo (getTotalNumChars(), false);
    moveCaretTo (0, true);
    return true;
}

//==============================================================================
void TextEditor::setEscapeAndReturnKeysConsumed (bool shouldBeConsumed) noexcept
{
    consumeEscAndReturnKeys = shouldBeConsumed;
}

bool TextEditor::keyPressed (const KeyPress& key)
{
    if (isReadOnly() && key != KeyPress ('c', ModifierKeys::commandModifier, 0)
                     && key != KeyPress ('a', ModifierKeys::commandModifier, 0))
        return false;

    if (! TextEditorKeyMapper<TextEditor>::invokeKeyFunction (*this, key))
    {
        if (key == KeyPress::returnKey)
        {
            newTransaction();

            if (returnKeyStartsNewLine)
            {
                insertTextAtCaret ("\n");
            }
            else
            {
                returnPressed();
                return consumeEscAndReturnKeys;
            }
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            newTransaction();
            moveCaretTo (getCaretPosition(), false);
            escapePressed();
            return consumeEscAndReturnKeys;
        }
        else if (key.getTextCharacter() >= ' '
                  || (tabKeyUsed && (key.getTextCharacter() == '\t')))
        {
            insertTextAtCaret (String::charToString (key.getTextCharacter()));

            lastTransactionTime = Time::getApproximateMillisecondCounter();
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool TextEditor::keyStateChanged (const bool isKeyDown)
{
    if (! isKeyDown)
        return false;

   #if JUCE_WINDOWS
    if (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0).isCurrentlyDown())
        return false;  // We need to explicitly allow alt-F4 to pass through on Windows
   #endif

    if ((! consumeEscAndReturnKeys)
         && (KeyPress (KeyPress::escapeKey).isCurrentlyDown()
          || KeyPress (KeyPress::returnKey).isCurrentlyDown()))
        return false;

    // (overridden to avoid forwarding key events to the parent)
    return ! ModifierKeys::getCurrentModifiers().isCommandDown();
}

//==============================================================================
void TextEditor::focusGained (FocusChangeType cause)
{
    newTransaction();

    if (selectAllTextWhenFocused)
    {
        moveCaretTo (0, false);
        moveCaretTo (getTotalNumChars(), true);
    }

    checkFocus();

    if (cause == FocusChangeType::focusChangedByMouseClick && selectAllTextWhenFocused)
        wasFocused = false;

    repaint();
    updateCaretPosition();
}

void TextEditor::focusLost (FocusChangeType)
{
    newTransaction();

    wasFocused = false;
    textHolder->stopTimer();

    underlinedSections.clear();

    updateCaretPosition();

    postCommandMessage (TextEditorDefs::focusLossMessageId);
    repaint();
}

//==============================================================================
void TextEditor::resized()
{
    viewport->setBoundsInset (borderSize);
    viewport->setSingleStepSizes (16, roundToInt (currentFont.getHeight()));
    updateBaseShapedTextOptions();

    checkLayout();

    if (isMultiLine())
        updateCaretPosition();
    else
        scrollToMakeSureCursorIsVisible();
}

void TextEditor::handleCommandMessage (const int commandId)
{
    Component::BailOutChecker checker (this);

    switch (commandId)
    {
    case TextEditorDefs::textChangeMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorTextChanged (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onTextChange);

        break;

    case TextEditorDefs::returnKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorReturnKeyPressed (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onReturnKey);

        break;

    case TextEditorDefs::escapeKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorEscapeKeyPressed (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onEscapeKey);

        break;

    case TextEditorDefs::focusLossMessageId:
        updateValueFromText();
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorFocusLost (*this); });

        if (! checker.shouldBailOut())
            NullCheckedInvocation::invoke (onFocusLost);

        break;

    default:
        jassertfalse;
        break;
    }
}

void TextEditor::setTemporaryUnderlining (const Array<Range<int>>& newUnderlinedSections)
{
    underlinedSections = newUnderlinedSections;
    repaint();
}

TextInputTarget::VirtualKeyboardType TextEditor::getKeyboardType()
{
    return passwordCharacter != 0 ? passwordKeyboard : keyboardType;
}

//==============================================================================
UndoManager* TextEditor::getUndoManager() noexcept
{
    return readOnly ? nullptr : &undoManager;
}

void TextEditor::clearInternal (UndoManager* const um)
{
    remove ({ 0, getTotalNumChars() }, um, getCaretPosition());
}

void TextEditor::insert (const String& text, int insertIndex, const Font& font,
                         Colour colour, UndoManager* um, int caretPositionToMoveTo)
{
    if (text.isNotEmpty())
    {
        if (um != nullptr)
        {
            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new InsertAction (*this, text, insertIndex, font, colour,
                                           getCaretPosition(), caretPositionToMoveTo));
        }
        else
        {
            textStorage->set ({ insertIndex, insertIndex }, text, font, colour);
            caretState.updateEdge();

            repaintText ({ insertIndex, getTotalNumChars() }); // must do this before and after changing the data, in case
                                                               // a line gets moved due to word wrap

            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            checkLayout();
            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ insertIndex, getTotalNumChars() });
        }
    }
}

void TextEditor::reinsert (const TextEditorStorageChunks& chunks)
{
    textStorage->addChunks (chunks);
    totalNumChars = -1;
    valueTextNeedsUpdating = true;
}

void TextEditor::remove (Range<int> range,
                         UndoManager* const um,
                         const int caretPositionToMoveTo,
                         TextEditorStorageChunks* removedOut)
{
    using namespace detail;

    if (! range.isEmpty())
    {
        if (um != nullptr)
        {
            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new RemoveAction (*this, range, caretState.getPosition(),
                                           caretPositionToMoveTo));
        }
        else
        {
            textStorage->remove (asInt64Range (range), removedOut);
            caretState.updateEdge();

            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            checkLayout();
            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ range.getStart(), getTotalNumChars() });
        }
    }
}

//==============================================================================
String TextEditor::getText() const
{
    return textStorage->getText();
}

String TextEditor::getTextInRange (const Range<int>& range) const
{
    return textStorage->getTextInRange (asInt64Range (range));
}

String TextEditor::getHighlightedText() const
{
    return getTextInRange (selection);
}

int TextEditor::getTotalNumChars() const
{
    return (int) textStorage->getTotalNumChars();
}

bool TextEditor::isEmpty() const
{
    return getTotalNumChars() == 0;
}

float TextEditor::getJustificationOffsetX() const
{
    const auto bottomRightX = (float) getMaximumTextWidth();

    if (justification.testFlags (Justification::horizontallyCentred)) return jmax (0.0f, bottomRightX * 0.5f);
    if (justification.testFlags (Justification::right))               return jmax (0.0f, bottomRightX);

    return 0.0f;
}

TextEditor::CaretEdge TextEditor::getDefaultCursorEdge() const
{
    return { { getJustificationOffsetX(), 0.0f }, currentFont.getHeight() * lineSpacing };
}

TextEditor::CaretEdge TextEditor::getCursorEdge (const CaretState& tempCaret) const
{
    const auto visualIndex = tempCaret.getVisualIndex();
    jassert (0 <= visualIndex && visualIndex <= getTotalNumChars());

    if (getWordWrapWidth() <= 0)
        return { {}, currentFont.getHeight() };

    if (textStorage->isEmpty())
        return getDefaultCursorEdge();

    if (visualIndex == getTotalNumChars())
    {
        const auto& lastParagraph = textStorage->back().value;

        return { { getJustificationOffsetX(), lastParagraph->getTop() + lastParagraph->getHeight() },
                 currentFont.getHeight() * lineSpacing };
    }

    return getTextSelectionEdge (visualIndex, tempCaret.getEdge());
}

int TextEditor::indexAtPosition (float x, float y) const
{
    y = std::max (0.0f, y);

    if (getWordWrapWidth() <= 0)
        return getTotalNumChars();

    auto paragraphIt = textStorage->begin();
    float paragraphTop = 0.0f;

    while (paragraphIt != textStorage->end())
    {
        auto& paragraph = paragraphIt->value;
        const auto paragraphBottom = paragraphTop + paragraph->getHeight();

        if (paragraphTop <= y && y < paragraphBottom)
            break;

        if (y < paragraphTop)
            return {};

        paragraphTop = paragraphBottom;
        ++paragraphIt;
    }

    if (paragraphIt == textStorage->end())
        return getTotalNumChars();

    auto& shapedText = paragraphIt->value->getShapedText();
    return (int) (shapedText.getTextIndexForCaret ({ x, y - paragraphTop }) + paragraphIt->range.getStart());
}

//==============================================================================
int TextEditor::findWordBreakAfter (const int position) const
{
    auto t = getTextInRange ({ position, position + 512 });
    auto totalLength = t.length();
    int i = 0;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    auto type = TextEditorDefs::getCharacterCategory (t[i]);

    while (i < totalLength && type == TextEditorDefs::getCharacterCategory (t[i]))
        ++i;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    return position + i;
}

int TextEditor::findWordBreakBefore (const int position) const
{
    if (position <= 0)
        return 0;

    auto startOfBuffer = jmax (0, position - 512);
    auto t = getTextInRange ({ startOfBuffer, position });

    int i = position - startOfBuffer;

    while (i > 0 && CharacterFunctions::isWhitespace (t [i - 1]))
        --i;

    if (i > 0)
    {
        auto type = TextEditorDefs::getCharacterCategory (t [i - 1]);

        while (i > 0 && type == TextEditorDefs::getCharacterCategory (t [i - 1]))
            --i;
    }

    jassert (startOfBuffer + i >= 0);
    return startOfBuffer + i;
}

//==============================================================================
TextEditor::CaretState::CaretState (const TextEditor* ownerIn)
    : owner { *ownerIn }
{
    updateEdge();
}

void TextEditor::CaretState::setPosition (int newPosition)
{
    if (std::exchange (position, newPosition) != newPosition)
        updateEdge();
}

void TextEditor::CaretState::setPreferredEdge (TextEditor::Edge newEdge)
{
    if (std::exchange (preferredEdge, newEdge) != newEdge)
        updateEdge();
}

int TextEditor::CaretState::getVisualIndex() const
{
    if (edge == Edge::leading)
        return position;

    return position - 1;
}

TextEditor::CaretState TextEditor::CaretState::withPosition (int newPosition) const
{
    auto copy = *this;
    copy.setPosition (newPosition);
    return copy;
}

TextEditor::CaretState TextEditor::CaretState::withPreferredEdge (Edge newEdge) const
{
    auto copy = *this;
    copy.setPreferredEdge (newEdge);
    return copy;
}

void TextEditor::CaretState::updateEdge()
{
    // The position can be temporarily outside the current text's bounds. It's the TextEditor's
    // responsibility to update the caret position after editing operations.
    const auto clampedPosition = std::clamp (position, 0, owner.getTotalNumChars());

    if (clampedPosition == 0)
    {
        edge = Edge::leading;
    }
    else if (owner.getText()[clampedPosition - 1] == '\n')
    {
        edge = Edge::leading;
    }
    else if (clampedPosition == owner.getTotalNumChars())
    {
        edge = Edge::trailing;
    }
    else
    {
        edge = preferredEdge;
    }
}

//==============================================================================
class TextEditor::EditorAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit EditorAccessibilityHandler (TextEditor& textEditorToWrap)
        : AccessibilityHandler (textEditorToWrap,
                                textEditorToWrap.isReadOnly() ? AccessibilityRole::staticText : AccessibilityRole::editableText,
                                {},
                                { std::make_unique<TextEditorTextInterface> (textEditorToWrap) }),
          textEditor (textEditorToWrap)
    {
    }

    String getHelp() const override  { return textEditor.getTooltip(); }

private:
    class TextEditorTextInterface final : public AccessibilityTextInterface
    {
    public:
        explicit TextEditorTextInterface (TextEditor& editor)
            : textEditor (editor)
        {
        }

        bool isDisplayingProtectedText() const override      { return textEditor.getPasswordCharacter() != 0; }
        bool isReadOnly() const override                     { return textEditor.isReadOnly(); }

        int getTotalNumCharacters() const override           { return textEditor.getText().length(); }
        Range<int> getSelection() const override             { return textEditor.getHighlightedRegion(); }

        void setSelection (Range<int> r) override
        {
            textEditor.setHighlightedRegion (r);
        }

        String getText (Range<int> r) const override
        {
            if (isDisplayingProtectedText())
                return String::repeatedString (String::charToString (textEditor.getPasswordCharacter()),
                                               getTotalNumCharacters());

            return textEditor.getTextInRange (r);
        }

        void setText (const String& newText) override
        {
            textEditor.setText (newText);
        }

        int getTextInsertionOffset() const override          { return textEditor.getCaretPosition(); }

        RectangleList<int> getTextBounds (Range<int> textRange) const override
        {
            auto localRects = textEditor.getTextBounds (textRange);
            RectangleList<int> globalRects;

            std::for_each (localRects.begin(), localRects.end(),
                           [&] (const Rectangle<int>& r) { globalRects.add (textEditor.localAreaToGlobal (r)); });

            return globalRects;
        }

        int getOffsetAtPoint (Point<int> point) const override
        {
            return textEditor.getTextIndexAt (textEditor.getLocalPoint (nullptr, point));
        }

    private:
        TextEditor& textEditor;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditorTextInterface)
    };

    TextEditor& textEditor;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorAccessibilityHandler)
};

std::unique_ptr<AccessibilityHandler> TextEditor::createAccessibilityHandler()
{
    return std::make_unique<EditorAccessibilityHandler> (*this);
}

} // namespace juce
