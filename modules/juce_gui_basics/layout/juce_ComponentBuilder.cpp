/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

        for (auto* child : c.getChildren())
            if (auto* found = findComponentWithID (*child, compId))
                return found;

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

//==============================================================================
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
    jassert (componentRef.get() == component.get());
   #endif
}

Component* ComponentBuilder::getManagedComponent()
{
    if (component == nullptr)
    {
        component.reset (createComponent());

       #if JUCE_DEBUG
        componentRef = component.get();
       #endif
    }

    return component.get();
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

void ComponentBuilder::valueTreeChildRemoved (ValueTree& tree, ValueTree&, int)
{
    ComponentBuilderHelpers::updateComponent (*this, tree);
}

void ComponentBuilder::valueTreeChildOrderChanged (ValueTree& tree, int, int)
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

    auto numExistingChildComps = parent.getNumChildComponents();

    Array<Component*> componentsInOrder;
    componentsInOrder.ensureStorageAllocated (numExistingChildComps);

    {
        OwnedArray<Component> existingComponents;
        existingComponents.ensureStorageAllocated (numExistingChildComps);

        for (int i = 0; i < numExistingChildComps; ++i)
            existingComponents.add (parent.getChildComponent (i));

        auto newNumChildren = children.getNumChildren();

        for (int i = 0; i < newNumChildren; ++i)
        {
            auto childState = children.getChild (i);
            auto* c = removeComponentWithID (existingComponents, getStateId (childState));

            if (c == nullptr)
            {
                if (auto* type = getHandlerForState (childState))
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

} // namespace juce
