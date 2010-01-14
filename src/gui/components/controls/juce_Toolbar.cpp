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

#include "juce_Toolbar.h"
#include "juce_ToolbarItemComponent.h"
#include "juce_ToolbarItemFactory.h"
#include "juce_ToolbarItemPalette.h"
#include "../menus/juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../layout/juce_StretchableObjectResizer.h"
#include "../windows/juce_DialogWindow.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "juce_ComboBox.h"
#include "../buttons/juce_TextButton.h"

const tchar* const Toolbar::toolbarDragDescriptor = T("_toolbarItem_");


//==============================================================================
class ToolbarSpacerComp  : public ToolbarItemComponent
{
public:
    ToolbarSpacerComp (const int itemId_, const float fixedSize_, const bool drawBar_)
        : ToolbarItemComponent (itemId_, String::empty, false),
          fixedSize (fixedSize_),
          drawBar (drawBar_)
    {
    }

    ~ToolbarSpacerComp()
    {
    }

    bool getToolbarItemSizes (int toolbarThickness, bool /*isToolbarVertical*/,
                              int& preferredSize, int& minSize, int& maxSize)
    {
        if (fixedSize <= 0)
        {
            preferredSize = toolbarThickness * 2;
            minSize = 4;
            maxSize = 32768;
        }
        else
        {
            maxSize = roundToInt (toolbarThickness * fixedSize);
            minSize = drawBar ? maxSize : jmin (4, maxSize);
            preferredSize = maxSize;

            if (getEditingMode() == editableOnPalette)
                preferredSize = maxSize = toolbarThickness / (drawBar ? 3 : 2);
        }

        return true;
    }

    void paintButtonArea (Graphics&, int, int, bool, bool)
    {
    }

    void contentAreaChanged (const Rectangle&)
    {
    }

    int getResizeOrder() const throw()
    {
        return fixedSize <= 0 ? 0 : 1;
    }

    void paint (Graphics& g)
    {
        const int w = getWidth();
        const int h = getHeight();

        if (drawBar)
        {
            g.setColour (findColour (Toolbar::separatorColourId, true));

            const float thickness = 0.2f;

            if (isToolbarVertical())
                g.fillRect (w * 0.1f, h * (0.5f - thickness * 0.5f), w * 0.8f, h * thickness);
            else
                g.fillRect (w * (0.5f - thickness * 0.5f), h * 0.1f, w * thickness, h * 0.8f);
        }

        if (getEditingMode() != normalMode && ! drawBar)
        {
            g.setColour (findColour (Toolbar::separatorColourId, true));

            const int indentX = jmin (2, (w - 3) / 2);
            const int indentY = jmin (2, (h - 3) / 2);
            g.drawRect (indentX, indentY, w - indentX * 2, h - indentY * 2, 1);

            if (fixedSize <= 0)
            {
                float x1, y1, x2, y2, x3, y3, x4, y4, hw, hl;

                if (isToolbarVertical())
                {
                    x1 = w * 0.5f;
                    y1 = h * 0.4f;
                    x2 = x1;
                    y2 = indentX * 2.0f;

                    x3 = x1;
                    y3 = h * 0.6f;
                    x4 = x1;
                    y4 = h - y2;

                    hw = w * 0.15f;
                    hl = w * 0.2f;
                }
                else
                {
                    x1 = w * 0.4f;
                    y1 = h * 0.5f;
                    x2 = indentX * 2.0f;
                    y2 = y1;

                    x3 = w * 0.6f;
                    y3 = y1;
                    x4 = w - x2;
                    y4 = y1;

                    hw = h * 0.15f;
                    hl = h * 0.2f;
                }

                Path p;
                p.addArrow (x1, y1, x2, y2, 1.5f, hw, hl);
                p.addArrow (x3, y3, x4, y4, 1.5f, hw, hl);
                g.fillPath (p);
            }
        }
    }

    juce_UseDebuggingNewOperator

private:
    const float fixedSize;
    const bool drawBar;

    ToolbarSpacerComp (const ToolbarSpacerComp&);
    const ToolbarSpacerComp& operator= (const ToolbarSpacerComp&);
};

//==============================================================================
class MissingItemsComponent  : public PopupMenuCustomComponent
{
public:
    MissingItemsComponent (Toolbar& owner_, const int height_)
        : PopupMenuCustomComponent (true),
          owner (owner_),
          height (height_)
    {
        for (int i = owner_.items.size(); --i >= 0;)
        {
            ToolbarItemComponent* const tc = owner_.items.getUnchecked(i);

            if (dynamic_cast <ToolbarSpacerComp*> (tc) == 0 && ! tc->isVisible())
            {
                oldIndexes.insert (0, i);
                addAndMakeVisible (tc, 0);
            }
        }

        layout (400);
    }

    ~MissingItemsComponent()
    {
        // deleting the toolbar while its menu it open??
        jassert (owner.isValidComponent());

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getChildComponent (i));

            if (tc != 0)
            {
                tc->setVisible (false);
                const int index = oldIndexes.remove (i);
                owner.addChildComponent (tc, index);
                --i;
            }
        }

        owner.resized();
    }

    void layout (const int preferredWidth)
    {
        const int indent = 8;
        int x = indent;
        int y = indent;
        int maxX = 0;

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (getChildComponent (i));

            if (tc != 0)
            {
                int preferredSize = 1, minSize = 1, maxSize = 1;

                if (tc->getToolbarItemSizes (height, false, preferredSize, minSize, maxSize))
                {
                    if (x + preferredSize > preferredWidth && x > indent)
                    {
                        x = indent;
                        y += height;
                    }

                    tc->setBounds (x, y, preferredSize, height);

                    x += preferredSize;
                    maxX = jmax (maxX, x);
                }
            }
        }

        setSize (maxX + 8, y + height + 8);
    }

    void getIdealSize (int& idealWidth, int& idealHeight)
    {
        idealWidth = getWidth();
        idealHeight = getHeight();
    }

    juce_UseDebuggingNewOperator

private:
    Toolbar& owner;
    const int height;
    Array <int> oldIndexes;

    MissingItemsComponent (const MissingItemsComponent&);
    const MissingItemsComponent& operator= (const MissingItemsComponent&);
};


//==============================================================================
Toolbar::Toolbar()
    : vertical (false),
      isEditingActive (false),
      toolbarStyle (Toolbar::iconsOnly)
{
    addChildComponent (missingItemsButton = getLookAndFeel().createToolbarMissingItemsButton (*this));

    missingItemsButton->setAlwaysOnTop (true);
    missingItemsButton->addButtonListener (this);
}

Toolbar::~Toolbar()
{
    animator.cancelAllAnimations (true);
    deleteAllChildren();
}

void Toolbar::setVertical (const bool shouldBeVertical)
{
    if (vertical != shouldBeVertical)
    {
        vertical = shouldBeVertical;
        resized();
    }
}

void Toolbar::clear()
{
    for (int i = items.size(); --i >= 0;)
    {
        ToolbarItemComponent* const tc = items.getUnchecked(i);
        items.remove (i);
        delete tc;
    }

    resized();
}

ToolbarItemComponent* Toolbar::createItem (ToolbarItemFactory& factory, const int itemId)
{
    if (itemId == ToolbarItemFactory::separatorBarId)
        return new ToolbarSpacerComp (itemId, 0.1f, true);
    else if (itemId == ToolbarItemFactory::spacerId)
        return new ToolbarSpacerComp (itemId, 0.5f, false);
    else if (itemId == ToolbarItemFactory::flexibleSpacerId)
        return new ToolbarSpacerComp (itemId, 0, false);

    return factory.createItem (itemId);
}

void Toolbar::addItemInternal (ToolbarItemFactory& factory,
                               const int itemId,
                               const int insertIndex)
{
    // An ID can't be zero - this might indicate a mistake somewhere?
    jassert (itemId != 0);

    ToolbarItemComponent* const tc = createItem (factory, itemId);

    if (tc != 0)
    {
#ifdef JUCE_DEBUG
        Array <int> allowedIds;
        factory.getAllToolbarItemIds (allowedIds);

        // If your factory can create an item for a given ID, it must also return
        // that ID from its getAllToolbarItemIds() method!
        jassert (allowedIds.contains (itemId));
#endif

        items.insert (insertIndex, tc);
        addAndMakeVisible (tc, insertIndex);
    }
}

void Toolbar::addItem (ToolbarItemFactory& factory,
                       const int itemId,
                       const int insertIndex)
{
    addItemInternal (factory, itemId, insertIndex);
    resized();
}

void Toolbar::addDefaultItems (ToolbarItemFactory& factoryToUse)
{
    Array <int> ids;
    factoryToUse.getDefaultItemSet (ids);

    clear();

    for (int i = 0; i < ids.size(); ++i)
        addItemInternal (factoryToUse, ids.getUnchecked (i), -1);

    resized();
}

void Toolbar::removeToolbarItem (const int itemIndex)
{
    ToolbarItemComponent* const tc = getItemComponent (itemIndex);

    if (tc != 0)
    {
        items.removeValue (tc);
        delete tc;
        resized();
    }
}

int Toolbar::getNumItems() const throw()
{
    return items.size();
}

int Toolbar::getItemId (const int itemIndex) const throw()
{
    ToolbarItemComponent* const tc = getItemComponent (itemIndex);
    return tc != 0 ? tc->getItemId() : 0;
}

ToolbarItemComponent* Toolbar::getItemComponent (const int itemIndex) const throw()
{
    return items [itemIndex];
}

ToolbarItemComponent* Toolbar::getNextActiveComponent (int index, const int delta) const
{
    for (;;)
    {
        index += delta;
        ToolbarItemComponent* const tc = getItemComponent (index);

        if (tc == 0)
            break;

        if (tc->isActive)
            return tc;
    }

    return 0;
}

void Toolbar::setStyle (const ToolbarItemStyle& newStyle)
{
    if (toolbarStyle != newStyle)
    {
        toolbarStyle = newStyle;
        updateAllItemPositions (false);
    }
}

const String Toolbar::toString() const
{
    String s (T("TB:"));

    for (int i = 0; i < getNumItems(); ++i)
        s << getItemId(i) << T(' ');

    return s.trimEnd();
}

bool Toolbar::restoreFromString (ToolbarItemFactory& factoryToUse,
                                 const String& savedVersion)
{
    if (! savedVersion.startsWith (T("TB:")))
        return false;

    StringArray tokens;
    tokens.addTokens (savedVersion.substring (3), false);

    clear();

    for (int i = 0; i < tokens.size(); ++i)
        addItemInternal (factoryToUse, tokens[i].getIntValue(), -1);

    resized();
    return true;
}

void Toolbar::paint (Graphics& g)
{
    getLookAndFeel().paintToolbarBackground (g, getWidth(), getHeight(), *this);
}

int Toolbar::getThickness() const throw()
{
    return vertical ? getWidth() : getHeight();
}

int Toolbar::getLength() const throw()
{
    return vertical ? getHeight() : getWidth();
}

void Toolbar::setEditingActive (const bool active)
{
    if (isEditingActive != active)
    {
        isEditingActive = active;
        updateAllItemPositions (false);
    }
}

//==============================================================================
void Toolbar::resized()
{
    updateAllItemPositions (false);
}

void Toolbar::updateAllItemPositions (const bool animate)
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        StretchableObjectResizer resizer;

        int i;
        for (i = 0; i < items.size(); ++i)
        {
            ToolbarItemComponent* const tc = items.getUnchecked(i);

            tc->setEditingMode (isEditingActive ? ToolbarItemComponent::editableOnToolbar
                                                : ToolbarItemComponent::normalMode);

            tc->setStyle (toolbarStyle);

            ToolbarSpacerComp* const spacer = dynamic_cast <ToolbarSpacerComp*> (tc);

            int preferredSize = 1, minSize = 1, maxSize = 1;

            if (tc->getToolbarItemSizes (getThickness(), isVertical(),
                                         preferredSize, minSize, maxSize))
            {
                tc->isActive = true;
                resizer.addItem (preferredSize, minSize, maxSize,
                                 spacer != 0 ? spacer->getResizeOrder() : 2);
            }
            else
            {
                tc->isActive = false;
                tc->setVisible (false);
            }
        }

        resizer.resizeToFit (getLength());

        int totalLength = 0;

        for (i = 0; i < resizer.getNumItems(); ++i)
            totalLength += (int) resizer.getItemSize (i);

        const bool itemsOffTheEnd = totalLength > getLength();

        const int extrasButtonSize = getThickness() / 2;
        missingItemsButton->setSize (extrasButtonSize, extrasButtonSize);
        missingItemsButton->setVisible (itemsOffTheEnd);
        missingItemsButton->setEnabled (! isEditingActive);

        if (vertical)
            missingItemsButton->setCentrePosition (getWidth() / 2,
                                                   getHeight() - 4 - extrasButtonSize / 2);
        else
            missingItemsButton->setCentrePosition (getWidth() - 4 - extrasButtonSize / 2,
                                                   getHeight() / 2);

        const int maxLength = itemsOffTheEnd ? (vertical ? missingItemsButton->getY()
                                                         : missingItemsButton->getX()) - 4
                                             : getLength();

        int pos = 0, activeIndex = 0;
        for (i = 0; i < items.size(); ++i)
        {
            ToolbarItemComponent* const tc = items.getUnchecked(i);

            if (tc->isActive)
            {
                const int size = (int) resizer.getItemSize (activeIndex++);

                Rectangle newBounds;
                if (vertical)
                    newBounds.setBounds (0, pos, getWidth(), size);
                else
                    newBounds.setBounds (pos, 0, size, getHeight());

                if (animate)
                {
                    animator.animateComponent (tc, newBounds, 200, 3.0, 0.0);
                }
                else
                {
                    animator.cancelAnimation (tc, false);
                    tc->setBounds (newBounds);
                }

                pos += size;
                tc->setVisible (pos <= maxLength
                                 && ((! tc->isBeingDragged)
                                      || tc->getEditingMode() == ToolbarItemComponent::editableOnPalette));
            }
        }
    }
}

//==============================================================================
void Toolbar::buttonClicked (Button*)
{
    jassert (missingItemsButton->isShowing());

    if (missingItemsButton->isShowing())
    {
        PopupMenu m;
        m.addCustomItem (1, new MissingItemsComponent (*this, getThickness()));
        m.showAt (missingItemsButton);
    }
}

//==============================================================================
bool Toolbar::isInterestedInDragSource (const String& sourceDescription,
                                        Component* /*sourceComponent*/)
{
    return sourceDescription == toolbarDragDescriptor && isEditingActive;
}

void Toolbar::itemDragMove (const String&, Component* sourceComponent, int x, int y)
{
    ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (sourceComponent);

    if (tc != 0)
    {
        if (getNumItems() == 0)
        {
            if (tc->getEditingMode() == ToolbarItemComponent::editableOnPalette)
            {
                ToolbarItemPalette* const palette = tc->findParentComponentOfClass ((ToolbarItemPalette*) 0);

                if (palette != 0)
                    palette->replaceComponent (tc);
            }
            else
            {
                jassert (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar);
            }

            items.add (tc);
            addChildComponent (tc);
            updateAllItemPositions (false);
        }
        else
        {
            for (int i = getNumItems(); --i >= 0;)
            {
                int currentIndex = getIndexOfChildComponent (tc);

                if (currentIndex < 0)
                {
                    if (tc->getEditingMode() == ToolbarItemComponent::editableOnPalette)
                    {
                        ToolbarItemPalette* const palette = tc->findParentComponentOfClass ((ToolbarItemPalette*) 0);

                        if (palette != 0)
                            palette->replaceComponent (tc);
                    }
                    else
                    {
                        jassert (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar);
                    }

                    items.add (tc);
                    addChildComponent (tc);
                    currentIndex = getIndexOfChildComponent (tc);
                    updateAllItemPositions (true);
                }

                int newIndex = currentIndex;

                const int dragObjectLeft = vertical ? (y - tc->dragOffsetY) : (x - tc->dragOffsetX);
                const int dragObjectRight = dragObjectLeft + (vertical ? tc->getHeight() : tc->getWidth());

                const Rectangle current (animator.getComponentDestination (getChildComponent (newIndex)));
                ToolbarItemComponent* const prev = getNextActiveComponent (newIndex, -1);

                if (prev != 0)
                {
                    const Rectangle previousPos (animator.getComponentDestination (prev));

                    if (abs (dragObjectLeft - (vertical ? previousPos.getY() : previousPos.getX())
                          < abs (dragObjectRight - (vertical ? current.getBottom() : current.getRight()))))
                    {
                        newIndex = getIndexOfChildComponent (prev);
                    }
                }

                ToolbarItemComponent* const next = getNextActiveComponent (newIndex, 1);
                if (next != 0)
                {
                    const Rectangle nextPos (animator.getComponentDestination (next));

                    if (abs (dragObjectLeft - (vertical ? current.getY() : current.getX())
                         > abs (dragObjectRight - (vertical ? nextPos.getBottom() : nextPos.getRight()))))
                    {
                        newIndex = getIndexOfChildComponent (next) + 1;
                    }
                }

                if (newIndex != currentIndex)
                {
                    items.removeValue (tc);
                    removeChildComponent (tc);
                    addChildComponent (tc, newIndex);
                    items.insert (newIndex, tc);
                    updateAllItemPositions (true);
                }
                else
                {
                    break;
                }
            }
        }
    }
}

void Toolbar::itemDragExit (const String&, Component* sourceComponent)
{
    ToolbarItemComponent* const tc = dynamic_cast <ToolbarItemComponent*> (sourceComponent);

    if (tc != 0)
    {
        if (isParentOf (tc))
        {
            items.removeValue (tc);
            removeChildComponent (tc);
            updateAllItemPositions (true);
        }
    }
}

void Toolbar::itemDropped (const String&, Component*, int, int)
{
}


//==============================================================================
void Toolbar::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
    }
}


//==============================================================================
class ToolbarCustomisationDialog   : public DialogWindow
{
public:
    ToolbarCustomisationDialog (ToolbarItemFactory& factory,
                                Toolbar* const toolbar_,
                                const int optionFlags)
        : DialogWindow (TRANS("Add/remove items from toolbar"), Colours::white, true, true),
          toolbar (toolbar_)
    {
        setContentComponent (new CustomiserPanel (factory, toolbar, optionFlags), true, true);
        setResizable (true, true);
        setResizeLimits (400, 300, 1500, 1000);
        positionNearBar();
    }

    ~ToolbarCustomisationDialog()
    {
        setContentComponent (0, true);
    }

    void closeButtonPressed()
    {
        setVisible (false);
    }

    bool canModalEventBeSentToComponent (const Component* comp)
    {
        return toolbar->isParentOf (comp);
    }

    void positionNearBar()
    {
        const Rectangle screenSize (toolbar->getParentMonitorArea());
        const int tbx = toolbar->getScreenX();
        const int tby = toolbar->getScreenY();
        const int gap = 8;

        int x, y;

        if (toolbar->isVertical())
        {
            y = tby;

            if (tbx > screenSize.getCentreX())
                x = tbx - getWidth() - gap;
            else
                x = tbx + toolbar->getWidth() + gap;
        }
        else
        {
            x = tbx + (toolbar->getWidth() - getWidth()) / 2;

            if (tby > screenSize.getCentreY())
                y = tby - getHeight() - gap;
            else
                y = tby + toolbar->getHeight() + gap;
        }

        setTopLeftPosition (x, y);
    }

private:
    Toolbar* const toolbar;

    class CustomiserPanel  : public Component,
                             private ComboBoxListener,
                             private ButtonListener
    {
    public:
        CustomiserPanel (ToolbarItemFactory& factory_,
                         Toolbar* const toolbar_,
                         const int optionFlags)
          : factory (factory_),
            toolbar (toolbar_),
            styleBox (0),
            defaultButton (0)
        {
            addAndMakeVisible (palette = new ToolbarItemPalette (factory, toolbar));

            if ((optionFlags & (Toolbar::allowIconsOnlyChoice
                                 | Toolbar::allowIconsWithTextChoice
                                 | Toolbar::allowTextOnlyChoice)) != 0)
            {
                addAndMakeVisible (styleBox = new ComboBox (String::empty));
                styleBox->setEditableText (false);

                if ((optionFlags & Toolbar::allowIconsOnlyChoice) != 0)
                    styleBox->addItem (TRANS("Show icons only"), 1);
                if ((optionFlags & Toolbar::allowIconsWithTextChoice) != 0)
                    styleBox->addItem (TRANS("Show icons and descriptions"), 2);
                if ((optionFlags & Toolbar::allowTextOnlyChoice) != 0)
                    styleBox->addItem (TRANS("Show descriptions only"), 3);

                if (toolbar_->getStyle() == Toolbar::iconsOnly)
                    styleBox->setSelectedId (1);
                else if (toolbar_->getStyle() == Toolbar::iconsWithText)
                    styleBox->setSelectedId (2);
                else if (toolbar_->getStyle() == Toolbar::textOnly)
                    styleBox->setSelectedId (3);

                styleBox->addListener (this);
            }

            if ((optionFlags & Toolbar::showResetToDefaultsButton) != 0)
            {
                addAndMakeVisible (defaultButton = new TextButton (TRANS ("Restore to default set of items")));
                defaultButton->addButtonListener (this);
            }

            addAndMakeVisible (instructions = new Label (String::empty,
                TRANS ("You can drag the items above and drop them onto a toolbar to add them.\n\nItems on the toolbar can also be dragged around to change their order, or dragged off the edge to delete them.")));
            instructions->setFont (Font (13.0f));

            setSize (500, 300);
        }

        ~CustomiserPanel()
        {
            deleteAllChildren();
        }

        void comboBoxChanged (ComboBox*)
        {
            if (styleBox->getSelectedId() == 1)
                toolbar->setStyle (Toolbar::iconsOnly);
            else if (styleBox->getSelectedId() == 2)
                toolbar->setStyle (Toolbar::iconsWithText);
            else if (styleBox->getSelectedId() == 3)
                toolbar->setStyle (Toolbar::textOnly);

            palette->resized(); // to make it update the styles
        }

        void buttonClicked (Button*)
        {
            toolbar->addDefaultItems (factory);
        }

        void paint (Graphics& g)
        {
            Colour background;

            DialogWindow* const dw = findParentComponentOfClass ((DialogWindow*) 0);

            if (dw != 0)
                background = dw->getBackgroundColour();

            g.setColour (background.contrasting().withAlpha (0.3f));
            g.fillRect (palette->getX(), palette->getBottom() - 1, palette->getWidth(), 1);
        }

        void resized()
        {
            palette->setBounds (0, 0, getWidth(), getHeight() - 120);

            if (styleBox != 0)
                styleBox->setBounds (10, getHeight() - 110, 200, 22);

            if (defaultButton != 0)
            {
                defaultButton->changeWidthToFitText (22);
                defaultButton->setTopLeftPosition (240, getHeight() - 110);
            }

            instructions->setBounds (10, getHeight() - 80, getWidth() - 20, 80);
        }

    private:
        ToolbarItemFactory& factory;
        Toolbar* const toolbar;

        Label* instructions;
        ToolbarItemPalette* palette;
        ComboBox* styleBox;
        TextButton* defaultButton;
    };
};

void Toolbar::showCustomisationDialog (ToolbarItemFactory& factory, const int optionFlags)
{
    setEditingActive (true);

    ToolbarCustomisationDialog dw (factory, this, optionFlags);
    dw.runModalLoop();

    jassert (isValidComponent()); // ? deleting the toolbar while it's being edited?
    setEditingActive (false);
}


END_JUCE_NAMESPACE
