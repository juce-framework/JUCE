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

#ifndef __JUCE_TEXTEDITOR_JUCEHEADER__
#define __JUCE_TEXTEDITOR_JUCEHEADER__

#include "../juce_Component.h"
#include "../../../events/juce_Timer.h"
#include "../../../utilities/juce_UndoManager.h"
#include "../layout/juce_Viewport.h"
#include "../menus/juce_PopupMenu.h"
#include "../../../containers/juce_Value.h"
class TextEditor;
class TextHolderComponent;


//==============================================================================
/**
    Receives callbacks from a TextEditor component when it changes.

    @see TextEditor::addListener
*/
class JUCE_API  TextEditorListener
{
public:
    /** Destructor. */
    virtual ~TextEditorListener()  {}

    /** Called when the user changes the text in some way. */
    virtual void textEditorTextChanged (TextEditor& editor) = 0;

    /** Called when the user presses the return key. */
    virtual void textEditorReturnKeyPressed (TextEditor& editor) = 0;

    /** Called when the user presses the escape key. */
    virtual void textEditorEscapeKeyPressed (TextEditor& editor) = 0;

    /** Called when the text editor loses focus. */
    virtual void textEditorFocusLost (TextEditor& editor) = 0;
};


//==============================================================================
/**
    A component containing text that can be edited.

    A TextEditor can either be in single- or multi-line mode, and supports mixed
    fonts and colours.

    @see TextEditorListener, Label
*/
class JUCE_API  TextEditor  : public Component,
                              public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates a new, empty text editor.

        @param componentName        the name to pass to the component for it to use as its name
        @param passwordCharacter    if this is not zero, this character will be used as a replacement
                                    for all characters that are drawn on screen - e.g. to create
                                    a password-style textbox containing circular blobs instead of text,
                                    you could set this value to 0x25cf, which is the unicode character
                                    for a black splodge (not all fonts include this, though), or 0x2022,
                                    which is a bullet (probably the best choice for linux).
    */
    TextEditor (const String& componentName = String::empty,
                const tchar passwordCharacter = 0);

    /** Destructor. */
    virtual ~TextEditor();


    //==============================================================================
    /** Puts the editor into either multi- or single-line mode.

        By default, the editor will be in single-line mode, so use this if you need a multi-line
        editor.

        See also the setReturnKeyStartsNewLine() method, which will also need to be turned
        on if you want a multi-line editor with line-breaks.

        @see isMultiLine, setReturnKeyStartsNewLine
    */
    void setMultiLine (const bool shouldBeMultiLine,
                       const bool shouldWordWrap = true);

    /** Returns true if the editor is in multi-line mode.
    */
    bool isMultiLine() const;

    //==============================================================================
    /** Changes the behaviour of the return key.

        If set to true, the return key will insert a new-line into the text; if false
        it will trigger a call to the TextEditorListener::textEditorReturnKeyPressed()
        method. By default this is set to false, and when true it will only insert
        new-lines when in multi-line mode (see setMultiLine()).
    */
    void setReturnKeyStartsNewLine (const bool shouldStartNewLine);

    /** Returns the value set by setReturnKeyStartsNewLine().

        See setReturnKeyStartsNewLine() for more info.
    */
    bool getReturnKeyStartsNewLine() const                      { return returnKeyStartsNewLine; }

    /** Indicates whether the tab key should be accepted and used to input a tab character,
        or whether it gets ignored.

        By default the tab key is ignored, so that it can be used to switch keyboard focus
        between components.
    */
    void setTabKeyUsedAsCharacter (const bool shouldTabKeyBeUsed);

    /** Returns true if the tab key is being used for input.
        @see setTabKeyUsedAsCharacter
    */
    bool isTabKeyUsedAsCharacter() const                        { return tabKeyUsed; }

    //==============================================================================
    /** Changes the editor to read-only mode.

        By default, the text editor is not read-only. If you're making it read-only, you
        might also want to call setCaretVisible (false) to get rid of the caret.

        The text can still be highlighted and copied when in read-only mode.

        @see isReadOnly, setCaretVisible
    */
    void setReadOnly (const bool shouldBeReadOnly);

    /** Returns true if the editor is in read-only mode.
    */
    bool isReadOnly() const;

    //==============================================================================
    /** Makes the caret visible or invisible.

        By default the caret is visible.

        @see setCaretColour, setCaretPosition
    */
    void setCaretVisible (const bool shouldBeVisible);

    /** Returns true if the caret is enabled.
        @see setCaretVisible
    */
    bool isCaretVisible() const                                 { return caretVisible; }

    //==============================================================================
    /** Enables/disables a vertical scrollbar.

        (This only applies when in multi-line mode). When the text gets too long to fit
        in the component, a scrollbar can appear to allow it to be scrolled. Even when
        this is enabled, the scrollbar will be hidden unless it's needed.

        By default the scrollbar is enabled.
    */
    void setScrollbarsShown (bool shouldBeEnabled);

    /** Returns true if scrollbars are enabled.
        @see setScrollbarsShown
    */
    bool areScrollbarsShown() const                             { return scrollbarVisible; }


    /** Changes the password character used to disguise the text.

        @param passwordCharacter    if this is not zero, this character will be used as a replacement
                                    for all characters that are drawn on screen - e.g. to create
                                    a password-style textbox containing circular blobs instead of text,
                                    you could set this value to 0x25cf, which is the unicode character
                                    for a black splodge (not all fonts include this, though), or 0x2022,
                                    which is a bullet (probably the best choice for linux).
    */
    void setPasswordCharacter (const tchar passwordCharacter);

    /** Returns the current password character.
        @see setPasswordCharacter
l    */
    tchar getPasswordCharacter() const                          { return passwordCharacter; }


    //==============================================================================
    /** Allows a right-click menu to appear for the editor.

        (This defaults to being enabled).

        If enabled, right-clicking (or command-clicking on the Mac) will pop up a menu
        of options such as cut/copy/paste, undo/redo, etc.
    */
    void setPopupMenuEnabled (const bool menuEnabled);

    /** Returns true if the right-click menu is enabled.
        @see setPopupMenuEnabled
    */
    bool isPopupMenuEnabled() const                                 { return popupMenuEnabled; }

    /** Returns true if a popup-menu is currently being displayed.
    */
    bool isPopupMenuCurrentlyActive() const                         { return menuActive; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId       = 0x1000200, /**< The colour to use for the text component's background - this can be
                                                   transparent if necessary. */

        textColourId             = 0x1000201, /**< The colour that will be used when text is added to the editor. Note
                                                   that because the editor can contain multiple colours, calling this
                                                   method won't change the colour of existing text - to do that, call
                                                   applyFontToAllText() after calling this method.*/

        highlightColourId        = 0x1000202, /**< The colour with which to fill the background of highlighted sections of
                                                   the text - this can be transparent if you don't want to show any
                                                   highlighting.*/

        highlightedTextColourId  = 0x1000203, /**< The colour with which to draw the text in highlighted sections. */

        caretColourId            = 0x1000204, /**< The colour with which to draw the caret. */

        outlineColourId          = 0x1000205, /**< If this is non-transparent, it will be used to draw a box around
                                                   the edge of the component. */

        focusedOutlineColourId   = 0x1000206, /**< If this is non-transparent, it will be used to draw a box around
                                                   the edge of the component when it has focus. */

        shadowColourId           = 0x1000207, /**< If this is non-transparent, it'll be used to draw an inner shadow
                                                   around the edge of the editor. */
    };

    //==============================================================================
    /** Sets the font to use for newly added text.

        This will change the font that will be used next time any text is added or entered
        into the editor. It won't change the font of any existing text - to do that, use
        applyFontToAllText() instead.

        @see applyFontToAllText
    */
    void setFont (const Font& newFont);

    /** Applies a font to all the text in the editor.

        This will also set the current font to use for any new text that's added.

        @see setFont
    */
    void applyFontToAllText (const Font& newFont);

    /** Returns the font that's currently being used for new text.

        @see setFont
    */
    const Font getFont() const;

    //==============================================================================
    /** If set to true, focusing on the editor will highlight all its text.

        (Set to false by default).

        This is useful for boxes where you expect the user to re-enter all the
        text when they focus on the component, rather than editing what's already there.
    */
    void setSelectAllWhenFocused (const bool b);

    /** Sets limits on the characters that can be entered.

        @param maxTextLength        if this is > 0, it sets a maximum length limit; if 0, no
                                    limit is set
        @param allowedCharacters    if this is non-empty, then only characters that occur in
                                    this string are allowed to be entered into the editor.
    */
    void setInputRestrictions (const int maxTextLength,
                               const String& allowedCharacters = String::empty);

    /** When the text editor is empty, it can be set to display a message.

        This is handy for things like telling the user what to type in the box - the
        string is only displayed, it's not taken to actually be the contents of
        the editor.
    */
    void setTextToShowWhenEmpty (const String& text, const Colour& colourToUse);

    //==============================================================================
    /** Changes the size of the scrollbars that are used.

        Handy if you need smaller scrollbars for a small text box.
    */
    void setScrollBarThickness (const int newThicknessPixels);

    /** Shows or hides the buttons on any scrollbars that are used.

        @see ScrollBar::setButtonVisibility
    */
    void setScrollBarButtonVisibility (const bool buttonsVisible);

    //==============================================================================
    /** Registers a listener to be told when things happen to the text.

        @see removeListener
    */
    void addListener (TextEditorListener* const newListener);

    /** Deregisters a listener.

        @see addListener
    */
    void removeListener (TextEditorListener* const listenerToRemove);

    //==============================================================================
    /** Returns the entire contents of the editor. */
    const String getText() const;

    /** Returns a section of the contents of the editor. */
    const String getTextSubstring (const int startCharacter, const int endCharacter) const;

    /** Returns true if there are no characters in the editor.

        This is more efficient than calling getText().isEmpty().
    */
    bool isEmpty() const;

    /** Sets the entire content of the editor.

        This will clear the editor and insert the given text (using the current text colour
        and font). You can set the current text colour using
        @code setColour (TextEditor::textColourId, ...);
        @endcode

        @param newText                  the text to add
        @param sendTextChangeMessage    if true, this will cause a change message to
                                        be sent to all the listeners.
        @see insertText
    */
    void setText (const String& newText,
                  const bool sendTextChangeMessage = true);

    /** Returns a Value object that can be used to get or set the text.

        Bear in mind that this operate quite slowly if your text box contains large
        amounts of text, as it needs to dynamically build the string that's involved. It's
        best used for small text boxes.
    */
    Value& getTextValue();

    /** Inserts some text at the current cursor position.

        If a section of the text is highlighted, it will be replaced by
        this string, otherwise it will be inserted.

        To delete a section of text, you can use setHighlightedRegion() to
        highlight it, and call insertTextAtCursor (String::empty).

        @see setCaretPosition, getCaretPosition, setHighlightedRegion
    */
    void insertTextAtCursor (String textToInsert);

    /** Deletes all the text from the editor. */
    void clear();

    /** Deletes the currently selected region, and puts it on the clipboard.

        @see copy, paste, SystemClipboard
    */
    void cut();

    /** Copies any currently selected region to the clipboard.

        @see cut, paste, SystemClipboard
    */
    void copy();

    /** Pastes the contents of the clipboard into the editor at the cursor position.

        @see cut, copy, SystemClipboard
    */
    void paste();

    //==============================================================================
    /** Moves the caret to be in front of a given character.

        @see getCaretPosition
    */
    void setCaretPosition (const int newIndex);

    /** Returns the current index of the caret.

        @see setCaretPosition
    */
    int getCaretPosition() const;

    /** Attempts to scroll the text editor so that the caret ends up at
        a specified position.

        This won't affect the caret's position within the text, it tries to scroll
        the entire editor vertically and horizontally so that the caret is sitting
        at the given position (relative to the top-left of this component).

        Depending on the amount of text available, it might not be possible to
        scroll far enough for the caret to reach this exact position, but it
        will go as far as it can in that direction.
    */
    void scrollEditorToPositionCaret (const int desiredCaretX,
                                      const int desiredCaretY);

    /** Get the graphical position of the caret.

        The rectangle returned is relative to the component's top-left corner.
        @see scrollEditorToPositionCaret
    */
    const Rectangle getCaretRectangle();

    /** Selects a section of the text.
    */
    void setHighlightedRegion (int startIndex,
                               int numberOfCharactersToHighlight);

    /** Returns the first character that is selected.

        If nothing is selected, this will still return a character index, but getHighlightedRegionLength()
        will return 0.

        @see setHighlightedRegion, getHighlightedRegionLength
    */
    int getHighlightedRegionStart() const                           { return selectionStart; }

    /** Returns the number of characters that are selected.

        @see setHighlightedRegion, getHighlightedRegionStart
    */
    int getHighlightedRegionLength() const                          { return jmax (0, selectionEnd - selectionStart); }

    /** Returns the section of text that is currently selected. */
    const String getHighlightedText() const;

    /** Finds the index of the character at a given position.

        The co-ordinates are relative to the component's top-left.
    */
    int getTextIndexAt (const int x, const int y);

    /** Counts the number of characters in the text.

        This is quicker than getting the text as a string if you just need to know
        the length.
    */
    int getTotalNumChars() const;

    /** Returns the total width of the text, as it is currently laid-out.

        This may be larger than the size of the TextEditor, and can change when
        the TextEditor is resized or the text changes.
    */
    int getTextWidth() const;

    /** Returns the maximum height of the text, as it is currently laid-out.

        This may be larger than the size of the TextEditor, and can change when
        the TextEditor is resized or the text changes.
    */
    int getTextHeight() const;

    /** Changes the size of the gap at the top and left-edge of the editor.

        By default there's a gap of 4 pixels.
    */
    void setIndents (const int newLeftIndent, const int newTopIndent);

    /** Changes the size of border left around the edge of the component.

        @see getBorder
    */
    void setBorder (const BorderSize& border);

    /** Returns the size of border around the edge of the component.

        @see setBorder
    */
    const BorderSize getBorder() const;

    /** Used to disable the auto-scrolling which keeps the cursor visible.

        If true (the default), the editor will scroll when the cursor moves offscreen. If
        set to false, it won't.
    */
    void setScrollToShowCursor (const bool shouldScrollToShowCursor);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void paintOverChildren (Graphics& g);
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseDoubleClick (const MouseEvent& e);
    /** @internal */
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    bool keyStateChanged (const bool isKeyDown);
    /** @internal */
    void focusGained (FocusChangeType cause);
    /** @internal */
    void focusLost (FocusChangeType cause);
    /** @internal */
    void resized();
    /** @internal */
    void enablementChanged();
    /** @internal */
    void colourChanged();

    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    /** This adds the items to the popup menu.

        By default it adds the cut/copy/paste items, but you can override this if
        you need to replace these with your own items.

        If you want to add your own items to the existing ones, you can override this,
        call the base class's addPopupMenuItems() method, then append your own items.

        When the menu has been shown, performPopupMenuAction() will be called to
        perform the item that the user has chosen.

        The default menu items will be added using item IDs in the range
        0x7fff0000 - 0x7fff1000, so you should avoid those values for your own
        menu IDs.

        If this was triggered by a mouse-click, the mouseClickEvent parameter will be
        a pointer to the info about it, or may be null if the menu is being triggered
        by some other means.

        @see performPopupMenuAction, setPopupMenuEnabled, isPopupMenuEnabled
    */
    virtual void addPopupMenuItems (PopupMenu& menuToAddTo,
                                    const MouseEvent* mouseClickEvent);

    /** This is called to perform one of the items that was shown on the popup menu.

        If you've overridden addPopupMenuItems(), you should also override this
        to perform the actions that you've added.

        If you've overridden addPopupMenuItems() but have still left the default items
        on the menu, remember to call the superclass's performPopupMenuAction()
        so that it can perform the default actions if that's what the user clicked on.

        @see addPopupMenuItems, setPopupMenuEnabled, isPopupMenuEnabled
    */
    virtual void performPopupMenuAction (const int menuItemID);

    //==============================================================================
    /** Scrolls the minimum distance needed to get the caret into view. */
    void scrollToMakeSureCursorIsVisible();

    /** @internal */
    void moveCaret (int newCaretPos);

    /** @internal */
    void moveCursorTo (const int newPosition, const bool isSelecting);

    /** Used internally to dispatch a text-change message. */
    void textChanged();

    /** Begins a new transaction in the UndoManager.
    */
    void newTransaction();

    /** Used internally to trigger an undo or redo. */
    void doUndoRedo (const bool isRedo);

    /** Can be overridden to intercept return key presses directly */
    virtual void returnPressed();

    /** Can be overridden to intercept escape key presses directly */
    virtual void escapePressed();

    /** @internal */
    void handleCommandMessage (int commandId);

private:
    //==============================================================================
    ScopedPointer <Viewport> viewport;
    TextHolderComponent* textHolder;
    BorderSize borderSize;

    bool readOnly                   : 1;
    bool multiline                  : 1;
    bool wordWrap                   : 1;
    bool returnKeyStartsNewLine     : 1;
    bool caretVisible               : 1;
    bool popupMenuEnabled           : 1;
    bool selectAllTextWhenFocused   : 1;
    bool scrollbarVisible           : 1;
    bool wasFocused                 : 1;
    bool caretFlashState            : 1;
    bool keepCursorOnScreen         : 1;
    bool tabKeyUsed                 : 1;
    bool menuActive                 : 1;
    bool valueTextNeedsUpdating     : 1;

    UndoManager undoManager;
    float cursorX, cursorY, cursorHeight;
    int maxTextLength;
    int selectionStart, selectionEnd;
    int leftIndent, topIndent;
    unsigned int lastTransactionTime;
    Font currentFont;
    mutable int totalNumChars;
    int caretPosition;
    VoidArray sections;
    String textToShowWhenEmpty;
    Colour colourForTextWhenEmpty;
    tchar passwordCharacter;
    Value textValue;

    enum
    {
        notDragging,
        draggingSelectionStart,
        draggingSelectionEnd
    } dragType;

    String allowedCharacters;
    SortedSet <void*> listeners;

    friend class TextEditorInsertAction;
    friend class TextEditorRemoveAction;

    void coalesceSimilarSections();
    void splitSection (const int sectionIndex, const int charToSplitAt);

    void clearInternal (UndoManager* const um);

    void insert (const String& text,
                 const int insertIndex,
                 const Font& font,
                 const Colour& colour,
                 UndoManager* const um,
                 const int caretPositionToMoveTo);

    void reinsert (const int insertIndex,
                   const VoidArray& sections);

    void remove (const int startIndex,
                 int endIndex,
                 UndoManager* const um,
                 const int caretPositionToMoveTo);

    void getCharPosition (const int index,
                          float& x, float& y,
                          float& lineHeight) const;

    void updateCaretPosition();

    void textWasChangedByValue();

    int indexAtPosition (const float x,
                         const float y);

    int findWordBreakAfter (const int position) const;
    int findWordBreakBefore (const int position) const;

    friend class TextHolderComponent;
    friend class TextEditorViewport;
    void drawContent (Graphics& g);
    void updateTextHolderSize();
    float getWordWrapWidth() const;
    void timerCallbackInt();
    void repaintCaret();
    void repaintText (int textStartIndex, int textEndIndex);

    TextEditor (const TextEditor&);
    const TextEditor& operator= (const TextEditor&);
};

#endif   // __JUCE_TEXTEDITOR_JUCEHEADER__
