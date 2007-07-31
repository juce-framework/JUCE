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

#include "juce_TextEditor.h"
#include "../../graphics/fonts/juce_GlyphArrangement.h"
#include "../../../application/juce_SystemClipboard.h"
#include "../../../../juce_core/basics/juce_Time.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"
#include "../lookandfeel/juce_LookAndFeel.h"

#define SHOULD_WRAP(x, wrapwidth)       (((x) - 0.0001f) >= (wrapwidth))

//==============================================================================
// a word or space that can't be broken down any further
struct TextAtom
{
    //==============================================================================
    String atomText;
    float width;
    uint16 numChars;

    //==============================================================================
    bool isWhitespace() const throw()       { return CharacterFunctions::isWhitespace (atomText[0]); }
    bool isNewLine() const throw()          { return atomText[0] == T('\r') || atomText[0] == T('\n'); }

    const String getText (const tchar passwordCharacter) const throw()
    {
        if (passwordCharacter == 0)
            return atomText;
        else
            return String::repeatedString (String::charToString (passwordCharacter),
                                           atomText.length());
    }

    const String getTrimmedText (const tchar passwordCharacter) const throw()
    {
        if (passwordCharacter == 0)
            return atomText;
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
                        const tchar passwordCharacter) throw()
      : font (font_),
        colour (colour_),
        atoms (64)
    {
        initialiseAtoms (text, passwordCharacter);
    }

    UniformTextSection (const UniformTextSection& other) throw()
      : font (other.font),
        colour (other.colour),
        atoms (64)
    {
        for (int i = 0; i < other.atoms.size(); ++i)
            atoms.add (new TextAtom (*(const TextAtom*) other.atoms.getUnchecked(i)));
    }

    ~UniformTextSection() throw()
    {
        // (no need to delete the atoms, as they're explicitly deleted by the caller)
    }

    void clear() throw()
    {
        for (int i = atoms.size(); --i >= 0;)
        {
            TextAtom* const atom = getAtom(i);
            delete atom;
        }

        atoms.clear();
    }

    int getNumAtoms() const throw()
    {
        return atoms.size();
    }

    TextAtom* getAtom (const int index) const throw()
    {
        return (TextAtom*) atoms.getUnchecked (index);
    }

    void append (const UniformTextSection& other, const tchar passwordCharacter) throw()
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

            while (i < other.atoms.size())
            {
                atoms.add (other.getAtom(i));
                ++i;
            }
        }
    }

    UniformTextSection* split (const int indexToBreakAt,
                               const tchar passwordCharacter) throw()
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

    const String getAllText() const throw()
    {
        String s;

        for (int i = 0; i < atoms.size(); ++i)
            s += getAtom(i)->atomText;

        return s;
    }

    const String getTextSubstring (const int startCharacter,
                                   const int endCharacter) const throw()
    {
        String s;
        int index = 0;

        for (int i = 0; i < atoms.size(); ++i)
        {
            const TextAtom* const atom = getAtom (i);
            const int nextIndex = index + atom->numChars;

            if (startCharacter < nextIndex)
            {
                if (endCharacter <= index)
                    break;

                const int start = jmax (index, startCharacter);
                s += atom->atomText.substring (start - index, endCharacter - index);
            }

            index = nextIndex;
        }

        return s;
    }

    int getTotalLength() const throw()
    {
        int c = 0;

        for (int i = atoms.size(); --i >= 0;)
            c += getAtom(i)->numChars;

        return c;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

    Font font;
    Colour colour;

private:
    VoidArray atoms;

    //==============================================================================
    void initialiseAtoms (const String& textToParse,
                          const tchar passwordCharacter) throw()
    {
        int i = 0;
        const int len = textToParse.length();
        const tchar* const text = (const tchar*) textToParse;

        while (i < len)
        {
            const int start = i;
            int end = i;

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

                end = i;
            }
            else
            {
                if (text[i] == T('\r'))
                {
                    ++i;
                    end = i;

                    if ((i < len) && (text[i] == T('\n')))
                        ++i;

                }
                else if (text[i] == T('\n'))
                {
                    ++i;
                    end = i;

                    if ((i < len) && (text[i] == T('\r')))
                        ++i;
                }
                else
                {
                    while ((i < len) && ! CharacterFunctions::isWhitespace (text[i]))
                        ++i;

                    end = i;
                }
            }

            TextAtom* const atom = new TextAtom();
            atom->atomText = textToParse.substring (start, end);
            atom->width = font.getStringWidthFloat (atom->getText (passwordCharacter));
            atom->numChars = (uint16) (end - start);

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
                        const tchar passwordCharacter_) throw()
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
            currentSection = (const UniformTextSection*) sections.getUnchecked (sectionIndex);

        if (currentSection != 0)
        {
            lineHeight = currentSection->font.getHeight();
            maxDescent = currentSection->font.getDescent();
        }
    }

    ~TextEditorIterator() throw()
    {
    }

    //==============================================================================
    bool next() throw()
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
                    if (SHOULD_WRAP (g.getGlyph (split).getRight(), wordWrapWidth))
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

        if (sectionIndex >= sections.size())
        {
            moveToEndOfLastAtom();
            return false;
        }
        else if (atomIndex >= currentSection->getNumAtoms())
        {
            if (++sectionIndex >= sections.size())
            {
                moveToEndOfLastAtom();
                return false;
            }

            atomIndex = 0;
            currentSection = (const UniformTextSection*) sections.getUnchecked (sectionIndex);

            lineHeight = jmax (lineHeight, currentSection->font.getHeight());
            maxDescent = jmax (maxDescent, currentSection->font.getDescent());
        }

        if (atom != 0)
        {
            atomX = atomRight;
            indexInText += atom->numChars;

            if (atom->isNewLine())
            {
                atomX = 0;
                lineY += lineHeight;
            }
        }

        atom = currentSection->getAtom (atomIndex);
        atomRight = atomX + atom->width;
        ++atomIndex;

        if (SHOULD_WRAP (atomRight, wordWrapWidth))
        {
            if (atom->isWhitespace())
            {
                // leave whitespace at the end of a line, but truncate it to avoid scrolling
                atomRight = jmin (atomRight, wordWrapWidth);
            }
            else
            {
                atomRight = atom->width;

                if (SHOULD_WRAP (atomRight, wordWrapWidth))  // atom too big to fit on a line, so break it up..
                {
                    tempAtom = *atom;
                    tempAtom.width = 0;
                    tempAtom.numChars = 0;
                    atom = &tempAtom;

                    if (atomX > 0)
                    {
                        atomX = 0;
                        lineY += lineHeight;
                    }

                    return next();
                }

                atomX = 0;
                lineY += lineHeight;
            }
        }

        return true;
    }

    //==============================================================================
    void draw (Graphics& g, const UniformTextSection*& lastSection) const throw()
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
                              (float) roundFloatToInt (lineY + lineHeight - maxDescent));
            ga.draw (g);
        }
    }

    void drawSelection (Graphics& g,
                        const int selectionStart,
                        const int selectionEnd) const throw()
    {
        const int startX = roundFloatToInt (indexToX (selectionStart));
        const int endX   = roundFloatToInt (indexToX (selectionEnd));

        const int y = roundFloatToInt (lineY);
        const int nextY = roundFloatToInt (lineY + lineHeight);

        g.fillRect (startX, y, endX - startX, nextY - y);
    }

    void drawSelectedText (Graphics& g,
                           const int selectionStart,
                           const int selectionEnd,
                           const Colour& selectedTextColour) const throw()
    {
        if (passwordCharacter != 0 || ! atom->isWhitespace())
        {
            GlyphArrangement ga;
            ga.addLineOfText (currentSection->font,
                              atom->getTrimmedText (passwordCharacter),
                              atomX,
                              (float) roundFloatToInt (lineY + lineHeight - maxDescent));

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
    float indexToX (const int indexToFind) const throw()
    {
        if (indexToFind <= indexInText)
            return atomX;

        if (indexToFind >= indexInText + atom->numChars)
            return atomRight;

        GlyphArrangement g;
        g.addLineOfText (currentSection->font,
                         atom->getText (passwordCharacter),
                         atomX, 0.0f);

        return jmin (atomRight, g.getGlyph (indexToFind - indexInText).getLeft());
    }

    int xToIndex (const float xToFind) const throw()
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
        for (j = 0; j < atom->numChars; ++j)
            if ((g.getGlyph(j).getLeft() + g.getGlyph(j).getRight()) / 2 > xToFind)
                break;

        return indexInText + j;
    }

    //==============================================================================
    void updateLineHeight() throw()
    {
        float x = atomRight;

        int tempSectionIndex = sectionIndex;
        int tempAtomIndex = atomIndex;
        const UniformTextSection* currentSection = (const UniformTextSection*) sections.getUnchecked (tempSectionIndex);

        while (! SHOULD_WRAP (x, wordWrapWidth))
        {
            if (tempSectionIndex >= sections.size())
                break;

            bool checkSize = false;

            if (tempAtomIndex >= currentSection->getNumAtoms())
            {
                if (++tempSectionIndex >= sections.size())
                    break;

                tempAtomIndex = 0;
                currentSection = (const UniformTextSection*) sections.getUnchecked (tempSectionIndex);
                checkSize = true;
            }

            const TextAtom* const atom = currentSection->getAtom (tempAtomIndex);

            if (atom == 0)
                break;

            x += atom->width;

            if (SHOULD_WRAP (x, wordWrapWidth) || atom->isNewLine())
                break;

            if (checkSize)
            {
                lineHeight = jmax (lineHeight, currentSection->font.getHeight());
                maxDescent = jmax (maxDescent, currentSection->font.getDescent());
            }

            ++tempAtomIndex;
        }
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

    TextEditorIterator (const TextEditorIterator&);
    const TextEditorIterator& operator= (const TextEditorIterator&);

    void moveToEndOfLastAtom() throw()
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
                             public Timer
{
    TextEditor* const owner;

    TextHolderComponent (const TextHolderComponent&);
    const TextHolderComponent& operator= (const TextHolderComponent&);

public:
    TextHolderComponent (TextEditor* const owner_)
        : owner (owner_)
    {
        setWantsKeyboardFocus (false);
        setInterceptsMouseClicks (false, true);
    }

    ~TextHolderComponent()
    {
    }

    void paint (Graphics& g)
    {
        owner->drawContent (g);
    }

    void timerCallback()
    {
        owner->timerCallbackInt();
    }

    const MouseCursor getMouseCursor()
    {
        return owner->getMouseCursor();
    }
};

//==============================================================================
class TextEditorViewport  : public Viewport
{
    TextEditor* const owner;

    TextEditorViewport (const TextEditorViewport&);
    const TextEditorViewport& operator= (const TextEditorViewport&);

public:
    TextEditorViewport (TextEditor* const owner_)
        : owner (owner_)
    {
    }

    ~TextEditorViewport()
    {
    }

    void visibleAreaChanged (int, int, int, int)
    {
        owner->updateTextHolderSize();
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
      sections (8),
      passwordCharacter (passwordCharacter_),
      dragType (notDragging),
      listeners (2)
{
    setOpaque (true);

    addAndMakeVisible (viewport = new TextEditorViewport (this));
    viewport->setViewedComponent (textHolder = new TextHolderComponent (this));
    viewport->setWantsKeyboardFocus (false);
    viewport->setScrollBarsShown (false, false);

    setMouseCursor (MouseCursor::IBeamCursor);
    setWantsKeyboardFocus (true);
}

TextEditor::~TextEditor()
{
    clearInternal (0);
    delete viewport;
}

//==============================================================================
void TextEditor::newTransaction() throw()
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

bool TextEditor::isMultiLine() const throw()
{
    return multiline;
}

void TextEditor::setScrollbarsShown (bool enabled) throw()
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

bool TextEditor::isReadOnly() const throw()
{
    return readOnly || ! isEnabled();
}

void TextEditor::setReturnKeyStartsNewLine (const bool shouldStartNewLine)
{
    returnKeyStartsNewLine = shouldStartNewLine;
}

void TextEditor::setTabKeyUsedAsCharacter (const bool shouldTabKeyBeUsed) throw()
{
    tabKeyUsed = shouldTabKeyBeUsed;
}

void TextEditor::setPopupMenuEnabled (const bool b) throw()
{
    popupMenuEnabled = b;
}

void TextEditor::setSelectAllWhenFocused (const bool b) throw()
{
    selectAllTextWhenFocused = b;
}

//==============================================================================
const Font TextEditor::getFont() const throw()
{
    return currentFont;
}

void TextEditor::setFont (const Font& newFont) throw()
{
    currentFont = newFont;
    scrollToMakeSureCursorIsVisible();
}

void TextEditor::applyFontToAllText (const Font& newFont)
{
    currentFont = newFont;

    const String oldText (getText());
    clearInternal (0);
    insert (oldText, 0, newFont, findColour (textColourId), 0, caretPosition);

    updateTextHolderSize();
    scrollToMakeSureCursorIsVisible();
    repaint();
}

void TextEditor::colourChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
    repaint();
}

void TextEditor::setCaretVisible (const bool shouldCaretBeVisible) throw()
{
    caretVisible = shouldCaretBeVisible;

    if (shouldCaretBeVisible)
        textHolder->startTimer (flashSpeedIntervalMs);

    setMouseCursor (shouldCaretBeVisible ? MouseCursor::IBeamCursor
                                         : MouseCursor::NormalCursor);
}

void TextEditor::setInputRestrictions (const int maxLen,
                                       const String& chars) throw()
{
    maxTextLength = jmax (0, maxLen);
    allowedCharacters = chars;
}

void TextEditor::setTextToShowWhenEmpty (const String& text, const Colour& colourToUse) throw()
{
    textToShowWhenEmpty = text;
    colourForTextWhenEmpty = colourToUse;
}

void TextEditor::setPasswordCharacter (const tchar newPasswordCharacter) throw()
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
void TextEditor::textChanged() throw()
{
    updateTextHolderSize();
    postCommandMessage (textChangeMessageId);
}

void TextEditor::returnPressed()
{
    postCommandMessage (returnKeyMessageId);
}

void TextEditor::escapePressed()
{
    postCommandMessage (escapeKeyMessageId);
}

void TextEditor::addListener (TextEditorListener* const newListener) throw()
{
    jassert (newListener != 0)

    if (newListener != 0)
        listeners.add (newListener);
}

void TextEditor::removeListener (TextEditorListener* const listenerToRemove) throw()
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
        repaint (borderSize.getLeft() + textHolder->getX() + leftIndent + roundFloatToInt (cursorX) - 1,
                 borderSize.getTop() + textHolder->getY() + topIndent + roundFloatToInt (cursorY) - 1,
                 4,
                 roundFloatToInt (cursorHeight) + 2);
}

void TextEditor::repaintText (int textStartIndex, int textEndIndex)
{
    if (textStartIndex > textEndIndex && textEndIndex > 0)
        swapVariables (textStartIndex, textEndIndex);

    float x, y, lh;
    getCharPosition (textStartIndex, x, y, lh);
    const int y1 = (int) y;

    int y2;

    if (textEndIndex >= 0)
    {
        getCharPosition (textEndIndex, x, y, lh);
        y2 = (int) (y + lh * 2.0f);
    }
    else
    {
        y2 = textHolder->getHeight();
    }

    textHolder->repaint (0, y1, textHolder->getWidth(), y2 - y1);
}

//==============================================================================
void TextEditor::moveCaret (int newCaretPos) throw()
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

void TextEditor::setCaretPosition (const int newIndex) throw()
{
    moveCursorTo (newIndex, false);
}

int TextEditor::getCaretPosition() const throw()
{
    return caretPosition;
}

//==============================================================================
float TextEditor::getWordWrapWidth() const throw()
{
    return (wordWrap) ? (float) (viewport->getMaximumVisibleWidth() - leftIndent - leftIndent / 2)
                      : 1.0e10f;
}

void TextEditor::updateTextHolderSize() throw()
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        float maxWidth = 0.0f;

        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

        while (i.next())
            maxWidth = jmax (maxWidth, i.atomRight);

        const int w = leftIndent + roundFloatToInt (maxWidth);
        const int h = topIndent + roundFloatToInt (jmax (i.lineY + i.lineHeight,
                                                         currentFont.getHeight()));

        textHolder->setSize (w + 1, h + 1);
    }
}

int TextEditor::getTextWidth() const throw()
{
    return textHolder->getWidth();
}

int TextEditor::getTextHeight() const throw()
{
    return textHolder->getHeight();
}

void TextEditor::setIndents (const int newLeftIndent,
                             const int newTopIndent) throw()
{
    leftIndent = newLeftIndent;
    topIndent = newTopIndent;
}

void TextEditor::setBorder (const BorderSize& border) throw()
{
    borderSize = border;
    resized();
}

const BorderSize TextEditor::getBorder() const throw()
{
    return borderSize;
}

void TextEditor::setScrollToShowCursor (const bool shouldScrollToShowCursor) throw()
{
    keepCursorOnScreen = shouldScrollToShowCursor;
}

void TextEditor::scrollToMakeSureCursorIsVisible() throw()
{
    cursorHeight = currentFont.getHeight(); // (in case the text is empty and the call below doesn't set this value)

    getCharPosition (caretPosition,
                     cursorX, cursorY,
                     cursorHeight);

    if (keepCursorOnScreen)
    {
        int x = viewport->getViewPositionX();
        int y = viewport->getViewPositionY();

        const int relativeCursorX = roundFloatToInt (cursorX) - x;
        const int relativeCursorY = roundFloatToInt (cursorY) - y;

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
            const int curH = roundFloatToInt (cursorHeight);

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
                               const bool isSelecting) throw()
{
    if (isSelecting)
    {
        moveCaret (newPosition);
        repaintText (selectionStart, selectionEnd);

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

        repaintText (selectionStart, selectionEnd);
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
                                const int y) throw()
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

    const int newCaretPos = selectionStart + newText.length();
    const int insertIndex = selectionStart;

    remove (selectionStart, selectionEnd,
            &undoManager,
            newCaretPos);

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

void TextEditor::setHighlightedRegion (int startPos, int numChars) throw()
{
    moveCursorTo (startPos, false);
    moveCursorTo (startPos + numChars, true);
}

//==============================================================================
void TextEditor::copy()
{
    const String selection (getTextSubstring (selectionStart, selectionEnd));

    if (selection.isNotEmpty())
        SystemClipboard::copyTextToClipboard (selection);
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

        if (selectionStart < selectionEnd)
        {
            g.setColour (findColour (highlightColourId)
                            .withMultipliedAlpha (hasKeyboardFocus (true) ? 1.0f : 0.5f));

            selectedTextColour = findColour (highlightedTextColourId);

            TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);

            while (i.next() && i.lineY < clip.getBottom())
            {
                if (i.lineY + getHeight() >= clip.getY())
                    i.updateLineHeight();

                if (i.lineY + i.lineHeight >= clip.getY()
                     && selectionEnd >= i.indexInText
                     && selectionStart <= i.indexInText + i.atom->numChars)
                {
                    i.drawSelection (g, selectionStart, selectionEnd);
                }
            }
        }

        TextEditorIterator i (sections, wordWrapWidth, passwordCharacter);
        const UniformTextSection* lastSection = 0;

        while (i.next() && i.lineY < clip.getBottom())
        {
            if (i.lineY + getHeight() >= clip.getY())
                i.updateLineHeight();

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
    g.fillAll (findColour (backgroundColourId));
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
        if (! (popupMenuEnabled && e.mods.isPopupMenu()))
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

        if (isMultiLine() && key.isKeyCode (KeyPress::upKey))
        {
            moveCursorTo (indexAtPosition (cursorX, cursorY - 1),
                          key.getModifiers().isShiftDown());
        }
        else if (moveInWholeWordSteps)
        {
            moveCursorTo (findWordBreakBefore (getCaretPosition()),
                          key.getModifiers().isShiftDown());
        }
        else
        {
            moveCursorTo (getCaretPosition() - 1, key.getModifiers().isShiftDown());
        }
    }
    else if (key.isKeyCode (KeyPress::rightKey)
              || key.isKeyCode (KeyPress::downKey))
    {
        newTransaction();

        if (key.isKeyCode (KeyPress::downKey) && isMultiLine())
        {
            moveCursorTo (indexAtPosition (cursorX, cursorY + cursorHeight + 1),
                          key.getModifiers().isShiftDown());
        }
        else if (moveInWholeWordSteps)
        {
            moveCursorTo (findWordBreakAfter (getCaretPosition()),
                          key.getModifiers().isShiftDown());
        }
        else
        {
            moveCursorTo (getCaretPosition() + 1, key.getModifiers().isShiftDown());
        }
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
    else if (key == KeyPress (T('c'), ModifierKeys::commandModifier, 0))
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
    else if (key == KeyPress (T('v'), ModifierKeys::commandModifier, 0))
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
    else if (key.isKeyCode (KeyPress::returnKey))
    {
        if (! isReadOnly())
        {
            newTransaction();

            if (returnKeyStartsNewLine)
                insertTextAtCursor (T("\n"));
            else
                returnPressed();
        }
    }
    else if (key.isKeyCode (KeyPress::escapeKey))
    {
        newTransaction();
        moveCursorTo (getCaretPosition(), false);
        escapePressed();
    }
    else if (key.getTextCharacter() != 0
              && (! isReadOnly())
              && (tabKeyUsed || ! key.isKeyCode (KeyPress::tabKey)))
    {
        if (! isReadOnly())
            insertTextAtCursor (String::charToString (key.getTextCharacter()));

        lastTransactionTime = Time::getApproximateMillisecondCounter();
    }
    else
    {
        return false;
    }

    return true;
}

bool TextEditor::keyStateChanged()
{
    // (overridden to avoid forwarding key events to the parent)
    return true;
}

//==============================================================================
const int baseMenuItemID = 0x7fff0000;

void TextEditor::addPopupMenuItems (PopupMenu& m, const MouseEvent*)
{
    const bool writable = ! isReadOnly();

    m.addItem (baseMenuItemID + 1, TRANS("cut"), writable);
    m.addItem (baseMenuItemID + 2, TRANS("copy"), selectionStart < selectionEnd);
    m.addItem (baseMenuItemID + 3, TRANS("paste"), writable);
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
    viewport->setSingleStepSizes (16, roundFloatToInt (currentFont.getHeight()));

    updateTextHolderSize();

    if (! isMultiLine())
    {
        scrollToMakeSureCursorIsVisible();
    }
    else
    {
        cursorHeight = currentFont.getHeight(); // (in case the text is empty and the call below doesn't set this value)

        getCharPosition (caretPosition,
                         cursorX, cursorY,
                         cursorHeight);
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
void TextEditor::clearInternal (UndoManager* const um) throw()
{
    remove (0, getTotalNumChars(), um, caretPosition);
}

void TextEditor::insert (const String& text,
                         const int insertIndex,
                         const Font& font,
                         const Colour& colour,
                         UndoManager* const um,
                         const int caretPositionToMoveTo) throw()
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

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (insertIndex, -1);
        }
    }
}

void TextEditor::reinsert (const int insertIndex,
                           const VoidArray& sectionsToInsert) throw()
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
}

void TextEditor::remove (const int startIndex,
                         int endIndex,
                         UndoManager* const um,
                         const int caretPositionToMoveTo) throw()
{
    if (endIndex > startIndex)
    {
        int index = 0;

        for (int i = 0; i < sections.size(); ++i)
        {
            const int nextIndex = index + ((UniformTextSection*)sections[i])->getTotalLength();

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

            moveCursorTo (caretPositionToMoveTo, false);

            repaintText (startIndex, -1);
        }
    }
}

//==============================================================================
const String TextEditor::getText() const throw()
{
    String t;

    for (int i = 0; i < sections.size(); ++i)
        t += ((const UniformTextSection*) sections.getUnchecked(i))->getAllText();

    return t;
}

const String TextEditor::getTextSubstring (const int startCharacter, const int endCharacter) const throw()
{
    String t;
    int index = 0;

    for (int i = 0; i < sections.size(); ++i)
    {
        const UniformTextSection* const s = (const UniformTextSection*) sections.getUnchecked(i);
        const int nextIndex = index + s->getTotalLength();

        if (startCharacter < nextIndex)
        {
            if (endCharacter <= index)
                break;

            const int start = jmax (index, startCharacter);
            t += s->getTextSubstring (start - index, endCharacter - index);
        }

        index = nextIndex;
    }

    return t;
}

const String TextEditor::getHighlightedText() const throw()
{
    return getTextSubstring (getHighlightedRegionStart(),
                             getHighlightedRegionStart() + getHighlightedRegionLength());
}

int TextEditor::getTotalNumChars() throw()
{
    if (totalNumChars < 0)
    {
        totalNumChars = 0;

        for (int i = sections.size(); --i >= 0;)
            totalNumChars += ((const UniformTextSection*) sections.getUnchecked(i))->getTotalLength();
    }

    return totalNumChars;
}

bool TextEditor::isEmpty() const throw()
{
    if (totalNumChars != 0)
    {
        for (int i = sections.size(); --i >= 0;)
            if (((const UniformTextSection*) sections.getUnchecked(i))->getTotalLength() > 0)
                return false;
    }

    return true;
}

void TextEditor::getCharPosition (const int index, float& cx, float& cy, float& lineHeight) const throw()
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        TextEditorIterator i (sections, getWordWrapWidth(), passwordCharacter);

        while (i.next())
        {
            cy = i.lineY;

            if (i.indexInText + i.atom->numChars > index)
            {
                i.updateLineHeight();
                cx = i.indexToX (index);
                lineHeight = i.lineHeight;
                return;
            }
        }

        cx = i.atomX;
        cy = i.lineY;
    }
    else
    {
        cx = cy = 0;
    }

    lineHeight = currentFont.getHeight();
}

int TextEditor::indexAtPosition (const float x, const float y) throw()
{
    const float wordWrapWidth = getWordWrapWidth();

    if (wordWrapWidth > 0)
    {
        TextEditorIterator i (sections, getWordWrapWidth(), passwordCharacter);

        while (i.next())
        {
            if (i.lineY + getHeight() > y)
                i.updateLineHeight();

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
static int getCharacterCategory (const tchar character) throw()
{
    return CharacterFunctions::isLetterOrDigit (character)
                ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
}

int TextEditor::findWordBreakAfter (int position) const throw()
{
    const String t (getTextSubstring (position, position + 512));
    const int totalLength = t.length();
    int i = 0;

    while (i < totalLength)
    {
        if (CharacterFunctions::isWhitespace (t [i]))
            ++position;
        else
            break;

        ++i;
    }

    const int type = getCharacterCategory (t [i]);

    while (i < totalLength)
    {
        if (type == getCharacterCategory (t [i]))
            ++position;
        else
            break;

        ++i;
    }

    return position;
}

int TextEditor::findWordBreakBefore (int position) const throw()
{
    if (position > 0)
    {
        const int maximumToDo = jmin (512, position);
        const int startOfBuffer = position - maximumToDo;
        const String t (getTextSubstring (startOfBuffer, position));

        while (position > startOfBuffer)
        {
            if (CharacterFunctions::isWhitespace (t [position - 1 - startOfBuffer]))
                --position;
            else
                break;
        }

        const int type = getCharacterCategory (t [position - 1 - startOfBuffer]);

        while (position > startOfBuffer)
        {
            if (type == getCharacterCategory (t [position - 1 - startOfBuffer]))
                --position;
            else
                break;
        }
    }

    return jmax (position, 0);
}


//==============================================================================
void TextEditor::splitSection (const int sectionIndex,
                               const int charToSplitAt) throw()
{
    jassert (sections[sectionIndex] != 0);

    sections.insert (sectionIndex + 1,
                      ((UniformTextSection*) sections.getUnchecked (sectionIndex))
                        ->split (charToSplitAt, passwordCharacter));
}

void TextEditor::coalesceSimilarSections() throw()
{
    for (int i = 0; i < sections.size() - 1; ++i)
    {
        UniformTextSection* const s1 = (UniformTextSection*) (sections.getUnchecked (i));
        UniformTextSection* const s2 = (UniformTextSection*) (sections.getUnchecked (i + 1));

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
