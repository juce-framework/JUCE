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
#include "Component Types/jucer_TextButton.h"
#include "Component Types/jucer_ToggleButton.h"


//==============================================================================
static const char* const componentDocumentTag   = "COMPONENT";
static const char* const componentGroupTag      = "COMPONENTS";

static const char* const idProperty             = "id";
static const char* const compBoundsProperty     = "position";
static const char* const memberNameProperty     = "memberName";
static const char* const compNameProperty       = "name";

static const char* const metadataTagStart       = "JUCER_" "COMPONENT_METADATA_START"; // written like this to avoid thinking this file is a component!
static const char* const metadataTagEnd         = "JUCER_" "COMPONENT_METADATA_END";


//==============================================================================
class ComponentBoundsEditor  : public PropertyComponent,
                               public ButtonListener,
                               public Value::Listener
{
public:
    enum Type
    {
        left, top, right, bottom
    };

    //==============================================================================
    ComponentBoundsEditor (ComponentDocument& document_, const String& name, Type type_,
                           const ValueTree& compState_, const Value& boundsValue_)
        : PropertyComponent (name, 40), document (document_), type (type_),
          compState (compState_), boundsValue (boundsValue_)
    {
        addAndMakeVisible (label = new Label (String::empty, String::empty));

        label->setEditable (true, true, false);
        label->setColour (Label::backgroundColourId, Colours::white);
        label->setColour (Label::outlineColourId, findColour (ComboBox::outlineColourId));
        label->getTextValue().referTo (Value (new BoundsCoordValueSource (boundsValue, type)));

        addAndMakeVisible (proportionButton = new TextButton ("%"));
        proportionButton->addButtonListener (this);

        addAndMakeVisible (anchorButton1 = new TextButton (String::empty));
        anchorButton1->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnTop | Button::ConnectedOnRight | Button::ConnectedOnBottom);
        anchorButton1->setTriggeredOnMouseDown (true);
        anchorButton1->addButtonListener (this);

        addAndMakeVisible (anchorButton2 = new TextButton (String::empty));
        anchorButton2->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnTop | Button::ConnectedOnRight | Button::ConnectedOnBottom);
        anchorButton2->setTriggeredOnMouseDown (true);
        anchorButton2->addButtonListener (this);

        boundsValue.addListener (this);
        valueChanged (boundsValue);
    }

    ~ComponentBoundsEditor()
    {
        boundsValue.removeListener (this);
        deleteAllChildren();
    }

    void resized()
    {
        const Rectangle<int> r (getLookAndFeel().getPropertyComponentContentPosition (*this));

        label->setBounds (r.getX(), r.getY(), r.getWidth() / 2, r.getHeight() / 2);
        proportionButton->setBounds (r.getX() + r.getWidth() / 2, r.getY(),
                                     r.getWidth() / 2, r.getHeight() / 2);

        if (anchorButton2->isVisible())
        {
            anchorButton1->setBounds (r.getX(), r.getY() + r.getHeight() / 2, r.getWidth() / 2, r.getHeight() / 2);
            anchorButton2->setBounds (r.getX() + r.getWidth() / 2, r.getY() + r.getHeight() / 2, r.getWidth() / 2, r.getHeight() / 2);
        }
        else
        {
            anchorButton1->setBounds (r.getX(), r.getY() + r.getHeight() / 2, r.getWidth(), r.getHeight() / 2);
        }
    }

    void refresh()
    {
    }

    void buttonClicked (Button* button)
    {
        RectangleCoordinates r (boundsValue.toString());
        Coordinate& coord = getCoord (r);
        ScopedPointer<Coordinate::MarkerResolver> markers (document.createMarkerResolver (compState));

        if (button == proportionButton)
        {
            coord.toggleProportionality (*markers);
            boundsValue = r.toString();
        }
        else if (button == anchorButton1)
        {
            const String marker (pickMarker (anchorButton1, coord.getAnchor1()));

            if (marker.isNotEmpty())
            {
                coord.changeAnchor1 (marker, *markers);
                boundsValue = r.toString();
            }
        }
        else if (button == anchorButton2)
        {
            const String marker (pickMarker (anchorButton2, coord.getAnchor2()));

            if (marker.isNotEmpty())
            {
                coord.changeAnchor2 (marker, *markers);
                boundsValue = r.toString();
            }
        }
    }

    void valueChanged (Value&)
    {
        RectangleCoordinates r (boundsValue.toString());
        Coordinate& coord = getCoord (r);

        anchorButton1->setButtonText (coord.getAnchor1());

        anchorButton2->setVisible (coord.isProportional());
        anchorButton2->setButtonText (coord.getAnchor2());
        resized();
    }

    //==============================================================================
    class BoundsCoordValueSource   : public Value::ValueSource,
                                     public Value::Listener
    {
    public:
        BoundsCoordValueSource (const Value& sourceValue_, Type type_)
           : sourceValue (sourceValue_), type (type_)
        {
            sourceValue.addListener (this);
        }

        ~BoundsCoordValueSource() {}

        const var getValue() const
        {
            RectangleCoordinates r (sourceValue.toString());
            Coordinate& coord = getCoord (r);

            if (coord.isProportional())
                return String (coord.getEditableValue()) + "%";

            return coord.getEditableValue();
        }

        void setValue (const var& newValue)
        {
            RectangleCoordinates r (sourceValue.toString());
            Coordinate& coord = getCoord (r);

            coord.setEditableValue ((double) newValue);

            const String newVal (r.toString());
            if (sourceValue != newVal)
                sourceValue = newVal;
        }

        void valueChanged (Value&)
        {
            sendChangeMessage (true);
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    protected:
        Value sourceValue;
        Type type;

        Coordinate& getCoord (RectangleCoordinates& r) const
        {
            return getCoordForType (type, r);
        }

        BoundsCoordValueSource (const BoundsCoordValueSource&);
        const BoundsCoordValueSource& operator= (const BoundsCoordValueSource&);
    };

    static Coordinate& getCoordForType (const Type type, RectangleCoordinates& r)
    {
        switch (type)
        {
            case left:   return r.left;
            case right:  return r.right;
            case top:    return r.top;
            case bottom: return r.bottom;
            default:     jassertfalse; break;
        }

        return r.left;
    }

    const String pickMarker (Component* button, const String& currentMarker)
    {
        const StringArray markers (document.getComponentMarkers (type == left || type == right));

        PopupMenu m;
        for (int i = 0; i < markers.size(); ++i)
            m.addItem (i + 1, markers[i], true, currentMarker == markers[i]);

        const int r = m.showAt (button);

        if (r > 0)
            return markers [r - 1];

        return String::empty;
    }

private:
    ComponentDocument& document;
    Type type;
    ValueTree compState;
    Value boundsValue;
    Label* label;
    TextButton* proportionButton;
    TextButton* anchorButton1;
    TextButton* anchorButton2;

    Coordinate& getCoord (RectangleCoordinates& r)
    {
        return getCoordForType (type, r);
    }
};


//==============================================================================
ComponentTypeHandler::ComponentTypeHandler (const String& name_, const String& xmlTag_,
                                            const String& memberNameRoot_)
    : name (name_), xmlTag (xmlTag_),
      memberNameRoot (memberNameRoot_)
{
}

ComponentTypeHandler::~ComponentTypeHandler()
{
}

Value ComponentTypeHandler::getValue (const var::identifier& name, ValueTree& state, ComponentDocument& document) const
{
    return state.getPropertyAsValue (name, document.getUndoManager());
}

void ComponentTypeHandler::updateComponent (ComponentDocument& document, Component* comp, const ValueTree& state)
{
    RectangleCoordinates pos (state [compBoundsProperty].toString());
    ScopedPointer<Coordinate::MarkerResolver> markers (document.createMarkerResolver (state));
    comp->setBounds (pos.resolve (*markers));

    comp->setName (state [compNameProperty]);
}

void ComponentTypeHandler::initialiseNewItem (ComponentDocument& document, ValueTree& state)
{
    state.setProperty (compNameProperty, String::empty, 0);
    state.setProperty (memberNameProperty, document.getNonExistentMemberName (getMemberNameRoot()), 0);

    const Rectangle<int> bounds (getDefaultSize().withPosition (Point<int> (Random::getSystemRandom().nextInt (100) + 100,
                                                                            Random::getSystemRandom().nextInt (100) + 100)));

    state.setProperty (compBoundsProperty, RectangleCoordinates (bounds).toString(), 0);
}

void ComponentTypeHandler::createPropertyEditors (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
{
    props.add (new ComponentBoundsEditor (document, "Left",   ComponentBoundsEditor::left,   state, getValue (compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Right",  ComponentBoundsEditor::right,  state, getValue (compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Top",    ComponentBoundsEditor::top,    state, getValue (compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Bottom", ComponentBoundsEditor::bottom, state, getValue (compBoundsProperty, state, document)));
}

//==============================================================================
class ComponentTypeManager  : public DeletedAtShutdown
{
public:
    ComponentTypeManager()
    {
        handlers.add (new TextButtonHandler());
        handlers.add (new ToggleButtonHandler());
    }

    ~ComponentTypeManager()
    {
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (ComponentTypeManager);

    Component* createFromStoredType (ComponentDocument& document, const ValueTree& value)
    {
        ComponentTypeHandler* handler = getHandlerFor (value.getType());
        if (handler == 0)
            return 0;

        Component* c = handler->createComponent();
        if (c != 0)
            handler->updateComponent (document, c, value);

        return c;
    }

    ComponentTypeHandler* getHandlerFor (const String& type)
    {
        for (int i = handlers.size(); --i >= 0;)
            if (handlers.getUnchecked(i)->getXmlTag() == type)
                return handlers.getUnchecked(i);

        return 0;
    }

    const StringArray getTypeNames() const
    {
        StringArray s;
        for (int i = 0; i < handlers.size(); ++i)
            s.add (handlers.getUnchecked(i)->getName());

        return s;
    }

    int getNumHandlers() const                                      { return handlers.size(); }
    ComponentTypeHandler* getHandler (const int index) const        { return handlers[index]; }

private:
    OwnedArray <ComponentTypeHandler> handlers;
};

juce_ImplementSingleton_SingleThreaded (ComponentTypeManager);


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
        if (name == "left")             return RectangleCoordinates (state [compBoundsProperty]).left;
        else if (name == "right")       return RectangleCoordinates (state [compBoundsProperty]).right;
        else if (name == "top")         return RectangleCoordinates (state [compBoundsProperty]).top;
        else if (name == "bottom")      return RectangleCoordinates (state [compBoundsProperty]).bottom;
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
