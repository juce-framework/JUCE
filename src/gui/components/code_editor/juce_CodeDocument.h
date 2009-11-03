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
    /**
    */
    CodeDocument();

    /**
    */
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
        Position() throw();
        Position (const CodeDocument* const ownerDocument, const int line, const int indexInLine) throw();
        Position (const CodeDocument* const ownerDocument, const int characterPos) throw();
        Position (const Position& other) throw();
        ~Position() throw();

        const Position& operator= (const Position& other) throw();
        bool operator== (const Position& other) const throw();
        bool operator!= (const Position& other) const throw();

        /**
        */
        void setLineAndIndex (const int newLine, const int newIndexInLine) throw();

        /**
        */
        void setPosition (const int newPosition) throw();

        /**
        */
        int getPosition() const throw()             { return characterPos; }

        /**
        */
        int getLineNumber() const throw()           { return line; }

        /**
        */
        int getIndexInLine() const throw()          { return indexInLine; }

        /**
        */
        void updateLineAndIndexFromPosition() throw();

        /**
        */
        void setPositionMaintained (const bool isMaintained) throw();

        //==============================================================================
        /**
        */
        void moveBy (int characterDelta) throw();

        /**
        */
        const Position movedBy (const int characterDelta) const throw();

        /**
        */
        const Position movedByLines (const int deltaLines) const throw();

        /**
        */
        const tchar getCharacter() const throw();

        /**
        */
        const String getLineText() const throw();

        //==============================================================================
    private:
        CodeDocument* owner;
        int characterPos, line, indexInLine;
        bool positionMaintained;
    };

    //==============================================================================
    /**
    */
    const String getAllContent() const throw();

    /**
    */
    const String getTextBetween (const Position& start, const Position& end) const throw();

    /**
    */
    const String getLine (const int lineIndex) const throw();

    /**
    */
    int getNumCharacters() const throw();

    /**
    */
    int getNumLines() const throw()                     { return lines.size(); }

    /**
    */
    int getMaximumLineLength() throw();

    /**
    */
    void deleteSection (const Position& startPosition, const Position& endPosition);

    /**
    */
    void insertText (const Position& position, const String& text);

    /**
    */
    void replaceAllContent (const String& newContent);

    //==============================================================================
    /**
    */
    void newTransaction();

    /**
    */
    void undo();

    /**
    */
    void redo();

    /**
    */
    void clearUndoManager();

    //==============================================================================
    /**
    */
    void setSavePoint() throw();

    /**
    */
    bool hasChangedSinceSavePoint() const throw();

    //==============================================================================
    /**
    */
    const Position findWordBreakAfter (const Position& position) const throw();

    /**
    */
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

        /**
        */
        virtual void codeDocumentChanged (const CodeDocument::Position& affectedTextStart,
                                          const CodeDocument::Position& affectedTextEnd) = 0;
    };

    /**
    */
    void addListener (Listener* const listener);

    /**
    */
    void removeListener (Listener* const listener);

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

        /**
        */
        juce_wchar nextChar() throw();

        /**
        */
        juce_wchar peekNextChar() const throw();

        /**
        */
        void skip() throw();

        /**
        */
        int getPosition() const throw()         { return position; }

        /**
        */
        void skipWhitespace();

        /**
        */
        void skipToEndOfLine();

        /**
        */
        int getLine() const throw()             { return line; }

        /**
        */
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
    friend class CodeDocument::Iterator;

    OwnedArray <CodeDocumentLine> lines;
    Array <Position*> positionsToMaintain;
    UndoManager undoManager;
    int currentActionIndex, indexOfSavedState;
    int maximumLineLength;
    VoidArray listeners;

    void sendListenerChangeMessage (const int startLine, const int endLine);

    void insert (const String& text, const int insertPos, const bool undoable);
    void remove (const int startPos, const int endPos, const bool undoable);

    CodeDocument (const CodeDocument&);
    const CodeDocument& operator= (const CodeDocument&);
};


#endif   // __JUCE_CODEDOCUMENT_JUCEHEADER__
