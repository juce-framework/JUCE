/*
  ==============================================================================

    PluginARAEditorView.h
    Created: 2 Nov 2018 3:08:37pm
    Author:  john

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

/** Naive Editor class that visualize current ARA Document RegionSequences state */
class ARASampleProjectEditorView : public ARA::PlugIn::EditorView
{
public:

    /** Listener class that can be used to get selection notification updates */
    class SelectionListener
    {
        ARASampleProjectEditorView* araEditorView;
    public:
        SelectionListener (ARA::PlugIn::EditorView* editorView);
        virtual ~SelectionListener ();
        const ARA::PlugIn::ViewSelection* getMostRecentSelection () const;

        // will be called from ARASampleProjectEditorView::doNotifySelection
        virtual void onNewSelection (const ARA::PlugIn::ViewSelection* currentSelection) = 0;
    };

    //==============================================================================

    ARASampleProjectEditorView (ARA::PlugIn::DocumentController*) noexcept;
    void doNotifySelection (const ARA::PlugIn::ViewSelection*) noexcept override;

    const ARA::PlugIn::ViewSelection* getMostRecentSelection () const;

    void addSelectionListener (SelectionListener* l);
    void removeSelectionListener (SelectionListener* l);

private:

    ARA::PlugIn::ViewSelection mostRecentSelection;
    std::vector<SelectionListener*> selectionChangeListeners;
};
