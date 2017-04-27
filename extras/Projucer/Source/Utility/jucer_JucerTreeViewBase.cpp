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

#include "../jucer_Headers.h"
#include "jucer_JucerTreeViewBase.h"
#include "../Project/jucer_ProjectContentComponent.h"

//==============================================================================
void TreePanelBase::setRoot (JucerTreeViewBase* root)
{
    rootItem = root;
    tree.setRootItem (root);
    tree.getRootItem()->setOpen (true);

    if (project != nullptr)
    {
        const ScopedPointer<XmlElement> treeOpenness (project->getStoredProperties()
                                                          .getXmlValue (opennessStateKey));
        if (treeOpenness != nullptr)
        {
            tree.restoreOpennessState (*treeOpenness, true);

            for (int i = tree.getNumSelectedItems(); --i >= 0;)
                if (JucerTreeViewBase* item = dynamic_cast<JucerTreeViewBase*> (tree.getSelectedItem (i)))
                    item->cancelDelayedSelectionTimer();
        }
    }
}

void TreePanelBase::saveOpenness()
{
    if (project != nullptr)
    {
        const ScopedPointer<XmlElement> opennessState (tree.getOpennessState (true));

        if (opennessState != nullptr)
            project->getStoredProperties().setValue (opennessStateKey, opennessState);
        else
            project->getStoredProperties().removeValue (opennessStateKey);
    }
}

//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()  : textX (0)
{
    setLinesDrawnForSubItems (false);
    setDrawsInLeftMargin (true);
}

JucerTreeViewBase::~JucerTreeViewBase()
{
    masterReference.clear();
}

void JucerTreeViewBase::refreshSubItems()
{
    WholeTreeOpennessRestorer wtor (*this);
    clearSubItems();
    addSubItems();
}

Font JucerTreeViewBase::getFont() const
{
    return Font (getItemHeight() * 0.6f);
}

void JucerTreeViewBase::paintOpenCloseButton (Graphics& g, const Rectangle<float>& area, Colour /*backgroundColour*/, bool isMouseOver)
{
    g.setColour (getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId : treeIconColourId));
    TreeViewItem::paintOpenCloseButton (g, area, getOwnerView()->findColour (defaultIconColourId), isMouseOver);
}

void JucerTreeViewBase::paintIcon (Graphics &g, Rectangle<float> area)
{
    g.setColour (getContentColour (true));
    getIcon().draw (g, area, isIconCrossedOut());
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
    if (isMissing())
        return Colours::red;

    if (isSelected())
        return getOwnerView()->findColour (defaultHighlightedTextColourId);

    return getOwnerView()->findColour (isIcon ? treeIconColourId : defaultTextColourId);
}

void JucerTreeViewBase::paintContent (Graphics& g, const Rectangle<int>& area)
{
    g.setFont (getFont());
    g.setColour (getContentColour (false));

    g.drawFittedText (getDisplayName(), area, Justification::centredLeft, 1, 1.0f);
}

Component* JucerTreeViewBase::createItemComponent()
{
    return new TreeItemComponent (*this);
}

//==============================================================================
class RenameTreeItemCallback  : public ModalComponentManager::Callback,
                                public TextEditorListener
{
public:
    RenameTreeItemCallback (JucerTreeViewBase& ti, Component& parent, const Rectangle<int>& bounds)
        : item (ti)
    {
        ed.setMultiLine (false, false);
        ed.setPopupMenuEnabled (false);
        ed.setSelectAllWhenFocused (true);
        ed.setFont (item.getFont());
        ed.addListener (this);
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

    void textEditorTextChanged (TextEditor&) override               {}
    void textEditorReturnKeyPressed (TextEditor& editor) override    { editor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& editor) override    { editor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& editor) override           { editor.exitModalState (0); }

private:
    struct RenameEditor   : public TextEditor
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
            showMultiSelectionPopupMenu();
        else
            showPopupMenu();
    }
    else if (isSelected())
    {
        itemSelectionChanged (true);
    }
}

void JucerTreeViewBase::deleteItem()    {}
void JucerTreeViewBase::deleteAllSelectedItems() {}
void JucerTreeViewBase::showDocument()  {}
void JucerTreeViewBase::showPopupMenu() {}
void JucerTreeViewBase::showPlusMenu()  {}
void JucerTreeViewBase::showMultiSelectionPopupMenu() {}

static void treeViewMenuItemChosen (int resultCode, WeakReference<JucerTreeViewBase> item)
{
    if (item != nullptr)
        item->handlePopupMenuResult (resultCode);
}

void JucerTreeViewBase::launchPopupMenu (PopupMenu& m)
{
    m.showMenuAsync (PopupMenu::Options(),
                     ModalCallbackFunction::create (treeViewMenuItemChosen, WeakReference<JucerTreeViewBase> (this)));
}

void JucerTreeViewBase::handlePopupMenuResult (int)
{
}

ProjectContentComponent* JucerTreeViewBase::getProjectContentComponent() const
{
    for (Component* c = getOwnerView(); c != nullptr; c = c->getParentComponent())
        if (ProjectContentComponent* pcc = dynamic_cast<ProjectContentComponent*> (c))
            return pcc;

    return nullptr;
}

//==============================================================================
class JucerTreeViewBase::ItemSelectionTimer  : public Timer
{
public:
    ItemSelectionTimer (JucerTreeViewBase& tvb)  : owner (tvb) {}

    void timerCallback() override    { owner.invokeShowDocument(); }

private:
    JucerTreeViewBase& owner;
    JUCE_DECLARE_NON_COPYABLE (ItemSelectionTimer)
};

void JucerTreeViewBase::itemSelectionChanged (bool isNowSelected)
{
    if (isNowSelected)
    {
        delayedSelectionTimer = new ItemSelectionTimer (*this);
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
    delayedSelectionTimer = nullptr;
}
