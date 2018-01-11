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

class TableHeaderComponent::DragOverlayComp   : public Component
{
public:
    DragOverlayComp (const Image& i) : image (i)
    {
        image.duplicateIfShared();
        image.multiplyAllAlphas (0.8f);
        setAlwaysOnTop (true);
    }

    void paint (Graphics& g) override
    {
        g.drawImageAt (image, 0, 0);
    }

    Image image;

    JUCE_DECLARE_NON_COPYABLE (DragOverlayComp)
};


//==============================================================================
TableHeaderComponent::TableHeaderComponent()
{
}

TableHeaderComponent::~TableHeaderComponent()
{
    dragOverlayComp.reset();
}

//==============================================================================
void TableHeaderComponent::setPopupMenuActive (bool hasMenu)
{
    menuActive = hasMenu;
}

bool TableHeaderComponent::isPopupMenuActive() const    { return menuActive; }


//==============================================================================
int TableHeaderComponent::getNumColumns (const bool onlyCountVisibleColumns) const
{
    if (onlyCountVisibleColumns)
    {
        int num = 0;

        for (auto* c : columns)
            if (c->isVisible())
                ++num;

        return num;
    }

    return columns.size();
}

String TableHeaderComponent::getColumnName (const int columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->name;

    return {};
}

void TableHeaderComponent::setColumnName (const int columnId, const String& newName)
{
    if (auto* ci = getInfoForId (columnId))
    {
        if (ci->name != newName)
        {
            ci->name = newName;
            sendColumnsChanged();
        }
    }
}

void TableHeaderComponent::addColumn (const String& columnName,
                                      int columnId,
                                      int width,
                                      int minimumWidth,
                                      int maximumWidth,
                                      int propertyFlags,
                                      int insertIndex)
{
    // can't have a duplicate or zero ID!
    jassert (columnId != 0 && getIndexOfColumnId (columnId, false) < 0);
    jassert (width > 0);

    auto ci = new ColumnInfo();
    ci->name = columnName;
    ci->id = columnId;
    ci->width = width;
    ci->lastDeliberateWidth = width;
    ci->minimumWidth = minimumWidth;
    ci->maximumWidth = maximumWidth >= 0 ? maximumWidth : std::numeric_limits<int>::max();
    jassert (ci->maximumWidth >= ci->minimumWidth);
    ci->propertyFlags = propertyFlags;

    columns.insert (insertIndex, ci);
    sendColumnsChanged();
}

void TableHeaderComponent::removeColumn (const int columnIdToRemove)
{
    auto index = getIndexOfColumnId (columnIdToRemove, false);

    if (index >= 0)
    {
        columns.remove (index);
        sortChanged = true;
        sendColumnsChanged();
    }
}

void TableHeaderComponent::removeAllColumns()
{
    if (columns.size() > 0)
    {
        columns.clear();
        sendColumnsChanged();
    }
}

void TableHeaderComponent::moveColumn (const int columnId, int newIndex)
{
    auto currentIndex = getIndexOfColumnId (columnId, false);
    newIndex = visibleIndexToTotalIndex (newIndex);

    if (columns [currentIndex] != 0 && currentIndex != newIndex)
    {
        columns.move (currentIndex, newIndex);
        sendColumnsChanged();
    }
}

int TableHeaderComponent::getColumnWidth (const int columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->width;

    return 0;
}

void TableHeaderComponent::setColumnWidth (const int columnId, const int newWidth)
{
    if (auto* ci = getInfoForId (columnId))
    {
        if (ci->width != newWidth)
        {
            auto numColumns = getNumColumns (true);

            ci->lastDeliberateWidth = ci->width
                = jlimit (ci->minimumWidth, ci->maximumWidth, newWidth);

            if (stretchToFit)
            {
                auto index = getIndexOfColumnId (columnId, true) + 1;

                if (isPositiveAndBelow (index, numColumns))
                {
                    auto x = getColumnPosition (index).getX();

                    if (lastDeliberateWidth == 0)
                        lastDeliberateWidth = getTotalWidth();

                    resizeColumnsToFit (visibleIndexToTotalIndex (index), lastDeliberateWidth - x);
                }
            }

            repaint();
            columnsResized = true;
            triggerAsyncUpdate();
        }
    }
}

//==============================================================================
int TableHeaderComponent::getIndexOfColumnId (const int columnId, const bool onlyCountVisibleColumns) const
{
    int n = 0;

    for (auto* c : columns)
    {
        if ((! onlyCountVisibleColumns) || c->isVisible())
        {
            if (c->id == columnId)
                return n;

            ++n;
        }
    }

    return -1;
}

int TableHeaderComponent::getColumnIdOfIndex (int index, const bool onlyCountVisibleColumns) const
{
    if (onlyCountVisibleColumns)
        index = visibleIndexToTotalIndex (index);

    if (auto* ci = columns [index])
        return ci->id;

    return 0;
}

Rectangle<int> TableHeaderComponent::getColumnPosition (const int index) const
{
    int x = 0, width = 0, n = 0;

    for (auto* c : columns)
    {
        x += width;

        if (c->isVisible())
        {
            width = c->width;

            if (n++ == index)
                break;
        }
        else
        {
            width = 0;
        }
    }

    return { x, 0, width, getHeight() };
}

int TableHeaderComponent::getColumnIdAtX (const int xToFind) const
{
    if (xToFind >= 0)
    {
        int x = 0;

        for (auto* ci : columns)
        {
            if (ci->isVisible())
            {
                x += ci->width;

                if (xToFind < x)
                    return ci->id;
            }
        }
    }

    return 0;
}

int TableHeaderComponent::getTotalWidth() const
{
    int w = 0;

    for (auto* c : columns)
        if (c->isVisible())
            w += c->width;

    return w;
}

void TableHeaderComponent::setStretchToFitActive (const bool shouldStretchToFit)
{
    stretchToFit = shouldStretchToFit;
    lastDeliberateWidth = getTotalWidth();
    resized();
}

bool TableHeaderComponent::isStretchToFitActive() const
{
    return stretchToFit;
}

void TableHeaderComponent::resizeAllColumnsToFit (int targetTotalWidth)
{
    if (stretchToFit && getWidth() > 0
         && columnIdBeingResized == 0 && columnIdBeingDragged == 0)
    {
        lastDeliberateWidth = targetTotalWidth;
        resizeColumnsToFit (0, targetTotalWidth);
    }
}

void TableHeaderComponent::resizeColumnsToFit (int firstColumnIndex, int targetTotalWidth)
{
    targetTotalWidth = jmax (targetTotalWidth, 0);
    StretchableObjectResizer sor;

    for (int i = firstColumnIndex; i < columns.size(); ++i)
    {
        auto* ci = columns.getUnchecked(i);

        if (ci->isVisible())
            sor.addItem (ci->lastDeliberateWidth, ci->minimumWidth, ci->maximumWidth);
    }

    sor.resizeToFit (targetTotalWidth);
    int visIndex = 0;

    for (int i = firstColumnIndex; i < columns.size(); ++i)
    {
        auto* ci = columns.getUnchecked(i);

        if (ci->isVisible())
        {
            auto newWidth = jlimit (ci->minimumWidth, ci->maximumWidth,
                                    (int) std::floor (sor.getItemSize (visIndex++)));

            if (newWidth != ci->width)
            {
                ci->width = newWidth;
                repaint();
                columnsResized = true;
                triggerAsyncUpdate();
            }
        }
    }
}

void TableHeaderComponent::setColumnVisible (const int columnId, const bool shouldBeVisible)
{
    if (auto* ci = getInfoForId (columnId))
    {
        if (shouldBeVisible != ci->isVisible())
        {
            if (shouldBeVisible)
                ci->propertyFlags |= visible;
            else
                ci->propertyFlags &= ~visible;

            sendColumnsChanged();
            resized();
        }
    }
}

bool TableHeaderComponent::isColumnVisible (const int columnId) const
{
    if (auto* ci = getInfoForId (columnId))
        return ci->isVisible();

    return false;
}

//==============================================================================
void TableHeaderComponent::setSortColumnId (const int columnId, const bool sortForwards)
{
    if (getSortColumnId() != columnId || isSortedForwards() != sortForwards)
    {
        for (auto* c : columns)
            c->propertyFlags &= ~(sortedForwards | sortedBackwards);

        if (auto* ci = getInfoForId (columnId))
            ci->propertyFlags |= (sortForwards ? sortedForwards : sortedBackwards);

        reSortTable();
    }
}

int TableHeaderComponent::getSortColumnId() const
{
    for (auto* c : columns)
        if ((c->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return c->id;

    return 0;
}

bool TableHeaderComponent::isSortedForwards() const
{
    for (auto* c : columns)
        if ((c->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return (c->propertyFlags & sortedForwards) != 0;

    return true;
}

void TableHeaderComponent::reSortTable()
{
    sortChanged = true;
    repaint();
    triggerAsyncUpdate();
}

//==============================================================================
String TableHeaderComponent::toString() const
{
    String s;

    XmlElement doc ("TABLELAYOUT");

    doc.setAttribute ("sortedCol", getSortColumnId());
    doc.setAttribute ("sortForwards", isSortedForwards());

    for (auto* ci : columns)
    {
        auto* e = doc.createNewChildElement ("COLUMN");
        e->setAttribute ("id", ci->id);
        e->setAttribute ("visible", ci->isVisible());
        e->setAttribute ("width", ci->width);
    }

    return doc.createDocument ({}, true, false);
}

void TableHeaderComponent::restoreFromString (const String& storedVersion)
{
    ScopedPointer<XmlElement> storedXml (XmlDocument::parse (storedVersion));
    int index = 0;

    if (storedXml != nullptr && storedXml->hasTagName ("TABLELAYOUT"))
    {
        forEachXmlChildElement (*storedXml, col)
        {
            auto tabId = col->getIntAttribute ("id");

            if (auto* ci = getInfoForId (tabId))
            {
                columns.move (columns.indexOf (ci), index);
                ci->width = col->getIntAttribute ("width");
                setColumnVisible (tabId, col->getBoolAttribute ("visible"));
            }

            ++index;
        }

        columnsResized = true;
        sendColumnsChanged();

        setSortColumnId (storedXml->getIntAttribute ("sortedCol"),
                         storedXml->getBoolAttribute ("sortForwards", true));
    }
}

//==============================================================================
void TableHeaderComponent::addListener (Listener* const newListener)
{
    listeners.addIfNotAlreadyThere (newListener);
}

void TableHeaderComponent::removeListener (Listener* const listenerToRemove)
{
    listeners.removeFirstMatchingValue (listenerToRemove);
}

//==============================================================================
void TableHeaderComponent::columnClicked (int columnId, const ModifierKeys& mods)
{
    if (auto* ci = getInfoForId (columnId))
        if ((ci->propertyFlags & sortable) != 0 && ! mods.isPopupMenu())
            setSortColumnId (columnId, (ci->propertyFlags & sortedForwards) == 0);
}

void TableHeaderComponent::addMenuItems (PopupMenu& menu, const int /*columnIdClicked*/)
{
    for (auto* ci : columns)
        if ((ci->propertyFlags & appearsOnColumnMenu) != 0)
            menu.addItem (ci->id, ci->name,
                          (ci->propertyFlags & (sortedForwards | sortedBackwards)) == 0,
                          isColumnVisible (ci->id));
}

void TableHeaderComponent::reactToMenuItem (const int menuReturnId, const int /*columnIdClicked*/)
{
    if (getIndexOfColumnId (menuReturnId, false) >= 0)
        setColumnVisible (menuReturnId, ! isColumnVisible (menuReturnId));
}

void TableHeaderComponent::paint (Graphics& g)
{
    LookAndFeel& lf = getLookAndFeel();

    lf.drawTableHeaderBackground (g, *this);

    const Rectangle<int> clip (g.getClipBounds());

    int x = 0;

    for (auto* ci : columns)
    {
        if (ci->isVisible())
        {
            if (x + ci->width > clip.getX()
                 && (ci->id != columnIdBeingDragged
                      || dragOverlayComp == nullptr
                      || ! dragOverlayComp->isVisible()))
            {
                Graphics::ScopedSaveState ss (g);

                g.setOrigin (x, 0);
                g.reduceClipRegion (0, 0, ci->width, getHeight());

                lf.drawTableHeaderColumn (g, *this, ci->name, ci->id, ci->width, getHeight(),
                                          ci->id == columnIdUnderMouse,
                                          ci->id == columnIdUnderMouse && isMouseButtonDown(),
                                          ci->propertyFlags);
            }

            x += ci->width;

            if (x >= clip.getRight())
                break;
        }
    }
}

void TableHeaderComponent::mouseMove  (const MouseEvent& e)  { updateColumnUnderMouse (e); }
void TableHeaderComponent::mouseEnter (const MouseEvent& e)  { updateColumnUnderMouse (e); }
void TableHeaderComponent::mouseExit  (const MouseEvent&)    { setColumnUnderMouse (0); }

void TableHeaderComponent::mouseDown (const MouseEvent& e)
{
    repaint();
    columnIdBeingResized = 0;
    columnIdBeingDragged = 0;

    if (columnIdUnderMouse != 0)
    {
        draggingColumnOffset = e.x - getColumnPosition (getIndexOfColumnId (columnIdUnderMouse, true)).getX();

        if (e.mods.isPopupMenu())
            columnClicked (columnIdUnderMouse, e.mods);
    }

    if (menuActive && e.mods.isPopupMenu())
        showColumnChooserMenu (columnIdUnderMouse);
}

void TableHeaderComponent::mouseDrag (const MouseEvent& e)
{
    if (columnIdBeingResized == 0
         && columnIdBeingDragged == 0
         && e.mouseWasDraggedSinceMouseDown()
         && ! e.mods.isPopupMenu())
    {
        dragOverlayComp.reset();

        columnIdBeingResized = getResizeDraggerAt (e.getMouseDownX());

        if (columnIdBeingResized != 0)
        {
            if (auto* ci = getInfoForId (columnIdBeingResized))
                initialColumnWidth = ci->width;
            else
                jassertfalse;
        }
        else
        {
            beginDrag (e);
        }
    }

    if (columnIdBeingResized != 0)
    {
        if (auto* ci = getInfoForId (columnIdBeingResized))
        {
            auto w = jlimit (ci->minimumWidth, ci->maximumWidth,
                             initialColumnWidth + e.getDistanceFromDragStartX());

            if (stretchToFit)
            {
                // prevent us dragging a column too far right if we're in stretch-to-fit mode
                int minWidthOnRight = 0;

                for (int i = getIndexOfColumnId (columnIdBeingResized, false) + 1; i < columns.size(); ++i)
                    if (columns.getUnchecked (i)->isVisible())
                        minWidthOnRight += columns.getUnchecked (i)->minimumWidth;

                auto currentPos = getColumnPosition (getIndexOfColumnId (columnIdBeingResized, true));
                w = jmax (ci->minimumWidth, jmin (w, lastDeliberateWidth - minWidthOnRight - currentPos.getX()));
            }

            setColumnWidth (columnIdBeingResized, w);
        }
    }
    else if (columnIdBeingDragged != 0)
    {
        if (e.y >= -50 && e.y < getHeight() + 50)
        {
            if (dragOverlayComp != nullptr)
            {
                dragOverlayComp->setVisible (true);
                dragOverlayComp->setBounds (jlimit (0,
                                                    jmax (0, getTotalWidth() - dragOverlayComp->getWidth()),
                                                    e.x - draggingColumnOffset),
                                            0,
                                            dragOverlayComp->getWidth(),
                                            getHeight());

                for (int i = columns.size(); --i >= 0;)
                {
                    const int currentIndex = getIndexOfColumnId (columnIdBeingDragged, true);
                    int newIndex = currentIndex;

                    if (newIndex > 0)
                    {
                        // if the previous column isn't draggable, we can't move our column
                        // past it, because that'd change the undraggable column's position..
                        auto* previous = columns.getUnchecked (newIndex - 1);

                        if ((previous->propertyFlags & draggable) != 0)
                        {
                            auto leftOfPrevious = getColumnPosition (newIndex - 1).getX();
                            auto rightOfCurrent = getColumnPosition (newIndex).getRight();

                            if (std::abs (dragOverlayComp->getX() - leftOfPrevious)
                                 < std::abs (dragOverlayComp->getRight() - rightOfCurrent))
                            {
                                --newIndex;
                            }
                        }
                    }

                    if (newIndex < columns.size() - 1)
                    {
                        // if the next column isn't draggable, we can't move our column
                        // past it, because that'd change the undraggable column's position..
                        auto* nextCol = columns.getUnchecked (newIndex + 1);

                        if ((nextCol->propertyFlags & draggable) != 0)
                        {
                            auto leftOfCurrent = getColumnPosition (newIndex).getX();
                            auto rightOfNext = getColumnPosition (newIndex + 1).getRight();

                            if (std::abs (dragOverlayComp->getX() - leftOfCurrent)
                                 > std::abs (dragOverlayComp->getRight() - rightOfNext))
                            {
                                ++newIndex;
                            }
                        }
                    }

                    if (newIndex != currentIndex)
                        moveColumn (columnIdBeingDragged, newIndex);
                    else
                        break;
                }
            }
        }
        else
        {
            endDrag (draggingColumnOriginalIndex);
        }
    }
}

void TableHeaderComponent::beginDrag (const MouseEvent& e)
{
    if (columnIdBeingDragged == 0)
    {
        columnIdBeingDragged = getColumnIdAtX (e.getMouseDownX());

        auto* ci = getInfoForId (columnIdBeingDragged);

        if (ci == nullptr || (ci->propertyFlags & draggable) == 0)
        {
            columnIdBeingDragged = 0;
        }
        else
        {
            draggingColumnOriginalIndex = getIndexOfColumnId (columnIdBeingDragged, true);

            auto columnRect = getColumnPosition (draggingColumnOriginalIndex);
            auto temp = columnIdBeingDragged;
            columnIdBeingDragged = 0;

            dragOverlayComp.reset (new DragOverlayComp (createComponentSnapshot (columnRect, false)));
            addAndMakeVisible (dragOverlayComp.get());
            columnIdBeingDragged = temp;

            dragOverlayComp->setBounds (columnRect);

            for (int i = listeners.size(); --i >= 0;)
            {
                listeners.getUnchecked(i)->tableColumnDraggingChanged (this, columnIdBeingDragged);
                i = jmin (i, listeners.size() - 1);
            }
        }
    }
}

void TableHeaderComponent::endDrag (const int finalIndex)
{
    if (columnIdBeingDragged != 0)
    {
        moveColumn (columnIdBeingDragged, finalIndex);

        columnIdBeingDragged = 0;
        repaint();

        for (int i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked(i)->tableColumnDraggingChanged (this, 0);
            i = jmin (i, listeners.size() - 1);
        }
    }
}

void TableHeaderComponent::mouseUp (const MouseEvent& e)
{
    mouseDrag (e);

    for (auto* c : columns)
        if (c->isVisible())
            c->lastDeliberateWidth = c->width;

    columnIdBeingResized = 0;
    repaint();

    endDrag (getIndexOfColumnId (columnIdBeingDragged, true));

    updateColumnUnderMouse (e);

    if (columnIdUnderMouse != 0 && ! (e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu()))
        columnClicked (columnIdUnderMouse, e.mods);

    dragOverlayComp.reset();
}

MouseCursor TableHeaderComponent::getMouseCursor()
{
    if (columnIdBeingResized != 0 || (getResizeDraggerAt (getMouseXYRelative().getX()) != 0 && ! isMouseButtonDown()))
        return MouseCursor (MouseCursor::LeftRightResizeCursor);

    return Component::getMouseCursor();
}

//==============================================================================
bool TableHeaderComponent::ColumnInfo::isVisible() const
{
    return (propertyFlags & TableHeaderComponent::visible) != 0;
}

TableHeaderComponent::ColumnInfo* TableHeaderComponent::getInfoForId (int id) const
{
    for (auto* c : columns)
        if (c->id == id)
            return c;

    return nullptr;
}

int TableHeaderComponent::visibleIndexToTotalIndex (const int visibleIndex) const
{
    int n = 0;

    for (int i = 0; i < columns.size(); ++i)
    {
        if (columns.getUnchecked(i)->isVisible())
        {
            if (n == visibleIndex)
                return i;

            ++n;
        }
    }

    return -1;
}

void TableHeaderComponent::sendColumnsChanged()
{
    if (stretchToFit && lastDeliberateWidth > 0)
        resizeAllColumnsToFit (lastDeliberateWidth);

    repaint();
    columnsChanged = true;
    triggerAsyncUpdate();
}

void TableHeaderComponent::handleAsyncUpdate()
{
    const bool changed = columnsChanged || sortChanged;
    const bool sized = columnsResized || changed;
    const bool sorted = sortChanged;
    columnsChanged = false;
    columnsResized = false;
    sortChanged = false;

    if (sorted)
    {
        for (int i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked(i)->tableSortOrderChanged (this);
            i = jmin (i, listeners.size() - 1);
        }
    }

    if (changed)
    {
        for (int i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked(i)->tableColumnsChanged (this);
            i = jmin (i, listeners.size() - 1);
        }
    }

    if (sized)
    {
        for (int i = listeners.size(); --i >= 0;)
        {
            listeners.getUnchecked(i)->tableColumnsResized (this);
            i = jmin (i, listeners.size() - 1);
        }
    }
}

int TableHeaderComponent::getResizeDraggerAt (const int mouseX) const
{
    if (isPositiveAndBelow (mouseX, getWidth()))
    {
        const int draggableDistance = 3;
        int x = 0;

        for (auto* ci : columns)
        {
            if (ci->isVisible())
            {
                if (std::abs (mouseX - (x + ci->width)) <= draggableDistance
                     && (ci->propertyFlags & resizable) != 0)
                    return ci->id;

                x += ci->width;
            }
        }
    }

    return 0;
}

void TableHeaderComponent::setColumnUnderMouse (const int newCol)
{
    if (newCol != columnIdUnderMouse)
    {
        columnIdUnderMouse = newCol;
        repaint();
    }
}

void TableHeaderComponent::updateColumnUnderMouse (const MouseEvent& e)
{
    setColumnUnderMouse (reallyContains (e.getPosition(), true) && getResizeDraggerAt (e.x) == 0
                            ? getColumnIdAtX (e.x) : 0);
}

static void tableHeaderMenuCallback (int result, TableHeaderComponent* tableHeader, int columnIdClicked)
{
    if (tableHeader != nullptr && result != 0)
        tableHeader->reactToMenuItem (result, columnIdClicked);
}

void TableHeaderComponent::showColumnChooserMenu (const int columnIdClicked)
{
    PopupMenu m;
    addMenuItems (m, columnIdClicked);

    if (m.getNumItems() > 0)
    {
        m.setLookAndFeel (&getLookAndFeel());

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (tableHeaderMenuCallback, this, columnIdClicked));
    }
}

void TableHeaderComponent::Listener::tableColumnDraggingChanged (TableHeaderComponent*, int)
{
}

} // namespace juce
