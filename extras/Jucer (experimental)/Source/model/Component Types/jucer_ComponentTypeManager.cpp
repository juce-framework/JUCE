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

#include "jucer_ComponentTypeManager.h"
#include "jucer_ComponentTypes.h"
#include "../../ui/jucer_CoordinatePropertyComponent.h"


//==============================================================================
class ComponentBoundsEditor  : public CoordinatePropertyComponent
{
public:
    enum Type
    {
        left, top, right, bottom
    };

    //==============================================================================
    ComponentBoundsEditor (ComponentDocument& document_, const String& name, Type type_,
                           const ValueTree& compState_, const Value& coordValue_)
        : CoordinatePropertyComponent (document_, name,
                                       Value (new CoordExtractor (coordValue_, type_)),
                                       type_ == left || type_ == right),
          type (type_),
          compState (compState_)
    {
    }

    ~ComponentBoundsEditor()
    {
    }

    //==============================================================================
    class CoordExtractor   : public Value::ValueSource,
                             public Value::Listener
    {
    public:
        CoordExtractor (const Value& sourceValue_, Type type_)
           : sourceValue (sourceValue_), type (type_)
        {
            sourceValue.addListener (this);
        }

        ~CoordExtractor() {}

        const var getValue() const
        {
            RectangleCoordinates r (sourceValue.toString());
            return getCoord (r).toString();
        }

        void setValue (const var& newValue)
        {
            RectangleCoordinates r (sourceValue.toString());
            Coordinate& coord = getCoord (r);
            coord = Coordinate (newValue.toString(), coord.isHorizontal());

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

        Coordinate& getCoord (RectangleCoordinates& r) const
        {
            return getCoordForType (type, r);
        }

        CoordExtractor (const CoordExtractor&);
        const CoordExtractor& operator= (const CoordExtractor&);
    };

    const String pickMarker (TextButton* button, const String& currentMarker, bool isAnchor1)
    {
        Coordinate coord (getCoordinate());

        PopupMenu m;
        document.addComponentMarkerMenuItems (compState, getTypeName(), coord, m, isAnchor1);

        const int r = m.showAt (button);

        if (r > 0)
            return document.getChosenMarkerMenuItem (compState, coord, r);

        return String::empty;
    }

private:
    Type type;
    ValueTree compState;

    const String getTypeName() const
    {
        switch (type)
        {
            case left:      return "left";
            case right:     return "right";
            case top:       return "top";
            case bottom:    return "bottom";
            default:        jassertfalse; break;
        }

        return String::empty;
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
    RectangleCoordinates pos (state [ComponentDocument::compBoundsProperty].toString());
    comp->setBounds (pos.resolve (document));

    comp->setName (state [ComponentDocument::compNameProperty]);

    comp->setExplicitFocusOrder (state [ComponentDocument::compFocusOrderProperty]);

    SettableTooltipClient* tooltipClient = dynamic_cast <SettableTooltipClient*> (comp);
    if (tooltipClient != 0)
        tooltipClient->setTooltip (state [ComponentDocument::compTooltipProperty]);
}

void ComponentTypeHandler::initialiseNewItem (ComponentDocument& document, ValueTree& state)
{
    state.setProperty (ComponentDocument::compNameProperty, String::empty, 0);
    state.setProperty (ComponentDocument::memberNameProperty, document.getNonExistentMemberName (getMemberNameRoot()), 0);

    const Rectangle<int> bounds (getDefaultSize().withPosition (Point<int> (Random::getSystemRandom().nextInt (100) + 100,
                                                                            Random::getSystemRandom().nextInt (100) + 100)));

    state.setProperty (ComponentDocument::compBoundsProperty, RectangleCoordinates (bounds, state [ComponentDocument::memberNameProperty]).toString(), 0);
}


//==============================================================================
class CompMemberNameValueSource   : public Value::ValueSource,
                                    public Value::Listener
{
public:
    CompMemberNameValueSource (ComponentDocument& document_, const ValueTree& state_)
       : sourceValue (state_.getPropertyAsValue (ComponentDocument::memberNameProperty, document_.getUndoManager())),
         document (document_),
         state (state_)
    {
        sourceValue.addListener (this);
    }

    ~CompMemberNameValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }
    const var getValue() const      { return sourceValue.toString(); }

    void setValue (const var& newValue)
    {
        String newVal (newValue.toString());

        //xxx check for uniqueness + rename any coords that use the name..


        newVal = makeValidCppIdentifier (newVal, false, true, false);

        if (sourceValue != newVal)
            sourceValue = newVal;
    }

private:
    Value sourceValue;
    ComponentDocument& document;
    ValueTree state;

    CompMemberNameValueSource (const CompMemberNameValueSource&);
    const CompMemberNameValueSource& operator= (const CompMemberNameValueSource&);
};

void ComponentTypeHandler::createPropertyEditors (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (Value (new CompMemberNameValueSource (document, state)), "Member Name", 256, false));

    props.add (new ComponentBoundsEditor (document, "Left",   ComponentBoundsEditor::left,   state, getValue (ComponentDocument::compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Right",  ComponentBoundsEditor::right,  state, getValue (ComponentDocument::compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Top",    ComponentBoundsEditor::top,    state, getValue (ComponentDocument::compBoundsProperty, state, document)));
    props.add (new ComponentBoundsEditor (document, "Bottom", ComponentBoundsEditor::bottom, state, getValue (ComponentDocument::compBoundsProperty, state, document)));
}

//==============================================================================
ComponentTypeManager::ComponentTypeManager()
{
    #define ADD_TO_LIST(HandlerType)   handlers.add (new HandlerType());
    #include "jucer_ComponentTypes.h"
    #undef ADD_TO_LIST
}

ComponentTypeManager::~ComponentTypeManager()
{
}

Component* ComponentTypeManager::createFromStoredType (ComponentDocument& document, const ValueTree& value)
{
    ComponentTypeHandler* handler = getHandlerFor (value.getType());
    if (handler == 0)
        return 0;

    Component* c = handler->createComponent();
    if (c != 0)
        handler->updateComponent (document, c, value);

    return c;
}

ComponentTypeHandler* ComponentTypeManager::getHandlerFor (const String& type)
{
    for (int i = handlers.size(); --i >= 0;)
        if (handlers.getUnchecked(i)->getXmlTag() == type)
            return handlers.getUnchecked(i);

    return 0;
}

const StringArray ComponentTypeManager::getTypeNames() const
{
    StringArray s;
    for (int i = 0; i < handlers.size(); ++i)
        s.add (handlers.getUnchecked(i)->getName());

    return s;
}

juce_ImplementSingleton_SingleThreaded (ComponentTypeManager);
