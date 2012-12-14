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

#ifndef __JUCE_RELATIVECOORDINATEPOSITIONER_JUCEHEADER__
#define __JUCE_RELATIVECOORDINATEPOSITIONER_JUCEHEADER__

#include "juce_RelativePoint.h"
#include "juce_MarkerList.h"
#include "../components/juce_Component.h"


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
    void componentChildrenChanged (Component& component);
    void componentBeingDeleted (Component& component);
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


#endif   // __JUCE_RELATIVECOORDINATEPOSITIONER_JUCEHEADER__
