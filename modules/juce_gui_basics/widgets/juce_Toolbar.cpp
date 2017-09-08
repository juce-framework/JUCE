/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

const char* const Toolbar::toolbarDragDescriptor = "_toolbarItem_";

//==============================================================================
class Toolbar::Spacer  : public ToolbarItemComponent
{
public:
    Spacer (const int itemId_, const float fixedSize_, const bool drawBar_)
        : ToolbarItemComponent (itemId_, String(), false),
          fixedSize (fixedSize_),
          drawBar (drawBar_)
    {
        setWantsKeyboardFocus (false);
    }

    bool getToolbarItemSizes (int toolbarThickness, bool /*isToolbarVertical*/,
                              int& preferredSize, int& minSize, int& maxSize) override
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

    void paintButtonArea (Graphics&, int, int, bool, bool) override
    {
    }

    void contentAreaChanged (const Rectangle<int>&) override
    {
    }

    int getResizeOrder() const noexcept
    {
        return fixedSize <= 0 ? 0 : 1;
    }

    void paint (Graphics& g) override
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
                p.addArrow (Line<float> (x1, y1, x2, y2), 1.5f, hw, hl);
                p.addArrow (Line<float> (x3, y3, x4, y4), 1.5f, hw, hl);
                g.fillPath (p);
            }
        }
    }

private:
    const float fixedSize;
    const bool drawBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spacer)
};

//==============================================================================
class Toolbar::MissingItemsComponent  : public PopupMenu::CustomComponent
{
public:
    MissingItemsComponent (Toolbar& bar, const int h)
        : PopupMenu::CustomComponent (true),
          owner (&bar),
          height (h)
    {
        for (int i = bar.items.size(); --i >= 0;)
        {
            ToolbarItemComponent* const tc = bar.items.getUnchecked(i);

            if (dynamic_cast<Spacer*> (tc) == nullptr && ! tc->isVisible())
            {
                oldIndexes.insert (0, i);
                addAndMakeVisible (tc, 0);
            }
        }

        layout (400);
    }

    ~MissingItemsComponent()
    {
        if (owner != nullptr)
        {
            for (int i = 0; i < getNumChildComponents(); ++i)
            {
                if (auto* tc = dynamic_cast<ToolbarItemComponent*> (getChildComponent (i)))
                {
                    tc->setVisible (false);
                    const int index = oldIndexes.removeAndReturn (i);
                    owner->addChildComponent (tc, index);
                    --i;
                }
            }

            owner->resized();
        }
    }

    void layout (const int preferredWidth)
    {
        const int indent = 8;
        int x = indent;
        int y = indent;
        int maxX = 0;

        for (auto* c : getChildren())
        {
            if (auto* tc = dynamic_cast<ToolbarItemComponent*> (c))
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

    void getIdealSize (int& idealWidth, int& idealHeight) override
    {
        idealWidth = getWidth();
        idealHeight = getHeight();
    }

private:
    Component::SafePointer<Toolbar> owner;
    const int height;
    Array<int> oldIndexes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingItemsComponent)
};


//==============================================================================
Toolbar::Toolbar()
    : vertical (false),
      isEditingActive (false),
      toolbarStyle (Toolbar::iconsOnly)
{
    addChildComponent (missingItemsButton = getLookAndFeel().createToolbarMissingItemsButton (*this));

    missingItemsButton->setAlwaysOnTop (true);
    missingItemsButton->addListener (this);
}

Toolbar::~Toolbar()
{
    items.clear();
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
    items.clear();
    resized();
}

ToolbarItemComponent* Toolbar::createItem (ToolbarItemFactory& factory, const int itemId)
{
    if (itemId == ToolbarItemFactory::separatorBarId)    return new Spacer (itemId, 0.1f, true);
    if (itemId == ToolbarItemFactory::spacerId)          return new Spacer (itemId, 0.5f, false);
    if (itemId == ToolbarItemFactory::flexibleSpacerId)  return new Spacer (itemId, 0.0f, false);

    return factory.createItem (itemId);
}

void Toolbar::addItemInternal (ToolbarItemFactory& factory,
                               const int itemId,
                               const int insertIndex)
{
    // An ID can't be zero - this might indicate a mistake somewhere?
    jassert (itemId != 0);

    if (ToolbarItemComponent* const tc = createItem (factory, itemId))
    {
       #if JUCE_DEBUG
        Array<int> allowedIds;
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
    Array<int> ids;
    factoryToUse.getDefaultItemSet (ids);

    clear();

    for (int i = 0; i < ids.size(); ++i)
        addItemInternal (factoryToUse, ids.getUnchecked (i), -1);

    resized();
}

void Toolbar::removeToolbarItem (const int itemIndex)
{
    items.remove (itemIndex);
    resized();
}

ToolbarItemComponent* Toolbar::removeAndReturnItem (const int itemIndex)
{
    if (ToolbarItemComponent* const tc = items.removeAndReturn (itemIndex))
    {
        removeChildComponent (tc);
        resized();
        return tc;
    }

    return nullptr;
}

int Toolbar::getNumItems() const noexcept
{
    return items.size();
}

int Toolbar::getItemId (const int itemIndex) const noexcept
{
    if (ToolbarItemComponent* const tc = getItemComponent (itemIndex))
        return tc->getItemId();

    return 0;
}

ToolbarItemComponent* Toolbar::getItemComponent (const int itemIndex) const noexcept
{
    return items [itemIndex];
}

ToolbarItemComponent* Toolbar::getNextActiveComponent (int index, const int delta) const
{
    for (;;)
    {
        index += delta;

        if (ToolbarItemComponent* const tc = getItemComponent (index))
        {
            if (tc->isActive)
                return tc;
        }
        else
        {
            return nullptr;
        }
    }
}

void Toolbar::setStyle (const ToolbarItemStyle& newStyle)
{
    if (toolbarStyle != newStyle)
    {
        toolbarStyle = newStyle;
        updateAllItemPositions (false);
    }
}

String Toolbar::toString() const
{
    String s ("TB:");

    for (int i = 0; i < getNumItems(); ++i)
        s << getItemId(i) << ' ';

    return s.trimEnd();
}

bool Toolbar::restoreFromString (ToolbarItemFactory& factoryToUse,
                                 const String& savedVersion)
{
    if (! savedVersion.startsWith ("TB:"))
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

int Toolbar::getThickness() const noexcept
{
    return vertical ? getWidth() : getHeight();
}

int Toolbar::getLength() const noexcept
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

        for (int i = 0; i < items.size(); ++i)
        {
            ToolbarItemComponent* const tc = items.getUnchecked(i);

            tc->setEditingMode (isEditingActive ? ToolbarItemComponent::editableOnToolbar
                                                : ToolbarItemComponent::normalMode);

            tc->setStyle (toolbarStyle);

            Spacer* const spacer = dynamic_cast<Spacer*> (tc);

            int preferredSize = 1, minSize = 1, maxSize = 1;

            if (tc->getToolbarItemSizes (getThickness(), isVertical(),
                                         preferredSize, minSize, maxSize))
            {
                tc->isActive = true;
                resizer.addItem (preferredSize, minSize, maxSize,
                                 spacer != nullptr ? spacer->getResizeOrder() : 2);
            }
            else
            {
                tc->isActive = false;
                tc->setVisible (false);
            }
        }

        resizer.resizeToFit (getLength());

        int totalLength = 0;

        for (int i = 0; i < resizer.getNumItems(); ++i)
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
        for (int i = 0; i < items.size(); ++i)
        {
            ToolbarItemComponent* const tc = items.getUnchecked(i);

            if (tc->isActive)
            {
                const int size = (int) resizer.getItemSize (activeIndex++);

                Rectangle<int> newBounds;
                if (vertical)
                    newBounds.setBounds (0, pos, getWidth(), size);
                else
                    newBounds.setBounds (pos, 0, size, getHeight());

                ComponentAnimator& animator = Desktop::getInstance().getAnimator();

                if (animate)
                {
                    animator.animateComponent (tc, newBounds, 1.0f, 200, false, 3.0, 0.0);
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
        m.showMenuAsync (PopupMenu::Options().withTargetComponent (missingItemsButton), nullptr);
    }
}

//==============================================================================
bool Toolbar::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    return dragSourceDetails.description == toolbarDragDescriptor && isEditingActive;
}

void Toolbar::itemDragMove (const SourceDetails& dragSourceDetails)
{
    if (ToolbarItemComponent* const tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
    {
        if (! items.contains (tc))
        {
            if (tc->getEditingMode() == ToolbarItemComponent::editableOnPalette)
            {
                if (ToolbarItemPalette* const palette = tc->findParentComponentOfClass<ToolbarItemPalette>())
                    palette->replaceComponent (*tc);
            }
            else
            {
                jassert (tc->getEditingMode() == ToolbarItemComponent::editableOnToolbar);
            }

            items.add (tc);
            addChildComponent (tc);
            updateAllItemPositions (true);
        }

        ComponentAnimator& animator = Desktop::getInstance().getAnimator();

        for (int i = getNumItems(); --i >= 0;)
        {
            const int currentIndex = items.indexOf (tc);
            int newIndex = currentIndex;

            const int dragObjectLeft = vertical ? (dragSourceDetails.localPosition.getY() - tc->dragOffsetY)
                                                : (dragSourceDetails.localPosition.getX() - tc->dragOffsetX);
            const int dragObjectRight = dragObjectLeft + (vertical ? tc->getHeight() : tc->getWidth());

            const Rectangle<int> current (animator.getComponentDestination (getChildComponent (newIndex)));

            if (ToolbarItemComponent* const prev = getNextActiveComponent (newIndex, -1))
            {
                const Rectangle<int> previousPos (animator.getComponentDestination (prev));

                if (std::abs (dragObjectLeft - (vertical ? previousPos.getY() : previousPos.getX()))
                     < std::abs (dragObjectRight - (vertical ? current.getBottom() : current.getRight())))
                {
                    newIndex = getIndexOfChildComponent (prev);
                }
            }

            if (ToolbarItemComponent* const next = getNextActiveComponent (newIndex, 1))
            {
                const Rectangle<int> nextPos (animator.getComponentDestination (next));

                if (std::abs (dragObjectLeft - (vertical ? current.getY() : current.getX()))
                     > std::abs (dragObjectRight - (vertical ? nextPos.getBottom() : nextPos.getRight())))
                {
                    newIndex = getIndexOfChildComponent (next) + 1;
                }
            }

            if (newIndex == currentIndex)
                break;

            items.removeObject (tc, false);
            removeChildComponent (tc);
            addChildComponent (tc, newIndex);
            items.insert (newIndex, tc);
            updateAllItemPositions (true);
        }
    }
}

void Toolbar::itemDragExit (const SourceDetails& dragSourceDetails)
{
    if (ToolbarItemComponent* const tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
    {
        if (isParentOf (tc))
        {
            items.removeObject (tc, false);
            removeChildComponent (tc);
            updateAllItemPositions (true);
        }
    }
}

void Toolbar::itemDropped (const SourceDetails& dragSourceDetails)
{
    if (ToolbarItemComponent* const tc = dynamic_cast<ToolbarItemComponent*> (dragSourceDetails.sourceComponent.get()))
        tc->setState (Button::buttonNormal);
}

void Toolbar::mouseDown (const MouseEvent&) {}

//==============================================================================
class Toolbar::CustomisationDialog   : public DialogWindow
{
public:
    CustomisationDialog (ToolbarItemFactory& factory, Toolbar& bar, int optionFlags)
        : DialogWindow (TRANS("Add/remove items from toolbar"), Colours::white, true, true),
          toolbar (bar)
    {
        setContentOwned (new CustomiserPanel (factory, toolbar, optionFlags), true);
        setResizable (true, true);
        setResizeLimits (400, 300, 1500, 1000);
        positionNearBar();
    }

    ~CustomisationDialog()
    {
        toolbar.setEditingActive (false);
    }

    void closeButtonPressed() override
    {
        setVisible (false);
    }

    bool canModalEventBeSentToComponent (const Component* comp) override
    {
        return toolbar.isParentOf (comp)
                 || dynamic_cast<const ToolbarItemComponent::ItemDragAndDropOverlayComponent*> (comp) != nullptr;
    }

    void positionNearBar()
    {
        const Rectangle<int> screenSize (toolbar.getParentMonitorArea());
        Point<int> pos (toolbar.getScreenPosition());
        const int gap = 8;

        if (toolbar.isVertical())
        {
            if (pos.x > screenSize.getCentreX())
                pos.x -= getWidth() - gap;
            else
                pos.x += toolbar.getWidth() + gap;
        }
        else
        {
            pos.x += (toolbar.getWidth() - getWidth()) / 2;

            if (pos.y > screenSize.getCentreY())
                pos.y -= getHeight() - gap;
            else
                pos.y += toolbar.getHeight() + gap;
        }

        setTopLeftPosition (pos);
    }

private:
    Toolbar& toolbar;

    class CustomiserPanel  : public Component,
                             private ComboBox::Listener,
                             private Button::Listener
    {
    public:
        CustomiserPanel (ToolbarItemFactory& tbf, Toolbar& bar, int optionFlags)
          : factory (tbf), toolbar (bar), palette (tbf, bar),
            instructions (String(), TRANS ("You can drag the items above and drop them onto a toolbar to add them.")
                                      + "\n\n"
                                      + TRANS ("Items on the toolbar can also be dragged around to change their order, or dragged off the edge to delete them.")),
            defaultButton (TRANS ("Restore to default set of items"))
        {
            addAndMakeVisible (palette);

            if ((optionFlags & (Toolbar::allowIconsOnlyChoice
                                 | Toolbar::allowIconsWithTextChoice
                                 | Toolbar::allowTextOnlyChoice)) != 0)
            {
                addAndMakeVisible (styleBox);
                styleBox.setEditableText (false);

                if ((optionFlags & Toolbar::allowIconsOnlyChoice) != 0)     styleBox.addItem (TRANS("Show icons only"), 1);
                if ((optionFlags & Toolbar::allowIconsWithTextChoice) != 0) styleBox.addItem (TRANS("Show icons and descriptions"), 2);
                if ((optionFlags & Toolbar::allowTextOnlyChoice) != 0)      styleBox.addItem (TRANS("Show descriptions only"), 3);

                int selectedStyle = 0;
                switch (bar.getStyle())
                {
                    case Toolbar::iconsOnly:        selectedStyle = 1; break;
                    case Toolbar::iconsWithText:    selectedStyle = 2; break;
                    case Toolbar::textOnly:         selectedStyle = 3; break;
                }

                styleBox.setSelectedId (selectedStyle);

                styleBox.addListener (this);
            }

            if ((optionFlags & Toolbar::showResetToDefaultsButton) != 0)
            {
                addAndMakeVisible (defaultButton);
                defaultButton.addListener (this);
            }

            addAndMakeVisible (instructions);
            instructions.setFont (Font (13.0f));

            setSize (500, 300);
        }

        void comboBoxChanged (ComboBox*) override
        {
            switch (styleBox.getSelectedId())
            {
                case 1:   toolbar.setStyle (Toolbar::iconsOnly); break;
                case 2:   toolbar.setStyle (Toolbar::iconsWithText); break;
                case 3:   toolbar.setStyle (Toolbar::textOnly); break;
            }

            palette.resized(); // to make it update the styles
        }

        void buttonClicked (Button*) override
        {
            toolbar.addDefaultItems (factory);
        }

        void paint (Graphics& g) override
        {
            Colour background;

            if (DialogWindow* const dw = findParentComponentOfClass<DialogWindow>())
                background = dw->getBackgroundColour();

            g.setColour (background.contrasting().withAlpha (0.3f));
            g.fillRect (palette.getX(), palette.getBottom() - 1, palette.getWidth(), 1);
        }

        void resized() override
        {
            palette.setBounds (0, 0, getWidth(), getHeight() - 120);
            styleBox.setBounds (10, getHeight() - 110, 200, 22);

            defaultButton.changeWidthToFitText (22);
            defaultButton.setTopLeftPosition (240, getHeight() - 110);

            instructions.setBounds (10, getHeight() - 80, getWidth() - 20, 80);
        }

    private:
        ToolbarItemFactory& factory;
        Toolbar& toolbar;

        ToolbarItemPalette palette;
        Label instructions;
        ComboBox styleBox;
        TextButton defaultButton;
    };
};

void Toolbar::showCustomisationDialog (ToolbarItemFactory& factory, const int optionFlags)
{
    setEditingActive (true);

    (new CustomisationDialog (factory, *this, optionFlags))
        ->enterModalState (true, nullptr, true);
}

} // namespace juce
