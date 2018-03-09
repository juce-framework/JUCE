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

// a word or space that can't be broken down any further
struct TextAtom
{
    //==============================================================================
    String atomText;
    float width;
    int numChars;

    //==============================================================================
    bool isWhitespace() const noexcept       { return CharacterFunctions::isWhitespace (atomText[0]); }
    bool isNewLine() const noexcept          { return atomText[0] == '\r' || atomText[0] == '\n'; }

    String getText (juce_wchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText;

        return String::repeatedString (String::charToString (passwordCharacter),
                                       atomText.length());
    }

    String getTrimmedText (const juce_wchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText.substring (0, numChars);

        if (isNewLine())
            return {};

        return String::repeatedString (String::charToString (passwordCharacter), numChars);
    }

    JUCE_LEAK_DETECTOR (TextAtom)
};

//==============================================================================
// a run of text with a single font and colour
class TextEditor::UniformTextSection
{
public:
    UniformTextSection (const String& text, const Font& f, Colour col, juce_wchar passwordChar)
        : font (f), colour (col)
    {
        initialiseAtoms (text, passwordChar);
    }

    UniformTextSection (const UniformTextSection&) = default;

    // VS2013 can't default move constructors
    UniformTextSection (UniformTextSection&& other)
        : font (static_cast<Font&&> (other.font)),
          colour (other.colour),
          atoms (static_cast<Array<TextAtom>&&> (other.atoms))
    {
    }

    UniformTextSection& operator= (const UniformTextSection&) = delete;

    void append (UniformTextSection& other, const juce_wchar passwordChar)
    {
        if (! other.atoms.isEmpty())
        {
            int i = 0;

            if (! atoms.isEmpty())
            {
                auto& lastAtom = atoms.getReference (atoms.size() - 1);

                if (! CharacterFunctions::isWhitespace (lastAtom.atomText.getLastCharacter()))
                {
                    auto& first = other.atoms.getReference(0);

                    if (! CharacterFunctions::isWhitespace (first.atomText[0]))
                    {
                        lastAtom.atomText += first.atomText;
                        lastAtom.numChars = (uint16) (lastAtom.numChars + first.numChars);
                        lastAtom.width = font.getStringWidthFloat (lastAtom.getText (passwordChar));
                        ++i;
                    }
                }
            }

            atoms.ensureStorageAllocated (atoms.size() + other.atoms.size() - i);

            while (i < other.atoms.size())
            {
                atoms.add (other.atoms.getReference(i));
                ++i;
            }
        }
    }

    UniformTextSection* split (int indexToBreakAt, juce_wchar passwordChar)
    {
        auto* section2 = new UniformTextSection (String(), font, colour, passwordChar);
        int index = 0;

        for (int i = 0; i < atoms.size(); ++i)
        {
            auto& atom = atoms.getReference(i);
            auto nextIndex = index + atom.numChars;

            if (index == indexToBreakAt)
            {
                for (int j = i; j < atoms.size(); ++j)
                    section2->atoms.add (atoms.getUnchecked (j));

                atoms.removeRange (i, atoms.size());
                break;
            }

            if (indexToBreakAt >= index && indexToBreakAt < nextIndex)
            {
                TextAtom secondAtom;
                secondAtom.atomText = atom.atomText.substring (indexToBreakAt - index);
                secondAtom.width = font.getStringWidthFloat (secondAtom.getText (passwordChar));
                secondAtom.numChars = (uint16) secondAtom.atomText.length();

                section2->atoms.add (secondAtom);

                atom.atomText = atom.atomText.substring (0, indexToBreakAt - index);
                atom.width = font.getStringWidthFloat (atom.getText (passwordChar));
                atom.numChars = (uint16) (indexToBreakAt - index);

                for (int j = i + 1; j < atoms.size(); ++j)
                    section2->atoms.add (atoms.getUnchecked (j));

                atoms.removeRange (i + 1, atoms.size());
                break;
            }

            index = nextIndex;
        }

        return section2;
    }

    void appendAllText (MemoryOutputStream& mo) const
    {
        for (auto& atom : atoms)
            mo << atom.atomText;
    }

    void appendSubstring (MemoryOutputStream& mo, Range<int> range) const
    {
        int index = 0;

        for (auto& atom : atoms)
        {
            auto nextIndex = index + atom.numChars;

            if (range.getStart() < nextIndex)
            {
                if (range.getEnd() <= index)
                    break;

                auto r = (range - index).getIntersectionWith ({ 0, (int) atom.numChars });

                if (! r.isEmpty())
                    mo << atom.atomText.substring (r.getStart(), r.getEnd());
            }

            index = nextIndex;
        }
    }

    int getTotalLength() const noexcept
    {
        int total = 0;

        for (auto& atom : atoms)
            total += atom.numChars;

        return total;
    }

    void setFont (const Font& newFont, const juce_wchar passwordChar)
    {
        if (font != newFont)
        {
            font = newFont;

            for (auto& atom : atoms)
                atom.width = newFont.getStringWidthFloat (atom.getText (passwordChar));
        }
    }

    //==============================================================================
    Font font;
    Colour colour;
    Array<TextAtom> atoms;

private:
    void initialiseAtoms (const String& textToParse, const juce_wchar passwordChar)
    {
        auto text = textToParse.getCharPointer();

        while (! text.isEmpty())
        {
            size_t numChars = 0;
            auto start = text;

            // create a whitespace atom unless it starts with non-ws
            if (text.isWhitespace() && *text != '\r' && *text != '\n')
            {
                do
                {
                    ++text;
                    ++numChars;
                }
                while (text.isWhitespace() && *text != '\r' && *text != '\n');
            }
            else
            {
                if (*text == '\r')
                {
                    ++text;
                    ++numChars;

                    if (*text == '\n')
                    {
                        ++start;
                        ++text;
                    }
                }
                else if (*text == '\n')
                {
                    ++text;
                    ++numChars;
                }
                else
                {
                    while (! (text.isEmpty() || text.isWhitespace()))
                    {
                        ++text;
                        ++numChars;
                    }
                }
            }

            TextAtom atom;
            atom.atomText = String (start, numChars);
            atom.width = font.getStringWidthFloat (atom.getText (passwordChar));
            atom.numChars = (uint16) numChars;
            atoms.add (atom);
        }
    }

    JUCE_LEAK_DETECTOR (UniformTextSection)
};

//==============================================================================
struct TextEditor::Iterator
{
    Iterator (const TextEditor& ed)
      : sections (ed.sections),
        justification (ed.justification),
        justificationWidth (ed.getJustificationWidth()),
        wordWrapWidth (ed.getWordWrapWidth()),
        passwordCharacter (ed.passwordCharacter),
        lineSpacing (ed.lineSpacing)
    {
        jassert (wordWrapWidth > 0);

        if (! sections.isEmpty())
        {
            currentSection = sections.getUnchecked (sectionIndex);

            if (currentSection != nullptr)
                beginNewLine();
        }
    }

    Iterator (const Iterator&) = default;
    Iterator& operator= (const Iterator&) = delete;

    //==============================================================================
    bool next()
    {
        if (atom == &tempAtom)
        {
            auto numRemaining = tempAtom.atomText.length() - tempAtom.numChars;

            if (numRemaining > 0)
            {
                tempAtom.atomText = tempAtom.atomText.substring (tempAtom.numChars);

                if (tempAtom.numChars > 0)
                    lineY += lineHeight * lineSpacing;

                indexInText += tempAtom.numChars;

                GlyphArrangement g;
                g.addLineOfText (currentSection->font, atom->getText (passwordCharacter), 0.0f, 0.0f);

                int split;
                for (split = 0; split < g.getNumGlyphs(); ++split)
                    if (shouldWrap (g.getGlyph (split).getRight()))
                        break;

                if (split > 0 && split <= numRemaining)
                {
                    tempAtom.numChars = (uint16) split;
                    tempAtom.width = g.getGlyph (split - 1).getRight();
                    atomX = getJustificationOffset (tempAtom.width);
                    atomRight = atomX + tempAtom.width;
                    return true;
                }
            }
        }

        if (sectionIndex >= sections.size())
        {
            moveToEndOfLastAtom();
            return false;
        }

        bool forceNewLine = false;

        if (atomIndex >= currentSection->atoms.size() - 1)
        {
            if (atomIndex >= currentSection->atoms.size())
            {
                if (++sectionIndex >= sections.size())
                {
                    moveToEndOfLastAtom();
                    return false;
                }

                atomIndex = 0;
                currentSection = sections.getUnchecked (sectionIndex);
            }
            else
            {
                auto& lastAtom = currentSection->atoms.getReference (atomIndex);

                if (! lastAtom.isWhitespace())
                {
                    // handle the case where the last atom in a section is actually part of the same
                    // word as the first atom of the next section...
                    float right = atomRight + lastAtom.width;
                    float lineHeight2 = lineHeight;
                    float maxDescent2 = maxDescent;

                    for (int section = sectionIndex + 1; section < sections.size(); ++section)
                    {
                        auto* s = sections.getUnchecked (section);

                        if (s->atoms.size() == 0)
                            break;

                        auto& nextAtom = s->atoms.getReference (0);

                        if (nextAtom.isWhitespace())
                            break;

                        right += nextAtom.width;

                        lineHeight2 = jmax (lineHeight2, s->font.getHeight());
                        maxDescent2 = jmax (maxDescent2, s->font.getDescent());

                        if (shouldWrap (right))
                        {
                            lineHeight = lineHeight2;
                            maxDescent = maxDescent2;

                            forceNewLine = true;
                            break;
                        }

                        if (s->atoms.size() > 1)
                            break;
                    }
                }
            }
        }

        if (atom != nullptr)
        {
            atomX = atomRight;
            indexInText += atom->numChars;

            if (atom->isNewLine())
                beginNewLine();
        }

        atom = &(currentSection->atoms.getReference (atomIndex));
        atomRight = atomX + atom->width;
        ++atomIndex;

        if (shouldWrap (atomRight) || forceNewLine)
        {
            if (atom->isWhitespace())
            {
                // leave whitespace at the end of a line, but truncate it to avoid scrolling
                atomRight = jmin (atomRight, wordWrapWidth);
            }
            else
            {
                if (shouldWrap (atom->width))  // atom too big to fit on a line, so break it up..
                {
                    tempAtom = *atom;
                    tempAtom.width = 0;
                    tempAtom.numChars = 0;
                    atom = &tempAtom;

                    if (atomX > justificationOffset)
                        beginNewLine();

                    return next();
                }

                beginNewLine();
                atomX = justificationOffset;
                atomRight = atomX + atom->width;
                return true;
            }
        }

        return true;
    }

    void beginNewLine()
    {
        lineY += lineHeight * lineSpacing;
        float lineWidth = 0;

        auto tempSectionIndex = sectionIndex;
        auto tempAtomIndex = atomIndex;
        auto* section = sections.getUnchecked (tempSectionIndex);

        lineHeight = section->font.getHeight();
        maxDescent = section->font.getDescent();

        float nextLineWidth = (atom != nullptr) ? atom->width : 0.0f;

        while (! shouldWrap (nextLineWidth))
        {
            lineWidth = nextLineWidth;

            if (tempSectionIndex >= sections.size())
                break;

            bool checkSize = false;

            if (tempAtomIndex >= section->atoms.size())
            {
                if (++tempSectionIndex >= sections.size())
                    break;

                tempAtomIndex = 0;
                section = sections.getUnchecked (tempSectionIndex);
                checkSize = true;
            }

            if (! isPositiveAndBelow (tempAtomIndex, section->atoms.size()))
                break;

            auto& nextAtom = section->atoms.getReference (tempAtomIndex);
            nextLineWidth += nextAtom.width;

            if (shouldWrap (nextLineWidth) || nextAtom.isNewLine())
                break;

            if (checkSize)
            {
                lineHeight = jmax (lineHeight, section->font.getHeight());
                maxDescent = jmax (maxDescent, section->font.getDescent());
            }

            ++tempAtomIndex;
        }

        justificationOffset = getJustificationOffset (lineWidth);
        atomX = justificationOffset;
    }

    float getJustificationOffset (float lineWidth) const
    {
        if (justification.getOnlyHorizontalFlags() == Justification::horizontallyCentred)
            return jmax (0.0f, (justificationWidth - lineWidth) * 0.5f);

        if (justification.getOnlyHorizontalFlags() == Justification::right)
            return jmax (0.0f, justificationWidth - lineWidth);

        return 0;
    }

    //==============================================================================
    void draw (Graphics& g, const UniformTextSection*& lastSection) const
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            if (lastSection != currentSection)
            {
                lastSection = currentSection;
                g.setColour (currentSection->colour);
                g.setFont (currentSection->font);
            }

            jassert (atom->getTrimmedText (passwordCharacter).isNotEmpty());

            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX, (float) roundToInt (lineY + lineHeight - maxDescent));
            ga.draw (g);
        }
    }

    void addSelection (RectangleList<float>& area, Range<int> selected) const
    {
        auto startX = indexToX (selected.getStart());
        auto endX   = indexToX (selected.getEnd());

        area.add (startX, lineY, endX - startX, lineHeight * lineSpacing);
    }

    void drawUnderline (Graphics& g, Range<int> underline, Colour colour) const
    {
        auto startX    = roundToInt (indexToX (underline.getStart()));
        auto endX      = roundToInt (indexToX (underline.getEnd()));
        auto baselineY = roundToInt (lineY + currentSection->font.getAscent() + 0.5f);

        Graphics::ScopedSaveState state (g);
        g.reduceClipRegion ({ startX, baselineY, endX - startX, 1 });
        g.fillCheckerBoard ({ (float) endX, baselineY + 1.0f }, 3.0f, 1.0f, colour, Colours::transparentBlack);
    }

    void drawSelectedText (Graphics& g, Range<int> selected, Colour selectedTextColour) const
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX, (float) roundToInt (lineY + lineHeight - maxDescent));

            if (selected.getEnd() < indexInText + atom->numChars)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (0, selected.getEnd() - indexInText);
                ga.removeRangeOfGlyphs (selected.getEnd() - indexInText, -1);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            if (selected.getStart() > indexInText)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (selected.getStart() - indexInText, -1);
                ga.removeRangeOfGlyphs (0, selected.getStart() - indexInText);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            g.setColour (selectedTextColour);
            ga.draw (g);
        }
    }

    //==============================================================================
    float indexToX (int indexToFind) const
    {
        if (indexToFind <= indexInText)
            return atomX;

        if (indexToFind >= indexInText + atom->numChars)
            return atomRight;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        if (indexToFind - indexInText >= g.getNumGlyphs())
            return atomRight;

        return jmin (atomRight, g.getGlyph (indexToFind - indexInText).getLeft());
    }

    int xToIndex (float xToFind) const
    {
        if (xToFind <= atomX || atom->isNewLine())
            return indexInText;

        if (xToFind >= atomRight)
            return indexInText + atom->numChars;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        auto numGlyphs = g.getNumGlyphs();

        int j;
        for (j = 0; j < numGlyphs; ++j)
        {
            auto& pg = g.getGlyph(j);

            if ((pg.getLeft() + pg.getRight()) / 2 > xToFind)
                break;
        }

        return indexInText + j;
    }

    //==============================================================================
    bool getCharPosition (int index, Point<float>& anchor, float& lineHeightFound)
    {
        while (next())
        {
            if (indexInText + atom->numChars > index)
            {
                anchor = { indexToX (index), lineY };
                lineHeightFound = lineHeight;
                return true;
            }
        }

        anchor = { atomX, lineY };
        lineHeightFound = lineHeight;
        return false;
    }

    //==============================================================================
    int indexInText = 0;
    float lineY = 0, justificationOffset = 0, lineHeight = 0, maxDescent = 0;
    float atomX = 0, atomRight = 0;
    const TextAtom* atom = nullptr;
    const UniformTextSection* currentSection = nullptr;

private:
    const OwnedArray<UniformTextSection>& sections;
    int sectionIndex = 0, atomIndex = 0;
    Justification justification;
    const float justificationWidth, wordWrapWidth;
    const juce_wchar passwordCharacter;
    const float lineSpacing;
    TextAtom tempAtom;

    void moveToEndOfLastAtom()
    {
        if (atom != nullptr)
        {
            atomX = atomRight;

            if (atom->isNewLine())
            {
                atomX = 0.0f;
                lineY += lineHeight * lineSpacing;
            }
        }
    }

    bool shouldWrap (const float x) const noexcept
    {
        return (x - 0.0001f) >= wordWrapWidth;
    }

    JUCE_LEAK_DETECTOR (Iterator)
};


//==============================================================================
struct TextEditor::InsertAction  : public UndoableAction
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
        owner.insert (text, insertIndex, font, colour, 0, newCaretPos);
        return true;
    }

    bool undo() override
    {
        owner.remove ({ insertIndex, insertIndex + text.length() }, 0, oldCaretPos);
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
struct TextEditor::RemoveAction  : public UndoableAction
{
    RemoveAction (TextEditor& ed, Range<int> rangeToRemove, int oldCaret, int newCaret,
                  const Array<UniformTextSection*>& oldSections)
        : owner (ed),
          range (rangeToRemove),
          oldCaretPos (oldCaret),
          newCaretPos (newCaret)
    {
        removedSections.addArray (oldSections);
    }

    bool perform() override
    {
        owner.remove (range, 0, newCaretPos);
        return true;
    }

    bool undo() override
    {
        owner.reinsert (range.getStart(), removedSections);
        owner.moveCaretTo (oldCaretPos, false);
        return true;
    }

    int getSizeInUnits() override
    {
        int n = 16;

        for (auto* s : removedSections)
            n += s->getTotalLength();

        return n;
    }

private:
    TextEditor& owner;
    const Range<int> range;
    const int oldCaretPos, newCaretPos;
    OwnedArray<UniformTextSection> removedSections;

    JUCE_DECLARE_NON_COPYABLE (RemoveAction)
};

//==============================================================================
struct TextEditor::TextHolderComponent  : public Component,
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

    ~TextHolderComponent()
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

    JUCE_DECLARE_NON_COPYABLE (TextHolderComponent)
};

//==============================================================================
struct TextEditor::TextEditorViewport  : public Viewport
{
    TextEditorViewport (TextEditor& ed) : owner (ed) {}

    void visibleAreaChanged (const Rectangle<int>&) override
    {
        if (! rentrant) // it's rare, but possible to get into a feedback loop as the viewport's scrollbars
                        // appear and disappear, causing the wrap width to change.
        {
            auto wordWrapWidth = owner.getWordWrapWidth();

            if (wordWrapWidth != lastWordWrapWidth)
            {
                lastWordWrapWidth = wordWrapWidth;

                rentrant = true;
                owner.updateTextHolderSize();
                rentrant = false;
            }
        }
    }

private:
    TextEditor& owner;
    float lastWordWrapWidth = 0;
    bool rentrant = false;

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
      passwordCharacter (passwordChar)
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
    if (wasFocused)
        if (auto* peer = getPeer())
            peer->dismissPendingTextInput();

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
            scrollToMakeSureCursorIsVisible();
            repaint();
            textChanged();
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

        viewport->setScrollBarsShown (scrollbarVisible && multiline,
                                      scrollbarVisible && multiline);
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
        shown = shown && isMultiLine();
        viewport->setScrollBarsShown (shown, shown);
    }
}

void TextEditor::setReadOnly (bool shouldBeReadOnly)
{
    if (readOnly != shouldBeReadOnly)
    {
        readOnly = shouldBeReadOnly;
        enablementChanged();
    }
}

bool TextEditor::isReadOnly() const noexcept
{
    return readOnly || ! isEnabled();
}

bool TextEditor::isTextInputActive() const
{
    return ! isReadOnly();
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
    }
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

    auto overallColour = findColour (textColourId);

    for (auto* uts : sections)
    {
        uts->setFont (newFont, passwordCharacter);
        uts->colour = overallColour;
    }

    coalesceSimilarSections();
    updateTextHolderSize();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

void TextEditor::applyColourToAllText (const Colour& newColour, bool changeCurrentTextColour)
{
    for (auto* uts : sections)
        uts->colour = newColour;

    if (changeCurrentTextColour)
        setColour (TextEditor::textColourId, newColour);
    else
        repaint();
}

void TextEditor::lookAndFeelChanged()
{
    caret.reset();
    recreateCaret();
    repaint();
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
    if (caret != nullptr)
        caret->setCaretPosition (getCaretRectangle().translated (leftIndent, topIndent));
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
    updateTextHolderSize();
    undoManager.clearUndoHistory();
}

void TextEditor::setText (const String& newText, bool sendTextChangeMessage)
{
    auto newLength = newText.length();

    if (newLength != getTotalNumChars() || getText() != newText)
    {
        if (! sendTextChangeMessage)
            textValue.removeListener (textHolder);

        textValue = newText;

        auto oldCursorPos = caretPosition;
        bool cursorWasAtEnd = oldCursorPos >= getTotalNumChars();

        clearInternal (nullptr);
        insert (newText, 0, currentFont, findColour (textColourId), 0, caretPosition);

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

        updateTextHolderSize();
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
    updateTextHolderSize();

    if (listeners.size() != 0 || onTextChange != nullptr)
        postCommandMessage (TextEditorDefs::textChangeMessageId);

    if (textValue.getValueSource().getReferenceCount() > 1)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }
}

void TextEditor::returnPressed()    { postCommandMessage (TextEditorDefs::returnKeyMessageId); }
void TextEditor::escapePressed()    { postCommandMessage (TextEditorDefs::escapeKeyMessageId); }

void TextEditor::addListener (Listener* l)      { listeners.add (l); }
void TextEditor::removeListener (Listener* l)   { listeners.remove (l); }

//==============================================================================
void TextEditor::timerCallbackInt()
{
    if (hasKeyboardFocus (false) && ! isCurrentlyBlockedByAnotherModalComponent())
    {
        wasFocused = true;

        if (auto* peer = getPeer())
            if (! isReadOnly())
                peer->textInputRequired (peer->globalToLocal (getScreenPosition()), *this);
    }

    auto now = Time::getApproximateMillisecondCounter();

    if (now > lastTransactionTime + 200)
        newTransaction();
}

void TextEditor::repaintText (Range<int> range)
{
    if (! range.isEmpty())
    {
        auto lh = currentFont.getHeight();
        auto wordWrapWidth = getWordWrapWidth();

        if (wordWrapWidth > 0)
        {
            Point<float> anchor;
            Iterator i (*this);
            i.getCharPosition (range.getStart(), anchor, lh);

            auto y1 = (int) anchor.y;
            int y2;

            if (range.getEnd() >= getTotalNumChars())
            {
                y2 = textHolder->getHeight();
            }
            else
            {
                i.getCharPosition (range.getEnd(), anchor, lh);
                y2 = (int) (anchor.y + lh * 2.0f);
            }

            textHolder->repaint (0, y1, textHolder->getWidth(), y2 - y1);
        }
    }
}

//==============================================================================
void TextEditor::moveCaret (int newCaretPos)
{
    if (newCaretPos < 0)
        newCaretPos = 0;
    else
        newCaretPos = jmin (newCaretPos, getTotalNumChars());

    if (newCaretPos != getCaretPosition())
    {
        caretPosition = newCaretPos;
        textHolder->restartTimer();
        scrollToMakeSureCursorIsVisible();
        updateCaretPosition();
    }
}

int TextEditor::getCaretPosition() const
{
    return caretPosition;
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
    auto caretPos = getCaretRectangle();

    auto vx = caretPos.getX() - desiredCaretX;
    auto vy = caretPos.getY() - desiredCaretY;

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
        else if (desiredCaretY > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - caretPos.getHeight()))
            vy += desiredCaretY + 2 + caretPos.getHeight() + topIndent - viewport->getMaximumVisibleHeight();
    }

    viewport->setViewPosition (vx, vy);
}

Rectangle<int> TextEditor::getCaretRectangle()
{
    return getCaretRectangleFloat().getSmallestIntegerContainer();
}

Rectangle<float> TextEditor::getCaretRectangleFloat() const
{
    Point<float> anchor;
    auto cursorHeight = currentFont.getHeight(); // (in case the text is empty and the call below doesn't set this value)
    getCharPosition (caretPosition, anchor, cursorHeight);

    return { anchor.x, anchor.y, 2.0f, cursorHeight };
}

//==============================================================================
enum { rightEdgeSpace = 2 };

float TextEditor::getWordWrapWidth() const
{
    return wordWrap ? getJustificationWidth()
                    : std::numeric_limits<float>::max();
}

float TextEditor::getJustificationWidth() const
{
    return (float) (viewport->getMaximumVisibleWidth() - (leftIndent + rightEdgeSpace + 1));
}

void TextEditor::updateTextHolderSize()
{
    if (getWordWrapWidth() > 0)
    {
        float maxWidth = getJustificationWidth();
        Iterator i (*this);

        while (i.next())
            maxWidth = jmax (maxWidth, i.atomRight);

        auto w = leftIndent + roundToInt (maxWidth);
        auto h = topIndent + roundToInt (jmax (i.lineY + i.lineHeight, currentFont.getHeight()));

        textHolder->setSize (w + rightEdgeSpace, h + 1); // (allows a bit of space for the cursor to be at the right-hand-edge)
    }
}

int TextEditor::getTextWidth() const    { return textHolder->getWidth(); }
int TextEditor::getTextHeight() const   { return textHolder->getHeight(); }

void TextEditor::setIndents (int newLeftIndent, int newTopIndent)
{
    leftIndent = newLeftIndent;
    topIndent  = newTopIndent;
}

void TextEditor::setBorder (const BorderSize<int>& border)
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
        auto caretRect = getCaretRectangle();
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
        else if (relativeCursor.y > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - caretRect.getHeight()))
        {
            viewPos.y += relativeCursor.y + 2 + caretRect.getHeight() + topIndent - viewport->getMaximumVisibleHeight();
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

            selection = Range<int>::between (getCaretPosition(), selection.getEnd());
        }
        else
        {
            if (getCaretPosition() < selection.getStart())
                dragType = draggingSelectionStart;

            selection = Range<int>::between (getCaretPosition(), selection.getStart());
        }

        repaintText (selection.getUnionWith (oldSelection));
    }
    else
    {
        dragType = notDragging;

        repaintText (selection);

        moveCaret (newPosition);
        selection = Range<int>::emptyRange (getCaretPosition());
    }
}

int TextEditor::getTextIndexAt (const int x, const int y)
{
    return indexAtPosition ((float) (x + viewport->getViewPositionX() - leftIndent - borderSize.getLeft()),
                            (float) (y + viewport->getViewPositionY() - topIndent  - borderSize.getTop()));
}

void TextEditor::insertTextAtCaret (const String& t)
{
    String newText (inputFilter != nullptr ? inputFilter->filterNewText (*this, t) : t);

    if (isMultiLine())
        newText = newText.replace ("\r\n", "\n");
    else
        newText = newText.replaceCharacters ("\r\n", "  ");

    const int insertIndex = selection.getStart();
    const int newCaretPos = insertIndex + newText.length();

    remove (selection, getUndoManager(),
            newText.isNotEmpty() ? newCaretPos - 1 : newCaretPos);

    insert (newText, insertIndex, currentFont, findColour (textColourId),
            getUndoManager(), newCaretPos);

    textChanged();
}

void TextEditor::setHighlightedRegion (const Range<int>& newSelection)
{
    moveCaretTo (newSelection.getStart(), false);
    moveCaretTo (newSelection.getEnd(), true);
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

//==============================================================================
void TextEditor::drawContent (Graphics& g)
{
    if (getWordWrapWidth() > 0)
    {
        g.setOrigin (leftIndent, topIndent);
        auto clip = g.getClipBounds();
        Colour selectedTextColour;
        Iterator i (*this);

        if (! selection.isEmpty())
        {
            Iterator i2 (i);
            RectangleList<float> selectionArea;

            while (i2.next() && i2.lineY < clip.getBottom())
            {
                if (i2.lineY + i2.lineHeight >= clip.getY()
                    && selection.intersects ({ i2.indexInText, i2.indexInText + i2.atom->numChars }))
                {
                    i2.addSelection (selectionArea, selection);
                }
            }

            g.setColour (findColour (highlightColourId).withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));
            g.fillRectList (selectionArea);

            selectedTextColour = findColour (highlightedTextColourId);
        }

        const UniformTextSection* lastSection = nullptr;

        while (i.next() && i.lineY < clip.getBottom())
        {
            if (i.lineY + i.lineHeight >= clip.getY())
            {
                if (selection.intersects ({ i.indexInText, i.indexInText + i.atom->numChars }))
                {
                    i.drawSelectedText (g, selection, selectedTextColour);
                    lastSection = nullptr;
                }
                else
                {
                    i.draw (g, lastSection);
                }
            }
        }

        for (auto& underlinedSection : underlinedSections)
        {
            Iterator i2 (*this);

            while (i2.next() && i2.lineY < clip.getBottom())
            {
                if (i2.lineY + i2.lineHeight >= clip.getY()
                      && underlinedSection.intersects ({ i2.indexInText, i2.indexInText + i2.atom->numChars }))
                {
                    i2.drawUnderline (g, underlinedSection, findColour (textColourId));
                }
            }
        }
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

        if (isMultiLine())
            g.drawText (textToShowWhenEmpty, getLocalBounds(),
                        Justification::centred, true);
        else
            g.drawText (textToShowWhenEmpty,
                        leftIndent, 0, viewport->getWidth() - leftIndent, getHeight(),
                        Justification::centredLeft, true);
    }

    getLookAndFeel().drawTextEditorOutline (g, getWidth(), getHeight(), *this);
}

//==============================================================================
void TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const bool writable = ! isReadOnly();

    if (passwordCharacter == 0)
    {
        m.addItem (StandardApplicationCommandIDs::cut,   TRANS("Cut"), writable);
        m.addItem (StandardApplicationCommandIDs::copy,  TRANS("Copy"), ! selection.isEmpty());
    }

    m.addItem (StandardApplicationCommandIDs::paste,     TRANS("Paste"), writable);
    m.addItem (StandardApplicationCommandIDs::del,       TRANS("Delete"), writable);
    m.addSeparator();
    m.addItem (StandardApplicationCommandIDs::selectAll, TRANS("Select All"));
    m.addSeparator();

    if (getUndoManager() != nullptr)
    {
        m.addItem (StandardApplicationCommandIDs::undo, TRANS("Undo"), undoManager.canUndo());
        m.addItem (StandardApplicationCommandIDs::redo, TRANS("Redo"), undoManager.canRedo());
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
            moveCaretTo (getTextIndexAt (e.x, e.y),
                         e.mods.isShiftDown());
        }
        else
        {
            PopupMenu m;
            m.setLookAndFeel (&getLookAndFeel());
            addPopupMenuItems (m, &e);

            menuActive = true;

            SafePointer<TextEditor> safeThis (this);

            m.showMenuAsync (PopupMenu::Options(),
                             [safeThis] (int menuResult)
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
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
            moveCaretTo (getTextIndexAt (e.x, e.y), true);
}

void TextEditor::mouseUp (const MouseEvent& e)
{
    newTransaction();
    textHolder->restartTimer();

    if (wasFocused || ! selectAllTextWhenFocused)
        if (e.mouseWasClicked() && ! (popupMenuEnabled && e.mods.isPopupMenu()))
            moveCaret (getTextIndexAt (e.x, e.y));

    wasFocused = true;
}

void TextEditor::mouseDoubleClick (const MouseEvent& e)
{
    int tokenEnd = getTextIndexAt (e.x, e.y);
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

bool TextEditor::moveCaretUp (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getY() - 1.0f), selecting);
}

bool TextEditor::moveCaretDown (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getBottom() + 1.0f), selecting);
}

bool TextEditor::pageUp (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToStartOfLine (selecting);

    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getY() - viewport->getViewHeight()), selecting);
}

bool TextEditor::pageDown (bool selecting)
{
    if (! isMultiLine())
        return moveCaretToEndOfLine (selecting);

    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition (caretPos.getX(), caretPos.getBottom() + viewport->getViewHeight()), selecting);
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
    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition (0.0f, caretPos.getY()), selecting);
}

bool TextEditor::moveCaretToEnd (bool selecting)
{
    return moveCaretWithTransaction (getTotalNumChars(), selecting);
}

bool TextEditor::moveCaretToEndOfLine (bool selecting)
{
    auto caretPos = getCaretRectangleFloat();
    return moveCaretWithTransaction (indexAtPosition ((float) textHolder->getWidth(), caretPos.getY()), selecting);
}

bool TextEditor::deleteBackwards (bool moveInWholeWordSteps)
{
    if (moveInWholeWordSteps)
        moveCaretTo (findWordBreakBefore (getCaretPosition()), true);
    else if (selection.isEmpty() && selection.getStart() > 0)
        selection = { selection.getEnd() - 1, selection.getEnd() };

    cut();
    return true;
}

bool TextEditor::deleteForwards (bool /*moveInWholeWordSteps*/)
{
    if (selection.isEmpty() && selection.getStart() < getTotalNumChars())
        selection = { selection.getStart(), selection.getStart() + 1 };

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
void TextEditor::focusGained (FocusChangeType)
{
    newTransaction();

    if (selectAllTextWhenFocused)
    {
        moveCaretTo (0, false);
        moveCaretTo (getTotalNumChars(), true);
    }

    repaint();
    updateCaretPosition();
}

void TextEditor::focusLost (FocusChangeType)
{
    newTransaction();

    wasFocused = false;
    textHolder->stopTimer();

    underlinedSections.clear();

    if (auto* peer = getPeer())
        peer->dismissPendingTextInput();

    updateCaretPosition();

    postCommandMessage (TextEditorDefs::focusLossMessageId);
    repaint();
}

//==============================================================================
void TextEditor::resized()
{
    viewport->setBoundsInset (borderSize);
    viewport->setSingleStepSizes (16, roundToInt (currentFont.getHeight()));

    updateTextHolderSize();

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

        if (! checker.shouldBailOut() && onTextChange != nullptr)
            onTextChange();

        break;

    case TextEditorDefs::returnKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorReturnKeyPressed (*this); });

        if (! checker.shouldBailOut() && onReturnKey != nullptr)
            onReturnKey();

        break;

    case TextEditorDefs::escapeKeyMessageId:
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorEscapeKeyPressed (*this); });

        if (! checker.shouldBailOut() && onEscapeKey != nullptr)
            onEscapeKey();

        break;

    case TextEditorDefs::focusLossMessageId:
        updateValueFromText();
        listeners.callChecked (checker, [this] (Listener& l) { l.textEditorFocusLost (*this); });

        if (! checker.shouldBailOut() && onFocusLost != nullptr)
            onFocusLost();

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

//==============================================================================
UndoManager* TextEditor::getUndoManager() noexcept
{
    return readOnly ? nullptr : &undoManager;
}

void TextEditor::clearInternal (UndoManager* const um)
{
    remove ({ 0, getTotalNumChars() }, um, caretPosition);
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
                                           caretPosition, caretPositionToMoveTo));
        }
        else
        {
            repaintText ({ insertIndex, getTotalNumChars() }); // must do this before and after changing the data, in case
                                                               // a line gets moved due to word wrap

            int index = 0;
            int nextIndex = 0;

            for (int i = 0; i < sections.size(); ++i)
            {
                nextIndex = index + sections.getUnchecked (i)->getTotalLength();

                if (insertIndex == index)
                {
                    sections.insert (i, new UniformTextSection (text, font, colour, passwordCharacter));
                    break;
                }

                if (insertIndex > index && insertIndex < nextIndex)
                {
                    splitSection (i, insertIndex - index);
                    sections.insert (i + 1, new UniformTextSection (text, font, colour, passwordCharacter));
                    break;
                }

                index = nextIndex;
            }

            if (nextIndex == insertIndex)
                sections.add (new UniformTextSection (text, font, colour, passwordCharacter));

            coalesceSimilarSections();
            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            updateTextHolderSize();
            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ insertIndex, getTotalNumChars() });
        }
    }
}

void TextEditor::reinsert (int insertIndex, const OwnedArray<UniformTextSection>& sectionsToInsert)
{
    int index = 0;
    int nextIndex = 0;

    for (int i = 0; i < sections.size(); ++i)
    {
        nextIndex = index + sections.getUnchecked (i)->getTotalLength();

        if (insertIndex == index)
        {
            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i, new UniformTextSection (*sectionsToInsert.getUnchecked(j)));

            break;
        }

        if (insertIndex > index && insertIndex < nextIndex)
        {
            splitSection (i, insertIndex - index);

            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i + 1, new UniformTextSection (*sectionsToInsert.getUnchecked(j)));

            break;
        }

        index = nextIndex;
    }

    if (nextIndex == insertIndex)
        for (auto* s : sectionsToInsert)
            sections.add (new UniformTextSection (*s));

    coalesceSimilarSections();
    totalNumChars = -1;
    valueTextNeedsUpdating = true;
}

void TextEditor::remove (Range<int> range, UndoManager* const um, const int caretPositionToMoveTo)
{
    if (! range.isEmpty())
    {
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            auto nextIndex = index + sections.getUnchecked(i)->getTotalLength();

            if (range.getStart() > index && range.getStart() < nextIndex)
            {
                splitSection (i, range.getStart() - index);
                --i;
            }
            else if (range.getEnd() > index && range.getEnd() < nextIndex)
            {
                splitSection (i, range.getEnd() - index);
                --i;
            }
            else
            {
                index = nextIndex;

                if (index > range.getEnd())
                    break;
            }
        }

        index = 0;

        if (um != nullptr)
        {
            Array<UniformTextSection*> removedSections;

            for (auto* section : sections)
            {
                if (range.getEnd() <= range.getStart())
                    break;

                auto nextIndex = index + section->getTotalLength();

                if (range.getStart() <= index && range.getEnd() >= nextIndex)
                    removedSections.add (new UniformTextSection (*section));

                index = nextIndex;
            }

            if (um->getNumActionsInCurrentTransaction() > TextEditorDefs::maxActionsPerTransaction)
                newTransaction();

            um->perform (new RemoveAction (*this, range, caretPosition,
                                           caretPositionToMoveTo, removedSections));
        }
        else
        {
            auto remainingRange = range;

            for (int i = 0; i < sections.size(); ++i)
            {
                auto* section = sections.getUnchecked (i);
                auto nextIndex = index + section->getTotalLength();

                if (remainingRange.getStart() <= index && remainingRange.getEnd() >= nextIndex)
                {
                    sections.remove (i);
                    remainingRange.setEnd (remainingRange.getEnd() - (nextIndex - index));

                    if (remainingRange.isEmpty())
                        break;

                    --i;
                }
                else
                {
                    index = nextIndex;
                }
            }

            coalesceSimilarSections();
            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            moveCaretTo (caretPositionToMoveTo, false);

            repaintText ({ range.getStart(), getTotalNumChars() });
        }
    }
}

//==============================================================================
String TextEditor::getText() const
{
    MemoryOutputStream mo;
    mo.preallocate ((size_t) getTotalNumChars());

    for (auto* s : sections)
        s->appendAllText (mo);

    return mo.toUTF8();
}

String TextEditor::getTextInRange (const Range<int>& range) const
{
    if (range.isEmpty())
        return {};

    MemoryOutputStream mo;
    mo.preallocate ((size_t) jmin (getTotalNumChars(), range.getLength()));

    int index = 0;

    for (auto* s : sections)
    {
        auto nextIndex = index + s->getTotalLength();

        if (range.getStart() < nextIndex)
        {
            if (range.getEnd() <= index)
                break;

            s->appendSubstring (mo, range - index);
        }

        index = nextIndex;
    }

    return mo.toUTF8();
}

String TextEditor::getHighlightedText() const
{
    return getTextInRange (selection);
}

int TextEditor::getTotalNumChars() const
{
    if (totalNumChars < 0)
    {
        totalNumChars = 0;

        for (auto* s : sections)
            totalNumChars += s->getTotalLength();
    }

    return totalNumChars;
}

bool TextEditor::isEmpty() const
{
    return getTotalNumChars() == 0;
}

void TextEditor::getCharPosition (int index, Point<float>& anchor, float& lineHeight) const
{
    if (getWordWrapWidth() <= 0)
    {
        anchor = {};
        lineHeight = currentFont.getHeight();
    }
    else
    {
        Iterator i (*this);

        if (sections.isEmpty())
        {
            anchor = { i.getJustificationOffset (0), 0 };
            lineHeight = currentFont.getHeight();
        }
        else
        {
            i.getCharPosition (index, anchor, lineHeight);
        }
    }
}

int TextEditor::indexAtPosition (const float x, const float y)
{
    if (getWordWrapWidth() > 0)
    {
        for (Iterator i (*this); i.next();)
        {
            if (y < i.lineY + i.lineHeight)
            {
                if (y < i.lineY)
                    return jmax (0, i.indexInText - 1);

                if (x <= i.atomX || i.atom->isNewLine())
                    return i.indexInText;

                if (x < i.atomRight)
                    return i.xToIndex (x);
            }
        }
    }

    return getTotalNumChars();
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
void TextEditor::splitSection (const int sectionIndex, const int charToSplitAt)
{
    jassert (sections[sectionIndex] != nullptr);

    sections.insert (sectionIndex + 1,
                     sections.getUnchecked (sectionIndex)->split (charToSplitAt, passwordCharacter));
}

void TextEditor::coalesceSimilarSections()
{
    for (int i = 0; i < sections.size() - 1; ++i)
    {
        auto* s1 = sections.getUnchecked (i);
        auto* s2 = sections.getUnchecked (i + 1);

        if (s1->font == s2->font
             && s1->colour == s2->colour)
        {
            s1->append (*s2, passwordCharacter);
            sections.remove (i + 1);
            --i;
        }
    }
}

} // namespace juce
