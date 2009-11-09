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

#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"
#include "../layout/juce_ScrollBar.h"
#include "juce_CodeDocument.h"
#include "juce_CodeTokeniser.h"
class CodeEditorLine;


//==============================================================================
/**
    A text editor component designed specifically for source code.

    This is designed to handle syntax highlighting and fast editing of very large
    files.
*/
class JUCE_API  CodeEditorComponent   : public Component,
                                        public Timer,
                                        public ScrollBarListener,
                                        public CodeDocument::Listener,
                                        public AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an editor for a document.

        The tokeniser object is optional - pass 0 to disable syntax highlighting.
        The object that you pass in is not owned or deleted by the editor - you must
        make sure that it doesn't get deleted while this component is still using it.

        @see CodeDocument
    */
    CodeEditorComponent (CodeDocument& document,
                         CodeTokeniser* const codeTokeniser);

    /** Destructor. */
    ~CodeEditorComponent();

    //==============================================================================
    /** Returns the code document that this component is editing. */
    CodeDocument& getDocument() const throw()           { return document; }

    /** Loads the given content into the document.
        This will completely reset the CodeDocument object, clear its undo history,
        and fill it with this text.
    */
    void loadContent (const String& newContent);

    //==============================================================================
    /** Returns the standard character width. */
    float getCharWidth() const throw()                          { return charWidth; }

    /** Returns the height of a line of text, in pixels. */
    int getLineHeight() const throw()                           { return lineHeight; }

    /** Returns the number of whole lines visible on the screen,
        This doesn't include a cut-off line that might be visible at the bottom if the
        component's height isn't an exact multiple of the line-height.
    */
    int getNumLinesOnScreen() const throw()                     { return linesOnScreen; }

    /** Returns the number of whole columns visible on the screen.
        This doesn't include any cut-off columns at the right-hand edge.
    */
    int getNumColumnsOnScreen() const throw()                   { return columnsOnScreen; }

    /** Returns the current caret position. */
    const CodeDocument::Position getCaretPos() const            { return caretPos; }

    /** Moves the caret.
        If selecting is true, the section of the document between the current
        caret position and the new one will become selected. If false, any currently
        selected region will be deselected.
    */
    void moveCaretTo (const CodeDocument::Position& newPos, const bool selecting);

    /** Returns the on-screen position of a character in the document.
        The rectangle returned is relative to this component's top-left origin.
    */
    const Rectangle getCharacterBounds (const CodeDocument::Position& pos) const throw();

    /** Finds the character at a given on-screen position.
        The co-ordinates are relative to this component's top-left origin.
    */
    const CodeDocument::Position getPositionAt (int x, int y);

    //==============================================================================
    void cursorLeft (const bool moveInWholeWordSteps, const bool selecting);
    void cursorRight (const bool moveInWholeWordSteps, const bool selecting);
    void cursorDown (const bool selecting);
    void cursorUp (const bool selecting);

    void pageDown (const bool selecting);
    void pageUp (const bool selecting);

    void scrollDown();
    void scrollUp();
    void scrollToLine (int newFirstLineOnScreen);
    void scrollBy (int deltaLines);
    void scrollToColumn (int newFirstColumnOnScreen);
    void scrollToKeepCaretOnScreen();

    void goToStartOfDocument (const bool selecting);
    void goToStartOfLine (const bool selecting);
    void goToEndOfDocument (const bool selecting);
    void goToEndOfLine (const bool selecting);

    void deselectAll();
    void selectAll();

    void insertTextAtCaret (const String& textToInsert);
    void insertTabAtCaret();
    void cut();
    void copy();
    void copyThenCut();
    void paste();
    void backspace (const bool moveInWholeWordSteps);
    void deleteForward (const bool moveInWholeWordSteps);

    void undo();
    void redo();

    //==============================================================================
    /** Changes the current tab settings.
        This lets you change the tab size and whether pressing the tab key inserts a
        tab character, or its equivalent number of spaces.
    */
    void setTabSize (const int numSpacesPerTab,
                     const bool insertSpacesInsteadOfTabCharacters) throw();

    /** Returns the current number of spaces per tab.
        @see setTabSize
    */
    int getTabSize() const throw()                      { return spacesPerTab; }

    /** Returns true if the tab key will insert spaces instead of actual tab characters.
        @see setTabSize
    */
    bool areSpacesInsertedForTabs() const               { return useSpacesForTabs; }

    /** Changes the font.
        Make sure you only use a fixed-width font, or this component will look pretty nasty!
    */
    void setFont (const Font& newFont);

    /** Resets the syntax highlighting colours to the default ones provided by the
        code tokeniser.
        @see CodeTokeniser::getDefaultColour
    */
    void resetToDefaultColours();

    /** Changes one of the syntax highlighting colours.
        The token type values are dependent on the tokeniser being used - use
        CodeTokeniser::getTokenTypes() to get a list of the token types.
        @see getColourForTokenType
    */
    void setColourForTokenType (const int tokenType, const Colour& colour);

    /** Returns one of the syntax highlighting colours.
        The token type values are dependent on the tokeniser being used - use
        CodeTokeniser::getTokenTypes() to get a list of the token types.
        @see setColourForTokenType
    */
    const Colour getColourForTokenType (const int tokenType) const throw();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1004500,  /**< A colour to use to fill the editor's background. */
        caretColourId               = 0x1004501,  /**< The colour to draw the caret. */
        highlightColourId           = 0x1004502,  /**< The colour to use for the highlighted background under
                                                       selected text. */
        defaultTextColourId         = 0x1004503   /**< The colour to use for text when no syntax colouring is
                                                       enabled. */
    };

    //==============================================================================
    /** Changes the size of the scrollbars. */
    void setScrollbarThickness (const int thickness) throw();

    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    void mouseDoubleClick (const MouseEvent& e);
    /** @internal */
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    /** @internal */
    void timerCallback();
    /** @internal */
    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, const double newRangeStart);
    /** @internal */
    void handleAsyncUpdate();
    /** @internal */
    void codeDocumentChanged (const CodeDocument::Position& affectedTextStart,
                              const CodeDocument::Position& affectedTextEnd);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    CodeDocument& document;

    Font font;
    int firstLineOnScreen, gutter, spacesPerTab;
    float charWidth;
    int lineHeight, linesOnScreen, columnsOnScreen;
    int scrollbarThickness;
    bool useSpacesForTabs;
    double xOffset;

    CodeDocument::Position caretPos;
    CodeDocument::Position selectionStart, selectionEnd;
    Component* caret;
    ScrollBar* verticalScrollBar;
    ScrollBar* horizontalScrollBar;

    enum DragType
    {
        notDragging,
        draggingSelectionStart,
        draggingSelectionEnd
    };

    DragType dragType;

    //==============================================================================
    CodeTokeniser* codeTokeniser;
    Array <Colour> coloursForTokenCategories;

    OwnedArray <CodeEditorLine> lines;
    void rebuildLineTokens();

    OwnedArray <CodeDocument::Iterator> cachedIterators;
    void clearCachedIterators (const int firstLineToBeInvalid) throw();
    void updateCachedIterators (int maxLineNum);
    void getIteratorForPosition (int position, CodeDocument::Iterator& result);

    //==============================================================================
    void updateScrollBars();
    void scrollToLineInternal (int line);
    void scrollToColumnInternal (double column);
    void newTransaction();

    int indexToColumn (int line, int index) const throw();
    int columnToIndex (int line, int column) const throw();

    CodeEditorComponent (const CodeEditorComponent&);
    const CodeEditorComponent& operator= (const CodeEditorComponent&);
};


#endif   // __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
