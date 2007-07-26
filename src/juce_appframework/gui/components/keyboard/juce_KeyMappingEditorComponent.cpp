/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

// N.B. these two includes are put here deliberately to avoid problems with
// old GCCs failing on long include paths
#include "../../../../juce_core/containers/juce_VoidArray.h"
#include "../../../../juce_core/containers/juce_OwnedArray.h"

#include "juce_KeyMappingEditorComponent.h"
#include "../menus/juce_PopupMenu.h"
#include "../windows/juce_AlertWindow.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"

const int maxKeys = 3;

//==============================================================================
class KeyMappingChangeButton  : public Button
{
public:
    KeyMappingChangeButton (KeyMappingEditorComponent* const owner_,
                            const CommandID commandID_,
                            const String& keyName,
                            const int keyNum_)
        : Button (keyName),
          owner (owner_),
          commandID (commandID_),
          keyNum (keyNum_)
    {
        setWantsKeyboardFocus (false);
        setTriggeredOnMouseDown (keyNum >= 0);

        if (keyNum_ < 0)
            setTooltip (TRANS("adds a new key-mapping"));
        else
            setTooltip (TRANS("click to change this key-mapping"));
    }

    ~KeyMappingChangeButton()
    {
    }

    void paintButton (Graphics& g, bool isOver, bool isDown)
    {
        if (keyNum >= 0)
        {
            if (isEnabled())
            {
                const float alpha = isDown ? 0.3f : (isOver ? 0.15f : 0.08f);
                g.fillAll (owner->textColour.withAlpha (alpha));

                g.setOpacity (0.3f);
                g.drawBevel (0, 0, getWidth(), getHeight(), 2);
            }

            g.setColour (owner->textColour);
            g.setFont (getHeight() * 0.6f);
            g.drawFittedText (getName(),
                              3, 0, getWidth() - 6, getHeight(),
                              Justification::centred, 1);
        }
        else
        {
            const float thickness = 7.0f;
            const float indent = 22.0f;

            Path p;
            p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
            p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
            p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
            p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
            p.setUsingNonZeroWinding (false);

            g.setColour (owner->textColour.withAlpha (isDown ? 0.7f : (isOver ? 0.5f : 0.3f)));
            g.fillPath (p, p.getTransformToScaleToFit (2.0f, 2.0f, getWidth() - 4.0f, getHeight() - 4.0f, true));
        }

        if (hasKeyboardFocus (false))
        {
            g.setColour (owner->textColour.withAlpha (0.4f));
            g.drawRect (0, 0, getWidth(), getHeight());
        }
    }

    void clicked()
    {
        if (keyNum >= 0)
        {
            // existing key clicked..
            PopupMenu m;
            m.addItem (1, TRANS("change this key-mapping"));
            m.addSeparator();
            m.addItem (2, TRANS("remove this key-mapping"));

            const int res = m.show();

            if (res == 1)
            {
                owner->assignNewKey (commandID, keyNum);
            }
            else if (res == 2)
            {
                owner->getMappings()->removeKeyPress (commandID, keyNum);
            }
        }
        else
        {
            // + button pressed..
            owner->assignNewKey (commandID, -1);
        }
    }

    void fitToContent (int h)
    {
        if (keyNum < 0)
        {
            setSize (h, h);
        }
        else
        {
            Font f (h * 0.6f);
            setSize (jlimit (h * 4, h * 8, 6 + f.getStringWidth (getName())), h);
        }
    }

private:
    KeyMappingEditorComponent* const owner;
    const CommandID commandID;
    const int keyNum;

    KeyMappingChangeButton (const KeyMappingChangeButton&);
    const KeyMappingChangeButton& operator= (const KeyMappingChangeButton&);
};

//==============================================================================
class KeyMappingItemComponent : public Component
{
public:
    KeyMappingItemComponent (KeyMappingEditorComponent* const owner_,
                             const CommandID commandID_)
        : owner (owner_),
          commandID (commandID_)
    {
        setInterceptsMouseClicks (false, true);

        const bool isReadOnly = owner->isCommandReadOnly (commandID);

        Array <KeyPress> keyPresses (owner->getMappings()->getKeyPressesAssignedToCommand (commandID));

        for (int i = 0; i < jmin (maxKeys, keyPresses.size()); ++i)
        {
            KeyMappingChangeButton* const kb
                = new KeyMappingChangeButton (owner, commandID,
                                              owner->getDescriptionForKeyPress (keyPresses.getReference (i)), i);

            kb->setEnabled (! isReadOnly);
            addAndMakeVisible (kb);
        }

        KeyMappingChangeButton* kb
            = new KeyMappingChangeButton (owner, commandID, String::empty, -1);

        addChildComponent (kb);
        kb->setVisible (keyPresses.size() < maxKeys && ! isReadOnly);
    }

    ~KeyMappingItemComponent()
    {
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        g.setFont (getHeight() * 0.7f);
        g.setColour (owner->textColour);

        g.drawFittedText (owner->getMappings()->getCommandManager()->getNameOfCommand (commandID),
                          4, 0, jmax (40, getChildComponent (0)->getX() - 5), getHeight(),
                          Justification::centredLeft, true);
    }

    void resized()
    {
        int x = getWidth() - 4;

        for (int i = getNumChildComponents(); --i >= 0;)
        {
            KeyMappingChangeButton* kb = dynamic_cast <KeyMappingChangeButton*> (getChildComponent (i));
            kb->fitToContent (getHeight() - 2);
            kb->setTopRightPosition (x, 1);
            x -= kb->getWidth() + 5;
        }
    }

private:
    KeyMappingEditorComponent* const owner;
    const CommandID commandID;

    KeyMappingItemComponent (const KeyMappingItemComponent&);
    const KeyMappingItemComponent& operator= (const KeyMappingItemComponent&);
};

//==============================================================================
class KeyMappingTreeViewItem  : public TreeViewItem
{
public:
    KeyMappingTreeViewItem (KeyMappingEditorComponent* const owner_,
                            const CommandID commandID_)
        : owner (owner_),
          commandID (commandID_)
    {
    }

    ~KeyMappingTreeViewItem()
    {
    }

    const String getUniqueName() const
    {
        return String ((int) commandID) + T("_id");
    }

    bool mightContainSubItems()                 { return false; }
    int getItemHeight() const                   { return 20; }

    Component* createItemComponent()
    {
        return new KeyMappingItemComponent (owner, commandID);
    }

private:
    KeyMappingEditorComponent* const owner;
    const CommandID commandID;

    KeyMappingTreeViewItem (const KeyMappingTreeViewItem&);
    const KeyMappingTreeViewItem& operator= (const KeyMappingTreeViewItem&);
};


//==============================================================================
class KeyCategoryTreeViewItem  : public TreeViewItem
{
public:
    KeyCategoryTreeViewItem (KeyMappingEditorComponent* const owner_,
                             const String& name)
        : owner (owner_),
          categoryName (name)
    {
    }

    ~KeyCategoryTreeViewItem()
    {
    }

    const String getUniqueName() const
    {
        return categoryName + T("_cat");
    }

    bool mightContainSubItems()                 { return true; }
    int getItemHeight() const                   { return 28; }

    void paintItem (Graphics& g, int width, int height)
    {
        g.setFont (height * 0.6f, Font::bold);
        g.setColour (owner->textColour);

        g.drawText (categoryName,
                    2, 0, width - 2, height,
                    Justification::centredLeft, true);
    }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
            if (getNumSubItems() == 0)
            {
                Array <CommandID> commands (owner->getMappings()->getCommandManager()->getCommandsInCategory (categoryName));

                for (int i = 0; i < commands.size(); ++i)
                {
                    if (owner->shouldCommandBeIncluded (commands[i]))
                        addSubItem (new KeyMappingTreeViewItem (owner, commands[i]));
                }
            }
        }
        else
        {
            clearSubItems();
        }
    }

private:
    KeyMappingEditorComponent* owner;
    String categoryName;

    KeyCategoryTreeViewItem (const KeyCategoryTreeViewItem&);
    const KeyCategoryTreeViewItem& operator= (const KeyCategoryTreeViewItem&);
};


//==============================================================================
KeyMappingEditorComponent::KeyMappingEditorComponent (KeyPressMappingSet* const mappingManager,
                                                      const bool showResetToDefaultButton)
    : mappings (mappingManager),
      textColour (Colours::black)
{
    jassert (mappingManager != 0); // can't be null!

    mappingManager->addChangeListener (this);

    setLinesDrawnForSubItems (false);

    resetButton = 0;

    if (showResetToDefaultButton)
    {
        addAndMakeVisible (resetButton = new TextButton (TRANS("reset to defaults")));
        resetButton->addButtonListener (this);
    }

    addAndMakeVisible (tree = new TreeView());
    tree->setColour (TreeView::backgroundColourId, backgroundColour);
    tree->setRootItemVisible (false);
    tree->setDefaultOpenness (true);
    tree->setRootItem (this);
}

KeyMappingEditorComponent::~KeyMappingEditorComponent()
{
    mappings->removeChangeListener (this);
    deleteAllChildren();
}

//==============================================================================
bool KeyMappingEditorComponent::mightContainSubItems()
{
    return true;
}

const String KeyMappingEditorComponent::getUniqueName() const
{
    return T("keys");
}

void KeyMappingEditorComponent::setColours (const Colour& mainBackground,
                                            const Colour& textColour_)
{
    backgroundColour = mainBackground;
    textColour = textColour_;
    tree->setColour (TreeView::backgroundColourId, backgroundColour);
}

void KeyMappingEditorComponent::parentHierarchyChanged()
{
    changeListenerCallback (0);
}

void KeyMappingEditorComponent::resized()
{
    int h = getHeight();

    if (resetButton != 0)
    {
        const int buttonHeight = 20;
        h -= buttonHeight + 8;
        int x = getWidth() - 8;
        const int y = h + 6;

        resetButton->changeWidthToFitText (buttonHeight);
        resetButton->setTopRightPosition (x, y);
    }

    tree->setBounds (0, 0, getWidth(), h);
}

void KeyMappingEditorComponent::buttonClicked (Button* button)
{
    if (button == resetButton)
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                          TRANS("Reset to defaults"),
                                          TRANS("Are you sure you want to reset all the key-mappings to their default state?"),
                                          TRANS("Reset")))
        {
            mappings->resetToDefaultMappings();
        }
    }
}

void KeyMappingEditorComponent::changeListenerCallback (void*)
{
    XmlElement* openness = tree->getOpennessState (true);

    clearSubItems();

    const StringArray categories (mappings->getCommandManager()->getCommandCategories());

    for (int i = 0; i < categories.size(); ++i)
    {
        const Array <CommandID> commands (mappings->getCommandManager()->getCommandsInCategory (categories[i]));
        int count = 0;

        for (int j = 0; j < commands.size(); ++j)
            if (shouldCommandBeIncluded (commands[j]))
                ++count;

        if (count > 0)
            addSubItem (new KeyCategoryTreeViewItem (this, categories[i]));
    }

    if (openness != 0)
    {
        tree->restoreOpennessState (*openness);
        delete openness;
    }
}

//==============================================================================
class KeyEntryWindow  : public AlertWindow
{
    KeyMappingEditorComponent* owner;

    KeyEntryWindow (const KeyEntryWindow&);
    const KeyEntryWindow& operator= (const KeyEntryWindow&);

public:
    KeyPress lastPress;

    KeyEntryWindow (KeyMappingEditorComponent* const owner_)
        : AlertWindow (TRANS("New key-mapping"),
                       TRANS("Please press a key combination now..."),
                       AlertWindow::NoIcon),
          owner (owner_)
    {
        addButton (TRANS("ok"), 1);
        addButton (TRANS("cancel"), 0);

        setWantsKeyboardFocus (true);
        grabKeyboardFocus();
    }

    ~KeyEntryWindow()
    {
    }

    bool keyPressed (const KeyPress& key)
    {
        lastPress = key;
        String message (TRANS("Key: ") + owner->getDescriptionForKeyPress (key));

        const CommandID previousCommand = owner->getMappings()->findCommandForKeyPress (key);

        if (previousCommand != 0)
        {
            message << T("\n\n")
                    << TRANS("(Currently assigned to \"")
                    << owner->getMappings()->getCommandManager()->getNameOfCommand (previousCommand)
                    << T("\")");
        }

        setMessage (message);

        return true;
    }

    bool keyStateChanged()
    {
        return true;
    }
};


void KeyMappingEditorComponent::assignNewKey (const CommandID commandID, int index)
{
    KeyEntryWindow entryWindow (this);

    if (entryWindow.runModalLoop() != 0)
    {
        entryWindow.setVisible (false);

        if (entryWindow.lastPress.isValid())
        {
            const CommandID previousCommand = mappings->findCommandForKeyPress (entryWindow.lastPress);

            if (previousCommand != 0)
            {
                if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                    TRANS("Change key-mapping"),
                                                    TRANS("This key is already assigned to the command \"")
                                                      + mappings->getCommandManager()->getNameOfCommand (previousCommand)
                                                      + TRANS("\"\n\nDo you want to re-assign it to this new command instead?"),
                                                    TRANS("re-assign"),
                                                    TRANS("cancel")))
                {
                    return;
                }
            }

            mappings->removeKeyPress (entryWindow.lastPress);

            if (index >= 0)
                mappings->removeKeyPress (commandID, index);

            mappings->addKeyPress (commandID, entryWindow.lastPress, index);
        }
    }
}


//==============================================================================
bool KeyMappingEditorComponent::shouldCommandBeIncluded (const CommandID commandID)
{
    const ApplicationCommandInfo* const ci = mappings->getCommandManager()->getCommandForID (commandID);

    return (ci != 0) && ((ci->flags & ApplicationCommandInfo::hiddenFromKeyEditor) == 0);
}

bool KeyMappingEditorComponent::isCommandReadOnly (const CommandID commandID)
{
    const ApplicationCommandInfo* const ci = mappings->getCommandManager()->getCommandForID (commandID);

    return (ci != 0) && ((ci->flags & ApplicationCommandInfo::readOnlyInKeyEditor) != 0);
}

const String KeyMappingEditorComponent::getDescriptionForKeyPress (const KeyPress& key)
{
    return key.getTextDescription();
}

END_JUCE_NAMESPACE
