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
class CodeEditorComponent::CodeEditorAccessibilityHandler final : public AccessibilityHandler
{
public:
    explicit CodeEditorAccessibilityHandler (CodeEditorComponent& codeEditorComponentToWrap)
        : AccessibilityHandler (codeEditorComponentToWrap,
                                codeEditorComponentToWrap.isReadOnly() ? AccessibilityRole::staticText
                                                                       : AccessibilityRole::editableText,
                                {},
                                { std::make_unique<CodeEditorComponentTextInterface> (codeEditorComponentToWrap) })
    {
    }

private:
    class CodeEditorComponentTextInterface final : public AccessibilityTextInterface
    {
    public:
        explicit CodeEditorComponentTextInterface (CodeEditorComponent& codeEditorComponentToWrap)
            : codeEditorComponent (codeEditorComponentToWrap)
        {
        }

        bool isDisplayingProtectedText() const override
        {
            return false;
        }

        bool isReadOnly() const override
        {
            return codeEditorComponent.isReadOnly();
        }

        int getTotalNumCharacters() const override
        {
            return codeEditorComponent.document.getAllContent().length();
        }

        Range<int> getSelection() const override
        {
            return { codeEditorComponent.selectionStart.getPosition(),
                     codeEditorComponent.selectionEnd.getPosition() };
        }

        void setSelection (Range<int> r) override
        {
            codeEditorComponent.setHighlightedRegion (r);
        }

        String getText (Range<int> r) const override
        {
            auto& doc = codeEditorComponent.document;

            return doc.getTextBetween (CodeDocument::Position (doc, r.getStart()),
                                       CodeDocument::Position (doc, r.getEnd()));
        }

        void setText (const String& newText) override
        {
            codeEditorComponent.document.replaceAllContent (newText);
        }

        int getTextInsertionOffset() const override
        {
            return codeEditorComponent.caretPos.getPosition();
        }

        RectangleList<int> getTextBounds (Range<int> textRange) const override
        {
            const auto localRects = codeEditorComponent.getTextBounds (textRange);

            RectangleList<int> globalRects;

            for (auto r : localRects)
                globalRects.add (codeEditorComponent.localAreaToGlobal (r));

            return globalRects;
        }

        int getOffsetAtPoint (Point<int> point) const override
        {
            return codeEditorComponent.getPositionAt (point.x, point.y).getPosition();
        }

    private:
        CodeEditorComponent& codeEditorComponent;
    };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorAccessibilityHandler)
};

//==============================================================================
class CodeEditorComponent::CodeEditorLine
{
public:
    CodeEditorLine() noexcept {}

    bool update (CodeDocument& codeDoc, int lineNum,
                 CodeDocument::Iterator& source,
                 CodeTokeniser* tokeniser, const int tabSpaces,
                 const CodeDocument::Position& selStart,
                 const CodeDocument::Position& selEnd)
    {
        Array<SyntaxToken> newTokens;
        newTokens.ensureStorageAllocated (8);

        if (tokeniser == nullptr)
        {
            auto line = codeDoc.getLine (lineNum);
            addToken (newTokens, line, line.length(), -1);
        }
        else if (lineNum < codeDoc.getNumLines())
        {
            const CodeDocument::Position pos (codeDoc, lineNum, 0);
            createTokens (pos.getPosition(), pos.getLineText(),
                          source, *tokeniser, newTokens);
        }

        replaceTabsWithSpaces (newTokens, tabSpaces);

        int newHighlightStart = 0;
        int newHighlightEnd = 0;

        if (selStart.getLineNumber() <= lineNum && selEnd.getLineNumber() >= lineNum)
        {
            auto line = codeDoc.getLine (lineNum);

            CodeDocument::Position lineStart (codeDoc, lineNum, 0), lineEnd (codeDoc, lineNum + 1, 0);
            newHighlightStart = indexToColumn (jmax (0, selStart.getPosition() - lineStart.getPosition()),
                                               line, tabSpaces);
            newHighlightEnd = indexToColumn (jmin (lineEnd.getPosition() - lineStart.getPosition(), selEnd.getPosition() - lineStart.getPosition()),
                                             line, tabSpaces);
        }

        if (newHighlightStart != highlightColumnStart || newHighlightEnd != highlightColumnEnd)
        {
            highlightColumnStart = newHighlightStart;
            highlightColumnEnd = newHighlightEnd;
        }
        else if (tokens == newTokens)
        {
            return false;
        }

        tokens.swapWith (newTokens);
        return true;
    }

    Optional<Rectangle<float>> getHighlightArea (float x, int y, int lineH, float characterWidth) const
    {
        return getHighlightArea (x, y, lineH, characterWidth, { highlightColumnStart, highlightColumnEnd });
    }

    Optional<Rectangle<float>> getHighlightArea (float x,
                                                 int y,
                                                 int lineH,
                                                 float characterWidth,
                                                 Range<int> highlightColumns) const
    {
        if (highlightColumns.isEmpty())
            return {};

        return Rectangle<float> (x + (float) highlightColumns.getStart() * characterWidth - 1.0f, (float) y - 0.5f,
                                 (float) (highlightColumns.getEnd() - highlightColumns.getStart()) * characterWidth + 1.5f, (float) lineH + 1.0f);

    }

    void draw (CodeEditorComponent& owner, Graphics& g, const Font& fontToUse,
               const float rightClip, const float x, const int y,
               const int lineH, const float characterWidth) const
    {
        AttributedString as;
        as.setJustification (Justification::centredLeft);

        int column = 0;

        for (auto& token : tokens)
        {
            const float tokenX = x + (float) column * characterWidth;
            if (tokenX > rightClip)
                break;

            as.append (token.text.initialSectionNotContaining ("\r\n"), fontToUse, owner.getColourForTokenType (token.tokenType));
            column += token.length;
        }

        as.draw (g, { x, (float) y, (float) column * characterWidth + 10.0f, (float) lineH });
    }

private:
    struct SyntaxToken
    {
        SyntaxToken (const String& t, const int len, const int type) noexcept
            : text (t), length (len), tokenType (type)
        {}

        bool operator== (const SyntaxToken& other) const noexcept
        {
            return tokenType == other.tokenType
                    && length == other.length
                    && text == other.text;
        }

        String text;
        int length;
        int tokenType;
    };

    Array<SyntaxToken> tokens;
    int highlightColumnStart = 0, highlightColumnEnd = 0;

    static void createTokens (int startPosition, const String& lineText,
                              CodeDocument::Iterator& source,
                              CodeTokeniser& tokeniser,
                              Array<SyntaxToken>& newTokens)
    {
        CodeDocument::Iterator lastIterator (source);
        const int lineLength = lineText.length();

        for (;;)
        {
            int tokenType = tokeniser.readNextToken (source);
            int tokenStart = lastIterator.getPosition();
            int tokenEnd = source.getPosition();

            if (tokenEnd <= tokenStart)
                break;

            tokenEnd -= startPosition;

            if (tokenEnd > 0)
            {
                tokenStart -= startPosition;
                const int start = jmax (0, tokenStart);
                addToken (newTokens, lineText.substring (start, tokenEnd),
                          tokenEnd - start, tokenType);

                if (tokenEnd >= lineLength)
                    break;
            }

            lastIterator = source;
        }

        source = lastIterator;
    }

    static void replaceTabsWithSpaces (Array<SyntaxToken>& tokens, const int spacesPerTab)
    {
        int x = 0;

        for (auto& t : tokens)
        {
            for (;;)
            {
                const int tabPos = t.text.indexOfChar ('\t');
                if (tabPos < 0)
                    break;

                const int spacesNeeded = spacesPerTab - ((tabPos + x) % spacesPerTab);
                t.text = t.text.replaceSection (tabPos, 1, String::repeatedString (" ", spacesNeeded));
                t.length = t.text.length();
            }

            x += t.length;
        }
    }

    int indexToColumn (int index, const String& line, int tabSpaces) const noexcept
    {
        jassert (index <= line.length());

        auto t = line.getCharPointer();
        int col = 0;

        for (int i = 0; i < index; ++i)
        {
            if (t.getAndAdvance() != '\t')
                ++col;
            else
                col += tabSpaces - (col % tabSpaces);
        }

        return col;
    }

    static void addToken (Array<SyntaxToken>& dest, const String& text, int length, int type)
    {
        if (length > 1000)
        {
            // subdivide very long tokens to avoid unwieldy glyph sequences
            addToken (dest, text.substring (0, length / 2), length / 2, type);
            addToken (dest, text.substring (length / 2), length - length / 2, type);
        }
        else
        {
            dest.add (SyntaxToken (text, length, type));
        }
    }
};

namespace CodeEditorHelpers
{
    static int findFirstNonWhitespaceChar (StringRef line) noexcept
    {
        auto t = line.text;
        int i = 0;

        while (! t.isEmpty())
        {
            if (! t.isWhitespace())
                return i;

            ++t;
            ++i;
        }

        return 0;
    }
}

//==============================================================================
class CodeEditorComponent::Pimpl   : public Timer,
                                     public AsyncUpdater,
                                     public ScrollBar::Listener,
                                     public CodeDocument::Listener
{
public:
    Pimpl (CodeEditorComponent& ed) : owner (ed) {}

private:
    CodeEditorComponent& owner;

    void timerCallback() override        { owner.newTransaction(); }
    void handleAsyncUpdate() override    { owner.rebuildLineTokens(); }

    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved->isVertical())
            owner.scrollToLineInternal ((int) newRangeStart);
        else
            owner.scrollToColumnInternal (newRangeStart);
    }

    void codeDocumentTextInserted (const String& newText, int pos) override
    {
        owner.codeDocumentChanged (pos, pos + newText.length());
    }

    void codeDocumentTextDeleted (int start, int end) override
    {
        owner.codeDocumentChanged (start, end);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
class CodeEditorComponent::GutterComponent final : public Component
{
public:
    GutterComponent() {}

    void paint (Graphics& g) override
    {
        jassert (dynamic_cast<CodeEditorComponent*> (getParentComponent()) != nullptr);
        auto& editor = *static_cast<CodeEditorComponent*> (getParentComponent());

        g.fillAll (editor.findColour (CodeEditorComponent::backgroundColourId)
                    .overlaidWith (editor.findColour (lineNumberBackgroundId)));

        auto clip = g.getClipBounds();
        const int lineH = editor.lineHeight;
        const float lineHeightFloat = (float) lineH;
        const int firstLineToDraw = jmax (0, clip.getY() / lineH);
        const int lastLineToDraw = jmin (editor.lines.size(), clip.getBottom() / lineH + 1,
                                         lastNumLines - editor.firstLineOnScreen);

        auto lineNumberFont = editor.getFont().withHeight (jmin (13.0f, lineHeightFloat * 0.8f));
        auto w = (float) getWidth() - 2.0f;
        GlyphArrangement ga;

        for (int i = firstLineToDraw; i < lastLineToDraw; ++i)
            ga.addFittedText (lineNumberFont, String (editor.firstLineOnScreen + i + 1),
                              0, (float) (lineH * i), w, lineHeightFloat,
                              Justification::centredRight, 1, 0.2f);

        g.setColour (editor.findColour (lineNumberTextId));
        ga.draw (g);
    }

    void documentChanged (CodeDocument& doc, int newFirstLine)
    {
        auto newNumLines = doc.getNumLines();

        if (newNumLines != lastNumLines || firstLine != newFirstLine)
        {
            firstLine = newFirstLine;
            lastNumLines = newNumLines;
            repaint();
        }
    }

private:
    int firstLine = 0, lastNumLines = 0;
};


//==============================================================================
CodeEditorComponent::CodeEditorComponent (CodeDocument& doc, CodeTokeniser* const tokeniser)
    : document (doc),
      caretPos (doc, 0, 0),
      selectionStart (doc, 0, 0),
      selectionEnd (doc, 0, 0),
      codeTokeniser (tokeniser)
{
    pimpl.reset (new Pimpl (*this));

    caretPos.setPositionMaintained (true);
    selectionStart.setPositionMaintained (true);
    selectionEnd.setPositionMaintained (true);

    setOpaque (true);
    setMouseCursor (MouseCursor::IBeamCursor);
    setWantsKeyboardFocus (true);

    addAndMakeVisible (verticalScrollBar);
    verticalScrollBar.setSingleStepSize (1.0);

    addAndMakeVisible (horizontalScrollBar);
    horizontalScrollBar.setSingleStepSize (1.0);

    Font f (withDefaultMetrics (FontOptions { 12.0f }));
    f.setTypefaceName (Font::getDefaultMonospacedFontName());
    setFont (f);

    if (codeTokeniser != nullptr)
        setColourScheme (codeTokeniser->getDefaultColourScheme());

    setLineNumbersShown (true);

    verticalScrollBar.addListener (pimpl.get());
    horizontalScrollBar.addListener (pimpl.get());
    document.addListener (pimpl.get());

    lookAndFeelChanged();
}

CodeEditorComponent::~CodeEditorComponent()
{
    if (auto* peer = getPeer())
        peer->refreshTextInputTarget();

    document.removeListener (pimpl.get());
}

int CodeEditorComponent::getGutterSize() const noexcept
{
    return showLineNumbers ? 35 : 5;
}

void CodeEditorComponent::loadContent (const String& newContent)
{
    clearCachedIterators (0);
    document.replaceAllContent (newContent);
    document.clearUndoHistory();
    document.setSavePoint();
    caretPos.setPosition (0);
    selectionStart.setPosition (0);
    selectionEnd.setPosition (0);
    scrollToLine (0);
}

bool CodeEditorComponent::isTextInputActive() const
{
    return true;
}

void CodeEditorComponent::setTemporaryUnderlining (const Array<Range<int>>&)
{
    // TODO IME composition ranges not yet supported for this component
}

void CodeEditorComponent::setLineNumbersShown (const bool shouldBeShown)
{
    if (showLineNumbers != shouldBeShown)
    {
        showLineNumbers = shouldBeShown;
        gutter.reset();

        if (shouldBeShown)
        {
            gutter.reset (new GutterComponent());
            addAndMakeVisible (gutter.get());
        }

        resized();
    }
}

void CodeEditorComponent::setReadOnly (bool b) noexcept
{
    if (readOnly != b)
    {
        readOnly = b;

        if (b)
            removeChildComponent (caret.get());
        else
            addAndMakeVisible (caret.get());

        invalidateAccessibilityHandler();
    }
}

//==============================================================================
void CodeEditorComponent::resized()
{
    auto visibleWidth = getWidth() - scrollbarThickness - getGutterSize();
    linesOnScreen   = jmax (1, (getHeight() - scrollbarThickness) / lineHeight);
    columnsOnScreen = jmax (1, (int) ((float) visibleWidth / charWidth));
    lines.clear();
    rebuildLineTokens();
    updateCaretPosition();

    if (gutter != nullptr)
        gutter->setBounds (0, 0, getGutterSize() - 2, getHeight());

    verticalScrollBar.setBounds (getWidth() - scrollbarThickness, 0,
                                 scrollbarThickness, getHeight() - scrollbarThickness);

    horizontalScrollBar.setBounds (getGutterSize(), getHeight() - scrollbarThickness,
                                   visibleWidth, scrollbarThickness);
    updateScrollBars();
}

void CodeEditorComponent::paint (Graphics& g)
{
    g.fillAll (findColour (CodeEditorComponent::backgroundColourId));

    const auto gutterSize = getGutterSize();
    const auto bottom = horizontalScrollBar.isVisible() ? horizontalScrollBar.getY() : getHeight();
    const auto right  = verticalScrollBar.isVisible()   ? verticalScrollBar.getX()   : getWidth();

    g.reduceClipRegion (gutterSize, 0, right - gutterSize, bottom);

    g.setFont (font);

    const auto clip = g.getClipBounds();
    const auto firstLineToDraw = jmax (0, clip.getY() / lineHeight);
    const auto lastLineToDraw  = jmin (lines.size(), clip.getBottom() / lineHeight + 1);
    const auto x = (float) (gutterSize - xOffset * charWidth);
    const auto rightClip = (float) clip.getRight();

    {
        RectangleList<float> highlightArea;

        for (int i = firstLineToDraw; i < lastLineToDraw; ++i)
            if (const auto area = lines.getUnchecked (i)->getHighlightArea (x, lineHeight * i, lineHeight, charWidth))
                highlightArea.add (*area);

        g.setColour (findColour (CodeEditorComponent::highlightColourId));
        g.fillRectList (highlightArea);
    }

    for (int i = firstLineToDraw; i < lastLineToDraw; ++i)
        lines.getUnchecked (i)->draw (*this, g, font, rightClip, x, lineHeight * i, lineHeight, charWidth);
}

void CodeEditorComponent::setScrollbarThickness (const int thickness)
{
    if (scrollbarThickness != thickness)
    {
        scrollbarThickness = thickness;
        resized();
    }
}

void CodeEditorComponent::rebuildLineTokensAsync()
{
    pimpl->triggerAsyncUpdate();
}

void CodeEditorComponent::rebuildLineTokens()
{
    pimpl->cancelPendingUpdate();

    auto numNeeded = linesOnScreen + 1;
    auto minLineToRepaint = numNeeded;
    int maxLineToRepaint = 0;

    if (numNeeded != lines.size())
    {
        lines.clear();

        for (int i = numNeeded; --i >= 0;)
            lines.add (new CodeEditorLine());

        minLineToRepaint = 0;
        maxLineToRepaint = numNeeded;
    }

    jassert (numNeeded == lines.size());

    CodeDocument::Iterator source (document);
    getIteratorForPosition (CodeDocument::Position (document, firstLineOnScreen, 0).getPosition(), source);

    for (int i = 0; i < numNeeded; ++i)
    {
        if (lines.getUnchecked (i)->update (document, firstLineOnScreen + i, source, codeTokeniser,
                                           spacesPerTab, selectionStart, selectionEnd))
        {
            minLineToRepaint = jmin (minLineToRepaint, i);
            maxLineToRepaint = jmax (maxLineToRepaint, i);
        }
    }

    if (minLineToRepaint <= maxLineToRepaint)
        repaint (0, lineHeight * minLineToRepaint - 1,
                 verticalScrollBar.getX(), lineHeight * (1 + maxLineToRepaint - minLineToRepaint) + 2);

    if (gutter != nullptr)
        gutter->documentChanged (document, firstLineOnScreen);
}

void CodeEditorComponent::codeDocumentChanged (const int startIndex, const int endIndex)
{
    const CodeDocument::Position affectedTextStart (document, startIndex);
    const CodeDocument::Position affectedTextEnd (document, endIndex);

    retokenise (startIndex, endIndex);

    updateCaretPosition();
    columnToTryToMaintain = -1;

    if (affectedTextEnd.getPosition() >= selectionStart.getPosition()
         && affectedTextStart.getPosition() <= selectionEnd.getPosition())
        deselectAll();

    if (shouldFollowDocumentChanges)
        if (caretPos.getPosition() > affectedTextEnd.getPosition()
            || caretPos.getPosition() < affectedTextStart.getPosition())
            moveCaretTo (affectedTextStart, false);

    updateScrollBars();
}

void CodeEditorComponent::retokenise (int startIndex, [[maybe_unused]] int endIndex)
{
    const CodeDocument::Position affectedTextStart (document, startIndex);

    clearCachedIterators (affectedTextStart.getLineNumber());

    rebuildLineTokensAsync();
}

//==============================================================================
void CodeEditorComponent::updateCaretPosition()
{
    if (caret != nullptr)
    {
        caret->setCaretPosition (getCharacterBounds (getCaretPos()));

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

void CodeEditorComponent::moveCaretTo (const CodeDocument::Position& newPos, const bool highlighting)
{
    caretPos = newPos;
    columnToTryToMaintain = -1;
    bool selectionWasActive = isHighlightActive();

    if (highlighting)
    {
        if (dragType == notDragging)
        {
            auto oldCaretPos = caretPos.getPosition();
            auto isStart = std::abs (oldCaretPos - selectionStart.getPosition())
                            < std::abs (oldCaretPos - selectionEnd.getPosition());

            dragType = isStart ? draggingSelectionStart : draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            if (selectionEnd.getPosition() < caretPos.getPosition())
            {
                setSelection (selectionEnd, caretPos);
                dragType = draggingSelectionEnd;
            }
            else
            {
                setSelection (caretPos, selectionEnd);
            }
        }
        else
        {
            if (caretPos.getPosition() < selectionStart.getPosition())
            {
                setSelection (caretPos, selectionStart);
                dragType = draggingSelectionStart;
            }
            else
            {
                setSelection (selectionStart, caretPos);
            }
        }

        rebuildLineTokensAsync();
    }
    else
    {
        deselectAll();
    }

    updateCaretPosition();
    scrollToKeepCaretOnScreen();
    updateScrollBars();
    caretPositionMoved();

    if (auto* handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);

    if (appCommandManager != nullptr && selectionWasActive != isHighlightActive())
        appCommandManager->commandStatusChanged();
}

void CodeEditorComponent::deselectAll()
{
    if (isHighlightActive())
        rebuildLineTokensAsync();

    setSelection (caretPos, caretPos);
    dragType = notDragging;
}

void CodeEditorComponent::updateScrollBars()
{
    verticalScrollBar.setRangeLimits (0, jmax (document.getNumLines(), firstLineOnScreen + linesOnScreen));
    verticalScrollBar.setCurrentRange (firstLineOnScreen, linesOnScreen);

    horizontalScrollBar.setRangeLimits (0, jmax ((double) document.getMaximumLineLength(), xOffset + columnsOnScreen));
    horizontalScrollBar.setCurrentRange (xOffset, columnsOnScreen);
}

void CodeEditorComponent::scrollToLineInternal (int newFirstLineOnScreen)
{
    newFirstLineOnScreen = jlimit (0, jmax (0, document.getNumLines() - 1),
                                   newFirstLineOnScreen);

    if (newFirstLineOnScreen != firstLineOnScreen)
    {
        firstLineOnScreen = newFirstLineOnScreen;
        updateCaretPosition();

        updateCachedIterators (firstLineOnScreen);
        rebuildLineTokensAsync();
        pimpl->handleUpdateNowIfNeeded();

        editorViewportPositionChanged();
    }
}

void CodeEditorComponent::scrollToColumnInternal (double column)
{
    const double newOffset = jlimit (0.0, document.getMaximumLineLength() + 3.0, column);

    if (! approximatelyEqual (xOffset, newOffset))
    {
        xOffset = newOffset;
        updateCaretPosition();
        repaint();
    }
}

void CodeEditorComponent::scrollToLine (int newFirstLineOnScreen)
{
    scrollToLineInternal (newFirstLineOnScreen);
    updateScrollBars();
}

void CodeEditorComponent::scrollToColumn (int newFirstColumnOnScreen)
{
    scrollToColumnInternal (newFirstColumnOnScreen);
    updateScrollBars();
}

void CodeEditorComponent::scrollBy (int deltaLines)
{
    scrollToLine (firstLineOnScreen + deltaLines);
}

void CodeEditorComponent::scrollToKeepLinesOnScreen (Range<int> rangeToShow)
{
    if (rangeToShow.getStart() < firstLineOnScreen)
        scrollBy (rangeToShow.getStart() - firstLineOnScreen);
    else if (rangeToShow.getEnd() >= firstLineOnScreen + linesOnScreen)
        scrollBy (rangeToShow.getEnd() - (firstLineOnScreen + linesOnScreen - 1));
}

void CodeEditorComponent::scrollToKeepCaretOnScreen()
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        auto caretLine = caretPos.getLineNumber();
        scrollToKeepLinesOnScreen ({ caretLine, caretLine });

        auto column = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());

        if (column >= xOffset + columnsOnScreen - 1)
            scrollToColumn (column + 1 - columnsOnScreen);
        else if (column < xOffset)
            scrollToColumn (column);
    }
}

Rectangle<int> CodeEditorComponent::getCharacterBounds (const CodeDocument::Position& pos) const
{
    return { roundToInt ((getGutterSize() - xOffset * charWidth) + (float) indexToColumn (pos.getLineNumber(), pos.getIndexInLine()) * charWidth),
             (pos.getLineNumber() - firstLineOnScreen) * lineHeight,
             roundToInt (charWidth),
             lineHeight };
}

CodeDocument::Position CodeEditorComponent::getPositionAt (int x, int y) const
{
    const int line = y / lineHeight + firstLineOnScreen;
    const int column = roundToInt ((x - (getGutterSize() - xOffset * charWidth)) / charWidth);
    const int index = columnToIndex (line, column);

    return CodeDocument::Position (document, line, index);
}

int CodeEditorComponent::getCharIndexForPoint (Point<int> point) const
{
    return getPositionAt (point.x, point.y).getPosition();
}

RectangleList<int> CodeEditorComponent::getTextBounds (Range<int> textRange) const
{
    RectangleList<int> localRects;

    const CodeDocument::Position startPosition (document, textRange.getStart());
    const CodeDocument::Position endPosition   (document, textRange.getEnd());

    for (int line = startPosition.getLineNumber(); line <= endPosition.getLineNumber(); ++line)
    {
        const CodeDocument::Position lineStartColumn0 { document, line, 0 };

        const auto lineStart = line == startPosition.getLineNumber() ? lineStartColumn0.movedBy (startPosition.getIndexInLine())
                                                                     : lineStartColumn0;

        const CodeDocument::Position lineEnd { document, line, line == endPosition.getLineNumber() ? endPosition.getIndexInLine()
                                                                                                   : document.getLine (line).length() };

        const auto startPos = getCharacterBounds (lineStart).getTopLeft();
        const auto endPos   = getCharacterBounds (lineEnd)  .getTopLeft();

        localRects.add (startPos.x, startPos.y, jmax (1, endPos.x - startPos.x), getLineHeight());
    }

    return localRects;
}

//==============================================================================
void CodeEditorComponent::insertTextAtCaret (const String& newText)
{
    insertText (newText);
}

void CodeEditorComponent::insertText (const String& newText)
{
    if (! readOnly)
    {
        document.deleteSection (selectionStart, selectionEnd);

        if (newText.isNotEmpty())
            document.insertText (caretPos, newText);

        scrollToKeepCaretOnScreen();
        caretPositionMoved();

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
    }
}

void CodeEditorComponent::insertTabAtCaret()
{
    if (! readOnly)
    {
        if (CharacterFunctions::isWhitespace (caretPos.getCharacter())
             && caretPos.getLineNumber() == caretPos.movedBy (1).getLineNumber())
        {
            moveCaretTo (document.findWordBreakAfter (caretPos), false);
        }

        if (useSpacesForTabs)
        {
            auto caretCol = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());
            auto spacesNeeded = spacesPerTab - (caretCol % spacesPerTab);
            insertTextAtCaret (String::repeatedString (" ", spacesNeeded));
        }
        else
        {
            insertTextAtCaret ("\t");
        }
    }
}

bool CodeEditorComponent::deleteWhitespaceBackwardsToTabStop()
{
    if (getHighlightedRegion().isEmpty() && ! readOnly)
    {
        for (;;)
        {
            auto currentColumn = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());

            if (currentColumn <= 0 || (currentColumn % spacesPerTab) == 0)
                break;

            moveCaretLeft (false, true);
        }

        auto selected = getTextInRange (getHighlightedRegion());

        if (selected.isNotEmpty() && selected.trim().isEmpty())
        {
            cut();
            return true;
        }
    }

    return false;
}

void CodeEditorComponent::indentSelection()     { indentSelectedLines ( spacesPerTab); }
void CodeEditorComponent::unindentSelection()   { indentSelectedLines (-spacesPerTab); }

void CodeEditorComponent::indentSelectedLines (const int spacesToAdd)
{
    if (! readOnly)
    {
        newTransaction();

        CodeDocument::Position oldSelectionStart (selectionStart), oldSelectionEnd (selectionEnd), oldCaret (caretPos);
        oldSelectionStart.setPositionMaintained (true);
        oldSelectionEnd.setPositionMaintained (true);
        oldCaret.setPositionMaintained (true);

        const int lineStart = selectionStart.getLineNumber();
        int lineEnd = selectionEnd.getLineNumber();

        if (lineEnd > lineStart && selectionEnd.getIndexInLine() == 0)
            --lineEnd;

        for (int line = lineStart; line <= lineEnd; ++line)
        {
            auto lineText = document.getLine (line);
            auto nonWhitespaceStart = CodeEditorHelpers::findFirstNonWhitespaceChar (lineText);

            if (nonWhitespaceStart > 0 || lineText.trimStart().isNotEmpty())
            {
                const CodeDocument::Position wsStart (document, line, 0);
                const CodeDocument::Position wsEnd   (document, line, nonWhitespaceStart);

                const int numLeadingSpaces = indexToColumn (line, wsEnd.getIndexInLine());
                const int newNumLeadingSpaces = jmax (0, numLeadingSpaces + spacesToAdd);

                if (newNumLeadingSpaces != numLeadingSpaces)
                {
                    document.deleteSection (wsStart, wsEnd);
                    document.insertText (wsStart, getTabString (newNumLeadingSpaces));
                }
            }
        }

        setSelection (oldSelectionStart, oldSelectionEnd);

        if (caretPos != oldCaret)
        {
            caretPos = oldCaret;

            if (auto* handler = getAccessibilityHandler())
                handler->notifyAccessibilityEvent (AccessibilityEvent::textChanged);
        }
    }
}

void CodeEditorComponent::cut()
{
    insertText ({});
}

bool CodeEditorComponent::copyToClipboard()
{
    newTransaction();
    auto selection = document.getTextBetween (selectionStart, selectionEnd);

    if (selection.isNotEmpty())
        SystemClipboard::copyTextToClipboard (selection);

    return true;
}

bool CodeEditorComponent::cutToClipboard()
{
    copyToClipboard();
    cut();
    newTransaction();
    return true;
}

bool CodeEditorComponent::pasteFromClipboard()
{
    newTransaction();
    auto clip = SystemClipboard::getTextFromClipboard();

    if (clip.isNotEmpty())
        insertText (clip);

    newTransaction();
    return true;
}

bool CodeEditorComponent::moveCaretLeft (const bool moveInWholeWordSteps, const bool selecting)
{
    newTransaction();

    if (selecting && dragType == notDragging)
    {
        selectRegion (CodeDocument::Position (selectionEnd), CodeDocument::Position (selectionStart));
        dragType = draggingSelectionStart;
    }

    if (isHighlightActive() && ! (selecting || moveInWholeWordSteps))
    {
        moveCaretTo (selectionStart, false);
        return true;
    }

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakBefore (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (-1), selecting);

    return true;
}

bool CodeEditorComponent::moveCaretRight (const bool moveInWholeWordSteps, const bool selecting)
{
    newTransaction();

    if (selecting && dragType == notDragging)
    {
        selectRegion (CodeDocument::Position (selectionStart), CodeDocument::Position (selectionEnd));
        dragType = draggingSelectionEnd;
    }

    if (isHighlightActive() && ! (selecting || moveInWholeWordSteps))
    {
        moveCaretTo (selectionEnd, false);
        return true;
    }

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakAfter (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (1), selecting);

    return true;
}

void CodeEditorComponent::moveLineDelta (const int delta, const bool selecting)
{
    CodeDocument::Position pos (caretPos);
    auto newLineNum = pos.getLineNumber() + delta;

    if (columnToTryToMaintain < 0)
        columnToTryToMaintain = indexToColumn (pos.getLineNumber(), pos.getIndexInLine());

    pos.setLineAndIndex (newLineNum, columnToIndex (newLineNum, columnToTryToMaintain));

    auto colToMaintain = columnToTryToMaintain;
    moveCaretTo (pos, selecting);
    columnToTryToMaintain = colToMaintain;
}

bool CodeEditorComponent::moveCaretDown (const bool selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == document.getNumLines() - 1)
        moveCaretTo (CodeDocument::Position (document, std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), selecting);
    else
        moveLineDelta (1, selecting);

    return true;
}

bool CodeEditorComponent::moveCaretUp (const bool selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == 0)
        moveCaretTo (CodeDocument::Position (document, 0, 0), selecting);
    else
        moveLineDelta (-1, selecting);

    return true;
}

bool CodeEditorComponent::pageDown (const bool selecting)
{
    newTransaction();
    scrollBy (jlimit (0, linesOnScreen, 1 + document.getNumLines() - firstLineOnScreen - linesOnScreen));
    moveLineDelta (linesOnScreen, selecting);
    return true;
}

bool CodeEditorComponent::pageUp (const bool selecting)
{
    newTransaction();
    scrollBy (-linesOnScreen);
    moveLineDelta (-linesOnScreen, selecting);
    return true;
}

bool CodeEditorComponent::scrollUp()
{
    newTransaction();
    scrollBy (1);

    if (caretPos.getLineNumber() < firstLineOnScreen)
        moveLineDelta (1, false);

    return true;
}

bool CodeEditorComponent::scrollDown()
{
    newTransaction();
    scrollBy (-1);

    if (caretPos.getLineNumber() >= firstLineOnScreen + linesOnScreen)
        moveLineDelta (-1, false);

    return true;
}

bool CodeEditorComponent::moveCaretToTop (const bool selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, 0, 0), selecting);
    return true;
}

bool CodeEditorComponent::moveCaretToStartOfLine (const bool selecting)
{
    newTransaction();

    int index = CodeEditorHelpers::findFirstNonWhitespaceChar (caretPos.getLineText());

    if (index >= caretPos.getIndexInLine() && caretPos.getIndexInLine() > 0)
        index = 0;

    moveCaretTo (CodeDocument::Position (document, caretPos.getLineNumber(), index), selecting);
    return true;
}

bool CodeEditorComponent::moveCaretToEnd (const bool selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, std::numeric_limits<int>::max(),
                                         std::numeric_limits<int>::max()), selecting);
    return true;
}

bool CodeEditorComponent::moveCaretToEndOfLine (const bool selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (document, caretPos.getLineNumber(),
                                         std::numeric_limits<int>::max()), selecting);
    return true;
}

bool CodeEditorComponent::deleteBackwards (const bool moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
    {
        cut(); // in case something is already highlighted
        moveCaretTo (document.findWordBreakBefore (caretPos), true);
    }
    else if (selectionStart == selectionEnd && ! skipBackwardsToPreviousTab())
    {
        selectionStart.moveBy (-1);
    }

    cut();
    return true;
}

bool CodeEditorComponent::skipBackwardsToPreviousTab()
{
    auto currentLineText = caretPos.getLineText().removeCharacters ("\r\n");
    auto currentIndex = caretPos.getIndexInLine();

    if (currentLineText.isNotEmpty() && currentLineText.length() == currentIndex)
    {
        const int currentLine = caretPos.getLineNumber();
        const int currentColumn = indexToColumn (currentLine, currentIndex);
        const int previousTabColumn = (currentColumn - 1) - ((currentColumn - 1) % spacesPerTab);
        const int previousTabIndex = columnToIndex (currentLine, previousTabColumn);

        if (currentLineText.substring (previousTabIndex, currentIndex).trim().isEmpty())
        {
            selectionStart.moveBy (previousTabIndex - currentIndex);
            return true;
        }
    }

    return false;
}

bool CodeEditorComponent::deleteForwards (const bool moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
    {
        cut(); // in case something is already highlighted
        moveCaretTo (document.findWordBreakAfter (caretPos), true);
    }
    else
    {
        if (selectionStart == selectionEnd)
            selectionEnd.moveBy (1);
        else
            newTransaction();
    }

    cut();
    return true;
}

bool CodeEditorComponent::selectAll()
{
    newTransaction();
    selectRegion (CodeDocument::Position (document, std::numeric_limits<int>::max(),
                                          std::numeric_limits<int>::max()),
                  CodeDocument::Position (document, 0, 0));
    return true;
}

void CodeEditorComponent::selectRegion (const CodeDocument::Position& start,
                                        const CodeDocument::Position& end)
{
    moveCaretTo (start, false);
    moveCaretTo (end, true);
}

//==============================================================================
bool CodeEditorComponent::undo()
{
    if (readOnly)
        return false;

    ScopedValueSetter<bool> svs (shouldFollowDocumentChanges, true, false);
    document.undo();
    scrollToKeepCaretOnScreen();
    return true;
}

bool CodeEditorComponent::redo()
{
    if (readOnly)
        return false;

    ScopedValueSetter<bool> svs (shouldFollowDocumentChanges, true, false);
    document.redo();
    scrollToKeepCaretOnScreen();
    return true;
}

void CodeEditorComponent::newTransaction()
{
    document.newTransaction();
    pimpl->startTimer (600);
}

void CodeEditorComponent::setCommandManager (ApplicationCommandManager* newManager) noexcept
{
    appCommandManager = newManager;
}

//==============================================================================
Range<int> CodeEditorComponent::getHighlightedRegion() const
{
    return { selectionStart.getPosition(),
             selectionEnd.getPosition() };
}

bool CodeEditorComponent::isHighlightActive() const noexcept
{
    return selectionStart != selectionEnd;
}

void CodeEditorComponent::setHighlightedRegion (const Range<int>& newRange)
{
    if (newRange == getHighlightedRegion())
        return;

    const auto cursorAtStart = newRange.getEnd() == getHighlightedRegion().getStart()
                            || newRange.getEnd() == getHighlightedRegion().getEnd();
    selectRegion (CodeDocument::Position (document, cursorAtStart ? newRange.getEnd() : newRange.getStart()),
                  CodeDocument::Position (document, cursorAtStart ? newRange.getStart() : newRange.getEnd()));
}

String CodeEditorComponent::getTextInRange (const Range<int>& range) const
{
    return document.getTextBetween (CodeDocument::Position (document, range.getStart()),
                                    CodeDocument::Position (document, range.getEnd()));
}

//==============================================================================
bool CodeEditorComponent::keyPressed (const KeyPress& key)
{
    if (! TextEditorKeyMapper<CodeEditorComponent>::invokeKeyFunction (*this, key))
    {
        if (readOnly)
            return false;

        if (key == KeyPress::tabKey || key.getTextCharacter() == '\t')      handleTabKey();
        else if (key == KeyPress::returnKey)                                handleReturnKey();
        else if (key == KeyPress::escapeKey)                                handleEscapeKey();
        else if (key == KeyPress ('[', ModifierKeys::commandModifier, 0))   unindentSelection();
        else if (key == KeyPress (']', ModifierKeys::commandModifier, 0))   indentSelection();
        else if (key.getTextCharacter() >= ' ')                             insertTextAtCaret (String::charToString (key.getTextCharacter()));
        else                                                                return false;
    }

    pimpl->handleUpdateNowIfNeeded();
    return true;
}

void CodeEditorComponent::handleReturnKey()
{
    insertTextAtCaret (document.getNewLineCharacters());
}

void CodeEditorComponent::handleTabKey()
{
    insertTabAtCaret();
}

void CodeEditorComponent::handleEscapeKey()
{
    newTransaction();
}

void CodeEditorComponent::editorViewportPositionChanged()
{
}

void CodeEditorComponent::caretPositionMoved()
{
}

//==============================================================================
ApplicationCommandTarget* CodeEditorComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void CodeEditorComponent::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = { StandardApplicationCommandIDs::cut,
                              StandardApplicationCommandIDs::copy,
                              StandardApplicationCommandIDs::paste,
                              StandardApplicationCommandIDs::del,
                              StandardApplicationCommandIDs::selectAll,
                              StandardApplicationCommandIDs::undo,
                              StandardApplicationCommandIDs::redo };

    commands.addArray (ids, numElementsInArray (ids));
}

void CodeEditorComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const bool anythingSelected = isHighlightActive();

    switch (commandID)
    {
        case StandardApplicationCommandIDs::cut:
            result.setInfo (TRANS ("Cut"), TRANS ("Copies the currently selected text to the clipboard and deletes it."), "Editing", 0);
            result.setActive (anythingSelected && ! readOnly);
            result.defaultKeypresses.add (KeyPress ('x', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::copy:
            result.setInfo (TRANS ("Copy"), TRANS ("Copies the currently selected text to the clipboard."), "Editing", 0);
            result.setActive (anythingSelected);
            result.defaultKeypresses.add (KeyPress ('c', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::paste:
            result.setInfo (TRANS ("Paste"), TRANS ("Inserts text from the clipboard."), "Editing", 0);
            result.setActive (! readOnly);
            result.defaultKeypresses.add (KeyPress ('v', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::del:
            result.setInfo (TRANS ("Delete"), TRANS ("Deletes any selected text."), "Editing", 0);
            result.setActive (anythingSelected && ! readOnly);
            break;

        case StandardApplicationCommandIDs::selectAll:
            result.setInfo (TRANS ("Select All"), TRANS ("Selects all the text in the editor."), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('a', ModifierKeys::commandModifier, 0));
            break;

        case StandardApplicationCommandIDs::undo:
            result.setInfo (TRANS ("Undo"), TRANS ("Undo"), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier, 0));
            result.setActive (document.getUndoManager().canUndo() && ! readOnly);
            break;

        case StandardApplicationCommandIDs::redo:
            result.setInfo (TRANS ("Redo"), TRANS ("Redo"), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
            result.setActive (document.getUndoManager().canRedo() && ! readOnly);
            break;

        default:
            break;
    }
}

bool CodeEditorComponent::perform (const InvocationInfo& info)
{
    return performCommand (info.commandID);
}

void CodeEditorComponent::lookAndFeelChanged()
{
    caret.reset (getLookAndFeel().createCaretComponent (this));
    addAndMakeVisible (caret.get());
}

bool CodeEditorComponent::performCommand (const CommandID commandID)
{
    switch (commandID)
    {
        case StandardApplicationCommandIDs::cut:        cutToClipboard(); break;
        case StandardApplicationCommandIDs::copy:       copyToClipboard(); break;
        case StandardApplicationCommandIDs::paste:      pasteFromClipboard(); break;
        case StandardApplicationCommandIDs::del:        cut(); break;
        case StandardApplicationCommandIDs::selectAll:  selectAll(); break;
        case StandardApplicationCommandIDs::undo:       undo(); break;
        case StandardApplicationCommandIDs::redo:       redo(); break;
        default:                                        return false;
    }

    return true;
}

void CodeEditorComponent::setSelection (CodeDocument::Position newSelectionStart,
                                        CodeDocument::Position newSelectionEnd)
{
    if (selectionStart != newSelectionStart
        || selectionEnd != newSelectionEnd)
    {
        selectionStart = newSelectionStart;
        selectionEnd = newSelectionEnd;

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::textSelectionChanged);
    }
}

//==============================================================================
void CodeEditorComponent::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    m.addItem (StandardApplicationCommandIDs::cut,   TRANS ("Cut"), isHighlightActive() && ! readOnly);
    m.addItem (StandardApplicationCommandIDs::copy,  TRANS ("Copy"), ! getHighlightedRegion().isEmpty());
    m.addItem (StandardApplicationCommandIDs::paste, TRANS ("Paste"), ! readOnly);
    m.addItem (StandardApplicationCommandIDs::del,   TRANS ("Delete"), ! readOnly);
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::selectAll, TRANS ("Select All"));
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::undo,  TRANS ("Undo"), document.getUndoManager().canUndo());
    m.addItem (StandardApplicationCommandIDs::redo,  TRANS ("Redo"), document.getUndoManager().canRedo());
}

void CodeEditorComponent::performPopupMenuAction (const int menuItemID)
{
    performCommand (menuItemID);
}

static void codeEditorMenuCallback (int menuResult, CodeEditorComponent* editor)
{
    if (editor != nullptr && menuResult != 0)
        editor->performPopupMenuAction (menuResult);
}

//==============================================================================
void CodeEditorComponent::mouseDown (const MouseEvent& e)
{
    newTransaction();
    dragType = notDragging;

    if (e.mods.isPopupMenu())
    {
        setMouseCursor (MouseCursor::NormalCursor);

        if (getHighlightedRegion().isEmpty())
        {
            CodeDocument::Position start, end;
            document.findTokenContaining (getPositionAt (e.x, e.y), start, end);

            if (start.getPosition() < end.getPosition())
                selectRegion (start, end);
        }

        PopupMenu m;
        m.setLookAndFeel (&getLookAndFeel());
        addPopupMenuItems (m, &e);

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMousePosition(),
                         ModalCallbackFunction::forComponent (codeEditorMenuCallback, this));
    }
    else
    {
        beginDragAutoRepeat (100);
        moveCaretTo (getPositionAt (e.x, e.y), e.mods.isShiftDown());
    }
}

void CodeEditorComponent::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
        moveCaretTo (getPositionAt (e.x, e.y), true);
}

void CodeEditorComponent::mouseUp (const MouseEvent&)
{
    newTransaction();
    beginDragAutoRepeat (0);
    dragType = notDragging;
    setMouseCursor (MouseCursor::IBeamCursor);
}

void CodeEditorComponent::mouseDoubleClick (const MouseEvent& e)
{
    CodeDocument::Position tokenStart (getPositionAt (e.x, e.y));
    CodeDocument::Position tokenEnd (tokenStart);

    if (e.getNumberOfClicks() > 2)
        document.findLineContaining (tokenStart, tokenStart, tokenEnd);
    else
        document.findTokenContaining (tokenStart, tokenStart, tokenEnd);

    selectRegion (tokenStart, tokenEnd);
    dragType = notDragging;
}

void CodeEditorComponent::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if ((verticalScrollBar.isVisible() && ! approximatelyEqual (wheel.deltaY, 0.0f))
         || (horizontalScrollBar.isVisible() && ! approximatelyEqual (wheel.deltaX, 0.0f)))
    {
        {
            MouseWheelDetails w (wheel);
            w.deltaX = 0;
            verticalScrollBar.mouseWheelMove (e, w);
        }

        {
            MouseWheelDetails w (wheel);
            w.deltaY = 0;
            horizontalScrollBar.mouseWheelMove (e, w);
        }
    }
    else
    {
        Component::mouseWheelMove (e, wheel);
    }
}

//==============================================================================
void CodeEditorComponent::focusGained (FocusChangeType)     { updateCaretPosition(); }
void CodeEditorComponent::focusLost (FocusChangeType)       { updateCaretPosition(); }

//==============================================================================
void CodeEditorComponent::setTabSize (const int numSpaces, const bool insertSpaces)
{
    useSpacesForTabs = insertSpaces;

    if (spacesPerTab != numSpaces)
    {
        spacesPerTab = numSpaces;
        rebuildLineTokensAsync();
    }
}

String CodeEditorComponent::getTabString (const int numSpaces) const
{
    return String::repeatedString (useSpacesForTabs ? " " : "\t",
                                   useSpacesForTabs ? numSpaces
                                                    : (numSpaces / spacesPerTab));
}

int CodeEditorComponent::indexToColumn (int lineNum, int index) const noexcept
{
    auto line = document.getLine (lineNum);
    auto t = line.getCharPointer();
    int col = 0;

    for (int i = 0; i < index; ++i)
    {
        if (t.isEmpty())
        {
            jassertfalse;
            break;
        }

        if (t.getAndAdvance() != '\t')
            ++col;
        else
            col += getTabSize() - (col % getTabSize());
    }

    return col;
}

int CodeEditorComponent::columnToIndex (int lineNum, int column) const noexcept
{
    auto line = document.getLine (lineNum);
    auto t = line.getCharPointer();
    int i = 0, col = 0;

    while (! t.isEmpty())
    {
        if (t.getAndAdvance() != '\t')
            ++col;
        else
            col += getTabSize() - (col % getTabSize());

        if (col > column)
            break;

        ++i;
    }

    return i;
}

//==============================================================================
void CodeEditorComponent::setFont (const Font& newFont)
{
    font = newFont;

    JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
    charWidth = font.getStringWidthFloat ("0");
    JUCE_END_IGNORE_DEPRECATION_WARNINGS

    lineHeight = roundToInt (font.getHeight());
    resized();
}

void CodeEditorComponent::ColourScheme::set (const String& name, Colour colour)
{
    for (auto& tt : types)
    {
        if (tt.name == name)
        {
            tt.colour = colour;
            return;
        }
    }

    TokenType tt;
    tt.name = name;
    tt.colour = colour;
    types.add (tt);
}

void CodeEditorComponent::setColourScheme (const ColourScheme& scheme)
{
    colourScheme = scheme;
    repaint();
}

Colour CodeEditorComponent::getColourForTokenType (const int tokenType) const
{
    return isPositiveAndBelow (tokenType, colourScheme.types.size())
                ? colourScheme.types.getReference (tokenType).colour
                : findColour (CodeEditorComponent::defaultTextColourId);
}

void CodeEditorComponent::clearCachedIterators (const int firstLineToBeInvalid)
{
    int i;
    for (i = cachedIterators.size(); --i >= 0;)
        if (cachedIterators.getUnchecked (i).getLine() < firstLineToBeInvalid)
            break;

    cachedIterators.removeRange (jmax (0, i - 1), cachedIterators.size());
}

void CodeEditorComponent::updateCachedIterators (int maxLineNum)
{
    const int maxNumCachedPositions = 5000;
    const int linesBetweenCachedSources = jmax (10, document.getNumLines() / maxNumCachedPositions);

    if (cachedIterators.size() == 0)
        cachedIterators.add (CodeDocument::Iterator (document));

    if (codeTokeniser != nullptr)
    {
        for (;;)
        {
            const auto last = cachedIterators.getLast();

            if (last.getLine() >= maxLineNum)
                break;

            cachedIterators.add (CodeDocument::Iterator (last));
            auto& t = cachedIterators.getReference (cachedIterators.size() - 1);
            const int targetLine = jmin (maxLineNum, last.getLine() + linesBetweenCachedSources);

            for (;;)
            {
                codeTokeniser->readNextToken (t);

                if (t.getLine() >= targetLine)
                    break;

                if (t.isEOF())
                    return;
            }
        }
    }
}

void CodeEditorComponent::getIteratorForPosition (int position, CodeDocument::Iterator& source)
{
    if (codeTokeniser != nullptr)
    {
        for (int i = cachedIterators.size(); --i >= 0;)
        {
            auto& t = cachedIterators.getReference (i);

            if (t.getPosition() <= position)
            {
                source = t;
                break;
            }
        }

        while (source.getPosition() < position)
        {
            const CodeDocument::Iterator original (source);
            codeTokeniser->readNextToken (source);

            if (source.getPosition() > position || source.isEOF())
            {
                source = original;
                break;
            }
        }
    }
}

CodeEditorComponent::State::State (const CodeEditorComponent& editor)
    : lastTopLine (editor.getFirstLineOnScreen()),
      lastCaretPos (editor.getCaretPos().getPosition()),
      lastSelectionEnd (lastCaretPos)
{
    auto selection = editor.getHighlightedRegion();

    if (lastCaretPos == selection.getStart())
        lastSelectionEnd = selection.getEnd();
    else
        lastSelectionEnd = selection.getStart();
}

CodeEditorComponent::State::State (const State& other) noexcept
    : lastTopLine (other.lastTopLine),
      lastCaretPos (other.lastCaretPos),
      lastSelectionEnd (other.lastSelectionEnd)
{
}

void CodeEditorComponent::State::restoreState (CodeEditorComponent& editor) const
{
    editor.selectRegion (CodeDocument::Position (editor.getDocument(), lastSelectionEnd),
                         CodeDocument::Position (editor.getDocument(), lastCaretPos));

    if (lastTopLine > 0 && lastTopLine < editor.getDocument().getNumLines())
        editor.scrollToLine (lastTopLine);
}

CodeEditorComponent::State::State (const String& s)
{
    auto tokens = StringArray::fromTokens (s, ":", {});

    lastTopLine      = tokens[0].getIntValue();
    lastCaretPos     = tokens[1].getIntValue();
    lastSelectionEnd = tokens[2].getIntValue();
}

String CodeEditorComponent::State::toString() const
{
    return String (lastTopLine) + ":" + String (lastCaretPos) + ":" + String (lastSelectionEnd);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> CodeEditorComponent::createAccessibilityHandler()
{
    return std::make_unique<CodeEditorAccessibilityHandler> (*this);
}

} // namespace juce
