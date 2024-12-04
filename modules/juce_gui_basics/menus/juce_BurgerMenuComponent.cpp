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
struct CustomMenuBarItemHolder final : public Component
{
    CustomMenuBarItemHolder (const ReferenceCountedObjectPtr<PopupMenu::CustomComponent>& customComponent)
    {
        setInterceptsMouseClicks (false, true);
        update (customComponent);
    }

    void update (const ReferenceCountedObjectPtr<PopupMenu::CustomComponent>& newComponent)
    {
        jassert (newComponent != nullptr);

        if (newComponent != custom)
        {
            if (custom != nullptr)
                removeChildComponent (custom.get());

            custom = newComponent;
            addAndMakeVisible (*custom);
            resized();
        }
    }

    void resized() override
    {
        custom->setBounds (getLocalBounds());
    }

    ReferenceCountedObjectPtr<PopupMenu::CustomComponent> custom;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomMenuBarItemHolder)
};

//==============================================================================
BurgerMenuComponent::BurgerMenuComponent (MenuBarModel* modelToUse)
{
    lookAndFeelChanged();
    listBox.addMouseListener (this, true);

    setModel (modelToUse);
    addAndMakeVisible (listBox);
}

BurgerMenuComponent::~BurgerMenuComponent()
{
    if (model != nullptr)
        model->removeListener (this);
}

void BurgerMenuComponent::setModel (MenuBarModel* newModel)
{
    if (newModel != model)
    {
        if (model != nullptr)
            model->removeListener (this);

        model = newModel;

        if (model != nullptr)
            model->addListener (this);

        refresh();
        listBox.updateContent();
    }
}

MenuBarModel* BurgerMenuComponent::getModel() const noexcept
{
    return model;
}

void BurgerMenuComponent::refresh()
{
    lastRowClicked = inputSourceIndexOfLastClick = -1;

    rows.clear();

    if (model != nullptr)
    {
        auto menuBarNames = model->getMenuBarNames();

        for (auto menuIdx = 0; menuIdx < menuBarNames.size(); ++menuIdx)
        {
            PopupMenu::Item menuItem;
            menuItem.text = menuBarNames[menuIdx];

            String ignore;
            auto menu = model->getMenuForIndex (menuIdx, ignore);

            rows.add (Row { true, menuIdx, menuItem });
            addMenuBarItemsForMenu (menu, menuIdx);
        }
    }
}

void BurgerMenuComponent::addMenuBarItemsForMenu (PopupMenu& menu, int menuIdx)
{
    for (PopupMenu::MenuItemIterator it (menu); it.next();)
    {
        auto& item = it.getItem();

        if (item.isSeparator)
            continue;

        if (hasSubMenu (item))
            addMenuBarItemsForMenu (*item.subMenu, menuIdx);
        else
            rows.add (Row {false, menuIdx, it.getItem()});
    }
}

int BurgerMenuComponent::getNumRows()
{
    return rows.size();
}

void BurgerMenuComponent::paint (Graphics& g)
{
    getLookAndFeel().drawPopupMenuBackground (g, getWidth(), getHeight());
}

void BurgerMenuComponent::paintListBoxItem (int rowIndex, Graphics& g, int w, int h, bool highlight)
{
    auto& lf = getLookAndFeel();
    Rectangle<int> r (w, h);

    auto row = (rowIndex < rows.size() ? rows.getReference (rowIndex)
                                       : Row { true, 0, {} });

    g.fillAll (findColour (PopupMenu::backgroundColourId));

    if (row.isMenuHeader)
    {
        lf.drawPopupMenuSectionHeader (g, r.reduced (20, 0), row.item.text);
        g.setColour (Colours::grey);
        g.fillRect (r.withHeight (1));
    }
    else
    {
        auto& item = row.item;
        auto* colour = item.colour != Colour() ? &item.colour : nullptr;

        if (item.customComponent == nullptr)
            lf.drawPopupMenuItem (g, r.reduced (20, 0),
                                  item.isSeparator,
                                  item.isEnabled,
                                  highlight,
                                  item.isTicked,
                                  hasSubMenu (item),
                                  item.text,
                                  item.shortcutKeyDescription,
                                  item.image.get(),
                                  colour);
    }
}

bool BurgerMenuComponent::hasSubMenu (const PopupMenu::Item& item)
{
    return item.subMenu != nullptr && (item.itemID == 0 || item.subMenu->getNumItems() > 0);
}

void BurgerMenuComponent::listBoxItemClicked (int rowIndex, const MouseEvent& e)
{
    auto row = rowIndex < rows.size() ? rows.getReference (rowIndex)
                                      : Row { true, 0, {} };

    if (! row.isMenuHeader)
    {
        lastRowClicked = rowIndex;
        inputSourceIndexOfLastClick = e.source.getIndex();
    }
}

Component* BurgerMenuComponent::refreshComponentForRow (int rowIndex, bool isRowSelected, Component* existing)
{
    auto row = rowIndex < rows.size() ? rows.getReference (rowIndex)
                                      : Row { true, 0, {} };

    auto hasCustomComponent = (row.item.customComponent != nullptr);

    if (existing == nullptr && hasCustomComponent)
        return new CustomMenuBarItemHolder (row.item.customComponent);

    if (existing != nullptr)
    {
        auto* componentToUpdate = dynamic_cast<CustomMenuBarItemHolder*> (existing);
        jassert (componentToUpdate != nullptr);

        if (hasCustomComponent && componentToUpdate != nullptr)
        {
            row.item.customComponent->setHighlighted (isRowSelected);
            componentToUpdate->update (row.item.customComponent);
        }
        else
        {
            delete existing;
            existing = nullptr;
        }
    }

    return existing;
}

void BurgerMenuComponent::resized()
{
    listBox.setBounds (getLocalBounds());
}

void BurgerMenuComponent::menuBarItemsChanged (MenuBarModel* menuBarModel)
{
    setModel (menuBarModel);
}

void BurgerMenuComponent::menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo&)
{
}

void BurgerMenuComponent::mouseUp (const MouseEvent& event)
{
    auto rowIndex = listBox.getSelectedRow();

    if (rowIndex == lastRowClicked && rowIndex < rows.size()
         && event.source.getIndex() == inputSourceIndexOfLastClick)
    {
        auto& row = rows.getReference (rowIndex);

        if (! row.isMenuHeader)
        {
            listBox.selectRow (-1);

            lastRowClicked = -1;
            inputSourceIndexOfLastClick = -1;

            topLevelIndexClicked = row.topLevelMenuIndex;
            auto& item = row.item;

            if (auto* managerOfChosenCommand = item.commandManager)
            {
                ApplicationCommandTarget::InvocationInfo info (item.itemID);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

                managerOfChosenCommand->invoke (info, true);
            }

            postCommandMessage (item.itemID);
        }
    }
}

void BurgerMenuComponent::handleCommandMessage (int commandID)
{
    if (model != nullptr)
    {
        model->menuItemSelected (commandID, topLevelIndexClicked);
        topLevelIndexClicked = -1;

        refresh();
        listBox.updateContent();
    }
}

void BurgerMenuComponent::lookAndFeelChanged()
{
    listBox.setRowHeight (roundToInt (getLookAndFeel().getPopupMenuFont().getHeight() * 2.0f));
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> BurgerMenuComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::menuBar);
}

} // namespace juce
