/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_RELATIVECOORDINATEPOSITIONER_H_INCLUDED
#define JUCE_RELATIVECOORDINATEPOSITIONER_H_INCLUDED


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


#endif   // JUCE_RELATIVECOORDINATEPOSITIONER_H_INCLUDED
