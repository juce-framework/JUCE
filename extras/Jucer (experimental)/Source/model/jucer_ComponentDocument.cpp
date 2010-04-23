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

#include "jucer_ComponentDocument.h"
#include "Component Types/jucer_ComponentTypeManager.h"


//==============================================================================
static const char* const componentDocumentTag   = "COMPONENT";
static const char* const componentGroupTag      = "COMPONENTS";
static const char* const markersGroupXTag       = "MARKERS_X";
static const char* const markersGroupYTag       = "MARKERS_Y";
static const char* const markerTag              = "MARKER";

static const char* const metadataTagStart       = "JUCER_" "COMPONENT_METADATA_START"; // written like this to avoid thinking this file is a component!
static const char* const metadataTagEnd         = "JUCER_" "COMPONENT_METADATA_END";

const char* const ComponentDocument::idProperty             = "id";
const char* const ComponentDocument::compBoundsProperty     = "position";
const char* const ComponentDocument::memberNameProperty     = "memberName";
const char* const ComponentDocument::compNameProperty       = "name";
const char* const ComponentDocument::markerNameProperty     = "name";
const char* const ComponentDocument::markerPosProperty      = "position";


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_), cppFile (cppFile_),
     root (componentDocumentTag),
     changedSinceSaved (false)
{
    reload();
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

void ComponentDocument::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property)
{
    changedSinceSaved = true;
}

void ComponentDocument::valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged)
{
    changedSinceSaved = true;
}

void ComponentDocument::valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged)
{
    changedSinceSaved = true;
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

void ComponentDocument::writeCode (OutputStream& cpp, OutputStream& header)
{
    cpp << "/**  */"
        << newLine << newLine;

    header << "/**  */"
           << newLine << newLine;
}

void ComponentDocument::writeMetadata (OutputStream& out)
{
    out << "#if 0" << newLine
        << "/** Jucer-generated metadata section - Edit this data at own risk!" << newLine
        << metadataTagStart << newLine << newLine;

    ScopedPointer<XmlElement> xml (root.createXml());
    jassert (xml != 0);

    if (xml != 0)
        xml->writeToStream (out, String::empty, false, false);

    out << newLine
        << metadataTagEnd << " */" << newLine
        << "#endif" << newLine;
}

bool ComponentDocument::save()
{
    MemoryOutputStream cpp, header;
    writeCode (cpp, header);
    writeMetadata (cpp);

    bool savedOk = overwriteFileWithNewDataIfDifferent (cppFile, cpp)
                    && overwriteFileWithNewDataIfDifferent (cppFile.withFileExtension (".h"), header);

    if (savedOk)
        changedSinceSaved = false;

    return savedOk;
}

bool ComponentDocument::reload()
{
    String xmlString;

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
                        break;

                    xml.append (line);
                    xml.append (newLine);
                }

                break;
            }
        }
    }

    XmlDocument doc (xmlString);
    ScopedPointer<XmlElement> xml (doc.getDocumentElement());

    if (xml != 0 && xml->hasTagName (componentDocumentTag))
    {
        ValueTree newTree (ValueTree::fromXml (*xml));

        if (newTree.isValid())
        {
            root = newTree;
            markersX = 0;
            markersY = 0;
            checkRootObject();
            undoManager.clearUndoHistory();
            changedSinceSaved = false;
            return true;
        }
    }

    return false;
}

bool ComponentDocument::hasChangedSinceLastSave()
{
    return changedSinceSaved;
}

void ComponentDocument::createSubTreeIfNotThere (const String& name)
{
    if (! root.getChildWithName (name).isValid())
        root.addChild (ValueTree (name), -1, 0);
}

void ComponentDocument::checkRootObject()
{
    jassert (root.hasType (componentDocumentTag));

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
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void ComponentDocument::addNewComponentMenuItems (PopupMenu& menu) const
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    for (int i = 0; i < typeNames.size(); ++i)
        menu.addItem (i + menuItemOffset, "New " + typeNames[i]);
}

void ComponentDocument::performNewComponentMenuItem (int menuResultCode)
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    if (menuResultCode >= menuItemOffset && menuResultCode < menuItemOffset + typeNames.size())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandler (menuResultCode - menuItemOffset);
        jassert (handler != 0);

        if (handler != 0)
        {
            ValueTree state (handler->getXmlTag());
            state.setProperty (idProperty, createAlphaNumericUID(), 0);
            handler->initialiseNewItem (*this, state);

            getComponentGroup().addChild (state, -1, getUndoManager());
        }
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
const Coordinate ComponentDocument::findMarker (const String& name, bool isHorizontal) const
{
    if (name == Coordinate::parentRightMarkerName)     return Coordinate ((double) getCanvasWidth().getValue(), isHorizontal);
    if (name == Coordinate::parentBottomMarkerName)    return Coordinate ((double) getCanvasHeight().getValue(), isHorizontal);

    if (name.containsChar ('.'))
    {
        const String compName (name.upToFirstOccurrenceOf (".", false, false).trim());
        const String edge (name.fromFirstOccurrenceOf (".", false, false).trim());

        if (compName.isNotEmpty() && edge.isNotEmpty())
        {
            const ValueTree comp (getComponentWithMemberName (compName));

            if (comp.isValid())
            {
                const RectangleCoordinates coords (getCoordsFor (comp));

                if (edge == "left")   return coords.left;
                if (edge == "right")  return coords.right;
                if (edge == "top")    return coords.top;
                if (edge == "bottom") return coords.bottom;
            }
        }
    }

    const ValueTree marker (getMarkerList (isHorizontal).getMarkerNamed (name));
    if (marker.isValid())
        return getMarkerList (isHorizontal).getCoordinate (marker);

    return Coordinate (isHorizontal);
}

const RectangleCoordinates ComponentDocument::getCoordsFor (const ValueTree& state) const
{
    return RectangleCoordinates (state [compBoundsProperty]);
}

bool ComponentDocument::setCoordsFor (ValueTree& state, const RectangleCoordinates& pr)
{
    const String newBoundsString (pr.toString());

    if (state[compBoundsProperty] == newBoundsString)
        return false;

    state.setProperty (compBoundsProperty, newBoundsString, getUndoManager());
    return true;
}

void ComponentDocument::addMarkerMenuItem (int i, Coordinate& coord, const String& name, PopupMenu& menu, bool isAnchor1,
                                           const ValueTree& componentState, const String& coordName)
{
    const String componentName (componentState [memberNameProperty].toString());
    Coordinate requestedCoord (findMarker (name, coord.isHorizontal()));
    const String fullCoordName (componentName + "." + coordName);

    menu.addItem (i, name,
                  ! (name == fullCoordName || requestedCoord.referencesIndirectly (fullCoordName, *this)),
                  name == (isAnchor1 ? coord.getAnchor1() : coord.getAnchor2()));
}

void ComponentDocument::getComponentMarkerMenuItems (const ValueTree& componentState, const String& coordName,
                                                     Coordinate& coord, PopupMenu& menu, bool isAnchor1)
{
    const String componentName (componentState [memberNameProperty].toString());

    if (coord.isHorizontal())
    {
        addMarkerMenuItem (1, coord, Coordinate::parentLeftMarkerName, menu, isAnchor1, componentState, coordName);
        addMarkerMenuItem (2, coord, Coordinate::parentRightMarkerName, menu, isAnchor1, componentState, coordName);
        menu.addSeparator();
        addMarkerMenuItem (3, coord, componentName + ".left", menu, isAnchor1, componentState, coordName);
        addMarkerMenuItem (4, coord, componentName + ".right", menu, isAnchor1, componentState, coordName);
    }
    else
    {
        addMarkerMenuItem (1, coord, Coordinate::parentTopMarkerName, menu, isAnchor1, componentState, coordName);
        addMarkerMenuItem (2, coord, Coordinate::parentBottomMarkerName, menu, isAnchor1, componentState, coordName);
        menu.addSeparator();
        addMarkerMenuItem (3, coord, componentName + ".top", menu, isAnchor1, componentState, coordName);
        addMarkerMenuItem (4, coord, componentName + ".bottom", menu, isAnchor1, componentState, coordName);
    }

    menu.addSeparator();
    const MarkerList& markerList = getMarkerList (coord.isHorizontal());

    int i;
    for (i = 0; i < markerList.size(); ++i)
        addMarkerMenuItem (100 + i, coord, markerList.getName (markerList.getMarker (i)), menu, isAnchor1, componentState, coordName);

    menu.addSeparator();
    for (i = 0; i < getNumComponents(); ++i)
    {
        const String compName (getComponent (i) [memberNameProperty].toString());

        if (compName != componentName)
        {
            if (coord.isHorizontal())
            {
                addMarkerMenuItem (10000 + i * 4, coord, compName + ".left", menu, isAnchor1, componentState, coordName);
                addMarkerMenuItem (10001 + i * 4, coord, compName + ".right", menu, isAnchor1, componentState, coordName);
            }
            else
            {
                addMarkerMenuItem (10002 + i * 4, coord, compName + ".top", menu, isAnchor1, componentState, coordName);
                addMarkerMenuItem (10003 + i * 4, coord, compName + ".bottom", menu, isAnchor1, componentState, coordName);
            }
        }
    }
}

const String ComponentDocument::getChosenMarkerMenuItem (const ValueTree& componentState, Coordinate& coord, int i) const
{
    const String componentName (componentState [memberNameProperty].toString());

    if (i == 1)  return coord.isHorizontal() ? Coordinate::parentLeftMarkerName : Coordinate::parentTopMarkerName;
    if (i == 2)  return coord.isHorizontal() ? Coordinate::parentRightMarkerName : Coordinate::parentBottomMarkerName;
    if (i == 3)  return componentName + (coord.isHorizontal() ? ".left" : ".top");
    if (i == 4)  return componentName + (coord.isHorizontal() ? ".right" : ".bottom");

    const MarkerList& markerList = getMarkerList (coord.isHorizontal());

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
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandlerFor (v.getType());
        jassert (handler != 0);

        if (handler != 0)
            handler->updateComponent (*this, comp, v);
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
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return comps.getChild(i);

    jassertfalse;
    return ValueTree::invalid;
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
    getComponentGroup().removeChild (state, getUndoManager());
}

const String ComponentDocument::getNonExistentMemberName (String suggestedName)
{
    suggestedName = makeValidCppIdentifier (suggestedName, false, true, false);
    const String original (suggestedName);
    int num = 1;

    while (getComponentWithMemberName (suggestedName).isValid())
    {
        suggestedName = original;
        while (String ("0123456789").containsChar (suggestedName.getLastCharacter()))
            suggestedName = suggestedName.dropLastCharacters (1);

        suggestedName << num++;
    }

    return suggestedName;
}

//==============================================================================
ComponentDocument::MarkerList::MarkerList (ComponentDocument& document_, const bool isX_)
    : document (document_),
      group (document_.getRoot().getChildWithName (isX_ ? markersGroupXTag : markersGroupYTag)),
      isX (isX_)
{
    jassert (group.isAChildOf (document_.getRoot()));
}

ValueTree& ComponentDocument::MarkerList::getGroup()
{
    return group;
}

int ComponentDocument::MarkerList::size() const
{
    return group.getNumChildren();
}

ValueTree ComponentDocument::MarkerList::getMarker (int index) const
{
    return group.getChild (index);
}

ValueTree ComponentDocument::MarkerList::getMarkerNamed (const String& name) const
{
    return group.getChildWithProperty (markerNameProperty, name);
}

bool ComponentDocument::MarkerList::contains (const ValueTree& markerState) const
{
    return markerState.isAChildOf (group);
}

const Coordinate ComponentDocument::MarkerList::getCoordinate (const ValueTree& markerState) const
{
    return Coordinate (markerState [markerPosProperty].toString(), isX);
}

const String ComponentDocument::MarkerList::getName (const ValueTree& markerState) const
{
    return markerState [markerNameProperty].toString();
}

Value ComponentDocument::MarkerList::getNameAsValue (const ValueTree& markerState) const
{
    return markerState.getPropertyAsValue (markerNameProperty, document.getUndoManager());
}

void ComponentDocument::MarkerList::setCoordinate (ValueTree& markerState, const Coordinate& newCoord)
{
    markerState.setProperty (markerPosProperty, newCoord.toString(), document.getUndoManager());
}

void ComponentDocument::MarkerList::createMarker (const String& name, int position)
{
    ValueTree marker (markerTag);
    marker.setProperty (markerNameProperty, document.getNonexistentMarkerName (name), 0);
    marker.setProperty (markerPosProperty, Coordinate (position, isX).toString(), 0);
    marker.setProperty (idProperty, createAlphaNumericUID(), 0);
    group.addChild (marker, -1, document.getUndoManager());
}

void ComponentDocument::MarkerList::deleteMarker (ValueTree& markerState)
{
    group.removeChild (markerState, document.getUndoManager());
}

const Coordinate ComponentDocument::MarkerList::findMarker (const String& name, bool isHorizontal) const
{
    if (isHorizontal == isX)
    {
        if (name == Coordinate::parentRightMarkerName)   return Coordinate ((double) document.getCanvasWidth().getValue(), isHorizontal);
        if (name == Coordinate::parentBottomMarkerName)  return Coordinate ((double) document.getCanvasHeight().getValue(), isHorizontal);

        const ValueTree marker (document.getMarkerList (isHorizontal).getMarkerNamed (name));
        if (marker.isValid())
            return document.getMarkerList (isHorizontal).getCoordinate (marker);
    }

    return Coordinate (isX);
}

void ComponentDocument::MarkerList::createMarkerProperties (Array <PropertyComponent*>& props, ValueTree& marker)
{
    props.add (new TextPropertyComponent (getNameAsValue (marker), "Marker Name", 256, false));
}

bool ComponentDocument::MarkerList::createProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree marker (group.getChildWithProperty (idProperty, itemId));

    if (marker.isValid())
    {
        createMarkerProperties (props, marker);
        return true;
    }

    return false;
}

const String ComponentDocument::getNonexistentMarkerName (const String& name)
{
    String n (makeValidCppIdentifier (name, false, true, false));
    int suffix = 2;

    while (markersX->getMarkerNamed (n).isValid() || markersY->getMarkerNamed (n).isValid())
        n = n.trimCharactersAtEnd ("0123456789") + String (suffix++);

    return n;
}

//==============================================================================
bool ComponentDocument::createItemProperties (Array <PropertyComponent*>& props, const String& itemId)
{
    ValueTree comp (getComponentWithID (itemId));

    if (comp.isValid())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandlerFor (comp.getType());
        jassert (handler != 0);

        if (handler != 0)
            handler->createPropertyEditors (*this, comp, props);

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
const char* const ComponentDocument::jucerIDProperty = "jucerID";

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
