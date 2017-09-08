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

struct MarkerListScope  : public Expression::Scope
{
    MarkerListScope (Component& comp) : component (comp) {}

    // Suppress a VS2013 compiler warning
    MarkerListScope& operator= (const MarkerListScope&) = delete;

    Expression getSymbolValue (const String& symbol) const override
    {
        switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
        {
            case RelativeCoordinate::StandardStrings::width:  return Expression ((double) component.getWidth());
            case RelativeCoordinate::StandardStrings::height: return Expression ((double) component.getHeight());
            default: break;
        }

        MarkerList* list;

        if (auto* marker = findMarker (component, symbol, list))
            return Expression (marker->position.getExpression().evaluate (*this));

        return Expression::Scope::getSymbolValue (symbol);
    }

    void visitRelativeScope (const String& scopeName, Visitor& visitor) const override
    {
        if (scopeName == RelativeCoordinate::Strings::parent)
        {
            if (auto* parent = component.getParentComponent())
            {
                visitor.visit (MarkerListScope (*parent));
                return;
            }
        }

        Expression::Scope::visitRelativeScope (scopeName, visitor);
    }

    String getScopeUID() const override
    {
        return String::toHexString ((pointer_sized_int) (void*) &component) + "m";
    }

    static const MarkerList::Marker* findMarker (Component& component, const String& name, MarkerList*& list)
    {
        const MarkerList::Marker* marker = nullptr;
        list = component.getMarkers (true);

        if (list != nullptr)
            marker = list->getMarker (name);

        if (marker == nullptr)
        {
            list = component.getMarkers (false);

            if (list != nullptr)
                marker = list->getMarker (name);
        }

        return marker;
    }

    Component& component;
};

//==============================================================================
RelativeCoordinatePositionerBase::ComponentScope::ComponentScope (Component& comp)
    : component (comp)
{
}

Expression RelativeCoordinatePositionerBase::ComponentScope::getSymbolValue (const String& symbol) const
{
    switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
    {
        case RelativeCoordinate::StandardStrings::x:
        case RelativeCoordinate::StandardStrings::left:   return Expression ((double) component.getX());
        case RelativeCoordinate::StandardStrings::y:
        case RelativeCoordinate::StandardStrings::top:    return Expression ((double) component.getY());
        case RelativeCoordinate::StandardStrings::width:  return Expression ((double) component.getWidth());
        case RelativeCoordinate::StandardStrings::height: return Expression ((double) component.getHeight());
        case RelativeCoordinate::StandardStrings::right:  return Expression ((double) component.getRight());
        case RelativeCoordinate::StandardStrings::bottom: return Expression ((double) component.getBottom());
        default: break;
    }

    if (Component* const parent = component.getParentComponent())
    {
        MarkerList* list;

        if (const MarkerList::Marker* const marker = MarkerListScope::findMarker (*parent, symbol, list))
        {
            MarkerListScope scope (*parent);
            return Expression (marker->position.getExpression().evaluate (scope));
        }
    }

    return Expression::Scope::getSymbolValue (symbol);
}

void RelativeCoordinatePositionerBase::ComponentScope::visitRelativeScope (const String& scopeName, Visitor& visitor) const
{
    if (auto* targetComp = (scopeName == RelativeCoordinate::Strings::parent)
                               ? component.getParentComponent()
                               : findSiblingComponent (scopeName))
        visitor.visit (ComponentScope (*targetComp));
    else
        Expression::Scope::visitRelativeScope (scopeName, visitor);
}

String RelativeCoordinatePositionerBase::ComponentScope::getScopeUID() const
{
    return String::toHexString ((pointer_sized_int) (void*) &component);
}

Component* RelativeCoordinatePositionerBase::ComponentScope::findSiblingComponent (const String& componentID) const
{
    if (Component* const parent = component.getParentComponent())
        return parent->findChildWithID (componentID);

    return nullptr;
}

//==============================================================================
class RelativeCoordinatePositionerBase::DependencyFinderScope  : public ComponentScope
{
public:
    DependencyFinderScope (Component& comp, RelativeCoordinatePositionerBase& p, bool& result)
        : ComponentScope (comp), positioner (p), ok (result)
    {
    }

    Expression getSymbolValue (const String& symbol) const override
    {
        switch (RelativeCoordinate::StandardStrings::getTypeOf (symbol))
        {
            case RelativeCoordinate::StandardStrings::x:
            case RelativeCoordinate::StandardStrings::left:
            case RelativeCoordinate::StandardStrings::y:
            case RelativeCoordinate::StandardStrings::top:
            case RelativeCoordinate::StandardStrings::width:
            case RelativeCoordinate::StandardStrings::height:
            case RelativeCoordinate::StandardStrings::right:
            case RelativeCoordinate::StandardStrings::bottom:
                positioner.registerComponentListener (component);
                break;

            default:
                if (Component* const parent = component.getParentComponent())
                {
                    MarkerList* list;

                    if (MarkerListScope::findMarker (*parent, symbol, list) != nullptr)
                    {
                        positioner.registerMarkerListListener (list);
                    }
                    else
                    {
                        // The marker we want doesn't exist, so watch all lists in case they change and the marker appears later..
                        positioner.registerMarkerListListener (parent->getMarkers (true));
                        positioner.registerMarkerListListener (parent->getMarkers (false));
                        ok = false;
                    }
                }
                break;
        }

        return ComponentScope::getSymbolValue (symbol);
    }

    void visitRelativeScope (const String& scopeName, Visitor& visitor) const override
    {
        if (Component* const targetComp = (scopeName == RelativeCoordinate::Strings::parent)
                                                ? component.getParentComponent()
                                                : findSiblingComponent (scopeName))
        {
            visitor.visit (DependencyFinderScope (*targetComp, positioner, ok));
        }
        else
        {
            // The named component doesn't exist, so we'll watch the parent for changes in case it appears later..
            if (Component* const parent = component.getParentComponent())
                positioner.registerComponentListener (*parent);

            positioner.registerComponentListener (component);
            ok = false;
        }
    }

private:
    RelativeCoordinatePositionerBase& positioner;
    bool& ok;
};

//==============================================================================
RelativeCoordinatePositionerBase::RelativeCoordinatePositionerBase (Component& comp)
    : Component::Positioner (comp), registeredOk (false)
{
}

RelativeCoordinatePositionerBase::~RelativeCoordinatePositionerBase()
{
    unregisterListeners();
}

void RelativeCoordinatePositionerBase::componentMovedOrResized (Component&, bool /*wasMoved*/, bool /*wasResized*/)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentParentHierarchyChanged (Component&)
{
    apply();
}

void RelativeCoordinatePositionerBase::componentChildrenChanged (Component& changed)
{
    if (getComponent().getParentComponent() == &changed && ! registeredOk)
        apply();
}

void RelativeCoordinatePositionerBase::componentBeingDeleted (Component& comp)
{
    jassert (sourceComponents.contains (&comp));
    sourceComponents.removeFirstMatchingValue (&comp);
    registeredOk = false;
}

void RelativeCoordinatePositionerBase::markersChanged (MarkerList*)
{
    apply();
}

void RelativeCoordinatePositionerBase::markerListBeingDeleted (MarkerList* markerList)
{
    jassert (sourceMarkerLists.contains (markerList));
    sourceMarkerLists.removeFirstMatchingValue (markerList);
}

void RelativeCoordinatePositionerBase::apply()
{
    if (! registeredOk)
    {
        unregisterListeners();
        registeredOk = registerCoordinates();
    }

    applyToComponentBounds();
}

bool RelativeCoordinatePositionerBase::addCoordinate (const RelativeCoordinate& coord)
{
    bool ok = true;
    DependencyFinderScope finderScope (getComponent(), *this, ok);
    coord.getExpression().evaluate (finderScope);
    return ok;
}

bool RelativeCoordinatePositionerBase::addPoint (const RelativePoint& point)
{
    const bool ok = addCoordinate (point.x);
    return addCoordinate (point.y) && ok;
}

void RelativeCoordinatePositionerBase::registerComponentListener (Component& comp)
{
    if (! sourceComponents.contains (&comp))
    {
        comp.addComponentListener (this);
        sourceComponents.add (&comp);
    }
}

void RelativeCoordinatePositionerBase::registerMarkerListListener (MarkerList* const list)
{
    if (list != nullptr && ! sourceMarkerLists.contains (list))
    {
        list->addListener (this);
        sourceMarkerLists.add (list);
    }
}

void RelativeCoordinatePositionerBase::unregisterListeners()
{
    for (int i = sourceComponents.size(); --i >= 0;)
        sourceComponents.getUnchecked(i)->removeComponentListener (this);

    for (int i = sourceMarkerLists.size(); --i >= 0;)
        sourceMarkerLists.getUnchecked(i)->removeListener (this);

    sourceComponents.clear();
    sourceMarkerLists.clear();
}

} // namespace juce
