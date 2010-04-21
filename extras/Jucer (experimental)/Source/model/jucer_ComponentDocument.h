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
class ComponentDocument   : public ValueTree::Listener,
                            public Coordinate::MarkerResolver
{
public:
    //==============================================================================
    ComponentDocument (Project* project, const File& cppFile);
    ~ComponentDocument();

    static bool isComponentFile (const File& file);

    bool save();
    bool reload();
    bool hasChangedSinceLastSave();

    //==============================================================================
    Value getClassName() const              { return getRootValueNonUndoable ("className"); }
    Value getClassDescription() const       { return getRootValueNonUndoable ("classDesc"); }

    Value getCanvasWidth() const            { return getRootValueNonUndoable ("width"); }
    Value getCanvasHeight() const           { return getRootValueNonUndoable ("height"); }

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
    void removeComponent (const ValueTree& state);
    const RectangleCoordinates getCoordsFor (const ValueTree& componentState) const;
    bool setCoordsFor (ValueTree& componentState, const RectangleCoordinates& newSize);

    // for Coordinate::MarkerResolver:
    const Coordinate findMarker (const String& name, bool isHorizontal) const;

    void getComponentMarkerMenuItems (const ValueTree& componentState, const String& coordName,
                                      Coordinate& coord, PopupMenu& menu, bool isAnchor1);
    const String getChosenMarkerMenuItem (const ValueTree& componentState, Coordinate& coord, int itemId) const;

    void addNewComponentMenuItems (PopupMenu& menu) const;
    void performNewComponentMenuItem (int menuResultCode);

    //==============================================================================
    class MarkerList    : public Coordinate::MarkerResolver
    {
    public:
        MarkerList (ComponentDocument& document, bool isX);

        ValueTree& getGroup();
        int size() const;
        ValueTree getMarker (int index) const;
        ValueTree getMarkerNamed (const String& name) const;
        bool contains (const ValueTree& markerState) const;
        const Coordinate getCoordinate (const ValueTree& markerState) const;
        const String getName (const ValueTree& markerState) const;
        Value getNameAsValue (const ValueTree& markerState) const;
        void setCoordinate (ValueTree& markerState, const Coordinate& newCoord);
        void createMarker (const String& name, int position);
        void deleteMarker (ValueTree& markerState);

        // for Coordinate::MarkerResolver:
        const Coordinate findMarker (const String& name, bool isHorizontal) const;

    private:
        ComponentDocument& document;
        ValueTree group;
        const bool isX;

        MarkerList (const MarkerList&);
        MarkerList& operator= (const MarkerList&);
    };

    MarkerList& getMarkerListX() const            { return *markersX; }
    MarkerList& getMarkerListY() const            { return *markersY; }
    MarkerList& getMarkerList (bool isX) const    { return isX ? *markersX : *markersY; }

    const String getNonexistentMarkerName (const String& name);

    //==============================================================================
    void beginDrag (const Array<Component*>& items, const MouseEvent& e,
                    Component* parentForOverlays, const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    ValueTree& getRoot()                                { return root; }
    UndoManager* getUndoManager() const;
    void beginNewTransaction();

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property);
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged);
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

    static const char* const idProperty;
    static const char* const compBoundsProperty;
    static const char* const memberNameProperty;
    static const char* const compNameProperty;
    static const char* const markerNameProperty;
    static const char* const markerPosProperty;

private:
    Project* project;
    File cppFile;
    ValueTree root;
    ScopedPointer<MarkerList> markersX, markersY;
    mutable UndoManager undoManager;
    bool changedSinceSaved;

    void checkRootObject();
    void createSubTreeIfNotThere (const String& name);
    ValueTree getComponentGroup() const;
    void addMarkerMenuItem (int i, Coordinate& coord, const String& name, PopupMenu& menu, bool isAnchor1,
                            const ValueTree& componentState, const String& coordName);

    Value getRootValueUndoable (const var::identifier& name) const        { return root.getPropertyAsValue (name, getUndoManager()); }
    Value getRootValueNonUndoable (const var::identifier& name) const     { return root.getPropertyAsValue (name, 0); }

    void writeCode (OutputStream& cpp, OutputStream& header);
    void writeMetadata (OutputStream& out);
};



#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
