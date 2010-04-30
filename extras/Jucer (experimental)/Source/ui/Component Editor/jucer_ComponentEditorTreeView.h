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

#include "../jucer_JucerTreeViewBase.h"


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
            editor.getCanvas()->getSelection().addChangeListener (this);
        }

        ~Base()
        {
            editor.getCanvas()->getSelection().removeChangeListener (this);
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
                editor.getCanvas()->getSelection().addToSelection (getItemId());
            else
                editor.getCanvas()->getSelection().deselect (getItemId());
        }

        void changeListenerCallback (void*)             { updateSelectionState(); }

        void updateSelectionState()
        {
            setSelected (editor.getCanvas()->getSelection().isSelected (getItemId()), false);
        }

        bool isMissing()                            { return false; }
        const String getTooltip()                   { return String::empty; }

    protected:
        ComponentEditor& editor;
    };


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

        const String getDragSourceDescription()     { return componentItemDragType; }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == ComponentDocument::memberNameProperty)
                repaintItem();
        }

    private:
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
        const String getItemId() const              { return markerState [ComponentDocument::idProperty]; }

        bool mightContainSubItems()                 { return false; }
        void refreshSubItems()                      {}

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return markerState [ComponentDocument::markerNameProperty]; }

        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }

        const String getDragSourceDescription()     { return componentItemDragType; }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == ComponentDocument::markerNameProperty)
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
