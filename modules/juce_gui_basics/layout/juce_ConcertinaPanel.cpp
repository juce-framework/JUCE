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

struct ConcertinaPanel::PanelSizes
{
    struct Panel
    {
        Panel() = default;

        Panel (int sz, int mn, int mx) noexcept
            : size (sz), minSize (mn), maxSize (mx) {}

        int setSize (int newSize) noexcept
        {
            jassert (minSize <= maxSize);
            auto oldSize = size;
            size = jlimit (minSize, maxSize, newSize);
            return size - oldSize;
        }

        int expand (int amount) noexcept
        {
            amount = jmin (amount, maxSize - size);
            size += amount;
            return amount;
        }

        int reduce (int amount) noexcept
        {
            amount = jmin (amount, size - minSize);
            size -= amount;
            return amount;
        }

        bool canExpand() const noexcept     { return size < maxSize; }
        bool isMinimised() const noexcept   { return size <= minSize; }

        int size, minSize, maxSize;
    };

    Array<Panel> sizes;

    Panel& get (int index) noexcept               { return sizes.getReference (index); }
    const Panel& get (int index) const noexcept   { return sizes.getReference (index); }

    PanelSizes withMovedPanel (int index, int targetPosition, int totalSpace) const
    {
        auto num = sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        targetPosition = jmax (targetPosition, totalSpace - getMaximumSize (index, num));

        PanelSizes newSizes (*this);
        newSizes.stretchRange (0, index, targetPosition - newSizes.getTotalSize (0, index), stretchLast);
        newSizes.stretchRange (index, num, totalSpace - newSizes.getTotalSize (0, index) - newSizes.getTotalSize (index, num), stretchFirst);
        return newSizes;
    }

    PanelSizes fittedInto (int totalSpace) const
    {
        auto newSizes (*this);
        auto num = newSizes.sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        newSizes.stretchRange (0, num, totalSpace - newSizes.getTotalSize (0, num), stretchAll);
        return newSizes;
    }

    PanelSizes withResizedPanel (int index, int panelHeight, int totalSpace) const
    {
        PanelSizes newSizes (*this);

        if (totalSpace <= 0)
        {
            newSizes.get (index).size = panelHeight;
        }
        else
        {
            auto num = sizes.size();
            auto minSize = getMinimumSize (0, num);
            totalSpace = jmax (totalSpace, minSize);

            newSizes.get (index).setSize (panelHeight);
            newSizes.stretchRange (0, index,   totalSpace - newSizes.getTotalSize (0, num), stretchLast);
            newSizes.stretchRange (index, num, totalSpace - newSizes.getTotalSize (0, num), stretchLast);
            newSizes = newSizes.fittedInto (totalSpace);
        }

        return newSizes;
    }

private:
    enum ExpandMode
    {
        stretchAll,
        stretchFirst,
        stretchLast
    };

    void growRangeFirst (int start, int end, int spaceDiff) noexcept
    {
        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = start; i < end && spaceDiff > 0; ++i)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    void growRangeLast (int start, int end, int spaceDiff) noexcept
    {
        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = end; --i >= start && spaceDiff > 0;)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    void growRangeAll (int start, int end, int spaceDiff) noexcept
    {
        Array<Panel*> expandableItems;

        for (int i = start; i < end; ++i)
            if (get (i).canExpand() && ! get (i).isMinimised())
                expandableItems.add (& get (i));

        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = expandableItems.size(); --i >= 0 && spaceDiff > 0;)
                spaceDiff -= expandableItems.getUnchecked (i)->expand (spaceDiff / (i + 1));

        growRangeLast (start, end, spaceDiff);
    }

    void shrinkRangeFirst (int start, int end, int spaceDiff) noexcept
    {
        for (int i = start; i < end && spaceDiff > 0; ++i)
            spaceDiff -= get (i).reduce (spaceDiff);
    }

    void shrinkRangeLast (int start, int end, int spaceDiff) noexcept
    {
        for (int i = end; --i >= start && spaceDiff > 0;)
            spaceDiff -= get (i).reduce (spaceDiff);
    }

    void stretchRange (int start, int end, int amountToAdd, ExpandMode expandMode) noexcept
    {
        if (end > start)
        {
            if (amountToAdd > 0)
            {
                if (expandMode == stretchAll)        growRangeAll   (start, end, amountToAdd);
                else if (expandMode == stretchFirst) growRangeFirst (start, end, amountToAdd);
                else if (expandMode == stretchLast)  growRangeLast  (start, end, amountToAdd);
            }
            else
            {
                if (expandMode == stretchFirst)  shrinkRangeFirst (start, end, -amountToAdd);
                else                             shrinkRangeLast  (start, end, -amountToAdd);
            }
        }
    }

    int getTotalSize (int start, int end) const noexcept
    {
        int tot = 0;
        while (start < end)  tot += get (start++).size;
        return tot;
    }

    int getMinimumSize (int start, int end) const noexcept
    {
        int tot = 0;
        while (start < end)  tot += get (start++).minSize;
        return tot;
    }

    int getMaximumSize (int start, int end) const noexcept
    {
        int tot = 0;

        while (start < end)
        {
            auto mx = get (start++).maxSize;

            if (mx > 0x100000)
                return mx;

            tot += mx;
        }

        return tot;
    }
};

//==============================================================================
class ConcertinaPanel::PanelHolder final : public Component
{
public:
    PanelHolder (Component* comp, bool takeOwnership)
        : component (comp, takeOwnership)
    {
        setRepaintsOnMouseActivity (true);
        setWantsKeyboardFocus (false);
        addAndMakeVisible (comp);
    }

    void paint (Graphics& g) override
    {
        if (customHeader.get() != nullptr)
            return;

        const Rectangle<int> area (getWidth(), getHeaderSize());
        g.reduceClipRegion (area);

        getLookAndFeel().drawConcertinaPanelHeader (g, area, isMouseOver(), isMouseButtonDown(),
                                                    getPanel(), *component);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto headerBounds = bounds.removeFromTop (getHeaderSize());

        if (customHeader.get() != nullptr)
            customHeader.get()->setBounds (headerBounds);

        component->setBounds (bounds);
    }

    void mouseDown (const MouseEvent&) override
    {
        mouseDownY = getY();
        dragStartSizes = getPanel().getFittedSizes();
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            auto& panel = getPanel();
            panel.setLayout (dragStartSizes.withMovedPanel (panel.holders.indexOf (this),
                                                            mouseDownY + e.getDistanceFromDragStartY(),
                                                            panel.getHeight()), false);
        }
    }

    void mouseDoubleClick (const MouseEvent&) override
    {
        getPanel().panelHeaderDoubleClicked (component);
    }

    void setCustomHeaderComponent (Component* headerComponent, bool shouldTakeOwnership)
    {
        customHeader = CustomHeader (this, OptionalScopedPointer (headerComponent, shouldTakeOwnership));
        addAndMakeVisible (headerComponent);
    }

    OptionalScopedPointer<Component> component;

private:
    PanelSizes dragStartSizes;
    int mouseDownY;

    struct CustomHeader
    {
        CustomHeader() = default;

        CustomHeader (MouseListener* l, OptionalScopedPointer<Component> c)
            : listener (l),
              customHeaderComponent (std::move (c))
        {
            if (customHeaderComponent != nullptr)
                customHeaderComponent->addMouseListener (listener, false);
        }

        CustomHeader (CustomHeader&& other) noexcept
            : listener (std::exchange (other.listener, nullptr)),
              customHeaderComponent (std::exchange (other.customHeaderComponent, {})) {}

        CustomHeader& operator= (CustomHeader&& other) noexcept
        {
            std::swap (other.listener, listener);
            std::swap (other.customHeaderComponent, customHeaderComponent);
            return *this;
        }

        CustomHeader (const CustomHeader& other) = delete;
        CustomHeader& operator= (const CustomHeader& other) = delete;

        ~CustomHeader() noexcept
        {
            if (customHeaderComponent != nullptr)
                customHeaderComponent->removeMouseListener (listener);
        }

        Component* get() const { return customHeaderComponent.get(); }

    private:
        MouseListener* listener = nullptr;
        OptionalScopedPointer<Component> customHeaderComponent;
    };

    CustomHeader customHeader;

    int getHeaderSize() const noexcept
    {
        ConcertinaPanel& panel = getPanel();
        auto ourIndex = panel.holders.indexOf (this);
        return panel.currentSizes->get (ourIndex).minSize;
    }

    ConcertinaPanel& getPanel() const
    {
        auto panel = dynamic_cast<ConcertinaPanel*> (getParentComponent());
        jassert (panel != nullptr);
        return *panel;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelHolder)
};

//==============================================================================
ConcertinaPanel::ConcertinaPanel()
    : currentSizes (new PanelSizes()),
      headerHeight (20)
{
}

ConcertinaPanel::~ConcertinaPanel() = default;

int ConcertinaPanel::getNumPanels() const noexcept
{
    return holders.size();
}

Component* ConcertinaPanel::getPanel (int index) const noexcept
{
    if (PanelHolder* h = holders[index])
        return h->component;

    return nullptr;
}

void ConcertinaPanel::addPanel (int insertIndex, Component* component, bool takeOwnership)
{
    jassert (component != nullptr); // can't use a null pointer here!
    jassert (indexOfComp (component) < 0); // You can't add the same component more than once!

    auto holder = new PanelHolder (component, takeOwnership);
    holders.insert (insertIndex, holder);
    currentSizes->sizes.insert (insertIndex, PanelSizes::Panel (headerHeight, headerHeight, std::numeric_limits<int>::max()));
    addAndMakeVisible (holder);
    resized();
}

void ConcertinaPanel::removePanel (Component* component)
{
    auto index = indexOfComp (component);

    if (index >= 0)
    {
        currentSizes->sizes.remove (index);
        holders.remove (index);
        resized();
    }
}

bool ConcertinaPanel::setPanelSize (Component* panelComponent, int height, bool animate)
{
    auto index = indexOfComp (panelComponent);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    height += currentSizes->get (index).minSize;
    auto oldSize = currentSizes->get (index).size;
    setLayout (currentSizes->withResizedPanel (index, height, getHeight()), animate);
    return oldSize != currentSizes->get (index).size;
}

bool ConcertinaPanel::expandPanelFully (Component* component, bool animate)
{
    return setPanelSize (component, getHeight(), animate);
}

void ConcertinaPanel::setMaximumPanelSize (Component* component, int maximumSize)
{
    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        currentSizes->get (index).maxSize = currentSizes->get (index).minSize + maximumSize;
        resized();
    }
}

void ConcertinaPanel::setPanelHeaderSize (Component* component, int headerSize)
{
    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        auto oldMin = currentSizes->get (index).minSize;

        currentSizes->get (index).minSize = headerSize;
        currentSizes->get (index).size += headerSize - oldMin;
        resized();
    }
}

void ConcertinaPanel::setCustomPanelHeader (Component* component, Component* customComponent, bool takeOwnership)
{
    OptionalScopedPointer<Component> optional (customComponent, takeOwnership);

    auto index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
        holders.getUnchecked (index)->setCustomHeaderComponent (optional.release(), takeOwnership);
}

void ConcertinaPanel::resized()
{
    applyLayout (getFittedSizes(), false);
}

int ConcertinaPanel::indexOfComp (Component* comp) const noexcept
{
    for (int i = 0; i < holders.size(); ++i)
        if (holders.getUnchecked (i)->component == comp)
            return i;

    return -1;
}

ConcertinaPanel::PanelSizes ConcertinaPanel::getFittedSizes() const
{
    return currentSizes->fittedInto (getHeight());
}

void ConcertinaPanel::applyLayout (const PanelSizes& sizes, bool animate)
{
    if (! animate)
        animator.cancelAllAnimations (false);

    const int animationDuration = 150;
    auto w = getWidth();
    int y = 0;

    for (int i = 0; i < holders.size(); ++i)
    {
        PanelHolder& p = *holders.getUnchecked (i);

        auto h = sizes.get (i).size;
        const Rectangle<int> pos (0, y, w, h);

        if (animate)
            animator.animateComponent (&p, pos, 1.0f, animationDuration, false, 1.0, 1.0);
        else
            p.setBounds (pos);

        y += h;
    }
}

void ConcertinaPanel::setLayout (const PanelSizes& sizes, bool animate)
{
    *currentSizes = sizes;
    applyLayout (getFittedSizes(), animate);
}

void ConcertinaPanel::panelHeaderDoubleClicked (Component* component)
{
    if (! expandPanelFully (component, true))
        setPanelSize (component, 0, true);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ConcertinaPanel::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
