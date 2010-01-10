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

#include "juce_CodeDocument.h"


//==============================================================================
class CodeDocumentLine
{
public:
    CodeDocumentLine (const String& line_,
                      const int lineLength_,
                      const int lineStartInFile_) throw()
        : line (line_),
          lineStartInFile (lineStartInFile_),
          lineLength (lineLength_)
    {
        lineLengthWithoutNewLines = lineLength;

        while (lineLengthWithoutNewLines > 0
                && (line [lineLengthWithoutNewLines - 1] == '\n'
                    || line [lineLengthWithoutNewLines - 1] == '\r'))
        {
            --lineLengthWithoutNewLines;
        }
    }

    ~CodeDocumentLine() throw()
    {
    }

    static void createLines (Array <CodeDocumentLine*>& newLines, const String& text) throw()
    {
        const tchar* const t = (const tchar*) text;
        int pos = 0;

        while (t [pos] != 0)
        {
            const int startOfLine = pos;

            while (t[pos] != 0)
            {
                if (t[pos] == T('\r'))
                {
                    ++pos;
                    if (t[pos] == T('\n'))
                        ++pos;

                    break;
                }

                if (t[pos] == T('\n'))
                {
                    ++pos;
                    break;
                }

                ++pos;
            }

            newLines.add (new CodeDocumentLine (String (t + startOfLine, pos - startOfLine),
                                                pos - startOfLine,
                                                startOfLine));
        }

        jassert (pos == text.length());
    }

    bool endsWithLineBreak() const throw()
    {
        return lineLengthWithoutNewLines != lineLength;
    }

    void updateLength() throw()
    {
        lineLengthWithoutNewLines = lineLength = line.length();

        while (lineLengthWithoutNewLines > 0
                && (line [lineLengthWithoutNewLines - 1] == '\n'
                    || line [lineLengthWithoutNewLines - 1] == '\r'))
        {
            --lineLengthWithoutNewLines;
        }
    }

    String line;
    int lineStartInFile, lineLength, lineLengthWithoutNewLines;
};

//==============================================================================
CodeDocument::Iterator::Iterator (CodeDocument* const document_) throw()
    : document (document_),
      line (0),
      position (0)
{
}

CodeDocument::Iterator::Iterator (const CodeDocument::Iterator& other)
    : document (other.document),
      line (other.line),
      position (other.position)
{
}

const CodeDocument::Iterator& CodeDocument::Iterator::operator= (const CodeDocument::Iterator& other) throw()
{
    document = other.document;
    line = other.line;
    position = other.position;

    return *this;
}

CodeDocument::Iterator::~Iterator() throw()
{
}

juce_wchar CodeDocument::Iterator::nextChar() throw()
{
    if (line >= document->lines.size())
        return 0;

    const CodeDocumentLine* const currentLine = document->lines.getUnchecked (line);
    const juce_wchar result = currentLine->line [position - currentLine->lineStartInFile];

    if (++position >= currentLine->lineStartInFile + currentLine->lineLength)
        ++line;

    return result;
}

void CodeDocument::Iterator::skip() throw()
{
    if (line < document->lines.size())
    {
        const CodeDocumentLine* const currentLine = document->lines.getUnchecked (line);

        if (++position >= currentLine->lineStartInFile + currentLine->lineLength)
            ++line;
    }
}

juce_wchar CodeDocument::Iterator::peekNextChar() const throw()
{
    if (line >= document->lines.size())
        return 0;

    const CodeDocumentLine* const currentLine = document->lines.getUnchecked (line);
    return currentLine->line [position - currentLine->lineStartInFile];
}

void CodeDocument::Iterator::skipWhitespace()
{
    while (line < document->lines.size())
    {
        const CodeDocumentLine* const currentLine = document->lines.getUnchecked (line);

        for (;;)
        {
            if (! CharacterFunctions::isWhitespace (currentLine->line [position - currentLine->lineStartInFile]))
                return;

            if (++position >= currentLine->lineStartInFile + currentLine->lineLength)
            {
                ++line;
                break;
            }
        }
    }
}

void CodeDocument::Iterator::skipToEndOfLine()
{
    if (line < document->lines.size())
    {
        const CodeDocumentLine* const currentLine = document->lines.getUnchecked (line);
        position = currentLine->lineStartInFile + currentLine->lineLength;
        ++line;
    }
}

bool CodeDocument::Iterator::isEOF() const throw()
{
    return position >= document->getNumCharacters();
}

//==============================================================================
CodeDocument::Position::Position() throw()
    : owner (0), characterPos (0), line (0),
      indexInLine (0), positionMaintained (false)
{
}

CodeDocument::Position::Position (const CodeDocument* const ownerDocument,
                                  const int line_, const int indexInLine_) throw()
    : owner (const_cast <CodeDocument*> (ownerDocument)),
      characterPos (0), line (line_),
      indexInLine (indexInLine_), positionMaintained (false)
{
    setLineAndIndex (line_, indexInLine_);
}

CodeDocument::Position::Position (const CodeDocument* const ownerDocument,
                                  const int characterPos_) throw()
    : owner (const_cast <CodeDocument*> (ownerDocument)),
      positionMaintained (false)
{
    setPosition (characterPos_);
}

CodeDocument::Position::Position (const Position& other) throw()
    : owner (other.owner), characterPos (other.characterPos), line (other.line),
      indexInLine (other.indexInLine), positionMaintained (false)
{
    jassert (*this == other);
}

CodeDocument::Position::~Position() throw()
{
    setPositionMaintained (false);
}

const CodeDocument::Position& CodeDocument::Position::operator= (const Position& other) throw()
{
    if (this != &other)
    {
        const bool wasPositionMaintained = positionMaintained;
        if (owner != other.owner)
            setPositionMaintained (false);

        owner = other.owner;
        line = other.line;
        indexInLine = other.indexInLine;
        characterPos = other.characterPos;
        setPositionMaintained (wasPositionMaintained);

        jassert (*this == other);
    }

    return *this;
}

bool CodeDocument::Position::operator== (const Position& other) const throw()
{
    jassert ((characterPos == other.characterPos)
               == (line == other.line && indexInLine == other.indexInLine));

    return characterPos == other.characterPos
            && line == other.line
            && indexInLine == other.indexInLine
            && owner == other.owner;
}

bool CodeDocument::Position::operator!= (const Position& other) const throw()
{
    return ! operator== (other);
}

void CodeDocument::Position::setLineAndIndex (const int newLine, const int newIndexInLine) throw()
{
    jassert (owner != 0);

    if (owner->lines.size() == 0)
    {
        line = 0;
        indexInLine = 0;
        characterPos = 0;
    }
    else
    {
        if (newLine >= owner->lines.size())
        {
            line = owner->lines.size() - 1;

            CodeDocumentLine* const l = owner->lines.getUnchecked (line);
            jassert (l != 0);

            indexInLine = l->lineLengthWithoutNewLines;
            characterPos = l->lineStartInFile + indexInLine;
        }
        else
        {
            line = jmax (0, newLine);

            CodeDocumentLine* const l = owner->lines.getUnchecked (line);
            jassert (l != 0);

            if (l->lineLengthWithoutNewLines > 0)
                indexInLine = jlimit (0, l->lineLengthWithoutNewLines, newIndexInLine);
            else
                indexInLine = 0;

            characterPos = l->lineStartInFile + indexInLine;
        }
    }
}

void CodeDocument::Position::setPosition (const int newPosition) throw()
{
    jassert (owner != 0);

    line = 0;
    indexInLine = 0;
    characterPos = 0;

    if (newPosition > 0)
    {
        int lineStart = 0;
        int lineEnd = owner->lines.size();

        for (;;)
        {
            if (lineEnd - lineStart < 4)
            {
                for (int i = lineStart; i < lineEnd; ++i)
                {
                    CodeDocumentLine* const l = owner->lines.getUnchecked (i);

                    int index = newPosition - l->lineStartInFile;

                    if (index >= 0 && (index < l->lineLength || i == lineEnd - 1))
                    {
                        line = i;
                        indexInLine = jmin (l->lineLengthWithoutNewLines, index);
                        characterPos = l->lineStartInFile + indexInLine;
                    }
                }

                break;
            }
            else
            {
                const int midIndex = (lineStart + lineEnd + 1) / 2;
                CodeDocumentLine* const mid = owner->lines.getUnchecked (midIndex);

                if (newPosition >= mid->lineStartInFile)
                    lineStart = midIndex;
                else
                    lineEnd = midIndex;
            }
        }
    }
}

void CodeDocument::Position::moveBy (int characterDelta) throw()
{
    jassert (owner != 0);

    if (characterDelta == 1)
    {
        setPosition (getPosition());

        // If moving right, make sure we don't get stuck between the \r and \n characters..
        CodeDocumentLine* const l = owner->lines.getUnchecked (line);
        if (indexInLine + characterDelta < l->lineLength
             && indexInLine + characterDelta >= l->lineLengthWithoutNewLines + 1)
            ++characterDelta;
    }

    setPosition (characterPos + characterDelta);
}

const CodeDocument::Position CodeDocument::Position::movedBy (const int characterDelta) const throw()
{
    CodeDocument::Position p (*this);
    p.moveBy (characterDelta);
    return p;
}

const CodeDocument::Position CodeDocument::Position::movedByLines (const int deltaLines) const throw()
{
    CodeDocument::Position p (*this);
    p.setLineAndIndex (getLineNumber() + deltaLines, getIndexInLine());
    return p;
}

const tchar CodeDocument::Position::getCharacter() const throw()
{
    const CodeDocumentLine* const l = owner->lines [line];
    return l == 0 ? 0 : l->line [getIndexInLine()];
}

const String CodeDocument::Position::getLineText() const throw()
{
    const CodeDocumentLine* const l = owner->lines [line];
    return l == 0 ? String::empty : l->line;
}

void CodeDocument::Position::setPositionMaintained (const bool isMaintained) throw()
{
    if (isMaintained != positionMaintained)
    {
        positionMaintained = isMaintained;

        if (owner != 0)
        {
            if (isMaintained)
            {
                jassert (! owner->positionsToMaintain.contains (this));
                owner->positionsToMaintain.add (this);
            }
            else
            {
                jassert (owner->positionsToMaintain.contains (this));
                owner->positionsToMaintain.removeValue (this);
            }
        }
    }
}

//==============================================================================
CodeDocument::CodeDocument()
    : undoManager (INT_MAX, 10000),
      currentActionIndex (0),
      indexOfSavedState (-1),
      maximumLineLength (-1),
      newLineChars ("\r\n")
{
}

CodeDocument::~CodeDocument()
{
}

const String CodeDocument::getAllContent() const throw()
{
    return getTextBetween (Position (this, 0),
                           Position (this, lines.size(), 0));
}

const String CodeDocument::getTextBetween (const Position& start, const Position& end) const throw()
{
    if (end.getPosition() <= start.getPosition())
        return String::empty;

    const int startLine = start.getLineNumber();
    const int endLine = end.getLineNumber();

    if (startLine == endLine)
    {
        CodeDocumentLine* const line = lines [startLine];
        return (line == 0) ? String::empty : line->line.substring (start.getIndexInLine(), end.getIndexInLine());
    }

    String result;
    result.preallocateStorage (end.getPosition() - start.getPosition() + 4);
    String::Concatenator concatenator (result);

    const int maxLine = jmin (lines.size() - 1, endLine);

    for (int i = jmax (0, startLine); i <= maxLine; ++i)
    {
        const CodeDocumentLine* line = lines.getUnchecked(i);
        int len = line->lineLength;

        if (i == startLine)
        {
            const int index = start.getIndexInLine();
            concatenator.append (line->line.substring (index, len));
        }
        else if (i == endLine)
        {
            len = end.getIndexInLine();
            concatenator.append (line->line.substring (0, len));
        }
        else
        {
            concatenator.append (line->line);
        }
    }

    return result;
}

int CodeDocument::getNumCharacters() const throw()
{
    const CodeDocumentLine* const lastLine = lines.getLast();
    return (lastLine == 0) ? 0 : lastLine->lineStartInFile + lastLine->lineLength;
}

const String CodeDocument::getLine (const int lineIndex) const throw()
{
    const CodeDocumentLine* const line = lines [lineIndex];
    return (line == 0) ? String::empty : line->line;
}

int CodeDocument::getMaximumLineLength() throw()
{
    if (maximumLineLength < 0)
    {
        maximumLineLength = 0;

        for (int i = lines.size(); --i >= 0;)
            maximumLineLength = jmax (maximumLineLength, lines.getUnchecked(i)->lineLength);
    }

    return maximumLineLength;
}

void CodeDocument::deleteSection (const Position& startPosition, const Position& endPosition)
{
    remove (startPosition.getPosition(), endPosition.getPosition(), true);
}

void CodeDocument::insertText (const Position& position, const String& text)
{
    insert (text, position.getPosition(), true);
}

void CodeDocument::replaceAllContent (const String& newContent)
{
    remove (0, getNumCharacters(), true);
    insert (newContent, 0, true);
}

void CodeDocument::setNewLineCharacters (const String& newLine) throw()
{
    jassert (newLine == T("\r\n") || newLine == T("\n") || newLine == T("\r"));
    newLineChars = newLine;
}

void CodeDocument::newTransaction()
{
    undoManager.beginNewTransaction (String::empty);
}

void CodeDocument::undo()
{
    newTransaction();
    undoManager.undo();
}

void CodeDocument::redo()
{
    undoManager.redo();
}

void CodeDocument::clearUndoHistory()
{
    undoManager.clearUndoHistory();
}

void CodeDocument::setSavePoint() throw()
{
    indexOfSavedState = currentActionIndex;
}

bool CodeDocument::hasChangedSinceSavePoint() const throw()
{
    return currentActionIndex != indexOfSavedState;
}

//==============================================================================
static int getCodeCharacterCategory (const tchar character) throw()
{
    return (CharacterFunctions::isLetterOrDigit (character) || character == '_')
                ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
}

const CodeDocument::Position CodeDocument::findWordBreakAfter (const Position& position) const throw()
{
    Position p (position);
    const int maxDistance = 256;
    int i = 0;

    while (i < maxDistance
            && CharacterFunctions::isWhitespace (p.getCharacter())
            && (i == 0 || (p.getCharacter() != T('\n')
                            && p.getCharacter() != T('\r'))))
    {
        ++i;
        p.moveBy (1);
    }

    if (i == 0)
    {
        const int type = getCodeCharacterCategory (p.getCharacter());

        while (i < maxDistance && type == getCodeCharacterCategory (p.getCharacter()))
        {
            ++i;
            p.moveBy (1);
        }

        while (i < maxDistance
                && CharacterFunctions::isWhitespace (p.getCharacter())
                && (i == 0 || (p.getCharacter() != T('\n')
                                && p.getCharacter() != T('\r'))))
        {
            ++i;
            p.moveBy (1);
        }
    }

    return p;
}

const CodeDocument::Position CodeDocument::findWordBreakBefore (const Position& position) const throw()
{
    Position p (position);
    const int maxDistance = 256;
    int i = 0;
    bool stoppedAtLineStart = false;

    while (i < maxDistance)
    {
        const tchar c = p.movedBy (-1).getCharacter();

        if (c == T('\r') || c == T('\n'))
        {
            stoppedAtLineStart = true;

            if (i > 0)
                break;
        }

        if (! CharacterFunctions::isWhitespace (c))
            break;

        p.moveBy (-1);
        ++i;
    }

    if (i < maxDistance && ! stoppedAtLineStart)
    {
        const int type = getCodeCharacterCategory (p.movedBy (-1).getCharacter());

        while (i < maxDistance && type == getCodeCharacterCategory (p.movedBy (-1).getCharacter()))
        {
            p.moveBy (-1);
            ++i;
        }
    }

    return p;
}


//==============================================================================
void CodeDocument::addListener (CodeDocument::Listener* const listener) throw()
{
    listeners.addIfNotAlreadyThere (listener);
}

void CodeDocument::removeListener (CodeDocument::Listener* const listener) throw()
{
    listeners.removeValue (listener);
}

void CodeDocument::sendListenerChangeMessage (const int startLine, const int endLine)
{
    const Position startPos (this, startLine, 0);
    const Position endPos (this, endLine, 0);

    for (int i = listeners.size(); --i >= 0;)
    {
        Listener* const l = (Listener*) listeners[i];

        if (l != 0)
            l->codeDocumentChanged (startPos, endPos);
    }
}

//==============================================================================
class CodeDocumentInsertAction   : public UndoableAction
{
    CodeDocument& owner;
    const String text;
    int insertPos;

    CodeDocumentInsertAction (const CodeDocumentInsertAction&);
    const CodeDocumentInsertAction& operator= (const CodeDocumentInsertAction&);

public:
    CodeDocumentInsertAction (CodeDocument& owner_, const String& text_, const int insertPos_) throw()
        : owner (owner_),
          text (text_),
          insertPos (insertPos_)
    {
    }

    ~CodeDocumentInsertAction() {}

    bool perform()
    {
        owner.currentActionIndex++;
        owner.insert (text, insertPos, false);
        return true;
    }

    bool undo()
    {
        owner.currentActionIndex--;
        owner.remove (insertPos, insertPos + text.length(), false);
        return true;
    }

    int getSizeInUnits()        { return text.length() + 32; }
};

void CodeDocument::insert (const String& text, const int insertPos, const bool undoable)
{
    if (text.isEmpty())
        return;

    if (undoable)
    {
        undoManager.perform (new CodeDocumentInsertAction (*this, text, insertPos));
    }
    else
    {
        Position pos (this, insertPos);
        const int firstAffectedLine = pos.getLineNumber();
        int lastAffectedLine = firstAffectedLine + 1;

        CodeDocumentLine* const firstLine = lines [firstAffectedLine];
        String textInsideOriginalLine (text);

        if (firstLine != 0)
        {
            const int index = pos.getIndexInLine();
            textInsideOriginalLine = firstLine->line.substring (0, index)
                                     + textInsideOriginalLine
                                     + firstLine->line.substring (index);
        }

        maximumLineLength = -1;
        Array <CodeDocumentLine*> newLines;
        CodeDocumentLine::createLines (newLines, textInsideOriginalLine);
        jassert (newLines.size() > 0);

        CodeDocumentLine* const newFirstLine = newLines.getUnchecked (0);
        newFirstLine->lineStartInFile = firstLine != 0 ? firstLine->lineStartInFile : 0;
        lines.set (firstAffectedLine, newFirstLine);

        if (newLines.size() > 1)
        {
            for (int i = 1; i < newLines.size(); ++i)
            {
                CodeDocumentLine* const l = newLines.getUnchecked (i);
                lines.insert (firstAffectedLine + i, l);
            }

            lastAffectedLine = lines.size();
        }

        int i, lineStart = newFirstLine->lineStartInFile;
        for (i = firstAffectedLine; i < lines.size(); ++i)
        {
            CodeDocumentLine* const l = lines.getUnchecked (i);
            l->lineStartInFile = lineStart;
            lineStart += l->lineLength;
        }

        const int newTextLength = text.length();
        for (i = 0; i < positionsToMaintain.size(); ++i)
        {
            CodeDocument::Position* const p = positionsToMaintain.getUnchecked(i);

            if (p->getPosition() >= insertPos)
                p->setPosition (p->getPosition() + newTextLength);
        }

        sendListenerChangeMessage (firstAffectedLine, lastAffectedLine);
    }
}

//==============================================================================
class CodeDocumentDeleteAction  : public UndoableAction
{
    CodeDocument& owner;
    int startPos, endPos;
    String removedText;

    CodeDocumentDeleteAction (const CodeDocumentDeleteAction&);
    const CodeDocumentDeleteAction& operator= (const CodeDocumentDeleteAction&);

public:
    CodeDocumentDeleteAction (CodeDocument& owner_, const int startPos_, const int endPos_) throw()
        : owner (owner_),
          startPos (startPos_),
          endPos (endPos_)
    {
        removedText = owner.getTextBetween (CodeDocument::Position (&owner, startPos),
                                            CodeDocument::Position (&owner, endPos));
    }

    ~CodeDocumentDeleteAction() {}

    bool perform()
    {
        owner.currentActionIndex++;
        owner.remove (startPos, endPos, false);
        return true;
    }

    bool undo()
    {
        owner.currentActionIndex--;
        owner.insert (removedText, startPos, false);
        return true;
    }

    int getSizeInUnits()    { return removedText.length() + 32; }
};

void CodeDocument::remove (const int startPos, const int endPos, const bool undoable)
{
    if (endPos <= startPos)
        return;

    if (undoable)
    {
        undoManager.perform (new CodeDocumentDeleteAction (*this, startPos, endPos));
    }
    else
    {
        Position startPosition (this, startPos);
        Position endPosition (this, endPos);

        maximumLineLength = -1;
        const int firstAffectedLine = startPosition.getLineNumber();
        const int endLine = endPosition.getLineNumber();
        int lastAffectedLine = firstAffectedLine + 1;
        CodeDocumentLine* const firstLine = lines.getUnchecked (firstAffectedLine);

        if (firstAffectedLine == endLine)
        {
            firstLine->line = firstLine->line.substring (0, startPosition.getIndexInLine())
                            + firstLine->line.substring (endPosition.getIndexInLine());
            firstLine->updateLength();
        }
        else
        {
            lastAffectedLine = lines.size();

            CodeDocumentLine* const lastLine = lines.getUnchecked (endLine);
            jassert (lastLine != 0);

            firstLine->line = firstLine->line.substring (0, startPosition.getIndexInLine())
                                + lastLine->line.substring (endPosition.getIndexInLine());
            firstLine->updateLength();

            int numLinesToRemove = endLine - firstAffectedLine;
            lines.removeRange (firstAffectedLine + 1, numLinesToRemove);
        }

        int i;
        for (i = firstAffectedLine + 1; i < lines.size(); ++i)
        {
            CodeDocumentLine* const l = lines.getUnchecked (i);
            const CodeDocumentLine* const previousLine = lines.getUnchecked (i - 1);
            l->lineStartInFile = previousLine->lineStartInFile + previousLine->lineLength;
        }

        const int totalChars = getNumCharacters();

        for (i = 0; i < positionsToMaintain.size(); ++i)
        {
            CodeDocument::Position* p = positionsToMaintain.getUnchecked(i);

            if (p->getPosition() > startPosition.getPosition())
                p->setPosition (jmax (startPos, p->getPosition() + startPos - endPos));

            if (p->getPosition() > totalChars)
                p->setPosition (totalChars);
        }

        sendListenerChangeMessage (firstAffectedLine, lastAffectedLine);
    }
}

END_JUCE_NAMESPACE
