/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

static juce_wchar getDefaultPasswordChar() noexcept
{
   #if JUCE_LINUX
    return 0x2022;
   #else
    return 0x25cf;
   #endif
}

//==============================================================================
AlertWindow::AlertWindow (const String& title,
                          const String& message,
                          AlertIconType iconType,
                          Component* comp)
   : TopLevelWindow (title, true),
     alertIconType (iconType),
     associatedComponent (comp),
     escapeKeyCancels (true)
{
    setAlwaysOnTop (juce_areThereAnyAlwaysOnTopWindows());

    if (message.isEmpty())
        text = " "; // to force an update if the message is empty

    setMessage (message);

    AlertWindow::lookAndFeelChanged();
    constrainer.setMinimumOnscreenAmounts (0x10000, 0x10000, 0x10000, 0x10000);
}

AlertWindow::~AlertWindow()
{
    removeAllChildren();
}

void AlertWindow::userTriedToCloseWindow()
{
    if (escapeKeyCancels || buttons.size() > 0)
        exitModalState (0);
}

//==============================================================================
void AlertWindow::setMessage (const String& message)
{
    const String newMessage (message.substring (0, 2048));

    if (text != newMessage)
    {
        text = newMessage;
        updateLayout (true);
        repaint();
    }
}

//==============================================================================
void AlertWindow::buttonClicked (Button* button)
{
    if (Component* parent = button->getParentComponent())
        parent->exitModalState (button->getCommandID());
}

//==============================================================================
void AlertWindow::addButton (const String& name,
                             const int returnValue,
                             const KeyPress& shortcutKey1,
                             const KeyPress& shortcutKey2)
{
    TextButton* const b = new TextButton (name, String());
    buttons.add (b);

    b->setWantsKeyboardFocus (true);
    b->setMouseClickGrabsKeyboardFocus (false);
    b->setCommandToTrigger (0, returnValue, false);
    b->addShortcut (shortcutKey1);
    b->addShortcut (shortcutKey2);
    b->addListener (this);
    b->changeWidthToFitText (getLookAndFeel().getAlertWindowButtonHeight());

    addAndMakeVisible (b, 0);

    updateLayout (false);
}

int AlertWindow::getNumButtons() const
{
    return buttons.size();
}

void AlertWindow::triggerButtonClick (const String& buttonName)
{
    for (int i = buttons.size(); --i >= 0;)
    {
        TextButton* const b = buttons.getUnchecked(i);

        if (buttonName == b->getName())
        {
            b->triggerClick();
            break;
        }
    }
}

void AlertWindow::setEscapeKeyCancels (bool shouldEscapeKeyCancel)
{
    escapeKeyCancels = shouldEscapeKeyCancel;
}

//==============================================================================
void AlertWindow::addTextEditor (const String& name,
                                 const String& initialContents,
                                 const String& onScreenLabel,
                                 const bool isPasswordBox)
{
    TextEditor* ed = new TextEditor (name, isPasswordBox ? getDefaultPasswordChar() : 0);
    ed->setSelectAllWhenFocused (true);
    ed->setEscapeAndReturnKeysConsumed (false);
    textBoxes.add (ed);
    allComps.add (ed);

    ed->setColour (TextEditor::outlineColourId, findColour (ComboBox::outlineColourId));
    ed->setFont (getLookAndFeel().getAlertWindowMessageFont());
    addAndMakeVisible (ed);
    ed->setText (initialContents);
    ed->setCaretPosition (initialContents.length());
    textboxNames.add (onScreenLabel);

    updateLayout (false);
}

TextEditor* AlertWindow::getTextEditor (const String& nameOfTextEditor) const
{
    for (int i = textBoxes.size(); --i >= 0;)
        if (textBoxes.getUnchecked(i)->getName() == nameOfTextEditor)
            return textBoxes.getUnchecked(i);

    return nullptr;
}

String AlertWindow::getTextEditorContents (const String& nameOfTextEditor) const
{
    if (TextEditor* const t = getTextEditor (nameOfTextEditor))
        return t->getText();

    return String();
}


//==============================================================================
void AlertWindow::addComboBox (const String& name,
                               const StringArray& items,
                               const String& onScreenLabel)
{
    ComboBox* const cb = new ComboBox (name);
    comboBoxes.add (cb);
    allComps.add (cb);

    cb->addItemList (items, 1);

    addAndMakeVisible (cb);
    cb->setSelectedItemIndex (0);

    comboBoxNames.add (onScreenLabel);
    updateLayout (false);
}

ComboBox* AlertWindow::getComboBoxComponent (const String& nameOfList) const
{
    for (int i = comboBoxes.size(); --i >= 0;)
        if (comboBoxes.getUnchecked(i)->getName() == nameOfList)
            return comboBoxes.getUnchecked(i);

    return nullptr;
}

//==============================================================================
class AlertTextComp  : public TextEditor
{
public:
    AlertTextComp (AlertWindow& owner, const String& message, const Font& font)
    {
        setReadOnly (true);
        setMultiLine (true, true);
        setCaretVisible (false);
        setScrollbarsShown (true);
        lookAndFeelChanged();
        setWantsKeyboardFocus (false);

        setFont (font);
        setText (message, false);

        bestWidth = 2 * (int) std::sqrt (font.getHeight() * font.getStringWidth (message));

        if (owner.isColourSpecified (AlertWindow::textColourId))
            setColour (TextEditor::textColourId, owner.findColour (AlertWindow::textColourId));

        setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
        setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        setColour (TextEditor::shadowColourId, Colours::transparentBlack);
    }

    int getPreferredWidth() const noexcept   { return bestWidth; }

    void updateLayout (const int width)
    {
        AttributedString s;
        s.setJustification (Justification::topLeft);
        s.append (getText(), getFont());

        TextLayout text;
        text.createLayoutWithBalancedLineLengths (s, width - 8.0f);
        setSize (width, jmin (width, (int) (text.getHeight() + getFont().getHeight())));
    }

private:
    int bestWidth;

    JUCE_DECLARE_NON_COPYABLE (AlertTextComp)
};

void AlertWindow::addTextBlock (const String& textBlock)
{
    AlertTextComp* const c = new AlertTextComp (*this, textBlock, getLookAndFeel().getAlertWindowMessageFont());
    textBlocks.add (c);
    allComps.add (c);

    addAndMakeVisible (c);

    updateLayout (false);
}

//==============================================================================
void AlertWindow::addProgressBarComponent (double& progressValue)
{
    ProgressBar* const pb = new ProgressBar (progressValue);
    progressBars.add (pb);
    allComps.add (pb);

    addAndMakeVisible (pb);

    updateLayout (false);
}

//==============================================================================
void AlertWindow::addCustomComponent (Component* const component)
{
    customComps.add (component);
    allComps.add (component);

    addAndMakeVisible (component);

    updateLayout (false);
}

int AlertWindow::getNumCustomComponents() const
{
    return customComps.size();
}

Component* AlertWindow::getCustomComponent (const int index) const
{
    return customComps [index];
}

Component* AlertWindow::removeCustomComponent (const int index)
{
    Component* const c = getCustomComponent (index);

    if (c != nullptr)
    {
        customComps.removeFirstMatchingValue (c);
        allComps.removeFirstMatchingValue (c);
        removeChildComponent (c);

        updateLayout (false);
    }

    return c;
}

//==============================================================================
void AlertWindow::paint (Graphics& g)
{
    getLookAndFeel().drawAlertBox (g, *this, textArea, textLayout);

    g.setColour (findColour (textColourId));
    g.setFont (getLookAndFeel().getAlertWindowFont());

    for (int i = textBoxes.size(); --i >= 0;)
    {
        const TextEditor* const te = textBoxes.getUnchecked(i);

        g.drawFittedText (textboxNames[i],
                          te->getX(), te->getY() - 14,
                          te->getWidth(), 14,
                          Justification::centredLeft, 1);
    }

    for (int i = comboBoxNames.size(); --i >= 0;)
    {
        const ComboBox* const cb = comboBoxes.getUnchecked(i);

        g.drawFittedText (comboBoxNames[i],
                          cb->getX(), cb->getY() - 14,
                          cb->getWidth(), 14,
                          Justification::centredLeft, 1);
    }

    for (int i = customComps.size(); --i >= 0;)
    {
        const Component* const c = customComps.getUnchecked(i);

        g.drawFittedText (c->getName(),
                          c->getX(), c->getY() - 14,
                          c->getWidth(), 14,
                          Justification::centredLeft, 1);
    }
}

void AlertWindow::updateLayout (const bool onlyIncreaseSize)
{
    const int titleH = 24;
    const int iconWidth = 80;

    LookAndFeel& lf = getLookAndFeel();

    const Font messageFont (lf.getAlertWindowMessageFont());

    const int wid = jmax (messageFont.getStringWidth (text),
                          messageFont.getStringWidth (getName()));

    const int sw = (int) std::sqrt (messageFont.getHeight() * wid);
    int w = jmin (300 + sw * 2, (int) (getParentWidth() * 0.7f));
    const int edgeGap = 10;
    const int labelHeight = 18;
    int iconSpace = 0;

    AttributedString attributedText;
    attributedText.append (getName(), lf.getAlertWindowTitleFont());

    if (text.isNotEmpty())
        attributedText.append ("\n\n" + text, messageFont);

    attributedText.setColour (findColour (textColourId));

    if (alertIconType == NoIcon)
    {
        attributedText.setJustification (Justification::centredTop);
        textLayout.createLayoutWithBalancedLineLengths (attributedText, (float) w);
    }
    else
    {
        attributedText.setJustification (Justification::topLeft);
        textLayout.createLayoutWithBalancedLineLengths (attributedText, (float) w);
        iconSpace = iconWidth;
    }

    w = jmax (350, (int) textLayout.getWidth() + iconSpace + edgeGap * 4);
    w = jmin (w, (int) (getParentWidth() * 0.7f));

    const int textLayoutH = (int) textLayout.getHeight();
    const int textBottom = 16 + titleH + textLayoutH;
    int h = textBottom;

    int buttonW = 40;
    for (int i = 0; i < buttons.size(); ++i)
        buttonW += 16 + buttons.getUnchecked (i)->getWidth();

    w = jmax (buttonW, w);

    h += (textBoxes.size() + comboBoxes.size() + progressBars.size()) * 50;

    if (buttons.size() > 0)
        h += 20 + buttons.getUnchecked (0)->getHeight();

    for (int i = customComps.size(); --i >= 0;)
    {
        Component* c = customComps.getUnchecked (i);
        w = jmax (w, (c->getWidth() * 100) / 80);
        h += 10 + c->getHeight();

        if (c->getName().isNotEmpty())
            h += labelHeight;
    }

    for (int i = textBlocks.size(); --i >= 0;)
    {
        const AlertTextComp* const ac = static_cast<const AlertTextComp*> (textBlocks.getUnchecked(i));
        w = jmax (w, ac->getPreferredWidth());
    }

    w = jmin (w, (int) (getParentWidth() * 0.7f));

    for (int i = textBlocks.size(); --i >= 0;)
    {
        AlertTextComp* const ac = static_cast<AlertTextComp*> (textBlocks.getUnchecked(i));
        ac->updateLayout ((int) (w * 0.8f));
        h += ac->getHeight() + 10;
    }

    h = jmin (getParentHeight() - 50, h);

    if (onlyIncreaseSize)
    {
        w = jmax (w, getWidth());
        h = jmax (h, getHeight());
    }

    if (! isVisible())
        centreAroundComponent (associatedComponent, w, h);
    else
        setBounds (getBounds().withSizeKeepingCentre (w, h));

    textArea.setBounds (edgeGap, edgeGap, w - (edgeGap * 2), h - edgeGap);

    const int spacer = 16;
    int totalWidth = -spacer;

    for (int i = buttons.size(); --i >= 0;)
        totalWidth += buttons.getUnchecked(i)->getWidth() + spacer;

    int x = (w - totalWidth) / 2;
    int y = (int) (getHeight() * 0.95f);

    for (int i = 0; i < buttons.size(); ++i)
    {
        TextButton* const c = buttons.getUnchecked(i);
        int ny = proportionOfHeight (0.95f) - c->getHeight();
        c->setTopLeftPosition (x, ny);
        if (ny < y)
            y = ny;

        x += c->getWidth() + spacer;

        c->toFront (false);
    }

    y = textBottom;

    for (int i = 0; i < allComps.size(); ++i)
    {
        Component* const c = allComps.getUnchecked(i);
        h = 22;

        const int comboIndex = comboBoxes.indexOf (dynamic_cast<ComboBox*> (c));
        if (comboIndex >= 0 && comboBoxNames [comboIndex].isNotEmpty())
            y += labelHeight;

        const int tbIndex = textBoxes.indexOf (dynamic_cast<TextEditor*> (c));
        if (tbIndex >= 0 && textboxNames[tbIndex].isNotEmpty())
            y += labelHeight;

        if (customComps.contains (c))
        {
            if (c->getName().isNotEmpty())
                y += labelHeight;

            c->setTopLeftPosition (proportionOfWidth (0.1f), y);
            h = c->getHeight();
        }
        else if (textBlocks.contains (c))
        {
            c->setTopLeftPosition ((getWidth() - c->getWidth()) / 2, y);
            h = c->getHeight();
        }
        else
        {
            c->setBounds (proportionOfWidth (0.1f), y, proportionOfWidth (0.8f), h);
        }

        y += h + 10;
    }

    setWantsKeyboardFocus (getNumChildComponents() == 0);
}

bool AlertWindow::containsAnyExtraComponents() const
{
    return allComps.size() > 0;
}

//==============================================================================
void AlertWindow::mouseDown (const MouseEvent& e)
{
    dragger.startDraggingComponent (this, e);
}

void AlertWindow::mouseDrag (const MouseEvent& e)
{
    dragger.dragComponent (this, e, &constrainer);
}

bool AlertWindow::keyPressed (const KeyPress& key)
{
    for (int i = buttons.size(); --i >= 0;)
    {
        TextButton* const b = buttons.getUnchecked(i);

        if (b->isRegisteredForShortcut (key))
        {
            b->triggerClick();
            return true;
        }
    }

    if (key.isKeyCode (KeyPress::escapeKey) && escapeKeyCancels && buttons.size() == 0)
    {
        exitModalState (0);
        return true;
    }

    if (key.isKeyCode (KeyPress::returnKey) && buttons.size() == 1)
    {
        buttons.getUnchecked(0)->triggerClick();
        return true;
    }

    return false;
}

void AlertWindow::lookAndFeelChanged()
{
    const int newFlags = getLookAndFeel().getAlertBoxWindowFlags();

    setUsingNativeTitleBar ((newFlags & ComponentPeer::windowHasTitleBar) != 0);
    setDropShadowEnabled (isOpaque() && (newFlags & ComponentPeer::windowHasDropShadow) != 0);
    updateLayout (false);
}

int AlertWindow::getDesktopWindowStyleFlags() const
{
    return getLookAndFeel().getAlertBoxWindowFlags();
}

//==============================================================================
class AlertWindowInfo
{
public:
    AlertWindowInfo (const String& t, const String& m, Component* component,
                     AlertWindow::AlertIconType icon, int numButts,
                     ModalComponentManager::Callback* cb, bool runModally)
        : title (t), message (m), iconType (icon), numButtons (numButts),
          returnValue (0), associatedComponent (component),
          callback (cb), modal (runModally)
    {
    }

    String title, message, button1, button2, button3;

    int invoke() const
    {
        MessageManager::getInstance()->callFunctionOnMessageThread (showCallback, (void*) this);
        return returnValue;
    }

private:
    AlertWindow::AlertIconType iconType;
    int numButtons, returnValue;
    WeakReference<Component> associatedComponent;
    ModalComponentManager::Callback* callback;
    bool modal;

    void show()
    {
        LookAndFeel& lf = associatedComponent != nullptr ? associatedComponent->getLookAndFeel()
                                                         : LookAndFeel::getDefaultLookAndFeel();

        ScopedPointer<Component> alertBox (lf.createAlertWindow (title, message, button1, button2, button3,
                                                                 iconType, numButtons, associatedComponent));

        jassert (alertBox != nullptr); // you have to return one of these!

       #if JUCE_MODAL_LOOPS_PERMITTED
        if (modal)
        {
            returnValue = alertBox->runModalLoop();
        }
        else
       #endif
        {
            ignoreUnused (modal);

            alertBox->enterModalState (true, callback, true);
            alertBox.release();
        }
    }

    static void* showCallback (void* userData)
    {
        static_cast<AlertWindowInfo*> (userData)->show();
        return nullptr;
    }
};

#if JUCE_MODAL_LOOPS_PERMITTED
void AlertWindow::showMessageBox (AlertIconType iconType,
                                  const String& title,
                                  const String& message,
                                  const String& buttonText,
                                  Component* associatedComponent)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
    {
        NativeMessageBox::showMessageBox (iconType, title, message, associatedComponent);
    }
    else
    {
        AlertWindowInfo info (title, message, associatedComponent, iconType, 1, nullptr, true);
        info.button1 = buttonText.isEmpty() ? TRANS("OK") : buttonText;

        info.invoke();
    }
}
#endif

void AlertWindow::showMessageBoxAsync (AlertIconType iconType,
                                       const String& title,
                                       const String& message,
                                       const String& buttonText,
                                       Component* associatedComponent,
                                       ModalComponentManager::Callback* callback)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
    {
        NativeMessageBox::showMessageBoxAsync (iconType, title, message, associatedComponent, callback);
    }
    else
    {
        AlertWindowInfo info (title, message, associatedComponent, iconType, 1, callback, false);
        info.button1 = buttonText.isEmpty() ? TRANS("OK") : buttonText;

        info.invoke();
    }
}

bool AlertWindow::showOkCancelBox (AlertIconType iconType,
                                   const String& title,
                                   const String& message,
                                   const String& button1Text,
                                   const String& button2Text,
                                   Component* associatedComponent,
                                   ModalComponentManager::Callback* callback)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        return NativeMessageBox::showOkCancelBox (iconType, title, message, associatedComponent, callback);

    AlertWindowInfo info (title, message, associatedComponent, iconType, 2, callback, callback == nullptr);
    info.button1 = button1Text.isEmpty() ? TRANS("OK")     : button1Text;
    info.button2 = button2Text.isEmpty() ? TRANS("Cancel") : button2Text;

    return info.invoke() != 0;
}

int AlertWindow::showYesNoCancelBox (AlertIconType iconType,
                                     const String& title,
                                     const String& message,
                                     const String& button1Text,
                                     const String& button2Text,
                                     const String& button3Text,
                                     Component* associatedComponent,
                                     ModalComponentManager::Callback* callback)
{
    if (LookAndFeel::getDefaultLookAndFeel().isUsingNativeAlertWindows())
        return NativeMessageBox::showYesNoCancelBox (iconType, title, message, associatedComponent, callback);

    AlertWindowInfo info (title, message, associatedComponent, iconType, 3, callback, callback == nullptr);
    info.button1 = button1Text.isEmpty() ? TRANS("Yes")     : button1Text;
    info.button2 = button2Text.isEmpty() ? TRANS("No")      : button2Text;
    info.button3 = button3Text.isEmpty() ? TRANS("Cancel")  : button3Text;

    return info.invoke();
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    if (isOkCancel)
        return NativeMessageBox::showOkCancelBox (AlertWindow::NoIcon, title, bodyText);

    NativeMessageBox::showMessageBox (AlertWindow::NoIcon, title, bodyText);
    return true;
}
#endif
