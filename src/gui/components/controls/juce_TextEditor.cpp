/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TextEditor.h"
#include "../../graphics/fonts/juce_GlyphArrangement.h"
#include "../../../utilities/juce_SystemClipboard.h"
#include "../../../core/juce_Time.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
// a word or space that can't be broken down any further
struct TextAtom
{
    //==============================================================================
    String atomText;
    float width;
    uint16 numChars;

    //==============================================================================
    bool isWhitespace() const       { return CharacterFunctions::isWhitespace (atomText[0]); }
    bool isNewLine() const          { return atomText[0] == T('\r') || atomText[0] == T('\n'); }

    const String getText (const tchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText;
        else
            return String::repeatedString (String::charToString (passwordCharacter),
                                           atomText.length());
    }

    const String getTrimmedText (const tchar passwordCharacter) const
    {
        if (passwordCharacter == 0)
            return atomText.substring (0, numChars);
        else if (isNewLine())
            return String::empty;
        else
            return String::repeatedString (String::charToString (passwordCharacter), numChars);
    }
};

//==============================================================================
// a run of text with a single font and colour
class UniformTextSection
{
public:
    //==============================================================================
    UniformTextSection (const String& text,
                        const Font& font_,
                        const Colour& colour_,
                        const tchar passwordCharacter)
      : font (font_),
        colour (colour_)
    {
        initialiseAtoms (text, passwordCharacter);
    }

    UniformTextSection (const UniformTextSection& other)
      : font (other.font),
        colour (other.colour)
    {
        atoms.ensureStorageAllocated (other.atoms.size());

        for (int i = 0; i < other.atoms.size(); ++i)
            atoms.add (new TextAtom (*(const TextAtom*) other.atoms.getUnchecked(i)));
    }

    ~UniformTextSection()
    {
        // (no need to delete the atoms, as they're explicitly deleted by the caller)
    }

    void clear()
    {
        for (int i = atoms.size(); --i >= 0;)
            delete getAtom(i);

        atoms.clear();
    }

    int getNumAtoms() const
    {
        return atoms.size();
    }

    TextAtom* getAtom (const int index) const
    {
        return (TextAtom*) atoms.getUnchecked (index);
    }

    void append (const UniformTextSection& other, const tchar passwordCharacter)
    {
        if (other.atoms.size() > 0)
        {
            TextAtom* const lastAtom = (TextAtom*) atoms.getLast();
            int i = 0;

            if (lastAtom != 0)
            {
                if (! CharacterFunctions::isWhitespace (lastAtom->atomText.getLastCharacter()))
                {
                    TextAtom* const first = other.getAtom(0);

                    if (! CharacterFunctions::isWhitespace (first->atomText[0]))
                    {
                        lastAtom->atomText += first->atomText;
                        lastAtom->numChars = (uint16) (lastAtom->numChars + first->numChars);
                        lastAtom->width = font.getStringWidthFloat (lastAtom->getText (passwordCharacter));
                        delete first;
                        ++i;
                    }
                }
            }

            atoms.ensureStorageAllocated (atoms.size() + other.atoms.size() - i);

            while (i < other.atoms.size())
            {
                atoms.add (other.getAtom(i));
                ++i;
            }
        }
    }

    UniformTextSection* split (const int indexToBreakAt,
                               const tchar passwordCharacter)
    {
        UniformTextSection* const section2 = new UniformTextSection (String::empty,
                                                                     font, colour,
                                                                     passwordCharacter);
        int index = 0;

        for (int i = 0; i < atoms.size(); ++i)
        {
            TextAtom* const atom = getAtom(i);

            const int nextIndex = index + atom->numChars;

            if (index == indexToBreakAt)
            {
                int j;
                for (j = i; j < atoms.size(); ++j)
                    section2->atoms.add (getAtom (j));

                for (j = atoms.size(); --j >= i;)
                    atoms.remove (j);

                break;
            }
            else if (indexToBreakAt >= index && indexToBreakAt < nextIndex)
            {
                TextAtom* const secondAtom = new TextAtom();

                secondAtom->atomText = atom->atomText.substring (indexToBreakAt - index);
                secondAtom->width = font.getStringWidthFloat (secondAtom->getText (passwordCharacter));
                secondAtom->numChars = (uint16) secondAtom->atomText.length();

                section2->atoms.add (secondAtom);

                atom->atomText = atom->atomText.substring (0, indexToBreakAt - index);
                atom->width = font.getStringWidthFloat (atom->getText (passwordCharacter));
                atom->numChars = (uint16) (indexToBreakAt - index);

                int j;
                for (j = i + 1; j < atoms.size(); ++j)
                    section2->atoms.add (getAtom (j));

                for (j = atoms.size(); --j > i;)
                    atoms.remove (j);

                break;
            }

            index = nextIndex;
        }

        return section2;
    }

    void appendAllText (String::Concatenator& concatenator) const
    {
        for (int i = 0; i < atoms.size(); ++i)
            concatenator.append (getAtom(i)->atomText);
    }

    void appendSubstring (String::Concatenator& concatenator,
                          const int startCharacter,
                          const int endCharacter) const
    {
        int index = 0;
        for (int i = 0; i < atoms.size(); ++i)
        {
            const TextAtom* const atom = getAtom (i);
            const int nextIndex = index + atom->numChars;

            if (startCharacter < nextIndex)
            {
                if (endCharacter <= index)
                    break;

                const int start = jmax (0, startCharacter - index);
                const int end = jmin (endCharacter - index, (int) atom->numChars);

                if (start < end)
                    concatenator.append (atom->atomText.substring (start, end));
            }

            index = nextIndex;
        }
    }

    int getTotalLength() const
    {
        int total = 0;

        for (int i = atoms.size(); --i >= 0;)
            total += getAtom(i)->numChars;

        return total;
    }

    void setFont (const Font& newFont,
                  const tchar passwordCharacter)
    {
        if (font != newFont)
        {
            font = newFont;

            for (int i = atoms.size(); --i >= 0;)
            {
                TextAtom* const atom = (TextAtom*) atoms.getUnchecked(i);
                atom->width = newFont.getStringWidthFloat (atom->getText (passwordCharacter));
            }
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    Font font;
    Colour colour;

private:
    VoidArray atoms;

    //==============================================================================
    void initialiseAtoms (const String& textToParse,
                          const tchar passwordCharacter)
    {
        int i = 0;
        const int len = textToParse.length();
        const tchar* const text = (const tchar*) textToParse;

        while (i < len)
        {
            int start = i;

            // create a whitespace atom unless it starts with non-ws
            if (CharacterFunctions::isWhitespace (text[i])
                 && text[i] != T('\r')
                 && text[i] != T('\n'))
            {
                while (i < len
                        && CharacterFunctions::isWhitespace (text[i])
                        && text[i] != T('\r')
                        && text[i] != T('\n'))
                {
                    ++i;
                }
            }
            else
            {
                if (text[i] == T('\r'))
                {
                    ++i;

                    if ((i < len) && (text[i] == T('\n')))
                    {
                        ++start;
                        ++i;
                    }
                }
                else if (text[i] == T('\n'))
                {
                    ++i;
                }
                else
                {
                    while ((i < len) && ! CharacterFunctions::isWhitespace (text[i]))
                        ++i;
                }
            }

            TextAtom* const atom = new TextAtom();
            atom->atomText = String (text + start, i - start);

            atom->width = font.getStringWidthFloat (atom->getText (passwordCharacter));
            atom->numChars = (uint16) (i - start);

            atoms.add (atom);
        }
    }

    const UniformTextSection& operator= (const UniformTextSection& other);
};

//==============================================================================
class TextEditorIterator
{
public:
    //==============================================================================
    TextEditorIterator (const VoidArray& sections_,
                        const float wordWrapWidth_,
                        const tchar passwordCharacter_)
      : indexInText (0),
        lineY (0),
        lineHeight (0),
        maxDescent (0),
        atomX (0),
        atomRight (0),
        atom (0),
        currentSection (0),
        sections (sections_),
        sectionIndex (0),
        atomIndex (0),
        wordWrapWidth (wordWrapWidth_),
        passwordCharacter (passwordCharacter_)
    {
        jassert (wordWrapWidth_ > 0);

        if (sections.size() > 0)
        {
            currentSection = (const UniformTextSection*) sections.getUnchecked (sectionIndex);

            if (currentSection != 0)
                beginNewLine();
        }
    }

    TextEditorIterator (const TextEditorIterator& other)
      : indexInText (other.indexInText),
        lineY (other.lineY),
        lineHeight (other.lineHeight),
        maxDescent (other.maxDescent),
        atomX (other.atomX),
        atomRight (other.atomRight),
        atom (other.atom),
        currentSection (other.currentSection),
        sections (other.sections),
        sectionIndex (other.sectionIndex),
        atomIndex (other.atomIndex),
        wordWrapWidth (other.wordWrapWidth),
        passwordCharacter (other.passwordCharacter),
        tempAtom (other.tempAtom)
    {
    }

    ~TextEditorIterator()
    {
    }

    //==============================================================================
    bool next()
    {
        if (atom == &tempAtom)
        {
            const int numRemaining = tempAtom.atomText.length() - tempAtom.numChars;

            if (numRemaining > 0)
            {
                tempAtom.atomText = tempAtom.atomText.substring (tempAtom.numChars);

                atomX = 0;

                if (tempAtom.numChars > 0)
                    lineY += lineHeight;

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
                    atomRight = atomX + tempAtom.width;
                    return true;
                }
            }
        }

        bool forceNewLine = false;

        if (sectionIndex >= sections.size())
        {
            moveToEndOfLastAtom();
            return false;
        }
        else if (atomIndex >= currentSection->getNumAtoms() - 1)
        {
            if (atomIndex >= currentSection->getNumAtoms())
            {
                if (++sectionIndex >= sections.size())
                {
                    moveToEndOfLastAtom();
                    return false;
                }

                atomIndex = 0;
                currentSection = (const UniformTextSection*) sections.getUnchecked (sectionIndex);
            }
            else
            {
                const TextAtom* const lastAtom = currentSection->getAtom (atomIndex);

                if (! lastAtom->isWhitespace())
                {
                    // handle the case where the last atom in a section is actually part of the same
                    // word as the first atom of the next section...
                    float right = atomRight + lastAtom->width;
                    float lineHeight2 = lineHeight;
                    float maxDescent2 = maxDescent;

                    for (int section = sectionIndex + 1; section < sections.size(); ++section)
                    {
                        const UniformTextSection* const s = (const UniformTextSection*) sections.getUnchecked (section);

                        if (s->getNumAtoms() == 0)
                            break;

                        const TextAtom* const nextAtom = s->getAtom (0);

                        if (nextAtom->isWhitespace())
                            break;

                        right += nextAtom->width;

                        lineHeight2 = jmax (lineHeight2, s->font.getHeight());
                        maxDescent2 = jmax (maxDescent2, s->font.getDescent());

                        if (shouldWrap (right))
                        {
                            lineHeight = lineHeight2;
                            maxDescent = maxDescent2;

                            forceNewLine = true;
                            break;
                        }

                        if (s->getNumAtoms() > 1)
                            break;
                    }
                }
            }
        }

        if (atom != 0)
        {
            atomX = atomRight;
            indexInText += atom->numChars;

            if (atom->isNewLine())
                beginNewLine();
        }

        atom = currentSection->getAtom (atomIndex);
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
                atomRight = atom->width;

                if (shouldWrap (atomRight))  // atom too big to fit on a line, so break it up..
                {
                    tempAtom = *atom;
                    tempAtom.width = 0;
                    tempAtom.numChars = 0;
                    atom = &tempAtom;

                    if (atomX > 0)
                        beginNewLine();

                    return next();
                }

                beginNewLine();
                return true;
            }
        }

        return true;
    }

    void beginNewLine()
    {
        atomX = 0;
        lineY += lineHeight;

        int tempSectionIndex = sectionIndex;
        int tempAtomIndex = atomIndex;
        const UniformTextSection* section = (const UniformTextSection*) sections.getUnchecked (tempSectionIndex);

        lineHeight = section->font.getHeight();
        maxDescent = section->font.getDescent();

        float x = (atom != 0) ? atom->width : 0;

        while (! shouldWrap (x))
        {
            if (tempSectionIndex >= sections.size())
                break;

            bool checkSize = false;

            if (tempAtomIndex >= section->getNumAtoms())
            {
                if (++tempSectionIndex >= sections.size())
                    break;

                tempAtomIndex = 0;
                section = (const UniformTextSection*) sections.getUnchecked (tempSectionIndex);
                checkSize = true;
            }

            const TextAtom* const nextAtom = section->getAtom (tempAtomIndex);

            if (nextAtom == 0)
                break;

            x += nextAtom->width;

            if (shouldWrap (x) || nextAtom->isNewLine())
                break;

            if (checkSize)
            {
                lineHeight = jmax (lineHeight, section->font.getHeight());
                maxDescent = jmax (maxDescent, section->font.getDescent());
            }

            ++tempAtomIndex;
        }
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
                              atomX,
                              (float) roundToInt (lineY + lineHeight - maxDescent));
            ga.draw (g);
        }
    }

    void drawSelection (Graphics& g,
                        const int selectionStart,
                        const int selectionEnd) const
    {
        const int startX = roundToInt (indexToX (selectionStart));
        const int endX   = roundToInt (indexToX (selectionEnd));

        const int y = roundToInt (lineY);
        const int nextY = roundToInt (lineY + lineHeight);

        g.fillRect (startX, y, endX - startX, nextY - y);
    }

    void drawSelectedText (Graphics& g,
                           const int selectionStart,
                           const int selectionEnd,
                           const Colour& selectedTextColour) const
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX,
                              (float) roundToInt (lineY + lineHeight - maxDescent));

            if (selectionEnd < indexInText + atom->numChars)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (0, selectionEnd - indexInText);
                ga.removeRangeOfGlyphs (selectionEnd - indexInText, -1);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            if (selectionStart > indexInText)
            {
                GlyphArrangement ga2 (ga);
                ga2.removeRangeOfGlyphs (selectionStart - indexInText, -1);
                ga.removeRangeOfGlyphs (0, selectionStart - indexInText);

                g.setColour (currentSection->colour);
                ga2.draw (g);
            }

            g.setColour (selectedTextColour);
            ga.draw (g);
        }
    }

    //==============================================================================
    float indexToX (const int indexToFind) const
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

    int xToIndex (const float xToFind) const
    {
        if (xToFind <= atomX || atom->isNewLine())
            return indexInText;

        if (xToFind >= atomRight)
            return indexInText + atom->numChars;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        int j;
        for (j = 0; j < g.getNumGlyphs(); ++j)
            if ((g.getGlyph(j).getLeft() + g.getGlyph(j).getRight()) / 2 > xToFind)
                break;

        return indexInText + j;
    }

    //==============================================================================
    bool getCharPosition (const int index, float& cx, float& cy, float& lineHeight_)
    {
        while (next())
        {
            if (indexInText + atom->numChars > index)
            {
                cx = indexToX (index);
                cy = lineY;
                lineHeight_ = lineHeight;
                return true;
            }
        }

        cx = atomX;
        cy = lineY;
        lineHeight_ = lineHeight;
        return false;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    int indexInText;
    float lineY, lineHeight, maxDescent;
    float atomX, atomRight;
    const TextAtom* atom;
    const UniformTextSection* currentSection;

private:
    const VoidArray& sections;
    int sectionIndex, atomIndex;
    const float wordWrapWidth;
    const tchar passwordCharacter;
    TextAtom tempAtom;

    const TextEditorIterator& operator= (const TextEditorIterator&);

    void moveToEndOfLastAtom()
    {
        if (atom != 0)
        {
            atomX = atomRight;

            if (atom->isNewLine())
            {
                atomX = 0.0f;
                lineY += lineHeight;
            }
        }
    }

    bool shouldWrap (const float x) const
    {
        return (x - 0.0001f) >= wordWrapWidth;
    }
};


//==============================================================================
class TextEditorInsertAction  : public UndoableAction
{
    TextEditor& owner;
    const String text;
    const int insertIndex, oldCaretPos, newCaretPos;
    const Font font;
    const Colour colour;

    TextEditorInsertAction (const TextEditorInsertAction&);
    const TextEditorInsertAction& operator= (const TextEditorInsertAction&);

public:
    TextEditorInsertAction (TextEditor& owner_,
                            const String& text_,
                            const int insertIndex_,
                            const Font& font_,
                            const Colour& colour_,
                            const int oldCaretPos_,
                            const int newCaretPos_)
        : owner (owner_),
          text (text_),
          insertIndex (insertIndex_),
          oldCaretPos (oldCaretPos_),
          newCaretPos (newCaretPos_),
          font (font_),
          colour (colour_)
    {
    }

    ~TextEditorInsertAction()
    {
    }

    bool perform()
    {
        owner.insert (text, insertIndex, font, colour, 0, newCaretPos);
        return true;
    }

    bool undo()
    {
        owner.remove (insertIndex, insertIndex + text.length(), 0, oldCaretPos);
        return true;
    }

    int getSizeInUnits()
    {
        return text.length() + 16;
    }
};

//==============================================================================
class TextEditorRemoveAction  : public UndoableAction
{
    TextEditor& owner;
    const int startIndex, endIndex, oldCaretPos, newCaretPos;
    VoidArray removedSections;

    TextEditorRemoveAction (const TextEditorRemoveAction&);
    const TextEditorRemoveAction& operator= (const TextEditorRemoveAction&);

public:
    TextEditorRemoveAction (TextEditor& owner_,
                            const int startIndex_,
                            const int endIndex_,
                            const int oldCaretPos_,
                            const int newCaretPos_,
                            const VoidArray& removedSections_)
        : owner (owner_),
          startIndex (startIndex_),
          endIndex (endIndex_),
          oldCaretPos (oldCaretPos_),
          newCaretPos (newCaretPos_),
          removedSections (removedSections_)
    {
    }

    ~TextEditorRemoveAction()
    {
        for (int i = removedSections.size(); --i >= 0;)
        {
            UniformTextSection* const section = (UniformTextSection*) removedSections.getUnchecked (i);
            section->clear();
            delete section;
        }
    }

    bool perform()
    {
        owner.remove (startIndex, endIndex, 0, newCaretPos);
        return true;
    }

    bool undo()
    {
        owner.reinsert (startIndex, removedSections);
        owner.moveCursorTo (oldCaretPos, false);
        return true;
    }

    int getSizeInUnits()
    {
        int n = 0;

        for (int i = removedSections.size(); --i >= 0;)
        {
            UniformTextSection* const section = (UniformTextSection*) removedSections.getUnchecked (i);
            n += section->getTotalLength();
        }

        return n + 16;
    }
};

//==============================================================================
class TextHolderComponent  : public Component,
                             public Timer,
                             public Value::Listener
{
    TextEditor& owner;

    TextHolderComponent (const TextHolderComponent&);
    const TextHolderComponent& operator= (const TextHolderComponent&);

public:
    TextHolderComponent (TextEditor& owner_)
        : owner (owner_)
    {
        setWantsKeyboardFocus (false);
        setInterceptsMouseClicks (false, true);

        owner.getTextValue().addListener (this);
    }

    ~TextHolderComponent()
    {
        owner.getTextValue().removeListener (this);
    }

    void paint (Graphics& g)
    {
        owner.drawContent (g);
    }

    void timerCallback()
    {
        owner.timerCallbackInt();
    }

    const MouseCursor getMouseCursor()
    {
        return owner.getMouseCursor();
    }

    void valueChanged (Value&)
    {
        owner.textWasChangedByValue();
    }
};

//==============================================================================
class TextEditorViewport  : public Viewport
{
    TextEditor* const owner;
    float lastWordWrapWidth;

    TextEditorViewport (const TextEditorViewport&);
    const TextEditorViewport& operator= (const TextEditorViewport&);

public:
    TextEditorViewport (TextEditor* const owner_)
        : owner (owner_),
          lastWordWrapWidth (0)
    {
    }

    ~TextEditorViewport()
    {
    }

    void visibleAreaChanged (int, int, int, int)
    {
        const float wordWrapWidth = owner->getWordWrapWidth();

        if (wordWrapWidth != lastWordWrapWidth)
        {
            lastWordWrapWidth = wordWrapWidth;
            owner->updateTextHolderSize();
        }
    }
};

//==============================================================================
const int flashSpeedIntervalMs = 380;

const int textChangeMessageId = 0x10003001;
const int returnKeyMessageId  = 0x10003002;
const int escapeKeyMessageId  = 0x10003003;
const int focusLossMessageId  = 0x10003004;


//==============================================================================
TextEditor::TextEditor (const String& name,
                        const tchar passwordCharacter_)
    : Component (name),
      borderSize (1, 1, 1, 3),
      readOnly (false),
      multiline (false),
      wordWrap (false),
      returnKeyStartsNewLine (false),
      caretVisible (true),
      popupMenuEnabled (true),
      selectAllTextWhenFocused (false),
      scrollbarVisible (true),
      wasFocused (false),
      caretFlashState (true),
      keepCursorOnScreen (true),
      tabKeyUsed (false),
      menuActive (false),
      valueTextNeedsUpdating (false),
      cursorX (0),
      cursorY (0),
      cursorHeight (0),
      maxTextLength (0),
      selectionStart (0),
      selectionEnd (0),
      leftIndent (4),
      topIndent (4),
      lastTransactionTime (0),
      currentFont (14.0f),
      totalNumChars (0),
      caretPosition (0),
      passwordCharacter (passwordCharacter_),
      dragType (notDragging)
{
    setOpaque (true);

    addAndMakeVisible (viewport = new TextEditorViewport (this));
    viewport->setViewedComponent (textHolder = new TextHolderComponent (*this));
    viewport->setWantsKeyboardFocus (false);
    viewport->setScrollBarsShown (false, false);

    setMouseCursor (MouseCursor::IBeamCursor);
    setWantsKeyboardFocus (true);
}

TextEditor::~TextEditor()
{
    textValue.referTo (Value());
    clearInternal (0);
    viewport = 0;
    textHolder = 0;
}

//==============================================================================
void TextEditor::newTransaction()
{
    lastTransactionTime = Time::getApproximateMillisecondCounter();
    undoManager.beginNewTransaction();
}

void TextEditor::doUndoRedo (const bool isRedo)
{
    if (! isReadOnly())
    {
        if ((isRedo) ? undoManager.redo()
                     : undoManager.undo())
        {
            scrollToMakeSureCursorIsVisible();
            repaint();
            textChanged();
        }
    }
}

//==============================================================================
void TextEditor::setMultiLine (const bool shouldBeMultiLine,
                               const bool shouldWordWrap)
{
    multiline = shouldBeMultiLine;
    wordWrap = shouldWordWrap && shouldBeMultiLine;

    setScrollbarsShown (scrollbarVisible);

    viewport->setViewPosition (0, 0);

    resized();
    scrollToMakeSureCursorIsVisible();
}

bool TextEditor::isMultiLine() const
{
    return multiline;
}

void TextEditor::setScrollbarsShown (bool enabled)
{
    scrollbarVisible = enabled;

    enabled = enabled && isMultiLine();

    viewport->setScrollBarsShown (enabled, enabled);
}

void TextEditor::setReadOnly (const bool shouldBeReadOnly)
{
    readOnly = shouldBeReadOnly;
    enablementChanged();
}

bool TextEditor::isReadOnly() const
{
    return readOnly || ! isEnabled();
}

void TextEditor::setReturnKeyStartsNewLine (const bool shouldStartNewLine)
{
    returnKeyStartsNewLine = shouldStartNewLine;
}

void TextEditor::setTabKeyUsedAsCharacter (const bool shouldTabKeyBeUsed)
{
    tabKeyUsed = shouldTabKeyBeUsed;
}

void TextEditor::setPopupMenuEnabled (const bool b)
{
    popupMenuEnabled = b;
}

void TextEditor::setSelectAllWhenFocused (const bool b)
{
    selectAllTextWhenFocused = b;
}

//==============================================================================
const Font TextEditor::getFont() const
{
    return currentFont;
}

void TextEditor::setFont (const Font& newFont)
{
    currentFont = newFont;
    scrollToMakeSureCursorIsVisible();
}

void TextEditor::applyFontToAllText (const Font& newFont)
{
    currentFont = newFont;

    const Colour overallColour (findColour (textColourId));

    for (int i = sections.size(); --i >= 0;)
    {
        UniformTextSection* const uts = (UniformTextSection*) sections.getUnchecked(i);
        uts->setFont (newFont, passwordCharacter);
        uts->colour = overallColour;
    }

    coalesceSimilarSections();
    updateTextHolderSize();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

void TextEditor::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    repaint();
}

void TextEditor::setCaretVisible (const bool shouldCaretBeVisible)
{
    caretVisible = shouldCaretBeVisible;

    if (shouldCaretBeVisible)
        textHolder->startTimer (flashSpeedIntervalMs);

    setMouseCursor (shouldCaretBeVisible ? MouseCursor::IBeamCursor
                                         : MouseCursor::NormalCursor);
}

void TextEditor::setInputRestrictions (const int maxLen,
                                       const String& chars)
{
    maxTextLength = jmax (0, maxLen);
    allowedCharacters = chars;
}

void TextEditor::setTextToShowWhenEmpty (const String& text, const Colour& colourToUse)
{
    textToShowWhenEmpty = text;
    colourForTextWhenEmpty = colourToUse;
}

void TextEditor::setPasswordCharacter (const tchar newPasswordCharacter)
{
    if (passwordCharacter != newPasswordCharacter)
    {
        passwordCharacter = newPasswordCharacter;
        resized();
        repaint();
    }
}

void TextEditor::setScrollBarThickness (const int newThicknessPixels)
{
    viewport->setScrollBarThickness (newThicknessPixels);
}

void TextEditor::setScrollBarButtonVisibility (const bool buttonsVisible)
{
    viewport->setScrollBarButtonVisibility (buttonsVisible);
}

//==============================================================================
void TextEditor::clear()
{
    clearInternal (0);
    updateTextHolderSize();
    undoManager.clearUndoHistory();
}

void TextEditor::setText (const String& newText,
                          const bool sendTextChangeMessage)
{
    const int newLength = newText.length();

    if (newLength != getTotalNumChars() || getText() != newText)
    {
        const int oldCursorPos = caretPosition;
        const bool cursorWasAtEnd = oldCursorPos >= getTotalNumChars();

        clearInternal (0);
        insert (newText, 0, currentFont, findColour (textColourId), 0, caretPosition);

        // if you're adding text with line-feeds to a single-line text editor, it
        // ain't gonna look right!
        jassert (multiline || ! newText.containsAnyOf (T("\r\n")));

        if (cursorWasAtEnd && ! isMultiLine())
            moveCursorTo (getTotalNumChars(), false);
        else
            moveCursorTo (oldCursorPos, false);

        if (sendTextChangeMessage)
            textChanged();

        repaint();
    }

    updateTextHolderSize();
    scrollToMakeSureCursorIsVisible();
    undoManager.clearUndoHistory();
}

//==============================================================================
Value& TextEditor::getTextValue()
{
    if (valueTextNeedsUpdating)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }

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
    postCommandMessage (textChangeMessageId);

    if (textValue.getValueSource().getReferenceCount() > 1)
    {
        valueTextNeedsUpdating = false;
        textValue = getText();
    }
}

void TextEditor::returnPressed()
{
    postCommandMessage (returnKeyMessageId);
}

void TextEditor::escapePressed()
{
    postCommandMessage (escapeKeyMessageId);
}

void TextEditor::addListener (TextEditorListener* const newListener)
{
    jassert (newListener != 0)

    if (newListener != 0)
        listeners.add (newListener);
}

void TextEditor::removeListener (TextEditorListener* const listenerToRemove)
{
    listeners.removeValue (listenerToRemove);
}

//==============================================================================
void TextEditor::timerCallbackInt()
{
    const bool newState = (! caretFlashState) && ! isCurrentlyBlockedByAnotherModalComponent();

    if (caretFlashState != newState)
    {
        caretFlashState = newState;

        if (caretFlashState)
            wasFocused = true;

        if (caretVisible
             && hasKeyboardFocus (false)
             && ! isReadOnly())
        {
            repaintCaret();
        }
    }

    const unsigned int now = Time::getApproximateMillisecondCounter();

    if (now > lastTransactionTime + 200)
        newTransaction();
}

void TextEditor::repaintCaret()
{
    if (! findColour (caretColourId).isTransparent())
        repaint (borderSize.getLeft() + textHolder->getX() + leftIndent + roundToInt (cursorX) - 1,
                 borderSize.getTop() + textHolder->getY() + topIndent + roundToInt (cursorY) - 1,
                 4,
                 roundToInt (cursorHeight) + 2);
}

void TextEditor::repaintText (int textStartIndex, int textEndIndex)
{
    if (textStartIndex > textEndIndex && textEndIndex > 0)
        swapVariables (textStartIndex, textEndIndex);

    float x = 0, y = 0, lh = currentFont.getHeight();

    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        i.getCharPosition (textStartIndex, x, y, lh);

        const int y1 = (int) y;
        int y2;

        if (textEndIndex >= 0)
        {
            i.getCharPosition (textEndIndex, x, y, lh);
            y2 = (int) (y + lh * 2.0f);
        }
        else
        {
            y2 = textHolder->getHeight();
        }

        textHolder->repaint (0, y1, textHolder->getWidth(), y2 - y1);
    }
}

//==============================================================================
void TextEditor::moveCaret (int newCaretPos)
{
    if (newCaretPos < 0)
        newCaretPos = 0;
    else if (newCaretPos > getTotalNumChars())
        newCaretPos = getTotalNumChars();

    if (newCaretPos != getCaretPosition())
    {
        repaintCaret();
        caretFlashState = true;
        caretPosition = newCaretPos;
        textHolder->startTimer (flashSpeedIntervalMs);
        scrollToMakeSureCursorIsVisible();
        repaintCaret();
    }
}

void TextEditor::setCaretPosition (const int newIndex)
{
    moveCursorTo (newIndex, false);
}

int TextEditor::getCaretPosition() const
{
    return caretPosition;
}

void TextEditor::scrollEditorToPositionCaret (const int desiredCaretX,
                                              const int desiredCaretY)

{
    updateCaretPosition();

    int vx = roundToInt (cursorX) - desiredCaretX;
    int vy = roundToInt (cursorY) - desiredCaretY;

    if (desiredCaretX < jmax (1, proportionOfWidth (0.05f)))
    {
        vx += desiredCaretX - proportionOfWidth (0.2f);
    }
    else if (desiredCaretX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
    {
        vx += desiredCaretX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
    }

    vx = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), vx);

    if (! isMultiLine())
    {
        vy = viewport->getViewPositionY();
    }
    else
    {
        vy = jlimit (0, jmax (0, textHolder->getHeight() - viewport->getMaximumVisibleHeight()), vy);

        const int curH = roundToInt (cursorHeight);

        if (desiredCaretY < 0)
        {
            vy = jmax (0, desiredCaretY + vy);
        }
        else if (desiredCaretY > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - curH))
        {
            vy += desiredCaretY + 2 + curH + topIndent - viewport->getMaximumVisibleHeight();
        }
    }

    viewport->setViewPosition (vx, vy);
}

const Rectangle TextEditor::getCaretRectangle()
{
    updateCaretPosition();

    return Rectangle (roundToInt (cursorX) - viewport->getX(),
                      roundToInt (cursorY) - viewport->getY(),
                      1, roundToInt (cursorHeight));
}

//==============================================================================
float TextEditor::getWordWrapWidth() const
{
    return (wordWrap) ? (float) (viewport->getMaximumVisibleWidth() - leftIndent - leftIndent / 2)
                      : 1.0e10f;
}

void TextEditor::updateTextHolderSize()
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        float maxWidth = 0.0f;

        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.next())
            maxWidth = jmax (maxWidth, i.atomRight);

        const int w = leftIndent + roundToInt (maxWidth);
        const int h = topIndent + roundToInt (jmax (i.lineY + i.lineHeight,
                                                    currentFont.getHeight()));

        textHolder->setSize (w + 1, h + 1);
    }
}

int TextEditor::getTextWidth() const
{
    return textHolder->getWidth();
}

int TextEditor::getTextHeight() const
{
    return textHolder->getHeight();
}

void TextEditor::setIndents (const int newLeftIndent,
                             const int newTopIndent)
{
    leftIndent = newLeftIndent;
    topIndent = newTopIndent;
}

void TextEditor::setBorder (const BorderSize& border)
{
    borderSize = border;
    resized();
}

const BorderSize TextEditor::getBorder() const
{
    return borderSize;
}

void TextEditor::setScrollToShowCursor (const bool shouldScrollToShowCursor)
{
    keepCursorOnScreen = shouldScrollToShowCursor;
}

void TextEditor::updateCaretPosition()
{
    cursorHeight = currentFont.getHeight(); // (in case the text is empty and the call below doesn't set this value)
    getCharPosition (caretPosition, cursorX, cursorY, cursorHeight);
}

void TextEditor::scrollToMakeSureCursorIsVisible()
{
    updateCaretPosition();

    if (keepCursorOnScreen)
    {
        int x = viewport->getViewPositionX();
        int y = viewport->getViewPositionY();

        const int relativeCursorX = roundToInt (cursorX) - x;
        const int relativeCursorY = roundToInt (cursorY) - y;

        if (relativeCursorX < jmax (1, proportionOfWidth (0.05f)))
        {
            x += relativeCursorX - proportionOfWidth (0.2f);
        }
        else if (relativeCursorX > jmax (0, viewport->getMaximumVisibleWidth() - (wordWrap ? 2 : 10)))
        {
            x += relativeCursorX + (isMultiLine() ? proportionOfWidth (0.2f) : 10) - viewport->getMaximumVisibleWidth();
        }

        x = jlimit (0, jmax (0, textHolder->getWidth() + 8 - viewport->getMaximumVisibleWidth()), x);

        if (! isMultiLine())
        {
            y = (getHeight() - textHolder->getHeight() - topIndent) / -2;
        }
        else
        {
            const int curH = roundToInt (cursorHeight);

            if (relativeCursorY < 0)
            {
                y = jmax (0, relativeCursorY + y);
            }
            else if (relativeCursorY > jmax (0, viewport->getMaximumVisibleHeight() - topIndent - curH))
            {
                y += relativeCursorY + 2 + curH + topIndent - viewport->getMaximumVisibleHeight();
            }
        }

        viewport->setViewPosition (x, y);
    }
}

void TextEditor::moveCursorTo (const int newPosition,
                               const bool isSelecting)
{
    if (isSelecting)
    {
        moveCaret (newPosition);

        const int oldSelStart = selectionStart;
        const int oldSelEnd = selectionEnd;

        if (dragType == notDragging)
        {
            if (abs (getCaretPosition() - selectionStart) < abs (getCaretPosition() - selectionEnd))
                dragType = draggingSelectionStart;
            else
                dragType = draggingSelectionEnd;
        }

        if (dragType == draggingSelectionStart)
        {
            selectionStart = getCaretPosition();

            if (selectionEnd < selectionStart)
            {
                swapVariables (selectionStart, selectionEnd);
                dragType = draggingSelectionEnd;
            }
        }
        else
        {
            selectionEnd = getCaretPosition();

            if (selectionEnd < selectionStart)
            {
                swapVariables (selectionStart, selectionEnd);
                dragType = draggingSelectionStart;
            }
        }

        jassert (selectionStart <= selectionEnd);
        jassert (oldSelStart <= oldSelEnd);

        repaintText (jmin (oldSelStart, selectionStart),
                     jmax (oldSelEnd, selectionEnd));
    }
    else
    {
        dragType = notDragging;

        if (selectionEnd > selectionStart)
            repaintText (selectionStart, selectionEnd);

        moveCaret (newPosition);
        selectionStart = getCaretPosition();
        selectionEnd = getCaretPosition();
    }
}

int TextEditor::getTextIndexAt (const int x,
                                const int y)
{
    return indexAtPosition ((float) (x + viewport->getViewPositionX() - leftIndent),
                            (float) (y + viewport->getViewPositionY() - topIndent));
}

void TextEditor::insertTextAtCursor (String newText)
{
    if (allowedCharacters.isNotEmpty())
        newText = newText.retainCharacters (allowedCharacters);

    if (! isMultiLine())
        newText = newText.replaceCharacters (T("\r\n"), T("  "));
    else
        newText = newText.replace (T("\r\n"), T("\n"));

    const int newCaretPos = selectionStart + newText.length();
    const int insertIndex = selectionStart;

    remove (selectionStart, selectionEnd,
            &undoManager,
            newText.isNotEmpty() ? newCaretPos - 1 : newCaretPos);

    if (maxTextLength > 0)
        newText = newText.substring (0, maxTextLength - getTotalNumChars());

    if (newText.isNotEmpty())
        insert (newText,
                insertIndex,
                currentFont,
                findColour (textColourId),
                &undoManager,
                newCaretPos);

    textChanged();
}

void TextEditor::setHighlightedRegion (int startPos, int numChars)
{
    moveCursorTo (startPos, false);
    moveCursorTo (startPos + numChars, true);
}

//==============================================================================
void TextEditor::copy()
{
    if (passwordCharacter == 0)
    {
        const String selection (getTextSubstring (selectionStart, selectionEnd));

        if (selection.isNotEmpty())
            SystemClipboard::copyTextToClipboard (selection);
    }
}

void TextEditor::paste()
{
    if (! isReadOnly())
    {
        const String clip (SystemClipboard::getTextFromClipboard());

        if (clip.isNotEmpty())
            insertTextAtCursor (clip);
    }
}

void TextEditor::cut()
{
    if (! isReadOnly())
    {
        moveCaret (selectionEnd);
        insertTextAtCursor (String::empty);
    }
}

//==============================================================================
void TextEditor::drawContent (Graphics& g)
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        g.setOrigin (leftIndent, topIndent);
        const Rectangle clip (g.getClipBounds());
        Colour selectedTextColour;

        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.lineY + 200.0 < clip.getY() && i.next())
        {}

        if (selectionStart < selectionEnd)
        {
            g.setColour (findColour (highlightColourId)
                            .withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));

            selectedTextColour = findColour (highlightedTextColourId);

            TextEditorIterator i2 (i);

            while (i2.next() && i2.lineY < clip.getBottom())
            {
                if (i2.lineY + i2.lineHeight >= clip.getY()
                     && selectionEnd >= i2.indexInText
                     && selectionStart <= i2.indexInText + i2.atom->numChars)
                {
                    i2.drawSelection (g, selectionStart, selectionEnd);
                }
            }
        }

        const UniformTextSection* lastSection = 0;

        while (i.next() && i.lineY < clip.getBottom())
        {
            if (i.lineY + i.lineHeight >= clip.getY())
            {
                if (selectionEnd >= i.indexInText
                     && selectionStart <= i.indexInText + i.atom->numChars)
                {
                    i.drawSelectedText (g, selectionStart, selectionEnd, selectedTextColour);
                    lastSection = 0;
                }
                else
                {
                    i.draw (g, lastSection);
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
    if (caretFlashState
         && hasKeyboardFocus (false)
         && caretVisible
         && ! isReadOnly())
    {
        g.setColour (findColour (caretColourId));

        g.fillRect (borderSize.getLeft() + textHolder->getX() + leftIndent + cursorX,
                    borderSize.getTop() + textHolder->getY() + topIndent + cursorY,
                    2.0f, cursorHeight);
    }

    if (textToShowWhenEmpty.isNotEmpty()
         && (! hasKeyboardFocus (false))
         && getTotalNumChars() == 0)
    {
        g.setColour (colourForTextWhenEmpty);
        g.setFont (getFont());

        if (isMultiLine())
        {
            g.drawText (textToShowWhenEmpty,
                        0, 0, getWidth(), getHeight(),
                        Justification::centred, true);
        }
        else
        {
            g.drawText (textToShowWhenEmpty,
                        leftIndent, topIndent,
                        viewport->getWidth() - leftIndent,
                        viewport->getHeight() - topIndent,
                        Justification::centredLeft, true);
        }
    }

    getLookAndFeel().drawTextEditorOutline (g, getWidth(), getHeight(), *this);
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
            moveCursorTo (getTextIndexAt (e.x, e.y),
                          e.mods.isShiftDown());
        }
        else
        {
            PopupMenu m;
            m.setLookAndFeel (&getLookAndFeel());
            addPopupMenuItems (m, &e);

            menuActive = true;
            const int result = m.show();
            menuActive = false;

            if (result != 0)
                performPopupMenuAction (result);
        }
    }
}

void TextEditor::mouseDrag (const MouseEvent& e)
{
    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            moveCursorTo (getTextIndexAt (e.x, e.y), true);
        }
    }
}

void TextEditor::mouseUp (const MouseEvent& e)
{
    newTransaction();
    textHolder->startTimer (flashSpeedIntervalMs);

    if (wasFocused || ! selectAllTextWhenFocused)
    {
        if (e.mouseWasClicked() && ! (popupMenuEnabled && e.mods.isPopupMenu()))
        {
            moveCaret (getTextIndexAt (e.x, e.y));
        }
    }

    wasFocused = true;
}

void TextEditor::mouseDoubleClick (const MouseEvent& e)
{
    int tokenEnd = getTextIndexAt (e.x, e.y);
    int tokenStart = tokenEnd;

    if (e.getNumberOfClicks() > 3)
    {
        tokenStart = 0;
        tokenEnd = getTotalNumChars();
    }
    else
    {
        const String t (getText());
        const int totalLength = getTotalNumChars();

        while (tokenEnd < totalLength)
        {
            if (CharacterFunctions::isLetterOrDigit (t [tokenEnd]))
                ++tokenEnd;
            else
                break;
        }

        tokenStart = tokenEnd;

        while (tokenStart > 0)
        {
            if (CharacterFunctions::isLetterOrDigit (t [tokenStart - 1]))
                --tokenStart;
            else
                break;
        }

        if (e.getNumberOfClicks() > 2)
        {
            while (tokenEnd < totalLength)
            {
                if (t [tokenEnd] != T('\r') && t [tokenEnd] != T('\n'))
                    ++tokenEnd;
                else
                    break;
            }

            while (tokenStart > 0)
            {
                if (t [tokenStart - 1] != T('\r') && t [tokenStart - 1] != T('\n'))
                    --tokenStart;
                else
                    break;
            }
        }
    }

    moveCursorTo (tokenEnd, false);
    moveCursorTo (tokenStart, true);
}

void TextEditor::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if (! viewport->useMouseWheelMoveIfNeeded (e, wheelIncrementX, wheelIncrementY))
        Component::mouseWheelMove (e, wheelIncrementX, wheelIncrementY);
}

//==============================================================================
bool TextEditor::keyPressed (const KeyPress& key)
{
    if (isReadOnly() && key != KeyPress (T('c'), ModifierKeys::commandModifier, 0))
        return false;

    const bool moveInWholeWordSteps = key.getModifiers().isCtrlDown() || key.getModifiers().isAltDown();

    if (key.isKeyCode (KeyPress::leftKey)
         || key.isKeyCode (KeyPress::upKey))
    {
        newTransaction();

        int newPos;

        if (isMultiLine() && key.isKeyCode (KeyPress::upKey))
            newPos = indexAtPosition (cursorX, cursorY - 1);
        else if (moveInWholeWordSteps)
            newPos = findWordBreakBefore (getCaretPosition());
        else
            newPos = getCaretPosition() - 1;

        moveCursorTo (newPos, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::rightKey)
              || key.isKeyCode (KeyPress::downKey))
    {
        newTransaction();

        int newPos;

        if (isMultiLine() && key.isKeyCode (KeyPress::downKey))
            newPos = indexAtPosition (cursorX, cursorY + cursorHeight + 1);
        else if (moveInWholeWordSteps)
            newPos = findWordBreakAfter (getCaretPosition());
        else
            newPos = getCaretPosition() + 1;

        moveCursorTo (newPos, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::pageDownKey) && isMultiLine())
    {
        newTransaction();

        moveCursorTo (indexAtPosition (cursorX, cursorY + cursorHeight + viewport->getViewHeight()),
                      key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::pageUpKey) && isMultiLine())
    {
        newTransaction();

        moveCursorTo (indexAtPosition (cursorX, cursorY - viewport->getViewHeight()),
                      key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::homeKey))
    {
        newTransaction();

        if (isMultiLine() && ! moveInWholeWordSteps)
            moveCursorTo (indexAtPosition (0.0f, cursorY),
                          key.getModifiers().isShiftDown());
        else
            moveCursorTo (0, key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::endKey))
    {
        newTransaction();

        if (isMultiLine() && ! moveInWholeWordSteps)
            moveCursorTo (indexAtPosition ((float) textHolder->getWidth(), cursorY),
                          key.getModifiers().isShiftDown());
        else
            moveCursorTo (getTotalNumChars(), key.getModifiers().isShiftDown());
    }
    else if (key.isKeyCode (KeyPress::backspaceKey))
    {
        if (moveInWholeWordSteps)
        {
            moveCursorTo (findWordBreakBefore (getCaretPosition()), true);
        }
        else
        {
            if (selectionStart == selectionEnd && selectionStart > 0)
                --selectionStart;
        }

        cut();
    }
    else if (key.isKeyCode (KeyPress::deleteKey))
    {
        if (key.getModifiers().isShiftDown())
            copy();

        if (selectionStart == selectionEnd
             && selectionEnd < getTotalNumChars())
        {
            ++selectionEnd;
        }

        cut();
    }
    else if (key == KeyPress (T('c'), ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::ctrlModifier, 0))
    {
        newTransaction();
        copy();
    }
    else if (key == KeyPress (T('x'), ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        copy();
        cut();
    }
    else if (key == KeyPress (T('v'), ModifierKeys::commandModifier, 0)
              || key == KeyPress (KeyPress::insertKey, ModifierKeys::shiftModifier, 0))
    {
        newTransaction();
        paste();
    }
    else if (key == KeyPress (T('z'), ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        doUndoRedo (false);
    }
    else if (key == KeyPress (T('y'), ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        doUndoRedo (true);
    }
    else if (key == KeyPress (T('a'), ModifierKeys::commandModifier, 0))
    {
        newTransaction();
        moveCursorTo (getTotalNumChars(), false);
        moveCursorTo (0, true);
    }
    else if (key == KeyPress::returnKey)
    {
        newTransaction();

        if (returnKeyStartsNewLine)
            insertTextAtCursor (T("\n"));
        else
            returnPressed();
    }
    else if (key.isKeyCode (KeyPress::escapeKey))
    {
        newTransaction();
        moveCursorTo (getCaretPosition(), false);
        escapePressed();
    }
    else if (key.getTextCharacter() >= ' '
              || (tabKeyUsed && (key.getTextCharacter() == '\t')))
    {
        insertTextAtCursor (String::charToString (key.getTextCharacter()));

        lastTransactionTime = Time::getApproximateMillisecondCounter();
    }
    else
    {
        return false;
    }

    return true;
}

bool TextEditor::keyStateChanged (const bool isKeyDown)
{
    if (! isKeyDown)
        return false;

#if JUCE_WIN32
    if (KeyPress (KeyPress::F4Key, ModifierKeys::altModifier, 0).isCurrentlyDown())
        return false;  // We need to explicitly allow alt-F4 to pass through on Windows
#endif

    // (overridden to avoid forwarding key events to the parent)
    return ! ModifierKeys::getCurrentModifiers().isCommandDown();
}

//==============================================================================
const int baseMenuItemID = 0x7fff0000;

void TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const bool writable = ! isReadOnly();

    if (passwordCharacter == 0)
    {
        m.addItem (baseMenuItemID + 1, TRANS("cut"), writable);
        m.addItem (baseMenuItemID + 2, TRANS("copy"), selectionStart < selectionEnd);
        m.addItem (baseMenuItemID + 3, TRANS("paste"), writable);
    }

    m.addItem (baseMenuItemID + 4, TRANS("delete"), writable);
    m.addSeparator();
    m.addItem (baseMenuItemID + 5, TRANS("select all"));
    m.addSeparator();
    m.addItem (baseMenuItemID + 6, TRANS("undo"), undoManager.canUndo());
    m.addItem (baseMenuItemID + 7, TRANS("redo"), undoManager.canRedo());
}

void TextEditor::performPopupMenuAction (const int menuItemID)
{
    switch (menuItemID)
    {
    case baseMenuItemID + 1:
        copy();
        cut();
        break;

    case baseMenuItemID + 2:
        copy();
        break;

    case baseMenuItemID + 3:
        paste();
        break;

    case baseMenuItemID + 4:
        cut();
        break;

    case baseMenuItemID + 5:
        moveCursorTo (getTotalNumChars(), false);
        moveCursorTo (0, true);
        break;

    case baseMenuItemID + 6:
        doUndoRedo (false);
        break;

    case baseMenuItemID + 7:
        doUndoRedo (true);
        break;

    default:
        break;
    }
}

//==============================================================================
void TextEditor::focusGained (FocusChangeType)
{
    newTransaction();

    caretFlashState = true;

    if (selectAllTextWhenFocused)
    {
        moveCursorTo (0, false);
        moveCursorTo (getTotalNumChars(), true);
    }

    repaint();

    if (caretVisible)
        textHolder->startTimer (flashSpeedIntervalMs);

    ComponentPeer* const peer = getPeer();
    if (peer != 0 && ! isReadOnly())
        peer->textInputRequired (getScreenX() - peer->getScreenX(),
                                 getScreenY() - peer->getScreenY());
}

void TextEditor::focusLost (FocusChangeType)
{
    newTransaction();

    wasFocused = false;
    textHolder->stopTimer();
    caretFlashState = false;

    postCommandMessage (focusLossMessageId);
    repaint();
}

//==============================================================================
void TextEditor::resized()
{
    viewport->setBoundsInset (borderSize);
    viewport->setSingleStepSizes (16, roundToInt (currentFont.getHeight()));

    updateTextHolderSize();

    if (! isMultiLine())
    {
        scrollToMakeSureCursorIsVisible();
    }
    else
    {
        updateCaretPosition();
    }
}

void TextEditor::handleCommandMessage (const int commandId)
{
    const ComponentDeletionWatcher deletionChecker (this);

    for (int i = listeners.size(); --i >= 0;)
    {
        TextEditorListener* const tl = (TextEditorListener*) listeners [i];

        if (tl != 0)
        {
            switch (commandId)
            {
            case textChangeMessageId:
                tl->textEditorTextChanged (*this);
                break;

            case returnKeyMessageId:
                tl->textEditorReturnKeyPressed (*this);
                break;

            case escapeKeyMessageId:
                tl->textEditorEscapeKeyPressed (*this);
                break;

            case focusLossMessageId:
                tl->textEditorFocusLost (*this);
                break;

            default:
                jassertfalse
                break;
            }

            if (i > 0 && deletionChecker.hasBeenDeleted())
                return;
        }
    }
}

void TextEditor::enablementChanged()
{
    setMouseCursor (MouseCursor (isReadOnly() ? MouseCursor::NormalCursor
                                              : MouseCursor::IBeamCursor));
    repaint();
}

//==============================================================================
void TextEditor::clearInternal (UndoManager* const um)
{
    remove (0, getTotalNumChars(), um, caretPosition);
}

void TextEditor::insert (const String& text,
                         const int insertIndex,
                         const Font& font,
                         const Colour& colour,
                         UndoManager* const um,
                         const int caretPositionToMoveTo)
{
    if (text.isNotEmpty())
    {
        if (um != 0)
        {
            um->perform (new TextEditorInsertAction (*this,
                                                     text,
                                                     insertIndex,
                                                     font,
                                                     colour,
                                                     caretPosition,
                                                     caretPositionToMoveTo));
        }
        else
        {
            repaintText (insertIndex, -1); // must do this before and after changing the data, in case
                                           // a line gets moved due to word wrap

            int index = 0;
            int nextIndex = 0;

            for (int i = 0; i < sections.size(); ++i)
            {
                nextIndex = index + ((UniformTextSection*) sections.getUnchecked(i))->getTotalLength();

                if (insertIndex == index)
                {
                    sections.insert (i, new UniformTextSection (text,
                                                                font, colour,
                                                                passwordCharacter));
                    break;
                }
                else if (insertIndex > index && insertIndex < nextIndex)
                {
                    splitSection (i, insertIndex - index);
                    sections.insert (i + 1, new UniformTextSection (text,
                                                                    font, colour,
                                                                    passwordCharacter));
                    break;
                }

                index = nextIndex;
            }

            if (nextIndex == insertIndex)
                sections.add (new UniformTextSection (text,
                                                      font, colour,
                                                      passwordCharacter));

            coalesceSimilarSections();
            totalNumChars = -1;
            valueTextNeedsUpdating = true;

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (insertIndex, -1);
        }
    }
}

void TextEditor::reinsert (const int insertIndex,
                           const VoidArray& sectionsToInsert)
{
    int index = 0;
    int nextIndex = 0;

    for (int i = 0; i < sections.size(); ++i)
    {
        nextIndex = index + ((UniformTextSection*) sections.getUnchecked(i))->getTotalLength();

        if (insertIndex == index)
        {
            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i, new UniformTextSection (*(UniformTextSection*) sectionsToInsert.getUnchecked(j)));

            break;
        }
        else if (insertIndex > index && insertIndex < nextIndex)
        {
            splitSection (i, insertIndex - index);

            for (int j = sectionsToInsert.size(); --j >= 0;)
                sections.insert (i + 1, new UniformTextSection (*(UniformTextSection*) sectionsToInsert.getUnchecked(j)));

            break;
        }

        index = nextIndex;
    }

    if (nextIndex == insertIndex)
    {
        for (int j = 0; j < sectionsToInsert.size(); ++j)
            sections.add (new UniformTextSection (*(UniformTextSection*) sectionsToInsert.getUnchecked(j)));
    }

    coalesceSimilarSections();
    totalNumChars = -1;
    valueTextNeedsUpdating = true;
}

void TextEditor::remove (const int startIndex,
                         int endIndex,
                         UndoManager* const um,
                         const int caretPositionToMoveTo)
{
    if (endIndex > startIndex)
    {
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            const int nextIndex = index + ((UniformTextSection*) sections[i])->getTotalLength();

            if (startIndex > index && startIndex < nextIndex)
            {
                splitSection (i, startIndex - index);
                --i;
            }
            else if (endIndex > index && endIndex < nextIndex)
            {
                splitSection (i, endIndex - index);
                --i;
            }
            else
            {
                index = nextIndex;

                if (index > endIndex)
                    break;
            }
        }

        index = 0;

        if (um != 0)
        {
            VoidArray removedSections;

            for (int i = 0; i < sections.size(); ++i)
            {
                if (endIndex <= startIndex)
                    break;

                UniformTextSection* const section = (UniformTextSection*) sections.getUnchecked (i);

                const int nextIndex = index + section->getTotalLength();

                if (startIndex <= index && endIndex >= nextIndex)
                    removedSections.add (new UniformTextSection (*section));

                index = nextIndex;
            }

            um->perform (new TextEditorRemoveAction (*this,
                                                     startIndex,
                                                     endIndex,
                                                     caretPosition,
                                                     caretPositionToMoveTo,
                                                     removedSections));
        }
        else
        {
            for (int i = 0; i < sections.size(); ++i)
            {
                if (endIndex <= startIndex)
                    break;

                UniformTextSection* const section = (UniformTextSection*) sections.getUnchecked (i);

                const int nextIndex = index + section->getTotalLength();

                if (startIndex <= index && endIndex >= nextIndex)
                {
                    sections.remove(i);
                    endIndex -= (nextIndex - index);
                    section->clear();
                    delete section;
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

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (startIndex, -1);
        }
    }
}

//==============================================================================
const String TextEditor::getText() const
{
    String t;
    t.preallocateStorage (getTotalNumChars());
    String::Concatenator concatenator (t);

    for (int i = 0; i < sections.size(); ++i)
        ((const UniformTextSection*) sections.getUnchecked(i))->appendAllText (concatenator);

    return t;
}

const String TextEditor::getTextSubstring (const int startCharacter, const int endCharacter) const
{
    String t;

    if (endCharacter > startCharacter)
    {
        t.preallocateStorage (jmin (getTotalNumChars(), endCharacter - startCharacter));
        String::Concatenator concatenator (t);
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            const UniformTextSection* const s = (const UniformTextSection*) sections.getUnchecked(i);
            const int nextIndex = index + s->getTotalLength();

            if (startCharacter < nextIndex)
            {
                if (endCharacter <= index)
                    break;

                s->appendSubstring (concatenator,
                                    startCharacter - index,
                                    endCharacter - index);
            }

            index = nextIndex;
        }
    }

    return t;
}

const String TextEditor::getHighlightedText() const
{
    return getTextSubstring (selectionStart, selectionEnd);
}

int TextEditor::getTotalNumChars() const
{
    if (totalNumChars < 0)
    {
        totalNumChars = 0;

        for (int i = sections.size(); --i >= 0;)
            totalNumChars += ((const UniformTextSection*) sections.getUnchecked(i))->getTotalLength();
    }

    return totalNumChars;
}

bool TextEditor::isEmpty() const
{
    return getTotalNumChars() == 0;
}

void TextEditor::getCharPosition (const int index, float& cx, float& cy, float& lineHeight) const
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0 && sections.size() > 0)
    {
        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        i.getCharPosition (index, cx, cy, lineHeight);
    }
    else
    {
        cx = cy = 0;
        lineHeight = currentFont.getHeight();
    }
}

int TextEditor::indexAtPosition (const float x, const float y)
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.next())
        {
            if (i.lineY + i.lineHeight > y)
            {
                if (i.lineY > y)
                    return jmax (0, i.indexInText - 1);

                if (i.atomX >= x)
                    return i.indexInText;

                if (x < i.atomRight)
                    return i.xToIndex (x);
            }
        }
    }

    return getTotalNumChars();
}

//==============================================================================
static int getCharacterCategory (const tchar character)
{
    return CharacterFunctions::isLetterOrDigit (character)
                ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
}

int TextEditor::findWordBreakAfter (const int position) const
{
    const String t (getTextSubstring (position, position + 512));
    const int totalLength = t.length();
    int i = 0;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    const int type = getCharacterCategory (t[i]);

    while (i < totalLength && type == getCharacterCategory (t[i]))
        ++i;

    while (i < totalLength && CharacterFunctions::isWhitespace (t[i]))
        ++i;

    return position + i;
}

int TextEditor::findWordBreakBefore (const int position) const
{
    if (position <= 0)
        return 0;

    const int startOfBuffer = jmax (0, position - 512);
    const String t (getTextSubstring (startOfBuffer, position));

    int i = position - startOfBuffer;

    while (i > 0 && CharacterFunctions::isWhitespace (t [i - 1]))
        --i;

    if (i > 0)
    {
        const int type = getCharacterCategory (t [i - 1]);

        while (i > 0 && type == getCharacterCategory (t [i - 1]))
            --i;
    }

    jassert (startOfBuffer + i >= 0);
    return startOfBuffer + i;
}


//==============================================================================
void TextEditor::splitSection (const int sectionIndex,
                               const int charToSplitAt)
{
    jassert (sections[sectionIndex] != 0);

    sections.insert (sectionIndex + 1,
                      ((UniformTextSection*) sections.getUnchecked (sectionIndex))
                           ->split (charToSplitAt, passwordCharacter));
}

void TextEditor::coalesceSimilarSections()
{
    for (int i = 0; i < sections.size() - 1; ++i)
    {
        UniformTextSection* const s1 = (UniformTextSection*) sections.getUnchecked (i);
        UniformTextSection* const s2 = (UniformTextSection*) sections.getUnchecked (i + 1);

        if (s1->font == s2->font
             && s1->colour == s2->colour)
        {
            s1->append (*s2, passwordCharacter);
            sections.remove (i + 1);
            delete s2;
            --i;
        }
    }
}


END_JUCE_NAMESPACE
