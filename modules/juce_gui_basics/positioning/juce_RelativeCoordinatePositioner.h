/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Base class for Component::Positioners that are based upon relative coordinates.

    @tags{GUI}
*/
class JUCE_API  RelativeCoordinatePositionerBase  : public Component::Positioner,
                                                    public ComponentListener,
                                                    public MarkerList::Listener
{
public:
    RelativeCoordinatePositionerBase (Component&);
    ~RelativeCoordinatePositionerBase() override;

    void componentMovedOrResized (Component&, bool, bool) override;
    void componentParentHierarchyChanged (Component&) override;
    void componentChildrenChanged (Component&) override;
    void componentBeingDeleted (Component&) override;
    void markersChanged (MarkerList*) override;
    void markerListBeingDeleted (MarkerList*) override;

    void apply();

    bool addCoordinate (const RelativeCoordinate&);
    bool addPoint (const RelativePoint&);

    //==============================================================================
    /** Used for resolving a RelativeCoordinate expression in the context of a component. */
    class ComponentScope  : public Expression::Scope
    {
    public:
        ComponentScope (Component&);

        Expression getSymbolValue (const String& symbol) const override;
        void visitRelativeScope (const String& scopeName, Visitor&) const override;
        String getScopeUID() const override;

    protected:
        Component& component;

        Component* findSiblingComponent (const String& componentID) const;
    };

protected:
    virtual bool registerCoordinates() = 0;
    virtual void applyToComponentBounds() = 0;

private:
    class DependencyFinderScope;
    friend class DependencyFinderScope;
    Array<Component*> sourceComponents;
    Array<MarkerList*> sourceMarkerLists;
    bool registeredOk;

    void registerComponentListener (Component&);
    void registerMarkerListListener (MarkerList*);
    void unregisterListeners();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativeCoordinatePositionerBase)
};

} // namespace juce
