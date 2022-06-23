/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class CodeDocumentLine
{
public:
    CodeDocumentLine (const String::CharPointerType startOfLine,
                      const String::CharPointerType endOfLine,
                      const int lineLen,
                      const int numNewLineChars,
                      const int startInFile)
        : line (startOfLine, endOfLine),
          lineStartInFile (startInFile),
          lineLength (lineLen),
          lineLengthWithoutNewLines (lineLen - numNewLineChars)
    {
    }

    static void createLines (Array<CodeDocumentLine*>& newLines, StringRef text)
    {
        auto t = text.text;
        int charNumInFile = 0;
        bool finished = false;

        while (! (finished || t.isEmpty()))
        {
            auto startOfLine = t;
            auto startOfLineInFile = charNumInFile;
            int lineLength = 0;
            int numNewLineChars = 0;

            for (;;)
            {
                auto c = t.getAndAdvance();

                if (c == 0)
                {
                    finished = true;
                    break;
                }

                ++charNumInFile;
                ++lineLength;

                if (c == '\r')
                {
                    ++numNewLineChars;

                    if (*t == '\n')
                    {
                        ++t;
                        ++charNumInFile;
                        ++lineLength;
                        ++numNewLineChars;
                    }

                    break;
                }

                if (c == '\n')
                {
                    ++numNewLineChars;
                    break;
                }
            }

            newLines.add (new CodeDocumentLine (startOfLine, t, lineLength,
                                                numNewLineChars, startOfLineInFile));
        }

        jassert (charNumInFile == text.length());
    }

    bool endsWithLineBreak() const noexcept
    {
        return lineLengthWithoutNewLines != lineLength;
    }

    void updateLength() noexcept
    {
        lineLength = 0;
        lineLengthWithoutNewLines = 0;

        for (auto t = line.getCharPointer();;)
        {
            auto c = t.getAndAdvance();

            if (c == 0)
                break;

            ++lineLength;

            if (c != '\n' && c != '\r')
                lineLengthWithoutNewLines = lineLength;
        }
    }

    String line;
    int lineStartInFile, lineLength, lineLengthWithoutNewLines;
};

//==============================================================================
CodeDocument::Iterator::Iterator (const CodeDocument& doc) noexcept
    : document (&doc)
{}

CodeDocument::Iterator::Iterator (CodeDocument::Position p) noexcept
    : document (p.owner),
      line (p.getLineNumber()),
      position (p.getPosition())
{
    reinitialiseCharPtr();

    for (int i = 0; i < p.getIndexInLine(); ++i)
    {
        charPointer.getAndAdvance();

        if (charPointer.isEmpty())
        {
            position -= (p.getIndexInLine() - i);
            break;
        }
    }
}

CodeDocument::Iterator::Iterator() noexcept
    : document (nullptr)
{
}

CodeDocument::Iterator::~Iterator() noexcept {}


bool CodeDocument::Iterator::reinitialiseCharPtr() const
{
    /** You're trying to use a default constructed iterator. Bad idea! */
    jassert (document != nullptr);

    if (charPointer.getAddress() == nullptr)
    {
        if (auto* l = document->lines[line])
            charPointer = l->line.getCharPointer();
        else
            return false;
    }

    return true;
}

juce_wchar CodeDocument::Iterator::nextChar() noexcept
{
    for (;;)
    {
        if (! reinitialiseCharPtr())
            return 0;

        if (auto result = charPointer.getAndAdvance())
        {
            if (charPointer.isEmpty())
            {
                ++line;
                charPointer = nullptr;
            }

            ++position;
            return result;
        }

        ++line;
        charPointer = nullptr;
    }
}

void CodeDocument::Iterator::skip() noexcept
{
    nextChar();
}

void CodeDocument::Iterator::skipToEndOfLine() noexcept
{
    if (! reinitialiseCharPtr())
        return;

    position += (int) charPointer.length();
    ++line;
    charPointer = nullptr;
}

void CodeDocument::Iterator::skipToStartOfLine() noexcept
{
    if (! reinitialiseCharPtr())
        return;

    if (auto* l = document->lines [line])
    {
        auto startPtr = l->line.getCharPointer();
        position -= (int) startPtr.lengthUpTo (charPointer);
        charPointer = startPtr;
    }
}

juce_wchar CodeDocument::Iterator::peekNextChar() const noexcept
{
    if (! reinitialiseCharPtr())
        return 0;

    if (auto c = *charPointer)
        return c;

    if (auto* l = document->lines [line + 1])
        return l->line[0];

    return 0;
}

juce_wchar CodeDocument::Iterator::previousChar() noexcept
{
    if (! reinitialiseCharPtr())
        return 0;

    for (;;)
    {
        if (auto* l = document->lines[line])
        {
            if (charPointer != l->line.getCharPointer())
            {
                --position;
                --charPointer;
                break;
            }
        }

        if (line == 0)
            return 0;

        --line;

        if (auto* prev = document->lines[line])
            charPointer = prev->line.getCharPointer().findTerminatingNull();
    }

    return *charPointer;
}

juce_wchar CodeDocument::Iterator::peekPreviousChar() const noexcept
{
    if (! reinitialiseCharPtr())
        return 0;

    if (auto* l = document->lines[line])
    {
        if (charPointer != l->line.getCharPointer())
            return *(charPointer - 1);

        if (auto* prev = document->lines[line - 1])
            return *(prev->line.getCharPointer().findTerminatingNull() - 1);
    }

    return 0;
}

void CodeDocument::Iterator::skipWhitespace() noexcept
{
    while (CharacterFunctions::isWhitespace (peekNextChar()))
        skip();
}

bool CodeDocument::Iterator::isEOF() const noexcept
{
    return charPointer.getAddress() == nullptr && line >= document->lines.size();
}

bool CodeDocument::Iterator::isSOF() const noexcept
{
    return position == 0;
}

CodeDocument::Position CodeDocument::Iterator::toPosition() const
{
    if (auto* l = document->lines[line])
    {
        reinitialiseCharPtr();
        int indexInLine = 0;
        auto linePtr = l->line.getCharPointer();

        while (linePtr != charPointer && ! linePtr.isEmpty())
        {
            ++indexInLine;
            ++linePtr;
        }

        return CodeDocument::Position (*document, line, indexInLine);
    }

    if (isEOF())
    {
        if (auto* last = document->lines.getLast())
        {
            auto lineIndex = document->lines.size() - 1;
            return CodeDocument::Position (*document, lineIndex, last->lineLength);
        }
    }

    return CodeDocument::Position (*document, 0, 0);
}

//==============================================================================
CodeDocument::Position::Position() noexcept
{
}

CodeDocument::Position::Position (const CodeDocument& ownerDocument,
                                  const int lineNum, const int index) noexcept
    : owner (const_cast<CodeDocument*> (&ownerDocument)),
      line (lineNum), indexInLine (index)
{
    setLineAndIndex (lineNum, index);
}

CodeDocument::Position::Position (const CodeDocument& ownerDocument, int pos) noexcept
    : owner (const_cast<CodeDocument*> (&ownerDocument))
{
    setPosition (pos);
}

CodeDocument::Position::Position (const Position& other) noexcept
    : owner (other.owner), characterPos (other.characterPos), line (other.line),
      indexInLine (other.indexInLine)
{
    jassert (*this == other);
}

CodeDocument::Position::~Position()
{
    setPositionMaintained (false);
}

CodeDocument::Position& CodeDocument::Position::operator= (const Position& other)
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

bool CodeDocument::Position::operator== (const Position& other) const noexcept
{
    jassert ((characterPos == other.characterPos)
               == (line == other.line && indexInLine == other.indexInLine));

    return characterPos == other.characterPos
            && line == other.line
            && indexInLine == other.indexInLine
            && owner == other.owner;
}

bool CodeDocument::Position::operator!= (const Position& other) const noexcept
{
    return ! operator== (other);
}

void CodeDocument::Position::setLineAndIndex (const int newLineNum, const int newIndexInLine)
{
    jassert (owner != nullptr);

    if (owner->lines.size() == 0)
    {
        line = 0;
        indexInLine = 0;
        characterPos = 0;
    }
    else
    {
        if (newLineNum >= owner->lines.size())
        {
            line = owner->lines.size() - 1;

            auto& l = *owner->lines.getUnchecked (line);
            indexInLine = l.lineLengthWithoutNewLines;
            characterPos = l.lineStartInFile + indexInLine;
        }
        else
        {
            line = jmax (0, newLineNum);

            auto& l = *owner->lines.getUnchecked (line);

            if (l.lineLengthWithoutNewLines > 0)
                indexInLine = jlimit (0, l.lineLengthWithoutNewLines, newIndexInLine);
            else
                indexInLine = 0;

            characterPos = l.lineStartInFile + indexInLine;
        }
    }
}

void CodeDocument::Position::setPosition (const int newPosition)
{
    jassert (owner != nullptr);

    line = 0;
    indexInLine = 0;
    characterPos = 0;

    if (newPosition > 0)
    {
        int lineStart = 0;
        auto lineEnd = owner->lines.size();

        for (;;)
        {
            if (lineEnd - lineStart < 4)
            {
                for (int i = lineStart; i < lineEnd; ++i)
                {
                    auto& l = *owner->lines.getUnchecked (i);
                    auto index = newPosition - l.lineStartInFile;

                    if (index >= 0 && (index < l.lineLength || i == lineEnd - 1))
                    {
                        line = i;
                        indexInLine = jmin (l.lineLengthWithoutNewLines, index);
                        characterPos = l.lineStartInFile + indexInLine;
                    }
                }

                break;
            }
            else
            {
                auto midIndex = (lineStart + lineEnd + 1) / 2;

                if (newPosition >= owner->lines.getUnchecked (midIndex)->lineStartInFile)
                    lineStart = midIndex;
                else
                    lineEnd = midIndex;
            }
        }
    }
}

void CodeDocument::Position::moveBy (int characterDelta)
{
    jassert (owner != nullptr);

    if (characterDelta == 1)
    {
        setPosition (getPosition());

        // If moving right, make sure we don't get stuck between the \r and \n characters..
        if (line < owner->lines.size())
        {
            auto& l = *owner->lines.getUnchecked (line);

            if (indexInLine + characterDelta < l.lineLength
                 && indexInLine + characterDelta >= l.lineLengthWithoutNewLines + 1)
                ++characterDelta;
        }
    }

    setPosition (characterPos + characterDelta);
}

CodeDocument::Position CodeDocument::Position::movedBy (const int characterDelta) const
{
    CodeDocument::Position p (*this);
    p.moveBy (characterDelta);
    return p;
}

CodeDocument::Position CodeDocument::Position::movedByLines (const int deltaLines) const
{
    CodeDocument::Position p (*this);
    p.setLineAndIndex (getLineNumber() + deltaLines, getIndexInLine());
    return p;
}

juce_wchar CodeDocument::Position::getCharacter() const
{
    if (auto* l = owner->lines [line])
        return l->line [getIndexInLine()];

    return 0;
}

String CodeDocument::Position::getLineText() const
{
    if (auto* l = owner->lines [line])
        return l->line;

    return {};
}

void CodeDocument::Position::setPositionMaintained (const bool isMaintained)
{
    if (isMaintained != positionMaintained)
    {
        positionMaintained = isMaintained;

        if (owner != nullptr)
        {
            if (isMaintained)
            {
                jassert (! owner->positionsToMaintain.contains (this));
                owner->positionsToMaintain.add (this);
            }
            else
            {
                // If this happens, you may have deleted the document while there are Position objects that are still using it...
                jassert (owner->positionsToMaintain.contains (this));
                owner->positionsToMaintain.removeFirstMatchingValue (this);
            }
        }
    }
}

//==============================================================================
CodeDocument::CodeDocument() : undoManager (std::numeric_limits<int>::max(), 10000)
{
}

CodeDocument::~CodeDocument()
{
}

String CodeDocument::getAllContent() const
{
    return getTextBetween (Position (*this, 0),
                           Position (*this, lines.size(), 0));
}

String CodeDocument::getTextBetween (const Position& start, const Position& end) const
{
    if (end.getPosition() <= start.getPosition())
        return {};

    auto startLine = start.getLineNumber();
    auto endLine = end.getLineNumber();

    if (startLine == endLine)
    {
        if (auto* line = lines [startLine])
            return line->line.substring (start.getIndexInLine(), end.getIndexInLine());

        return {};
    }

    MemoryOutputStream mo;
    mo.preallocate ((size_t) (end.getPosition() - start.getPosition() + 4));

    auto maxLine = jmin (lines.size() - 1, endLine);

    for (int i = jmax (0, startLine); i <= maxLine; ++i)
    {
        auto& line = *lines.getUnchecked(i);
        auto len = line.lineLength;

        if (i == startLine)
        {
            auto index = start.getIndexInLine();
            mo << line.line.substring (index, len);
        }
        else if (i == endLine)
        {
            len = end.getIndexInLine();
            mo << line.line.substring (0, len);
        }
        else
        {
            mo << line.line;
        }
    }

    return mo.toUTF8();
}

int CodeDocument::getNumCharacters() const noexcept
{
    if (auto* lastLine = lines.getLast())
        return lastLine->lineStartInFile + lastLine->lineLength;

    return 0;
}

String CodeDocument::getLine (const int lineIndex) const noexcept
{
    if (auto* line = lines[lineIndex])
        return line->line;

    return {};
}

int CodeDocument::getMaximumLineLength() noexcept
{
    if (maximumLineLength < 0)
    {
        maximumLineLength = 0;

        for (auto* l : lines)
            maximumLineLength = jmax (maximumLineLength, l->lineLength);
    }

    return maximumLineLength;
}

void CodeDocument::deleteSection (const Position& startPosition, const Position& endPosition)
{
    deleteSection (startPosition.getPosition(), endPosition.getPosition());
}

void CodeDocument::deleteSection (const int start, const int end)
{
    remove (start, end, true);
}

void CodeDocument::insertText (const Position& position, const String& text)
{
    insertText (position.getPosition(), text);
}

void CodeDocument::insertText (const int insertIndex, const String& text)
{
    insert (text, insertIndex, true);
}

void CodeDocument::replaceSection (const int start, const int end, const String& newText)
{
    insertText (end, newText);
    deleteSection (start, end);
}

void CodeDocument::applyChanges (const String& newContent)
{
    const String corrected (StringArray::fromLines (newContent)
                                .joinIntoString (newLineChars));

    TextDiff diff (getAllContent(), corrected);

    for (auto& c : diff.changes)
    {
        if (c.isDeletion())
            remove (c.start, c.start + c.length, true);
        else
            insert (c.insertedText, c.start, true);
    }
}

void CodeDocument::replaceAllContent (const String& newContent)
{
    remove (0, getNumCharacters(), true);
    insert (newContent, 0, true);
}

bool CodeDocument::loadFromStream (InputStream& stream)
{
    remove (0, getNumCharacters(), false);
    insert (stream.readEntireStreamAsString(), 0, false);
    setSavePoint();
    clearUndoHistory();
    return true;
}

bool CodeDocument::writeToStream (OutputStream& stream)
{
    for (auto* l : lines)
    {
        auto temp = l->line; // use a copy to avoid bloating the memory footprint of the stored string.
        const char* utf8 = temp.toUTF8();

        if (! stream.write (utf8, strlen (utf8)))
            return false;
    }

    return true;
}

void CodeDocument::setNewLineCharacters (const String& newChars) noexcept
{
    jassert (newChars == "\r\n" || newChars == "\n" || newChars == "\r");
    newLineChars = newChars;
}

void CodeDocument::newTransaction()
{
    undoManager.beginNewTransaction (String());
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

void CodeDocument::setSavePoint() noexcept
{
    indexOfSavedState = currentActionIndex;
}

bool CodeDocument::hasChangedSinceSavePoint() const noexcept
{
    return currentActionIndex != indexOfSavedState;
}

//==============================================================================
static int getCharacterType (juce_wchar character) noexcept
{
    return (CharacterFunctions::isLetterOrDigit (character) || character == '_')
                ? 2 : (CharacterFunctions::isWhitespace (character) ? 0 : 1);
}

CodeDocument::Position CodeDocument::findWordBreakAfter (const Position& position) const noexcept
{
    auto p = position;
    const int maxDistance = 256;
    int i = 0;

    while (i < maxDistance
            && CharacterFunctions::isWhitespace (p.getCharacter())
            && (i == 0 || (p.getCharacter() != '\n'
                            && p.getCharacter() != '\r')))
    {
        ++i;
        p.moveBy (1);
    }

    if (i == 0)
    {
        auto type = getCharacterType (p.getCharacter());

        while (i < maxDistance && type == getCharacterType (p.getCharacter()))
        {
            ++i;
            p.moveBy (1);
        }

        while (i < maxDistance
                && CharacterFunctions::isWhitespace (p.getCharacter())
                && (i == 0 || (p.getCharacter() != '\n'
                                && p.getCharacter() != '\r')))
        {
            ++i;
            p.moveBy (1);
        }
    }

    return p;
}

CodeDocument::Position CodeDocument::findWordBreakBefore (const Position& position) const noexcept
{
    auto p = position;
    const int maxDistance = 256;
    int i = 0;
    bool stoppedAtLineStart = false;

    while (i < maxDistance)
    {
        auto c = p.movedBy (-1).getCharacter();

        if (c == '\r' || c == '\n')
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
        auto type = getCharacterType (p.movedBy (-1).getCharacter());

        while (i < maxDistance && type == getCharacterType (p.movedBy (-1).getCharacter()))
        {
            p.moveBy (-1);
            ++i;
        }
    }

    return p;
}

void CodeDocument::findTokenContaining (const Position& pos, Position& start, Position& end) const noexcept
{
    auto isTokenCharacter = [] (juce_wchar c)  { return CharacterFunctions::isLetterOrDigit (c) || c == '.' || c == '_'; };

    end = pos;
    while (isTokenCharacter (end.getCharacter()))
        end.moveBy (1);

    start = end;
    while (start.getIndexInLine() > 0
            && isTokenCharacter (start.movedBy (-1).getCharacter()))
        start.moveBy (-1);
}

void CodeDocument::findLineContaining  (const Position& pos, Position& s, Position& e) const noexcept
{
    s.setLineAndIndex (pos.getLineNumber(), 0);
    e.setLineAndIndex (pos.getLineNumber() + 1, 0);
}

void CodeDocument::checkLastLineStatus()
{
    while (lines.size() > 0
            && lines.getLast()->lineLength == 0
            && (lines.size() == 1 || ! lines.getUnchecked (lines.size() - 2)->endsWithLineBreak()))
    {
        // remove any empty lines at the end if the preceding line doesn't end in a newline.
        lines.removeLast();
    }

    const CodeDocumentLine* const lastLine = lines.getLast();

    if (lastLine != nullptr && lastLine->endsWithLineBreak())
    {
        // check that there's an empty line at the end if the preceding one ends in a newline..
        lines.add (new CodeDocumentLine (StringRef(), StringRef(), 0, 0,
                                         lastLine->lineStartInFile + lastLine->lineLength));
    }
}

//==============================================================================
void CodeDocument::addListener    (CodeDocument::Listener* l)   { listeners.add (l); }
void CodeDocument::removeListener (CodeDocument::Listener* l)   { listeners.remove (l); }

//==============================================================================
struct CodeDocument::InsertAction   : public UndoableAction
{
    InsertAction (CodeDocument& doc, const String& t, const int pos) noexcept
        : owner (doc), text (t), insertPos (pos)
    {
    }

    bool perform() override
    {
        owner.currentActionIndex++;
        owner.insert (text, insertPos, false);
        return true;
    }

    bool undo() override
    {
        owner.currentActionIndex--;
        owner.remove (insertPos, insertPos + text.length(), false);
        return true;
    }

    int getSizeInUnits() override        { return text.length() + 32; }

    CodeDocument& owner;
    const String text;
    const int insertPos;

    JUCE_DECLARE_NON_COPYABLE (InsertAction)
};

void CodeDocument::insert (const String& text, const int insertPos, const bool undoable)
{
    if (text.isNotEmpty())
    {
        if (undoable)
        {
            undoManager.perform (new InsertAction (*this, text, insertPos));
        }
        else
        {
            Position pos (*this, insertPos);
            auto firstAffectedLine = pos.getLineNumber();

            auto* firstLine = lines[firstAffectedLine];
            auto textInsideOriginalLine = text;

            if (firstLine != nullptr)
            {
                auto index = pos.getIndexInLine();
                textInsideOriginalLine = firstLine->line.substring (0, index)
                                         + textInsideOriginalLine
                                         + firstLine->line.substring (index);
            }

            maximumLineLength = -1;
            Array<CodeDocumentLine*> newLines;
            CodeDocumentLine::createLines (newLines, textInsideOriginalLine);
            jassert (newLines.size() > 0);

            auto* newFirstLine = newLines.getUnchecked (0);
            newFirstLine->lineStartInFile = firstLine != nullptr ? firstLine->lineStartInFile : 0;
            lines.set (firstAffectedLine, newFirstLine);

            if (newLines.size() > 1)
                lines.insertArray (firstAffectedLine + 1, newLines.getRawDataPointer() + 1, newLines.size() - 1);

            int lineStart = newFirstLine->lineStartInFile;

            for (int i = firstAffectedLine; i < lines.size(); ++i)
            {
                auto& l = *lines.getUnchecked (i);
                l.lineStartInFile = lineStart;
                lineStart += l.lineLength;
            }

            checkLastLineStatus();
            auto newTextLength = text.length();

            for (auto* p : positionsToMaintain)
                if (p->getPosition() >= insertPos)
                    p->setPosition (p->getPosition() + newTextLength);

            listeners.call ([&] (Listener& l) { l.codeDocumentTextInserted (text, insertPos); });
        }
    }
}

//==============================================================================
struct CodeDocument::DeleteAction  : public UndoableAction
{
    DeleteAction (CodeDocument& doc, int start, int end) noexcept
        : owner (doc), startPos (start), endPos (end),
          removedText (doc.getTextBetween (CodeDocument::Position (doc, start),
                                           CodeDocument::Position (doc, end)))
    {
    }

    bool perform() override
    {
        owner.currentActionIndex++;
        owner.remove (startPos, endPos, false);
        return true;
    }

    bool undo() override
    {
        owner.currentActionIndex--;
        owner.insert (removedText, startPos, false);
        return true;
    }

    int getSizeInUnits() override    { return (endPos - startPos) + 32; }

    CodeDocument& owner;
    const int startPos, endPos;
    const String removedText;

    JUCE_DECLARE_NON_COPYABLE (DeleteAction)
};

void CodeDocument::remove (const int startPos, const int endPos, const bool undoable)
{
    if (endPos <= startPos)
        return;

    if (undoable)
    {
        undoManager.perform (new DeleteAction (*this, startPos, endPos));
    }
    else
    {
        Position startPosition (*this, startPos);
        Position endPosition (*this, endPos);

        maximumLineLength = -1;
        auto firstAffectedLine = startPosition.getLineNumber();
        auto endLine = endPosition.getLineNumber();
        auto& firstLine = *lines.getUnchecked (firstAffectedLine);

        if (firstAffectedLine == endLine)
        {
            firstLine.line = firstLine.line.substring (0, startPosition.getIndexInLine())
                           + firstLine.line.substring (endPosition.getIndexInLine());
            firstLine.updateLength();
        }
        else
        {
            auto& lastLine = *lines.getUnchecked (endLine);

            firstLine.line = firstLine.line.substring (0, startPosition.getIndexInLine())
                            + lastLine.line.substring (endPosition.getIndexInLine());
            firstLine.updateLength();

            int numLinesToRemove = endLine - firstAffectedLine;
            lines.removeRange (firstAffectedLine + 1, numLinesToRemove);
        }

        for (int i = firstAffectedLine + 1; i < lines.size(); ++i)
        {
            auto& l = *lines.getUnchecked (i);
            auto& previousLine = *lines.getUnchecked (i - 1);
            l.lineStartInFile = previousLine.lineStartInFile + previousLine.lineLength;
        }

        checkLastLineStatus();
        auto totalChars = getNumCharacters();

        for (auto* p : positionsToMaintain)
        {
            if (p->getPosition() > startPosition.getPosition())
                p->setPosition (jmax (startPos, p->getPosition() + startPos - endPos));

            if (p->getPosition() > totalChars)
                p->setPosition (totalChars);
        }

        listeners.call ([=] (Listener& l) { l.codeDocumentTextDeleted (startPos, endPos); });
    }
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct CodeDocumentTest  : public UnitTest
{
    CodeDocumentTest()
        : UnitTest ("CodeDocument", UnitTestCategories::text)
    {}

    void runTest() override
    {
        const juce::String jabberwocky ("'Twas brillig, and the slithy toves\n"
                                        "Did gyre and gimble in the wabe;\n"
                                        "All mimsy were the borogoves,\n"
                                        "And the mome raths outgrabe.\n\n"

                                        "'Beware the Jabberwock, my son!\n"
                                        "The jaws that bite, the claws that catch!\n"
                                        "Beware the Jubjub bird, and shun\n"
                                        "The frumious Bandersnatch!'");

        {
            beginTest ("Basic checks");
            CodeDocument d;
            d.replaceAllContent (jabberwocky);

            expectEquals (d.getNumLines(), 9);
            expect (d.getLine (0).startsWith ("'Twas brillig"));
            expect (d.getLine (2).startsWith ("All mimsy"));
            expectEquals (d.getLine (4), String ("\n"));
        }

        {
            beginTest ("Insert/replace/delete");

            CodeDocument d;
            d.replaceAllContent (jabberwocky);

            d.insertText (CodeDocument::Position (d, 0, 6), "very ");
            expect (d.getLine (0).startsWith ("'Twas very brillig"),
                    "Insert text within a line");

            d.replaceSection (74, 83, "Quite hungry");
            expectEquals (d.getLine (2), String ("Quite hungry were the borogoves,\n"),
                          "Replace section at start of line");

            d.replaceSection (11, 18, "cold");
            expectEquals (d.getLine (0), String ("'Twas very cold, and the slithy toves\n"),
                          "Replace section within a line");

            d.deleteSection (CodeDocument::Position (d, 2, 0), CodeDocument::Position (d, 2, 6));
            expectEquals (d.getLine (2), String ("hungry were the borogoves,\n"),
                          "Delete section within a line");

            d.deleteSection (CodeDocument::Position (d, 2, 6), CodeDocument::Position (d, 5, 11));
            expectEquals (d.getLine (2), String ("hungry Jabberwock, my son!\n"),
                          "Delete section across multiple line");
        }

        {
            beginTest ("Line splitting and joining");

            CodeDocument d;
            d.replaceAllContent (jabberwocky);
            expectEquals (d.getNumLines(), 9);

            const String splitComment ("Adding a newline should split a line into two.");
            d.insertText (49, "\n");

            expectEquals (d.getNumLines(), 10, splitComment);
            expectEquals (d.getLine (1), String ("Did gyre and \n"), splitComment);
            expectEquals (d.getLine (2), String ("gimble in the wabe;\n"), splitComment);

            const String joinComment ("Removing a newline should join two lines.");
            d.deleteSection (CodeDocument::Position (d, 0, 35),
                             CodeDocument::Position (d, 1, 0));

            expectEquals (d.getNumLines(), 9, joinComment);
            expectEquals (d.getLine (0), String ("'Twas brillig, and the slithy tovesDid gyre and \n"), joinComment);
            expectEquals (d.getLine (1), String ("gimble in the wabe;\n"), joinComment);
        }

        {
            beginTest ("Undo/redo");

            CodeDocument d;
            d.replaceAllContent (jabberwocky);
            d.newTransaction();
            d.insertText (30, "INSERT1");
            d.newTransaction();
            d.insertText (70, "INSERT2");
            d.undo();

            expect (d.getAllContent().contains ("INSERT1"), "1st edit should remain.");
            expect (! d.getAllContent().contains ("INSERT2"), "2nd edit should be undone.");

            d.redo();
            expect (d.getAllContent().contains ("INSERT2"), "2nd edit should be redone.");

            d.newTransaction();
            d.deleteSection (25, 90);
            expect (! d.getAllContent().contains ("INSERT1"), "1st edit should be deleted.");
            expect (! d.getAllContent().contains ("INSERT2"), "2nd edit should be deleted.");
            d.undo();
            expect (d.getAllContent().contains ("INSERT1"), "1st edit should be restored.");
            expect (d.getAllContent().contains ("INSERT2"), "1st edit should be restored.");

            d.undo();
            d.undo();
            expectEquals (d.getAllContent(), jabberwocky, "Original document should be restored.");
        }

        {
            beginTest ("Positions");

            CodeDocument d;
            d.replaceAllContent (jabberwocky);

            {
                const String comment ("Keeps negative positions inside document.");
                CodeDocument::Position p1 (d, 0, -3);
                CodeDocument::Position p2 (d, -8);
                expectEquals (p1.getLineNumber(), 0, comment);
                expectEquals (p1.getIndexInLine(), 0, comment);
                expectEquals (p1.getCharacter(), juce_wchar ('\''), comment);
                expectEquals (p2.getLineNumber(), 0, comment);
                expectEquals (p2.getIndexInLine(), 0, comment);
                expectEquals (p2.getCharacter(), juce_wchar ('\''), comment);
            }

            {
                const String comment ("Moving by character handles newlines correctly.");
                CodeDocument::Position p1 (d, 0, 35);
                p1.moveBy (1);
                expectEquals (p1.getLineNumber(), 1, comment);
                expectEquals (p1.getIndexInLine(), 0, comment);
                p1.moveBy (75);
                expectEquals (p1.getLineNumber(), 3, comment);
            }

            {
                const String comment1 ("setPositionMaintained tracks position.");
                const String comment2 ("setPositionMaintained tracks position following undos.");

                CodeDocument::Position p1 (d, 3, 0);
                p1.setPositionMaintained (true);
                expectEquals (p1.getCharacter(), juce_wchar ('A'), comment1);

                d.newTransaction();
                d.insertText (p1, "INSERT1");

                expectEquals (p1.getCharacter(), juce_wchar ('A'), comment1);
                expectEquals (p1.getLineNumber(), 3, comment1);
                expectEquals (p1.getIndexInLine(), 7, comment1);
                d.undo();
                expectEquals (p1.getIndexInLine(), 0, comment2);

                d.newTransaction();
                d.insertText (15, "\n");

                expectEquals (p1.getLineNumber(), 4, comment1);
                d.undo();
                expectEquals (p1.getLineNumber(), 3, comment2);
            }
        }

        {
            beginTest ("Iterators");

            CodeDocument d;
            d.replaceAllContent (jabberwocky);

            {
                const String comment1 ("Basic iteration.");
                const String comment2 ("Reverse iteration.");
                const String comment3 ("Reverse iteration stops at doc start.");
                const String comment4 ("Check iteration across line boundaries.");

                CodeDocument::Iterator it (d);
                expectEquals (it.peekNextChar(), juce_wchar ('\''), comment1);
                expectEquals (it.nextChar(), juce_wchar ('\''), comment1);
                expectEquals (it.nextChar(), juce_wchar ('T'), comment1);
                expectEquals (it.nextChar(), juce_wchar ('w'), comment1);
                expectEquals (it.peekNextChar(), juce_wchar ('a'), comment2);
                expectEquals (it.previousChar(), juce_wchar ('w'), comment2);
                expectEquals (it.previousChar(), juce_wchar ('T'), comment2);
                expectEquals (it.previousChar(), juce_wchar ('\''), comment2);
                expectEquals (it.previousChar(), juce_wchar (0), comment3);
                expect (it.isSOF(), comment3);

                while (it.peekNextChar() != juce_wchar ('D')) // "Did gyre..."
                    it.nextChar();

                expectEquals (it.nextChar(), juce_wchar ('D'), comment3);
                expectEquals (it.peekNextChar(), juce_wchar ('i'), comment3);
                expectEquals (it.previousChar(), juce_wchar ('D'), comment3);
                expectEquals (it.previousChar(), juce_wchar ('\n'), comment3);
                expectEquals (it.previousChar(), juce_wchar ('s'), comment3);
            }

            {
                const String comment1 ("Iterator created from CodeDocument::Position objects.");
                const String comment2 ("CodeDocument::Position created from Iterator objects.");
                const String comment3 ("CodeDocument::Position created from EOF Iterator objects.");

                CodeDocument::Position p (d, 6, 0); // "The jaws..."
                CodeDocument::Iterator it (p);

                expectEquals (it.nextChar(), juce_wchar ('T'), comment1);
                expectEquals (it.nextChar(), juce_wchar ('h'), comment1);
                expectEquals (it.previousChar(), juce_wchar ('h'), comment1);
                expectEquals (it.previousChar(), juce_wchar ('T'), comment1);
                expectEquals (it.previousChar(), juce_wchar ('\n'), comment1);
                expectEquals (it.previousChar(), juce_wchar ('!'), comment1);

                const auto p2 = it.toPosition();
                expectEquals (p2.getLineNumber(), 5, comment2);
                expectEquals (p2.getIndexInLine(), 30, comment2);

                while (! it.isEOF())
                    it.nextChar();

                const auto p3 = it.toPosition();
                expectEquals (p3.getLineNumber(), d.getNumLines() - 1, comment3);
                expectEquals (p3.getIndexInLine(), d.getLine (d.getNumLines() - 1).length(), comment3);
            }
        }
    }
};

static CodeDocumentTest codeDocumentTests;

#endif

} // namespace juce
