/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

class CodeEditorComponent::CodeEditorLine
{
public:
    CodeEditorLine() noexcept
       : highlightColumnStart (0), highlightColumnEnd (0)
    {
    }

    bool update (CodeDocument& document, int lineNum,
                 CodeDocument::Iterator& source,
                 CodeTokeniser* analyser, const int spacesPerTab,
                 const CodeDocument::Position& selectionStart,
                 const CodeDocument::Position& selectionEnd)
    {
        Array <SyntaxToken> newTokens;
        newTokens.ensureStorageAllocated (8);

        if (analyser == nullptr)
        {
            newTokens.add (SyntaxToken (document.getLine (lineNum), -1));
        }
        else if (lineNum < document.getNumLines())
        {
            const CodeDocument::Position pos (&document, lineNum, 0);
            createTokens (pos.getPosition(), pos.getLineText(),
                          source, analyser, newTokens);
        }

        replaceTabsWithSpaces (newTokens, spacesPerTab);

        int newHighlightStart = 0;
        int newHighlightEnd = 0;

        if (selectionStart.getLineNumber() <= lineNum && selectionEnd.getLineNumber() >= lineNum)
        {
            const String line (document.getLine (lineNum));

            CodeDocument::Position lineStart (&document, lineNum, 0), lineEnd (&document, lineNum + 1, 0);
            newHighlightStart = indexToColumn (jmax (0, selectionStart.getPosition() - lineStart.getPosition()),
                                               line, spacesPerTab);
            newHighlightEnd = indexToColumn (jmin (lineEnd.getPosition() - lineStart.getPosition(), selectionEnd.getPosition() - lineStart.getPosition()),
                                             line, spacesPerTab);
        }

        if (newHighlightStart != highlightColumnStart || newHighlightEnd != highlightColumnEnd)
        {
            highlightColumnStart = newHighlightStart;
            highlightColumnEnd = newHighlightEnd;
        }
        else
        {
            if (tokens.size() == newTokens.size())
            {
                bool allTheSame = true;

                for (int i = newTokens.size(); --i >= 0;)
                {
                    if (tokens.getReference(i) != newTokens.getReference(i))
                    {
                        allTheSame = false;
                        break;
                    }
                }

                if (allTheSame)
                    return false;
            }
        }

        tokens.swapWithArray (newTokens);
        return true;
    }

    void draw (CodeEditorComponent& owner, Graphics& g, const Font& font,
               float x, const int y, const int baselineOffset, const int lineHeight,
               const Colour& highlightColour) const
    {
        if (highlightColumnStart < highlightColumnEnd)
        {
            g.setColour (highlightColour);
            g.fillRect (roundToInt (x + highlightColumnStart * owner.getCharWidth()), y,
                        roundToInt ((highlightColumnEnd - highlightColumnStart) * owner.getCharWidth()), lineHeight);
        }

        int lastType = std::numeric_limits<int>::min();

        for (int i = 0; i < tokens.size(); ++i)
        {
            SyntaxToken& token = tokens.getReference(i);

            if (lastType != token.tokenType)
            {
                lastType = token.tokenType;
                g.setColour (owner.getColourForTokenType (lastType));
            }

            g.drawSingleLineText (token.text, roundToInt (x), y + baselineOffset);

            if (i < tokens.size() - 1)
            {
                if (token.width < 0)
                    token.width = font.getStringWidthFloat (token.text);

                x += token.width;
            }
        }
    }

private:
    struct SyntaxToken
    {
        SyntaxToken (const String& text_, const int type) noexcept
            : text (text_), tokenType (type), width (-1.0f)
        {
        }

        bool operator!= (const SyntaxToken& other) const noexcept
        {
            return text != other.text || tokenType != other.tokenType;
        }

        String text;
        int tokenType;
        float width;
    };

    Array <SyntaxToken> tokens;
    int highlightColumnStart, highlightColumnEnd;

    static void createTokens (int startPosition, const String& lineText,
                              CodeDocument::Iterator& source,
                              CodeTokeniser* analyser,
                              Array <SyntaxToken>& newTokens)
    {
        CodeDocument::Iterator lastIterator (source);
        const int lineLength = lineText.length();

        for (;;)
        {
            int tokenType = analyser->readNextToken (source);
            int tokenStart = lastIterator.getPosition();
            int tokenEnd = source.getPosition();

            if (tokenEnd <= tokenStart)
                break;

            tokenEnd -= startPosition;

            if (tokenEnd > 0)
            {
                tokenStart -= startPosition;
                newTokens.add (SyntaxToken (lineText.substring (jmax (0, tokenStart), tokenEnd),
                                            tokenType));

                if (tokenEnd >= lineLength)
                    break;
            }

            lastIterator = source;
        }

        source = lastIterator;
    }

    static void replaceTabsWithSpaces (Array <SyntaxToken>& tokens, const int spacesPerTab)
    {
        int x = 0;
        for (int i = 0; i < tokens.size(); ++i)
        {
            SyntaxToken& t = tokens.getReference(i);

            for (;;)
            {
                int tabPos = t.text.indexOfChar ('\t');
                if (tabPos < 0)
                    break;

                const int spacesNeeded = spacesPerTab - ((tabPos + x) % spacesPerTab);
                t.text = t.text.replaceSection (tabPos, 1, String::repeatedString (" ", spacesNeeded));
            }

            x += t.text.length();
        }
    }

    int indexToColumn (int index, const String& line, int spacesPerTab) const noexcept
    {
        jassert (index <= line.length());

        String::CharPointerType t (line.getCharPointer());
        int col = 0;
        for (int i = 0; i < index; ++i)
        {
            if (t.getAndAdvance() != '\t')
                ++col;
            else
                col += spacesPerTab - (col % spacesPerTab);
        }

        return col;
    }
};

//==============================================================================
CodeEditorComponent::CodeEditorComponent (CodeDocument& document_,
                                          CodeTokeniser* const codeTokeniser_)
    : document (document_),
      firstLineOnScreen (0),
      gutter (5),
      spacesPerTab (4),
      lineHeight (0),
      linesOnScreen (0),
      columnsOnScreen (0),
      scrollbarThickness (16),
      columnToTryToMaintain (-1),
      useSpacesForTabs (false),
      xOffset (0),
      verticalScrollBar (true),
      horizontalScrollBar (false),
      codeTokeniser (codeTokeniser_)
{
    caretPos = CodeDocument::Position (&document_, 0, 0);
    caretPos.setPositionMaintained (true);

    selectionStart = CodeDocument::Position (&document_, 0, 0);
    selectionStart.setPositionMaintained (true);

    selectionEnd = CodeDocument::Position (&document_, 0, 0);
    selectionEnd.setPositionMaintained (true);

    setOpaque (true);
    setMouseCursor (MouseCursor (MouseCursor::IBeamCursor));
    setWantsKeyboardFocus (true);

    addAndMakeVisible (&verticalScrollBar);
    verticalScrollBar.setSingleStepSize (1.0);

    addAndMakeVisible (&horizontalScrollBar);
    horizontalScrollBar.setSingleStepSize (1.0);

    addAndMakeVisible (caret = getLookAndFeel().createCaretComponent (this));

    Font f (12.0f);
    f.setTypefaceName (Font::getDefaultMonospacedFontName());
    setFont (f);

    resetToDefaultColours();

    verticalScrollBar.addListener (this);
    horizontalScrollBar.addListener (this);
    document.addListener (this);
}

CodeEditorComponent::~CodeEditorComponent()
{
    document.removeListener (this);
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

void CodeEditorComponent::setTemporaryUnderlining (const Array <Range<int> >&)
{
    jassertfalse; // TODO Windows IME not yet supported for this comp..
}

Rectangle<int> CodeEditorComponent::getCaretRectangle()
{
    return getLocalArea (caret, caret->getLocalBounds());
}

//==============================================================================
void CodeEditorComponent::codeDocumentChanged (const CodeDocument::Position& affectedTextStart,
                                               const CodeDocument::Position& affectedTextEnd)
{
    clearCachedIterators (affectedTextStart.getLineNumber());

    triggerAsyncUpdate();

    updateCaretPosition();
    columnToTryToMaintain = -1;

    if (affectedTextEnd.getPosition() >= selectionStart.getPosition()
         && affectedTextStart.getPosition() <= selectionEnd.getPosition())
        deselectAll();

    if (caretPos.getPosition() > affectedTextEnd.getPosition()
         || caretPos.getPosition() < affectedTextStart.getPosition())
        moveCaretTo (affectedTextStart, false);

    updateScrollBars();
}

void CodeEditorComponent::resized()
{
    linesOnScreen = (getHeight() - scrollbarThickness) / lineHeight;
    columnsOnScreen = (int) ((getWidth() - scrollbarThickness) / charWidth);
    lines.clear();
    rebuildLineTokens();
    updateCaretPosition();

    verticalScrollBar.setBounds (getWidth() - scrollbarThickness, 0, scrollbarThickness, getHeight() - scrollbarThickness);
    horizontalScrollBar.setBounds (gutter, getHeight() - scrollbarThickness, getWidth() - scrollbarThickness - gutter, scrollbarThickness);
    updateScrollBars();
}

void CodeEditorComponent::paint (Graphics& g)
{
    handleUpdateNowIfNeeded();

    g.fillAll (findColour (CodeEditorComponent::backgroundColourId));

    g.reduceClipRegion (gutter, 0, verticalScrollBar.getX() - gutter, horizontalScrollBar.getY());

    g.setFont (font);
    const int baselineOffset = (int) font.getAscent();
    const Colour defaultColour (findColour (CodeEditorComponent::defaultTextColourId));
    const Colour highlightColour (findColour (CodeEditorComponent::highlightColourId));

    const Rectangle<int> clip (g.getClipBounds());
    const int firstLineToDraw = jmax (0, clip.getY() / lineHeight);
    const int lastLineToDraw = jmin (lines.size(), clip.getBottom() / lineHeight + 1);

    for (int j = firstLineToDraw; j < lastLineToDraw; ++j)
    {
        lines.getUnchecked(j)->draw (*this, g, font,
                                     (float) (gutter - xOffset * charWidth),
                                     lineHeight * j, baselineOffset, lineHeight,
                                     highlightColour);
    }
}

void CodeEditorComponent::setScrollbarThickness (const int thickness)
{
    if (scrollbarThickness != thickness)
    {
        scrollbarThickness = thickness;
        resized();
    }
}

void CodeEditorComponent::handleAsyncUpdate()
{
    rebuildLineTokens();
}

void CodeEditorComponent::rebuildLineTokens()
{
    cancelPendingUpdate();

    const int numNeeded = linesOnScreen + 1;

    int minLineToRepaint = numNeeded;
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

    CodeDocument::Iterator source (&document);
    getIteratorForPosition (CodeDocument::Position (&document, firstLineOnScreen, 0).getPosition(), source);

    for (int i = 0; i < numNeeded; ++i)
    {
        CodeEditorLine* const line = lines.getUnchecked(i);

        if (line->update (document, firstLineOnScreen + i, source, codeTokeniser, spacesPerTab,
                          selectionStart, selectionEnd))
        {
            minLineToRepaint = jmin (minLineToRepaint, i);
            maxLineToRepaint = jmax (maxLineToRepaint, i);
        }
    }

    if (minLineToRepaint <= maxLineToRepaint)
    {
        repaint (gutter, lineHeight * minLineToRepaint - 1,
                 verticalScrollBar.getX() - gutter,
                 lineHeight * (1 + maxLineToRepaint - minLineToRepaint) + 2);
    }
}

//==============================================================================
void CodeEditorComponent::updateCaretPosition()
{
    caret->setCaretPosition (getCharacterBounds (getCaretPos()));
}

void CodeEditorComponent::moveCaretTo (const CodeDocument::Position& newPos, const bool highlighting)
{
    caretPos = newPos;
    columnToTryToMaintain = -1;

    if (highlighting)
    {
        if (dragType == notDragging)
        {
            if (abs (caretPos.getPosition() - selectionStart.getPosition())
                  < abs (caretPos.getPosition() - selectionEnd.getPosition()))
                dragType = draggingSelectionStart;
            else
                dragType = draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            selectionStart = caretPos;

            if (selectionEnd.getPosition() < selectionStart.getPosition())
            {
                const CodeDocument::Position temp (selectionStart);
                selectionStart = selectionEnd;
                selectionEnd = temp;

                dragType = draggingSelectionEnd;
            }
        }
        else
        {
            selectionEnd = caretPos;

            if (selectionEnd.getPosition() < selectionStart.getPosition())
            {
                const CodeDocument::Position temp (selectionStart);
                selectionStart = selectionEnd;
                selectionEnd = temp;

                dragType = draggingSelectionStart;
            }
        }

        triggerAsyncUpdate();
    }
    else
    {
        deselectAll();
    }

    updateCaretPosition();
    scrollToKeepCaretOnScreen();
    updateScrollBars();
}

void CodeEditorComponent::deselectAll()
{
    if (selectionStart != selectionEnd)
        triggerAsyncUpdate();

    selectionStart = caretPos;
    selectionEnd = caretPos;
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
        triggerAsyncUpdate();
    }
}

void CodeEditorComponent::scrollToColumnInternal (double column)
{
    const double newOffset = jlimit (0.0, document.getMaximumLineLength() + 3.0, column);

    if (xOffset != newOffset)
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

void CodeEditorComponent::scrollToKeepCaretOnScreen()
{
    if (caretPos.getLineNumber() < firstLineOnScreen)
        scrollBy (caretPos.getLineNumber() - firstLineOnScreen);
    else if (caretPos.getLineNumber() >= firstLineOnScreen + linesOnScreen)
        scrollBy (caretPos.getLineNumber() - (firstLineOnScreen + linesOnScreen - 1));

    const int column = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());
    if (column >= xOffset + columnsOnScreen - 1)
        scrollToColumn (column + 1 - columnsOnScreen);
    else if (column < xOffset)
        scrollToColumn (column);
}

Rectangle<int> CodeEditorComponent::getCharacterBounds (const CodeDocument::Position& pos) const
{
    return Rectangle<int> (roundToInt ((gutter - xOffset * charWidth) + indexToColumn (pos.getLineNumber(), pos.getIndexInLine()) * charWidth),
                           (pos.getLineNumber() - firstLineOnScreen) * lineHeight,
                           roundToInt (charWidth),
                           lineHeight);
}

CodeDocument::Position CodeEditorComponent::getPositionAt (int x, int y)
{
    const int line = y / lineHeight + firstLineOnScreen;
    const int column = roundToInt ((x - (gutter - xOffset * charWidth)) / charWidth);
    const int index = columnToIndex (line, column);

    return CodeDocument::Position (&document, line, index);
}

//==============================================================================
void CodeEditorComponent::insertTextAtCaret (const String& newText)
{
    document.deleteSection (selectionStart, selectionEnd);

    if (newText.isNotEmpty())
        document.insertText (caretPos, newText);

    scrollToKeepCaretOnScreen();
}

void CodeEditorComponent::insertTabAtCaret()
{
    if (CharacterFunctions::isWhitespace (caretPos.getCharacter())
         && caretPos.getLineNumber() == caretPos.movedBy (1).getLineNumber())
    {
        moveCaretTo (document.findWordBreakAfter (caretPos), false);
    }

    if (useSpacesForTabs)
    {
        const int caretCol = indexToColumn (caretPos.getLineNumber(), caretPos.getIndexInLine());
        const int spacesNeeded = spacesPerTab - (caretCol % spacesPerTab);
        insertTextAtCaret (String::repeatedString (" ", spacesNeeded));
    }
    else
    {
        insertTextAtCaret ("\t");
    }
}

void CodeEditorComponent::cut()
{
    insertTextAtCaret (String::empty);
}

bool CodeEditorComponent::copyToClipboard()
{
    newTransaction();

    const String selection (document.getTextBetween (selectionStart, selectionEnd));

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
    const String clip (SystemClipboard::getTextFromClipboard());

    if (clip.isNotEmpty())
        insertTextAtCaret (clip);

    newTransaction();
    return true;
}

bool CodeEditorComponent::moveCaretLeft (const bool moveInWholeWordSteps, const bool selecting)
{
    newTransaction();

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakBefore (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (-1), selecting);

    return true;
}

bool CodeEditorComponent::moveCaretRight (const bool moveInWholeWordSteps, const bool selecting)
{
    newTransaction();

    if (moveInWholeWordSteps)
        moveCaretTo (document.findWordBreakAfter (caretPos), selecting);
    else
        moveCaretTo (caretPos.movedBy (1), selecting);

    return true;
}

void CodeEditorComponent::moveLineDelta (const int delta, const bool selecting)
{
    CodeDocument::Position pos (caretPos);
    const int newLineNum = pos.getLineNumber() + delta;

    if (columnToTryToMaintain < 0)
        columnToTryToMaintain = indexToColumn (pos.getLineNumber(), pos.getIndexInLine());

    pos.setLineAndIndex (newLineNum, columnToIndex (newLineNum, columnToTryToMaintain));

    const int colToMaintain = columnToTryToMaintain;
    moveCaretTo (pos, selecting);
    columnToTryToMaintain = colToMaintain;
}

bool CodeEditorComponent::moveCaretDown (const bool selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == document.getNumLines() - 1)
        moveCaretTo (CodeDocument::Position (&document, std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), selecting);
    else
        moveLineDelta (1, selecting);

    return true;
}

bool CodeEditorComponent::moveCaretUp (const bool selecting)
{
    newTransaction();

    if (caretPos.getLineNumber() == 0)
        moveCaretTo (CodeDocument::Position (&document, 0, 0), selecting);
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
    moveCaretTo (CodeDocument::Position (&document, 0, 0), selecting);
    return true;
}

namespace CodeEditorHelpers
{
    static int findFirstNonWhitespaceChar (const String& line) noexcept
    {
        String::CharPointerType t (line.getCharPointer());
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

bool CodeEditorComponent::moveCaretToStartOfLine (const bool selecting)
{
    newTransaction();

    int index = CodeEditorHelpers::findFirstNonWhitespaceChar (caretPos.getLineText());

    if (index >= caretPos.getIndexInLine() && caretPos.getIndexInLine() > 0)
        index = 0;

    moveCaretTo (CodeDocument::Position (&document, caretPos.getLineNumber(), index), selecting);
    return true;
}

bool CodeEditorComponent::moveCaretToEnd (const bool selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (&document, std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), selecting);
    return true;
}

bool CodeEditorComponent::moveCaretToEndOfLine (const bool selecting)
{
    newTransaction();
    moveCaretTo (CodeDocument::Position (&document, caretPos.getLineNumber(), std::numeric_limits<int>::max()), selecting);
    return true;
}

bool CodeEditorComponent::deleteBackwards (const bool moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
    {
        cut(); // in case something is already highlighted
        moveCaretTo (document.findWordBreakBefore (caretPos), true);
    }
    else
    {
        if (selectionStart == selectionEnd)
            selectionStart.moveBy (-1);
    }

    cut();
    return true;
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
    moveCaretTo (CodeDocument::Position (&document, std::numeric_limits<int>::max(), std::numeric_limits<int>::max()), false);
    moveCaretTo (CodeDocument::Position (&document, 0, 0), true);
    return true;
}

//==============================================================================
bool CodeEditorComponent::undo()
{
    document.undo();
    scrollToKeepCaretOnScreen();
    return true;
}

bool CodeEditorComponent::redo()
{
    document.redo();
    scrollToKeepCaretOnScreen();
    return true;
}

void CodeEditorComponent::newTransaction()
{
    document.newTransaction();
    startTimer (600);
}

void CodeEditorComponent::timerCallback()
{
    newTransaction();
}

//==============================================================================
Range<int> CodeEditorComponent::getHighlightedRegion() const
{
    return Range<int> (selectionStart.getPosition(), selectionEnd.getPosition());
}

void CodeEditorComponent::setHighlightedRegion (const Range<int>& newRange)
{
    moveCaretTo (CodeDocument::Position (&document, newRange.getStart()), false);
    moveCaretTo (CodeDocument::Position (&document, newRange.getEnd()), true);
}

String CodeEditorComponent::getTextInRange (const Range<int>& range) const
{
    return document.getTextBetween (CodeDocument::Position (&document, range.getStart()),
                                    CodeDocument::Position (&document, range.getEnd()));
}

//==============================================================================
bool CodeEditorComponent::keyPressed (const KeyPress& key)
{
    if (! TextEditorKeyMapper<CodeEditorComponent>::invokeKeyFunction (*this, key))
    {
        if (key == KeyPress::tabKey || key.getTextCharacter() == '\t')
        {
            insertTabAtCaret();
        }
        else if (key == KeyPress::returnKey)
        {
            newTransaction();
            insertTextAtCaret (document.getNewLineCharacters());
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            newTransaction();
        }
        else if (key.getTextCharacter() >= ' ')
        {
            insertTextAtCaret (String::charToString (key.getTextCharacter()));
        }
        else
        {
            return false;
        }
    }

    return true;
}

void CodeEditorComponent::mouseDown (const MouseEvent& e)
{
    newTransaction();
    dragType = notDragging;

    if (! e.mods.isPopupMenu())
    {
        beginDragAutoRepeat (100);
        moveCaretTo (getPositionAt (e.x, e.y), e.mods.isShiftDown());
    }
    else
    {
        /*PopupMenu m;
        addPopupMenuItems (m, &e);

        const int result = m.show();

        if (result != 0)
            performPopupMenuAction (result);
        */
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
}

void CodeEditorComponent::mouseDoubleClick (const MouseEvent& e)
{
    CodeDocument::Position tokenStart (getPositionAt (e.x, e.y));
    CodeDocument::Position tokenEnd (tokenStart);

    if (e.getNumberOfClicks() > 2)
    {
        tokenStart.setLineAndIndex (tokenStart.getLineNumber(), 0);
        tokenEnd.setLineAndIndex (tokenStart.getLineNumber() + 1, 0);
    }
    else
    {
        while (CharacterFunctions::isLetterOrDigit (tokenEnd.getCharacter()))
            tokenEnd.moveBy (1);

        tokenStart = tokenEnd;

        while (tokenStart.getIndexInLine() > 0
                && CharacterFunctions::isLetterOrDigit (tokenStart.movedBy (-1).getCharacter()))
            tokenStart.moveBy (-1);
    }

    moveCaretTo (tokenEnd, false);
    moveCaretTo (tokenStart, true);
}

void CodeEditorComponent::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if ((verticalScrollBar.isVisible() && wheelIncrementY != 0)
         || (horizontalScrollBar.isVisible() && wheelIncrementX != 0))
    {
        verticalScrollBar.mouseWheelMove (e, 0, wheelIncrementY);
        horizontalScrollBar.mouseWheelMove (e, wheelIncrementX, 0);
    }
    else
    {
        Component::mouseWheelMove (e, wheelIncrementX, wheelIncrementY);
    }
}

void CodeEditorComponent::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == &verticalScrollBar)
        scrollToLineInternal ((int) newRangeStart);
    else
        scrollToColumnInternal (newRangeStart);
}

//==============================================================================
void CodeEditorComponent::focusGained (FocusChangeType)
{
    updateCaretPosition();
}

void CodeEditorComponent::focusLost (FocusChangeType)
{
    updateCaretPosition();
}

//==============================================================================
void CodeEditorComponent::setTabSize (const int numSpaces, const bool insertSpaces)
{
    useSpacesForTabs = insertSpaces;

    if (spacesPerTab != numSpaces)
    {
        spacesPerTab = numSpaces;
        triggerAsyncUpdate();
    }
}

int CodeEditorComponent::indexToColumn (int lineNum, int index) const noexcept
{
    String::CharPointerType t (document.getLine (lineNum).getCharPointer());

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
    String::CharPointerType t (document.getLine (lineNum).getCharPointer());

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
    charWidth = font.getStringWidthFloat ("0");
    lineHeight = roundToInt (font.getHeight());
    resized();
}

void CodeEditorComponent::resetToDefaultColours()
{
    coloursForTokenCategories.clear();

    if (codeTokeniser != nullptr)
    {
        for (int i = codeTokeniser->getTokenTypes().size(); --i >= 0;)
            setColourForTokenType (i, codeTokeniser->getDefaultColour (i));
    }
}

void CodeEditorComponent::setColourForTokenType (const int tokenType, const Colour& colour)
{
    jassert (tokenType < 256);

    while (coloursForTokenCategories.size() < tokenType)
        coloursForTokenCategories.add (Colours::black);

    coloursForTokenCategories.set (tokenType, colour);
    repaint();
}

Colour CodeEditorComponent::getColourForTokenType (const int tokenType) const
{
    if (! isPositiveAndBelow (tokenType, coloursForTokenCategories.size()))
        return findColour (CodeEditorComponent::defaultTextColourId);

    return coloursForTokenCategories.getReference (tokenType);
}

void CodeEditorComponent::clearCachedIterators (const int firstLineToBeInvalid)
{
    int i;
    for (i = cachedIterators.size(); --i >= 0;)
        if (cachedIterators.getUnchecked (i)->getLine() < firstLineToBeInvalid)
            break;

    cachedIterators.removeRange (jmax (0, i - 1), cachedIterators.size());
}

void CodeEditorComponent::updateCachedIterators (int maxLineNum)
{
    const int maxNumCachedPositions = 5000;
    const int linesBetweenCachedSources = jmax (10, document.getNumLines() / maxNumCachedPositions);

    if (cachedIterators.size() == 0)
        cachedIterators.add (new CodeDocument::Iterator (&document));

    if (codeTokeniser == nullptr)
        return;

    for (;;)
    {
        CodeDocument::Iterator* last = cachedIterators.getLast();

        if (last->getLine() >= maxLineNum)
            break;

        CodeDocument::Iterator* t = new CodeDocument::Iterator (*last);
        cachedIterators.add (t);
        const int targetLine = last->getLine() + linesBetweenCachedSources;

        for (;;)
        {
            codeTokeniser->readNextToken (*t);

            if (t->getLine() >= targetLine)
                break;

            if (t->isEOF())
                return;
        }
    }
}

void CodeEditorComponent::getIteratorForPosition (int position, CodeDocument::Iterator& source)
{
    if (codeTokeniser == nullptr)
        return;

    for (int i = cachedIterators.size(); --i >= 0;)
    {
        CodeDocument::Iterator* t = cachedIterators.getUnchecked (i);
        if (t->getPosition() <= position)
        {
            source = *t;
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
