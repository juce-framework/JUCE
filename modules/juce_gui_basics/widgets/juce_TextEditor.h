/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An editable text box.

    A TextEditor can either be in single- or multi-line mode, and supports mixed
    fonts and colours.

    @see TextEditor::Listener, Label

    @tags{GUI}
*/
class JUCE_API  TextEditor  : public TextInputTarget,
                              public Component,
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
    explicit TextEditor (const String& componentName = String(),
                         juce_wchar passwordCharacter = 0);

    /** Destructor. */
    ~TextEditor() override;

    //==============================================================================
    /** Puts the editor into either multi- or single-line mode.

        By default, the editor will be in single-line mode, so use this if you need a multi-line
        editor.

        See also the setReturnKeyStartsNewLine() method, which will also need to be turned
        on if you want a multi-line editor with line-breaks.

        @param shouldBeMultiLine whether the editor should be multi- or single-line.
        @param shouldWordWrap    sets whether long lines should be broken up in multi-line editors.
                                 If this is false and scrollbars are enabled a horizontal scrollbar
                                 will be shown.

        @see isMultiLine, setReturnKeyStartsNewLine, setScrollbarsShown
    */
    void setMultiLine (bool shouldBeMultiLine,
                       bool shouldWordWrap = true);

    /** Returns true if the editor is in multi-line mode. */
    bool isMultiLine() const;

    //==============================================================================
    /** Changes the behaviour of the return key.

        If set to true, the return key will insert a new-line into the text; if false
        it will trigger a call to the TextEditor::Listener::textEditorReturnKeyPressed()
        method. By default this is set to false, and when true it will only insert
        new-lines when in multi-line mode (see setMultiLine()).
    */
    void setReturnKeyStartsNewLine (bool shouldStartNewLine);

    /** Returns the value set by setReturnKeyStartsNewLine().
        See setReturnKeyStartsNewLine() for more info.
    */
    bool getReturnKeyStartsNewLine() const                      { return returnKeyStartsNewLine; }

    /** Indicates whether the tab key should be accepted and used to input a tab character,
        or whether it gets ignored.

        By default the tab key is ignored, so that it can be used to switch keyboard focus
        between components.
    */
    void setTabKeyUsedAsCharacter (bool shouldTabKeyBeUsed);

    /** Returns true if the tab key is being used for input.
        @see setTabKeyUsedAsCharacter
    */
    bool isTabKeyUsedAsCharacter() const                        { return tabKeyUsed; }

    /** This can be used to change whether escape and return keypress events are
        propagated up to the parent component.
        The default here is true, meaning that these events are not allowed to reach the
        parent, but you may want to allow them through so that they can trigger other
        actions, e.g. closing a dialog box, etc.
    */
    void setEscapeAndReturnKeysConsumed (bool shouldBeConsumed) noexcept;

    //==============================================================================
    /** Changes the editor to read-only mode.

        By default, the text editor is not read-only. If you're making it read-only, you
        might also want to call setCaretVisible (false) to get rid of the caret.

        The text can still be highlighted and copied when in read-only mode.

        @see isReadOnly, setCaretVisible
    */
    void setReadOnly (bool shouldBeReadOnly);

    /** Returns true if the editor is in read-only mode. */
    bool isReadOnly() const noexcept;

    //==============================================================================
    /** Makes the caret visible or invisible.
        By default the caret is visible.
        @see setCaretColour, setCaretPosition
    */
    void setCaretVisible (bool shouldBeVisible);

    /** Returns true if the caret is enabled.
        @see setCaretVisible
    */
    bool isCaretVisible() const noexcept                            { return caretVisible && ! isReadOnly(); }

    //==============================================================================
    /** Enables or disables scrollbars (this only applies when in multi-line mode).

        When the text gets too long to fit in the component, a scrollbar can appear to
        allow it to be scrolled. Even when this is enabled, the scrollbar will be hidden
        unless it's needed.

        By default scrollbars are enabled.
    */
    void setScrollbarsShown (bool shouldBeEnabled);

    /** Returns true if scrollbars are enabled.
        @see setScrollbarsShown
    */
    bool areScrollbarsShown() const noexcept                        { return scrollbarVisible; }

    /** Changes the password character used to disguise the text.

        @param passwordCharacter    if this is not zero, this character will be used as a replacement
                                    for all characters that are drawn on screen - e.g. to create
                                    a password-style textbox containing circular blobs instead of text,
                                    you could set this value to 0x25cf, which is the unicode character
                                    for a black splodge (not all fonts include this, though), or 0x2022,
                                    which is a bullet (probably the best choice for linux).
    */
    void setPasswordCharacter (juce_wchar passwordCharacter);

    /** Returns the current password character.
        @see setPasswordCharacter
    */
    juce_wchar getPasswordCharacter() const noexcept                { return passwordCharacter; }

    //==============================================================================
    /** Allows a right-click menu to appear for the editor.

        (This defaults to being enabled).

        If enabled, right-clicking (or command-clicking on the Mac) will pop up a menu
        of options such as cut/copy/paste, undo/redo, etc.
    */
    void setPopupMenuEnabled (bool menuEnabled);

    /** Returns true if the right-click menu is enabled.
        @see setPopupMenuEnabled
    */
    bool isPopupMenuEnabled() const noexcept                        { return popupMenuEnabled; }

    /** Returns true if a popup-menu is currently being displayed. */
    bool isPopupMenuCurrentlyActive() const noexcept                { return menuActive; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the editor.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        NB: You can also set the caret colour using CaretComponent::caretColourId

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId       = 0x1000200, /**< The colour to use for the text component's background - this can be
                                                   transparent if necessary. */

        textColourId             = 0x1000201, /**< The colour that will be used when text is added to the editor. Note
                                                   that because the editor can contain multiple colours, calling this
                                                   method won't change the colour of existing text - to do that, use
                                                   the applyColourToAllText() method */

        highlightColourId        = 0x1000202, /**< The colour with which to fill the background of highlighted sections of
                                                   the text - this can be transparent if you don't want to show any
                                                   highlighting.*/

        highlightedTextColourId  = 0x1000203, /**< The colour with which to draw the text in highlighted sections. */

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

        This function also calls
        applyColourToAllText (findColour (TextEditor::ColourIds::textColourId), false);

        If the changeCurrentFont argument is true then this will also set the
        new font as the font to be used for any new text that's added.

        @see setFont
    */
    void applyFontToAllText (const Font& newFont, bool changeCurrentFont = true);

    /** Returns the font that's currently being used for new text.

        @see setFont
    */
    const Font& getFont() const noexcept  { return currentFont; }

    /** Applies a colour to all the text in the editor.

        If the changeCurrentTextColour argument is true then this will also set the
        new colour as the colour to be used for any new text that's added.
    */
    void applyColourToAllText (const Colour& newColour, bool changeCurrentTextColour = true);

    /** Sets whether whitespace should be underlined when the editor font is underlined.

        @see isWhitespaceUnderlined
    */
    void setWhitespaceUnderlined (bool shouldUnderlineWhitespace) noexcept  { underlineWhitespace = shouldUnderlineWhitespace; }

    /** Returns true if whitespace is underlined for underlined fonts.

        @see setWhitespaceIsUnderlined
    */
    bool isWhitespaceUnderlined() const noexcept                            { return underlineWhitespace; }

    //==============================================================================
    /** If set to true, focusing on the editor will highlight all its text.

        (Set to false by default).

        This is useful for boxes where you expect the user to re-enter all the
        text when they focus on the component, rather than editing what's already there.
    */
    void setSelectAllWhenFocused (bool shouldSelectAll);

    /** When the text editor is empty, it can be set to display a message.

        This is handy for things like telling the user what to type in the box - the
        string is only displayed, it's not taken to actually be the contents of
        the editor.
    */
    void setTextToShowWhenEmpty (const String& text, Colour colourToUse);

    /** Returns the text that will be shown when the text editor is empty.

        @see setTextToShowWhenEmpty
    */
    String getTextToShowWhenEmpty() const noexcept    { return textToShowWhenEmpty; }

    //==============================================================================
    /** Changes the size of the scrollbars that are used.
        Handy if you need smaller scrollbars for a small text box.
    */
    void setScrollBarThickness (int newThicknessPixels);

    //==============================================================================
    /**
        Receives callbacks from a TextEditor component when it changes.

        @see TextEditor::addListener
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Called when the user changes the text in some way. */
        virtual void textEditorTextChanged (TextEditor&) {}

        /** Called when the user presses the return key. */
        virtual void textEditorReturnKeyPressed (TextEditor&) {}

        /** Called when the user presses the escape key. */
        virtual void textEditorEscapeKeyPressed (TextEditor&) {}

        /** Called when the text editor loses focus. */
        virtual void textEditorFocusLost (TextEditor&) {}
    };

    /** Registers a listener to be told when things happen to the text.
        @see removeListener
    */
    void addListener (Listener* newListener);

    /** Deregisters a listener.
        @see addListener
    */
    void removeListener (Listener* listenerToRemove);

    //==============================================================================
    /** You can assign a lambda to this callback object to have it called when the text is changed. */
    std::function<void()> onTextChange;

    /** You can assign a lambda to this callback object to have it called when the return key is pressed. */
    std::function<void()> onReturnKey;

    /** You can assign a lambda to this callback object to have it called when the escape key is pressed. */
    std::function<void()> onEscapeKey;

    /** You can assign a lambda to this callback object to have it called when the editor loses key focus. */
    std::function<void()> onFocusLost;

    //==============================================================================
    /** Returns the entire contents of the editor. */
    String getText() const;

    /** Returns a section of the contents of the editor. */
    String getTextInRange (const Range<int>& textRange) const override;

    /** Returns true if there are no characters in the editor.
        This is far more efficient than calling getText().isEmpty().
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
        @see insertTextAtCaret
    */
    void setText (const String& newText,
                  bool sendTextChangeMessage = true);

    /** Returns a Value object that can be used to get or set the text.

        Bear in mind that this operate quite slowly if your text box contains large
        amounts of text, as it needs to dynamically build the string that's involved.
        It's best used for small text boxes.
    */
    Value& getTextValue();

    /** Inserts some text at the current caret position.

        If a section of the text is highlighted, it will be replaced by
        this string, otherwise it will be inserted.

        To delete a section of text, you can use setHighlightedRegion() to
        highlight it, and call insertTextAtCaret (String()).

        @see setCaretPosition, getCaretPosition, setHighlightedRegion
    */
    void insertTextAtCaret (const String& textToInsert) override;

    /** Deletes all the text from the editor. */
    void clear();

    /** Deletes the currently selected region.
        This doesn't copy the deleted section to the clipboard - if you need to do that, call copy() first.
        @see copy, paste, SystemClipboard
    */
    void cut();

    /** Copies the currently selected region to the clipboard.
        @see cut, paste, SystemClipboard
    */
    void copy();

    /** Pastes the contents of the clipboard into the editor at the caret position.
        @see cut, copy, SystemClipboard
    */
    void paste();

    //==============================================================================
    /** Returns the current index of the caret.
        @see setCaretPosition
    */
    int getCaretPosition() const override;

    /** Moves the caret to be in front of a given character.
        @see getCaretPosition, moveCaretToEnd
    */
    void setCaretPosition (int newIndex);

    /** Attempts to scroll the text editor so that the caret ends up at
        a specified position.

        This won't affect the caret's position within the text, it tries to scroll
        the entire editor vertically and horizontally so that the caret is sitting
        at the given position (relative to the top-left of this component).

        Depending on the amount of text available, it might not be possible to
        scroll far enough for the caret to reach this exact position, but it
        will go as far as it can in that direction.
    */
    void scrollEditorToPositionCaret (int desiredCaretX, int desiredCaretY);

    /** Get the graphical position of the caret for a particular index in the text.

        The rectangle returned is relative to the component's top-left corner.
    */
    Rectangle<int> getCaretRectangleForCharIndex (int index) const override;

    /** Selects a section of the text. */
    void setHighlightedRegion (const Range<int>& newSelection) override;

    /** Returns the range of characters that are selected.
        If nothing is selected, this will return an empty range.
        @see setHighlightedRegion
    */
    Range<int> getHighlightedRegion() const override            { return selection; }

    /** Returns the section of text that is currently selected. */
    String getHighlightedText() const;

    /** Finds the index of the character at a given position.
        The coordinates are relative to the component's top-left.
    */
    int getTextIndexAt (int x, int y) const;

    /** Finds the index of the character at a given position.
        The coordinates are relative to the component's top-left.
    */
    int getTextIndexAt (Point<int>) const;

    /** Like getTextIndexAt, but doesn't snap to the beginning/end of the range for
        points vertically outside the text.
    */
    int getCharIndexForPoint (Point<int> point) const override;

    /** Counts the number of characters in the text.

        This is quicker than getting the text as a string if you just need to know
        the length.
    */
    int getTotalNumChars() const override;

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
    void setIndents (int newLeftIndent, int newTopIndent);

    /** Returns the gap at the top edge of the editor.
        @see setIndents
    */
    int getTopIndent() const noexcept   { return topIndent; }

    /** Returns the gap at the left edge of the editor.
        @see setIndents
    */
    int getLeftIndent() const noexcept  { return leftIndent; }

    /** Changes the size of border left around the edge of the component.
        @see getBorder
    */
    void setBorder (BorderSize<int> border);

    /** Returns the size of border around the edge of the component.
        @see setBorder
    */
    BorderSize<int> getBorder() const;

    /** Used to disable the auto-scrolling which keeps the caret visible.

        If true (the default), the editor will scroll when the caret moves offscreen. If
        set to false, it won't.
    */
    void setScrollToShowCursor (bool shouldScrollToShowCaret);

    /** Modifies the justification of the text within the editor window. */
    void setJustification (Justification newJustification);

    /** Returns the type of justification, as set in setJustification(). */
    Justification getJustificationType() const noexcept             { return justification; }

    /** Sets the line spacing of the TextEditor.
        The default (and minimum) value is 1.0 and values > 1.0 will increase the line spacing as a
        multiple of the line height e.g. for double-spacing call this method with an argument of 2.0.
    */
    void setLineSpacing (float newLineSpacing) noexcept;

    /** Returns the current line spacing of the TextEditor. */
    float getLineSpacing() const noexcept                           { return lineSpacing; }

    /** Returns the bounding box for a range of text in the editor. As the range may span
        multiple lines, this method returns a RectangleList.

        The bounds are relative to the component's top-left and may extend beyond the bounds
        of the component if the text is long and word wrapping is disabled.
    */
    RectangleList<int> getTextBounds (Range<int> textRange) const override;

    //==============================================================================
    void moveCaretToEnd();
    bool moveCaretLeft (bool moveInWholeWordSteps, bool selecting);
    bool moveCaretRight (bool moveInWholeWordSteps, bool selecting);
    bool moveCaretUp (bool selecting);
    bool moveCaretDown (bool selecting);
    bool pageUp (bool selecting);
    bool pageDown (bool selecting);
    bool scrollDown();
    bool scrollUp();
    bool moveCaretToTop (bool selecting);
    bool moveCaretToStartOfLine (bool selecting);
    bool moveCaretToEnd (bool selecting);
    bool moveCaretToEndOfLine (bool selecting);
    bool deleteBackwards (bool moveInWholeWordSteps);
    bool deleteForwards (bool moveInWholeWordSteps);
    bool copyToClipboard();
    bool cutToClipboard();
    bool pasteFromClipboard();
    bool selectAll();
    bool undo();
    bool redo();

    //==============================================================================
    /** This adds the items to the popup menu.

        By default it adds the cut/copy/paste items, but you can override this if
        you need to replace these with your own items.

        If you want to add your own items to the existing ones, you can override this,
        call the base class's addPopupMenuItems() method, then append your own items.

        When the menu has been shown, performPopupMenuAction() will be called to
        perform the item that the user has chosen.

        The default menu items will be added using item IDs from the
        StandardApplicationCommandIDs namespace.

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
    virtual void performPopupMenuAction (int menuItemID);

    //==============================================================================
    /** Base class for input filters that can be applied to a TextEditor to restrict
        the text that can be entered.
    */
    class JUCE_API  InputFilter
    {
    public:
        InputFilter() = default;
        virtual ~InputFilter() = default;

        /** This method is called whenever text is entered into the editor.
            An implementation of this class should check the input string,
            and return an edited version of it that should be used.
        */
        virtual String filterNewText (TextEditor&, const String& newInput) = 0;
    };

    /** An input filter for a TextEditor that limits the length of text and/or the
        characters that it may contain.
    */
    class JUCE_API  LengthAndCharacterRestriction  : public InputFilter
    {
    public:
        /** Creates a filter that limits the length of text, and/or the characters that it can contain.
            @param maxNumChars          if this is > 0, it sets a maximum length limit; if <= 0, no
                                        limit is set
            @param allowedCharacters    if this is non-empty, then only characters that occur in
                                        this string are allowed to be entered into the editor.
        */
        LengthAndCharacterRestriction (int maxNumChars, const String& allowedCharacters);

        String filterNewText (TextEditor&, const String&) override;

    private:
        String allowedCharacters;
        int maxLength;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LengthAndCharacterRestriction)
    };

    /** Sets an input filter that should be applied to this editor.
        The filter can be nullptr, to remove any existing filters.
        If takeOwnership is true, then the filter will be owned and deleted by the editor
        when no longer needed.
    */
    void setInputFilter (InputFilter* newFilter, bool takeOwnership);

    /** Returns the current InputFilter, as set by setInputFilter(). */
    InputFilter* getInputFilter() const noexcept                { return inputFilter; }

    /** Sets limits on the characters that can be entered.
        This is just a shortcut that passes an instance of the LengthAndCharacterRestriction
        class to setInputFilter().

        @param maxTextLength        if this is > 0, it sets a maximum length limit; if 0, no
                                    limit is set
        @param allowedCharacters    if this is non-empty, then only characters that occur in
                                    this string are allowed to be entered into the editor.
    */
    void setInputRestrictions (int maxTextLength,
                               const String& allowedCharacters = String());

    /** Sets the type of virtual keyboard that should be displayed when this editor has
        focus.
    */
    void setKeyboardType (VirtualKeyboardType type) noexcept    { keyboardType = type; }

    /** Sets the behaviour of mouse/touch interactions outside this component.

        If true, then presses outside of the TextEditor will dismiss the virtual keyboard.
        If false, then the virtual keyboard will remain onscreen for as long as the TextEditor has
        keyboard focus.
    */
    void setClicksOutsideDismissVirtualKeyboard (bool);

    /** Returns true if the editor is configured to hide the virtual keyboard when the mouse is
        pressed on another component.
    */
    bool getClicksOutsideDismissVirtualKeyboard() const     { return clicksOutsideDismissVirtualKeyboard; }

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        TextEditor drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void fillTextEditorBackground (Graphics&, int width, int height, TextEditor&) = 0;
        virtual void drawTextEditorOutline (Graphics&, int width, int height, TextEditor&) = 0;

        virtual CaretComponent* createCaretComponent (Component* keyFocusOwner) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void paintOverChildren (Graphics&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseDoubleClick (const MouseEvent&) override;
    /** @internal */
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    bool keyPressed (const KeyPress&) override;
    /** @internal */
    bool keyStateChanged (bool) override;
    /** @internal */
    void focusGained (FocusChangeType) override;
    /** @internal */
    void focusLost (FocusChangeType) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void enablementChanged() override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    bool isTextInputActive() const override;
    /** @internal */
    void setTemporaryUnderlining (const Array<Range<int>>&) override;
    /** @internal */
    VirtualKeyboardType getKeyboardType() override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** Scrolls the minimum distance needed to get the caret into view. */
    void scrollToMakeSureCursorIsVisible();

    /** Used internally to dispatch a text-change message. */
    void textChanged();

    /** Begins a new transaction in the UndoManager. */
    void newTransaction();

    /** Can be overridden to intercept return key presses directly */
    virtual void returnPressed();

    /** Can be overridden to intercept escape key presses directly */
    virtual void escapePressed();

private:
    //==============================================================================
    struct TextHolderComponent;
    struct TextEditorViewport;
    struct InsertAction;
    struct RemoveAction;
    class EditorAccessibilityHandler;

    class GlobalMouseListener : private MouseListener
    {
    public:
        explicit GlobalMouseListener (Component& e) : editor (e) { Desktop::getInstance().addGlobalMouseListener    (this); }
        ~GlobalMouseListener() override                          { Desktop::getInstance().removeGlobalMouseListener (this); }

        bool lastMouseDownInEditor() const { return mouseDownInEditor; }

    private:
        void mouseDown (const MouseEvent& event) override { mouseDownInEditor = event.originalComponent == &editor; }

        Component& editor;
        bool mouseDownInEditor = false;
    };

    std::unique_ptr<Viewport> viewport;
    TextHolderComponent* textHolder;
    BorderSize<int> borderSize { 1, 1, 1, 3 };
    Justification justification { Justification::topLeft };
    const GlobalMouseListener globalMouseListener { *this };

    bool readOnly = false;
    bool caretVisible = true;
    bool multiline = false;
    bool wordWrap = false;
    bool returnKeyStartsNewLine = false;
    bool popupMenuEnabled = true;
    bool selectAllTextWhenFocused = false;
    bool scrollbarVisible = true;
    bool wasFocused = false;
    bool keepCaretOnScreen = true;
    bool tabKeyUsed = false;
    bool menuActive = false;
    bool valueTextNeedsUpdating = false;
    bool consumeEscAndReturnKeys = true;
    bool underlineWhitespace = true;
    bool clicksOutsideDismissVirtualKeyboard = false;

    UndoManager undoManager;
    std::unique_ptr<CaretComponent> caret;
    Range<int> selection;
    int leftIndent = 4, topIndent = 4;
    unsigned int lastTransactionTime = 0;
    Font currentFont { withDefaultMetrics (FontOptions { 14.0f }) };
    mutable int totalNumChars = 0;

    //==============================================================================
    enum class Edge
    {
        leading,
        trailing
    };

    //==============================================================================
    struct CaretState
    {
    public:
        explicit CaretState (const TextEditor* ownerIn);

        int getPosition() const { return position; }
        Edge getEdge() const { return edge; }

        void setPosition (int newPosition);

        /*  Not all visual edge positions are permitted e.g. a trailing caret after a newline
            is not allowed. getVisualIndex() and getEdge() will return the closest permitted
            values to the preferred one.
        */
        void setPreferredEdge (Edge newEdge);

        /*  The returned value is in the range [0, TextEditor::getTotalNumChars()]. It returns the
            glyph index to which the caret is closest visually. This is significant when
            differentiating between the end of one line and the beginning of the next.
        */
        int getVisualIndex() const;

        void updateEdge();

        //==============================================================================
        CaretState withPosition (int newPosition) const;
        CaretState withPreferredEdge (Edge newEdge) const;

    private:
        const TextEditor& owner;
        int position = 0;
        Edge edge = Edge::trailing;
        Edge preferredEdge = Edge::trailing;
    };

    //==============================================================================
    String textToShowWhenEmpty;
    Colour colourForTextWhenEmpty;
    juce_wchar passwordCharacter;
    OptionalScopedPointer<InputFilter> inputFilter;
    Value textValue;
    VirtualKeyboardType keyboardType = TextInputTarget::textKeyboard;
    float lineSpacing = 1.0f;

    enum DragType
    {
        notDragging,
        draggingSelectionStart,
        draggingSelectionEnd
    };

    DragType dragType = notDragging;

    ListenerList<Listener> listeners;
    Array<Range<int>> underlinedSections;

    class ParagraphStorage;
    class ParagraphsModel;
    struct TextEditorStorageChunks;
    class TextEditorStorage;

    void moveCaret (int newCaretPos);
    void moveCaretTo (int newPosition, bool isSelecting);
    void recreateCaret();
    void handleCommandMessage (int) override;
    void clearInternal (UndoManager*);
    void insert (const String&, int insertIndex, const Font&, Colour, UndoManager*, int newCaretPos);
    void reinsert (const TextEditorStorageChunks& chunks);
    void remove (Range<int>, UndoManager*, int caretPositionToMoveTo, TextEditorStorageChunks* removedOut = nullptr);

    struct CaretEdge
    {
        Point<float> anchor;
        float height{};
    };

    float getJustificationOffsetX() const;
    CaretEdge getDefaultCursorEdge() const;
    CaretEdge getTextSelectionEdge (int index, Edge edge) const;
    CaretEdge getCursorEdge (const CaretState& caret) const;
    void updateCaretPosition();
    void updateValueFromText();
    void textWasChangedByValue();
    int indexAtPosition (float x, float y) const;
    int findWordBreakAfter (int position) const;
    int findWordBreakBefore (int position) const;
    bool moveCaretWithTransaction (int newPos, bool selecting);
    void drawContent (Graphics&);
    void checkLayout();
    int getWordWrapWidth() const;
    int getMaximumTextWidth() const;
    int getMaximumTextHeight() const;
    void timerCallbackInt();
    void checkFocus();
    void repaintText (Range<int>);
    void scrollByLines (int deltaLines);
    bool undoOrRedo (bool shouldUndo);
    UndoManager* getUndoManager() noexcept;
    void setSelection (Range<int>) noexcept;
    Point<int> getTextOffset() const;

    Edge getEdgeTypeCloserToPosition (int indexInText, Point<float> pos) const;

    std::unique_ptr<TextEditorStorage> textStorage;
    CaretState caretState;

    bool isTextStorageHeightGreaterEqualThan (float value) const;
    float getTextStorageHeight() const;
    float getYOffset() const;
    void updateBaseShapedTextOptions();
    Range<int64> getLineRangeForIndex (int index);

    template <typename T>
    detail::RangedValues<T> getGlyphRanges (const detail::RangedValues<T>& textRanges) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextEditor)
};


} // namespace juce
