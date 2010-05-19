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

#ifndef __JUCER_COORDINATE_H_EF56ACFA__
#define __JUCER_COORDINATE_H_EF56ACFA__

#include "../jucer_Headers.h"



//==============================================================================
/**
*/
class ComponentAutoLayoutManager    : public ComponentListener,
                                      public RelativeCoordinate::NamedCoordinateFinder,
                                      public AsyncUpdater
{
public:
    //==============================================================================
    /**
    */
    ComponentAutoLayoutManager (Component* parentComponent);

    /** Destructor. */
    ~ComponentAutoLayoutManager();

    //==============================================================================
    /**
    */
    void setMarker (const String& name, const RelativeCoordinate& coord);

    /**
    */
    void setComponentBounds (Component* component, const String& componentName, const RelativeRectangle& bounds);

    /**
    */
    void applyLayout();

    //==============================================================================
    /** @internal */
    const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;
    /** @internal */
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized);
    /** @internal */
    void componentBeingDeleted (Component& component);
    /** @internal */
    void handleAsyncUpdate();

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    struct ComponentPosition
    {
        ComponentPosition (Component* component, const String& name, const RelativeRectangle& coords);

        Component* component;
        String name;
        RelativeRectangle coords;
    };

    struct MarkerPosition
    {
        MarkerPosition (const String& name, const RelativeCoordinate& coord);

        String markerName;
        RelativeCoordinate position;
    };

    Component* parent;
    OwnedArray <ComponentPosition> components;
    OwnedArray <MarkerPosition> markers;

    ComponentAutoLayoutManager (const ComponentAutoLayoutManager&);
    ComponentAutoLayoutManager& operator= (const ComponentAutoLayoutManager&);
};


#endif  // __JUCER_COORDINATE_H_EF56ACFA__
