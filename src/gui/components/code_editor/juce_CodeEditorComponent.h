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
                                        public CodeDocument::Listener
{
public:
    //==============================================================================
    CodeEditorComponent (CodeDocument& document, CodeTokeniser* const codeTokeniser);
    ~CodeEditorComponent();

    //==============================================================================
    CodeDocument& getDocument() const throw()           { return document; }

    void loadContent (const String& newContent);

    //==============================================================================
    void insertTextAtCaret (const String& newText);
    void insertTabAtCaret();
    void cut();
    void copy();
    void copyThenCut();
    void paste();
    void backspace (const bool moveInWholeWordSteps);
    void deleteForward (const bool moveInWholeWordSteps);

    void cursorLeft (const bool moveInWholeWordSteps, const bool selecting);
    void cursorRight (const bool moveInWholeWordSteps, const bool selecting);
    void cursorDown (const bool selecting);
    void cursorUp (const bool selecting);
    void pageDown (const bool selecting);
    void pageUp (const bool selecting);
    void scrollDown();
    void scrollUp();
    void goToStart (const bool selecting);
    void goToStartOfLine (const bool selecting);
    void goToEnd (const bool selecting);
    void goToEndOfLine (const bool selecting);
    void selectAll();

    void undo();
    void redo();

    //==============================================================================
    float getCharWidth() const throw()                          { return charWidth; }
    int getLineHeight() const throw()                           { return lineHeight; }
    int getNumLinesOnScreen() const throw()                     { return linesOnScreen; }
    int getNumColumnsOnScreen() const throw()                   { return columnsOnScreen; }

    const CodeDocument::Position getCaretPos() const            { return caretPos; }
    void moveCaretTo (const CodeDocument::Position& newPos, const bool highlighting);

    void deselectAll();
    void scrollToLine (int firstLineOnScreen);
    void scrollToColumn (int firstColumnOnScreen);
    void scrollBy (int deltaLines);
    void scrollToKeepCaretOnScreen();

    const Rectangle getCharacterBounds (const CodeDocument::Position& pos) const throw();
    const CodeDocument::Position getPositionAt (int x, int y);

    //==============================================================================
    void setTabSize (const int numSpaces, const bool insertSpaces);
    int getTabSize() const throw()                      { return spacesPerTab; }
    bool areSpacesInsertedForTabs() const               { return useSpacesForTabs; }

    void setFont (const Font& newFont);

    void setDefaultColours();
    void setColourForTokenCategory (const int tokenCategory, const Colour& colour);
    const Colour getColourForTokenCategory (const int tokenCategory) const;

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
    void resized();
    void paint (Graphics& g);
    bool keyPressed (const KeyPress& key);
    void mouseDown (const MouseEvent& e);
    void mouseDrag (const MouseEvent& e);
    void mouseUp (const MouseEvent& e);
    void mouseDoubleClick (const MouseEvent& e);
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    void timerCallback();
    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, const double newRangeStart);
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
