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
#include "../../utility/jucer_Coordinate.h"
#include "../../utility/jucer_MarkerListBase.h"
#include "jucer_CodeGenerator.h"


//==============================================================================
class ComponentDocument   : public ValueTree::Listener,
                            public Coordinate::MarkerResolver
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

    Value getClassName() const              { return getRootValueNonUndoable ("className"); }
    Value getClassDescription() const       { return getRootValueNonUndoable ("classDesc"); }

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
    const RectangleCoordinates getCoordsFor (const ValueTree& componentState) const;
    bool setCoordsFor (ValueTree& componentState, const RectangleCoordinates& newSize);
    void renameAnchor (const String& oldName, const String& newName);

    // for Coordinate::MarkerResolver:
    const Coordinate findMarker (const String& name, bool isHorizontal) const;

    void addComponentMarkerMenuItems (const ValueTree& componentState, const String& coordName,
                                      Coordinate& coord, PopupMenu& menu, bool isAnchor1);
    const String getChosenMarkerMenuItem (const ValueTree& componentState, Coordinate& coord, int itemId) const;

    void addNewComponentMenuItems (PopupMenu& menu) const;
    const ValueTree performNewComponentMenuItem (int menuResultCode);

    void updateComponentsIn (Component* compHolder);

    //==============================================================================
    class MarkerList    : public MarkerListBase
    {
    public:
        MarkerList (ComponentDocument& document, bool isX);

        const Coordinate findMarker (const String& name, bool isHorizontal) const;
        bool createProperties (Array <PropertyComponent*>& props, const String& itemId);
        void addMarkerMenuItems (const ValueTree& markerState, const Coordinate& coord, PopupMenu& menu, bool isAnchor1);
        const String getChosenMarkerMenuItem (const Coordinate& coord, int itemId) const;
        UndoManager* getUndoManager() const;
        void renameAnchor (const String& oldName, const String& newName);
        const String getNonexistentMarkerName (const String& name);

        ComponentDocument& getDocument() throw()    { return document; }

    private:
        ComponentDocument& document;

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
    class TestComponent     : public Component
    {
    public:
        TestComponent (ComponentDocument& document);
        ~TestComponent();

        void paint (Graphics& g);

    private:
        ScopedPointer<ComponentAutoLayoutManager> layoutManager;
        Colour background;
    };

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
    void addMarkerMenuItem (int i, const Coordinate& coord, const String& name, PopupMenu& menu,
                            bool isAnchor1, const String& fullCoordName);

    Value getRootValueUndoable (const Identifier& name) const        { return root.getPropertyAsValue (name, getUndoManager()); }
    Value getRootValueNonUndoable (const Identifier& name) const     { return root.getPropertyAsValue (name, 0); }

    void writeCode (OutputStream& cpp, OutputStream& header);
    void writeMetadata (OutputStream& out);

    bool createItemProperties (Array <PropertyComponent*>& props, const String& itemId);

    Component* findComponentForState (Component* compHolder, const ValueTree& state);
};



#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
