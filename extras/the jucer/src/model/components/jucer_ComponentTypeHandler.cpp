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

#include "../../jucer_Headers.h"
#include "../jucer_ObjectTypes.h"
#include "../../ui/jucer_ComponentOverlayComponent.h"
#include "jucer_ComponentNameProperty.h"
#include "../../properties/jucer_PositionPropertyBase.h"
#include "../../properties/jucer_ComponentColourProperty.h"
#include "../../ui/jucer_TestComponent.h"

#if JUCE_MSVC
  #define type_name     raw_name
#else
  #define type_name     name
#endif


//==============================================================================
ComponentTypeHandler::ComponentTypeHandler (const String& typeName_,
                                            const String& className_,
                                            const std::type_info& componentClass_,
                                            const int defaultWidth_,
                                            const int defaultHeight_)
    : typeName (typeName_),
      className (className_),
      componentClassRawName (componentClass_.type_name()),
      defaultWidth (defaultWidth_),
      defaultHeight (defaultHeight_)
{
}

ComponentTypeHandler::~ComponentTypeHandler()
{
}

Component* ComponentTypeHandler::createCopyOf (JucerDocument* document, Component& existing)
{
    jassert (getHandlerFor (existing) == this);

    Component* const newOne = createNewComponent (document);
    XmlElement* const xml = createXmlFor (&existing, document->getComponentLayout());

    if (xml != 0)
    {
        restoreFromXml (*xml, newOne, document->getComponentLayout());
        delete xml;
    }

    return newOne;
}

ComponentOverlayComponent* ComponentTypeHandler::createOverlayComponent (Component* child, ComponentLayout& layout)
{
    return new ComponentOverlayComponent (child, layout);
}

void ComponentTypeHandler::showPopupMenu (Component* component,
                                          ComponentLayout& layout)
{
    PopupMenu m;

    m.addCommandItem (commandManager, CommandIDs::toFront);
    m.addCommandItem (commandManager, CommandIDs::toBack);
    m.addSeparator();
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    m.addCommandItem (commandManager, StandardApplicationCommandIDs::del);

    m.show();
}

JucerDocument* ComponentTypeHandler::findParentDocument (Component* component)
{
    Component* p = component->getParentComponent();

    while (p != 0)
    {
        JucerDocumentHolder* const dh = dynamic_cast <JucerDocumentHolder*> (p);

        if (dh != 0)
            return dh->getDocument();

        TestComponent* const t = dynamic_cast <TestComponent*> (p);

        if (t != 0)
            return t->getDocument();

        p = p->getParentComponent();
    }

    return 0;
}

//==============================================================================
bool ComponentTypeHandler::canHandle (Component& component) const
{
    return componentClassRawName == String (typeid (component).type_name());
}

ComponentTypeHandler* ComponentTypeHandler::getHandlerFor (Component& component)
{
    for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
        if (ObjectTypes::componentTypeHandlers[i]->canHandle (component))
            return ObjectTypes::componentTypeHandlers[i];

    jassertfalse
    return 0;
}

ComponentTypeHandler* ComponentTypeHandler::getHandlerForXmlTag (const String& tagName)
{
    for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
        if (ObjectTypes::componentTypeHandlers[i]->getXmlTagName().equalsIgnoreCase (tagName))
            return ObjectTypes::componentTypeHandlers[i];

    return 0;
}

XmlElement* ComponentTypeHandler::createXmlFor (Component* comp, const ComponentLayout* layout)
{
    XmlElement* e = new XmlElement (getXmlTagName());

    e->setAttribute (T("name"), comp->getName());
    e->setAttribute (T("id"), String::toHexString (getComponentId (comp)));
    e->setAttribute (T("memberName"), comp->getComponentProperty (T("memberName"), false));
    e->setAttribute (T("virtualName"), comp->getComponentProperty (T("virtualName"), false));
    e->setAttribute (T("explicitFocusOrder"), comp->getExplicitFocusOrder());

    RelativePositionedRectangle pos (getComponentPosition (comp));
    pos.updateFromComponent (*comp, layout);
    pos.applyToXml (*e);

    SettableTooltipClient* const ttc = dynamic_cast <SettableTooltipClient*> (comp);
    if (ttc != 0 && ttc->getTooltip().isNotEmpty())
        e->setAttribute (T("tooltip"), ttc->getTooltip());

    for (int i = 0; i < colours.size(); ++i)
    {
        if (comp->isColourSpecified (colours[i]->colourId))
        {
            e->setAttribute (colours[i]->xmlTagName,
                             colourToHex (comp->findColour (colours[i]->colourId)));
        }
    }

    return e;
}

bool ComponentTypeHandler::restoreFromXml (const XmlElement& xml,
                                           Component* comp,
                                           const ComponentLayout* layout)
{
    jassert (xml.hasTagName (getXmlTagName()));

    if (! xml.hasTagName (getXmlTagName()))
        return false;

    comp->setName (xml.getStringAttribute (T("name"), comp->getName()));
    setComponentId (comp, xml.getStringAttribute (T("id")).getHexValue64());
    comp->setComponentProperty (T("memberName"), xml.getStringAttribute (T("memberName")));
    comp->setComponentProperty (T("virtualName"), xml.getStringAttribute (T("virtualName")));
    comp->setExplicitFocusOrder (xml.getIntAttribute (T("explicitFocusOrder")));

    RelativePositionedRectangle currentPos (getComponentPosition (comp));
    currentPos.updateFromComponent (*comp, layout);

    RelativePositionedRectangle rpr;
    rpr.restoreFromXml (xml, currentPos);

    jassert (layout != 0);
    setComponentPosition (comp, rpr, layout);

    SettableTooltipClient* const ttc = dynamic_cast <SettableTooltipClient*> (comp);
    if (ttc != 0)
        ttc->setTooltip (xml.getStringAttribute (T("tooltip")));

    for (int i = 0; i < colours.size(); ++i)
    {
        const String col (xml.getStringAttribute (colours[i]->xmlTagName, String::empty));

        if (col.isNotEmpty())
        {
            comp->setColour (colours[i]->colourId,
                             Colour (col.getHexValue32()));
        }
    }

    return true;
}

//==============================================================================
int64 ComponentTypeHandler::getComponentId (Component* comp)
{
    if (comp == 0)
        return 0;

    int64 compId = comp->getComponentProperty (T("jucerCompId"), false).getHexValue64();

    if (compId == 0)
    {
        compId = Random::getSystemRandom().nextInt64();
        setComponentId (comp, compId);
    }

    return compId;
}

void ComponentTypeHandler::setComponentId (Component* comp, const int64 newID)
{
    jassert (comp != 0);
    if (newID != 0)
        comp->setComponentProperty (T("jucerCompId"), String::toHexString (newID));
}

const RelativePositionedRectangle ComponentTypeHandler::getComponentPosition (Component* comp)
{
    RelativePositionedRectangle rp;
    rp.rect = PositionedRectangle (comp->getComponentProperty (T("pos"), false));
    rp.relativeToX = comp->getComponentProperty (T("relativeToX"), false).getHexValue64();
    rp.relativeToY = comp->getComponentProperty (T("relativeToY"), false).getHexValue64();
    rp.relativeToW = comp->getComponentProperty (T("relativeToW"), false).getHexValue64();
    rp.relativeToH = comp->getComponentProperty (T("relativeToH"), false).getHexValue64();

    return rp;
}

void ComponentTypeHandler::setComponentPosition (Component* comp,
                                                 const RelativePositionedRectangle& newPos,
                                                 const ComponentLayout* layout)
{
    comp->setComponentProperty (T("pos"), newPos.rect.toString());
    comp->setComponentProperty (T("relativeToX"), String::toHexString (newPos.relativeToX));
    comp->setComponentProperty (T("relativeToY"), String::toHexString (newPos.relativeToY));
    comp->setComponentProperty (T("relativeToW"), String::toHexString (newPos.relativeToW));
    comp->setComponentProperty (T("relativeToH"), String::toHexString (newPos.relativeToH));

    comp->setBounds (newPos.getRectangle (Rectangle (0, 0, comp->getParentWidth(), comp->getParentHeight()),
                                          layout));
}

//==============================================================================
class TooltipProperty   : public ComponentTextProperty <Component>
{
public:
    TooltipProperty (Component* comp, JucerDocument& document)
        : ComponentTextProperty <Component> (T("tooltip"), 1024, false, comp, document)
    {
    }

    const String getText() const
    {
        SettableTooltipClient* ttc = dynamic_cast <SettableTooltipClient*> (component);
        return ttc->getTooltip();
    }

    void setText (const String& newText)
    {
        document.perform (new SetTooltipAction (component, *document.getComponentLayout(), newText),
                          T("Change tooltip"));
    }

private:
    class SetTooltipAction  : public ComponentUndoableAction <Component>
    {
    public:
        SetTooltipAction (Component* const comp, ComponentLayout& layout, const String& newValue_)
            : ComponentUndoableAction <Component> (comp, layout),
              newValue (newValue_)
        {
            SettableTooltipClient* ttc = dynamic_cast <SettableTooltipClient*> (comp);
            jassert (ttc != 0);
            oldValue = ttc->getTooltip();
        }

        bool perform()
        {
            showCorrectTab();
            SettableTooltipClient* ttc = dynamic_cast <SettableTooltipClient*> (getComponent());

            jassert (ttc != 0);
            if (ttc == 0)
                return false;

            ttc->setTooltip (newValue);
            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            SettableTooltipClient* ttc = dynamic_cast <SettableTooltipClient*> (getComponent());

            jassert (ttc != 0);
            if (ttc == 0)
                return false;

            ttc->setTooltip (oldValue);
            changed();
            return true;
        }

        String newValue, oldValue;
    };
};

//==============================================================================
class ComponentPositionProperty   : public PositionPropertyBase
{
public:
    //==============================================================================
    ComponentPositionProperty (Component* component_,
                               JucerDocument& document_,
                               const String& name,
                               ComponentPositionDimension dimension_)
        : PositionPropertyBase (component_, name, dimension_,
                                true, true,
                                document_.getComponentLayout()),
          document (document_)
    {
        document.addChangeListener (this);
    }

    ~ComponentPositionProperty()
    {
        document.removeChangeListener (this);
    }

    //==============================================================================
    void setPosition (const RelativePositionedRectangle& newPos)
    {
        document.getComponentLayout()->setComponentPosition (component, newPos, true);
    }

    const RelativePositionedRectangle getPosition() const
    {
        return ComponentTypeHandler::getComponentPosition (component);
    }

private:
    JucerDocument& document;
};


//==============================================================================
class FocusOrderProperty   : public ComponentTextProperty <Component>
{
public:
    FocusOrderProperty (Component* comp, JucerDocument& document)
        : ComponentTextProperty <Component> (T("focus order"), 8, false, comp, document)
    {
    }

    const String getText() const
    {
        return String (component->getExplicitFocusOrder());
    }

    void setText (const String& newText)
    {
        document.perform (new SetFocusOrderAction (component, *document.getComponentLayout(), jmax (0, newText.getIntValue())),
                          T("Change focus order"));
    }

private:
    class SetFocusOrderAction  : public ComponentUndoableAction <Component>
    {
    public:
        SetFocusOrderAction (Component* const comp, ComponentLayout& layout, const int newOrder_)
            : ComponentUndoableAction <Component> (comp, layout),
              newValue (newOrder_)
        {
            oldValue = comp->getExplicitFocusOrder();
        }

        bool perform()
        {
            showCorrectTab();
            getComponent()->setExplicitFocusOrder (newValue);
            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getComponent()->setExplicitFocusOrder (oldValue);
            changed();
            return true;
        }

        int newValue, oldValue;
    };
};

//==============================================================================
void ComponentTypeHandler::getEditableProperties (Component* component,
                                                  JucerDocument& document,
                                                  Array <PropertyComponent*>& properties)
{
    properties.add (new ComponentMemberNameProperty (component, document));
    properties.add (new ComponentNameProperty (component, document));
    properties.add (new ComponentVirtualClassProperty (component, document));

    properties.add (new ComponentPositionProperty (component, document, T("x"), ComponentPositionProperty::componentX));
    properties.add (new ComponentPositionProperty (component, document, T("y"), ComponentPositionProperty::componentY));
    properties.add (new ComponentPositionProperty (component, document, T("width"), ComponentPositionProperty::componentWidth));
    properties.add (new ComponentPositionProperty (component, document, T("height"), ComponentPositionProperty::componentHeight));

    if (dynamic_cast <SettableTooltipClient*> (component) != 0)
        properties.add (new TooltipProperty (component, document));

    properties.add (new FocusOrderProperty (component, document));
}

void ComponentTypeHandler::addPropertiesToPropertyPanel (Component* comp,
                                                         JucerDocument& document,
                                                         PropertyPanel& panel)
{
    Array <PropertyComponent*> props;
    getEditableProperties (comp, document, props);

    panel.addSection (getClassName (comp), props);
}

void ComponentTypeHandler::registerEditableColour (int colourId,
                                                   const String& colourIdCode,
                                                   const String& colourName, const String& xmlTagName)
{
    ComponentColourInfo* const c = new ComponentColourInfo();

    c->colourId = colourId;
    c->colourIdCode = colourIdCode;
    c->colourName = colourName;
    c->xmlTagName = xmlTagName;

    colours.add (c);
}

void ComponentTypeHandler::addColourProperties (Component* component,
                                                JucerDocument& document,
                                                Array <PropertyComponent*>& properties)
{
    for (int i = 0; i < colours.size(); ++i)
    {
        properties.add (new ComponentColourIdProperty (component, document,
                                                       colours[i]->colourId,
                                                       colours[i]->colourName,
                                                       true));
    }
}

const String ComponentTypeHandler::getColourIntialisationCode (Component* component,
                                                               const String& objectName)
{
    String s;

    for (int i = 0; i < colours.size(); ++i)
    {
        if (component->isColourSpecified (colours[i]->colourId))
        {
            s << objectName << T("->setColour (")
              << colours[i]->colourIdCode
              << T(", ")
              << colourToCode (component->findColour (colours[i]->colourId))
              << T(");\n");
        }
    }

    return s;
}

//==============================================================================
void ComponentTypeHandler::fillInGeneratedCode (Component* component, GeneratedCode& code)
{
    const String memberVariableName (code.document->getComponentLayout()->getComponentMemberVariableName (component));

    fillInMemberVariableDeclarations (code, component, memberVariableName);
    fillInCreationCode (code, component, memberVariableName);
    fillInDeletionCode (code, component, memberVariableName);
    fillInResizeCode (code, component, memberVariableName);
}

void ComponentTypeHandler::fillInMemberVariableDeclarations (GeneratedCode& code, Component* component, const String& memberVariableName)
{
    const String virtualName (component->getComponentProperty (T("virtualName"), false));

    if (virtualName.isNotEmpty())
    {
        code.privateMemberDeclarations
            << makeValidCppIdentifier (virtualName, false, false, true);
    }
    else
    {
        code.privateMemberDeclarations
            << getClassName (component);
    }

    code.privateMemberDeclarations
        << T("* ") << memberVariableName << T(";\n");

    code.initialisers.add (memberVariableName + T(" (0)"));
}

void ComponentTypeHandler::fillInResizeCode (GeneratedCode& code, Component* component, const String& memberVariableName)
{
    const RelativePositionedRectangle pos (getComponentPosition (component));

    String x, y, w, h, r;
    positionToCode (pos, code.document->getComponentLayout(), x, y, w, h);

    r << memberVariableName << "->setBounds ("
      << x << ", " << y << ", " << w << ", " << h << ");\n";

    if (pos.rect.isPositionAbsolute())
        code.constructorCode += r + T("\n");
    else
        code.getCallbackCode (String::empty, T("void"), T("resized()"), false) += r;
}

const String ComponentTypeHandler::getCreationParameters (Component* component)
{
    return String::empty;
}

void ComponentTypeHandler::fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
{
    String params (getCreationParameters (component));
    const String virtualName (component->getComponentProperty (T("virtualName"), false));

    String s;
    s << "addAndMakeVisible (" << memberVariableName << " = new ";

    if (virtualName.isNotEmpty())
        s << makeValidCppIdentifier (virtualName, false, false, true);
    else
        s << getClassName (component);

    if (params.isEmpty())
        s << "());\n";
    else
    {
        StringArray lines;
        lines.addLines (params);

        params = lines.joinIntoString (T("\n") + String::repeatedString (T(" "), s.length() + 2));

        s << " (" << params << "));\n";
    }

    SettableTooltipClient* ttc = dynamic_cast <SettableTooltipClient*> (component);
    if (ttc != 0 && ttc->getTooltip().isNotEmpty())
    {
        s << memberVariableName << "->setTooltip ("
          << quotedString (ttc->getTooltip())
          << ");\n";
    }

    if (component->getExplicitFocusOrder() > 0)
        s << memberVariableName << "->setExplicitFocusOrder ("
          << component->getExplicitFocusOrder()
          << ");\n";

    code.constructorCode += s;
}

void ComponentTypeHandler::fillInDeletionCode (GeneratedCode& code, Component* component, const String& memberVariableName)
{
    code.destructorCode
        << "deleteAndZero (" << memberVariableName << ");\n";
}
