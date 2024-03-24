/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
