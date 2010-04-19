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

#ifndef __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
#define __JUCER_COMPONENTDOCUMENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"
#include "jucer_Coordinate.h"


//==============================================================================
class ComponentDocument   : public ValueTree::Listener
{
public:
    //==============================================================================
    ComponentDocument (Project* project, const File& cppFile);
    ~ComponentDocument();

    static bool isComponentFile (const File& file);

    bool save();
    bool reload();
    bool hasChangedSinceLastSave();

    typedef SelectedItemSet<uint32> SelectedItems;

    //==============================================================================
    Value getClassName()            { return getRootValueNonUndoable ("className"); }
    Value getClassDescription()     { return getRootValueNonUndoable ("classDesc"); }

    Value getCanvasWidth()          { return getRootValueNonUndoable ("width"); }
    Value getCanvasHeight()         { return getRootValueNonUndoable ("height"); }

    void createClassProperties (Array <PropertyComponent*>& props);

    const String getNonExistentMemberName (String suggestedName);

    //==============================================================================
    int getNumComponents() const;
    const ValueTree getComponent (int index) const;
    const ValueTree getComponentWithMemberName (const String& name) const;
    Component* createComponent (int index);
    void updateComponent (Component* comp);
    bool containsComponent (Component* comp) const;
    const ValueTree getComponentState (Component* comp) const;
    void getComponentProperties (Array <PropertyComponent*>& props, Component* comp);
    bool isStateForComponent (const ValueTree& storedState, Component* comp) const;
    Coordinate::MarkerResolver* createMarkerResolver (const ValueTree& state);
    const StringArray getComponentMarkers (bool horizontal) const;
    const RectangleCoordinates getCoordsFor (const ValueTree& componentState) const;
    bool setCoordsFor (ValueTree& componentState, const RectangleCoordinates& newSize);

    void addNewComponentMenuItems (PopupMenu& menu) const;
    void performNewComponentMenuItem (int menuResultCode);

    //==============================================================================
    void beginDrag (const Array<Component*>& items, const MouseEvent& e,
                    Component* parentForOverlays, const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    ValueTree& getRoot()                                { return root; }
    UndoManager* getUndoManager();
    void beginNewTransaction();

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property);
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged);
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

    static const char* const idProperty;
    static const char* const compBoundsProperty;
    static const char* const memberNameProperty;
    static const char* const compNameProperty;

private:
    Project* project;
    File cppFile;
    ValueTree root;
    UndoManager undoManager;
    bool changedSinceSaved;

    void checkRootObject();
    ValueTree getComponentGroup() const;

    Value getRootValueUndoable (const var::identifier& name)        { return root.getPropertyAsValue (name, getUndoManager()); }
    Value getRootValueNonUndoable (const var::identifier& name)     { return root.getPropertyAsValue (name, 0); }

    void writeCode (OutputStream& cpp, OutputStream& header);
    void writeMetadata (OutputStream& out);
};



#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
