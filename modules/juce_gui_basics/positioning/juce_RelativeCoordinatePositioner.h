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

#pragma once


//==============================================================================
/**
    Base class for Component::Positioners that are based upon relative coordinates.
*/
class JUCE_API  RelativeCoordinatePositionerBase  : public Component::Positioner,
                                                    public ComponentListener,
                                                    public MarkerList::Listener
{
public:
    RelativeCoordinatePositionerBase (Component& component);
    ~RelativeCoordinatePositionerBase();

    void componentMovedOrResized (Component&, bool, bool);
    void componentParentHierarchyChanged (Component&);
    void componentChildrenChanged (Component&);
    void componentBeingDeleted (Component&);
    void markersChanged (MarkerList*);
    void markerListBeingDeleted (MarkerList* markerList);

    void apply();

    bool addCoordinate (const RelativeCoordinate& coord);
    bool addPoint (const RelativePoint& point);

    //==============================================================================
    /** Used for resolving a RelativeCoordinate expression in the context of a component. */
    class ComponentScope  : public Expression::Scope
    {
    public:
        ComponentScope (Component& component);

        Expression getSymbolValue (const String& symbol) const;
        void visitRelativeScope (const String& scopeName, Visitor& visitor) const;
        String getScopeUID() const;

    protected:
        Component& component;

        Component* findSiblingComponent (const String& componentID) const;

    private:
        JUCE_DECLARE_NON_COPYABLE (ComponentScope)
    };

protected:
    virtual bool registerCoordinates() = 0;
    virtual void applyToComponentBounds() = 0;

private:
    class DependencyFinderScope;
    friend class DependencyFinderScope;
    Array <Component*> sourceComponents;
    Array <MarkerList*> sourceMarkerLists;
    bool registeredOk;

    void registerComponentListener (Component& comp);
    void registerMarkerListListener (MarkerList* const list);
    void unregisterListeners();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeCoordinatePositionerBase)
};
