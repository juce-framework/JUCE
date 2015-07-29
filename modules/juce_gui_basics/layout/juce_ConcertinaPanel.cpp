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

struct ConcertinaPanel::PanelSizes
{
    struct Panel
    {
        Panel() noexcept {}

        Panel (const int sz, const int mn, const int mx) noexcept
            : size (sz), minSize (mn), maxSize (mx) {}

        int setSize (const int newSize) noexcept
        {
            jassert (minSize <= maxSize);
            const int oldSize = size;
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

    Panel& get (const int index) const noexcept    { return sizes.getReference(index); }

    PanelSizes withMovedPanel (const int index, int targetPosition, int totalSpace) const
    {
        const int num = sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        targetPosition = jmax (targetPosition, totalSpace - getMaximumSize (index, num));

        PanelSizes newSizes (*this);
        newSizes.stretchRange (0, index, targetPosition - newSizes.getTotalSize (0, index), stretchLast);
        newSizes.stretchRange (index, num, totalSpace - newSizes.getTotalSize (0, index) - newSizes.getTotalSize (index, num), stretchFirst);
        return newSizes;
    }

    PanelSizes fittedInto (int totalSpace) const
    {
        PanelSizes newSizes (*this);
        const int num = newSizes.sizes.size();
        totalSpace = jmax (totalSpace, getMinimumSize (0, num));
        newSizes.stretchRange (0, num, totalSpace - newSizes.getTotalSize (0, num), stretchAll);
        return newSizes;
    }

    PanelSizes withResizedPanel (const int index, int panelHeight, int totalSpace) const
    {
        PanelSizes newSizes (*this);

        if (totalSpace <= 0)
        {
            newSizes.get(index).size = panelHeight;
        }
        else
        {
            const int num = sizes.size();
            const int minSize = getMinimumSize (0, num);
            totalSpace = jmax (totalSpace, minSize);

            newSizes.get(index).setSize (panelHeight);
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

    void growRangeFirst (const int start, const int end, int spaceDiff) noexcept
    {
        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = start; i < end && spaceDiff > 0; ++i)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    void growRangeLast (const int start, const int end, int spaceDiff) noexcept
    {
        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = end; --i >= start && spaceDiff > 0;)
                spaceDiff -= get (i).expand (spaceDiff);
    }

    void growRangeAll (const int start, const int end, int spaceDiff) noexcept
    {
        Array<Panel*> expandableItems;

        for (int i = start; i < end; ++i)
            if (get(i).canExpand() && ! get(i).isMinimised())
                expandableItems.add (& get(i));

        for (int attempts = 4; --attempts >= 0 && spaceDiff > 0;)
            for (int i = expandableItems.size(); --i >= 0 && spaceDiff > 0;)
                spaceDiff -= expandableItems.getUnchecked(i)->expand (spaceDiff / (i + 1));

        growRangeLast (start, end, spaceDiff);
    }

    void shrinkRangeFirst (const int start, const int end, int spaceDiff) noexcept
    {
        for (int i = start; i < end && spaceDiff > 0; ++i)
            spaceDiff -= get(i).reduce (spaceDiff);
    }

    void shrinkRangeLast (const int start, const int end, int spaceDiff) noexcept
    {
        for (int i = end; --i >= start && spaceDiff > 0;)
            spaceDiff -= get(i).reduce (spaceDiff);
    }

    void stretchRange (const int start, const int end, const int amountToAdd,
                       const ExpandMode expandMode) noexcept
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

    int getTotalSize (int start, const int end) const noexcept
    {
        int tot = 0;
        while (start < end)  tot += get(start++).size;
        return tot;
    }

    int getMinimumSize (int start, const int end) const noexcept
    {
        int tot = 0;
        while (start < end)  tot += get(start++).minSize;
        return tot;
    }

    int getMaximumSize (int start, const int end) const noexcept
    {
        int tot = 0;
        while (start < end)
        {
            const int mx = get(start++).maxSize;
            if (mx > 0x100000)
                return mx;

            tot += mx;
        }

        return tot;
    }
};

//==============================================================================
class ConcertinaPanel::PanelHolder  : public Component
{
public:
    PanelHolder (Component* const comp, bool takeOwnership)
        : component (comp, takeOwnership)
    {
        setRepaintsOnMouseActivity (true);
        setWantsKeyboardFocus (false);
        addAndMakeVisible (comp);
    }

    void paint (Graphics& g) override
    {
        const Rectangle<int> area (getWidth(), getHeaderSize());
        g.reduceClipRegion (area);

        getLookAndFeel().drawConcertinaPanelHeader (g, area, isMouseOver(), isMouseButtonDown(),
                                                    getPanel(), *component);
    }

    void resized() override
    {
        component->setBounds (getLocalBounds().withTop (getHeaderSize()));
    }

    void mouseDown (const MouseEvent&) override
    {
        mouseDownY = getY();
        dragStartSizes = getPanel().getFittedSizes();
    }

    void mouseDrag (const MouseEvent& e) override
    {
        ConcertinaPanel& panel = getPanel();
        panel.setLayout (dragStartSizes.withMovedPanel (panel.holders.indexOf (this),
                                                        mouseDownY + e.getDistanceFromDragStartY(),
                                                        panel.getHeight()), false);
    }

    void mouseDoubleClick (const MouseEvent&) override
    {
        getPanel().panelHeaderDoubleClicked (component);
    }

    OptionalScopedPointer<Component> component;

private:
    PanelSizes dragStartSizes;
    int mouseDownY;

    int getHeaderSize() const noexcept
    {
        ConcertinaPanel& panel = getPanel();
        const int ourIndex = panel.holders.indexOf (this);
        return panel.currentSizes->get(ourIndex).minSize;
    }

    ConcertinaPanel& getPanel() const
    {
        ConcertinaPanel* const panel = dynamic_cast<ConcertinaPanel*> (getParentComponent());
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

ConcertinaPanel::~ConcertinaPanel() {}

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

    PanelHolder* const holder = new PanelHolder (component, takeOwnership);
    holders.insert (insertIndex, holder);
    currentSizes->sizes.insert (insertIndex, PanelSizes::Panel (headerHeight, headerHeight, std::numeric_limits<int>::max()));
    addAndMakeVisible (holder);
    resized();
}

void ConcertinaPanel::removePanel (Component* component)
{
    const int index = indexOfComp (component);

    if (index >= 0)
    {
        currentSizes->sizes.remove (index);
        holders.remove (index);
        resized();
    }
}

bool ConcertinaPanel::setPanelSize (Component* panelComponent, int height, const bool animate)
{
    const int index = indexOfComp (panelComponent);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    height += currentSizes->get(index).minSize;
    const int oldSize = currentSizes->get(index).size;
    setLayout (currentSizes->withResizedPanel (index, height, getHeight()), animate);
    return oldSize != currentSizes->get(index).size;
}

bool ConcertinaPanel::expandPanelFully (Component* component, const bool animate)
{
    return setPanelSize (component, getHeight(), animate);
}

void ConcertinaPanel::setMaximumPanelSize (Component* component, int maximumSize)
{
    const int index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        currentSizes->get(index).maxSize = currentSizes->get(index).minSize + maximumSize;
        resized();
    }
}

void ConcertinaPanel::setPanelHeaderSize (Component* component, int headerSize)
{
    const int index = indexOfComp (component);
    jassert (index >= 0); // The specified component doesn't seem to have been added!

    if (index >= 0)
    {
        currentSizes->get(index).minSize = headerSize;
        resized();
    }
}

void ConcertinaPanel::resized()
{
    applyLayout (getFittedSizes(), false);
}

int ConcertinaPanel::indexOfComp (Component* comp) const noexcept
{
    for (int i = 0; i < holders.size(); ++i)
        if (holders.getUnchecked(i)->component == comp)
            return i;

    return -1;
}

ConcertinaPanel::PanelSizes ConcertinaPanel::getFittedSizes() const
{
    return currentSizes->fittedInto (getHeight());
}

void ConcertinaPanel::applyLayout (const PanelSizes& sizes, const bool animate)
{
    if (! animate)
        animator.cancelAllAnimations (false);

    const int animationDuration = 150;
    const int w = getWidth();
    int y = 0;

    for (int i = 0; i < holders.size(); ++i)
    {
        PanelHolder& p = *holders.getUnchecked(i);

        const int h = sizes.get(i).size;
        const Rectangle<int> pos (0, y, w, h);

        if (animate)
            animator.animateComponent (&p, pos, 1.0f, animationDuration, false, 1.0, 1.0);
        else
            p.setBounds (pos);

        y += h;
    }
}

void ConcertinaPanel::setLayout (const PanelSizes& sizes, const bool animate)
{
    *currentSizes = sizes;
    applyLayout (getFittedSizes(), animate);
}

void ConcertinaPanel::panelHeaderDoubleClicked (Component* component)
{
    if (! expandPanelFully (component, true))
        setPanelSize (component, 0, true);
}
