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

#ifndef JUCE_COMPONENT_H_INCLUDED
#define JUCE_COMPONENT_H_INCLUDED


//==============================================================================
/**
    The base class for all JUCE user-interface objects.
*/
class JUCE_API  Component  : public MouseListener
{
public:
    //==============================================================================
    /** Creates a component.

        To get it to actually appear, you'll also need to:
        - Either add it to a parent component or use the addToDesktop() method to
          make it a desktop window
        - Set its size and position to something sensible
        - Use setVisible() to make it visible

        And for it to serve any useful purpose, you'll need to write a
        subclass of Component or use one of the other types of component from
        the library.
    */
    Component() noexcept;

    /** Destructor.

        Note that when a component is deleted, any child components it contains are NOT
        automatically deleted. It's your responsibilty to manage their lifespan - you
        may want to use helper methods like deleteAllChildren(), or less haphazard
        approaches like using ScopedPointers or normal object aggregation to manage them.

        If the component being deleted is currently the child of another one, then during
        deletion, it will be removed from its parent, and the parent will receive a childrenChanged()
        callback. Any ComponentListener objects that have registered with it will also have their
        ComponentListener::componentBeingDeleted() methods called.
    */
    virtual ~Component();

    //==============================================================================
    /** Creates a component, setting its name at the same time.
        @see getName, setName
    */
    explicit Component (const String& componentName) noexcept;

    /** Returns the name of this component.
        @see setName
    */
    const String& getName() const noexcept                  { return componentName; }

    /** Sets the name of this component.

        When the name changes, all registered ComponentListeners will receive a
        ComponentListener::componentNameChanged() callback.

        @see getName
    */
    virtual void setName (const String& newName);

    /** Returns the ID string that was set by setComponentID().
        @see setComponentID, findChildWithID
    */
    const String& getComponentID() const noexcept           { return componentID; }

    /** Sets the component's ID string.
        You can retrieve the ID using getComponentID().
        @see getComponentID, findChildWithID
    */
    void setComponentID (const String& newID);

    //==============================================================================
    /** Makes the component visible or invisible.

        This method will show or hide the component.
        Note that components default to being non-visible when first created.
        Also note that visible components won't be seen unless all their parent components
        are also visible.

        This method will call visibilityChanged() and also componentVisibilityChanged()
        for any component listeners that are interested in this component.

        @param shouldBeVisible  whether to show or hide the component
        @see isVisible, isShowing, visibilityChanged, ComponentListener::componentVisibilityChanged
    */
    virtual void setVisible (bool shouldBeVisible);

    /** Tests whether the component is visible or not.

        this doesn't necessarily tell you whether this comp is actually on the screen
        because this depends on whether all the parent components are also visible - use
        isShowing() to find this out.

        @see isShowing, setVisible
    */
    bool isVisible() const noexcept                         { return flags.visibleFlag; }

    /** Called when this component's visibility changes.
        @see setVisible, isVisible
    */
    virtual void visibilityChanged();

    /** Tests whether this component and all its parents are visible.

        @returns    true only if this component and all its parents are visible.
        @see isVisible
    */
    bool isShowing() const;

    //==============================================================================
    /** Makes this component appear as a window on the desktop.

        Note that before calling this, you should make sure that the component's opacity is
        set correctly using setOpaque(). If the component is non-opaque, the windowing
        system will try to create a special transparent window for it, which will generally take
        a lot more CPU to operate (and might not even be possible on some platforms).

        If the component is inside a parent component at the time this method is called, it
        will be first be removed from that parent. Likewise if a component on the desktop
        is subsequently added to another component, it'll be removed from the desktop.

        @param windowStyleFlags             a combination of the flags specified in the
                                            ComponentPeer::StyleFlags enum, which define the
                                            window's characteristics.
        @param nativeWindowToAttachTo       this allows an OS object to be passed-in as the window
                                            in which the juce component should place itself. On Windows,
                                            this would be a HWND, a HIViewRef on the Mac. Not necessarily
                                            supported on all platforms, and best left as 0 unless you know
                                            what you're doing
        @see removeFromDesktop, isOnDesktop, userTriedToCloseWindow,
             getPeer, ComponentPeer::setMinimised, ComponentPeer::StyleFlags,
             ComponentPeer::getStyleFlags, ComponentPeer::setFullScreen
    */
    virtual void addToDesktop (int windowStyleFlags,
                               void* nativeWindowToAttachTo = nullptr);

    /** If the component is currently showing on the desktop, this will hide it.

        You can also use setVisible() to hide a desktop window temporarily, but
        removeFromDesktop() will free any system resources that are being used up.

        @see addToDesktop, isOnDesktop
    */
    void removeFromDesktop();

    /** Returns true if this component is currently showing on the desktop.
        @see addToDesktop, removeFromDesktop
    */
    bool isOnDesktop() const noexcept;

    /** Returns the heavyweight window that contains this component.

        If this component is itself on the desktop, this will return the window
        object that it is using. Otherwise, it will return the window of
        its top-level parent component.

        This may return nullptr if there isn't a desktop component.

        @see addToDesktop, isOnDesktop
    */
    ComponentPeer* getPeer() const;

    /** For components on the desktop, this is called if the system wants to close the window.

        This is a signal that either the user or the system wants the window to close. The
        default implementation of this method will trigger an assertion to warn you that your
        component should do something about it, but you can override this to ignore the event
        if you want.
    */
    virtual void userTriedToCloseWindow();

    /** Called for a desktop component which has just been minimised or un-minimised.
        This will only be called for components on the desktop.
        @see getPeer, ComponentPeer::setMinimised, ComponentPeer::isMinimised
    */
    virtual void minimisationStateChanged (bool isNowMinimised);

    /** Returns the default scale factor to use for this component when it is placed
        on the desktop.
        The default implementation of this method just returns the value from
        Desktop::getGlobalScaleFactor(), but it can be overridden if a particular component
        has different requirements. The method only used if this component is added
        to the desktop - it has no effect for child components.
    */
    virtual float getDesktopScaleFactor() const;

    //==============================================================================
    /** Brings the component to the front of its siblings.

        If some of the component's siblings have had their 'always-on-top' flag set,
        then they will still be kept in front of this one (unless of course this
        one is also 'always-on-top').

        @param shouldAlsoGainFocus  if true, this will also try to assign keyboard focus
                                    to the component (see grabKeyboardFocus() for more details)
        @see toBack, toBehind, setAlwaysOnTop
    */
    void toFront (bool shouldAlsoGainFocus);

    /** Changes this component's z-order to be at the back of all its siblings.

        If the component is set to be 'always-on-top', it will only be moved to the
        back of the other other 'always-on-top' components.

        @see toFront, toBehind, setAlwaysOnTop
    */
    void toBack();

    /** Changes this component's z-order so that it's just behind another component.
        @see toFront, toBack
    */
    void toBehind (Component* other);

    /** Sets whether the component should always be kept at the front of its siblings.
        @see isAlwaysOnTop
    */
    void setAlwaysOnTop (bool shouldStayOnTop);

    /** Returns true if this component is set to always stay in front of its siblings.
        @see setAlwaysOnTop
    */
    bool isAlwaysOnTop() const noexcept;

    //==============================================================================
    /** Returns the x coordinate of the component's left edge.
        This is a distance in pixels from the left edge of the component's parent.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to its bounding box.
    */
    int getX() const noexcept                               { return bounds.getX(); }

    /** Returns the y coordinate of the top of this component.
        This is a distance in pixels from the top edge of the component's parent.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to its bounding box.
    */
    int getY() const noexcept                               { return bounds.getY(); }

    /** Returns the component's width in pixels. */
    int getWidth() const noexcept                           { return bounds.getWidth(); }

    /** Returns the component's height in pixels. */
    int getHeight() const noexcept                          { return bounds.getHeight(); }

    /** Returns the x coordinate of the component's right-hand edge.
        This is a distance in pixels from the left edge of the component's parent.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to its bounding box.
    */
    int getRight() const noexcept                           { return bounds.getRight(); }

    /** Returns the component's top-left position as a Point. */
    Point<int> getPosition() const noexcept                 { return bounds.getPosition(); }

    /** Returns the y coordinate of the bottom edge of this component.
        This is a distance in pixels from the top edge of the component's parent.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to its bounding box.
    */
    int getBottom() const noexcept                          { return bounds.getBottom(); }

    /** Returns this component's bounding box.
        The rectangle returned is relative to the top-left of the component's parent.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to its bounding box.
    */
    const Rectangle<int>& getBounds() const noexcept        { return bounds; }

    /** Returns the component's bounds, relative to its own origin.
        This is like getBounds(), but returns the rectangle in local coordinates, In practice, it'll
        return a rectangle with position (0, 0), and the same size as this component.
    */
    Rectangle<int> getLocalBounds() const noexcept;

    /** Returns the area of this component's parent which this component covers.

        The returned area is relative to the parent's coordinate space.
        If the component has an affine transform specified, then the resulting area will be
        the smallest rectangle that fully covers the component's transformed bounding box.
        If this component has no parent, the return value will simply be the same as getBounds().
    */
    Rectangle<int> getBoundsInParent() const noexcept;

    //==============================================================================
    /** Returns this component's x coordinate relative the screen's top-left origin.
        @see getX, localPointToGlobal
    */
    int getScreenX() const;

    /** Returns this component's y coordinate relative the screen's top-left origin.
        @see getY, localPointToGlobal
    */
    int getScreenY() const;

    /** Returns the position of this component's top-left corner relative to the screen's top-left.
        @see getScreenBounds
    */
    Point<int> getScreenPosition() const;

    /** Returns the bounds of this component, relative to the screen's top-left.
        @see getScreenPosition
    */
    Rectangle<int> getScreenBounds() const;

    /** Converts a point to be relative to this component's coordinate space.

        This takes a point relative to a different component, and returns its position relative to this
        component. If the sourceComponent parameter is null, the source point is assumed to be a global
        screen coordinate.
    */
    Point<int> getLocalPoint (const Component* sourceComponent,
                              Point<int> pointRelativeToSourceComponent) const;

    /** Converts a point to be relative to this component's coordinate space.

        This takes a point relative to a different component, and returns its position relative to this
        component. If the sourceComponent parameter is null, the source point is assumed to be a global
        screen coordinate.
    */
    Point<float> getLocalPoint (const Component* sourceComponent,
                                Point<float> pointRelativeToSourceComponent) const;

    /** Converts a rectangle to be relative to this component's coordinate space.

        This takes a rectangle that is relative to a different component, and returns its position relative
        to this component. If the sourceComponent parameter is null, the source rectangle is assumed to be
        a screen coordinate.

        If you've used setTransform() to apply one or more transforms to components, then the source rectangle
        may not actually be rectanglular when converted to the target space, so in that situation this will return
        the smallest rectangle that fully contains the transformed area.
    */
    Rectangle<int> getLocalArea (const Component* sourceComponent,
                                 const Rectangle<int>& areaRelativeToSourceComponent) const;

    /** Converts a point relative to this component's top-left into a screen coordinate.
        @see getLocalPoint, localAreaToGlobal
    */
    Point<int> localPointToGlobal (Point<int> localPoint) const;

    /** Converts a point relative to this component's top-left into a screen coordinate.
        @see getLocalPoint, localAreaToGlobal
    */
    Point<float> localPointToGlobal (Point<float> localPoint) const;

    /** Converts a rectangle from this component's coordinate space to a screen coordinate.

        If you've used setTransform() to apply one or more transforms to components, then the source rectangle
        may not actually be rectanglular when converted to the target space, so in that situation this will return
        the smallest rectangle that fully contains the transformed area.
        @see getLocalPoint, localPointToGlobal
    */
    Rectangle<int> localAreaToGlobal (const Rectangle<int>& localArea) const;

    //==============================================================================
    /** Moves the component to a new position.

        Changes the component's top-left position (without changing its size).
        The position is relative to the top-left of the component's parent.

        If the component actually moves, this method will make a synchronous call to moved().

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.

        @see setBounds, ComponentListener::componentMovedOrResized
    */
    void setTopLeftPosition (int x, int y);

    /** Moves the component to a new position.

        Changes the component's top-left position (without changing its size).
        The position is relative to the top-left of the component's parent.

        If the component actually moves, this method will make a synchronous call to moved().

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.

        @see setBounds, ComponentListener::componentMovedOrResized
    */
    void setTopLeftPosition (Point<int> newTopLeftPosition);

    /** Moves the component to a new position.

        Changes the position of the component's top-right corner (keeping it the same size).
        The position is relative to the top-left of the component's parent.

        If the component actually moves, this method will make a synchronous call to moved().

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.
    */
    void setTopRightPosition (int x, int y);

    /** Changes the size of the component.

        A synchronous call to resized() will be occur if the size actually changes.

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.
    */
    void setSize (int newWidth, int newHeight);

    /** Changes the component's position and size.

        The coordinates are relative to the top-left of the component's parent, or relative
        to the origin of the screen is the component is on the desktop.

        If this method changes the component's top-left position, it will make a synchronous
        call to moved(). If it changes the size, it will also make a call to resized().

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.

        @see setTopLeftPosition, setSize, ComponentListener::componentMovedOrResized
    */
    void setBounds (int x, int y, int width, int height);

    /** Changes the component's position and size.

        The coordinates are relative to the top-left of the component's parent, or relative
        to the origin of the screen is the component is on the desktop.

        If this method changes the component's top-left position, it will make a synchronous
        call to moved(). If it changes the size, it will also make a call to resized().

        Note that if you've used setTransform() to apply a transform, then the component's
        bounds will no longer be a direct reflection of the position at which it appears within
        its parent, as the transform will be applied to whatever bounds you set for it.

        @see setBounds
    */
    void setBounds (const Rectangle<int>& newBounds);

    /** Changes the component's position and size.

        This is similar to the other setBounds() methods, but uses RelativeRectangle::applyToComponent()
        to set the position, This uses a Component::Positioner to make sure that any dynamic
        expressions are used in the RelativeRectangle will be automatically re-applied to the
        component's bounds when the source values change. See RelativeRectangle::applyToComponent()
        for more details.

        When using relative expressions, the following symbols are available:
         - "left", "right", "top", "bottom" refer to the position of those edges in this component, so
           e.g. for a component whose width is always 100, you might set the right edge to the "left + 100".
         - "[id].left", "[id].right", "[id].top", "[id].bottom", "[id].width", "[id].height", where [id] is
           the identifier of one of this component's siblings. A component's identifier is set with
           Component::setComponentID(). So for example if you want your component to always be 50 pixels to the
           right of the one called "xyz", you could set your left edge to be "xyz.right + 50".
         - Instead of an [id], you can use the name "parent" to refer to this component's parent. Like
           any other component, these values are relative to their component's parent, so "parent.right" won't be
           very useful for positioning a component because it refers to a position with the parent's parent.. but
           "parent.width" can be used for setting positions relative to the parent's size. E.g. to make a 10x10
           component which remains 1 pixel away from its parent's bottom-right, you could use
           "right - 10, bottom - 10, parent.width - 1, parent.height - 1".
         - The name of one of the parent component's markers can also be used as a symbol. For markers to be
           used, the parent component must implement its Component::getMarkers() method, and return at least one
           valid MarkerList. So if you want your component's top edge to be 10 pixels below the
           marker called "foobar", you'd set it to "foobar + 10".

        See the Expression class for details about the operators that are supported, but for example
        if you wanted to make your component remain centred within its parent with a size of 100, 100,
        you could express it as:
        @code myComp.setBounds (RelativeBounds ("parent.width / 2 - 50, parent.height / 2 - 50, left + 100, top + 100"));
        @endcode
        ..or an alternative way to achieve the same thing:
        @code myComp.setBounds (RelativeBounds ("right - 100, bottom - 100, parent.width / 2 + 50, parent.height / 2 + 50"));
        @endcode

        Or if you wanted a 100x100 component whose top edge is lined up to a marker called "topMarker" and
        which is positioned 50 pixels to the right of another component called "otherComp", you could write:
        @code myComp.setBounds (RelativeBounds ("otherComp.right + 50, topMarker, left + 100, top + 100"));
        @endcode

        Be careful not to make your coordinate expressions recursive, though, or exceptions and assertions will
        be thrown!

        @see setBounds, RelativeRectangle::applyToComponent(), Expression
    */
    void setBounds (const RelativeRectangle& newBounds);

    /** Sets the component's bounds with an expression.
        The string is parsed as a RelativeRectangle expression - see the notes for
        Component::setBounds (const RelativeRectangle&) for more information. This method is
        basically just a shortcut for writing setBounds (RelativeRectangle ("..."))
    */
    void setBounds (const String& newBoundsExpression);

    /** Changes the component's position and size in terms of fractions of its parent's size.

        The values are factors of the parent's size, so for example
        setBoundsRelative (0.2f, 0.2f, 0.5f, 0.5f) would give it half the
        width and height of the parent, with its top-left position 20% of
        the way across and down the parent.

        @see setBounds
    */
    void setBoundsRelative (float proportionalX, float proportionalY,
                            float proportionalWidth, float proportionalHeight);

    /** Changes the component's position and size based on the amount of space to leave around it.

        This will position the component within its parent, leaving the specified number of
        pixels around each edge.

        @see setBounds
    */
    void setBoundsInset (const BorderSize<int>& borders);

    /** Positions the component within a given rectangle, keeping its proportions
        unchanged.

        If onlyReduceInSize is false, the component will be resized to fill as much of the
        rectangle as possible without changing its aspect ratio (the component's
        current size is used to determine its aspect ratio, so a zero-size component
        won't work here). If onlyReduceInSize is true, it will only be resized if it's
        too big to fit inside the rectangle.

        It will then be positioned within the rectangle according to the justification flags
        specified.

        @see setBounds
    */
    void setBoundsToFit (int x, int y, int width, int height,
                         Justification justification,
                         bool onlyReduceInSize);

    /** Changes the position of the component's centre.

        Leaves the component's size unchanged, but sets the position of its centre
        relative to its parent's top-left.

        @see setBounds
    */
    void setCentrePosition (int x, int y);

    /** Changes the position of the component's centre.

        Leaves the position unchanged, but positions its centre relative to its
        parent's size. E.g. setCentreRelative (0.5f, 0.5f) would place it centrally in
        its parent.
    */
    void setCentreRelative (float x, float y);

    /** Changes the component's size and centres it within its parent.

        After changing the size, the component will be moved so that it's
        centred within its parent. If the component is on the desktop (or has no
        parent component), then it'll be centred within the main monitor area.
    */
    void centreWithSize (int width, int height);

    //==============================================================================
    /** Sets a transform matrix to be applied to this component.

        If you set a transform for a component, the component's position will be warped by it, relative to
        the component's parent's top-left origin. This means that the values you pass into setBounds() will no
        longer reflect the actual area within the parent that the component covers, as the bounds will be
        transformed and the component will probably end up actually appearing somewhere else within its parent.

        When using transforms you need to be extremely careful when converting coordinates between the
        coordinate spaces of different components or the screen - you should always use getLocalPoint(),
        getLocalArea(), etc to do this, and never just manually add a component's position to a point in order to
        convert it between different components (but I'm sure you would never have done that anyway...).

        Currently, transforms are not supported for desktop windows, so the transform will be ignored if you
        put a component on the desktop.

        To remove a component's transform, simply pass AffineTransform::identity as the parameter to this method.
    */
    void setTransform (const AffineTransform& transform);

    /** Returns the transform that is currently being applied to this component.
        For more details about transforms, see setTransform().
        @see setTransform
    */
    AffineTransform getTransform() const;

    /** Returns true if a non-identity transform is being applied to this component.
        For more details about transforms, see setTransform().
        @see setTransform
    */
    bool isTransformed() const noexcept;

    //==============================================================================
    /** Returns a proportion of the component's width.
        This is a handy equivalent of (getWidth() * proportion).
    */
    int proportionOfWidth (float proportion) const noexcept;

    /** Returns a proportion of the component's height.
        This is a handy equivalent of (getHeight() * proportion).
    */
    int proportionOfHeight (float proportion) const noexcept;

    /** Returns the width of the component's parent.

        If the component has no parent (i.e. if it's on the desktop), this will return
        the width of the screen.
    */
    int getParentWidth() const noexcept;

    /** Returns the height of the component's parent.

        If the component has no parent (i.e. if it's on the desktop), this will return
        the height of the screen.
    */
    int getParentHeight() const noexcept;

    /** Returns the screen coordinates of the monitor that contains this component.

        If there's only one monitor, this will return its size - if there are multiple
        monitors, it will return the area of the monitor that contains the component's
        centre.
    */
    Rectangle<int> getParentMonitorArea() const;

    //==============================================================================
    /** Returns the number of child components that this component contains.

        @see getChildComponent, getIndexOfChildComponent
    */
    int getNumChildComponents() const noexcept;

    /** Returns one of this component's child components, by it index.

        The component with index 0 is at the back of the z-order, the one at the
        front will have index (getNumChildComponents() - 1).

        If the index is out-of-range, this will return a null pointer.

        @see getNumChildComponents, getIndexOfChildComponent
    */
    Component* getChildComponent (int index) const noexcept;

    /** Returns the index of this component in the list of child components.

        A value of 0 means it is first in the list (i.e. behind all other components). Higher
        values are further towards the front.

        Returns -1 if the component passed-in is not a child of this component.

        @see getNumChildComponents, getChildComponent, addChildComponent, toFront, toBack, toBehind
    */
    int getIndexOfChildComponent (const Component* child) const noexcept;

    /** Looks for a child component with the specified ID.
        @see setComponentID, getComponentID
    */
    Component* findChildWithID (StringRef componentID) const noexcept;

    /** Adds a child component to this one.

        Adding a child component does not mean that the component will own or delete the child - it's
        your responsibility to delete the component. Note that it's safe to delete a component
        without first removing it from its parent - doing so will automatically remove it and
        send out the appropriate notifications before the deletion completes.

        If the child is already a child of this component, then no action will be taken, and its
        z-order will be left unchanged.

        @param child    the new component to add. If the component passed-in is already
                        the child of another component, it'll first be removed from it current parent.
        @param zOrder   The index in the child-list at which this component should be inserted.
                        A value of -1 will insert it in front of the others, 0 is the back.
        @see removeChildComponent, addAndMakeVisible, addChildAndSetID, getChild, ComponentListener::componentChildrenChanged
    */
    void addChildComponent (Component* child, int zOrder = -1);

    /** Adds a child component to this one.

        Adding a child component does not mean that the component will own or delete the child - it's
        your responsibility to delete the component. Note that it's safe to delete a component
        without first removing it from its parent - doing so will automatically remove it and
        send out the appropriate notifications before the deletion completes.

        If the child is already a child of this component, then no action will be taken, and its
        z-order will be left unchanged.

        @param child    the new component to add. If the component passed-in is already
                        the child of another component, it'll first be removed from it current parent.
        @param zOrder   The index in the child-list at which this component should be inserted.
                        A value of -1 will insert it in front of the others, 0 is the back.
        @see removeChildComponent, addAndMakeVisible, addChildAndSetID, getChild, ComponentListener::componentChildrenChanged
    */
    void addChildComponent (Component& child, int zOrder = -1);

    /** Adds a child component to this one, and also makes the child visible if it isn't already.

        This is the same as calling setVisible (true) on the child and then addChildComponent().
        See addChildComponent() for more details.
    */
    void addAndMakeVisible (Component* child, int zOrder = -1);

    /** Adds a child component to this one, and also makes the child visible if it isn't already.

        This is the same as calling setVisible (true) on the child and then addChildComponent().
        See addChildComponent() for more details.
    */
    void addAndMakeVisible (Component& child, int zOrder = -1);

    /** Adds a child component to this one, makes it visible, and sets its component ID.
        @see addAndMakeVisible, addChildComponent
    */
    void addChildAndSetID (Component* child, const String& componentID);

    /** Removes one of this component's child-components.

        If the child passed-in isn't actually a child of this component (either because
        it's invalid or is the child of a different parent), then no action is taken.

        Note that removing a child will not delete it! But it's ok to delete a component
        without first removing it - doing so will automatically remove it and send out the
        appropriate notifications before the deletion completes.

        @see addChildComponent, ComponentListener::componentChildrenChanged
    */
    void removeChildComponent (Component* childToRemove);

    /** Removes one of this component's child-components by index.

        This will return a pointer to the component that was removed, or null if
        the index was out-of-range.

        Note that removing a child will not delete it! But it's ok to delete a component
        without first removing it - doing so will automatically remove it and send out the
        appropriate notifications before the deletion completes.

        @see addChildComponent, ComponentListener::componentChildrenChanged
    */
    Component* removeChildComponent (int childIndexToRemove);

    /** Removes all this component's children.
        Note that this won't delete them! To do that, use deleteAllChildren() instead.
    */
    void removeAllChildren();

    /** Removes and deletes all of this component's children.
        My advice is to avoid this method! It's an old function that is only kept here for
        backwards-compatibility with legacy code, and should be viewed with extreme
        suspicion by anyone attempting to write modern C++. In almost all cases, it's much
        smarter to manage the lifetimes of your child components via modern RAII techniques
        such as simply making them member variables, or using ScopedPointer, OwnedArray, etc
        to manage their lifetimes appropriately.
        @see removeAllChildren
    */
    void deleteAllChildren();

    /** Returns the component which this component is inside.

        If this is the highest-level component or hasn't yet been added to
        a parent, this will return null.
    */
    Component* getParentComponent() const noexcept                  { return parentComponent; }

    /** Searches the parent components for a component of a specified class.

        For example findParentComponentOfClass \<MyComp\>() would return the first parent
        component that can be dynamically cast to a MyComp, or will return 0 if none
        of the parents are suitable.
    */
    template <class TargetClass>
    TargetClass* findParentComponentOfClass() const
    {
        for (Component* p = parentComponent; p != nullptr; p = p->parentComponent)
            if (TargetClass* const target = dynamic_cast<TargetClass*> (p))
                return target;

        return nullptr;
    }

    /** Returns the highest-level component which contains this one or its parents.

        This will search upwards in the parent-hierarchy from this component, until it
        finds the highest one that doesn't have a parent (i.e. is on the desktop or
        not yet added to a parent), and will return that.
    */
    Component* getTopLevelComponent() const noexcept;

    /** Checks whether a component is anywhere inside this component or its children.

        This will recursively check through this component's children to see if the
        given component is anywhere inside.
    */
    bool isParentOf (const Component* possibleChild) const noexcept;

    //==============================================================================
    /** Called to indicate that the component's parents have changed.

        When a component is added or removed from its parent, this method will
        be called on all of its children (recursively - so all children of its
        children will also be called as well).

        Subclasses can override this if they need to react to this in some way.

        @see getParentComponent, isShowing, ComponentListener::componentParentHierarchyChanged
    */
    virtual void parentHierarchyChanged();

    /** Subclasses can use this callback to be told when children are added or removed, or
        when their z-order changes.
        @see parentHierarchyChanged, ComponentListener::componentChildrenChanged
    */
    virtual void childrenChanged();

    //==============================================================================
    /** Tests whether a given point inside the component.

        Overriding this method allows you to create components which only intercept
        mouse-clicks within a user-defined area.

        This is called to find out whether a particular x, y coordinate is
        considered to be inside the component or not, and is used by methods such
        as contains() and getComponentAt() to work out which component
        the mouse is clicked on.

        Components with custom shapes will probably want to override it to perform
        some more complex hit-testing.

        The default implementation of this method returns either true or false,
        depending on the value that was set by calling setInterceptsMouseClicks() (true
        is the default return value).

        Note that the hit-test region is not related to the opacity with which
        areas of a component are painted.

        Applications should never call hitTest() directly - instead use the
        contains() method, because this will also test for occlusion by the
        component's parent.

        Note that for components on the desktop, this method will be ignored, because it's
        not always possible to implement this behaviour on all platforms.

        @param x    the x coordinate to test, relative to the left hand edge of this
                    component. This value is guaranteed to be greater than or equal to
                    zero, and less than the component's width
        @param y    the y coordinate to test, relative to the top edge of this
                    component. This value is guaranteed to be greater than or equal to
                    zero, and less than the component's height
        @returns    true if the click is considered to be inside the component
        @see setInterceptsMouseClicks, contains
    */
    virtual bool hitTest (int x, int y);

    /** Changes the default return value for the hitTest() method.

        Setting this to false is an easy way to make a component pass its mouse-clicks
        through to the components behind it.

        When a component is created, the default setting for this is true.

        @param allowClicksOnThisComponent   if true, hitTest() will always return true; if false, it will
                                            return false (or true for child components if allowClicksOnChildComponents
                                            is true)
        @param allowClicksOnChildComponents if this is true and allowClicksOnThisComponent is false, then child
                                            components can be clicked on as normal but clicks on this component pass
                                            straight through; if this is false and allowClicksOnThisComponent
                                            is false, then neither this component nor any child components can
                                            be clicked on
        @see hitTest, getInterceptsMouseClicks
    */
    void setInterceptsMouseClicks (bool allowClicksOnThisComponent,
                                   bool allowClicksOnChildComponents) noexcept;

    /** Retrieves the current state of the mouse-click interception flags.

        On return, the two parameters are set to the state used in the last call to
        setInterceptsMouseClicks().

        @see setInterceptsMouseClicks
    */
    void getInterceptsMouseClicks (bool& allowsClicksOnThisComponent,
                                   bool& allowsClicksOnChildComponents) const noexcept;


    /** Returns true if a given point lies within this component or one of its children.

        Never override this method! Use hitTest to create custom hit regions.

        @param localPoint    the coordinate to test, relative to this component's top-left.
        @returns    true if the point is within the component's hit-test area, but only if
                    that part of the component isn't clipped by its parent component. Note
                    that this won't take into account any overlapping sibling components
                    which might be in the way - for that, see reallyContains()
        @see hitTest, reallyContains, getComponentAt
    */
    bool contains (Point<int> localPoint);

    /** Returns true if a given point lies in this component, taking any overlapping
        siblings into account.

        @param localPoint    the coordinate to test, relative to this component's top-left.
        @param returnTrueIfWithinAChild     if the point actually lies within a child of this component,
                                            this determines whether that is counted as a hit.
        @see contains, getComponentAt
    */
    bool reallyContains (Point<int> localPoint, bool returnTrueIfWithinAChild);

    /** Returns the component at a certain point within this one.

        @param x    the x coordinate to test, relative to this component's left edge.
        @param y    the y coordinate to test, relative to this component's top edge.
        @returns    the component that is at this position - which may be 0, this component,
                    or one of its children. Note that overlapping siblings that might actually
                    be in the way are not taken into account by this method - to account for these,
                    instead call getComponentAt on the top-level parent of this component.
        @see hitTest, contains, reallyContains
    */
    Component* getComponentAt (int x, int y);

    /** Returns the component at a certain point within this one.

        @param position  the coordinate to test, relative to this component's top-left.
        @returns    the component that is at this position - which may be 0, this component,
                    or one of its children. Note that overlapping siblings that might actually
                    be in the way are not taken into account by this method - to account for these,
                    instead call getComponentAt on the top-level parent of this component.
        @see hitTest, contains, reallyContains
    */
    Component* getComponentAt (Point<int> position);

    //==============================================================================
    /** Marks the whole component as needing to be redrawn.

        Calling this will not do any repainting immediately, but will mark the component
        as 'dirty'. At some point in the near future the operating system will send a paint
        message, which will redraw all the dirty regions of all components.
        There's no guarantee about how soon after calling repaint() the redraw will actually
        happen, and other queued events may be delivered before a redraw is done.

        If the setBufferedToImage() method has been used to cause this component to use a
        buffer, the repaint() call will invalidate the cached buffer. If setCachedComponentImage()
        has been used to provide a custom image cache, that cache will be invalidated appropriately.

        To redraw just a subsection of the component rather than the whole thing,
        use the repaint (int, int, int, int) method.

        @see paint
    */
    void repaint();

    /** Marks a subsection of this component as needing to be redrawn.

        Calling this will not do any repainting immediately, but will mark the given region
        of the component as 'dirty'. At some point in the near future the operating system
        will send a paint message, which will redraw all the dirty regions of all components.
        There's no guarantee about how soon after calling repaint() the redraw will actually
        happen, and other queued events may be delivered before a redraw is done.

        The region that is passed in will be clipped to keep it within the bounds of this
        component.

        @see repaint()
    */
    void repaint (int x, int y, int width, int height);

    /** Marks a subsection of this component as needing to be redrawn.

        Calling this will not do any repainting immediately, but will mark the given region
        of the component as 'dirty'. At some point in the near future the operating system
        will send a paint message, which will redraw all the dirty regions of all components.
        There's no guarantee about how soon after calling repaint() the redraw will actually
        happen, and other queued events may be delivered before a redraw is done.

        The region that is passed in will be clipped to keep it within the bounds of this
        component.

        @see repaint()
    */
    void repaint (const Rectangle<int>& area);

    //==============================================================================
    /** Makes the component use an internal buffer to optimise its redrawing.

        Setting this flag to true will cause the component to allocate an
        internal buffer into which it paints itself, so that when asked to
        redraw itself, it can use this buffer rather than actually calling the
        paint() method.

        The buffer is kept until the repaint() method is called directly on
        this component (or until it is resized), when the image is invalidated
        and then redrawn the next time the component is painted.

        Note that only the drawing that happens within the component's paint()
        method is drawn into the buffer, it's child components are not buffered, and
        nor is the paintOverChildren() method.

        @see repaint, paint, createComponentSnapshot
    */
    void setBufferedToImage (bool shouldBeBuffered);

    /** Generates a snapshot of part of this component.

        This will return a new Image, the size of the rectangle specified,
        containing a snapshot of the specified area of the component and all
        its children.

        The image may or may not have an alpha-channel, depending on whether the
        image is opaque or not.

        If the clipImageToComponentBounds parameter is true and the area is greater than
        the size of the component, it'll be clipped. If clipImageToComponentBounds is false
        then parts of the component beyond its bounds can be drawn.

        @see paintEntireComponent
    */
    Image createComponentSnapshot (const Rectangle<int>& areaToGrab,
                                   bool clipImageToComponentBounds = true,
                                   float scaleFactor = 1.0f);

    /** Draws this component and all its subcomponents onto the specified graphics
        context.

        You should very rarely have to use this method, it's simply there in case you need
        to draw a component with a custom graphics context for some reason, e.g. for
        creating a snapshot of the component.

        It calls paint(), paintOverChildren() and recursively calls paintEntireComponent()
        on its children in order to render the entire tree.

        The graphics context may be left in an undefined state after this method returns,
        so you may need to reset it if you're going to use it again.

        If ignoreAlphaLevel is false, then the component will be drawn with the opacity level
        specified by getAlpha(); if ignoreAlphaLevel is true, then this will be ignored and
        an alpha of 1.0 will be used.
    */
    void paintEntireComponent (Graphics& context, bool ignoreAlphaLevel);

    /** This allows you to indicate that this component doesn't require its graphics
        context to be clipped when it is being painted.

        Most people will never need to use this setting, but in situations where you have a very large
        number of simple components being rendered, and where they are guaranteed never to do any drawing
        beyond their own boundaries, setting this to true will reduce the overhead involved in clipping
        the graphics context that gets passed to the component's paint() callback.
        If you enable this mode, you'll need to make sure your paint method doesn't call anything like
        Graphics::fillAll(), and doesn't draw beyond the component's bounds, because that'll produce
        artifacts. Your component also can't have any child components that may be placed beyond its
        bounds.
    */
    void setPaintingIsUnclipped (bool shouldPaintWithoutClipping) noexcept;

    //==============================================================================
    /** Adds an effect filter to alter the component's appearance.

        When a component has an effect filter set, then this is applied to the
        results of its paint() method. There are a few preset effects, such as
        a drop-shadow or glow, but they can be user-defined as well.

        The effect that is passed in will not be deleted by the component - the
        caller must take care of deleting it.

        To remove an effect from a component, pass a null pointer in as the parameter.

        @see ImageEffectFilter, DropShadowEffect, GlowEffect
    */
    void setComponentEffect (ImageEffectFilter* newEffect);

    /** Returns the current component effect.
        @see setComponentEffect
    */
    ImageEffectFilter* getComponentEffect() const noexcept              { return effect; }

    //==============================================================================
    /** Finds the appropriate look-and-feel to use for this component.

        If the component hasn't had a look-and-feel explicitly set, this will
        return the parent's look-and-feel, or just the default one if there's no
        parent.

        @see setLookAndFeel, lookAndFeelChanged
    */
    LookAndFeel& getLookAndFeel() const noexcept;

    /** Sets the look and feel to use for this component.

        This will also change the look and feel for any child components that haven't
        had their look set explicitly.

        The object passed in will not be deleted by the component, so it's the caller's
        responsibility to manage it. It may be used at any time until this component
        has been deleted.

        Calling this method will also invoke the sendLookAndFeelChange() method.

        @see getLookAndFeel, lookAndFeelChanged
    */
    void setLookAndFeel (LookAndFeel* newLookAndFeel);

    /** Called to let the component react to a change in the look-and-feel setting.

        When the look-and-feel is changed for a component, this will be called in
        all its child components, recursively.

        It can also be triggered manually by the sendLookAndFeelChange() method, in case
        an application uses a LookAndFeel class that might have changed internally.

        @see sendLookAndFeelChange, getLookAndFeel
    */
    virtual void lookAndFeelChanged();

    /** Calls the lookAndFeelChanged() method in this component and all its children.

        This will recurse through the children and their children, calling lookAndFeelChanged()
        on them all.

        @see lookAndFeelChanged
    */
    void sendLookAndFeelChange();

    //==============================================================================
    /** Indicates whether any parts of the component might be transparent.

        Components that always paint all of their contents with solid colour and
        thus completely cover any components behind them should use this method
        to tell the repaint system that they are opaque.

        This information is used to optimise drawing, because it means that
        objects underneath opaque windows don't need to be painted.

        By default, components are considered transparent, unless this is used to
        make it otherwise.

        @see isOpaque
    */
    void setOpaque (bool shouldBeOpaque);

    /** Returns true if no parts of this component are transparent.

        @returns the value that was set by setOpaque, (the default being false)
        @see setOpaque
    */
    bool isOpaque() const noexcept;

    //==============================================================================
    /** Indicates whether the component should be brought to the front when clicked.

        Setting this flag to true will cause the component to be brought to the front
        when the mouse is clicked somewhere inside it or its child components.

        Note that a top-level desktop window might still be brought to the front by the
        operating system when it's clicked, depending on how the OS works.

        By default this is set to false.

        @see setMouseClickGrabsKeyboardFocus
    */
    void setBroughtToFrontOnMouseClick (bool shouldBeBroughtToFront) noexcept;

    /** Indicates whether the component should be brought to the front when clicked-on.
        @see setBroughtToFrontOnMouseClick
    */
    bool isBroughtToFrontOnMouseClick() const noexcept;

    //==============================================================================
    // Keyboard focus methods

    /** Sets a flag to indicate whether this component needs keyboard focus or not.

        By default components aren't actually interested in gaining the
        focus, but this method can be used to turn this on.

        See the grabKeyboardFocus() method for details about the way a component
        is chosen to receive the focus.

        @see grabKeyboardFocus, getWantsKeyboardFocus
    */
    void setWantsKeyboardFocus (bool wantsFocus) noexcept;

    /** Returns true if the component is interested in getting keyboard focus.

        This returns the flag set by setWantsKeyboardFocus(). The default
        setting is false.

        @see setWantsKeyboardFocus
    */
    bool getWantsKeyboardFocus() const noexcept;

    //==============================================================================
    /** Chooses whether a click on this component automatically grabs the focus.

        By default this is set to true, but you might want a component which can
        be focused, but where you don't want the user to be able to affect it directly
        by clicking.
    */
    void setMouseClickGrabsKeyboardFocus (bool shouldGrabFocus);

    /** Returns the last value set with setMouseClickGrabsKeyboardFocus().
        See setMouseClickGrabsKeyboardFocus() for more info.
    */
    bool getMouseClickGrabsKeyboardFocus() const noexcept;

    //==============================================================================
    /** Tries to give keyboard focus to this component.

        When the user clicks on a component or its grabKeyboardFocus()
        method is called, the following procedure is used to work out which
        component should get it:

        - if the component that was clicked on actually wants focus (as indicated
          by calling getWantsKeyboardFocus), it gets it.
        - if the component itself doesn't want focus, it will try to pass it
          on to whichever of its children is the default component, as determined by
          KeyboardFocusTraverser::getDefaultComponent()
        - if none of its children want focus at all, it will pass it up to its
          parent instead, unless it's a top-level component without a parent,
          in which case it just takes the focus itself.

        @see setWantsKeyboardFocus, getWantsKeyboardFocus, hasKeyboardFocus,
             getCurrentlyFocusedComponent, focusGained, focusLost,
             keyPressed, keyStateChanged
    */
    void grabKeyboardFocus();

    /** Returns true if this component currently has the keyboard focus.

        @param trueIfChildIsFocused     if this is true, then the method returns true if
                                        either this component or any of its children (recursively)
                                        have the focus. If false, the method only returns true if
                                        this component has the focus.

        @see grabKeyboardFocus, setWantsKeyboardFocus, getCurrentlyFocusedComponent,
             focusGained, focusLost
    */
    bool hasKeyboardFocus (bool trueIfChildIsFocused) const;

    /** Returns the component that currently has the keyboard focus.
        @returns the focused component, or null if nothing is focused.
    */
    static Component* JUCE_CALLTYPE getCurrentlyFocusedComponent() noexcept;

    /** If any component has keyboard focus, this will defocus it. */
    static void JUCE_CALLTYPE unfocusAllComponents();

    //==============================================================================
    /** Tries to move the keyboard focus to one of this component's siblings.

        This will try to move focus to either the next or previous component. (This
        is the method that is used when shifting focus by pressing the tab key).

        Components for which getWantsKeyboardFocus() returns false are not looked at.

        @param moveToNext   if true, the focus will move forwards; if false, it will
                            move backwards
        @see grabKeyboardFocus, setFocusContainer, setWantsKeyboardFocus
    */
    void moveKeyboardFocusToSibling (bool moveToNext);

    /** Creates a KeyboardFocusTraverser object to use to determine the logic by
        which focus should be passed from this component.

        The default implementation of this method will return a default
        KeyboardFocusTraverser if this component is a focus container (as determined
        by the setFocusContainer() method). If the component isn't a focus
        container, then it will recursively ask its parents for a KeyboardFocusTraverser.

        If you overrride this to return a custom KeyboardFocusTraverser, then
        this component and all its sub-components will use the new object to
        make their focusing decisions.

        The method should return a new object, which the caller is required to
        delete when no longer needed.
    */
    virtual KeyboardFocusTraverser* createFocusTraverser();

    /** Returns the focus order of this component, if one has been specified.

        By default components don't have a focus order - in that case, this
        will return 0. Lower numbers indicate that the component will be
        earlier in the focus traversal order.

        To change the order, call setExplicitFocusOrder().

        The focus order may be used by the KeyboardFocusTraverser class as part of
        its algorithm for deciding the order in which components should be traversed.
        See the KeyboardFocusTraverser class for more details on this.

        @see moveKeyboardFocusToSibling, createFocusTraverser, KeyboardFocusTraverser
    */
    int getExplicitFocusOrder() const;

    /** Sets the index used in determining the order in which focusable components
        should be traversed.

        A value of 0 or less is taken to mean that no explicit order is wanted, and
        that traversal should use other factors, like the component's position.

        @see getExplicitFocusOrder, moveKeyboardFocusToSibling
    */
    void setExplicitFocusOrder (int newFocusOrderIndex);

    /** Indicates whether this component is a parent for components that can have
        their focus traversed.

        This flag is used by the default implementation of the createFocusTraverser()
        method, which uses the flag to find the first parent component (of the currently
        focused one) which wants to be a focus container.

        So using this method to set the flag to 'true' causes this component to
        act as the top level within which focus is passed around.

        @see isFocusContainer, createFocusTraverser, moveKeyboardFocusToSibling
    */
    void setFocusContainer (bool shouldBeFocusContainer) noexcept;

    /** Returns true if this component has been marked as a focus container.

        See setFocusContainer() for more details.

        @see setFocusContainer, moveKeyboardFocusToSibling, createFocusTraverser
    */
    bool isFocusContainer() const noexcept;

    //==============================================================================
    /** Returns true if the component (and all its parents) are enabled.

        Components are enabled by default, and can be disabled with setEnabled(). Exactly
        what difference this makes to the component depends on the type. E.g. buttons
        and sliders will choose to draw themselves differently, etc.

        Note that if one of this component's parents is disabled, this will always
        return false, even if this component itself is enabled.

        @see setEnabled, enablementChanged
    */
    bool isEnabled() const noexcept;

    /** Enables or disables this component.

        Disabling a component will also cause all of its child components to become
        disabled.

        Similarly, enabling a component which is inside a disabled parent
        component won't make any difference until the parent is re-enabled.

        @see isEnabled, enablementChanged
    */
    void setEnabled (bool shouldBeEnabled);

    /** Callback to indicate that this component has been enabled or disabled.

        This can be triggered by one of the component's parent components
        being enabled or disabled, as well as changes to the component itself.

        The default implementation of this method does nothing; your class may
        wish to repaint itself or something when this happens.

        @see setEnabled, isEnabled
    */
    virtual void enablementChanged();

    /** Changes the transparency of this component.
        When painted, the entire component and all its children will be rendered
        with this as the overall opacity level, where 0 is completely invisible, and
        1.0 is fully opaque (i.e. normal).

        @see getAlpha
    */
    void setAlpha (float newAlpha);

    /** Returns the component's current transparancy level.
        See setAlpha() for more details.
    */
    float getAlpha() const;

    //==============================================================================
    /** Changes the mouse cursor shape to use when the mouse is over this component.

        Note that the cursor set by this method can be overridden by the getMouseCursor
        method.

        @see MouseCursor
    */
    void setMouseCursor (const MouseCursor& cursorType);

    /** Returns the mouse cursor shape to use when the mouse is over this component.

        The default implementation will return the cursor that was set by setCursor()
        but can be overridden for more specialised purposes, e.g. returning different
        cursors depending on the mouse position.

        @see MouseCursor
    */
    virtual MouseCursor getMouseCursor();

    /** Forces the current mouse cursor to be updated.

        If you're overriding the getMouseCursor() method to control which cursor is
        displayed, then this will only be checked each time the user moves the mouse. So
        if you want to force the system to check that the cursor being displayed is
        up-to-date (even if the mouse is just sitting there), call this method.

        (If you're changing the cursor using setMouseCursor(), you don't need to bother
        calling this).
    */
    void updateMouseCursor() const;

    //==============================================================================
    /** Components can override this method to draw their content.

        The paint() method gets called when a region of a component needs redrawing,
        either because the component's repaint() method has been called, or because
        something has happened on the screen that means a section of a window needs
        to be redrawn.

        Any child components will draw themselves over whatever this method draws. If
        you need to paint over the top of your child components, you can also implement
        the paintOverChildren() method to do this.

        If you want to cause a component to redraw itself, this is done asynchronously -
        calling the repaint() method marks a region of the component as "dirty", and the
        paint() method will automatically be called sometime later, by the message thread,
        to paint any bits that need refreshing. In Juce (and almost all modern UI frameworks),
        you never redraw something synchronously.

        You should never need to call this method directly - to take a snapshot of the
        component you could use createComponentSnapshot() or paintEntireComponent().

        @param g    the graphics context that must be used to do the drawing operations.
        @see repaint, paintOverChildren, Graphics
    */
    virtual void paint (Graphics& g);

    /** Components can override this method to draw over the top of their children.

        For most drawing operations, it's better to use the normal paint() method,
        but if you need to overlay something on top of the children, this can be
        used.

        @see paint, Graphics
    */
    virtual void paintOverChildren (Graphics& g);


    //==============================================================================
    /** Called when the mouse moves inside a component.

        If the mouse button isn't pressed and the mouse moves over a component,
        this will be called to let the component react to this.

        A component will always get a mouseEnter callback before a mouseMove.

        @param event details about the position and status of the mouse event, including
                     the source component in which it occurred
        @see mouseEnter, mouseExit, mouseDrag, contains
    */
    virtual void mouseMove (const MouseEvent& event) override;

    /** Called when the mouse first enters a component.

        If the mouse button isn't pressed and the mouse moves into a component,
        this will be called to let the component react to this.

        When the mouse button is pressed and held down while being moved in
        or out of a component, no mouseEnter or mouseExit callbacks are made - only
        mouseDrag messages are sent to the component that the mouse was originally
        clicked on, until the button is released.

        @param event details about the position and status of the mouse event, including
                     the source component in which it occurred
        @see mouseExit, mouseDrag, mouseMove, contains
    */
    virtual void mouseEnter (const MouseEvent& event) override;

    /** Called when the mouse moves out of a component.

        This will be called when the mouse moves off the edge of this
        component.

        If the mouse button was pressed, and it was then dragged off the
        edge of the component and released, then this callback will happen
        when the button is released, after the mouseUp callback.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseEnter, mouseDrag, mouseMove, contains
    */
    virtual void mouseExit (const MouseEvent& event) override;

    /** Called when a mouse button is pressed.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        Once a button is held down, the mouseDrag method will be called when the
        mouse moves, until the button is released.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseUp, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseDown (const MouseEvent& event) override;

    /** Called when the mouse is moved while a button is held down.

        When a mouse button is pressed inside a component, that component
        receives mouseDrag callbacks each time the mouse moves, even if the
        mouse strays outside the component's bounds.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseUp, mouseMove, contains, setDragRepeatInterval
    */
    virtual void mouseDrag (const MouseEvent& event) override;

    /** Called when a mouse button is released.

        A mouseUp callback is sent to the component in which a button was pressed
        even if the mouse is actually over a different component when the
        button is released.

        The MouseEvent object passed in contains lots of methods for finding out
        which buttons were down just before they were released.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseUp (const MouseEvent& event) override;

    /** Called when a mouse button has been double-clicked on a component.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        @param event  details about the position and status of the mouse event, including
                      the source component in which it occurred
        @see mouseDown, mouseUp
    */
    virtual void mouseDoubleClick (const MouseEvent& event) override;

    /** Called when the mouse-wheel is moved.

        This callback is sent to the component that the mouse is over when the
        wheel is moved.

        If not overridden, a component will forward this message to its parent, so
        that parent components can collect mouse-wheel messages that happen to
        child components which aren't interested in them. (Bear in mind that if
        you attach a component as a mouse-listener to other components, then
        those wheel moves will also end up calling this method and being passed up
        to the parents, which may not be what you intended to happen).

        @param event   details about the mouse event
        @param wheel   details about the mouse wheel movement
    */
    virtual void mouseWheelMove (const MouseEvent& event,
                                 const MouseWheelDetails& wheel) override;

    /** Called when a pinch-to-zoom mouse-gesture is used.

        If not overridden, a component will forward this message to its parent, so
        that parent components can collect gesture messages that are unused by child
        components.

        @param event   details about the mouse event
        @param scaleFactor  a multiplier to indicate by how much the size of the target
                            should be changed. A value of 1.0 would indicate no change,
                            values greater than 1.0 mean it should be enlarged.
    */
    virtual void mouseMagnify (const MouseEvent& event, float scaleFactor);

    //==============================================================================
    /** Ensures that a non-stop stream of mouse-drag events will be sent during the
        current mouse-drag operation.

        This allows you to make sure that mouseDrag() events are sent continuously, even
        when the mouse isn't moving. This can be useful for things like auto-scrolling
        components when the mouse is near an edge.

        Call this method during a mouseDown() or mouseDrag() callback, specifying the
        minimum interval between consecutive mouse drag callbacks. The callbacks
        will continue until the mouse is released, and then the interval will be reset,
        so you need to make sure it's called every time you begin a drag event.
        Passing an interval of 0 or less will cancel the auto-repeat.

        @see mouseDrag, Desktop::beginDragAutoRepeat
    */
    static void JUCE_CALLTYPE beginDragAutoRepeat (int millisecondsBetweenCallbacks);

    /** Causes automatic repaints when the mouse enters or exits this component.

        If turned on, then when the mouse enters/exits, or when the button is pressed/released
        on the component, it will trigger a repaint.

        This is handy for things like buttons that need to draw themselves differently when
        the mouse moves over them, and it avoids having to override all the different mouse
        callbacks and call repaint().

        @see mouseEnter, mouseExit, mouseDown, mouseUp
    */
    void setRepaintsOnMouseActivity (bool shouldRepaint) noexcept;

    /** Registers a listener to be told when mouse events occur in this component.

        If you need to get informed about mouse events in a component but can't or
        don't want to override its methods, you can attach any number of listeners
        to the component, and these will get told about the events in addition to
        the component's own callbacks being called.

        Note that a MouseListener can also be attached to more than one component.

        @param newListener                              the listener to register
        @param wantsEventsForAllNestedChildComponents   if true, the listener will receive callbacks
                                                        for events that happen to any child component
                                                        within this component, including deeply-nested
                                                        child components. If false, it will only be
                                                        told about events that this component handles.
        @see MouseListener, removeMouseListener
    */
    void addMouseListener (MouseListener* newListener,
                           bool wantsEventsForAllNestedChildComponents);

    /** Deregisters a mouse listener.
        @see addMouseListener, MouseListener
    */
    void removeMouseListener (MouseListener* listenerToRemove);

    //==============================================================================
    /** Adds a listener that wants to hear about keypresses that this component receives.

        The listeners that are registered with a component are called by its keyPressed() or
        keyStateChanged() methods (assuming these haven't been overridden to do something else).

        If you add an object as a key listener, be careful to remove it when the object
        is deleted, or the component will be left with a dangling pointer.

        @see keyPressed, keyStateChanged, removeKeyListener
    */
    void addKeyListener (KeyListener* newListener);

    /** Removes a previously-registered key listener.
        @see addKeyListener
    */
    void removeKeyListener (KeyListener* listenerToRemove);

    /** Called when a key is pressed.

        When a key is pressed, the component that has the keyboard focus will have this
        method called. Remember that a component will only be given the focus if its
        setWantsKeyboardFocus() method has been used to enable this.

        If your implementation returns true, the event will be consumed and not passed
        on to any other listeners. If it returns false, the key will be passed to any
        KeyListeners that have been registered with this component. As soon as one of these
        returns true, the process will stop, but if they all return false, the event will
        be passed upwards to this component's parent, and so on.

        The default implementation of this method does nothing and returns false.

        @see keyStateChanged, getCurrentlyFocusedComponent, addKeyListener
    */
    virtual bool keyPressed (const KeyPress& key);

    /** Called when a key is pressed or released.

        Whenever a key on the keyboard is pressed or released (including modifier keys
        like shift and ctrl), this method will be called on the component that currently
        has the keyboard focus. Remember that a component will only be given the focus if
        its setWantsKeyboardFocus() method has been used to enable this.

        If your implementation returns true, the event will be consumed and not passed
        on to any other listeners. If it returns false, then any KeyListeners that have
        been registered with this component will have their keyStateChanged methods called.
        As soon as one of these returns true, the process will stop, but if they all return
        false, the event will be passed upwards to this component's parent, and so on.

        The default implementation of this method does nothing and returns false.

        To find out which keys are up or down at any time, see the KeyPress::isKeyCurrentlyDown()
        method.

        @param isKeyDown    true if a key has been pressed; false if it has been released

        @see keyPressed, KeyPress, getCurrentlyFocusedComponent, addKeyListener
    */
    virtual bool keyStateChanged (bool isKeyDown);

    /** Called when a modifier key is pressed or released.

        Whenever the shift, control, alt or command keys are pressed or released,
        this method will be called on the component that currently has the keyboard focus.
        Remember that a component will only be given the focus if its setWantsKeyboardFocus()
        method has been used to enable this.

        The default implementation of this method actually calls its parent's modifierKeysChanged
        method, so that focused components which aren't interested in this will give their
        parents a chance to act on the event instead.

        @see keyStateChanged, ModifierKeys
    */
    virtual void modifierKeysChanged (const ModifierKeys& modifiers);

    //==============================================================================
    /** Enumeration used by the focusChanged() and focusLost() methods. */
    enum FocusChangeType
    {
        focusChangedByMouseClick,   /**< Means that the user clicked the mouse to change focus. */
        focusChangedByTabKey,       /**< Means that the user pressed the tab key to move the focus. */
        focusChangedDirectly        /**< Means that the focus was changed by a call to grabKeyboardFocus(). */
    };

    /** Called to indicate that this component has just acquired the keyboard focus.
        @see focusLost, setWantsKeyboardFocus, getCurrentlyFocusedComponent, hasKeyboardFocus
    */
    virtual void focusGained (FocusChangeType cause);

    /** Called to indicate that this component has just lost the keyboard focus.
        @see focusGained, setWantsKeyboardFocus, getCurrentlyFocusedComponent, hasKeyboardFocus
    */
    virtual void focusLost (FocusChangeType cause);

    /** Called to indicate a change in whether or not this component is the parent of the
        currently-focused component.

        Essentially this is called when the return value of a call to hasKeyboardFocus (true) has
        changed. It happens when focus moves from one of this component's children (at any depth)
        to a component that isn't contained in this one, (or vice-versa).
        Note that this method does NOT get called to when focus simply moves from one of its
        child components to another.

        @see focusGained, setWantsKeyboardFocus, getCurrentlyFocusedComponent, hasKeyboardFocus
    */
    virtual void focusOfChildComponentChanged (FocusChangeType cause);

    //==============================================================================
    /** Returns true if the mouse is currently over this component.

        If the mouse isn't over the component, this will return false, even if the
        mouse is currently being dragged - so you can use this in your mouseDrag
        method to find out whether it's really over the component or not.

        Note that when the mouse button is being held down, then the only component
        for which this method will return true is the one that was originally
        clicked on.

        If includeChildren is true, then this will also return true if the mouse is over
        any of the component's children (recursively) as well as the component itself.

        @see isMouseButtonDown. isMouseOverOrDragging, mouseDrag
    */
    bool isMouseOver (bool includeChildren = false) const;

    /** Returns true if the mouse button is currently held down in this component.

        Note that this is a test to see whether the mouse is being pressed in this
        component, so it'll return false if called on component A when the mouse
        is actually being dragged in component B.

        @see isMouseButtonDownAnywhere, isMouseOver, isMouseOverOrDragging
    */
    bool isMouseButtonDown() const;

    /** True if the mouse is over this component, or if it's being dragged in this component.
        This is a handy equivalent to (isMouseOver() || isMouseButtonDown()).
        @see isMouseOver, isMouseButtonDown, isMouseButtonDownAnywhere
    */
    bool isMouseOverOrDragging() const;

    /** Returns true if a mouse button is currently down.

        Unlike isMouseButtonDown, this will test the current state of the
        buttons without regard to which component (if any) it has been
        pressed in.

        @see isMouseButtonDown, ModifierKeys
    */
    static bool JUCE_CALLTYPE isMouseButtonDownAnywhere() noexcept;

    /** Returns the mouse's current position, relative to this component.
        The return value is relative to the component's top-left corner.
    */
    Point<int> getMouseXYRelative() const;

    //==============================================================================
    /** Called when this component's size has been changed.

        A component can implement this method to do things such as laying out its
        child components when its width or height changes.

        The method is called synchronously as a result of the setBounds or setSize
        methods, so repeatedly changing a components size will repeatedly call its
        resized method (unlike things like repainting, where multiple calls to repaint
        are coalesced together).

        If the component is a top-level window on the desktop, its size could also
        be changed by operating-system factors beyond the application's control.

        @see moved, setSize
    */
    virtual void resized();

    /** Called when this component's position has been changed.

        This is called when the position relative to its parent changes, not when
        its absolute position on the screen changes (so it won't be called for
        all child components when a parent component is moved).

        The method is called synchronously as a result of the setBounds, setTopLeftPosition
        or any of the other repositioning methods, and like resized(), it will be
        called each time those methods are called.

        If the component is a top-level window on the desktop, its position could also
        be changed by operating-system factors beyond the application's control.

        @see resized, setBounds
    */
    virtual void moved();

    /** Called when one of this component's children is moved or resized.

        If the parent wants to know about changes to its immediate children (not
        to children of its children), this is the method to override.

        @see moved, resized, parentSizeChanged
    */
    virtual void childBoundsChanged (Component* child);

    /** Called when this component's immediate parent has been resized.

        If the component is a top-level window, this indicates that the screen size
        has changed.

        @see childBoundsChanged, moved, resized
    */
    virtual void parentSizeChanged();

    /** Called when this component has been moved to the front of its siblings.

        The component may have been brought to the front by the toFront() method, or
        by the operating system if it's a top-level window.

        @see toFront
    */
    virtual void broughtToFront();

    /** Adds a listener to be told about changes to the component hierarchy or position.

        Component listeners get called when this component's size, position or children
        change - see the ComponentListener class for more details.

        @param newListener  the listener to register - if this is already registered, it
                            will be ignored.
        @see ComponentListener, removeComponentListener
    */
    void addComponentListener (ComponentListener* newListener);

    /** Removes a component listener.
        @see addComponentListener
    */
    void removeComponentListener (ComponentListener* listenerToRemove);

    //==============================================================================
    /** Dispatches a numbered message to this component.

        This is a quick and cheap way of allowing simple asynchronous messages to
        be sent to components. It's also safe, because if the component that you
        send the message to is a null or dangling pointer, this won't cause an error.

        The command ID is later delivered to the component's handleCommandMessage() method by
        the application's message queue.

        @see handleCommandMessage
    */
    void postCommandMessage (int commandId);

    /** Called to handle a command that was sent by postCommandMessage().

        This is called by the message thread when a command message arrives, and
        the component can override this method to process it in any way it needs to.

        @see postCommandMessage
    */
    virtual void handleCommandMessage (int commandId);

    //==============================================================================
    /** Runs a component modally, waiting until the loop terminates.

        This method first makes the component visible, brings it to the front and
        gives it the keyboard focus.

        It then runs a loop, dispatching messages from the system message queue, but
        blocking all mouse or keyboard messages from reaching any components other
        than this one and its children.

        This loop continues until the component's exitModalState() method is called (or
        the component is deleted), and then this method returns, returning the value
        passed into exitModalState().

        @see enterModalState, exitModalState, isCurrentlyModal, getCurrentlyModalComponent,
             isCurrentlyBlockedByAnotherModalComponent, ModalComponentManager
    */
   #if JUCE_MODAL_LOOPS_PERMITTED
    int runModalLoop();
   #endif

    /** Puts the component into a modal state.

        This makes the component modal, so that messages are blocked from reaching
        any components other than this one and its children, but unlike runModalLoop(),
        this method returns immediately.

        If takeKeyboardFocus is true, the component will use grabKeyboardFocus() to
        get the focus, which is usually what you'll want it to do. If not, it will leave
        the focus unchanged.

        The callback is an optional object which will receive a callback when the modal
        component loses its modal status, either by being hidden or when exitModalState()
        is called. If you pass an object in here, the system will take care of deleting it
        later, after making the callback

        If deleteWhenDismissed is true, then when it is dismissed, the component will be
        deleted and then the callback will be called. (This will safely handle the situation
        where the component is deleted before its exitModalState() method is called).

        @see exitModalState, runModalLoop, ModalComponentManager::attachCallback
    */
    void enterModalState (bool takeKeyboardFocus = true,
                          ModalComponentManager::Callback* callback = nullptr,
                          bool deleteWhenDismissed = false);

    /** Ends a component's modal state.

        If this component is currently modal, this will turn off its modalness, and return
        a value to the runModalLoop() method that might have be running its modal loop.

        @see runModalLoop, enterModalState, isCurrentlyModal
    */
    void exitModalState (int returnValue);

    /** Returns true if this component is the modal one.

        It's possible to have nested modal components, e.g. a pop-up dialog box
        that launches another pop-up, but this will only return true for
        the one at the top of the stack.

        @see getCurrentlyModalComponent
    */
    bool isCurrentlyModal() const noexcept;

    /** Returns the number of components that are currently in a modal state.
        @see getCurrentlyModalComponent
     */
    static int JUCE_CALLTYPE getNumCurrentlyModalComponents() noexcept;

    /** Returns one of the components that are currently modal.

        The index specifies which of the possible modal components to return. The order
        of the components in this list is the reverse of the order in which they became
        modal - so the component at index 0 is always the active component, and the others
        are progressively earlier ones that are themselves now blocked by later ones.

        @returns the modal component, or null if no components are modal (or if the
                index is out of range)
        @see getNumCurrentlyModalComponents, runModalLoop, isCurrentlyModal
    */
    static Component* JUCE_CALLTYPE getCurrentlyModalComponent (int index = 0) noexcept;

    /** Checks whether there's a modal component somewhere that's stopping this one
        from receiving messages.

        If there is a modal component, its canModalEventBeSentToComponent() method
        will be called to see if it will still allow this component to receive events.

        @see runModalLoop, getCurrentlyModalComponent
    */
    bool isCurrentlyBlockedByAnotherModalComponent() const;

    /** When a component is modal, this callback allows it to choose which other
        components can still receive events.

        When a modal component is active and the user clicks on a non-modal component,
        this method is called on the modal component, and if it returns true, the
        event is allowed to reach its target. If it returns false, the event is blocked
        and the inputAttemptWhenModal() callback is made.

        It called by the isCurrentlyBlockedByAnotherModalComponent() method. The default
        implementation just returns false in all cases.
    */
    virtual bool canModalEventBeSentToComponent (const Component* targetComponent);

    /** Called when the user tries to click on a component that is blocked by another
        modal component.

        When a component is modal and the user clicks on one of the other components,
        the modal component will receive this callback.

        The default implementation of this method will play a beep, and bring the currently
        modal component to the front, but it can be overridden to do other tasks.

        @see isCurrentlyBlockedByAnotherModalComponent, canModalEventBeSentToComponent
    */
    virtual void inputAttemptWhenModal();


    //==============================================================================
    /** Returns the set of properties that belong to this component.
        Each component has a NamedValueSet object which you can use to attach arbitrary
        items of data to it.
    */
    NamedValueSet& getProperties() noexcept                             { return properties; }

    /** Returns the set of properties that belong to this component.
        Each component has a NamedValueSet object which you can use to attach arbitrary
        items of data to it.
    */
    const NamedValueSet& getProperties() const noexcept                 { return properties; }

    //==============================================================================
    /** Looks for a colour that has been registered with the given colour ID number.

        If a colour has been set for this ID number using setColour(), then it is
        returned. If none has been set, the method will try calling the component's
        LookAndFeel class's findColour() method. If none has been registered with the
        look-and-feel either, it will just return black.

        The colour IDs for various purposes are stored as enums in the components that
        they are relevent to - for an example, see Slider::ColourIds,
        Label::ColourIds, TextEditor::ColourIds, TreeView::ColourIds, etc.

        @see setColour, isColourSpecified, colourChanged, LookAndFeel::findColour, LookAndFeel::setColour
    */
    Colour findColour (int colourId, bool inheritFromParent = false) const;

    /** Registers a colour to be used for a particular purpose.

        Changing a colour will cause a synchronous callback to the colourChanged()
        method, which your component can override if it needs to do something when
        colours are altered.

        For more details about colour IDs, see the comments for findColour().

        @see findColour, isColourSpecified, colourChanged, LookAndFeel::findColour, LookAndFeel::setColour
    */
    void setColour (int colourId, Colour newColour);

    /** If a colour has been set with setColour(), this will remove it.
        This allows you to make a colour revert to its default state.
    */
    void removeColour (int colourId);

    /** Returns true if the specified colour ID has been explicitly set for this
        component using the setColour() method.
    */
    bool isColourSpecified (int colourId) const;

    /** This looks for any colours that have been specified for this component,
        and copies them to the specified target component.
    */
    void copyAllExplicitColoursTo (Component& target) const;

    /** This method is called when a colour is changed by the setColour() method.
        @see setColour, findColour
    */
    virtual void colourChanged();

    //==============================================================================
    /** Components can implement this method to provide a MarkerList.
        The default implementation of this method returns nullptr, but you can override
        it to return a pointer to the component's marker list. If xAxis is true, it should
        return the X marker list; if false, it should return the Y markers.
    */
    virtual MarkerList* getMarkers (bool xAxis);

    //==============================================================================
    /** Returns the underlying native window handle for this component.

        This is platform-dependent and strictly for power-users only!
    */
    void* getWindowHandle() const;

    //==============================================================================
    /** Holds a pointer to some type of Component, which automatically becomes null if
        the component is deleted.

        If you're using a component which may be deleted by another event that's outside
        of your control, use a SafePointer instead of a normal pointer to refer to it,
        and you can test whether it's null before using it to see if something has deleted
        it.

        The ComponentType typedef must be Component, or some subclass of Component.

        You may also want to use a WeakReference<Component> object for the same purpose.
    */
    template <class ComponentType>
    class SafePointer
    {
    public:
        /** Creates a null SafePointer. */
        SafePointer() noexcept {}

        /** Creates a SafePointer that points at the given component. */
        SafePointer (ComponentType* component)                : weakRef (component) {}

        /** Creates a copy of another SafePointer. */
        SafePointer (const SafePointer& other) noexcept       : weakRef (other.weakRef) {}

        /** Copies another pointer to this one. */
        SafePointer& operator= (const SafePointer& other)     { weakRef = other.weakRef; return *this; }

        /** Copies another pointer to this one. */
        SafePointer& operator= (ComponentType* newComponent)  { weakRef = newComponent; return *this; }

        /** Returns the component that this pointer refers to, or null if the component no longer exists. */
        ComponentType* getComponent() const noexcept          { return dynamic_cast<ComponentType*> (weakRef.get()); }

        /** Returns the component that this pointer refers to, or null if the component no longer exists. */
        operator ComponentType*() const noexcept              { return getComponent(); }

        /** Returns the component that this pointer refers to, or null if the component no longer exists. */
        ComponentType* operator->() noexcept                  { return getComponent(); }

        /** Returns the component that this pointer refers to, or null if the component no longer exists. */
        const ComponentType* operator->() const noexcept      { return getComponent(); }

        /** If the component is valid, this deletes it and sets this pointer to null. */
        void deleteAndZero()                                  { delete getComponent(); }

        bool operator== (ComponentType* component) const noexcept   { return weakRef == component; }
        bool operator!= (ComponentType* component) const noexcept   { return weakRef != component; }

    private:
        WeakReference<Component> weakRef;
    };

    //==============================================================================
    /** A class to keep an eye on a component and check for it being deleted.

        This is designed for use with the ListenerList::callChecked() methods, to allow
        the list iterator to stop cleanly if the component is deleted by a listener callback
        while the list is still being iterated.
    */
    class JUCE_API  BailOutChecker
    {
    public:
        /** Creates a checker that watches one component. */
        BailOutChecker (Component* component);

        /** Returns true if either of the two components have been deleted since this object was created. */
        bool shouldBailOut() const noexcept;

    private:
        const WeakReference<Component> safePointer;

        JUCE_DECLARE_NON_COPYABLE (BailOutChecker)
    };

    //==============================================================================
    /**
        Base class for objects that can be used to automatically position a component according to
        some kind of algorithm.

        The component class simply holds onto a reference to a Positioner, but doesn't actually do
        anything with it - all the functionality must be implemented by the positioner itself (e.g.
        it might choose to watch some kind of value and move the component when the value changes).
    */
    class JUCE_API  Positioner
    {
    public:
        /** Creates a Positioner which can control the specified component. */
        explicit Positioner (Component& component) noexcept;
        /** Destructor. */
        virtual ~Positioner() {}

        /** Returns the component that this positioner controls. */
        Component& getComponent() const noexcept    { return component; }

        /** Attempts to set the component's position to the given rectangle.
            Unlike simply calling Component::setBounds(), this may involve the positioner
            being smart enough to adjust itself to fit the new bounds, e.g. a RelativeRectangle's
            positioner may try to reverse the expressions used to make them fit these new coordinates.
        */
        virtual void applyNewBounds (const Rectangle<int>& newBounds) = 0;

    private:
        Component& component;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Positioner)
    };

    /** Returns the Positioner object that has been set for this component.
        @see setPositioner()
    */
    Positioner* getPositioner() const noexcept;

    /** Sets a new Positioner object for this component.
        If there's currently another positioner set, it will be deleted. The object that is passed in
        will be deleted automatically by this component when it's no longer required. Pass a null pointer
        to clear the current positioner.
        @see getPositioner()
    */
    void setPositioner (Positioner* newPositioner);

    /** Gives the component a CachedComponentImage that should be used to buffer its painting.
        The object that is passed-in will be owned by this component, and will be deleted automatically
        later on.
        @see setBufferedToImage
    */
    void setCachedComponentImage (CachedComponentImage* newCachedImage);

    /** Returns the object that was set by setCachedComponentImage().
        @see setCachedComponentImage
    */
    CachedComponentImage* getCachedComponentImage() const noexcept  { return cachedImage; }

    //==============================================================================
    // These methods are deprecated - use localPointToGlobal, getLocalPoint, getLocalPoint, etc instead.
    JUCE_DEPRECATED (Point<int> relativePositionToGlobal (Point<int>) const);
    JUCE_DEPRECATED (Point<int> globalPositionToRelative (Point<int>) const);
    JUCE_DEPRECATED (Point<int> relativePositionToOtherComponent (const Component*, Point<int>) const);

private:
    //==============================================================================
    friend class ComponentPeer;
    friend class MouseInputSource;
    friend class MouseInputSourceInternal;

   #ifndef DOXYGEN
    static Component* currentlyFocusedComponent;

    //==============================================================================
    String componentName, componentID;
    Component* parentComponent;
    Rectangle<int> bounds;
    ScopedPointer<Positioner> positioner;
    ScopedPointer<AffineTransform> affineTransform;
    Array<Component*> childComponentList;
    LookAndFeel* lookAndFeel;
    MouseCursor cursor;
    ImageEffectFilter* effect;
    ScopedPointer<CachedComponentImage> cachedImage;

    class MouseListenerList;
    friend class MouseListenerList;
    friend struct ContainerDeletePolicy<MouseListenerList>;
    ScopedPointer<MouseListenerList> mouseListeners;
    ScopedPointer<Array<KeyListener*> > keyListeners;
    ListenerList<ComponentListener> componentListeners;
    NamedValueSet properties;

    friend class WeakReference<Component>;
    WeakReference<Component>::Master masterReference;

    struct ComponentFlags
    {
        bool hasHeavyweightPeerFlag     : 1;
        bool visibleFlag                : 1;
        bool opaqueFlag                 : 1;
        bool ignoresMouseClicksFlag     : 1;
        bool allowChildMouseClicksFlag  : 1;
        bool wantsFocusFlag             : 1;
        bool isFocusContainerFlag       : 1;
        bool dontFocusOnMouseClickFlag  : 1;
        bool alwaysOnTopFlag            : 1;
        bool bufferToImageFlag          : 1;
        bool bringToFrontOnClickFlag    : 1;
        bool repaintOnMouseActivityFlag : 1;
        bool currentlyModalFlag         : 1;
        bool isDisabledFlag             : 1;
        bool childCompFocusedFlag       : 1;
        bool dontClipGraphicsFlag       : 1;
        bool mouseDownWasBlocked        : 1;
        bool isMoveCallbackPending      : 1;
        bool isResizeCallbackPending    : 1;
       #if JUCE_DEBUG
        bool isInsidePaintCall          : 1;
       #endif
    };

    union
    {
        uint32 componentFlags;
        ComponentFlags flags;
    };

    uint8 componentTransparency;

    //==============================================================================
    void internalMouseEnter (MouseInputSource, Point<float>, Time);
    void internalMouseExit  (MouseInputSource, Point<float>, Time);
    void internalMouseDown  (MouseInputSource, Point<float>, Time);
    void internalMouseUp    (MouseInputSource, Point<float>, Time, const ModifierKeys oldModifiers);
    void internalMouseDrag  (MouseInputSource, Point<float>, Time);
    void internalMouseMove  (MouseInputSource, Point<float>, Time);
    void internalMouseWheel (MouseInputSource, Point<float>, Time, const MouseWheelDetails&);
    void internalMagnifyGesture (MouseInputSource, Point<float>, Time, float);
    void internalBroughtToFront();
    void internalFocusGain (FocusChangeType, const WeakReference<Component>&);
    void internalFocusGain (FocusChangeType);
    void internalFocusLoss (FocusChangeType);
    void internalChildFocusChange (FocusChangeType, const WeakReference<Component>&);
    void internalModalInputAttempt();
    void internalModifierKeysChanged();
    void internalChildrenChanged();
    void internalHierarchyChanged();
    void internalRepaint (const Rectangle<int>&);
    void internalRepaintUnchecked (const Rectangle<int>&, bool);
    Component* removeChildComponent (int index, bool sendParentEvents, bool sendChildEvents);
    void reorderChildInternal (int sourceIndex, int destIndex);
    void paintComponentAndChildren (Graphics&);
    void paintWithinParentContext (Graphics&);
    void sendMovedResizedMessages (bool wasMoved, bool wasResized);
    void sendMovedResizedMessagesIfPending();
    void repaintParent();
    void sendFakeMouseMove() const;
    void takeKeyboardFocus (const FocusChangeType);
    void grabFocusInternal (const FocusChangeType, bool canTryParent);
    static void giveAwayFocus (bool sendFocusLossEvent);
    void sendEnablementChangeMessage();
    void sendVisibilityChangeMessage();

    struct ComponentHelpers;
    friend struct ComponentHelpers;

    /* Components aren't allowed to have copy constructors, as this would mess up parent hierarchies.
       You might need to give your subclasses a private dummy constructor to avoid compiler warnings.
    */
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Component)

    //==============================================================================
   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This is included here just to cause a compile error if your code is still handling
    // drag-and-drop with this method. If so, just update it to use the new FileDragAndDropTarget
    // class, which is easy (just make your class inherit from FileDragAndDropTarget, and
    // implement its methods instead of this Component method).
    virtual void filesDropped (const StringArray&, int, int) {}

    // This is included here to cause an error if you use or overload it - it has been deprecated in
    // favour of contains (Point<int>)
    void contains (int, int) JUCE_DELETED_FUNCTION;
   #endif

protected:
    //==============================================================================
    /** @internal */
    virtual ComponentPeer* createNewPeer (int styleFlags, void* nativeWindowToAttachTo);
   #endif
};


#endif   // JUCE_COMPONENT_H_INCLUDED
