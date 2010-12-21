/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

// N.B. these two includes are put here deliberately to avoid problems with
// old GCCs failing on long include paths
#include "../../../containers/juce_Array.h"
#include "../../../containers/juce_OwnedArray.h"

#include "juce_KeyMappingEditorComponent.h"
#include "../menus/juce_PopupMenu.h"
#include "../windows/juce_AlertWindow.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../../../text/juce_LocalisedStrings.h"


//==============================================================================
class KeyMappingEditorComponent::ChangeKeyButton  : public Button
{
public:
    ChangeKeyButton (KeyMappingEditorComponent& owner_,
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

        setTooltip (keyNum_ < 0 ? TRANS("adds a new key-mapping")
                                : TRANS("click to change this key-mapping"));
    }

    void paintButton (Graphics& g, bool /*isOver*/, bool /*isDown*/)
    {
        getLookAndFeel().drawKeymapChangeButton (g, getWidth(), getHeight(), *this,
                                                 keyNum >= 0 ? getName() : String::empty);
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

            switch (m.show())
            {
                case 1: assignNewKey(); break;
                case 2: owner.getMappings().removeKeyPress (commandID, keyNum); break;
                default: break;
            }
        }
        else
        {
            assignNewKey();  // + button pressed..
        }
    }

    void fitToContent (const int h) throw()
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

    //==============================================================================
    class KeyEntryWindow  : public AlertWindow
    {
    public:
        KeyEntryWindow (KeyMappingEditorComponent& owner_)
            : AlertWindow (TRANS("New key-mapping"),
                           TRANS("Please press a key combination now..."),
                           AlertWindow::NoIcon),
              owner (owner_)
        {
            addButton (TRANS("Ok"), 1);
            addButton (TRANS("Cancel"), 0);

            // (avoid return + escape keys getting processed by the buttons..)
            for (int i = getNumChildComponents(); --i >= 0;)
                getChildComponent (i)->setWantsKeyboardFocus (false);

            setWantsKeyboardFocus (true);
            grabKeyboardFocus();
        }

        bool keyPressed (const KeyPress& key)
        {
            lastPress = key;
            String message (TRANS("Key: ") + owner.getDescriptionForKeyPress (key));

            const CommandID previousCommand = owner.getMappings().findCommandForKeyPress (key);

            if (previousCommand != 0)
                message << "\n\n" << TRANS("(Currently assigned to \"")
                        << owner.getMappings().getCommandManager()->getNameOfCommand (previousCommand) << "\")";

            setMessage (message);
            return true;
        }

        bool keyStateChanged (bool)
        {
            return true;
        }

        KeyPress lastPress;

    private:
        KeyMappingEditorComponent& owner;

        JUCE_DECLARE_NON_COPYABLE (KeyEntryWindow);
    };

    void assignNewKey()
    {
        KeyEntryWindow entryWindow (owner);

        if (entryWindow.runModalLoop() != 0)
        {
            entryWindow.setVisible (false);

            if (entryWindow.lastPress.isValid())
            {
                const CommandID previousCommand = owner.getMappings().findCommandForKeyPress (entryWindow.lastPress);

                if (previousCommand == 0
                     || AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                      TRANS("Change key-mapping"),
                                                      TRANS("This key is already assigned to the command \"")
                                                        + owner.getMappings().getCommandManager()->getNameOfCommand (previousCommand)
                                                        + TRANS("\"\n\nDo you want to re-assign it to this new command instead?"),
                                                      TRANS("Re-assign"),
                                                      TRANS("Cancel")))
                {
                    owner.getMappings().removeKeyPress (entryWindow.lastPress);

                    if (keyNum >= 0)
                        owner.getMappings().removeKeyPress (commandID, keyNum);

                    owner.getMappings().addKeyPress (commandID, entryWindow.lastPress, keyNum);
                }
            }
        }
    }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;
    const int keyNum;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChangeKeyButton);
};

//==============================================================================
class KeyMappingEditorComponent::ItemComponent  : public Component
{
public:
    ItemComponent (KeyMappingEditorComponent& owner_, const CommandID commandID_)
        : owner (owner_), commandID (commandID_)
    {
        setInterceptsMouseClicks (false, true);

        const bool isReadOnly = owner.isCommandReadOnly (commandID);

        const Array <KeyPress> keyPresses (owner.getMappings().getKeyPressesAssignedToCommand (commandID));

        for (int i = 0; i < jmin ((int) maxNumAssignments, keyPresses.size()); ++i)
            addKeyPressButton (owner.getDescriptionForKeyPress (keyPresses.getReference (i)), i, isReadOnly);

        addKeyPressButton (String::empty, -1, isReadOnly);
    }

    void addKeyPressButton (const String& desc, const int index, const bool isReadOnly)
    {
        ChangeKeyButton* const b = new ChangeKeyButton (owner, commandID, desc, index);
        keyChangeButtons.add (b);

        b->setEnabled (! isReadOnly);
        b->setVisible (keyChangeButtons.size() <= (int) maxNumAssignments);
        addChildComponent (b);
    }

    void paint (Graphics& g)
    {
        g.setFont (getHeight() * 0.7f);
        g.setColour (findColour (KeyMappingEditorComponent::textColourId));

        g.drawFittedText (owner.getMappings().getCommandManager()->getNameOfCommand (commandID),
                          4, 0, jmax (40, getChildComponent (0)->getX() - 5), getHeight(),
                          Justification::centredLeft, true);
    }

    void resized()
    {
        int x = getWidth() - 4;

        for (int i = keyChangeButtons.size(); --i >= 0;)
        {
            ChangeKeyButton* const b = keyChangeButtons.getUnchecked(i);

            b->fitToContent (getHeight() - 2);
            b->setTopRightPosition (x, 1);
            x = b->getX() - 5;
        }
    }

private:
    KeyMappingEditorComponent& owner;
    OwnedArray<ChangeKeyButton> keyChangeButtons;
    const CommandID commandID;

    enum { maxNumAssignments = 3 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent);
};

//==============================================================================
class KeyMappingEditorComponent::MappingItem  : public TreeViewItem
{
public:
    MappingItem (KeyMappingEditorComponent& owner_, const CommandID commandID_)
        : owner (owner_), commandID (commandID_)
    {
    }

    const String getUniqueName() const          { return String ((int) commandID) + "_id"; }
    bool mightContainSubItems()                 { return false; }
    int getItemHeight() const                   { return 20; }

    Component* createItemComponent()
    {
        return new ItemComponent (owner, commandID);
    }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingItem);
};


//==============================================================================
class KeyMappingEditorComponent::CategoryItem  : public TreeViewItem
{
public:
    CategoryItem (KeyMappingEditorComponent& owner_, const String& name)
        : owner (owner_), categoryName (name)
    {
    }

    const String getUniqueName() const          { return categoryName + "_cat"; }
    bool mightContainSubItems()                 { return true; }
    int getItemHeight() const                   { return 28; }

    void paintItem (Graphics& g, int width, int height)
    {
        g.setFont (height * 0.6f, Font::bold);
        g.setColour (owner.findColour (KeyMappingEditorComponent::textColourId));

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
                Array <CommandID> commands (owner.getMappings().getCommandManager()->getCommandsInCategory (categoryName));

                for (int i = 0; i < commands.size(); ++i)
                {
                    if (owner.shouldCommandBeIncluded (commands[i]))
                        addSubItem (new MappingItem (owner, commands[i]));
                }
            }
        }
        else
        {
            clearSubItems();
        }
    }

private:
    KeyMappingEditorComponent& owner;
    String categoryName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CategoryItem);
};

//==============================================================================
class KeyMappingEditorComponent::TopLevelItem   : public TreeViewItem,
                                                  public ChangeListener,
                                                  public ButtonListener
{
public:
    TopLevelItem (KeyMappingEditorComponent& owner_)
        : owner (owner_)
    {
        setLinesDrawnForSubItems (false);
        owner.getMappings().addChangeListener (this);
    }

    ~TopLevelItem()
    {
        owner.getMappings().removeChangeListener (this);
    }

    bool mightContainSubItems()             { return true; }
    const String getUniqueName() const      { return "keys"; }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        const ScopedPointer <XmlElement> oldOpenness (owner.tree.getOpennessState (true));

        clearSubItems();

        const StringArray categories (owner.getMappings().getCommandManager()->getCommandCategories());

        for (int i = 0; i < categories.size(); ++i)
        {
            const Array <CommandID> commands (owner.getMappings().getCommandManager()->getCommandsInCategory (categories[i]));
            int count = 0;

            for (int j = 0; j < commands.size(); ++j)
                if (owner.shouldCommandBeIncluded (commands[j]))
                    ++count;

            if (count > 0)
                addSubItem (new CategoryItem (owner, categories[i]));
        }

        if (oldOpenness != 0)
            owner.tree.restoreOpennessState (*oldOpenness);
    }

    void buttonClicked (Button*)
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                          TRANS("Reset to defaults"),
                                          TRANS("Are you sure you want to reset all the key-mappings to their default state?"),
                                          TRANS("Reset")))
        {
            owner.getMappings().resetToDefaultMappings();
        }
    }

private:
    KeyMappingEditorComponent& owner;
};


//==============================================================================
KeyMappingEditorComponent::KeyMappingEditorComponent (KeyPressMappingSet& mappingManager,
                                                      const bool showResetToDefaultButton)
    : mappings (mappingManager),
      resetButton (TRANS ("reset to defaults"))
{
    treeItem = new TopLevelItem (*this);

    if (showResetToDefaultButton)
    {
        addAndMakeVisible (&resetButton);
        resetButton.addListener (treeItem);
    }

    addAndMakeVisible (&tree);
    tree.setColour (TreeView::backgroundColourId, findColour (backgroundColourId));
    tree.setRootItemVisible (false);
    tree.setDefaultOpenness (true);
    tree.setRootItem (treeItem);
}

KeyMappingEditorComponent::~KeyMappingEditorComponent()
{
    tree.setRootItem (0);
}

//==============================================================================
void KeyMappingEditorComponent::setColours (const Colour& mainBackground,
                                            const Colour& textColour)
{
    setColour (backgroundColourId, mainBackground);
    setColour (textColourId, textColour);
    tree.setColour (TreeView::backgroundColourId, mainBackground);
}

void KeyMappingEditorComponent::parentHierarchyChanged()
{
    treeItem->changeListenerCallback (0);
}

void KeyMappingEditorComponent::resized()
{
    int h = getHeight();

    if (resetButton.isVisible())
    {
        const int buttonHeight = 20;
        h -= buttonHeight + 8;
        int x = getWidth() - 8;

        resetButton.changeWidthToFitText (buttonHeight);
        resetButton.setTopRightPosition (x, h + 6);
    }

    tree.setBounds (0, 0, getWidth(), h);
}

//==============================================================================
bool KeyMappingEditorComponent::shouldCommandBeIncluded (const CommandID commandID)
{
    const ApplicationCommandInfo* const ci = mappings.getCommandManager()->getCommandForID (commandID);

    return ci != 0 && (ci->flags & ApplicationCommandInfo::hiddenFromKeyEditor) == 0;
}

bool KeyMappingEditorComponent::isCommandReadOnly (const CommandID commandID)
{
    const ApplicationCommandInfo* const ci = mappings.getCommandManager()->getCommandForID (commandID);

    return ci != 0 && (ci->flags & ApplicationCommandInfo::readOnlyInKeyEditor) != 0;
}

const String KeyMappingEditorComponent::getDescriptionForKeyPress (const KeyPress& key)
{
    return key.getTextDescription();
}

END_JUCE_NAMESPACE
