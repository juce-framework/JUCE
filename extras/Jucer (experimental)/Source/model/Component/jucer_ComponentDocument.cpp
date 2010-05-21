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

#include "jucer_ComponentDocument.h"
#include "Types/jucer_ComponentTypeManager.h"
#include "../../utility/jucer_CoordinatePropertyComponent.h"


//==============================================================================
static const Identifier componentDocumentTag ("COMPONENT");
static const Identifier componentGroupTag ("COMPONENTS");
static const Identifier markersGroupXTag ("MARKERS_X");
static const Identifier markersGroupYTag ("MARKERS_Y");

static const char* const metadataTagStart = "JUCER_" "COMPONENT_METADATA_START"; // written like this to avoid thinking this file is a component!
static const char* const metadataTagEnd = "JUCER_" "COMPONENT_METADATA_END";

const Identifier ComponentDocument::idProperty ("id");
const Identifier ComponentDocument::compBoundsProperty ("position");
const Identifier ComponentDocument::memberNameProperty ("memberName");
const Identifier ComponentDocument::compNameProperty ("name");
const Identifier ComponentDocument::compTooltipProperty ("tooltip");
const Identifier ComponentDocument::compFocusOrderProperty ("focusOrder");
const Identifier ComponentDocument::jucerIDProperty ("jucerID");


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_),
     cppFile (cppFile_),
     root (componentDocumentTag),
     changedSinceSaved (false),
     usingTemporaryCanvasSize (false)
{
    checkRootObject();

    root.addListener (this);
}

ComponentDocument::ComponentDocument (const ComponentDocument& other)
   : project (other.project),
     cppFile (other.cppFile),
     root (other.root),
     changedSinceSaved (false)
{
    checkRootObject();

    root.addListener (this);
}

ComponentDocument::~ComponentDocument()
{
    root.removeListener (this);
}

void ComponentDocument::beginNewTransaction()
{
    undoManager.beginNewTransaction();
}

void ComponentDocument::changed()
{
    changedSinceSaved = true;
}

void ComponentDocument::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property)
{
    changed();
}

void ComponentDocument::valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)
{
    changed();
}

void ComponentDocument::valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)
{
    changed();
}

bool ComponentDocument::isComponentFile (const File& file)
{
    if (! file.hasFileExtension (".cpp"))
        return false;

    InputStream* in = file.createInputStream();

    if (in != 0)
    {
        BufferedInputStream buf (in, 8192, true);

        while (! buf.isExhausted())
            if (buf.readNextLine().contains (metadataTagStart))
                return true;
    }

    return false;
}

const String ComponentDocument::getCppTemplate() const      { return String (BinaryData::jucer_ComponentTemplate_cpp); }
const String ComponentDocument::getHeaderTemplate() const   { return String (BinaryData::jucer_ComponentTemplate_h); }

const String ComponentDocument::getCppContent()
{
    MemoryOutputStream cpp, header;
    writeCode (cpp, header);
    return cpp.toUTF8();
}

const String ComponentDocument::getHeaderContent()
{
    MemoryOutputStream cpp, header;
    writeCode (cpp, header);
    return header.toUTF8();
}

void ComponentDocument::writeCode (OutputStream& cpp, OutputStream& header)
{
    CodeGenerator codeGen;

    codeGen.className = getClassName().toString();
    codeGen.parentClasses = "public Component";

    {
        MemoryOutputStream stateStream (1024, 1024, &codeGen.componentStateData);
        root.writeToStream (stateStream);
    }

    for (int i = 0; i < getNumComponents(); ++i)
    {
        ComponentTypeInstance item (*this, getComponent (i));
        item.createCode (codeGen);
    }

    {
        MemoryOutputStream metaData;
        writeMetadata (metaData);
        codeGen.jucerMetadata = metaData.toUTF8();
    }

    {
        String code (getCppTemplate());
        String oldContent;

        codeGen.applyToCode (code, cppFile, false, project);
        customCode.applyTo (code);
        cpp << code;
    }

    {
        String code (getHeaderTemplate());
        String oldContent;

        codeGen.applyToCode (code, cppFile.withFileExtension (".h"), false, project);
        customCode.applyTo (code);
        header << code;
    }
}

void ComponentDocument::writeMetadata (OutputStream& out)
{
    out << metadataTagStart << newLine << newLine;

    ScopedPointer<XmlElement> xml (root.createXml());
    jassert (xml != 0);

    if (xml != 0)
        xml->writeToStream (out, String::empty, false, false);

    out << newLine << metadataTagEnd;
}

bool ComponentDocument::save()
{
    MemoryOutputStream cpp, header;
    writeCode (cpp, header);

    bool savedOk = FileHelpers::overwriteFileWithNewDataIfDifferent (cppFile, cpp)
                    && FileHelpers::overwriteFileWithNewDataIfDifferent (cppFile.withFileExtension (".h"), header);

    if (savedOk)
        changedSinceSaved = false;

    return savedOk;
}

bool ComponentDocument::reload()
{
    String xmlString;
    bool hadMetaDataTags = false;

    {
        InputStream* in = cppFile.createInputStream();

        if (in == 0)
            return false;

        BufferedInputStream buf (in, 8192, true);
        String::Concatenator xml (xmlString);

        while (! buf.isExhausted())
        {
            String line (buf.readNextLine());

            if (line.contains (metadataTagStart))
            {
                while (! buf.isExhausted())
                {
                    line = buf.readNextLine();
                    if (line.contains (metadataTagEnd))
                    {
                        hadMetaDataTags = true;
                        break;
                    }

                    xml.append (line);
                    xml.append (newLine);
                }

                break;
            }
        }
    }

    XmlDocument doc (xmlString);
    ScopedPointer<XmlElement> xml (doc.getDocumentElement());

    if (xml == 0 && hadMetaDataTags)
        xml = new XmlElement (componentDocumentTag.toString());

    if (xml != 0 && xml->hasTagName (componentDocumentTag.toString()))
    {
        ValueTree newTree (ValueTree::fromXml (*xml));

        if (newTree.isValid())
        {
            root = newTree;
            markersX = 0;
            markersY = 0;
            checkRootObject();
            customCode.reloadFrom (cppFile.loadFileAsString());

            root.addChild (ValueTree ("dummy"), 0, 0);
            root.removeChild (root.getChildWithName("dummy"), 0);

            undoManager.clearUndoHistory();
            changedSinceSaved = false;
            return true;
        }
    }

    return false;
}

bool ComponentDocument::hasChangedSinceLastSave()
{
    return changedSinceSaved || customCode.needsSaving();
}

void ComponentDocument::createSubTreeIfNotThere (const Identifier& name)
{
    if (! root.getChildWithName (name).isValid())
        root.addChild (ValueTree (name), -1, 0);
}

void ComponentDocument::checkRootObject()
{
    jassert (root.hasType (componentDocumentTag));

    if (root [idProperty].toString().isEmpty())
        root.setProperty (idProperty, createAlphaNumericUID(), 0);

    createSubTreeIfNotThere (componentGroupTag);
    createSubTreeIfNotThere (markersGroupXTag);
    createSubTreeIfNotThere (markersGroupYTag);

    if (markersX == 0)
        markersX = new MarkerList (*this, true);

    if (markersY == 0)
        markersY = new MarkerList (*this, false);

    if (getClassName().toString().isEmpty())
        getClassName() = "NewComponent";

    if ((int) getCanvasWidth().getValue() <= 0)
        getCanvasWidth() = 640;

    if ((int) getCanvasHeight().getValue() <= 0)
        getCanvasHeight() = 480;

    if (! root.hasProperty (Ids::background))
        getBackgroundColour() = Colours::white.toString();
}

void ComponentDocument::setUsingTemporaryCanvasSize (bool b)
{
    tempCanvasWidth = root.getProperty (Ids::width);
    tempCanvasHeight = root.getProperty (Ids::height);
    usingTemporaryCanvasSize = b;
}

Value ComponentDocument::getCanvasWidth() const
{
    return usingTemporaryCanvasSize ? tempCanvasWidth : getRootValueNonUndoable (Ids::width);
}

Value ComponentDocument::getCanvasHeight() const
{
    return usingTemporaryCanvasSize ? tempCanvasHeight : getRootValueNonUndoable (Ids::height);
}

Value ComponentDocument::getBackgroundColour() const
{
    return getRootValueUndoable (Ids::background);
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void ComponentDocument::addNewComponentMenuItems (PopupMenu& menu) const
{
    const StringArray displayNames (ComponentTypeManager::getInstance()->getDisplayNames());

    for (int i = 0; i < displayNames.size(); ++i)
        menu.addItem (i + menuItemOffset, "New " + displayNames[i]);
}

const ValueTree ComponentDocument::performNewComponentMenuItem (int menuResultCode)
{
    const StringArray displayNames (ComponentTypeManager::getInstance()->getDisplayNames());

    if (menuResultCode >= menuItemOffset && menuResultCode < menuItemOffset + displayNames.size())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandler (menuResultCode - menuItemOffset);
        jassert (handler != 0);

        if (handler != 0)
        {
            ValueTree state (handler->getValueTreeType());
            state.setProperty (idProperty, createAlphaNumericUID(), 0);

            ComponentTypeInstance comp (*this, state);
            handler->initialiseNewItem (comp);

            getComponentGroup().addChild (state, -1, getUndoManager());

            return state;
        }
    }

    return ValueTree::invalid;
}

void ComponentDocument::componentDoubleClicked (const MouseEvent& e, const ValueTree& state)
{
    ComponentTypeInstance item (*this, state);
    item.getHandler()->itemDoubleClicked (e, item);
}

void ComponentDocument::updateComponentsIn (Component* compHolder)
{
    int i;
    for (i = compHolder->getNumChildComponents(); --i >= 0;)
    {
        Component* c = compHolder->getChildComponent (i);

        if (! containsComponent (c))
            delete c;
    }

    Array <Component*> componentsInOrder;

    const int num = getNumComponents();
    for (i = 0; i < num; ++i)
    {
        const ValueTree v (getComponent (i));
        Component* c = findComponentForState (compHolder, v);

        if (c == 0)
            compHolder->addAndMakeVisible (c = createComponent (i));
        else
            updateComponent (c);

        componentsInOrder.add (c);
    }

    // Make sure the z-order is correct..
    if (num > 0)
    {
        componentsInOrder.getLast()->toFront (false);

        for (i = num - 1; --i >= 0;)
            componentsInOrder.getUnchecked(i)->toBehind (componentsInOrder.getUnchecked (i + 1));
    }
}

//==============================================================================
ValueTree ComponentDocument::getComponentGroup() const
{
    return root.getChildWithName (componentGroupTag);
}

int ComponentDocument::getNumComponents() const
{
    return getComponentGroup().getNumChildren();
}

const ValueTree ComponentDocument::getComponent (int index) const
{
    return getComponentGroup().getChild (index);
}

const ValueTree ComponentDocument::getComponentWithMemberName (const String& name) const
{
    return getComponentGroup().getChildWithProperty (memberNameProperty, name);
}

const ValueTree ComponentDocument::getComponentWithID (const String& uid) const
{
    return getComponentGroup().getChildWithProperty (idProperty, uid);
}

Component* ComponentDocument::createComponent (int index)
{
    const ValueTree v (getComponentGroup().getChild (index));

    if (v.isValid())
    {
        Component* c = ComponentTypeManager::getInstance()->createFromStoredType (*this, v);
        c->getProperties().set (jucerIDProperty, v[idProperty]);
        jassert (getJucerIDFor (c).isNotEmpty());
        return c;
    }

    return 0;
}

//==============================================================================
const RelativeCoordinate ComponentDocument::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == RelativeCoordinate::Strings::parent)
    {
        if (edge == RelativeCoordinate::Strings::right)     return RelativeCoordinate ((double) getCanvasWidth().getValue(), true);
        if (edge == RelativeCoordinate::Strings::bottom)    return RelativeCoordinate ((double) getCanvasHeight().getValue(), false);
    }

    if (objectName.isNotEmpty() && edge.isNotEmpty())
    {
        const ValueTree comp (getComponentWithMemberName (objectName));

        if (comp.isValid())
        {
            const RelativeRectangle coords (getCoordsFor (comp));

            if (edge == RelativeCoordinate::Strings::left)   return coords.left;
            if (edge == RelativeCoordinate::Strings::right)  return coords.right;
            if (edge == RelativeCoordinate::Strings::top)    return coords.top;
            if (edge == RelativeCoordinate::Strings::bottom) return coords.bottom;
        }
    }

    {
        const ValueTree marker (getMarkerListX().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListX().getCoordinate (marker);
    }

    {
        const ValueTree marker (getMarkerListY().getMarkerNamed (objectName));
        if (marker.isValid())
            return getMarkerListY().getCoordinate (marker);
    }

    return RelativeCoordinate();
}

const RelativeRectangle ComponentDocument::getCoordsFor (const ValueTree& state) const
{
    return RelativeRectangle (state [compBoundsProperty]);
}

bool ComponentDocument::setCoordsFor (ValueTree& state, const RelativeRectangle& pr)
{
    const String newBoundsString (pr.toString());

    if (state[compBoundsProperty] == newBoundsString)
        return false;

    state.setProperty (compBoundsProperty, newBoundsString, getUndoManager());
    return true;
}

const String ComponentDocument::getNonexistentMemberName (String name)
{
    String n (CodeHelpers::makeValidIdentifier (name, false, true, false));
    int suffix = 2;

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid()
            || getComponentWithMemberName (n).isValid())
        n = n.trimCharactersAtEnd ("0123456789") + String (suffix++);

    return n;
}

void ComponentDocument::renameAnchor (const String& oldName, const String& newName)
{
    int i;
    for (i = getNumComponents(); --i >= 0;)
    {
        ValueTree v (getComponent(i));
        RelativeRectangle coords (getCoordsFor (v));
        coords.renameAnchorIfUsed (oldName, newName, this);
        setCoordsFor (v, coords);
    }

    markersX->renameAnchorInMarkers (oldName, newName);
    markersY->renameAnchorInMarkers (oldName, newName);
}

void ComponentDocument::addMarkerMenuItem (int i, const RelativeCoordinate& coord,
                                           const String& objectName, const String& edge, PopupMenu& menu,
                                           bool isAnchor1, const String& fullCoordName)
{
    RelativeCoordinate requestedCoord (findNamedCoordinate (objectName, edge));

    String name (objectName);
    if (edge.isNotEmpty())
        name << '.' << edge;

    menu.addItem (i, name,
                  ! (name == fullCoordName || requestedCoord.references (fullCoordName, this)),
                  name == (isAnchor1 ? coord.getAnchorName1() : coord.getAnchorName2()));
}

void ComponentDocument::addComponentMarkerMenuItems (const ValueTree& componentState, const String& coordName,
                                                     RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1, bool isHorizontal)
{
    const String componentName (componentState [memberNameProperty].toString());
    const String fullCoordName (componentName + "." + coordName);

    if (isHorizontal)
    {
        addMarkerMenuItem (1, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::left, menu, isAnchor1, fullCoordName);
        addMarkerMenuItem (2, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::right, menu, isAnchor1, fullCoordName);
        menu.addSeparator();
        addMarkerMenuItem (3, coord, componentName, RelativeCoordinate::Strings::left, menu, isAnchor1, fullCoordName);
        addMarkerMenuItem (4, coord, componentName, RelativeCoordinate::Strings::right, menu, isAnchor1, fullCoordName);
    }
    else
    {
        addMarkerMenuItem (1, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::top, menu, isAnchor1, fullCoordName);
        addMarkerMenuItem (2, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::bottom, menu, isAnchor1, fullCoordName);
        menu.addSeparator();
        addMarkerMenuItem (3, coord, componentName, RelativeCoordinate::Strings::top, menu, isAnchor1, fullCoordName);
        addMarkerMenuItem (4, coord, componentName, RelativeCoordinate::Strings::bottom, menu, isAnchor1, fullCoordName);
    }

    menu.addSeparator();
    const MarkerList& markerList = getMarkerList (isHorizontal);

    int i;
    for (i = 0; i < markerList.size(); ++i)
        addMarkerMenuItem (100 + i, coord, markerList.getName (markerList.getMarker (i)), String::empty, menu, isAnchor1, fullCoordName);

    menu.addSeparator();
    for (i = 0; i < getNumComponents(); ++i)
    {
        const String compName (getComponent (i) [memberNameProperty].toString());

        if (compName != componentName)
        {
            if (isHorizontal)
            {
                addMarkerMenuItem (10000 + i * 4, coord, compName, RelativeCoordinate::Strings::left, menu, isAnchor1, fullCoordName);
                addMarkerMenuItem (10001 + i * 4, coord, compName, RelativeCoordinate::Strings::right, menu, isAnchor1, fullCoordName);
            }
            else
            {
                addMarkerMenuItem (10002 + i * 4, coord, compName, RelativeCoordinate::Strings::top, menu, isAnchor1, fullCoordName);
                addMarkerMenuItem (10003 + i * 4, coord, compName, RelativeCoordinate::Strings::bottom, menu, isAnchor1, fullCoordName);
            }
        }
    }
}

const String ComponentDocument::getChosenMarkerMenuItem (const ValueTree& componentState, RelativeCoordinate& coord, int i, bool isHorizontal) const
{
    const String componentName (componentState [memberNameProperty].toString());

    if (i == 1)  return isHorizontal ? RelativeCoordinate::Strings::parentLeft : RelativeCoordinate::Strings::parentTop;
    if (i == 2)  return isHorizontal ? RelativeCoordinate::Strings::parentRight : RelativeCoordinate::Strings::parentBottom;
    if (i == 3)  return componentName + (isHorizontal ? ".left" : ".top");
    if (i == 4)  return componentName + (isHorizontal ? ".right" : ".bottom");

    const MarkerList& markerList = getMarkerList (isHorizontal);

    if (i >= 100 && i < 10000)
        return markerList.getName (markerList.getMarker (i - 100));

    if (i >= 10000)
    {
        const String compName (getComponent ((i - 10000) / 4) [memberNameProperty].toString());
        switch (i & 3)
        {
            case 0:     return compName + ".left";
            case 1:     return compName + ".right";
            case 2:     return compName + ".top";
            case 3:     return compName + ".bottom";
            default:    break;
        }
    }

    jassertfalse;
    return String::empty;
}

void ComponentDocument::updateComponent (Component* comp)
{
    const ValueTree v (getComponentState (comp));

    if (v.isValid())
    {
        ComponentTypeInstance item (*this, v);
        item.updateComponent (comp);
    }
}

bool ComponentDocument::containsComponent (Component* comp) const
{
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return true;

    return false;
}

const ValueTree ComponentDocument::getComponentState (Component* comp) const
{
    jassert (comp != 0);
    return getComponentGroup().getChildWithProperty (idProperty, getJucerIDFor (comp));
}

Component* ComponentDocument::findComponentForState (Component* compHolder, const ValueTree& state)
{
    const String compId (state [idProperty].toString());

    for (int i = compHolder->getNumChildComponents(); --i >= 0;)
    {
        Component* const c = compHolder->getChildComponent (i);
        if (getJucerIDFor (c) == compId)
            return c;
    }

    return 0;
}

bool ComponentDocument::isStateForComponent (const ValueTree& storedState, Component* comp) const
{
    jassert (comp != 0);
    jassert (! storedState [idProperty].isVoid());
    return storedState [idProperty] == getJucerIDFor (comp);
}

void ComponentDocument::removeComponent (const ValueTree& state)
{
    jassert (state.isAChildOf (getComponentGroup()));
    renameAnchor (state [memberNameProperty], String::empty);
    getComponentGroup().removeChild (state, getUndoManager());
}

//==============================================================================
ComponentDocument::MarkerList::MarkerList (ComponentDocument& document_, const bool isX_)
    : MarkerListBase (document_.getRoot().getChildWithName (isX_ ? markersGroupXTag : markersGroupYTag), isX_),
      document (document_)
{
    jassert (group.isValid());
    jassert (group.isAChildOf (document_.getRoot()));
}

UndoManager* ComponentDocument::MarkerList::getUndoManager() const
{
    return document.getUndoManager();
}

const String ComponentDocument::MarkerList::getNonexistentMarkerName (const String& name)
{
    return document.getNonexistentMemberName (name);
}

void ComponentDocument::MarkerList::renameAnchor (const String& oldName, const String& newName)
{
    document.renameAnchor (oldName, newName);
}

const RelativeCoordinate ComponentDocument::MarkerList::findNamedCoordinate (const String& objectName, const String& edge) const
{
    if (objectName == RelativeCoordinate::Strings::parent)
    {
        if (edge == RelativeCoordinate::Strings::right)     return RelativeCoordinate ((double) document.getCanvasWidth().getValue(), true);
        if (edge == RelativeCoordinate::Strings::bottom)    return RelativeCoordinate ((double) document.getCanvasHeight().getValue(), false);
    }

    const ValueTree marker (getMarkerNamed (objectName));
    if (marker.isValid())
        return getCoordinate (marker);

    return RelativeCoordinate();
}

void ComponentDocument::MarkerList::addMarkerMenuItems (const ValueTree& markerState, const RelativeCoordinate& coord, PopupMenu& menu, bool isAnchor1)
{
    const String fullCoordName (getName (markerState));

    if (isX)
    {
        document.addMarkerMenuItem (1, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::left, menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::right, menu, isAnchor1, fullCoordName);
    }
    else
    {
        document.addMarkerMenuItem (1, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::top, menu, isAnchor1, fullCoordName);
        document.addMarkerMenuItem (2, coord, RelativeCoordinate::Strings::parent, RelativeCoordinate::Strings::bottom, menu, isAnchor1, fullCoordName);
    }

    menu.addSeparator();
    const MarkerList& markerList = document.getMarkerList (isX);

    for (int i = 0; i < markerList.size(); ++i)
        document.addMarkerMenuItem (100 + i, coord, markerList.getName (markerList.getMarker (i)),
                                    String::empty, menu, isAnchor1, fullCoordName);
}

const String ComponentDocument::MarkerList::getChosenMarkerMenuItem (const RelativeCoordinate& coord, int i) const
{
    if (i == 1)  return isX ? "parent.left" : "parent.top";
    if (i == 2)  return isX ? "parent.right" : "parent.bottom";

    const MarkerList& markerList = document.getMarkerList (isX);

    if (i >= 100 && i < 10000)
        return markerList.getName (markerList.getMarker (i - 100));

    jassertfalse;
    return String::empty;
}


//==============================================================================
bool ComponentDocument::MarkerList::createProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree marker (group.getChildWithProperty (idProperty, itemId));

    if (marker.isValid())
    {
        props.add (new TextPropertyComponent (Value (new MarkerListBase::MarkerNameValueSource (this, getNameAsValue (marker))),
                                              "Marker Name", 256, false));

        props.add (new MarkerListBase::PositionPropertyComponent (&document, *this, "Position", marker,
                                                                  marker.getPropertyAsValue (getMarkerPosProperty(), document.getUndoManager())));
        return true;
    }

    return false;
}

//==============================================================================
bool ComponentDocument::createItemProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree comp (getComponentWithID (itemId));

    if (comp.isValid())
    {
        ComponentTypeInstance item (*this, comp);
        item.createProperties (props);
        return true;
    }

    if (markersX->createProperties (props, itemId)
         || markersY->createProperties (props, itemId))
        return true;

    return false;
}

void ComponentDocument::createItemProperties (Array <PropertyComponent*>& props, const StringArray& selectedItemIds)
{
    if (selectedItemIds.size() != 1)
        return; //xxx

    for (int i = 0; i < selectedItemIds.size(); ++i)
        createItemProperties (props, selectedItemIds[i]);
}

//==============================================================================
UndoManager* ComponentDocument::getUndoManager() const
{
    return &undoManager;
}

//==============================================================================
const String ComponentDocument::getJucerIDFor (Component* c)
{
    if (c == 0)
    {
        jassertfalse;
        return String::empty;
    }

    jassert (c->getProperties().contains (jucerIDProperty));
    return c->getProperties() [jucerIDProperty];
}

//==============================================================================
void ComponentDocument::createClassProperties (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getClassName(), "Class Name", 256, false));
    props.getLast()->setTooltip ("The C++ class name for the component class.");

    props.add (new TextPropertyComponent (getClassDescription(), "Description", 512, false));
    props.getLast()->setTooltip ("A freeform description of the component.");

    props.add (new SliderPropertyComponent (getCanvasWidth(), "Initial Width", 1.0, 8192.0, 1.0));
    props.getLast()->setTooltip ("The initial width of the component when it is created.");

    props.add (new SliderPropertyComponent (getCanvasHeight(), "Initial Height", 1.0, 8192.0, 1.0));
    props.getLast()->setTooltip ("The initial height of the component when it is created.");
}
