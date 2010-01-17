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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Label.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
Label::Label (const String& componentName,
              const String& labelText)
    : Component (componentName),
      textValue (labelText),
      lastTextValue (labelText),
      font (15.0f),
      justification (Justification::centredLeft),
      ownerComponent (0),
      horizontalBorderSize (5),
      verticalBorderSize (1),
      minimumHorizontalScale (0.7f),
      editSingleClick (false),
      editDoubleClick (false),
      lossOfFocusDiscardsChanges (false)
{
    setColour (TextEditor::textColourId, Colours::black);
    setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    setColour (TextEditor::outlineColourId, Colours::transparentBlack);

    textValue.addListener (this);
}

Label::~Label()
{
    textValue.removeListener (this);

    if (ownerComponent != 0 && ! deletionWatcher->hasBeenDeleted())
        ownerComponent->removeComponentListener (this);

    editor = 0;
}

//==============================================================================
void Label::setText (const String& newText,
                     const bool broadcastChangeMessage)
{
    hideEditor (true);

    if (lastTextValue != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != 0 && ! deletionWatcher->hasBeenDeleted())
            componentMovedOrResized (*ownerComponent, true, true);

        if (broadcastChangeMessage)
            callChangeListeners();
    }
}

const String Label::getText (const bool returnActiveEditorContents) const throw()
{
    return (returnActiveEditorContents && isBeingEdited())
                ? editor->getText()
                : textValue.toString();
}

void Label::valueChanged (Value&)
{
    if (lastTextValue != textValue.toString())
        setText (textValue.toString(), true);
}

//==============================================================================
void Label::setFont (const Font& newFont) throw()
{
    font = newFont;
    repaint();
}

const Font& Label::getFont() const throw()
{
    return font;
}

void Label::setEditable (const bool editOnSingleClick,
                         const bool editOnDoubleClick,
                         const bool lossOfFocusDiscardsChanges_) throw()
{
    editSingleClick = editOnSingleClick;
    editDoubleClick = editOnDoubleClick;
    lossOfFocusDiscardsChanges = lossOfFocusDiscardsChanges_;

    setWantsKeyboardFocus (editOnSingleClick || editOnDoubleClick);
    setFocusContainer (editOnSingleClick || editOnDoubleClick);
}

void Label::setJustificationType (const Justification& justification_) throw()
{
    justification = justification_;
    repaint();
}

void Label::setBorderSize (int h, int v)
{
    horizontalBorderSize = h;
    verticalBorderSize = v;
    repaint();
}

//==============================================================================
void Label::attachToComponent (Component* owner,
                               const bool onLeft)
{
    if (ownerComponent != 0 && ! deletionWatcher->hasBeenDeleted())
        ownerComponent->removeComponentListener (this);

    deletionWatcher = 0;
    ownerComponent = owner;

    leftOfOwnerComp = onLeft;

    if (ownerComponent != 0)
    {
        deletionWatcher = new ComponentDeletionWatcher (owner);

        setVisible (owner->isVisible());
        ownerComponent->addComponentListener (this);
        componentParentHierarchyChanged (*ownerComponent);
        componentMovedOrResized (*ownerComponent, true, true);
    }
}

void Label::componentMovedOrResized (Component& component,
                                     bool /*wasMoved*/,
                                     bool /*wasResized*/)
{
    if (leftOfOwnerComp)
    {
        setSize (jmin (getFont().getStringWidth (textValue.toString()) + 8, component.getX()),
                 component.getHeight());

        setTopRightPosition (component.getX(), component.getY());
    }
    else
    {
        setSize (component.getWidth(),
                 8 + roundToInt (getFont().getHeight()));

        setTopLeftPosition (component.getX(), component.getY() - getHeight());
    }
}

void Label::componentParentHierarchyChanged (Component& component)
{
    if (component.getParentComponent() != 0)
        component.getParentComponent()->addChildComponent (this);
}

void Label::componentVisibilityChanged (Component& component)
{
    setVisible (component.isVisible());
}

//==============================================================================
void Label::textWasEdited()
{
}

void Label::textWasChanged()
{
}

void Label::showEditor()
{
    if (editor == 0)
    {
        addAndMakeVisible (editor = createEditorComponent());
        editor->setText (getText(), false);
        editor->addListener (this);
        editor->grabKeyboardFocus();
        editor->setHighlightedRegion (0, textValue.toString().length());
        editor->addListener (this);

        resized();
        repaint();

        editorShown (editor);

        enterModalState();
        editor->grabKeyboardFocus();
    }
}

void Label::editorShown (TextEditor* /*editorComponent*/)
{
}

void Label::editorAboutToBeHidden (TextEditor* /*editorComponent*/)
{
}

bool Label::updateFromTextEditorContents()
{
    jassert (editor != 0);
    const String newText (editor->getText());

    if (textValue.toString() != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != 0 && ! deletionWatcher->hasBeenDeleted())
            componentMovedOrResized (*ownerComponent, true, true);

        return true;
    }

    return false;
}

void Label::hideEditor (const bool discardCurrentEditorContents)
{
    if (editor != 0)
    {
        editorAboutToBeHidden (editor);

        const bool changed = (! discardCurrentEditorContents)
                               && updateFromTextEditorContents();

        editor = 0;
        repaint();

        if (changed)
            textWasEdited();

        exitModalState (0);

        if (changed && isValidComponent())
            callChangeListeners();
    }
}

void Label::inputAttemptWhenModal()
{
    if (editor != 0)
    {
        if (lossOfFocusDiscardsChanges)
            textEditorEscapeKeyPressed (*editor);
        else
            textEditorReturnKeyPressed (*editor);
    }
}

bool Label::isBeingEdited() const throw()
{
    return editor != 0;
}

TextEditor* Label::createEditorComponent()
{
    TextEditor* const ed = new TextEditor (getName());
    ed->setFont (font);

    // copy these colours from our own settings..
    const int cols[] = { TextEditor::backgroundColourId,
                         TextEditor::textColourId,
                         TextEditor::highlightColourId,
                         TextEditor::highlightedTextColourId,
                         TextEditor::caretColourId,
                         TextEditor::outlineColourId,
                         TextEditor::focusedOutlineColourId,
                         TextEditor::shadowColourId };

    for (int i = 0; i < numElementsInArray (cols); ++i)
        ed->setColour (cols[i], findColour (cols[i]));

    return ed;
}

//==============================================================================
void Label::paint (Graphics& g)
{
    getLookAndFeel().drawLabel (g, *this);
}

void Label::mouseUp (const MouseEvent& e)
{
    if (editSingleClick
         && e.mouseWasClicked()
         && contains (e.x, e.y)
         && ! e.mods.isPopupMenu())
    {
        showEditor();
    }
}

void Label::mouseDoubleClick (const MouseEvent& e)
{
    if (editDoubleClick && ! e.mods.isPopupMenu())
        showEditor();
}

void Label::resized()
{
    if (editor != 0)
        editor->setBoundsInset (BorderSize (0));
}

void Label::focusGained (FocusChangeType cause)
{
    if (editSingleClick && cause == focusChangedByTabKey)
        showEditor();
}

void Label::enablementChanged()
{
    repaint();
}

void Label::colourChanged()
{
    repaint();
}

void Label::setMinimumHorizontalScale (const float newScale)
{
    if (minimumHorizontalScale != newScale)
    {
        minimumHorizontalScale = newScale;
        repaint();
    }
}

//==============================================================================
// We'll use a custom focus traverser here to make sure focus goes from the
// text editor to another component rather than back to the label itself.
class LabelKeyboardFocusTraverser   : public KeyboardFocusTraverser
{
public:
    LabelKeyboardFocusTraverser() {}

    Component* getNextComponent (Component* current)
    {
        return KeyboardFocusTraverser::getNextComponent (dynamic_cast <TextEditor*> (current) != 0
                                                            ? current->getParentComponent() : current);
    }

    Component* getPreviousComponent (Component* current)
    {
        return KeyboardFocusTraverser::getPreviousComponent (dynamic_cast <TextEditor*> (current) != 0
                                                                ? current->getParentComponent() : current);
    }
};

KeyboardFocusTraverser* Label::createFocusTraverser()
{
    return new LabelKeyboardFocusTraverser();
}

//==============================================================================
void Label::addListener (LabelListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void Label::removeListener (LabelListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void Label::callChangeListeners()
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((LabelListener*) listeners.getUnchecked (i))->labelTextChanged (this);
        i = jmin (i, listeners.size());
    }
}

//==============================================================================
void Label::textEditorTextChanged (TextEditor& ed)
{
    if (editor != 0)
    {
        jassert (&ed == editor);

        if (! (hasKeyboardFocus (true) || isCurrentlyBlockedByAnotherModalComponent()))
        {
            if (lossOfFocusDiscardsChanges)
                textEditorEscapeKeyPressed (ed);
            else
                textEditorReturnKeyPressed (ed);
        }
    }
}

void Label::textEditorReturnKeyPressed (TextEditor& ed)
{
    if (editor != 0)
    {
        jassert (&ed == editor);
        (void) ed;

        const bool changed = updateFromTextEditorContents();
        hideEditor (true);

        if (changed)
        {
            textWasEdited();

            if (isValidComponent())
                callChangeListeners();
        }
    }
}

void Label::textEditorEscapeKeyPressed (TextEditor& ed)
{
    if (editor != 0)
    {
        jassert (&ed == editor);
        (void) ed;

        editor->setText (textValue.toString(), false);
        hideEditor (true);
    }
}

void Label::textEditorFocusLost (TextEditor& ed)
{
    textEditorTextChanged (ed);
}


END_JUCE_NAMESPACE
