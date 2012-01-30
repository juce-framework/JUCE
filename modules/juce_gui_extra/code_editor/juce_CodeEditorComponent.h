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

#ifndef __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
#define __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__

#include "juce_CodeDocument.h"
#include "juce_CodeTokeniser.h"


//==============================================================================
/**
    A text editor component designed specifically for source code.

    This is designed to handle syntax highlighting and fast editing of very large
    files.
*/
class JUCE_API  CodeEditorComponent   : public Component,
                                        public TextInputTarget,
                                        public Timer,
                                        public ScrollBar::Listener,
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
                         CodeTokeniser* codeTokeniser);

    /** Destructor. */
    ~CodeEditorComponent();

    //==============================================================================
    /** Returns the code document that this component is editing. */
    CodeDocument& getDocument() const noexcept          { return document; }

    /** Loads the given content into the document.
        This will completely reset the CodeDocument object, clear its undo history,
        and fill it with this text.
    */
    void loadContent (const String& newContent);

    //==============================================================================
    /** Returns the standard character width. */
    float getCharWidth() const noexcept                         { return charWidth; }

    /** Returns the height of a line of text, in pixels. */
    int getLineHeight() const noexcept                          { return lineHeight; }

    /** Returns the number of whole lines visible on the screen,
        This doesn't include a cut-off line that might be visible at the bottom if the
        component's height isn't an exact multiple of the line-height.
    */
    int getNumLinesOnScreen() const noexcept                    { return linesOnScreen; }

    /** Returns the number of whole columns visible on the screen.
        This doesn't include any cut-off columns at the right-hand edge.
    */
    int getNumColumnsOnScreen() const noexcept                  { return columnsOnScreen; }

    /** Returns the current caret position. */
    CodeDocument::Position getCaretPos() const                  { return caretPos; }

    /** Returns the position of the caret, relative to the editor's origin. */
    Rectangle<int> getCaretRectangle();

    /** Moves the caret.
        If selecting is true, the section of the document between the current
        caret position and the new one will become selected. If false, any currently
        selected region will be deselected.
    */
    void moveCaretTo (const CodeDocument::Position& newPos, bool selecting);

    /** Returns the on-screen position of a character in the document.
        The rectangle returned is relative to this component's top-left origin.
    */
    Rectangle<int> getCharacterBounds (const CodeDocument::Position& pos) const;

    /** Finds the character at a given on-screen position.
        The co-ordinates are relative to this component's top-left origin.
    */
    CodeDocument::Position getPositionAt (int x, int y);

    //==============================================================================
    bool moveCaretLeft (bool moveInWholeWordSteps, bool selecting);
    bool moveCaretRight (bool moveInWholeWordSteps, bool selecting);
    bool moveCaretUp (bool selecting);
    bool moveCaretDown (bool selecting);
    bool scrollDown();
    bool scrollUp();
    bool pageUp (bool selecting);
    bool pageDown (bool selecting);
    bool moveCaretToTop (bool selecting);
    bool moveCaretToStartOfLine (bool selecting);
    bool moveCaretToEnd (bool selecting);
    bool moveCaretToEndOfLine (bool selecting);
    bool deleteBackwards (bool moveInWholeWordSteps);
    bool deleteForwards (bool moveInWholeWordSteps);
    bool copyToClipboard();
    bool cutToClipboard();
    bool pasteFromClipboard();
    bool undo();
    bool redo();

    bool selectAll();
    void deselectAll();

    void scrollToLine (int newFirstLineOnScreen);
    void scrollBy (int deltaLines);
    void scrollToColumn (int newFirstColumnOnScreen);
    void scrollToKeepCaretOnScreen();

    void insertTextAtCaret (const String& textToInsert);
    void insertTabAtCaret();

    //==============================================================================
    Range<int> getHighlightedRegion() const;
    void setHighlightedRegion (const Range<int>& newRange);
    String getTextInRange (const Range<int>& range) const;

    //==============================================================================
    /** Changes the current tab settings.
        This lets you change the tab size and whether pressing the tab key inserts a
        tab character, or its equivalent number of spaces.
    */
    void setTabSize (int numSpacesPerTab, bool insertSpacesInsteadOfTabCharacters);

    /** Returns the current number of spaces per tab.
        @see setTabSize
    */
    int getTabSize() const noexcept                     { return spacesPerTab; }

    /** Returns true if the tab key will insert spaces instead of actual tab characters.
        @see setTabSize
    */
    bool areSpacesInsertedForTabs() const               { return useSpacesForTabs; }

    /** Changes the font.
        Make sure you only use a fixed-width font, or this component will look pretty nasty!
    */
    void setFont (const Font& newFont);

    /** Returns the font that the editor is using. */
    const Font& getFont() const noexcept                { return font; }

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
    void setColourForTokenType (int tokenType, const Colour& colour);

    /** Returns one of the syntax highlighting colours.
        The token type values are dependent on the tokeniser being used - use
        CodeTokeniser::getTokenTypes() to get a list of the token types.
        @see setColourForTokenType
    */
    Colour getColourForTokenType (int tokenType) const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1004500,  /**< A colour to use to fill the editor's background. */
        highlightColourId           = 0x1004502,  /**< The colour to use for the highlighted background under
                                                       selected text. */
        defaultTextColourId         = 0x1004503   /**< The colour to use for text when no syntax colouring is
                                                       enabled. */
    };

    //==============================================================================
    /** Changes the size of the scrollbars. */
    void setScrollbarThickness (int thickness);

    /** Returns the thickness of the scrollbars. */
    int getScrollbarThickness() const noexcept          { return scrollbarThickness; }

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
    void focusGained (FocusChangeType cause);
    /** @internal */
    void focusLost (FocusChangeType cause);
    /** @internal */
    void timerCallback();
    /** @internal */
    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart);
    /** @internal */
    void handleAsyncUpdate();
    /** @internal */
    void codeDocumentChanged (const CodeDocument::Position& affectedTextStart,
                              const CodeDocument::Position& affectedTextEnd);
    /** @internal */
    bool isTextInputActive() const;
    /** @internal */
    void setTemporaryUnderlining (const Array <Range<int> >&);

private:
    //==============================================================================
    CodeDocument& document;

    Font font;
    int firstLineOnScreen, gutter, spacesPerTab;
    float charWidth;
    int lineHeight, linesOnScreen, columnsOnScreen;
    int scrollbarThickness, columnToTryToMaintain;
    bool useSpacesForTabs;
    double xOffset;

    CodeDocument::Position caretPos;
    CodeDocument::Position selectionStart, selectionEnd;

    ScopedPointer<CaretComponent> caret;
    ScrollBar verticalScrollBar, horizontalScrollBar;

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

    class CodeEditorLine;
    OwnedArray <CodeEditorLine> lines;
    void rebuildLineTokens();

    OwnedArray <CodeDocument::Iterator> cachedIterators;
    void clearCachedIterators (int firstLineToBeInvalid);
    void updateCachedIterators (int maxLineNum);
    void getIteratorForPosition (int position, CodeDocument::Iterator& result);
    void moveLineDelta (int delta, bool selecting);

    //==============================================================================
    void updateCaretPosition();
    void updateScrollBars();
    void scrollToLineInternal (int line);
    void scrollToColumnInternal (double column);
    void newTransaction();
    void cut();

    int indexToColumn (int line, int index) const noexcept;
    int columnToIndex (int line, int column) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorComponent);
};


#endif   // __JUCE_CODEEDITORCOMPONENT_JUCEHEADER__
