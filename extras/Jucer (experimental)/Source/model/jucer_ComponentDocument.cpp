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

static const char* const metadataTagStart       = "JUCER_" "COMPONENT_METADATA_START"; // written like this to avoid thinking this file is a component!
static const char* const metadataTagEnd         = "JUCER_" "COMPONENT_METADATA_END";

const char* const ComponentDocument::idProperty             = "id";
const char* const ComponentDocument::compBoundsProperty     = "position";
const char* const ComponentDocument::memberNameProperty     = "memberName";
const char* const ComponentDocument::compNameProperty       = "name";


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_), cppFile (cppFile_), root (componentDocumentTag),
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

void ComponentDocument::checkRootObject()
{
    jassert (root.hasType (componentDocumentTag));

    if (! getComponentGroup().isValid())
        root.addChild (ValueTree (componentGroupTag), -1, 0);

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
    const ValueTree comps (getComponentGroup());

    for (int i = comps.getNumChildren(); --i >= 0;)
    {
        const ValueTree v (comps.getChild(i));
        if (v [memberNameProperty] == name)
            return v;
    }

    return ValueTree::invalid;
}

Component* ComponentDocument::createComponent (int index)
{
    const ValueTree v (getComponentGroup().getChild (index));

    if (v.isValid())
    {
        Component* c = ComponentTypeManager::getInstance()->createFromStoredType (*this, v);
        c->getProperties().set (idProperty, v[idProperty]);
        jassert (c->getProperties()[idProperty].toString().isNotEmpty());
        return c;
    }

    return 0;
}

//==============================================================================
class ComponentMarkerResolver  : public Coordinate::MarkerResolver
{
public:
    ComponentMarkerResolver (ComponentDocument& doc, const ValueTree& state_, int parentWidth_, int parentHeight_)
        : owner (doc), state (state_),
          parentWidth (parentWidth_),
          parentHeight (parentHeight_)
    {}

    ~ComponentMarkerResolver() {}

    const Coordinate findMarker (const String& name, bool isHorizontal)
    {
        if (name == "left")             return RectangleCoordinates (state [ComponentDocument::compBoundsProperty]).left;
        else if (name == "right")       return RectangleCoordinates (state [ComponentDocument::compBoundsProperty]).right;
        else if (name == "top")         return RectangleCoordinates (state [ComponentDocument::compBoundsProperty]).top;
        else if (name == "bottom")      return RectangleCoordinates (state [ComponentDocument::compBoundsProperty]).bottom;
        else if (name == Coordinate::parentRightMarkerName)     return Coordinate (parentWidth, isHorizontal);
        else if (name == Coordinate::parentBottomMarkerName)    return Coordinate (parentHeight, isHorizontal);

        return Coordinate (isHorizontal);
    }

private:
    ComponentDocument& owner;
    ValueTree state;
    int parentWidth, parentHeight;
};

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

Coordinate::MarkerResolver* ComponentDocument::createMarkerResolver (const ValueTree& state)
{
    return new ComponentMarkerResolver (*this, state, getCanvasWidth().getValue(), getCanvasHeight().getValue());
}

const StringArray ComponentDocument::getComponentMarkers (bool horizontal) const
{
    StringArray s;

    if (horizontal)
    {
        s.add (Coordinate::parentLeftMarkerName);
        s.add (Coordinate::parentRightMarkerName);
        s.add ("left");
        s.add ("right");
    }
    else
    {
        s.add (Coordinate::parentTopMarkerName);
        s.add (Coordinate::parentBottomMarkerName);
        s.add ("top");
        s.add ("bottom");
    }

    return s;
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

void ComponentDocument::getComponentProperties (Array <PropertyComponent*>& props, Component* comp)
{
    ValueTree v (getComponentState (comp));

    if (v.isValid())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandlerFor (v.getType());
        jassert (handler != 0);

        if (handler != 0)
            handler->createPropertyEditors (*this, v, props);
    }
}

bool ComponentDocument::isStateForComponent (const ValueTree& storedState, Component* comp) const
{
    jassert (comp != 0);
    jassert (! storedState [idProperty].isVoid());
    return storedState [idProperty] == comp->getProperties() [idProperty];
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
UndoManager* ComponentDocument::getUndoManager()
{
    return &undoManager;
}
