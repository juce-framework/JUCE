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

class KeyMappingEditorComponent::ChangeKeyButton final : public Button
{
public:
    ChangeKeyButton (KeyMappingEditorComponent& kec, CommandID command,
                     const String& keyName, int keyIndex)
        : Button (keyName),
          owner (kec),
          commandID (command),
          keyNum (keyIndex)
    {
        setWantsKeyboardFocus (false);
        setTriggeredOnMouseDown (keyNum >= 0);

        setTooltip (keyIndex < 0 ? TRANS ("Adds a new key-mapping")
                                 : TRANS ("Click to change this key-mapping"));
    }

    void paintButton (Graphics& g, bool /*isOver*/, bool /*isDown*/) override
    {
        getLookAndFeel().drawKeymapChangeButton (g, getWidth(), getHeight(), *this,
                                                 keyNum >= 0 ? getName() : String());
    }

    void clicked() override
    {
        if (keyNum >= 0)
        {
            Component::SafePointer<ChangeKeyButton> button (this);
            PopupMenu m;

            m.addItem (TRANS ("Change this key-mapping"),
                       [button]
                       {
                           if (button != nullptr)
                               button.getComponent()->assignNewKey();
                       });

            m.addSeparator();

            m.addItem (TRANS ("Remove this key-mapping"),
                       [button]
                       {
                           if (button != nullptr)
                               button->owner.getMappings().removeKeyPress (button->commandID,
                                                                           button->keyNum);
                       });

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this));
        }
        else
        {
            assignNewKey();  // + button pressed..
        }
    }

    using Button::clicked;

    void fitToContent (const int h) noexcept
    {
        if (keyNum < 0)
        {
            setSize (h, h);
        }
        else
        {
            const auto idealWidth = GlyphArrangement::getStringWidthInt (withDefaultMetrics (FontOptions { (float) h * 0.6f }), getName());
            setSize (jlimit (h * 4, h * 8, 6 + idealWidth), h);
        }
    }

    //==============================================================================
    class KeyEntryWindow final : public AlertWindow
    {
    public:
        KeyEntryWindow (KeyMappingEditorComponent& kec)
            : AlertWindow (TRANS ("New key-mapping"),
                           TRANS ("Please press a key combination now..."),
                           MessageBoxIconType::NoIcon),
              owner (kec)
        {
            addButton (TRANS ("OK"), 1);
            addButton (TRANS ("Cancel"), 0);

            // (avoid return + escape keys getting processed by the buttons..)
            for (auto* child : getChildren())
                child->setWantsKeyboardFocus (false);

            setWantsKeyboardFocus (true);
            grabKeyboardFocus();
        }

        bool keyPressed (const KeyPress& key) override
        {
            lastPress = key;
            String message (TRANS ("Key") + ": " + owner.getDescriptionForKeyPress (key));

            auto previousCommand = owner.getMappings().findCommandForKeyPress (key);

            if (previousCommand != 0)
                message << "\n\n("
                        << TRANS ("Currently assigned to \"CMDN\"")
                            .replace ("CMDN", TRANS (owner.getCommandManager().getNameOfCommand (previousCommand)))
                        << ')';

            setMessage (message);
            return true;
        }

        bool keyStateChanged (bool) override
        {
            return true;
        }

        KeyPress lastPress;

    private:
        KeyMappingEditorComponent& owner;

        JUCE_DECLARE_NON_COPYABLE (KeyEntryWindow)
    };

    void setNewKey (const KeyPress& newKey, bool dontAskUser)
    {
        if (newKey.isValid())
        {
            auto previousCommand = owner.getMappings().findCommandForKeyPress (newKey);

            if (previousCommand == 0 || dontAskUser)
            {
                owner.getMappings().removeKeyPress (newKey);

                if (keyNum >= 0)
                    owner.getMappings().removeKeyPress (commandID, keyNum);

                owner.getMappings().addKeyPress (commandID, newKey, keyNum);
            }
            else
            {
                auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::WarningIcon,
                                                                       TRANS ("Change key-mapping"),
                                                                       TRANS ("This key is already assigned to the command \"CMDN\"")
                                                                           .replace ("CMDN", owner.getCommandManager().getNameOfCommand (previousCommand))
                                                                         + "\n\n"
                                                                         + TRANS ("Do you want to re-assign it to this new command instead?"),
                                                                       TRANS ("Re-assign"),
                                                                       TRANS ("Cancel"),
                                                                       this);
                messageBox = AlertWindow::showScopedAsync (options, [this, newKey] (int result)
                {
                    if (result != 0)
                        setNewKey (newKey, true);
                });
            }
        }
    }

    static void keyChosen (int result, ChangeKeyButton* button)
    {
        if (button != nullptr && button->currentKeyEntryWindow != nullptr)
        {
            if (result != 0)
            {
                button->currentKeyEntryWindow->setVisible (false);
                button->setNewKey (button->currentKeyEntryWindow->lastPress, false);
            }

            button->currentKeyEntryWindow.reset();
        }
    }

    void assignNewKey()
    {
        currentKeyEntryWindow.reset (new KeyEntryWindow (owner));
        currentKeyEntryWindow->enterModalState (true, ModalCallbackFunction::forComponent (keyChosen, this));
    }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;
    const int keyNum;
    std::unique_ptr<KeyEntryWindow> currentKeyEntryWindow;
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChangeKeyButton)
};

//==============================================================================
class KeyMappingEditorComponent::ItemComponent final : public Component
{
public:
    ItemComponent (KeyMappingEditorComponent& kec, CommandID command)
        : owner (kec), commandID (command)
    {
        setInterceptsMouseClicks (false, true);

        const bool isReadOnly = owner.isCommandReadOnly (commandID);

        auto keyPresses = owner.getMappings().getKeyPressesAssignedToCommand (commandID);

        for (int i = 0; i < jmin ((int) maxNumAssignments, keyPresses.size()); ++i)
            addKeyPressButton (owner.getDescriptionForKeyPress (keyPresses.getReference (i)), i, isReadOnly);

        addKeyPressButton ("Change Key Mapping", -1, isReadOnly);
    }

    void addKeyPressButton (const String& desc, const int index, const bool isReadOnly)
    {
        auto* b = new ChangeKeyButton (owner, commandID, desc, index);
        keyChangeButtons.add (b);

        b->setEnabled (! isReadOnly);
        b->setVisible (keyChangeButtons.size() <= (int) maxNumAssignments);
        addChildComponent (b);
    }

    void paint (Graphics& g) override
    {
        g.setFont ((float) getHeight() * 0.7f);
        g.setColour (owner.findColour (KeyMappingEditorComponent::textColourId));

        g.drawFittedText (TRANS (owner.getCommandManager().getNameOfCommand (commandID)),
                          4, 0, jmax (40, getChildComponent (0)->getX() - 5), getHeight(),
                          Justification::centredLeft, true);
    }

    void resized() override
    {
        int x = getWidth() - 4;

        for (int i = keyChangeButtons.size(); --i >= 0;)
        {
            auto* b = keyChangeButtons.getUnchecked (i);

            b->fitToContent (getHeight() - 2);
            b->setTopRightPosition (x, 1);
            x = b->getX() - 5;
        }
    }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    KeyMappingEditorComponent& owner;
    OwnedArray<ChangeKeyButton> keyChangeButtons;
    const CommandID commandID;

    enum { maxNumAssignments = 3 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
class KeyMappingEditorComponent::MappingItem final : public TreeViewItem
{
public:
    MappingItem (KeyMappingEditorComponent& kec, CommandID command)
        : owner (kec), commandID (command)
    {}

    String getUniqueName() const override                      { return String ((int) commandID) + "_id"; }
    bool mightContainSubItems() override                       { return false; }
    int getItemHeight() const override                         { return 20; }
    std::unique_ptr<Component> createItemComponent() override  { return std::make_unique<ItemComponent> (owner, commandID); }
    String getAccessibilityName() override                     { return TRANS (owner.getCommandManager().getNameOfCommand (commandID)); }

private:
    KeyMappingEditorComponent& owner;
    const CommandID commandID;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MappingItem)
};


//==============================================================================
class KeyMappingEditorComponent::CategoryItem final : public TreeViewItem
{
public:
    CategoryItem (KeyMappingEditorComponent& kec, const String& name)
        : owner (kec), categoryName (name)
    {}

    String getUniqueName() const override       { return categoryName + "_cat"; }
    bool mightContainSubItems() override        { return true; }
    int getItemHeight() const override          { return 22; }
    String getAccessibilityName() override      { return categoryName; }

    void paintItem (Graphics& g, int width, int height) override
    {
        g.setFont (owner.withDefaultMetrics (FontOptions ((float) height * 0.7f, Font::bold)));
        g.setColour (owner.findColour (KeyMappingEditorComponent::textColourId));

        g.drawText (TRANS (categoryName), 2, 0, width - 2, height, Justification::centredLeft, true);
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
        {
            if (getNumSubItems() == 0)
                for (auto command : owner.getCommandManager().getCommandsInCategory (categoryName))
                    if (owner.shouldCommandBeIncluded (command))
                        addSubItem (new MappingItem (owner, command));
        }
        else
        {
            clearSubItems();
        }
    }

private:
    KeyMappingEditorComponent& owner;
    String categoryName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CategoryItem)
};

//==============================================================================
class KeyMappingEditorComponent::TopLevelItem final : public TreeViewItem,
                                                      private ChangeListener
{
public:
    TopLevelItem (KeyMappingEditorComponent& kec)   : owner (kec)
    {
        setLinesDrawnForSubItems (false);
        owner.getMappings().addChangeListener (this);
    }

    ~TopLevelItem() override
    {
        owner.getMappings().removeChangeListener (this);
    }

    bool mightContainSubItems() override             { return true; }
    String getUniqueName() const override            { return "keys"; }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        const OpennessRestorer opennessRestorer (*this);
        clearSubItems();

        for (auto category : owner.getCommandManager().getCommandCategories())
        {
            int count = 0;

            for (auto command : owner.getCommandManager().getCommandsInCategory (category))
                if (owner.shouldCommandBeIncluded (command))
                    ++count;

            if (count > 0)
                addSubItem (new CategoryItem (owner, category));
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
    treeItem.reset (new TopLevelItem (*this));

    if (showResetToDefaultButton)
    {
        addAndMakeVisible (resetButton);

        resetButton.onClick = [this]
        {
            auto options = MessageBoxOptions::makeOptionsOkCancel (MessageBoxIconType::QuestionIcon,
                                                                   TRANS ("Reset to defaults"),
                                                                   TRANS ("Are you sure you want to reset all the key-mappings to their default state?"),
                                                                   TRANS ("Reset"),
                                                                   {},
                                                                   this);
            messageBox = AlertWindow::showScopedAsync (options, [this] (int result)
            {
                if (result != 0)
                    getMappings().resetToDefaultMappings();
            });
        };
    }

    addAndMakeVisible (tree);
    tree.setTitle ("Key Mappings");
    tree.setColour (TreeView::backgroundColourId, findColour (backgroundColourId));
    tree.setRootItemVisible (false);
    tree.setDefaultOpenness (true);
    tree.setRootItem (treeItem.get());
    tree.setIndentSize (12);
}

KeyMappingEditorComponent::~KeyMappingEditorComponent()
{
    tree.setRootItem (nullptr);
}

//==============================================================================
void KeyMappingEditorComponent::setColours (Colour mainBackground,
                                            Colour textColour)
{
    setColour (backgroundColourId, mainBackground);
    setColour (textColourId, textColour);
    tree.setColour (TreeView::backgroundColourId, mainBackground);
}

void KeyMappingEditorComponent::parentHierarchyChanged()
{
    treeItem->changeListenerCallback (nullptr);
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
    auto* ci = mappings.getCommandManager().getCommandForID (commandID);

    return ci != nullptr && (ci->flags & ApplicationCommandInfo::hiddenFromKeyEditor) == 0;
}

bool KeyMappingEditorComponent::isCommandReadOnly (const CommandID commandID)
{
    auto* ci = mappings.getCommandManager().getCommandForID (commandID);

    return ci != nullptr && (ci->flags & ApplicationCommandInfo::readOnlyInKeyEditor) != 0;
}

String KeyMappingEditorComponent::getDescriptionForKeyPress (const KeyPress& key)
{
    return key.getTextDescription();
}

} // namespace juce
