/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
            if (Component* const child = findComponentWithID (*c.getChildComponent (i), compId))
                return child;

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
        if (Component* topLevelComp = builder.getManagedComponent())
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
                if (Component* const changedComp = findComponentWithID (*topLevelComp, uid))
                    type->updateComponentFromState (changedComp, state);
            }
        }
    }
}

//=============================================================================
const Identifier ComponentBuilder::idProperty ("id");

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

    if (TypeHandler* const type = getHandlerForState (state))
        return ComponentBuilderHelpers::createNewComponent (*type, state, nullptr);

    jassertfalse; // trying to create a component from an unknown type of ValueTree
    return nullptr;
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

        for (int i = 0; i < numExistingChildComps; ++i)
            existingComponents.add (parent.getChildComponent (i));

        const int newNumChildren = children.getNumChildren();
        for (int i = 0; i < newNumChildren; ++i)
        {
            const ValueTree childState (children.getChild (i));
            Component* c = removeComponentWithID (existingComponents, getStateId (childState));

            if (c == nullptr)
            {
                if (TypeHandler* const type = getHandlerForState (childState))
                    c = ComponentBuilderHelpers::createNewComponent (*type, childState, &parent);
                else
                    jassertfalse;
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
