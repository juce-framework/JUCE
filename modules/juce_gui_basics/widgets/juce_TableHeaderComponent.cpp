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

class TableHeaderComponent::DragOverlayComp   : public Component
{
public:
    DragOverlayComp (const Image& image_)
        : image (image_)
    {
        image.duplicateIfShared();
        image.multiplyAllAlphas (0.8f);
        setAlwaysOnTop (true);
    }

    void paint (Graphics& g) override
    {
        g.drawImageAt (image, 0, 0);
    }

private:
    Image image;

    JUCE_DECLARE_NON_COPYABLE (DragOverlayComp)
};


//==============================================================================
TableHeaderComponent::TableHeaderComponent()
    : columnsChanged (false),
      columnsResized (false),
      sortChanged (false),
      menuActive (true),
      stretchToFit (false),
      columnIdBeingResized (0),
      columnIdBeingDragged (0),
      columnIdUnderMouse (0),
      lastDeliberateWidth (0)
{
}

TableHeaderComponent::~TableHeaderComponent()
{
    dragOverlayComp = nullptr;
}

//==============================================================================
void TableHeaderComponent::setPopupMenuActive (const bool hasMenu)
{
    menuActive = hasMenu;
}

bool TableHeaderComponent::isPopupMenuActive() const                    { return menuActive; }


//==============================================================================
int TableHeaderComponent::getNumColumns (const bool onlyCountVisibleColumns) const
{
    if (onlyCountVisibleColumns)
    {
        int num = 0;

        for (int i = columns.size(); --i >= 0;)
            if (columns.getUnchecked(i)->isVisible())
                ++num;

        return num;
    }

    return columns.size();
}

String TableHeaderComponent::getColumnName (const int columnId) const
{
    if (const ColumnInfo* const ci = getInfoForId (columnId))
        return ci->name;

    return String::empty;
}

void TableHeaderComponent::setColumnName (const int columnId, const String& newName)
{
    if (ColumnInfo* const ci = getInfoForId (columnId))
    {
        if (ci->name != newName)
        {
            ci->name = newName;
            sendColumnsChanged();
        }
    }
}

void TableHeaderComponent::addColumn (const String& columnName,
                                      const int columnId,
                                      const int width,
                                      const int minimumWidth,
                                      const int maximumWidth,
                                      const int propertyFlags,
                                      const int insertIndex)
{
    // can't have a duplicate or null ID!
    jassert (columnId != 0 && getIndexOfColumnId (columnId, false) < 0);
    jassert (width > 0);

    ColumnInfo* const ci = new ColumnInfo();
    ci->name = columnName;
    ci->id = columnId;
    ci->width = width;
    ci->lastDeliberateWidth = width;
    ci->minimumWidth = minimumWidth;
    ci->maximumWidth = maximumWidth;
    if (ci->maximumWidth < 0)
        ci->maximumWidth = std::numeric_limits<int>::max();
    jassert (ci->maximumWidth >= ci->minimumWidth);
    ci->propertyFlags = propertyFlags;

    columns.insert (insertIndex, ci);
    sendColumnsChanged();
}

void TableHeaderComponent::removeColumn (const int columnIdToRemove)
{
    const int index = getIndexOfColumnId (columnIdToRemove, false);

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
    const int currentIndex = getIndexOfColumnId (columnId, false);
    newIndex = visibleIndexToTotalIndex (newIndex);

    if (columns [currentIndex] != 0 && currentIndex != newIndex)
    {
        columns.move (currentIndex, newIndex);
        sendColumnsChanged();
    }
}

int TableHeaderComponent::getColumnWidth (const int columnId) const
{
    if (const ColumnInfo* const ci = getInfoForId (columnId))
        return ci->width;

    return 0;
}

void TableHeaderComponent::setColumnWidth (const int columnId, const int newWidth)
{
    ColumnInfo* const ci = getInfoForId (columnId);

    if (ci != nullptr && ci->width != newWidth)
    {
        const int numColumns = getNumColumns (true);

        ci->lastDeliberateWidth = ci->width
            = jlimit (ci->minimumWidth, ci->maximumWidth, newWidth);

        if (stretchToFit)
        {
            const int index = getIndexOfColumnId (columnId, true) + 1;

            if (isPositiveAndBelow (index, numColumns))
            {
                const int x = getColumnPosition (index).getX();

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

//==============================================================================
int TableHeaderComponent::getIndexOfColumnId (const int columnId, const bool onlyCountVisibleColumns) const
{
    int n = 0;

    for (int i = 0; i < columns.size(); ++i)
    {
        if ((! onlyCountVisibleColumns) || columns.getUnchecked(i)->isVisible())
        {
            if (columns.getUnchecked(i)->id == columnId)
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

    if (const ColumnInfo* const ci = columns [index])
        return ci->id;

    return 0;
}

Rectangle<int> TableHeaderComponent::getColumnPosition (const int index) const
{
    int x = 0, width = 0, n = 0;

    for (int i = 0; i < columns.size(); ++i)
    {
        x += width;

        if (columns.getUnchecked(i)->isVisible())
        {
            width = columns.getUnchecked(i)->width;

            if (n++ == index)
                break;
        }
        else
        {
            width = 0;
        }
    }

    return Rectangle<int> (x, 0, width, getHeight());
}

int TableHeaderComponent::getColumnIdAtX (const int xToFind) const
{
    if (xToFind >= 0)
    {
        int x = 0;

        for (int i = 0; i < columns.size(); ++i)
        {
            const ColumnInfo* const ci = columns.getUnchecked(i);

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

    for (int i = columns.size(); --i >= 0;)
        if (columns.getUnchecked(i)->isVisible())
            w += columns.getUnchecked(i)->width;

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
        ColumnInfo* const ci = columns.getUnchecked(i);

        if (ci->isVisible())
            sor.addItem (ci->lastDeliberateWidth, ci->minimumWidth, ci->maximumWidth);
    }

    sor.resizeToFit (targetTotalWidth);

    int visIndex = 0;
    for (int i = firstColumnIndex; i < columns.size(); ++i)
    {
        ColumnInfo* const ci = columns.getUnchecked(i);

        if (ci->isVisible())
        {
            const int newWidth = jlimit (ci->minimumWidth, ci->maximumWidth,
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
    if (ColumnInfo* const ci = getInfoForId (columnId))
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
    const ColumnInfo* const ci = getInfoForId (columnId);
    return ci != nullptr && ci->isVisible();
}

//==============================================================================
void TableHeaderComponent::setSortColumnId (const int columnId, const bool sortForwards)
{
    if (getSortColumnId() != columnId || isSortedForwards() != sortForwards)
    {
        for (int i = columns.size(); --i >= 0;)
            columns.getUnchecked(i)->propertyFlags &= ~(sortedForwards | sortedBackwards);

        if (ColumnInfo* const ci = getInfoForId (columnId))
            ci->propertyFlags |= (sortForwards ? sortedForwards : sortedBackwards);

        reSortTable();
    }
}

int TableHeaderComponent::getSortColumnId() const
{
    for (int i = columns.size(); --i >= 0;)
        if ((columns.getUnchecked(i)->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return columns.getUnchecked(i)->id;

    return 0;
}

bool TableHeaderComponent::isSortedForwards() const
{
    for (int i = columns.size(); --i >= 0;)
        if ((columns.getUnchecked(i)->propertyFlags & (sortedForwards | sortedBackwards)) != 0)
            return (columns.getUnchecked(i)->propertyFlags & sortedForwards) != 0;

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

    for (int i = 0; i < columns.size(); ++i)
    {
        const ColumnInfo* const ci = columns.getUnchecked (i);

        XmlElement* const e = doc.createNewChildElement ("COLUMN");
        e->setAttribute ("id", ci->id);
        e->setAttribute ("visible", ci->isVisible());
        e->setAttribute ("width", ci->width);
    }

    return doc.createDocument ("", true, false);
}

void TableHeaderComponent::restoreFromString (const String& storedVersion)
{
    ScopedPointer<XmlElement> storedXml (XmlDocument::parse (storedVersion));
    int index = 0;

    if (storedXml != nullptr && storedXml->hasTagName ("TABLELAYOUT"))
    {
        forEachXmlChildElement (*storedXml, col)
        {
            const int tabId = col->getIntAttribute ("id");

            if (ColumnInfo* const ci = getInfoForId (tabId))
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
    if (const ColumnInfo* const ci = getInfoForId (columnId))
        if ((ci->propertyFlags & sortable) != 0 && ! mods.isPopupMenu())
            setSortColumnId (columnId, (ci->propertyFlags & sortedForwards) == 0);
}

void TableHeaderComponent::addMenuItems (PopupMenu& menu, const int /*columnIdClicked*/)
{
    for (int i = 0; i < columns.size(); ++i)
    {
        const ColumnInfo* const ci = columns.getUnchecked(i);

        if ((ci->propertyFlags & appearsOnColumnMenu) != 0)
            menu.addItem (ci->id, ci->name,
                          (ci->propertyFlags & (sortedForwards | sortedBackwards)) == 0,
                          isColumnVisible (ci->id));
    }
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
    for (int i = 0; i < columns.size(); ++i)
    {
        const ColumnInfo* const ci = columns.getUnchecked(i);

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

                lf.drawTableHeaderColumn (g, ci->name, ci->id, ci->width, getHeight(),
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

void TableHeaderComponent::resized()
{
}

void TableHeaderComponent::mouseMove (const MouseEvent& e)
{
    updateColumnUnderMouse (e);
}

void TableHeaderComponent::mouseEnter (const MouseEvent& e)
{
    updateColumnUnderMouse (e);
}

void TableHeaderComponent::mouseExit (const MouseEvent&)
{
    setColumnUnderMouse (0);
}

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
         && ! (e.mouseWasClicked() || e.mods.isPopupMenu()))
    {
        dragOverlayComp = nullptr;

        columnIdBeingResized = getResizeDraggerAt (e.getMouseDownX());

        if (columnIdBeingResized != 0)
        {
            const ColumnInfo* const ci = getInfoForId (columnIdBeingResized);
            initialColumnWidth = ci->width;
        }
        else
        {
            beginDrag (e);
        }
    }

    if (columnIdBeingResized != 0)
    {
        if (const ColumnInfo* const ci = getInfoForId (columnIdBeingResized))
        {
            int w = jlimit (ci->minimumWidth, ci->maximumWidth,
                            initialColumnWidth + e.getDistanceFromDragStartX());

            if (stretchToFit)
            {
                // prevent us dragging a column too far right if we're in stretch-to-fit mode
                int minWidthOnRight = 0;
                for (int i = getIndexOfColumnId (columnIdBeingResized, false) + 1; i < columns.size(); ++i)
                    if (columns.getUnchecked (i)->isVisible())
                        minWidthOnRight += columns.getUnchecked (i)->minimumWidth;

                const Rectangle<int> currentPos (getColumnPosition (getIndexOfColumnId (columnIdBeingResized, true)));
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
                        const ColumnInfo* const previous = columns.getUnchecked (newIndex - 1);

                        if ((previous->propertyFlags & draggable) != 0)
                        {
                            const int leftOfPrevious = getColumnPosition (newIndex - 1).getX();
                            const int rightOfCurrent = getColumnPosition (newIndex).getRight();

                            if (abs (dragOverlayComp->getX() - leftOfPrevious)
                                < abs (dragOverlayComp->getRight() - rightOfCurrent))
                            {
                                --newIndex;
                            }
                        }
                    }

                    if (newIndex < columns.size() - 1)
                    {
                        // if the next column isn't draggable, we can't move our column
                        // past it, because that'd change the undraggable column's position..
                        const ColumnInfo* const nextCol = columns.getUnchecked (newIndex + 1);

                        if ((nextCol->propertyFlags & draggable) != 0)
                        {
                            const int leftOfCurrent = getColumnPosition (newIndex).getX();
                            const int rightOfNext = getColumnPosition (newIndex + 1).getRight();

                            if (abs (dragOverlayComp->getX() - leftOfCurrent)
                                > abs (dragOverlayComp->getRight() - rightOfNext))
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

        const ColumnInfo* const ci = getInfoForId (columnIdBeingDragged);

        if (ci == nullptr || (ci->propertyFlags & draggable) == 0)
        {
            columnIdBeingDragged = 0;
        }
        else
        {
            draggingColumnOriginalIndex = getIndexOfColumnId (columnIdBeingDragged, true);

            const Rectangle<int> columnRect (getColumnPosition (draggingColumnOriginalIndex));

            const int temp = columnIdBeingDragged;
            columnIdBeingDragged = 0;

            addAndMakeVisible (dragOverlayComp = new DragOverlayComp (createComponentSnapshot (columnRect, false)));
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

    for (int i = columns.size(); --i >= 0;)
        if (columns.getUnchecked (i)->isVisible())
            columns.getUnchecked (i)->lastDeliberateWidth = columns.getUnchecked (i)->width;

    columnIdBeingResized = 0;
    repaint();

    endDrag (getIndexOfColumnId (columnIdBeingDragged, true));

    updateColumnUnderMouse (e);

    if (columnIdUnderMouse != 0 && e.mouseWasClicked() && ! e.mods.isPopupMenu())
        columnClicked (columnIdUnderMouse, e.mods);

    dragOverlayComp = nullptr;
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

TableHeaderComponent::ColumnInfo* TableHeaderComponent::getInfoForId (const int id) const
{
    for (int i = columns.size(); --i >= 0;)
        if (columns.getUnchecked(i)->id == id)
            return columns.getUnchecked(i);

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

        for (int i = 0; i < columns.size(); ++i)
        {
            const ColumnInfo* const ci = columns.getUnchecked(i);

            if (ci->isVisible())
            {
                if (abs (mouseX - (x + ci->width)) <= draggableDistance
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
