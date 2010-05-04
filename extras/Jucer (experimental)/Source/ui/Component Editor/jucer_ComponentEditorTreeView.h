/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_COMPONENTEDITORTREEVIEW_H_F3B95A41__
#define __JUCE_COMPONENTEDITORTREEVIEW_H_F3B95A41__


//==============================================================================
namespace ComponentEditorTreeView
{
    //==============================================================================
    class Base  : public JucerTreeViewBase,
                  public ValueTree::Listener,
                  public ChangeListener
    {
    public:
        Base (ComponentEditor& editor_)
            : editor (editor_)
        {
            editor.getSelection().addChangeListener (this);
        }

        ~Base()
        {
            editor.getSelection().removeChangeListener (this);
        }

        //==============================================================================
        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)  {}
        void valueTreeParentChanged (ValueTree& tree)       {}
        void valueTreeChildrenChanged (ValueTree& tree)     {}

        const String getUniqueName() const
        {
            jassert (getItemId().isNotEmpty());
            return getItemId();
        }

        //==============================================================================
        void itemOpennessChanged (bool isNowOpen)
        {
            if (isNowOpen)
                refreshSubItems();
        }

        virtual void refreshSubItems() = 0;
        virtual const String getItemId() const = 0;

        void setName (const String& newName)            {}
        void itemClicked (const MouseEvent& e)          {}
        void itemDoubleClicked (const MouseEvent& e)    {}

        void itemSelectionChanged (bool isNowSelected)
        {
            if (isNowSelected)
                editor.getSelection().addToSelection (getItemId());
            else
                editor.getSelection().deselect (getItemId());
        }

        void changeListenerCallback (void*)             { updateSelectionState(); }

        void updateSelectionState()
        {
            setSelected (editor.getSelection().isSelected (getItemId()), false);
        }

        bool isMissing()                            { return false; }
        const String getTooltip()                   { return String::empty; }

    protected:
        ComponentEditor& editor;
    };

    static const String getDragIdFor (ComponentEditor& editor)
    {
        return componentItemDragType + editor.getDocument().getUniqueId();
    }

    //==============================================================================
    class ComponentItem  : public Base
    {
    public:
        ComponentItem (ComponentEditor& editor_, const ValueTree& componentState_)
            : Base (editor_), componentState (componentState_)
        {
            componentState.addListener (this);
            updateSelectionState();
        }

        ~ComponentItem()
        {
            componentState.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return componentState [ComponentDocument::idProperty]; }

        bool mightContainSubItems()                 { return false; }
        void refreshSubItems()                      {}

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return componentState [ComponentDocument::memberNameProperty]; }

        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }

        const String getDragSourceDescription()     { return getDragIdFor (editor); }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == ComponentDocument::memberNameProperty)
                repaintItem();
        }

        ValueTree componentState;
    };

    //==============================================================================
    class ComponentList  : public Base
    {
    public:
        ComponentList (ComponentEditor& editor_)
            : Base (editor_), componentTree (editor.getDocument().getComponentGroup())
        {
            componentTree.addListener (this);
        }

        ~ComponentList()
        {
            componentTree.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return "components"; }
        bool mightContainSubItems()                 { return true; }

        void valueTreeChildrenChanged (ValueTree& tree)
        {
            if (tree == componentTree)
                refreshSubItems();
        }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            ComponentDocument& doc = editor.getDocument();

            const int num = doc.getNumComponents();
            for (int i = 0; i < num; ++i)
                addSubItem (new ComponentItem (editor, doc.getComponent (i)));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return "Components"; }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }

        bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent)
        {
            return sourceDescription == getDragIdFor (editor)
                    && editor.getSelection().getNumSelected() > 0;
        }

        void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex)
        {
            if (editor.getSelection().getNumSelected() > 0)
            {
                TreeView* tree = getOwnerView();
                const ScopedPointer <XmlElement> openness (tree->getOpennessState (false));

                Array <ValueTree> selectedComps;
                // scan the source tree rather than look at the selection manager, because it might
                // be from a different editor, and the order needs to be correct.
                getAllSelectedNodesInTree (sourceComponent, selectedComps);
                insertItems (selectedComps, insertIndex);

                if (openness != 0)
                    tree->restoreOpennessState (*openness);
            }
        }

        static void getAllSelectedNodesInTree (Component* componentInTree, Array<ValueTree>& selectedComps)
        {
            TreeView* tree = dynamic_cast <TreeView*> (componentInTree);

            if (tree == 0)
                tree = componentInTree->findParentComponentOfClass ((TreeView*) 0);

            if (tree != 0)
            {
                const int numSelected = tree->getNumSelectedItems();

                for (int i = 0; i < numSelected; ++i)
                {
                    const ComponentItem* const item = dynamic_cast <ComponentItem*> (tree->getSelectedItem (i));

                    if (item != 0)
                        selectedComps.add (item->componentState);
                }
            }
        }

        void insertItems (Array <ValueTree>& comps, int insertIndex)
        {
            int i;
            for (i = comps.size(); --i >= 0;)
                if (componentTree == comps.getReference(i) || componentTree.isAChildOf (comps.getReference(i))) // Check for recursion.
                    return;

            // Don't include any nodes that are children of other selected nodes..
            for (i = comps.size(); --i >= 0;)
            {
                const ValueTree& n = comps.getReference(i);

                for (int j = comps.size(); --j >= 0;)
                {
                    if (j != i && n.isAChildOf (comps.getReference(j)))
                    {
                        comps.remove (i);
                        break;
                    }
                }
            }

            // Remove and re-insert them one at a time..
            for (i = 0; i < comps.size(); ++i)
            {
                ValueTree& n = comps.getReference(i);

                if (n.getParent() == componentTree && componentTree.indexOf (n) < insertIndex)
                    --insertIndex;

                if (n.getParent() == componentTree)
                {
                    n.getParent().moveChild (componentTree.indexOf (n), insertIndex++, editor.getDocument().getUndoManager());
                }
                else
                {
                    n.getParent().removeChild (n, editor.getDocument().getUndoManager());
                    componentTree.addChild (n, insertIndex++, editor.getDocument().getUndoManager());
                }
            }
        }

    private:
        ValueTree componentTree;
    };


    //==============================================================================
    class MarkerItem  : public Base
    {
    public:
        MarkerItem (ComponentEditor& editor_, const ValueTree& markerState_)
            : Base (editor_), markerState (markerState_)
        {
            markerState.addListener (this);
            updateSelectionState();
        }

        ~MarkerItem()
        {
            markerState.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return MarkerListBase::getId (markerState); }

        bool mightContainSubItems()                 { return false; }
        void refreshSubItems()                      {}

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return markerState [MarkerListBase::getMarkerNameProperty()]; }

        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }

        const String getDragSourceDescription()     { return String::empty; }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == MarkerListBase::getMarkerNameProperty())
                repaintItem();
        }

    private:
        ValueTree markerState;
    };

    //==============================================================================
    class MarkerList  : public Base
    {
    public:
        MarkerList (ComponentEditor& editor_, bool isX_)
            : Base (editor_), markerList (editor_.getDocument().getMarkerList (isX_).getGroup()), isX (isX_)
        {
            markerList.addListener (this);
        }

        ~MarkerList()
        {
            markerList.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return isX ? "markersX" : "markersY"; }
        bool mightContainSubItems()                 { return true; }

        void valueTreeChildrenChanged (ValueTree& tree)
        {
            refreshSubItems();
        }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            ComponentDocument::MarkerList& markers = editor.getDocument().getMarkerList (isX);

            const int num = markers.size();
            for (int i = 0; i < num; ++i)
                addSubItem (new MarkerItem (editor, markers.getMarker (i)));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return isX ? "Markers (X-axis)" : "Markers (Y-axis)"; }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }

    private:
        ValueTree markerList;
        bool isX;
    };

    //==============================================================================
    class Root  : public Base
    {
    public:
        Root (ComponentEditor& editor_)  : Base (editor_)  {}
        ~Root()    {}

        //==============================================================================
        const String getItemId() const              { return "root"; }
        bool mightContainSubItems()                 { return true; }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            addSubItem (new ComponentList (editor));
            addSubItem (new MarkerList (editor, true));
            addSubItem (new MarkerList (editor, false));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return editor.getDocument().getClassName().toString(); }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }
    };
}

#endif  // __JUCE_COMPONENTEDITORTREEVIEW_H_F3B95A41__
