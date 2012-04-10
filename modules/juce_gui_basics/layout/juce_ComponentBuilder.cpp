/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

namespace ComponentBuilderHelpers
{
    static String getStateId (const ValueTree& state)
    {
        return state [ComponentBuilder::idProperty].toString();
    }

    static Component* removeComponentWithID (OwnedArray<Component>& components, const String& compId)
    {
        jassert (compId.isNotEmpty());

        for (int i = components.size(); --i >= 0;)
        {
            Component* const c = components.getUnchecked (i);

            if (c->getComponentID() == compId)
                return components.removeAndReturn (i);
        }

        return nullptr;
    }

    static Component* findComponentWithID (Component& c, const String& compId)
    {
        jassert (compId.isNotEmpty());
        if (c.getComponentID() == compId)
            return &c;

        for (int i = c.getNumChildComponents(); --i >= 0;)
        {
            Component* const child = findComponentWithID (*c.getChildComponent (i), compId);

            if (child != nullptr)
                return child;
        }

        return nullptr;
    }

    static Component* createNewComponent (ComponentBuilder::TypeHandler& type,
                                          const ValueTree& state, Component* parent)
    {
        Component* const c = type.addNewComponentFromState (state, parent);
        jassert (c != nullptr && c->getParentComponent() == parent);
        c->setComponentID (getStateId (state));
        return c;
    }

    static void updateComponent (ComponentBuilder& builder, const ValueTree& state)
    {
        Component* topLevelComp = builder.getManagedComponent();

        if (topLevelComp != nullptr)
        {
            ComponentBuilder::TypeHandler* const type = builder.getHandlerForState (state);
            const String uid (getStateId (state));

            if (type == nullptr || uid.isEmpty())
            {
                // ..handle the case where a child of the actual state node has changed.
                if (state.getParent().isValid())
                    updateComponent (builder, state.getParent());
            }
            else
            {
                Component* const changedComp = findComponentWithID (*topLevelComp, uid);

                if (changedComp != nullptr)
                    type->updateComponentFromState (changedComp, state);
            }
        }
    }

    static void updateComponentColours (Component& component, const ValueTree& colourState)
    {
        NamedValueSet& properties = component.getProperties();

        for (int i = properties.size(); --i >= 0;)
        {
            const Identifier name (properties.getName (i));

            if (name.toString().startsWith ("jcclr_"))
            {
                const String colourName (name.toString().substring (6));

                if (colourState [colourName].isVoid())
                    component.removeColour (colourName.getHexValue32());
            }
        }

        for (int i = 0; i < colourState.getNumProperties(); ++i)
        {
            const Identifier colourName (colourState.getPropertyName (i));
            const String colour (colourState [colourName].toString());

            if (colour.isNotEmpty())
                component.setColour (colourName.toString().getHexValue32(), Colour::fromString (colour));
        }
    }

    template <class ComponentClass>
    class StandardTypeHandler  : public ComponentBuilder::TypeHandler
    {
    public:
        StandardTypeHandler()  : ComponentBuilder::TypeHandler (ComponentClass::Ids::tagType)
        {}

        Component* addNewComponentFromState (const ValueTree& state, Component* parent)
        {
            ComponentClass* const c = new ComponentClass();

            if (parent != nullptr)
                parent->addAndMakeVisible (c);

            updateComponentFromState (c, state);
            return c;
        }

        void updateComponentFromState (Component* component, const ValueTree& state)
        {
            ComponentClass* const c = dynamic_cast <ComponentClass*> (component);
            jassert (c != nullptr);

            c->setComponentID (state [ComponentBuilder::idProperty]);
            c->refreshFromValueTree (state, *this->getBuilder());
        }
    };
}

//=============================================================================
const Identifier ComponentBuilder::idProperty ("id");
const Identifier ComponentBuilder::positionID ("position");

ComponentBuilder::ComponentBuilder()
    : imageProvider (nullptr)
{
}

ComponentBuilder::ComponentBuilder (const ValueTree& state_)
    : state (state_), imageProvider (nullptr)
{
    state.addListener (this);
}

ComponentBuilder::~ComponentBuilder()
{
    state.removeListener (this);

   #if JUCE_DEBUG
    // Don't delete the managed component!! The builder owns that component, and will delete
    // it automatically when it gets deleted.
    jassert (componentRef.get() == static_cast <Component*> (component));
   #endif
}

Component* ComponentBuilder::getManagedComponent()
{
    if (component == nullptr)
    {
        component = createComponent();

       #if JUCE_DEBUG
        componentRef = component;
       #endif
    }

    return component;
}

Component* ComponentBuilder::createComponent()
{
    jassert (types.size() > 0);  // You need to register all the necessary types before you can load a component!

    TypeHandler* const type = getHandlerForState (state);
    jassert (type != nullptr); // trying to create a component from an unknown type of ValueTree

    return type != nullptr ? ComponentBuilderHelpers::createNewComponent (*type, state, nullptr) : nullptr;
}

void ComponentBuilder::registerTypeHandler (ComponentBuilder::TypeHandler* const type)
{
    jassert (type != nullptr);

    // Don't try to move your types around! Once a type has been added to a builder, the
    // builder owns it, and you should leave it alone!
    jassert (type->builder == nullptr);

    types.add (type);
    type->builder = this;
}

ComponentBuilder::TypeHandler* ComponentBuilder::getHandlerForState (const ValueTree& s) const
{
    const Identifier targetType (s.getType());

    for (int i = 0; i < types.size(); ++i)
    {
        TypeHandler* const t = types.getUnchecked(i);

        if (t->type == targetType)
            return t;
    }

    return nullptr;
}

int ComponentBuilder::getNumHandlers() const noexcept
{
    return types.size();
}

ComponentBuilder::TypeHandler* ComponentBuilder::getHandler (const int index) const noexcept
{
    return types [index];
}

void ComponentBuilder::registerStandardComponentTypes()
{
    Drawable::registerDrawableTypeHandlers (*this);

    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <ComboBox>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <Slider>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <Label>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <Slider>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <TextEditor>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <GroupComponent>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <TextButton>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <ToggleButton>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <ImageButton>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <ImageComponent>());
    registerTypeHandler (new ComponentBuilderHelpers::StandardTypeHandler <HyperlinkButton>());
}

void ComponentBuilder::setImageProvider (ImageProvider* newImageProvider) noexcept
{
    imageProvider = newImageProvider;
}

ComponentBuilder::ImageProvider* ComponentBuilder::getImageProvider() const noexcept
{
    return imageProvider;
}

void ComponentBuilder::valueTreePropertyChanged (ValueTree& tree, const Identifier&)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

void ComponentBuilder::valueTreeChildAdded (ValueTree& tree, ValueTree&)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

void ComponentBuilder::valueTreeChildRemoved (ValueTree& tree, ValueTree&)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

void ComponentBuilder::valueTreeChildOrderChanged (ValueTree& tree)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

void ComponentBuilder::valueTreeParentChanged (ValueTree& tree)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

//==============================================================================
ComponentBuilder::TypeHandler::TypeHandler (const Identifier& valueTreeType)
   : type (valueTreeType), builder (nullptr)
{
}

ComponentBuilder::TypeHandler::~TypeHandler()
{
}

ComponentBuilder* ComponentBuilder::TypeHandler::getBuilder() const noexcept
{
    // A type handler needs to be registered with a ComponentBuilder before using it!
    jassert (builder != nullptr);
    return builder;
}

void ComponentBuilder::updateChildComponents (Component& parent, const ValueTree& children)
{
    using namespace ComponentBuilderHelpers;

    const int numExistingChildComps = parent.getNumChildComponents();

    Array <Component*> componentsInOrder;
    componentsInOrder.ensureStorageAllocated (numExistingChildComps);

    {
        OwnedArray<Component> existingComponents;
        existingComponents.ensureStorageAllocated (numExistingChildComps);

        int i;
        for (i = 0; i < numExistingChildComps; ++i)
            existingComponents.add (parent.getChildComponent (i));

        const int newNumChildren = children.getNumChildren();
        for (i = 0; i < newNumChildren; ++i)
        {
            const ValueTree childState (children.getChild (i));
            Component* c = removeComponentWithID (existingComponents, getStateId (childState));

            if (c == nullptr)
            {
                TypeHandler* const type = getHandlerForState (childState);
                jassert (type != nullptr);

                if (type != nullptr)
                    c = ComponentBuilderHelpers::createNewComponent (*type, childState, &parent);
            }

            if (c != nullptr)
                componentsInOrder.add (c);
        }

        // (remaining unused items in existingComponents get deleted here as it goes out of scope)
    }

    // Make sure the z-order is correct..
    if (componentsInOrder.size() > 0)
    {
        componentsInOrder.getLast()->toFront (false);

        for (int i = componentsInOrder.size() - 1; --i >= 0;)
            componentsInOrder.getUnchecked(i)->toBehind (componentsInOrder.getUnchecked (i + 1));
    }
}

static void updateMarkers (MarkerList* const list, const ValueTree& state)
{
    if (list != nullptr)
        MarkerList::ValueTreeWrapper (state).applyTo (*list);
}

void ComponentBuilder::initialiseRecursively (Component& comp, const ValueTree& state)
{
    refreshBasicComponentProperties (comp, state);

    updateMarkers (comp.getMarkers (true),  state.getChildWithName ("MARKERS_X"));
    updateMarkers (comp.getMarkers (false), state.getChildWithName ("MARKERS_Y"));

    const ValueTree childList (state.getChildWithName ("COMPONENTS"));

    if (childList.isValid())
    {
        updateChildComponents (comp, childList);

        for (int i = 0; i < childList.getNumChildren(); ++i)
        {
            const ValueTree childState (childList.getChild(i));
            Component* const c = ComponentBuilderHelpers::findComponentWithID (comp, ComponentBuilderHelpers::getStateId (childState));

            if (c != nullptr)
            {
                ComponentBuilder::TypeHandler* const type = getHandlerForState (childState);

                if (type != nullptr)
                    type->updateComponentFromState (c, childState);
                else
                    initialiseRecursively (*c, childState);
            }
        }
    }
}

void ComponentBuilder::initialiseFromValueTree (Component& comp,
                                                const ValueTree& state,
                                                ImageProvider* const imageProvider)
{
    ComponentBuilder builder;
    builder.setImageProvider (imageProvider);
    builder.registerStandardComponentTypes();
    builder.initialiseRecursively (comp, state);
}

RelativeRectangle ComponentBuilder::getComponentBounds (const ValueTree& state)
{
    try
    {
        return RelativeRectangle (state [positionID].toString());
    }
    catch (Expression::ParseError&)
    {}

    return RelativeRectangle();
}

void ComponentBuilder::refreshBasicComponentProperties (Component& comp, const ValueTree& state)
{
    static const Identifier focusOrderID ("focusOrder");
    static const Identifier tooltipID ("tooltip");
    static const Identifier nameID ("name");

    comp.setName (state [nameID].toString());

    if (state.hasProperty (positionID))
        getComponentBounds (state).applyToComponent (comp);

    comp.setExplicitFocusOrder (state [focusOrderID]);
    const var tip (state [tooltipID]);

    if (! tip.isVoid())
    {
        SettableTooltipClient* tooltipClient = dynamic_cast <SettableTooltipClient*> (&comp);
        if (tooltipClient != nullptr)
            tooltipClient->setTooltip (tip.toString());
    }

    ComponentBuilderHelpers::updateComponentColours (comp, state.getChildWithName ("COLOURS"));
}
