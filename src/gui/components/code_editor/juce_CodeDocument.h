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

#ifndef __JUCE_CODEDOCUMENT_JUCEHEADER__
#define __JUCE_CODEDOCUMENT_JUCEHEADER__

#include "../../../utilities/juce_UndoManager.h"
#include "../../graphics/colour/juce_Colour.h"
#include "../../../containers/juce_VoidArray.h"
class CodeDocumentLine;


//==============================================================================
/**
    A class for storing and manipulating a source code file.

    When using a CodeEditorComponent, it takes one of these as its source object.

    The CodeDocument stores its content as an array of lines, which makes it
    quick to insert and delete.

    @see CodeEditorComponent
*/
class JUCE_API  CodeDocument
{
public:
    /** Creates a new, empty document.
    */
    CodeDocument();

    /** Destructor. */
    ~CodeDocument();

    //==============================================================================
    /** A position in a code document.

        Using this class you can find a position in a code document and quickly get its
        character position, line, and index. By calling setPositionMaintained (true), the
        position is automatically updated when text is inserted or deleted in the document,
        so that it maintains its original place in the text.
    */
    class JUCE_API  Position
    {
    public:
        /** Creates an uninitialised postion.
            Don't attempt to call any methods on this until you've given it an owner document
            to refer to!
        */
        Position() throw();

        /** Creates a position based on a line and index in a document.

            Note that this index is NOT the column number, it's the number of characters from the
            start of the line. The "column" number isn't quite the same, because if the line
            contains any tab characters, the relationship of the index to its visual column depends on
            the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        Position (const CodeDocument* const ownerDocument,
                  const int line, const int indexInLine) throw();

        /** Creates a position based on a character index in a document.
            This position is placed at the specified number of characters from the start of the
            document. The line and column are auto-calculated.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
        */
        Position (const CodeDocument* const ownerDocument,
                  const int charactersFromStartOfDocument) throw();

        /** Creates a copy of another position.

            This will copy the position, but the new object will not be set to maintain its position,
            even if the source object was set to do so.
        */
        Position (const Position& other) throw();

        /** Destructor. */
        ~Position() throw();

        const Position& operator= (const Position& other) throw();
        bool operator== (const Position& other) const throw();
        bool operator!= (const Position& other) const throw();

        /** Points this object at a new position within the document.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
            @see getPosition, setLineAndIndex
        */
        void setPosition (const int charactersFromStartOfDocument) throw();

        /** Returns the position as the number of characters from the start of the document.
            @see setPosition, getLineNumber, getIndexInLine
        */
        int getPosition() const throw()             { return characterPos; }

        /** Moves the position to a new line and index within the line.

            Note that the index is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        void setLineAndIndex (const int newLine, const int newIndexInLine) throw();

        /** Returns the line number of this position.
            The first line in the document is numbered zero, not one!
        */
        int getLineNumber() const throw()           { return line; }

        /** Returns the number of characters from the start of the line.

            Note that this value is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!
        */
        int getIndexInLine() const throw()          { return indexInLine; }

        /** Allows the position to be automatically updated when the document changes.

            If this is set to true, the positon will register with its document so that
            when the document has text inserted or deleted, this position will be automatically
            moved to keep it at the same position in the text.
        */
        void setPositionMaintained (const bool isMaintained) throw();

        //==============================================================================
        /** Moves the position forwards or backwards by the specified number of characters.
            @see movedBy
        */
        void moveBy (int characterDelta) throw();

        /** Returns a position which is the same as this one, moved by the specified number of
            characters.
            @see moveBy
        */
        const Position movedBy (const int characterDelta) const throw();

        /** Returns a position which is the same as this one, moved up or down by the specified
            number of lines.
            @see movedBy
        */
        const Position movedByLines (const int deltaLines) const throw();

        /** Returns the character in the document at this position.
            @see getLineText
        */
        const tchar getCharacter() const throw();

        /** Returns the line from the document that this position is within.
            @see getCharacter, getLineNumber
        */
        const String getLineText() const throw();

        //==============================================================================
    private:
        CodeDocument* owner;
        int characterPos, line, indexInLine;
        bool positionMaintained;
    };

    //==============================================================================
    /** Returns the full text of the document. */
    const String getAllContent() const throw();

    /** Returns a section of the document's text. */
    const String getTextBetween (const Position& start, const Position& end) const throw();

    /** Returns a line from the document. */
    const String getLine (const int lineIndex) const throw();

    /** Returns the number of characters in the document. */
    int getNumCharacters() const throw();

    /** Returns the number of lines in the document. */
    int getNumLines() const throw()                     { return lines.size(); }

    /** Returns the number of characters in the longest line of the document. */
    int getMaximumLineLength() throw();

    /** Deletes a section of the text.

        This operation is undoable.
    */
    void deleteSection (const Position& startPosition, const Position& endPosition);

    /** Inserts some text into the document at a given position.

        This operation is undoable.
    */
    void insertText (const Position& position, const String& text);

    /** Clears the document and replaces it with some new text.

        This operation is undoable - if you're trying to completely reset the document, you
        might want to also call clearUndoHistory() and setSavePoint() after using this method.
    */
    void replaceAllContent (const String& newContent);

    //==============================================================================
    /** Returns the preferred new-line characters for the document.
        This will be either "\n", "\r\n", or (rarely) "\r".
        @see setNewLineCharacters
    */
    const String getNewLineCharacters() const throw()           { return newLineChars; }

    /** Sets the new-line characters that the document should use.
        The string must be either "\n", "\r\n", or (rarely) "\r".
        @see getNewLineCharacters
    */
    void setNewLineCharacters (const String& newLine) throw();

    //==============================================================================
    /** Begins a new undo transaction.

        The document itself will not call this internally, so relies on whatever is using the
        document to periodically call this to break up the undo sequence into sensible chunks.
        @see UndoManager::beginNewTransaction
    */
    void newTransaction();

    /** Undo the last operation.
        @see UndoManager::undo
    */
    void undo();

    /** Redo the last operation.
        @see UndoManager::redo
    */
    void redo();

    /** Clears the undo history.
        @see UndoManager::clearUndoHistory
    */
    void clearUndoHistory();

    /** Returns the document's UndoManager */
    UndoManager& getUndoManager() throw()               { return undoManager; }

    //==============================================================================
    /** Makes a note that the document's current state matches the one that is saved.

        After this has been called, hasChangedSinceSavePoint() will return false until
        the document has been altered, and then it'll start returning true. If the document is
        altered, but then undone until it gets back to this state, hasChangedSinceSavePoint()
        will again return false.

        @see hasChangedSinceSavePoint
    */
    void setSavePoint() throw();

    /** Returns true if the state of the document differs from the state it was in when
        setSavePoint() was last called.

        @see setSavePoint
    */
    bool hasChangedSinceSavePoint() const throw();

    //==============================================================================
    /** Searches for a word-break. */
    const Position findWordBreakAfter (const Position& position) const throw();

    /** Searches for a word-break. */
    const Position findWordBreakBefore (const Position& position) const throw();

    //==============================================================================
    /** An object that receives callbacks from the CodeDocument when its text changes.
        @see CodeDocument::addListener, CodeDocument::removeListener
    */
    class JUCE_API  Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}

        /** Called by a CodeDocument when it is altered.
        */
        virtual void codeDocumentChanged (const Position& affectedTextStart,
                                          const Position& affectedTextEnd) = 0;
    };

    /** Registers a listener object to receive callbacks when the document changes.
        If the listener is already registered, this method has no effect.
        @see removeListener
    */
    void addListener (Listener* const listener) throw();

    /** Deregisters a listener.
        @see addListener
    */
    void removeListener (Listener* const listener) throw();

    //==============================================================================
    /** Iterates the text in a CodeDocument.

        This class lets you read characters from a CodeDocument. It's designed to be used
        by a SyntaxAnalyser object.

        @see CodeDocument, SyntaxAnalyser
    */
    class Iterator
    {
    public:
        Iterator (CodeDocument* const document) throw();
        Iterator (const Iterator& other);
        const Iterator& operator= (const Iterator& other) throw();
        ~Iterator() throw();

        /** Reads the next character and returns it.
            @see peekNextChar
        */
        juce_wchar nextChar() throw();

        /** Reads the next character without advancing the current position. */
        juce_wchar peekNextChar() const throw();

        /** Advances the position by one character. */
        void skip() throw();

        /** Returns the position of the next character as its position within the
            whole document.
        */
        int getPosition() const throw()         { return position; }

        /** Skips over any whitespace characters until the next character is non-whitespace. */
        void skipWhitespace();

        /** Skips forward until the next character will be the first character on the next line */
        void skipToEndOfLine();

        /** Returns the line number of the next character. */
        int getLine() const throw()             { return line; }

        /** Returns true if the iterator has reached the end of the document. */
        bool isEOF() const throw();

    private:
        CodeDocument* document;
        int line, position;
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class CodeDocumentInsertAction;
    friend class CodeDocumentDeleteAction;
    friend class Iterator;
    friend class Position;

    OwnedArray <CodeDocumentLine> lines;
    Array <Position*> positionsToMaintain;
    UndoManager undoManager;
    int currentActionIndex, indexOfSavedState;
    int maximumLineLength;
    VoidArray listeners;
    String newLineChars;

    void sendListenerChangeMessage (const int startLine, const int endLine);

    void insert (const String& text, const int insertPos, const bool undoable);
    void remove (const int startPos, const int endPos, const bool undoable);

    CodeDocument (const CodeDocument&);
    const CodeDocument& operator= (const CodeDocument&);
};


#endif   // __JUCE_CODEDOCUMENT_JUCEHEADER__
