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

#ifndef __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
#define __JUCER_COMPONENTDOCUMENT_JUCEHEADER__

#include "../../jucer_Headers.h"
#include "../Project/jucer_Project.h"
#include "../../utility/jucer_MarkerListBase.h"
#include "jucer_CodeGenerator.h"


//==============================================================================
class ComponentDocument   : public ValueTree::Listener,
                            public RelativeCoordinate::NamedCoordinateFinder
{
public:
    //==============================================================================
    ComponentDocument (Project* project, const File& cppFile);
    ComponentDocument (const ComponentDocument& other);
    ~ComponentDocument();

    static bool isComponentFile (const File& file);

    bool save();
    bool reload();
    bool hasChangedSinceLastSave();
    void changed();

    Project* getProject() const                     { return project; }
    const File getCppFile() const                   { return cppFile; }
    void cppFileHasMoved (const File& newFile)      { cppFile = newFile; }

    //==============================================================================
    const String getUniqueId() const        { return root [idProperty]; }

    Value getClassName() const              { return getRootValueNonUndoable (Ids::className); }
    Value getClassDescription() const       { return getRootValueNonUndoable (Ids::classDesc); }

    void setUsingTemporaryCanvasSize (bool b);
    Value getCanvasWidth() const;
    Value getCanvasHeight() const;
    Value getBackgroundColour() const;

    void createClassProperties (Array <PropertyComponent*>& props);

    const String getNonexistentMemberName (String suggestedName);

    //==============================================================================
    int getNumComponents() const;
    const ValueTree getComponent (int index) const;
    const ValueTree getComponentWithMemberName (const String& name) const;
    const ValueTree getComponentWithID (const String& uid) const;
    Component* createComponent (int index);
    void updateComponent (Component* comp);
    bool containsComponent (Component* comp) const;
    const ValueTree getComponentState (Component* comp) const;
    bool isStateForComponent (const ValueTree& storedState, Component* comp) const;
    void removeComponent (const ValueTree& state);
    const RelativeRectangle getCoordsFor (const ValueTree& componentState) const;
    void setCoordsFor (ValueTree& componentState, const RelativeRectangle& newSize);
    void renameAnchor (const String& oldName, const String& newName);

    // for RelativeCoordinate::Resolver:
    const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;

    void addComponentMarkerMenuItems (const ValueTree& componentState, const String& coordName,
                                      RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1, bool isHorizontal);
    const String getChosenMarkerMenuItem (const ValueTree& componentState, RelativeCoordinate& coord, int itemId, bool isHorizontal) const;

    void addNewComponentMenuItems (PopupMenu& menu) const;
    const ValueTree performNewComponentMenuItem (int menuResultCode);

    void componentDoubleClicked (const MouseEvent& e, const ValueTree& state);

    void updateComponentsIn (Component* compHolder);
    Component* findComponentForState (Component* parentComp, const ValueTree& state);

    //==============================================================================
    class MarkerList    : public MarkerListBase
    {
    public:
        MarkerList (ComponentDocument& document, bool isX);

        const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;
        bool createProperties (Array <PropertyComponent*>& props, const String& itemId);
        void addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1);
        const String getChosenMarkerMenuItem (const RelativeCoordinate& coord, int itemId) const;
        UndoManager* getUndoManager() const;
        void renameAnchor (const String& oldName, const String& newName);
        const String getNonexistentMarkerName (const String& name);
        const String getId (const ValueTree& markerState)                   { return markerState [Ids::id_]; }
        int size() const                                                    { return group.getNumChildren(); }
        ValueTree getMarker (int index) const                               { return group.getChild (index); }
        ValueTree getMarkerNamed (const String& name) const                 { return group.getChildWithProperty (getMarkerNameProperty(), name); }
        bool contains (const ValueTree& markerState) const                  { return markerState.isAChildOf (group); }
        void createMarker (const String& name, int position);
        void deleteMarker (ValueTree& markerState);

        ComponentDocument& getDocument() throw()    { return document; }
        ValueTree& getGroup()                       { return group; }

    private:
        ComponentDocument& document;
        ValueTree group;

        MarkerList (const MarkerList&);
        MarkerList& operator= (const MarkerList&);
    };

    MarkerList& getMarkerListX() const            { return *markersX; }
    MarkerList& getMarkerListY() const            { return *markersY; }
    MarkerList& getMarkerList (bool isX) const    { return isX ? *markersX : *markersY; }

    //==============================================================================
    void createItemProperties (Array <PropertyComponent*>& props, const StringArray& selectedItemIds);

    //==============================================================================
    void beginDrag (const Array<Component*>& items, const MouseEvent& e,
                    Component* parentForOverlays, const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    CodeGenerator::CustomCodeList& getCustomCodeList() throw()         { return customCode; }

    const String getCppTemplate() const;
    const String getHeaderTemplate() const;

    const String getCppContent();
    const String getHeaderContent();

    //==============================================================================
    ValueTree& getRoot()                                        { return root; }
    ValueTree getComponentGroup() const;

    UndoManager* getUndoManager() const;
    void beginNewTransaction();

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property);
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged);
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

    static const Identifier idProperty;
    static const Identifier compBoundsProperty;
    static const Identifier memberNameProperty;
    static const Identifier compNameProperty;
    static const Identifier compTooltipProperty;
    static const Identifier compFocusOrderProperty;

    static const Identifier jucerIDProperty;
    static const String getJucerIDFor (Component* c);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    Project* project;
    File cppFile;
    ValueTree root;
    ScopedPointer<MarkerList> markersX, markersY;
    CodeGenerator::CustomCodeList customCode;
    mutable UndoManager undoManager;
    bool changedSinceSaved, usingTemporaryCanvasSize;
    Value tempCanvasWidth, tempCanvasHeight;

    void checkRootObject();
    void createSubTreeIfNotThere (const Identifier& name);
    void addMarkerMenuItem (int i, const RelativeCoordinate& coord, const String& objectName, const String& edge,
                            PopupMenu& menu, bool isAnchor1, const String& fullCoordName);

    Value getRootValueUndoable (const Identifier& name) const        { return root.getPropertyAsValue (name, getUndoManager()); }
    Value getRootValueNonUndoable (const Identifier& name) const     { return root.getPropertyAsValue (name, 0); }

    void writeCode (OutputStream& cpp, OutputStream& header);
    void writeMetadata (OutputStream& out);

    bool createItemProperties (Array <PropertyComponent*>& props, const String& itemId);
};



#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
