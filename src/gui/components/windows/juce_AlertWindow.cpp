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

#include "juce_AlertWindow.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../buttons/juce_TextButton.h"
#include "../controls/juce_TextEditor.h"
#include "../controls/juce_ProgressBar.h"
#include "../juce_Desktop.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../../../events/juce_MessageManager.h"
#include "../../../application/juce_Application.h"
#include "../../../containers/juce_ScopedPointer.h"

static const int titleH = 24;
static const int iconWidth = 80;


//==============================================================================
class AlertWindowTextEditor  : public TextEditor
{
public:
    static const tchar passwordChar;

    AlertWindowTextEditor (const String& name, const bool isPasswordBox)
        : TextEditor (name, isPasswordBox ? passwordChar :  0)
    {
        setSelectAllWhenFocused (true);
    }

    ~AlertWindowTextEditor()
    {
    }

    void returnPressed()
    {
        // pass these up the component hierarchy to be trigger the buttons
        getParentComponent()->keyPressed (KeyPress (KeyPress::returnKey, 0, T('\n')));
    }

    void escapePressed()
    {
        // pass these up the component hierarchy to be trigger the buttons
        getParentComponent()->keyPressed (KeyPress (KeyPress::escapeKey, 0, 0));
    }

private:
    AlertWindowTextEditor (const AlertWindowTextEditor&);
    const AlertWindowTextEditor& operator= (const AlertWindowTextEditor&);
};

#if JUCE_LINUX
const tchar AlertWindowTextEditor::passwordChar = 0x2022;
#else
const tchar AlertWindowTextEditor::passwordChar = 0x25cf;
#endif

//==============================================================================
AlertWindow::AlertWindow (const String& title,
                          const String& message,
                          AlertIconType iconType,
                          Component* associatedComponent_)
   : TopLevelWindow (title, true),
     alertIconType (iconType),
     associatedComponent (associatedComponent_)
{
    if (message.isEmpty())
        text = T(" "); // to force an update if the message is empty

    setMessage (message);

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        Component* const c = Desktop::getInstance().getComponent (i);

        if (c != 0 && c->isAlwaysOnTop() && c->isShowing())
        {
            setAlwaysOnTop (true);
            break;
        }
    }

    if (JUCEApplication::getInstance() == 0)
        setAlwaysOnTop (true); // for a plugin, make it always-on-top because the host windows are often top-level

    lookAndFeelChanged();

    constrainer.setMinimumOnscreenAmounts (0x10000, 0x10000, 0x10000, 0x10000);
}

AlertWindow::~AlertWindow()
{
    for (int i = customComps.size(); --i >= 0;)
        removeChildComponent ((Component*) customComps[i]);

    deleteAllChildren();
}

void AlertWindow::userTriedToCloseWindow()
{
    exitModalState (0);
}

//==============================================================================
void AlertWindow::setMessage (const String& message)
{
    const String newMessage (message.substring (0, 2048));

    if (text != newMessage)
    {
        text = newMessage;

        font.setHeight (15.0f);

        Font titleFont (font.getHeight() * 1.1f, Font::bold);
        textLayout.setText (getName() + T("\n\n"), titleFont);

        textLayout.appendText (text, font);

        updateLayout (true);
        repaint();
    }
}

//==============================================================================
void AlertWindow::buttonClicked (Button* button)
{
    for (int i = 0; i < buttons.size(); i++)
    {
        TextButton* const c = (TextButton*) buttons[i];

        if (button->getName() == c->getName())
        {
            if (c->getParentComponent() != 0)
                c->getParentComponent()->exitModalState (c->getCommandID());

            break;
        }
    }
}

//==============================================================================
void AlertWindow::addButton (const String& name,
                             const int returnValue,
                             const KeyPress& shortcutKey1,
                             const KeyPress& shortcutKey2)
{
    TextButton* const b = new TextButton (name, String::empty);

    b->setWantsKeyboardFocus (true);
    b->setMouseClickGrabsKeyboardFocus (false);
    b->setCommandToTrigger (0, returnValue, false);
    b->addShortcut (shortcutKey1);
    b->addShortcut (shortcutKey2);
    b->addButtonListener (this);
    b->changeWidthToFitText (getLookAndFeel().getAlertWindowButtonHeight());

    addAndMakeVisible (b, 0);
    buttons.add (b);

    updateLayout (false);
}

int AlertWindow::getNumButtons() const
{
    return buttons.size();
}

//==============================================================================
void AlertWindow::addTextEditor (const String& name,
                                 const String& initialContents,
                                 const String& onScreenLabel,
                                 const bool isPasswordBox)
{
    AlertWindowTextEditor* const tc = new AlertWindowTextEditor (name, isPasswordBox);

    tc->setColour (TextEditor::outlineColourId, findColour (ComboBox::outlineColourId));
    tc->setFont (font);
    tc->setText (initialContents);
    tc->setCaretPosition (initialContents.length());
    addAndMakeVisible (tc);
    textBoxes.add (tc);
    allComps.add (tc);
    textboxNames.add (onScreenLabel);

    updateLayout (false);
}

const String AlertWindow::getTextEditorContents (const String& nameOfTextEditor) const
{
    for (int i = textBoxes.size(); --i >= 0;)
        if (((TextEditor*)textBoxes[i])->getName() == nameOfTextEditor)
            return ((TextEditor*)textBoxes[i])->getText();

    return String::empty;
}


//==============================================================================
void AlertWindow::addComboBox (const String& name,
                               const StringArray& items,
                               const String& onScreenLabel)
{
    ComboBox* const cb = new ComboBox (name);

    for (int i = 0; i < items.size(); ++i)
        cb->addItem (items[i], i + 1);

    addAndMakeVisible (cb);
    cb->setSelectedItemIndex (0);

    comboBoxes.add (cb);
    allComps.add (cb);

    comboBoxNames.add (onScreenLabel);

    updateLayout (false);
}

ComboBox* AlertWindow::getComboBoxComponent (const String& nameOfList) const
{
    for (int i = comboBoxes.size(); --i >= 0;)
        if (((ComboBox*) comboBoxes[i])->getName() == nameOfList)
            return (ComboBox*) comboBoxes[i];

    return 0;
}

//==============================================================================
class AlertTextComp : public TextEditor
{
    AlertTextComp (const AlertTextComp&);
    const AlertTextComp& operator= (const AlertTextComp&);

    int bestWidth;

public:
    AlertTextComp (const String& message,
                   const Font& font)
    {
        setReadOnly (true);
        setMultiLine (true, true);
        setCaretVisible (false);
        setScrollbarsShown (true);
        lookAndFeelChanged();
        setWantsKeyboardFocus (false);

        setFont (font);
        setText (message, false);

        bestWidth = 2 * (int) sqrt (font.getHeight() * font.getStringWidth (message));

        setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
        setColour (TextEditor::outlineColourId, Colours::transparentBlack);
        setColour (TextEditor::shadowColourId, Colours::transparentBlack);
    }

    ~AlertTextComp()
    {
    }

    int getPreferredWidth() const throw()    { return bestWidth; }

    void updateLayout (const int width)
    {
        TextLayout text;
        text.appendText (getText(), getFont());
        text.layout (width - 8, Justification::topLeft, true);
        setSize (width, jmin (width, text.getHeight() + (int) getFont().getHeight()));
    }
};

void AlertWindow::addTextBlock (const String& text)
{
    AlertTextComp* const c = new AlertTextComp (text, font);

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
    return (Component*) customComps [index];
}

Component* AlertWindow::removeCustomComponent (const int index)
{
    Component* const c = getCustomComponent (index);

    if (c != 0)
    {
        customComps.removeValue (c);
        allComps.removeValue (c);
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

    int i;
    for (i = textBoxes.size(); --i >= 0;)
    {
        if (textboxNames[i].isNotEmpty())
        {
            const TextEditor* const te = (TextEditor*) textBoxes[i];

            g.drawFittedText (textboxNames[i],
                              te->getX(), te->getY() - 14,
                              te->getWidth(), 14,
                              Justification::centredLeft, 1);
        }
    }

    for (i = comboBoxNames.size(); --i >= 0;)
    {
        if (comboBoxNames[i].isNotEmpty())
        {
            const ComboBox* const cb = (ComboBox*) comboBoxes[i];

            g.drawFittedText (comboBoxNames[i],
                              cb->getX(), cb->getY() - 14,
                              cb->getWidth(), 14,
                              Justification::centredLeft, 1);
        }
    }
}

void AlertWindow::updateLayout (const bool onlyIncreaseSize)
{
    const int wid = jmax (font.getStringWidth (text),
                          font.getStringWidth (getName()));

    const int sw = (int) sqrt (font.getHeight() * wid);
    int w = jmin (300 + sw * 2, (int) (getParentWidth() * 0.7f));
    const int edgeGap = 10;
    int iconSpace;

    if (alertIconType == NoIcon)
    {
        textLayout.layout (w, Justification::horizontallyCentred, true);
        iconSpace = 0;
    }
    else
    {
        textLayout.layout (w, Justification::left, true);
        iconSpace = iconWidth;
    }

    w = jmax (350, textLayout.getWidth() + iconSpace + edgeGap * 4);
    w = jmin (w, (int) (getParentWidth() * 0.7f));

    const int textLayoutH = textLayout.getHeight();
    const int textBottom = 16 + titleH + textLayoutH;
    int h = textBottom;

    int buttonW = 40;
    int i;
    for (i = 0; i < buttons.size(); ++i)
        buttonW += 16 + ((const TextButton*) buttons[i])->getWidth();

    w = jmax (buttonW, w);

    h += (textBoxes.size() + comboBoxes.size() + progressBars.size()) * 50;

    if (buttons.size() > 0)
        h += 20 + ((TextButton*) buttons[0])->getHeight();

    for (i = customComps.size(); --i >= 0;)
    {
        w = jmax (w, ((Component*) customComps[i])->getWidth() + 40);
        h += 10 + ((Component*) customComps[i])->getHeight();
    }

    for (i = textBlocks.size(); --i >= 0;)
    {
        const AlertTextComp* const ac = (AlertTextComp*) textBlocks[i];
        w = jmax (w, ac->getPreferredWidth());
    }

    w = jmin (w, (int) (getParentWidth() * 0.7f));

    for (i = textBlocks.size(); --i >= 0;)
    {
        AlertTextComp* const ac = (AlertTextComp*) textBlocks[i];
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
    {
        centreAroundComponent (associatedComponent, w, h);
    }
    else
    {
        const int cx = getX() + getWidth() / 2;
        const int cy = getY() + getHeight() / 2;

        setBounds (cx - w / 2,
                   cy - h / 2,
                   w, h);
    }

    textArea.setBounds (edgeGap, edgeGap, w - (edgeGap * 2), h - edgeGap);

    const int spacer = 16;
    int totalWidth = -spacer;

    for (i = buttons.size(); --i >= 0;)
        totalWidth += ((TextButton*) buttons[i])->getWidth() + spacer;

    int x = (w - totalWidth) / 2;
    int y = (int) (getHeight() * 0.95f);

    for (i = 0; i < buttons.size(); ++i)
    {
        TextButton* const c = (TextButton*) buttons[i];
        int ny = proportionOfHeight (0.95f) - c->getHeight();
        c->setTopLeftPosition (x, ny);
        if (ny < y)
            y = ny;

        x += c->getWidth() + spacer;

        c->toFront (false);
    }

    y = textBottom;

    for (i = 0; i < allComps.size(); ++i)
    {
        Component* const c = (Component*) allComps[i];

        const int h = 22;

        const int comboIndex = comboBoxes.indexOf (c);
        if (comboIndex >= 0 && comboBoxNames [comboIndex].isNotEmpty())
            y += 18;

        const int tbIndex = textBoxes.indexOf (c);
        if (tbIndex >= 0 && textboxNames[tbIndex].isNotEmpty())
            y += 18;

        if (customComps.contains (c) || textBlocks.contains (c))
        {
            c->setTopLeftPosition ((getWidth() - c->getWidth()) / 2, y);
            y += c->getHeight() + 10;
        }
        else
        {
            c->setBounds (proportionOfWidth (0.1f), y, proportionOfWidth (0.8f), h);
            y += h + 10;
        }
    }

    setWantsKeyboardFocus (getNumChildComponents() == 0);
}

bool AlertWindow::containsAnyExtraComponents() const
{
    return textBoxes.size()
            + comboBoxes.size()
            + progressBars.size()
            + customComps.size() > 0;
}

//==============================================================================
void AlertWindow::mouseDown (const MouseEvent&)
{
    dragger.startDraggingComponent (this, &constrainer);
}

void AlertWindow::mouseDrag (const MouseEvent& e)
{
    dragger.dragComponent (this, e);
}

bool AlertWindow::keyPressed (const KeyPress& key)
{
    for (int i = buttons.size(); --i >= 0;)
    {
        TextButton* const b = (TextButton*) buttons[i];

        if (b->isRegisteredForShortcut (key))
        {
            b->triggerClick();
            return true;
        }
    }

    if (key.isKeyCode (KeyPress::escapeKey) && buttons.size() == 0)
    {
        exitModalState (0);
        return true;
    }
    else if (key.isKeyCode (KeyPress::returnKey) && buttons.size() == 1)
    {
        ((TextButton*) buttons.getFirst())->triggerClick();
        return true;
    }

    return false;
}

void AlertWindow::lookAndFeelChanged()
{
    const int flags = getLookAndFeel().getAlertBoxWindowFlags();

    setUsingNativeTitleBar ((flags & ComponentPeer::windowHasTitleBar) != 0);
    setDropShadowEnabled (isOpaque() && (flags & ComponentPeer::windowHasDropShadow) != 0);
}

int AlertWindow::getDesktopWindowStyleFlags() const
{
    return getLookAndFeel().getAlertBoxWindowFlags();
}

//==============================================================================
struct AlertWindowInfo
{
    String title, message, button1, button2, button3;
    AlertWindow::AlertIconType iconType;
    int numButtons;
    Component* associatedComponent;

    int run() const
    {
        return (int) (pointer_sized_int)
                MessageManager::getInstance()->callFunctionOnMessageThread (showCallback, (void*) this);
    }

private:
    int show() const
    {
        jassert (associatedComponent == 0 || associatedComponent->isValidComponent()); // has your comp been deleted?

        LookAndFeel& lf = associatedComponent->isValidComponent() ? associatedComponent->getLookAndFeel()
                                                                  : LookAndFeel::getDefaultLookAndFeel();

        ScopedPointer <Component> alertBox (lf.createAlertWindow (title, message, button1, button2, button3,
                                                                  iconType, numButtons, associatedComponent));

        jassert (alertBox != 0); // you have to return one of these!

        return alertBox->runModalLoop();
    }

    static void* showCallback (void* userData)
    {
        return (void*) (pointer_sized_int) ((const AlertWindowInfo*) userData)->show();
    }
};

void AlertWindow::showMessageBox (AlertIconType iconType,
                                  const String& title,
                                  const String& message,
                                  const String& buttonText,
                                  Component* associatedComponent)
{
    AlertWindowInfo info;
    info.title = title;
    info.message = message;
    info.button1 = buttonText.isEmpty() ? TRANS("ok") : buttonText;
    info.iconType = iconType;
    info.numButtons = 1;
    info.associatedComponent = associatedComponent;

    info.run();
}

bool AlertWindow::showOkCancelBox (AlertIconType iconType,
                                   const String& title,
                                   const String& message,
                                   const String& button1Text,
                                   const String& button2Text,
                                   Component* associatedComponent)
{
    AlertWindowInfo info;
    info.title = title;
    info.message = message;
    info.button1 = button1Text.isEmpty() ? TRANS("ok")     : button1Text;
    info.button2 = button2Text.isEmpty() ? TRANS("cancel") : button2Text;
    info.iconType = iconType;
    info.numButtons = 2;
    info.associatedComponent = associatedComponent;

    return info.run() != 0;
}

int AlertWindow::showYesNoCancelBox (AlertIconType iconType,
                                     const String& title,
                                     const String& message,
                                     const String& button1Text,
                                     const String& button2Text,
                                     const String& button3Text,
                                     Component* associatedComponent)
{
    AlertWindowInfo info;
    info.title = title;
    info.message = message;
    info.button1 = button1Text.isEmpty() ? TRANS("yes")     : button1Text;
    info.button2 = button2Text.isEmpty() ? TRANS("no")      : button2Text;
    info.button3 = button3Text.isEmpty() ? TRANS("cancel")  : button3Text;
    info.iconType = iconType;
    info.numButtons = 3;
    info.associatedComponent = associatedComponent;

    return info.run();
}

END_JUCE_NAMESPACE
