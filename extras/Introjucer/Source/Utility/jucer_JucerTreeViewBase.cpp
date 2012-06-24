/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "jucer_JucerTreeViewBase.h"
#include "../Project/jucer_ProjectContentComponent.h"


//==============================================================================
JucerTreeViewBase::JucerTreeViewBase()
    : textX (0)
{
    setLinesDrawnForSubItems (false);
}

void JucerTreeViewBase::refreshSubItems()
{
    WholeTreeOpennessRestorer openness (*this);
    clearSubItems();
    addSubItems();
}

Font JucerTreeViewBase::getFont() const
{
    return Font (getItemHeight() * 0.6f);
}

void JucerTreeViewBase::paintItem (Graphics& g, int width, int height)
{
    if (isSelected())
        g.fillAll (Colour (0x401111ee));
}

void JucerTreeViewBase::paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver)
{
    Path p;

    if (isOpen())
        p.addTriangle (width * 0.2f,  height * 0.25f, width * 0.8f, height * 0.25f, width * 0.5f, height * 0.75f);
    else
        p.addTriangle (width * 0.25f, height * 0.25f, width * 0.8f, height * 0.5f,  width * 0.25f, height * 0.75f);

    g.setColour (Colours::lightgrey);
    g.fillPath (p);
}

//==============================================================================
class TreeItemComponent   : public Component
{
public:
    TreeItemComponent (JucerTreeViewBase& item_)
        : item (item_)
    {
        setInterceptsMouseClicks (false, true);

        item.createLeftEdgeComponents (leftComps);

        for (int i = 0; i < leftComps.size(); ++i)
            addAndMakeVisible (leftComps.getUnchecked(i));

        addAndMakeVisible (rightHandComponent = item.createRightEdgeComponent());
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::black);

        const int height = getHeight();

        item.getIcon()->drawWithin (g, Rectangle<float> (0.0f, 2.0f, height + 6.0f, height - 4.0f),
                                    RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, 1.0f);

        g.setFont (item.getFont());
        g.setColour (item.isMissing() ? Colours::red : Colours::black);

        const int right = rightHandComponent != nullptr ? rightHandComponent->getX() - 2
                                                        : getWidth();

        g.drawFittedText (item.getDisplayName(),
                          item.textX, 0, right - item.textX, height, Justification::centredLeft, 1, 0.8f);
    }

    void resized()
    {
        const int edge = 1;
        const int itemSize = getHeight() - edge * 2;
        item.textX = (leftComps.size() + 1) * getHeight() + 8;

        for (int i = 0; i < leftComps.size(); ++i)
            leftComps.getUnchecked(i)->setBounds (5 + (i + 1) * getHeight(), edge, itemSize, itemSize);

        if (rightHandComponent != nullptr)
            rightHandComponent->setBounds (getWidth() - itemSize - edge, edge, itemSize, itemSize);
    }

private:
    JucerTreeViewBase& item;
    OwnedArray<Component> leftComps;
    ScopedPointer<Component> rightHandComponent;
};

Component* JucerTreeViewBase::createItemComponent()
{
    return new TreeItemComponent (*this);
}

//==============================================================================
class RenameTreeItemCallback  : public ModalComponentManager::Callback,
                                public TextEditorListener
{
public:
    RenameTreeItemCallback (JucerTreeViewBase& item_, Component& parent, const Rectangle<int>& bounds)
        : item (item_)
    {
        ed.setMultiLine (false, false);
        ed.setPopupMenuEnabled (false);
        ed.setSelectAllWhenFocused (true);
        ed.setFont (item.getFont());
        ed.addListener (this);
        ed.setText (item.getRenamingName());
        ed.setBounds (bounds);

        parent.addAndMakeVisible (&ed);
        ed.enterModalState (true, this);
    }

    void modalStateFinished (int resultCode)
    {
        if (resultCode != 0)
            item.setName (ed.getText());
    }

    void textEditorTextChanged (TextEditor&)                {}
    void textEditorReturnKeyPressed (TextEditor& editor)    { editor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& editor)    { editor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& editor)           { editor.exitModalState (0); }

private:
    TextEditor ed;
    JucerTreeViewBase& item;

    JUCE_DECLARE_NON_COPYABLE (RenameTreeItemCallback);
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
}

void JucerTreeViewBase::deleteItem()    {}
void JucerTreeViewBase::deleteAllSelectedItems() {}
void JucerTreeViewBase::showDocument()  {}
void JucerTreeViewBase::showPopupMenu() {}
void JucerTreeViewBase::showMultiSelectionPopupMenu() {}

static void treeViewMenuItemChosen (int resultCode, JucerTreeViewBase* item)
{
    item->handlePopupMenuResult (resultCode);
}

void JucerTreeViewBase::launchPopupMenu (PopupMenu& m)
{
    m.showMenuAsync (PopupMenu::Options(),
                     ModalCallbackFunction::create (treeViewMenuItemChosen, this));
}

void JucerTreeViewBase::handlePopupMenuResult (int)
{
}

ProjectContentComponent* JucerTreeViewBase::getProjectContentComponent() const
{
    Component* c = getOwnerView();

    while (c != nullptr)
    {
        ProjectContentComponent* pcc = dynamic_cast <ProjectContentComponent*> (c);

        if (pcc != nullptr)
            return pcc;

        c = c->getParentComponent();
    }

    return 0;
}

//==============================================================================
class JucerTreeViewBase::ItemSelectionTimer  : public Timer
{
public:
    ItemSelectionTimer (JucerTreeViewBase& owner_)  : owner (owner_) {}

    void timerCallback()    { owner.invokeShowDocument(); }

private:
    JucerTreeViewBase& owner;
    JUCE_DECLARE_NON_COPYABLE (ItemSelectionTimer);
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

void JucerTreeViewBase::itemDoubleClicked (const MouseEvent& e)
{
    invokeShowDocument();
}

void JucerTreeViewBase::cancelDelayedSelectionTimer()
{
    delayedSelectionTimer = nullptr;
}
