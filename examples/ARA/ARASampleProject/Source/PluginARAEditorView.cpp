/*
  ==============================================================================

    PluginARAEditorView.cpp
    Created: 2 Nov 2018 3:08:37pm
    Author:  john

  ==============================================================================
*/

#include "PluginARAEditorView.h"

ARASampleProjectEditorView::SelectionListener::SelectionListener (ARA::PlugIn::EditorView * editorView)
: araEditorView (static_cast<ARASampleProjectEditorView*> (editorView))
{
    if (araEditorView)
        araEditorView->addSelectionListener (this);
}

ARASampleProjectEditorView::SelectionListener::~SelectionListener ()
{
    if (araEditorView)
        araEditorView->removeSelectionListener (this);
}

ARASampleProjectEditorView::ARASampleProjectEditorView (ARA::PlugIn::DocumentController* ctrl) noexcept
: ARA::PlugIn::EditorView (ctrl)
{}

void ARASampleProjectEditorView::doNotifySelection (const ARA::PlugIn::ViewSelection* currentSelection) noexcept
{
    mostRecentSelection = *currentSelection;
    for (SelectionListener* listener : selectionChangeListeners)
        listener->onNewSelection (currentSelection);
}

const ARA::PlugIn::ViewSelection * ARASampleProjectEditorView::getMostRecentSelection () const 
{ 
    return &mostRecentSelection; 
}

void ARASampleProjectEditorView::addSelectionListener (SelectionListener * l)
{ 
    selectionChangeListeners.push_back (l); 
}

void ARASampleProjectEditorView::removeSelectionListener (SelectionListener * l) 
{
    find_erase (selectionChangeListeners, l); 
}
