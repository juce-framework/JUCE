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

#include "../../Application/jucer_Headers.h"
#include "jucer_JucerTreeViewBase.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
void TreePanelBase::setRoot (std::unique_ptr<JucerTreeViewBase> root)
{
    rootItem = std::move (root);
    tree.setRootItem (rootItem.get());
    tree.getRootItem()->setOpen (true);

    if (project != nullptr)
    {
        if (auto treeOpenness = project->getStoredProperties().getXmlValue (opennessStateKey))
        {
            tree.restoreOpennessState (*treeOpenness, true);

            for (int i = tree.getNumSelectedItems(); --i >= 0;)
                if (auto item = dynamic_cast<JucerTreeViewBase*> (tree.getSelectedItem (i)))
                    item->cancelDelayedSelectionTimer();
        }
    }
}

void TreePanelBase::saveOpenness()
{
    if (project != nullptr)
    {
        std::unique_ptr<XmlElement> opennessState (tree.getOpennessState (true));

        if (opennessState != nullptr)
            project->getStoredProperties().setValue (opennessStateKey, opennessState.get());
        else
            project->getStoredProperties().removeValue (opennessStateKey);
    }
}

//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()
{
    setLinesDrawnForSubItems (false);
    setDrawsInLeftMargin (true);
}

void JucerTreeViewBase::refreshSubItems()
{
    WholeTreeOpennessRestorer wtor (*this);
    clearSubItems();
    addSubItems();
}

Font JucerTreeViewBase::getFont() const
{
    return FontOptions ((float) getItemHeight() * 0.6f);
}

void JucerTreeViewBase::paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour /*backgroundColour*/, bool isMouseOver)
{
    g.setColour (getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId : treeIconColourId));
    TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColour (defaultIconColourId), isMouseOver);
}

void JucerTreeViewBase::paintIcon (Graphics& g, Rectangle<float> area)
{
    g.setColour (getContentColour (true));
    getIcon().draw (g, area, isIconCrossedOut());
    textX = roundToInt (area.getRight()) + 7;
}

void JucerTreeViewBase::paintItem (Graphics& g, int width, int height)
{
    ignoreUnused (width, height);

    auto bounds = g.getClipBounds().withY (0).withHeight (height).toFloat();

    g.setColour (getOwnerView()->findColour (treeIconColourId).withMultipliedAlpha (0.4f));
    g.fillRect (bounds.removeFromBottom (0.5f).reduced (5, 0));
}

Colour JucerTreeViewBase::getContentColour (bool isIcon) const
{
    if (isMissing())      return Colours::red;
    if (isSelected())     return getOwnerView()->findColour (defaultHighlightedTextColourId);
    if (hasWarnings())    return getOwnerView()->findColour (defaultHighlightColourId);

    return getOwnerView()->findColour (isIcon ? treeIconColourId : defaultTextColourId);
}

void JucerTreeViewBase::paintContent (Graphics& g, Rectangle<int> area)
{
    g.setFont (getFont());
    g.setColour (getContentColour (false));

    g.drawFittedText (getDisplayName(), area, Justification::centredLeft, 1, 1.0f);
}

std::unique_ptr<Component> JucerTreeViewBase::createItemComponent()
{
    return std::make_unique<TreeItemComponent> (*this);
}

//==============================================================================
class RenameTreeItemCallback final : public ModalComponentManager::Callback
{
public:
    RenameTreeItemCallback (JucerTreeViewBase& ti, Component& parent, const Rectangle<int>& bounds)
        : item (ti)
    {
        ed.setMultiLine (false, false);
        ed.setPopupMenuEnabled (false);
        ed.setSelectAllWhenFocused (true);
        ed.setFont (item.getFont());
        ed.onReturnKey = [this] { ed.exitModalState (1); };
        ed.onEscapeKey = [this] { ed.exitModalState (0); };
        ed.onFocusLost = [this] { ed.exitModalState (0); };
        ed.setText (item.getRenamingName());
        ed.setBounds (bounds);

        parent.addAndMakeVisible (ed);
        ed.enterModalState (true, this);
    }

    void modalStateFinished (int resultCode) override
    {
        if (resultCode != 0)
            item.setName (ed.getText());
    }

private:
    struct RenameEditor final : public TextEditor
    {
        void inputAttemptWhenModal() override   { exitModalState (0); }
    };

    RenameEditor ed;
    JucerTreeViewBase& item;

    JUCE_DECLARE_NON_COPYABLE (RenameTreeItemCallback)
};

void JucerTreeViewBase::showRenameBox()
{
    Rectangle<int> r (getItemPosition (true));
    r.setLeft (r.getX() + textX);
    r.setHeight (getItemHeight());

    new RenameTreeItemCallback (*this, *getOwnerView(), r);
}

void JucerTreeViewBase::itemClicked (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (getOwnerView()->getNumSelectedItems() > 1)
            showMultiSelectionPopupMenu (e.getMouseDownScreenPosition());
        else
            showPopupMenu (e.getMouseDownScreenPosition());
    }
    else if (isSelected())
    {
        itemSelectionChanged (true);
    }
}

static void treeViewMenuItemChosen (int resultCode, WeakReference<JucerTreeViewBase> item)
{
    if (item != nullptr)
        item->handlePopupMenuResult (resultCode);
}

void JucerTreeViewBase::launchPopupMenu (PopupMenu& m, Point<int> p)
{
    m.showMenuAsync (PopupMenu::Options().withTargetScreenArea ({ p.x, p.y, 1, 1 }),
                     ModalCallbackFunction::create (treeViewMenuItemChosen, WeakReference<JucerTreeViewBase> (this)));
}

ProjectContentComponent* JucerTreeViewBase::getProjectContentComponent() const
{
    for (Component* c = getOwnerView(); c != nullptr; c = c->getParentComponent())
        if (ProjectContentComponent* pcc = dynamic_cast<ProjectContentComponent*> (c))
            return pcc;

    return nullptr;
}

//==============================================================================
class JucerTreeViewBase::ItemSelectionTimer final : public Timer
{
public:
    explicit ItemSelectionTimer (JucerTreeViewBase& tvb)  : owner (tvb) {}

    void timerCallback() override    { owner.invokeShowDocument(); }

private:
    JucerTreeViewBase& owner;
    JUCE_DECLARE_NON_COPYABLE (ItemSelectionTimer)
};

void JucerTreeViewBase::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        delayedSelectionTimer.reset (new ItemSelectionTimer (*this));
        delayedSelectionTimer->startTimer (getMillisecsAllowedForDragGesture());
    }
    else
    {
        cancelDelayedSelectionTimer();
    }
}

void JucerTreeViewBase::invokeShowDocument()
{
    cancelDelayedSelectionTimer();
    showDocument();
}

void JucerTreeViewBase::itemDoubleClicked (const MouseEvent&)
{
    invokeShowDocument();
}

void JucerTreeViewBase::cancelDelayedSelectionTimer()
{
    delayedSelectionTimer.reset();
}
