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
#include "../../../utility/jucer_CoordinatePropertyComponent.h"


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
        : CoordinatePropertyComponent (&document_, name,
                                       Value (new CoordExtractor (coordValue_, type_)),
                                       type_ == left || type_ == right),
          document (document_),
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
            RelativeRectangle r (sourceValue.toString());
            return getCoord (r).toString();
        }

        void setValue (const var& newValue)
        {
            RelativeRectangle r (sourceValue.toString());
            RelativeCoordinate& coord = getCoord (r);
            coord = RelativeCoordinate (newValue.toString(), type == left || type == right);

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

        static RelativeCoordinate& getCoordForType (const Type type, RelativeRectangle& r)
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

        RelativeCoordinate& getCoord (RelativeRectangle& r) const
        {
            return getCoordForType (type, r);
        }

        CoordExtractor (const CoordExtractor&);
        const CoordExtractor& operator= (const CoordExtractor&);
    };

    const String pickMarker (TextButton* button, const String& currentMarker, bool isAnchor1)
    {
        RelativeCoordinate coord (getCoordinate());

        PopupMenu m;
        document.addComponentMarkerMenuItems (compState, getTypeName(), coord, m, isAnchor1, type == left || type == right);

        const int r = m.showAt (button);

        if (r > 0)
            return document.getChosenMarkerMenuItem (compState, coord, r, type == left || type == right);

        return String::empty;
    }

private:
    ComponentDocument& document;
    Type type;
    ValueTree compState;

    const String getTypeName() const
    {
        switch (type)
        {
            case left:      return RelativeCoordinate::Strings::left;
            case right:     return RelativeCoordinate::Strings::right;
            case top:       return RelativeCoordinate::Strings::top;
            case bottom:    return RelativeCoordinate::Strings::bottom;
            default:        jassertfalse; break;
        }

        return String::empty;
    }
};

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
    {
        ComponentTypeInstance item (document, value);
        handler->updateComponent (item, c);
    }

    return c;
}

ComponentTypeHandler* ComponentTypeManager::getHandlerFor (const Identifier& type)
{
    for (int i = handlers.size(); --i >= 0;)
        if (handlers.getUnchecked(i)->getValueTreeType() == type)
            return handlers.getUnchecked(i);

    return 0;
}

const StringArray ComponentTypeManager::getDisplayNames() const
{
    StringArray s;
    for (int i = 0; i < handlers.size(); ++i)
        s.add (handlers.getUnchecked(i)->getDisplayName());

    return s;
}

juce_ImplementSingleton_SingleThreaded (ComponentTypeManager);



//==============================================================================
ComponentTypeHandler::ComponentTypeHandler (const String& displayName_, const String& className_,
                                            const Identifier& valueTreeType_, const String& memberNameRoot_)
    : displayName (displayName_), className (className_), valueTreeType (valueTreeType_),
      memberNameRoot (memberNameRoot_)
{
}

ComponentTypeHandler::~ComponentTypeHandler()
{
}

//==============================================================================
class CompMemberNameValueSource   : public Value::ValueSource,
                                    public Value::Listener
{
public:
    CompMemberNameValueSource (ComponentTypeInstance& item_)
       : sourceValue (item_.getValue (ComponentDocument::memberNameProperty)),
         item (item_)
    {
        sourceValue.addListener (this);
    }

    ~CompMemberNameValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }
    const var getValue() const      { return sourceValue.toString(); }

    void setValue (const var& newValue)
    {
        if (newValue == sourceValue)
            return;

        const String name (item.getDocument().getNonexistentMemberName (newValue));

        if (sourceValue != name)
        {
            item.getDocument().renameAnchor (sourceValue.toString(), name);
            sourceValue = name;
        }
    }

private:
    Value sourceValue;
    ComponentTypeInstance item;

    CompMemberNameValueSource (const CompMemberNameValueSource&);
    const CompMemberNameValueSource& operator= (const CompMemberNameValueSource&);
};


//==============================================================================
ComponentTypeInstance::ComponentTypeInstance (ComponentDocument& document_, const ValueTree& state_)
    : document (document_), state (state_)
{
}

Value ComponentTypeInstance::getValue (const Identifier& name) const
{
    return state.getPropertyAsValue (name, document.getUndoManager());
}

void ComponentTypeInstance::set (const Identifier& name, const var& value)
{
    state.setProperty (name, value, 0);
}

ComponentTypeHandler* ComponentTypeInstance::getHandler() const
{
    ComponentTypeHandler* const handler = ComponentTypeManager::getInstance()->getHandlerFor (state.getType());
    jassert (handler != 0);
    return handler;
}

void ComponentTypeInstance::updateComponent (Component* comp)
{
    getHandler()->updateComponent (*this, comp);
}

void ComponentTypeInstance::createProperties (Array <PropertyComponent*>& props)
{
    getHandler()->createPropertyEditors (*this, props);
}

void ComponentTypeInstance::createCode (CodeGenerator& code)
{
    code.addPrivateMember (getHandler()->getClassName (*this) + "*", getMemberName());
    code.memberInitialisers.add (getMemberName() + " (0)");
    getHandler()->createCode (*this, code);
}

//==============================================================================
void ComponentTypeInstance::initialiseNewItemBasics()
{
    ComponentTypeHandler* handler = getHandler();

    set (ComponentDocument::compNameProperty, String::empty);
    set (ComponentDocument::memberNameProperty, document.getNonexistentMemberName (handler->getMemberNameRoot()));

    Rectangle<int> bounds (handler->getDefaultSize());
    int cw = document.getCanvasWidth().getValue();
    int ch = document.getCanvasHeight().getValue();
    bounds.setPosition (Random::getSystemRandom().nextInt (cw / 3) + cw / 4,
                        Random::getSystemRandom().nextInt (ch / 3) + ch / 4);

    set (ComponentDocument::compBoundsProperty, RelativeRectangle (bounds.toFloat(), getMemberName()).toString());
}

void ComponentTypeInstance::updateComponentBasics (Component* comp)
{
    RelativeRectangle pos (state [ComponentDocument::compBoundsProperty].toString());
    comp->setBounds (pos.resolve (&document).getSmallestIntegerContainer());

    //comp->setName (state [ComponentDocument::compNameProperty]);

    comp->setExplicitFocusOrder (state [ComponentDocument::compFocusOrderProperty]);

    SettableTooltipClient* tooltipClient = dynamic_cast <SettableTooltipClient*> (comp);
    if (tooltipClient != 0)
        tooltipClient->setTooltip (state [ComponentDocument::compTooltipProperty]);
}

void ComponentTypeInstance::addMemberNameProperty (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (Value (new CompMemberNameValueSource (*this)),
                                          "Member Name", 256, false));
}

void ComponentTypeInstance::addBoundsProperties (Array <PropertyComponent*>& props)
{
    const Value bounds (getValue (ComponentDocument::compBoundsProperty));
    props.add (new ComponentBoundsEditor (document, "Left",   ComponentBoundsEditor::left,   state, bounds));
    props.add (new ComponentBoundsEditor (document, "Right",  ComponentBoundsEditor::right,  state, bounds));
    props.add (new ComponentBoundsEditor (document, "Top",    ComponentBoundsEditor::top,    state, bounds));
    props.add (new ComponentBoundsEditor (document, "Bottom", ComponentBoundsEditor::bottom, state, bounds));
}

void ComponentTypeInstance::addTooltipProperty (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (getValue (ComponentDocument::compTooltipProperty),
                                          "Tooltip", 4096, false));
}

void ComponentTypeInstance::addFocusOrderProperty (Array <PropertyComponent*>& props)
{
    props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (getValue (ComponentDocument::compFocusOrderProperty))),
                                          "Focus Order", 10, false));
}

void ComponentTypeInstance::addColourProperty (Array <PropertyComponent*>& props, int colourId, const String& name, const Identifier& propertyName)
{
    props.add (new ColourPropertyComponent (document, name, getValue (propertyName),
                                            LookAndFeel::getDefaultLookAndFeel().findColour (colourId), true));
}

//==============================================================================
class FontNameValueSource   : public Value::ValueSource,
                              public Value::Listener
{
public:
    FontNameValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontNameValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        return Font::fromString (sourceValue.toString()).getTypefaceName();
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setTypefaceName (newValue.toString());
        sourceValue = font.toString();
    }

    static ChoicePropertyComponent* createProperty (const String& title, const Value& value)
    {
        StringArray fontNames;
        fontNames.add (Font::getDefaultSansSerifFontName());
        fontNames.add (Font::getDefaultSerifFontName());
        fontNames.add (Font::getDefaultMonospacedFontName());
        fontNames.add (String::empty);
        fontNames.addArray (StoredSettings::getInstance()->getFontNames());

        Array<var> values;
        for (int i = 0; i < fontNames.size(); ++i)
            values.add (fontNames[i]);

        return new ChoicePropertyComponent (Value (new FontNameValueSource (value)), title, fontNames, values);
    }

private:
    Value sourceValue;
};

class FontSizeValueSource   : public Value::ValueSource,
                              public Value::Listener
{
public:
    FontSizeValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontSizeValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        return Font::fromString (sourceValue.toString()).getHeight();
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setHeight (newValue);
        sourceValue = font.toString();
    }

    static PropertyComponent* createProperty (const String& title, const Value& value)
    {
        return new SliderPropertyComponent (Value (new FontSizeValueSource (value)), title, 1.0, 150.0, 0.1, 0.5);
    }

private:
    Value sourceValue;
};

class FontStyleValueSource   : public Value::ValueSource,
                               public Value::Listener
{
public:
    FontStyleValueSource (const Value& source)
       : sourceValue (source)
    {
        sourceValue.addListener (this);
    }

    ~FontStyleValueSource() {}

    void valueChanged (Value&)      { sendChangeMessage (true); }

    const var getValue() const
    {
        const Font f (Font::fromString (sourceValue.toString()));

        if (f.isBold() && f.isItalic()) return getStyles() [3];
        if (f.isBold())                 return getStyles() [1];
        if (f.isItalic())               return getStyles() [2];

        return getStyles() [0];
    }

    void setValue (const var& newValue)
    {
        Font font (Font::fromString (sourceValue.toString()));
        font.setBold (newValue.toString().containsIgnoreCase ("Bold"));
        font.setItalic (newValue.toString().containsIgnoreCase ("Italic"));
        sourceValue = font.toString();
    }

    static PropertyComponent* createProperty (const String& title, const Value& value)
    {
        return new ChoicePropertyComponent (Value (new FontStyleValueSource (value)), title, StringArray (getStyles()), Array<var> (getStyles()));
    }

    static const char* const* getStyles()
    {
        static const char* const fontStyles[] = { "Normal", "Bold", "Italic", "Bold + Italic", 0 };
        return fontStyles;
    }

private:
    Value sourceValue;
};

void ComponentTypeInstance::addFontProperties (Array <PropertyComponent*>& props, const Identifier& name)
{
    Value v (getValue (name));
    props.add (FontNameValueSource::createProperty ("Font", v));
    props.add (FontSizeValueSource::createProperty ("Font Size", v));
    props.add (FontStyleValueSource::createProperty ("Font Style", v));
}

//==============================================================================
void ComponentTypeInstance::addJustificationProperty (Array <PropertyComponent*>& props, const String& name, const Value& value, bool onlyHorizontal)
{
    StringArray strings;

    if (onlyHorizontal)
    {
        const char* const layouts[] = { "Left", "Centred", "Right", 0 };
        const var justifications[] = { (int) Justification::left, (int) Justification::centred, (int) Justification::right, 0 };

        props.add (new ChoicePropertyComponent (value, name, StringArray (layouts), Array<var> (justifications)));
    }
    else
    {
        const char* const layouts[] = { "Centred", "Centred-left", "Centred-right", "Centred-top", "Centred-bottom", "Top-left",
                                        "Top-right", "Bottom-left", "Bottom-right", 0 };
        const var justifications[] = { (int) Justification::centred, (int) Justification::centredLeft, (int) Justification::centredRight,
                                       (int) Justification::centredTop, (int) Justification::centredBottom, (int) Justification::topLeft,
                                       (int) Justification::topRight, (int) Justification::bottomLeft, (int) Justification::bottomRight, var() };

        props.add (new ChoicePropertyComponent (value, name, StringArray (layouts), Array<var> (justifications)));
    }
}

//==============================================================================
const String ComponentTypeInstance::createConstructorStatement (const String& params)
{
    String s;
    s << "addAndMakeVisible (" << getMemberName()
      << " = new " << getHandler()->getClassName (*this);

    if (params.isEmpty())
        s << "());" << newLine;
    else
    {
        s << " (";
        s << CodeHelpers::indent (params.trim(), s.length(), false) << "));" << newLine;
    }

//    s << getMemberName() << "->updateStateFrom (componentStateList.getChild ("
  //    << document.getComponentGroup().indexOf (state) << ");" << newLine;

    return s;
}
