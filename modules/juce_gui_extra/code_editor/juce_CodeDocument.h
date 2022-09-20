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

class CodeDocumentLine;


//==============================================================================
/**
    A class for storing and manipulating a source code file.

    When using a CodeEditorComponent, it takes one of these as its source object.

    The CodeDocument stores its content as an array of lines, which makes it
    quick to insert and delete.

    @see CodeEditorComponent

    @tags{GUI}
*/
class JUCE_API  CodeDocument
{
public:
    /** Creates a new, empty document. */
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
        /** Creates an uninitialised position.
            Don't attempt to call any methods on this until you've given it an owner document
            to refer to!
        */
        Position() noexcept;

        /** Creates a position based on a line and index in a document.

            Note that this index is NOT the column number, it's the number of characters from the
            start of the line. The "column" number isn't quite the same, because if the line
            contains any tab characters, the relationship of the index to its visual column depends on
            the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        Position (const CodeDocument& ownerDocument,
                  int line, int indexInLine) noexcept;

        /** Creates a position based on a character index in a document.
            This position is placed at the specified number of characters from the start of the
            document. The line and column are auto-calculated.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
        */
        Position (const CodeDocument& ownerDocument,
                  int charactersFromStartOfDocument) noexcept;

        /** Creates a copy of another position.

            This will copy the position, but the new object will not be set to maintain its position,
            even if the source object was set to do so.
        */
        Position (const Position&) noexcept;

        /** Destructor. */
        ~Position();

        Position& operator= (const Position&);

        bool operator== (const Position&) const noexcept;
        bool operator!= (const Position&) const noexcept;

        /** Points this object at a new position within the document.

            If the position is beyond the range of the document, it'll be adjusted to keep it
            inside.
            @see getPosition, setLineAndIndex
        */
        void setPosition (int charactersFromStartOfDocument);

        /** Returns the position as the number of characters from the start of the document.
            @see setPosition, getLineNumber, getIndexInLine
        */
        int getPosition() const noexcept            { return characterPos; }

        /** Moves the position to a new line and index within the line.

            Note that the index is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!

            Lines are numbered from zero, and if the line or index are beyond the bounds of the document,
            they will be adjusted to keep them within its limits.
        */
        void setLineAndIndex (int newLineNumber, int newIndexInLine);

        /** Returns the line number of this position.
            The first line in the document is numbered zero, not one!
        */
        int getLineNumber() const noexcept          { return line; }

        /** Returns the number of characters from the start of the line.

            Note that this value is NOT the column at which the position appears in an editor.
            If the line contains any tab characters, the relationship of the index to its
            visual position depends on the number of spaces per tab being used!
        */
        int getIndexInLine() const noexcept         { return indexInLine; }

        /** Allows the position to be automatically updated when the document changes.

            If this is set to true, the position will register with its document so that
            when the document has text inserted or deleted, this position will be automatically
            moved to keep it at the same position in the text.
        */
        void setPositionMaintained (bool isMaintained);

        //==============================================================================
        /** Moves the position forwards or backwards by the specified number of characters.
            @see movedBy
        */
        void moveBy (int characterDelta);

        /** Returns a position which is the same as this one, moved by the specified number of
            characters.
            @see moveBy
        */
        Position movedBy (int characterDelta) const;

        /** Returns a position which is the same as this one, moved up or down by the specified
            number of lines.
            @see movedBy
        */
        Position movedByLines (int deltaLines) const;

        /** Returns the character in the document at this position.
            @see getLineText
        */
        juce_wchar getCharacter() const;

        /** Returns the line from the document that this position is within.
            @see getCharacter, getLineNumber
        */
        String getLineText() const;

    private:
        CodeDocument* owner = nullptr;
        int characterPos = 0, line = 0, indexInLine = 0;
        bool positionMaintained = false;

        friend class CodeDocument;
    };

    //==============================================================================
    /** Returns the full text of the document. */
    String getAllContent() const;

    /** Returns a section of the document's text. */
    String getTextBetween (const Position& start, const Position& end) const;

    /** Returns a line from the document. */
    String getLine (int lineIndex) const noexcept;

    /** Returns the number of characters in the document. */
    int getNumCharacters() const noexcept;

    /** Returns the number of lines in the document. */
    int getNumLines() const noexcept                    { return lines.size(); }

    /** Returns the number of characters in the longest line of the document. */
    int getMaximumLineLength() noexcept;

    /** Deletes a section of the text.
        This operation is undoable.
    */
    void deleteSection (const Position& startPosition, const Position& endPosition);

    /** Deletes a section of the text.
        This operation is undoable.
    */
    void deleteSection (int startIndex, int endIndex);

    /** Inserts some text into the document at a given position.
        This operation is undoable.
    */
    void insertText (const Position& position, const String& text);

    /** Inserts some text into the document at a given position.
        This operation is undoable.
    */
    void insertText (int insertIndex, const String& text);

    /** Replaces a section of the text with a new string.
        This operation is undoable.
    */
    void replaceSection (int startIndex, int endIndex, const String& newText);

    /** Clears the document and replaces it with some new text.

        This operation is undoable - if you're trying to completely reset the document, you
        might want to also call clearUndoHistory() and setSavePoint() after using this method.
    */
    void replaceAllContent (const String& newContent);

    /** Analyses the changes between the current content and some new text, and applies
        those changes.
    */
    void applyChanges (const String& newContent);

    /** Replaces the editor's contents with the contents of a stream.
        This will also reset the undo history and save point marker.
    */
    bool loadFromStream (InputStream& stream);

    /** Writes the editor's current contents to a stream. */
    bool writeToStream (OutputStream& stream);

    //==============================================================================
    /** Returns the preferred new-line characters for the document.
        This will be either "\\n", "\\r\\n", or (rarely) "\\r".
        @see setNewLineCharacters
    */
    String getNewLineCharacters() const noexcept          { return newLineChars; }

    /** Sets the new-line characters that the document should use.
        The string must be either "\\n", "\\r\\n", or (rarely) "\\r".
        @see getNewLineCharacters
    */
    void setNewLineCharacters (const String& newLineCharacters) noexcept;

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
    UndoManager& getUndoManager() noexcept              { return undoManager; }

    //==============================================================================
    /** Makes a note that the document's current state matches the one that is saved.

        After this has been called, hasChangedSinceSavePoint() will return false until
        the document has been altered, and then it'll start returning true. If the document is
        altered, but then undone until it gets back to this state, hasChangedSinceSavePoint()
        will again return false.

        @see hasChangedSinceSavePoint
    */
    void setSavePoint() noexcept;

    /** Returns true if the state of the document differs from the state it was in when
        setSavePoint() was last called.

        @see setSavePoint
    */
    bool hasChangedSinceSavePoint() const noexcept;

    //==============================================================================
    /** Searches for a word-break. */
    Position findWordBreakAfter (const Position& position) const noexcept;
    /** Searches for a word-break. */
    Position findWordBreakBefore (const Position& position) const noexcept;
    /** Finds the token that contains the given position. */
    void findTokenContaining (const Position& pos, Position& start, Position& end) const noexcept;
    /** Finds the line that contains the given position. */
    void findLineContaining  (const Position& pos, Position& start, Position& end) const noexcept;

    //==============================================================================
    /** An object that receives callbacks from the CodeDocument when its text changes.
        @see CodeDocument::addListener, CodeDocument::removeListener
    */
    class JUCE_API  Listener
    {
    public:
        Listener() = default;
        virtual ~Listener() = default;

        /** Called by a CodeDocument when text is added. */
        virtual void codeDocumentTextInserted (const String& newText, int insertIndex) = 0;

        /** Called by a CodeDocument when text is deleted. */
        virtual void codeDocumentTextDeleted (int startIndex, int endIndex) = 0;
    };

    /** Registers a listener object to receive callbacks when the document changes.
        If the listener is already registered, this method has no effect.
        @see removeListener
    */
    void addListener (Listener* listener);

    /** Deregisters a listener.
        @see addListener
    */
    void removeListener (Listener* listener);

    //==============================================================================
    /** Iterates the text in a CodeDocument.

        This class lets you read characters from a CodeDocument. It's designed to be used
        by a CodeTokeniser object.

        @see CodeDocument
    */
    class JUCE_API  Iterator
    {
    public:
        /** Creates an uninitialised iterator.
            Don't attempt to call any methods on this until you've given it an
            owner document to refer to!
         */
        Iterator() noexcept;

        Iterator (const CodeDocument& document) noexcept;
        Iterator (CodeDocument::Position) noexcept;
        ~Iterator() noexcept;

        Iterator (const Iterator&) = default;
        Iterator& operator= (const Iterator&) = default;

        /** Reads the next character and returns it. Returns 0 if you try to
            read past the document's end.
            @see peekNextChar, previousChar
        */
        juce_wchar nextChar() noexcept;

        /** Reads the next character without moving the current position. */
        juce_wchar peekNextChar() const noexcept;

        /** Reads the previous character and returns it. Returns 0 if you try to
            read past the document's start.
            @see isSOF, peekPreviousChar, nextChar
         */
        juce_wchar previousChar() noexcept;

        /** Reads the next character without moving the current position. */
        juce_wchar peekPreviousChar() const noexcept;

        /** Advances the position by one character. */
        void skip() noexcept;

        /** Returns the position as the number of characters from the start of the document. */
        int getPosition() const noexcept        { return position; }

        /** Skips over any whitespace characters until the next character is non-whitespace. */
        void skipWhitespace() noexcept;

        /** Skips forward until the next character will be the first character on the next line */
        void skipToEndOfLine() noexcept;

        /** Skips backward until the next character will be the first character on this line */
        void skipToStartOfLine() noexcept;

        /** Returns the line number of the next character. */
        int getLine() const noexcept            { return line; }

        /** Returns true if the iterator has reached the end of the document. */
        bool isEOF() const noexcept;

        /** Returns true if the iterator is at the start of the document. */
        bool isSOF() const noexcept;

        /** Convert this iterator to a CodeDocument::Position. */
        CodeDocument::Position toPosition() const;

    private:
        bool reinitialiseCharPtr() const;

        const CodeDocument* document;
        mutable String::CharPointerType charPointer { nullptr };
        int line = 0, position = 0;
    };

private:
    //==============================================================================
    struct InsertAction;
    struct DeleteAction;
    friend class Iterator;
    friend class Position;

    OwnedArray<CodeDocumentLine> lines;
    Array<Position*> positionsToMaintain;
    UndoManager undoManager;
    int currentActionIndex = 0, indexOfSavedState = -1;
    int maximumLineLength = -1;
    ListenerList<Listener> listeners;
    String newLineChars { "\r\n" };

    void insert (const String& text, int insertPos, bool undoable);
    void remove (int startPos, int endPos, bool undoable);
    void checkLastLineStatus();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeDocument)
};

} // namespace juce
