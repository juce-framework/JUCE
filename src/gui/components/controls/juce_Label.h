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

#ifndef __JUCE_LABEL_JUCEHEADER__
#define __JUCE_LABEL_JUCEHEADER__

#include "../juce_ComponentDeletionWatcher.h"
#include "juce_TextEditor.h"
class Label;


//==============================================================================
/**
    A class for receiving events from a Label.

    You can register a LabelListener with a Label using the Label::addListener()
    method, and it will be called when the text of the label changes, either because
    of a call to Label::setText() or by the user editing the text (if the label is
    editable).

    @see Label::addListener, Label::removeListener
*/
class JUCE_API  LabelListener
{
public:
    /** Destructor. */
    virtual ~LabelListener() {}

    /** Called when a Label's text has changed.
    */
    virtual void labelTextChanged (Label* labelThatHasChanged) = 0;
};


//==============================================================================
/**
    A component that displays a text string, and can optionally become a text
    editor when clicked.
*/
class JUCE_API  Label  : public Component,
                         public SettableTooltipClient,
                         protected TextEditorListener,
                         private ComponentListener,
                         private Value::Listener
{
public:
    //==============================================================================
    /** Creates a Label.

        @param componentName    the name to give the component
        @param labelText        the text to show in the label
    */
    Label (const String& componentName,
           const String& labelText);

    /** Destructor. */
    ~Label();

    //==============================================================================
    /** Changes the label text.

        If broadcastChangeMessage is true and the new text is different to the current
        text, then the class will broadcast a change message to any LabelListeners that
        are registered.
    */
    void setText (const String& newText,
                  const bool broadcastChangeMessage);

    /** Returns the label's current text.

        @param returnActiveEditorContents   if this is true and the label is currently
                                            being edited, then this method will return the
                                            text as it's being shown in the editor. If false,
                                            then the value returned here won't be updated until
                                            the user has finished typing and pressed the return
                                            key.
    */
    const String getText (const bool returnActiveEditorContents = false) const throw();

    /** Returns the text content as a Value object.
        You can call Value::referTo() on this object to make the label read and control
        a Value object that you supply.
    */
    Value& getTextValue()                               { return textValue; }

    //==============================================================================
    /** Changes the font to use to draw the text.

        @see getFont
    */
    void setFont (const Font& newFont) throw();

    /** Returns the font currently being used.

        @see setFont
    */
    const Font& getFont() const throw();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        Note that you can also use the constants from TextEditor::ColourIds to change the
        colour of the text editor that is opened when a label is editable.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId     = 0x1000280, /**< The background colour to fill the label with. */
        textColourId           = 0x1000281, /**< The colour for the text. */
        outlineColourId        = 0x1000282  /**< An optional colour to use to draw a border around the label.
                                                 Leave this transparent to not have an outline. */
    };

    //==============================================================================
    /** Sets the style of justification to be used for positioning the text.

        (The default is Justification::centredLeft)
    */
    void setJustificationType (const Justification& justification) throw();

    /** Returns the type of justification, as set in setJustificationType(). */
    const Justification getJustificationType() const throw()                    { return justification; }

    /** Changes the gap that is left between the edge of the component and the text.
        By default there's a small gap left at the sides of the component to allow for
        the drawing of the border, but you can change this if necessary.
    */
    void setBorderSize (int horizontalBorder, int verticalBorder);

    /** Returns the size of the horizontal gap being left around the text.
    */
    int getHorizontalBorderSize() const throw()                                 { return horizontalBorderSize; }

    /** Returns the size of the vertical gap being left around the text.
    */
    int getVerticalBorderSize() const throw()                                   { return verticalBorderSize; }

    /** Makes this label "stick to" another component.

        This will cause the label to follow another component around, staying
        either to its left or above it.

        @param owner    the component to follow
        @param onLeft   if true, the label will stay on the left of its component; if
                        false, it will stay above it.
    */
    void attachToComponent (Component* owner,
                            const bool onLeft);

    /** If this label has been attached to another component using attachToComponent, this
        returns the other component.

        Returns 0 if the label is not attached.
    */
    Component* getAttachedComponent() const throw()                             { return ownerComponent; }

    /** If the label is attached to the left of another component, this returns true.

        Returns false if the label is above the other component. This is only relevent if
        attachToComponent() has been called.
    */
    bool isAttachedOnLeft() const throw()                                       { return leftOfOwnerComp; }

    /** Specifies the minimum amount that the font can be squashed horizantally before it starts
        using ellipsis.

        @see Graphics::drawFittedText
    */
    void setMinimumHorizontalScale (const float newScale);

    float getMinimumHorizontalScale() const throw()                             { return minimumHorizontalScale; }

    //==============================================================================
    /** Registers a listener that will be called when the label's text changes. */
    void addListener (LabelListener* const listener) throw();

    /** Deregisters a previously-registered listener. */
    void removeListener (LabelListener* const listener) throw();

    //==============================================================================
    /** Makes the label turn into a TextEditor when clicked.

        By default this is turned off.

        If turned on, then single- or double-clicking will turn the label into
        an editor. If the user then changes the text, then the ChangeBroadcaster
        base class will be used to send change messages to any listeners that
        have registered.

        If the user changes the text, the textWasEdited() method will be called
        afterwards, and subclasses can override this if they need to do anything
        special.

        @param editOnSingleClick            if true, just clicking once on the label will start editing the text
        @param editOnDoubleClick            if true, a double-click is needed to start editing
        @param lossOfFocusDiscardsChanges   if true, clicking somewhere else while the text is being
                                            edited will discard any changes; if false, then this will
                                            commit the changes.
        @see showEditor, setEditorColours, TextEditor
    */
    void setEditable (const bool editOnSingleClick,
                      const bool editOnDoubleClick = false,
                      const bool lossOfFocusDiscardsChanges = false) throw();

    /** Returns true if this option was set using setEditable(). */
    bool isEditableOnSingleClick() const throw()                        { return editSingleClick; }

    /** Returns true if this option was set using setEditable(). */
    bool isEditableOnDoubleClick() const throw()                        { return editDoubleClick; }

    /** Returns true if this option has been set in a call to setEditable(). */
    bool doesLossOfFocusDiscardChanges() const throw()                  { return lossOfFocusDiscardsChanges; }

    /** Returns true if the user can edit this label's text. */
    bool isEditable() const throw()                                     { return editSingleClick || editDoubleClick; }

    /** Makes the editor appear as if the label had been clicked by the user.

        @see textWasEdited, setEditable
    */
    void showEditor();

    /** Hides the editor if it was being shown.

        @param discardCurrentEditorContents     if true, the label's text will be
                                                reset to whatever it was before the editor
                                                was shown; if false, the current contents of the
                                                editor will be used to set the label's text
                                                before it is hidden.
    */
    void hideEditor (const bool discardCurrentEditorContents);

    /** Returns true if the editor is currently focused and active. */
    bool isBeingEdited() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** Creates the TextEditor component that will be used when the user has clicked on the label.

        Subclasses can override this if they need to customise this component in some way.
    */
    virtual TextEditor* createEditorComponent();

    /** Called after the user changes the text.
    */
    virtual void textWasEdited();

    /** Called when the text has been altered.
    */
    virtual void textWasChanged();

    /** Called when the text editor has just appeared, due to a user click or other
        focus change.
    */
    virtual void editorShown (TextEditor* editorComponent);

    /** Called when the text editor is going to be deleted, after editing has finished.
    */
    virtual void editorAboutToBeHidden (TextEditor* editorComponent);

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();
    /** @internal */
    void mouseUp (const MouseEvent& e);
    /** @internal */
    void mouseDoubleClick (const MouseEvent& e);
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    void componentParentHierarchyChanged (Component& component);
    /** @internal */
    void componentVisibilityChanged (Component& component);
    /** @internal */
    void inputAttemptWhenModal();
    /** @internal */
    void focusGained (FocusChangeType);
    /** @internal */
    void enablementChanged();
    /** @internal */
    KeyboardFocusTraverser* createFocusTraverser();
    /** @internal */
    void textEditorTextChanged (TextEditor& editor);
    /** @internal */
    void textEditorReturnKeyPressed (TextEditor& editor);
    /** @internal */
    void textEditorEscapeKeyPressed (TextEditor& editor);
    /** @internal */
    void textEditorFocusLost (TextEditor& editor);
    /** @internal */
    void colourChanged();
    /** @internal */
    void valueChanged (Value&);

private:
    Value textValue;
    String lastTextValue;
    Font font;
    Justification justification;
    ScopedPointer <TextEditor> editor;
    SortedSet <void*> listeners;
    Component* ownerComponent;
    ScopedPointer <ComponentDeletionWatcher> deletionWatcher;
    int horizontalBorderSize, verticalBorderSize;
    float minimumHorizontalScale;
    bool editSingleClick : 1;
    bool editDoubleClick : 1;
    bool lossOfFocusDiscardsChanges : 1;
    bool leftOfOwnerComp : 1;

    bool updateFromTextEditorContents();
    void callChangeListeners();

    Label (const Label&);
    const Label& operator= (const Label&);
};


#endif   // __JUCE_LABEL_JUCEHEADER__
