/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

Label::Label (const String& name, const String& labelText)
    : Component (name),
      textValue (labelText),
      lastTextValue (labelText),
      font (15.0f),
      justification (Justification::centredLeft),
      border (1, 5, 1, 5),
      minimumHorizontalScale (0.0f),
      keyboardType (TextEditor::textKeyboard),
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

    if (ownerComponent != nullptr)
        ownerComponent->removeComponentListener (this);

    editor = nullptr;
}

//==============================================================================
void Label::setText (const String& newText,
                     const NotificationType notification)
{
    hideEditor (true);

    if (lastTextValue != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != nullptr)
            componentMovedOrResized (*ownerComponent, true, true);

        if (notification != dontSendNotification)
            callChangeListeners();
    }
}

String Label::getText (const bool returnActiveEditorContents) const
{
    return (returnActiveEditorContents && isBeingEdited())
                ? editor->getText()
                : textValue.toString();
}

void Label::valueChanged (Value&)
{
    if (lastTextValue != textValue.toString())
        setText (textValue.toString(), sendNotification);
}

//==============================================================================
void Label::setFont (const Font& newFont)
{
    if (font != newFont)
    {
        font = newFont;
        repaint();
    }
}

Font Label::getFont() const noexcept
{
    return font;
}

void Label::setEditable (const bool editOnSingleClick,
                         const bool editOnDoubleClick,
                         const bool lossOfFocusDiscardsChanges_)
{
    editSingleClick = editOnSingleClick;
    editDoubleClick = editOnDoubleClick;
    lossOfFocusDiscardsChanges = lossOfFocusDiscardsChanges_;

    setWantsKeyboardFocus (editOnSingleClick || editOnDoubleClick);
    setFocusContainer (editOnSingleClick || editOnDoubleClick);
}

void Label::setJustificationType (Justification newJustification)
{
    if (justification != newJustification)
    {
        justification = newJustification;
        repaint();
    }
}

void Label::setBorderSize (BorderSize<int> newBorder)
{
    if (border != newBorder)
    {
        border = newBorder;
        repaint();
    }
}

//==============================================================================
Component* Label::getAttachedComponent() const
{
    return static_cast<Component*> (ownerComponent);
}

void Label::attachToComponent (Component* owner, const bool onLeft)
{
    if (ownerComponent != nullptr)
        ownerComponent->removeComponentListener (this);

    ownerComponent = owner;

    leftOfOwnerComp = onLeft;

    if (ownerComponent != nullptr)
    {
        setVisible (owner->isVisible());
        ownerComponent->addComponentListener (this);
        componentParentHierarchyChanged (*ownerComponent);
        componentMovedOrResized (*ownerComponent, true, true);
    }
}

void Label::componentMovedOrResized (Component& component, bool /*wasMoved*/, bool /*wasResized*/)
{
    const Font f (getLookAndFeel().getLabelFont (*this));

    if (leftOfOwnerComp)
    {
        setSize (jmin (roundToInt (f.getStringWidthFloat (textValue.toString()) + 0.5f) + getBorderSize().getLeftAndRight(),
                       component.getX()),
                 component.getHeight());

        setTopRightPosition (component.getX(), component.getY());
    }
    else
    {
        setSize (component.getWidth(),
                 getBorderSize().getTopAndBottom() + 6 + roundToInt (f.getHeight() + 0.5f));

        setTopLeftPosition (component.getX(), component.getY() - getHeight());
    }
}

void Label::componentParentHierarchyChanged (Component& component)
{
    if (Component* parent = component.getParentComponent())
        parent->addChildComponent (this);
}

void Label::componentVisibilityChanged (Component& component)
{
    setVisible (component.isVisible());
}

//==============================================================================
void Label::textWasEdited() {}
void Label::textWasChanged() {}

void Label::editorShown (TextEditor* textEditor)
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &LabelListener::editorShown, this, *textEditor);
}

void Label::editorAboutToBeHidden (TextEditor*)
{
    if (ComponentPeer* const peer = getPeer())
        peer->dismissPendingTextInput();
}

void Label::showEditor()
{
    if (editor == nullptr)
    {
        addAndMakeVisible (editor = createEditorComponent());
        editor->setText (getText(), false);
        editor->setKeyboardType (keyboardType);
        editor->addListener (this);
        editor->grabKeyboardFocus();

        if (editor == nullptr) // may be deleted by a callback
            return;

        editor->setHighlightedRegion (Range<int> (0, textValue.toString().length()));

        resized();
        repaint();

        editorShown (editor);

        enterModalState (false);
        editor->grabKeyboardFocus();
    }
}

bool Label::updateFromTextEditorContents (TextEditor& ed)
{
    const String newText (ed.getText());

    if (textValue.toString() != newText)
    {
        lastTextValue = newText;
        textValue = newText;
        repaint();

        textWasChanged();

        if (ownerComponent != nullptr)
            componentMovedOrResized (*ownerComponent, true, true);

        return true;
    }

    return false;
}

void Label::hideEditor (const bool discardCurrentEditorContents)
{
    if (editor != nullptr)
    {
        WeakReference<Component> deletionChecker (this);

        ScopedPointer<TextEditor> outgoingEditor (editor);

        editorAboutToBeHidden (outgoingEditor);

        const bool changed = (! discardCurrentEditorContents)
                               && updateFromTextEditorContents (*outgoingEditor);
        outgoingEditor = nullptr;
        repaint();

        if (changed)
            textWasEdited();

        if (deletionChecker != nullptr)
            exitModalState (0);

        if (changed && deletionChecker != nullptr)
            callChangeListeners();
    }
}

void Label::inputAttemptWhenModal()
{
    if (editor != nullptr)
    {
        if (lossOfFocusDiscardsChanges)
            textEditorEscapeKeyPressed (*editor);
        else
            textEditorReturnKeyPressed (*editor);
    }
}

bool Label::isBeingEdited() const noexcept
{
    return editor != nullptr;
}

static void copyColourIfSpecified (Label& l, TextEditor& ed, int colourID, int targetColourID)
{
    if (l.isColourSpecified (colourID) || l.getLookAndFeel().isColourSpecified (colourID))
        ed.setColour (targetColourID, l.findColour (colourID));
}

TextEditor* Label::createEditorComponent()
{
    TextEditor* const ed = new TextEditor (getName());
    ed->applyFontToAllText (getLookAndFeel().getLabelFont (*this));
    copyAllExplicitColoursTo (*ed);

    copyColourIfSpecified (*this, *ed, textWhenEditingColourId, TextEditor::textColourId);
    copyColourIfSpecified (*this, *ed, backgroundWhenEditingColourId, TextEditor::backgroundColourId);
    copyColourIfSpecified (*this, *ed, outlineWhenEditingColourId, TextEditor::outlineColourId);

    return ed;
}

TextEditor* Label::getCurrentTextEditor() const noexcept
{
    return editor;
}

//==============================================================================
void Label::paint (Graphics& g)
{
    getLookAndFeel().drawLabel (g, *this);
}

void Label::mouseUp (const MouseEvent& e)
{
    if (editSingleClick
         && isEnabled()
         && e.mouseWasClicked()
         && contains (e.getPosition())
         && ! e.mods.isPopupMenu())
    {
        showEditor();
    }
}

void Label::mouseDoubleClick (const MouseEvent& e)
{
    if (editDoubleClick
         && isEnabled()
         && ! e.mods.isPopupMenu())
        showEditor();
}

void Label::resized()
{
    if (editor != nullptr)
        editor->setBounds (getLocalBounds());
}

void Label::focusGained (FocusChangeType cause)
{
    if (editSingleClick
         && isEnabled()
         && cause == focusChangedByTabKey)
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

    Component* getNextComponent (Component* c)     { return KeyboardFocusTraverser::getNextComponent (getComp (c)); }
    Component* getPreviousComponent (Component* c) { return KeyboardFocusTraverser::getPreviousComponent (getComp (c)); }

    static Component* getComp (Component* current)
    {
        return dynamic_cast <TextEditor*> (current) != nullptr
                 ? current->getParentComponent() : current;
    }
};

KeyboardFocusTraverser* Label::createFocusTraverser()
{
    return new LabelKeyboardFocusTraverser();
}

//==============================================================================
void Label::addListener (LabelListener* const listener)
{
    listeners.add (listener);
}

void Label::removeListener (LabelListener* const listener)
{
    listeners.remove (listener);
}

void Label::callChangeListeners()
{
    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &LabelListener::labelTextChanged, this);  // (can't use Label::Listener due to idiotic VC2005 bug)
}

//==============================================================================
void Label::textEditorTextChanged (TextEditor& ed)
{
    if (editor != nullptr)
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
    if (editor != nullptr)
    {
        jassert (&ed == editor);

        const bool changed = updateFromTextEditorContents (ed);
        hideEditor (true);

        if (changed)
        {
            WeakReference<Component> deletionChecker (this);
            textWasEdited();

            if (deletionChecker != nullptr)
                callChangeListeners();
        }
    }
}

void Label::textEditorEscapeKeyPressed (TextEditor& ed)
{
    if (editor != nullptr)
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
