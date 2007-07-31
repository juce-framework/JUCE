/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_COMPONENT_JUCEHEADER__
#define __JUCE_COMPONENT_JUCEHEADER__

#include "mouse/juce_MouseCursor.h"
#include "mouse/juce_MouseListener.h"
#include "juce_ComponentListener.h"
#include "keyboard/juce_KeyListener.h"
#include "keyboard/juce_KeyboardFocusTraverser.h"
#include "../graphics/effects/juce_ImageEffectFilter.h"
#include "../graphics/geometry/juce_RectangleList.h"
#include "../graphics/geometry/juce_BorderSize.h"
#include "windows/juce_ComponentPeer.h"
#include "../../events/juce_MessageListener.h"
#include "../../../juce_core/text/juce_StringArray.h"
#include "../../../juce_core/containers/juce_VoidArray.h"
#include "../../../juce_core/containers/juce_PropertySet.h"
class LookAndFeel;


//==============================================================================
/**
    The base class for all JUCE user-interface objects.

*/
class JUCE_API  Component  : public MouseListener,
                             protected MessageListener
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
    Component() throw();

    /** Destructor.

        Note that when a component is deleted, any child components it might
        contain are NOT deleted unless you explicitly call deleteAllChildren() first.
    */
    virtual ~Component();

    //==============================================================================
    /** Creates a component, setting its name at the same time.

        @see getName, setName
    */
    Component (const String& componentName) throw();

    /** Returns the name of this component.

        @see setName
    */
    const String& getName() const throw()                   { return componentName_; }

    /** Sets the name of this component.

        When the name changes, all registered ComponentListeners will receive a
        ComponentListener::componentNameChanged() callback.

        @see getName
    */
    virtual void setName (const String& newName);

    //==============================================================================
    /** Checks whether this Component object has been deleted.

        This will check whether this object is still a valid component, or whether
        it's been deleted.

        It's safe to call this on null or dangling pointers, but note that there is a
        small risk if another new (but different) component has been created at the
        same memory address which this one occupied, this methods can return a
        false positive.
    */
    bool isValidComponent() const throw();

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
    bool isVisible() const throw()                          { return flags.visibleFlag; }

    /** Called when this component's visiblility changes.

        @see setVisible, isVisible
    */
    virtual void visibilityChanged();

    /** Tests whether this component and all its parents are visible.

        @returns    true only if this component and all its parents are visible.
        @see isVisible
    */
    bool isShowing() const throw();

    /** Makes a component invisible using a groovy fade-out and animated zoom effect.

        To do this, this function will cunningly:
            - take a snapshot of the component as it currently looks
            - call setVisible(false) on the component
            - replace it with a special component that will continue drawing the
              snapshot, animating it and gradually making it more transparent
            - when it's gone, the special component will also be deleted

        As soon as this method returns, the component can be safely removed and deleted
        leaving the proxy to do the fade-out, so it's even ok to call this in a
        component's destructor.

        Passing non-zero x and y values will cause the ghostly component image to
        also whizz off by this distance while fading out. If the scale factor is
        not 1.0, it will also zoom from the component's current size to this new size.

        One thing to be careful about is that the parent component must be able to cope
        with this unknown component type being added to it.
    */
    void fadeOutComponent (const int lengthOfFadeOutInMilliseconds,
                           const int deltaXToMove = 0,
                           const int deltaYToMove = 0,
                           const float scaleFactorAtEnd = 1.0f);

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
                               void* nativeWindowToAttachTo = 0);

    /** If the component is currently showing on the desktop, this will hide it.

        You can also use setVisible() to hide a desktop window temporarily, but
        removeFromDesktop() will free any system resources that are being used up.

        @see addToDesktop, isOnDesktop
    */
    void removeFromDesktop();

    /** Returns true if this component is currently showing on the desktop.

        @see addToDesktop, removeFromDesktop
    */
    bool isOnDesktop() const throw();

    /** Returns the heavyweight window that contains this component.

        If this component is itself on the desktop, this will return the window
        object that it is using. Otherwise, it will return the window of
        its top-level parent component.

        This may return 0 if there isn't a desktop component.

        @see addToDesktop, isOnDesktop
    */
    ComponentPeer* getPeer() const throw();

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

    //==============================================================================
    /** Brings the component to the front of its siblings.

        If some of the component's siblings have had their 'always-on-top' flag set,
        then they will still be kept in front of this one (unless of course this
        one is also 'always-on-top').

        @param shouldAlsoGainFocus  if true, this will also try to assign keyboard focus
                                    to the component (see grabKeyboardFocus() for more details)
        @see toBack, toBehind, setAlwaysOnTop
    */
    void toFront (const bool shouldAlsoGainFocus);

    /** Changes this component's z-order to be at the back of all its siblings.

        If the component is set to be 'always-on-top', it will only be moved to the
        back of the other other 'always-on-top' components.

        @see toFront, toBehind, setAlwaysOnTop
    */
    void toBack();

    /** Changes this component's z-order so that it's just behind another component.

        @see toFront, toBack
    */
    void toBehind (Component* const other);

    /** Sets whether the component should always be kept at the front of its siblings.

        @see isAlwaysOnTop
    */
    void setAlwaysOnTop (const bool shouldStayOnTop);

    /** Returns true if this component is set to always stay in front of its siblings.

        @see setAlwaysOnTop
    */
    bool isAlwaysOnTop() const throw();

    //==============================================================================
    /** Returns the x co-ordinate of the component's left edge.

        This is a distance in pixels from the left edge of the component's parent.

        @see getScreenX
    */
    inline int getX() const throw()                         { return bounds_.getX(); }

    /** Returns the y co-ordinate of the top of this component.

        This is a distance in pixels from the top edge of the component's parent.

        @see getScreenY
    */
    inline int getY() const throw()                         { return bounds_.getY(); }

    /** Returns the component's width in pixels. */
    inline int getWidth() const throw()                     { return bounds_.getWidth(); }

    /** Returns the component's height in pixels. */
    inline int getHeight() const throw()                    { return bounds_.getHeight(); }

    /** Returns the x co-ordinate of the component's right-hand edge.

        This is a distance in pixels from the left edge of the component's parent.
    */
    int getRight() const throw()                            { return bounds_.getRight(); }

    /** Returns the y co-ordinate of the bottom edge of this component.

        This is a distance in pixels from the top edge of the component's parent.
    */
    int getBottom() const throw()                           { return bounds_.getBottom(); }

    /** Returns this component's bounding box.

        The rectangle returned is relative to the top-left of the component's parent.
    */
    const Rectangle& getBounds() const throw()              { return bounds_; }

    /** Returns the region of this component that's not obscured by other, opaque components.

        The RectangleList that is returned represents the area of this component
        which isn't covered by opaque child components.

        If includeSiblings is true, it will also take into account any siblings
        that may be overlapping the component.
    */
    void getVisibleArea (RectangleList& result,
                         const bool includeSiblings) const;

    //==============================================================================
    /** Returns this component's x co-ordinate relative the the screen's top-left origin.

        @see getX, relativePositionToGlobal
    */
    int getScreenX() const throw();

    /** Returns this component's y co-ordinate relative the the screen's top-left origin.

        @see getY, relativePositionToGlobal
    */
    int getScreenY() const throw();

    /** Converts a position relative to this component's top-left into a screen co-ordinate.

        @see globalPositionToRelative, relativePositionToOtherComponent
    */
    void relativePositionToGlobal (int& x, int& y) const throw();

    /** Converts a screen co-ordinate into a position relative to this component's top-left.

        @see relativePositionToGlobal, relativePositionToOtherComponent
    */
    void globalPositionToRelative (int& x, int& y) const throw();

    /** Converts a position relative to this component's top-left into a position
        relative to another component's top-left.

        @see relativePositionToGlobal, globalPositionToRelative
    */
    void relativePositionToOtherComponent (const Component* const targetComponent,
                                           int& x, int& y) const throw();

    //==============================================================================
    /** Moves the component to a new position.

        Changes the component's top-left position (without changing its size).
        The position is relative to the top-left of the component's parent.

        If the component actually moves, this method will make a synchronous call to moved().

        @see setBounds, ComponentListener::componentMovedOrResized
    */
    void setTopLeftPosition (const int x, const int y);

    /** Moves the component to a new position.

        Changes the position of the component's top-right corner (keeping it the same size).
        The position is relative to the top-left of the component's parent.

        If the component actually moves, this method will make a synchronous call to moved().
    */
    void setTopRightPosition (const int x, const int y);

    /** Changes the size of the component.

        A synchronous call to resized() will be occur if the size actually changes.
    */
    void setSize (const int newWidth, const int newHeight);

    /** Changes the component's position and size.

        The co-ordinates are relative to the top-left of the component's parent, or relative
        to the origin of the screen is the component is on the desktop.

        If this method changes the component's top-left position, it will make a synchronous
        call to moved(). If it changes the size, it will also make a call to resized().

        @see setTopLeftPosition, setSize, ComponentListener::componentMovedOrResized
    */
    void setBounds (int x, int y, int width, int height);

    /** Changes the component's position and size.

        @see setBounds
    */
    void setBounds (const Rectangle& newBounds);

    /** Changes the component's position and size in terms of fractions of its parent's size.

        The values are factors of the parent's size, so for example
        setBoundsRelative (0.2f, 0.2f, 0.5f, 0.5f) would give it half the
        width and height of the parent, with its top-left position 20% of
        the way across and down the parent.
    */
    void setBoundsRelative (const float proportionalX, const float proportionalY,
                            const float proportionalWidth, const float proportionalHeight);

    /** Changes the component's position and size based on the amount of space to leave around it.

        This will position the component within its parent, leaving the specified number of
        pixels around each edge.
    */
    void setBoundsInset (const BorderSize& borders);

    /** Positions the component within a given rectangle, keeping its proportions
        unchanged.

        If onlyReduceInSize is false, the component will be resized to fill as much of the
        rectangle as possible without changing its aspect ratio (the component's
        current size is used to determine its aspect ratio, so a zero-size component
        won't work here). If onlyReduceInSize is true, it will only be resized if it's
        too big to fit inside the rectangle.

        It will then be positioned within the rectangle according to the justification flags
        specified.
    */
    void setBoundsToFit (int x, int y, int width, int height,
                         const Justification& justification,
                         const bool onlyReduceInSize);

    /** Changes the position of the component's centre.

        Leaves the component's size unchanged, but sets the position of its centre
        relative to its parent's top-left.
    */
    void setCentrePosition (const int x, const int y);

    /** Changes the position of the component's centre.

        Leaves the position unchanged, but positions its centre relative to its
        parent's size. E.g. setCentreRelative (0.5f, 0.5f) would place it centrally in
        its parent.
    */
    void setCentreRelative (const float x, const float y);

    /** Changes the component's size and centres it within its parent.

        After changing the size, the component will be moved so that it's
        centred within its parent.
    */
    void centreWithSize (const int width, const int height);

    //==============================================================================
    /** Returns a proportion of the component's width.

        This is a handy equivalent of (getWidth() * proportion).
    */
    int proportionOfWidth (const float proportion) const throw();

    /** Returns a proportion of the component's height.

        This is a handy equivalent of (getHeight() * proportion).
    */
    int proportionOfHeight (const float proportion) const throw();

    /** Returns the width of the component's parent.

        If the component has no parent (i.e. if it's on the desktop), this will return
        the width of the screen.
    */
    int getParentWidth() const throw();

    /** Returns the height of the component's parent.

        If the component has no parent (i.e. if it's on the desktop), this will return
        the height of the screen.
    */
    int getParentHeight() const throw();

    /** Returns the screen co-ordinates of the monitor that contains this component.

        If there's only one monitor, this will return its size - if there are multiple
        monitors, it will return the area of the monitor that contains the component's
        centre.
    */
    const Rectangle getParentMonitorArea() const throw();

    //==============================================================================
    /** Returns the number of child components that this component contains.

        @see getChildComponent, getIndexOfChildComponent
    */
    int getNumChildComponents() const throw();

    /** Returns one of this component's child components, by it index.

        The component with index 0 is at the back of the z-order, the one at the
        front will have index (getNumChildComponents() - 1).

        If the index is out-of-range, this will return a null pointer.

        @see getNumChildComponents, getIndexOfChildComponent
    */
    Component* getChildComponent (const int index) const throw();

    /** Returns the index of this component in the list of child components.

        A value of 0 means it is first in the list (i.e. behind all other components). Higher
        values are further towards the front.

        Returns -1 if the component passed-in is not a child of this component.

        @see getNumChildComponents, getChildComponent, addChildComponent, toFront, toBack, toBehind
    */
    int getIndexOfChildComponent (const Component* const child) const throw();

    /** Adds a child component to this one.

        @param child    the new component to add. If the component passed-in is already
                        the child of another component, it'll first be removed from that.

        @param zOrder   The index in the child-list at which this component should be inserted.
                        A value of -1 will insert it in front of the others, 0 is the back.
        @see removeChildComponent, addAndMakeVisible, getChild,
             ComponentListener::componentChildrenChanged
    */
    void addChildComponent (Component* const child,
                            int zOrder = -1);

    /** Adds a child component to this one, and also makes the child visible if it isn't.

        Quite a useful function, this is just the same as calling addChildComponent()
        followed by setVisible (true) on the child.
    */
    void addAndMakeVisible (Component* const child,
                            int zOrder = -1);

    /** Removes one of this component's child-components.

        If the child passed-in isn't actually a child of this component (either because
        it's invalid or is the child of a different parent), then nothing is done.

        Note that removing a child will not delete it!

        @see addChildComponent, ComponentListener::componentChildrenChanged
    */
    void removeChildComponent (Component* const childToRemove);

    /** Removes one of this component's child-components by index.

        This will return a pointer to the component that was removed, or null if
        the index was out-of-range.

        Note that removing a child will not delete it!

        @see addChildComponent, ComponentListener::componentChildrenChanged
    */
    Component* removeChildComponent (const int childIndexToRemove);

    /** Removes all this component's children.

        Note that this won't delete them! To do that, use deleteAllChildren() instead.
    */
    void removeAllChildren();

    /** Removes all this component's children, and deletes them.

        @see removeAllChildren
    */
    void deleteAllChildren();

    /** Returns the component which this component is inside.

        If this is the highest-level component or hasn't yet been added to
        a parent, this will return null.
    */
    Component* getParentComponent() const throw()                   { return parentComponent_; }

    /** Searches the parent components for a component of a specified class.

        For example findParentComponentOfClass \<MyComp\>() would return the first parent
        component that can be dynamically cast to a MyComp, or will return 0 if none
        of the parents are suitable.

        N.B. The dummy parameter is needed to work around a VC6 compiler bug.
    */
    template <class TargetClass>
    TargetClass* findParentComponentOfClass (TargetClass* const dummyParameter = 0) const
    {
        (void) dummyParameter;
        Component* p = parentComponent_;
        while (p != 0)
        {
            TargetClass* target = dynamic_cast <TargetClass*> (p);
            if (target != 0)
                return target;

            p = p->parentComponent_;
        }

        return 0;
    }

    /** Returns the highest-level component which contains this one or its parents.

        This will search upwards in the parent-hierarchy from this component, until it
        finds the highest one that doesn't have a parent (i.e. is on the desktop or
        not yet added to a parent), and will return that.
    */
    Component* getTopLevelComponent() const throw();

    /** Checks whether a component is anywhere inside this component or its children.

        This will recursively check through this components children to see if the
        given component is anywhere inside.
    */
    bool isParentOf (const Component* possibleChild) const throw();

    //==============================================================================
    /** Called to indicate that the component's parents have changed.

        When a component is added or removed from its parent, this method will
        be called on all of its children (recursively - so all children of its
        children will also be called as well).

        Subclasses can override this if they need to react to this in some way.

        @see getParentComponent, isShowing, ComponentListener::componentParentHierarchyChanged
    */
    virtual void parentHierarchyChanged();

    /** Subclasses can use this callback to be told when children are added or removed.

        @see parentHierarchyChanged
    */
    virtual void childrenChanged();

    //==============================================================================
    /** Tests whether a given point inside the component.

        Overriding this method allows you to create components which only intercept
        mouse-clicks within a user-defined area.

        This is called to find out whether a particular x, y co-ordinate is
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

        @param x    the x co-ordinate to test, relative to the left hand edge of this
                    component. This value is guaranteed to be greater than or equal to
                    zero, and less than the component's width
        @param y    the y co-ordinate to test, relative to the top edge of this
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
    void setInterceptsMouseClicks (const bool allowClicksOnThisComponent,
                                   const bool allowClicksOnChildComponents) throw();

    /** Retrieves the current state of the mouse-click interception flags.

        On return, the two parameters are set to the state used in the last call to
        setInterceptsMouseClicks().

        @see setInterceptsMouseClicks
    */
    void getInterceptsMouseClicks (bool& allowsClicksOnThisComponent,
                                   bool& allowsClicksOnChildComponents) const throw();


    /** Returns true if a given point lies within this component or one of its children.

        Never override this method! Use hitTest to create custom hit regions.

        @param x    the x co-ordinate to test, relative to this component's left hand edge.
        @param y    the y co-ordinate to test, relative to this component's top edge.
        @returns    true if the point is within the component's hit-test area, but only if
                    that part of the component isn't clipped by its parent component. Note
                    that this won't take into account any overlapping sibling components
                    which might be in the way - for that, see reallyContains()
        @see hitTest, reallyContains, getComponentAt
    */
    virtual bool contains (int x, int y);

    /** Returns true if a given point lies in this component, taking any overlapping
        siblings into account.

        @param x    the x co-ordinate to test, relative to this component's left hand edge.
        @param y    the y co-ordinate to test, relative to this component's top edge.
        @param returnTrueIfWithinAChild     if the point actually lies within a child of this
                                            component, this determines the value that will
                                            be returned.

        @see contains, getComponentAt
    */
    bool reallyContains (int x, int y,
                         const bool returnTrueIfWithinAChild);

    /** Returns the component at a certain point within this one.

        @param x    the x co-ordinate to test, relative to this component's left hand edge.
        @param y    the y co-ordinate to test, relative to this component's top edge.
        @returns    the component that is at this position - which may be 0, this component,
                    or one of its children. Note that overlapping siblings that might actually
                    be in the way are not taken into account by this method - to account for these,
                    instead call getComponentAt on the top-level parent of this component.
        @see hitTest, contains, reallyContains
    */
    Component* getComponentAt (const int x, const int y);

    //==============================================================================
    /** Marks the whole component as needing to be redrawn.

        Calling this will not do any repainting immediately, but will mark the component
        as 'dirty'. At some point in the near future the operating system will send a paint
        message, which will redraw all the dirty regions of all components.
        There's no guarantee about how soon after calling repaint() the redraw will actually
        happen, and other queued events may be delivered before a redraw is done.

        If the setBufferedToImage() method has been used to cause this component
        to use a buffer, the repaint() call will invalidate the component's buffer.

        To redraw just a subsection of the component rather than the whole thing,
        use the repaint (int, int, int, int) method.

        @see paint
    */
    void repaint() throw();

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
    void repaint (const int x, const int y,
                  const int width, const int height) throw();

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
    void setBufferedToImage (const bool shouldBeBuffered) throw();

    /** Generates a snapshot of part of this component.

        This will return a new Image, the size of the rectangle specified,
        containing a snapshot of the specified area of the component and all
        its children.

        The image may or may not have an alpha-channel, depending on whether the
        image is opaque or not.

        If the clipImageToComponentBounds parameter is true and the area is greater than
        the size of the component, it'll be clipped. If clipImageToComponentBounds is false
        then parts of the component beyond its bounds can be drawn.

        The caller is responsible for deleting the image that is returned.

        @see paintEntireComponent
    */
    Image* createComponentSnapshot (const Rectangle& areaToGrab,
                                    const bool clipImageToComponentBounds = true);

    /** Draws this component and all its subcomponents onto the specified graphics
        context.

        You should very rarely have to use this method, it's simply there in case you need
        to draw a component with a custom graphics context for some reason, e.g. for
        creating a snapshot of the component.

        It calls paint(), paintOverChildren() and recursively calls paintEntireComponent()
        on its children in order to render the entire tree.

        The graphics context may be left in an undefined state after this method returns,
        so you may need to reset it if you're going to use it again.
    */
    void paintEntireComponent (Graphics& context);


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
    void setComponentEffect (ImageEffectFilter* const newEffect);

    /** Returns the current component effect.

        @see setComponentEffect
    */
    ImageEffectFilter* getComponentEffect() const throw()               { return effect_; }

    //==============================================================================
    /** Finds the appropriate look-and-feel to use for this component.

        If the component hasn't had a look-and-feel explicitly set, this will
        return the parent's look-and-feel, or just the default one if there's no
        parent.

        @see setLookAndFeel, lookAndFeelChanged
    */
    LookAndFeel& getLookAndFeel() const throw();

    /** Sets the look and feel to use for this component.

        This will also change the look and feel for any child components that haven't
        had their look set explicitly.

        The object passed in will not be deleted by the component, so it's the caller's
        responsibility to manage it. It may be used at any time until this component
        has been deleted.

        Calling this method will also invoke the sendLookAndFeelChange() method.

        @see getLookAndFeel, lookAndFeelChanged
    */
    void setLookAndFeel (LookAndFeel* const newLookAndFeel);

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

        @see isOpaque, getVisibleArea
    */
    void setOpaque (const bool shouldBeOpaque) throw();

    /** Returns true if no parts of this component are transparent.

        @returns the value that was set by setOpaque, (the default being false)
        @see setOpaque
    */
    bool isOpaque() const throw();

    //==============================================================================
    /** Indicates whether the component should be brought to the front when clicked.

        Setting this flag to true will cause the component to be brought to the front
        when the mouse is clicked somewhere inside it or its child components.

        Note that a top-level desktop window might still be brought to the front by the
        operating system when it's clicked, depending on how the OS works.

        By default this is set to false.

        @see setMouseClickGrabsKeyboardFocus
    */
    void setBroughtToFrontOnMouseClick (const bool shouldBeBroughtToFront) throw();

    /** Indicates whether the component should be brought to the front when clicked-on.

        @see setBroughtToFrontOnMouseClick
    */
    bool isBroughtToFrontOnMouseClick() const throw();

    //==============================================================================
    // Keyboard focus methods

    /** Sets a flag to indicate whether this component needs keyboard focus or not.

        By default components aren't actually interested in gaining the
        focus, but this method can be used to turn this on.

        See the grabKeyboardFocus() method for details about the way a component
        is chosen to receive the focus.

        @see grabKeyboardFocus, getWantsKeyboardFocus
    */
    void setWantsKeyboardFocus (const bool wantsFocus) throw();

    /** Returns true if the component is interested in getting keyboard focus.

        This returns the flag set by setWantsKeyboardFocus(). The default
        setting is false.

        @see setWantsKeyboardFocus
    */
    bool getWantsKeyboardFocus() const throw();

    //==============================================================================
    /** Chooses whether a click on this component automatically grabs the focus.

        By default this is set to true, but you might want a component which can
        be focused, but where you don't want the user to be able to affect it directly
        by clicking.
    */
    void setMouseClickGrabsKeyboardFocus (const bool shouldGrabFocus);

    /** Returns the last value set with setMouseClickGrabsKeyboardFocus().

        See setMouseClickGrabsKeyboardFocus() for more info.
    */
    bool getMouseClickGrabsKeyboardFocus() const throw();

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
    bool hasKeyboardFocus (const bool trueIfChildIsFocused) const throw();

    /** Returns the component that currently has the keyboard focus.

        @returns the focused component, or null if nothing is focused.
    */
    static Component* getCurrentlyFocusedComponent() throw();

    //==============================================================================
    /** Tries to move the keyboard focus to one of this component's siblings.

        This will try to move focus to either the next or previous component. (This
        is the method that is used when shifting focus by pressing the tab key).

        Components for which getWantsKeyboardFocus() returns false are not looked at.

        @param moveToNext   if true, the focus will move forwards; if false, it will
                            move backwards
        @see grabKeyboardFocus, setFocusContainer, setWantsKeyboardFocus
    */
    void moveKeyboardFocusToSibling (const bool moveToNext);

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
    int getExplicitFocusOrder() const throw();

    /** Sets the index used in determining the order in which focusable components
        should be traversed.

        A value of 0 or less is taken to mean that no explicit order is wanted, and
        that traversal should use other factors, like the component's position.

        @see getExplicitFocusOrder, moveKeyboardFocusToSibling
    */
    void setExplicitFocusOrder (const int newFocusOrderIndex) throw();

    /** Indicates whether this component is a parent for components that can have
        their focus traversed.

        This flag is used by the default implementation of the createFocusTraverser()
        method, which uses the flag to find the first parent component (of the currently
        focused one) which wants to be a focus container.

        So using this method to set the flag to 'true' causes this component to
        act as the top level within which focus is passed around.

        @see isFocusContainer, createFocusTraverser, moveKeyboardFocusToSibling
    */
    void setFocusContainer (const bool isFocusContainer) throw();

    /** Returns true if this component has been marked as a focus container.

        See setFocusContainer() for more details.

        @see setFocusContainer, moveKeyboardFocusToSibling, createFocusTraverser
    */
    bool isFocusContainer() const throw();

    //==============================================================================
    /** Returns true if the component (and all its parents) are enabled.

        Components are enabled by default, and can be disabled with setEnabled(). Exactly
        what difference this makes to the component depends on the type. E.g. buttons
        and sliders will choose to draw themselves differently, etc.

        Note that if one of this component's parents is disabled, this will always
        return false, even if this component itself is enabled.

        @see setEnabled, enablementChanged
    */
    bool isEnabled() const throw();

    /** Enables or disables this component.

        Disabling a component will also cause all of its child components to become
        disabled.

        Similarly, enabling a component which is inside a disabled parent
        component won't make any difference until the parent is re-enabled.

        @see isEnabled, enablementChanged
    */
    void setEnabled (const bool shouldBeEnabled);

    /** Callback to indicate that this component has been enabled or disabled.

        This can be triggered by one of the component's parent components
        being enabled or disabled, as well as changes to the component itself.

        The default implementation of this method does nothing; your class may
        wish to repaint itself or something when this happens.

        @see setEnabled, isEnabled
    */
    virtual void enablementChanged();

    //==============================================================================
    /** Changes the mouse cursor shape to use when the mouse is over this component.

        Note that the cursor set by this method can be overridden by the getMouseCursor
        method.

        @see MouseCursor
    */
    void setMouseCursor (const MouseCursor& cursorType) throw();

    /** Returns the mouse cursor shape to use when the mouse is over this component.

        The default implementation will return the cursor that was set by setCursor()
        but can be overridden for more specialised purposes, e.g. returning different
        cursors depending on the mouse position.

        @see MouseCursor
    */
    virtual const MouseCursor getMouseCursor();

    /** Forces the current mouse cursor to be updated.

        If you're overriding the getMouseCursor() method to control which cursor is
        displayed, then this will only be checked each time the user moves the mouse. So
        if you want to force the system to check that the cursor being displayed is
        up-to-date (even if the mouse is just sitting there), call this method.

        This isn't needed if you're only using setMouseCursor().
    */
    void updateMouseCursor() const throw();

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
    /** Called when the mouse moves inside this component.

        If the mouse button isn't pressed and the mouse moves over a component,
        this will be called to let the component react to this.

        A component will always get a mouseEnter callback before a mouseMove.

        @param e    details about the position and status of the mouse event
        @see mouseEnter, mouseExit, mouseDrag, contains
    */
    virtual void mouseMove         (const MouseEvent& e);

    /** Called when the mouse first enters this component.

        If the mouse button isn't pressed and the mouse moves into a component,
        this will be called to let the component react to this.

        When the mouse button is pressed and held down while being moved in
        or out of a component, no mouseEnter or mouseExit callbacks are made - only
        mouseDrag messages are sent to the component that the mouse was originally
        clicked on, until the button is released.

        If you're writing a component that needs to repaint itself when the mouse
        enters and exits, it might be quicker to use the setRepaintsOnMouseActivity()
        method.

        @param e    details about the position and status of the mouse event
        @see mouseExit, mouseDrag, mouseMove, contains
    */
    virtual void mouseEnter        (const MouseEvent& e);

    /** Called when the mouse moves out of this component.

        This will be called when the mouse moves off the edge of this
        component.

        If the mouse button was pressed, and it was then dragged off the
        edge of the component and released, then this callback will happen
        when the button is released, after the mouseUp callback.

        If you're writing a component that needs to repaint itself when the mouse
        enters and exits, it might be quicker to use the setRepaintsOnMouseActivity()
        method.

        @param e    details about the position and status of the mouse event
        @see mouseEnter, mouseDrag, mouseMove, contains
    */
    virtual void mouseExit         (const MouseEvent& e);

    /** Called when a mouse button is pressed while it's over this component.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        Once a button is held down, the mouseDrag method will be called when the
        mouse moves, until the button is released.

        @param e    details about the position and status of the mouse event
        @see mouseUp, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseDown         (const MouseEvent& e);

    /** Called when the mouse is moved while a button is held down.

        When a mouse button is pressed inside a component, that component
        receives mouseDrag callbacks each time the mouse moves, even if the
        mouse strays outside the component's bounds.

        If you want to be able to drag things off the edge of a component
        and have the component scroll when you get to the edges, the
        beginDragAutoRepeat() method might be useful.

        @param e    details about the position and status of the mouse event
        @see mouseDown, mouseUp, mouseMove, contains, beginDragAutoRepeat
    */
    virtual void mouseDrag         (const MouseEvent& e);

    /** Called when a mouse button is released.

        A mouseUp callback is sent to the component in which a button was pressed
        even if the mouse is actually over a different component when the
        button is released.

        The MouseEvent object passed in contains lots of methods for finding out
        which buttons were down just before they were released.

        @param e    details about the position and status of the mouse event
        @see mouseDown, mouseDrag, mouseDoubleClick, contains
    */
    virtual void mouseUp           (const MouseEvent& e);

    /** Called when a mouse button has been double-clicked in this component.

        The MouseEvent object passed in contains lots of methods for finding out
        which button was pressed, as well as which modifier keys (e.g. shift, ctrl)
        were held down at the time.

        For altering the time limit used to detect double-clicks,
        see MouseEvent::setDoubleClickTimeout.

        @param e    details about the position and status of the mouse event
        @see mouseDown, mouseUp, MouseEvent::setDoubleClickTimeout,
             MouseEvent::getDoubleClickTimeout
    */
    virtual void mouseDoubleClick  (const MouseEvent& e);

    /** Called when the mouse-wheel is moved.

        This callback is sent to the component that the mouse is over when the
        wheel is moved.

        If not overridden, the component will forward this message to its parent, so
        that parent components can collect mouse-wheel messages that happen to
        child components which aren't interested in them.

        @param e                details about the position and status of the mouse event
        @param wheelIncrementX   the speed and direction of the horizontal scroll-wheel - a positive
                                 value means the wheel has been pushed to the right, negative means it
                                 was pushed to the left
        @param wheelIncrementY   the speed and direction of the vertical scroll-wheel - a positive
                                 value means the wheel has been pushed upwards, negative means it
                                 was pushed downwards
    */
    virtual void mouseWheelMove    (const MouseEvent& e,
                                    float wheelIncrementX,
                                    float wheelIncrementY);

    //==============================================================================
    /** Ensures that a non-stop stream of mouse-drag events will be sent during the
        next mouse-drag operation.

        This allows you to make sure that mouseDrag() events sent continuously, even
        when the mouse isn't moving. This can be useful for things like auto-scrolling
        components when the mouse is near an edge.

        Call this method during a mouseDown() or mouseDrag() callback, specifying the
        minimum interval between consecutive mouse drag callbacks. The callbacks
        will continue until the mouse is released, and then the interval will be reset,
        so you need to make sure it's called every time you begin a drag event. If it
        is called when the mouse isn't actually being pressed, it will apply to the next
        mouse-drag operation that happens.

        Passing an interval of 0 or less will cancel the auto-repeat.

        @see mouseDrag
    */
    static void beginDragAutoRepeat (const int millisecondIntervalBetweenCallbacks);

    /** Causes automatic repaints when the mouse enters or exits this component.

        If turned on, then when the mouse enters/exits, or when the button is pressed/released
        on the component, it will trigger a repaint.

        This is handy for things like buttons that need to draw themselves differently when
        the mouse moves over them, and it avoids having to override all the different mouse
        callbacks and call repaint().

        @see mouseEnter, mouseExit, mouseDown, mouseUp
    */
    void setRepaintsOnMouseActivity (const bool shouldRepaint) throw();

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
    void addMouseListener (MouseListener* const newListener,
                           const bool wantsEventsForAllNestedChildComponents) throw();

    /** Deregisters a mouse listener.

        @see addMouseListener, MouseListener
    */
    void removeMouseListener (MouseListener* const listenerToRemove) throw();

    //==============================================================================
    /** Adds a listener that wants to hear about keypresses that this component receives.

        The listeners that are registered with a component are called by its keyPressed() or
        keyStateChanged() methods (assuming these haven't been overridden to do something else).

        If you add an object as a key listener, be careful to remove it when the object
        is deleted, or the component will be left with a dangling pointer.

        @see keyPressed, keyStateChanged, removeKeyListener
    */
    void addKeyListener (KeyListener* const newListener) throw();

    /** Removes a previously-registered key listener.

        @see addKeyListener
    */
    void removeKeyListener (KeyListener* const listenerToRemove) throw();

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

        @see keyPressed, KeyPress, getCurrentlyFocusedComponent, addKeyListener
    */
    virtual bool keyStateChanged();

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

    /** Called to indicate that one of this component's children has been focused or unfocused.

        Essentially this means that the return value of a call to hasKeyboardFocus (true) has
        changed. It happens when focus moves from one of this component's children (at any depth)
        to a component that isn't contained in this one, (or vice-versa).

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

        @see isMouseButtonDown. isMouseOverOrDragging, mouseDrag
    */
    bool isMouseOver() const throw();

    /** Returns true if the mouse button is currently held down in this component.

        Note that this is a test to see whether the mouse is being pressed in this
        component, so it'll return false if called on component A when the mouse
        is actually being dragged in component B.

        @see isMouseButtonDownAnywhere, isMouseOver, isMouseOverOrDragging
    */
    bool isMouseButtonDown() const throw();

    /** True if the mouse is over this component, or if it's being dragged in this component.

        This is a handy equivalent to (isMouseOver() || isMouseButtonDown()).

        @see isMouseOver, isMouseButtonDown, isMouseButtonDownAnywhere
    */
    bool isMouseOverOrDragging() const throw();

    /** Returns true if a mouse button is currently down.

        Unlike isMouseButtonDown, this will test the current state of the
        buttons without regard to which component (if any) it has been
        pressed in.

        @see isMouseButtonDown, ModifierKeys
    */
    static bool isMouseButtonDownAnywhere() throw();

    /** Returns the mouse's current position, relative to this component.

        The co-ordinates are relative to the component's top-left corner.
    */
    void getMouseXYRelative (int& x, int& y) const throw();

    /** Returns the component that's currently underneath the mouse.

        @returns the component or 0 if there isn't one.
        @see contains, getComponentAt
    */
    static Component* getComponentUnderMouse() throw();

    /** Allows the mouse to move beyond the edges of the screen.

        Calling this method when the mouse button is currently pressed inside this component
        will remove the cursor from the screen and allow the mouse to (seem to) move beyond
        the edges of the screen.

        This means that the co-ordinates returned to mouseDrag() will be unbounded, and this
        can be used for things like custom slider controls or dragging objects around, where
        movement would be otherwise be limited by the mouse hitting the edges of the screen.

        The unbounded mode is automatically turned off when the mouse button is released, or
        it can be turned off explicitly by calling this method again.

        @param shouldUnboundedMovementBeEnabled     whether to turn this mode on or off
        @param keepCursorVisibleUntilOffscreen      if set to false, the cursor will immediately be
                                                    hidden; if true, it will only be hidden when it
                                                    is moved beyond the edge of the screen
    */
    void enableUnboundedMouseMovement (bool shouldUnboundedMovementBeEnabled,
                                       bool keepCursorVisibleUntilOffscreen = false) throw();

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
    void addComponentListener (ComponentListener* const newListener) throw();

    /** Removes a component listener.

        @see addComponentListener
    */
    void removeComponentListener (ComponentListener* const listenerToRemove) throw();

    //==============================================================================
    /** Called when files are dragged-and-dropped onto this component.

        If the component isn't interested in the files, it should return false, to indicate
        that its parent can be offered the files instead.

        @param filenames    a list of the filenames of the files that were dropped
        @param mouseX       x co-ordinate of the mouse when they were dropped, (relative to this
                            component's top-left)
        @param mouseY       y co-ordinate of the mouse when they were dropped, (relative to this
                            component's top-left)
    */
    virtual bool filesDropped (const StringArray& filenames,
                               int mouseX,
                               int mouseY);

    //==============================================================================
    /** Dispatches a numbered message to this component.

        This is a quick and cheap way of allowing simple asynchronous messages to
        be sent to components. It's also safe, because if the component that you
        send the message to is a null or dangling pointer, this won't cause an error.

        The command ID is later delivered to the component's handleCommandMessage() method by
        the application's message queue.

        @see handleCommandMessage
    */
    void postCommandMessage (const int commandId) throw();

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
             isCurrentlyBlockedByAnotherModalComponent, MessageManager::dispatchNextMessage
    */
    int runModalLoop();

    /** Puts the component into a modal state.

        This makes the component modal, so that messages are blocked from reaching
        any components other than this one and its children, but unlike runModalLoop(),
        this method returns immediately.

        If takeKeyboardFocus is true, the component will use grabKeyboardFocus() to
        get the focus, which is usually what you'll want it to do. If not, it will leave
        the focus unchanged.

        @see exitModalState, runModalLoop
    */
    void enterModalState (const bool takeKeyboardFocus = true);

    /** Ends a component's modal state.

        If this component is currently modal, this will turn of its modalness, and return
        a value to the runModalLoop() method that might have be running its modal loop.

        @see runModalLoop, enterModalState, isCurrentlyModal
    */
    void exitModalState (const int returnValue);

    /** Returns true if this component is the modal one.

        It's possible to have nested modal components, e.g. a pop-up dialog box
        that launches another pop-up, but this will only return true for
        the one at the top of the stack.

        @see getCurrentlyModalComponent
    */
    bool isCurrentlyModal() const throw();

    /** Returns the component that is currently modal.

        @returns the modal component, or null if no components are modal
        @see runModalLoop, isCurrentlyModal
    */
    static Component* JUCE_CALLTYPE getCurrentlyModalComponent() throw();

    /** Checks whether there's a modal component somewhere that's stopping this one
        from receiving messages.

        If there is a modal component, its canModalEventBeSentToComponent() method
        will be called to see if it will still allow this component to receive events.

        @see runModalLoop, getCurrentlyModalComponent
    */
    bool isCurrentlyBlockedByAnotherModalComponent() const throw();

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
    /** Returns one of the component's properties as a string.

        @param keyName                          the name of the property to retrieve
        @param useParentComponentIfNotFound     if this is true and the key isn't present in this component's
                                                properties, then it will check whether the parent component has
                                                the key.
        @param defaultReturnValue               a value to return if the named property doesn't actually exist
    */
    const String getComponentProperty (const String& keyName,
                                       const bool useParentComponentIfNotFound,
                                       const String& defaultReturnValue = String::empty) const throw();

    /** Returns one of the properties as an integer.

        @param keyName                          the name of the property to retrieve
        @param useParentComponentIfNotFound     if this is true and the key isn't present in this component's
                                                properties, then it will check whether the parent component has
                                                the key.
        @param defaultReturnValue               a value to return if the named property doesn't actually exist
    */
    int getComponentPropertyInt (const String& keyName,
                                 const bool useParentComponentIfNotFound,
                                 const int defaultReturnValue = 0) const throw();

    /** Returns one of the properties as an double.

        @param keyName                          the name of the property to retrieve
        @param useParentComponentIfNotFound     if this is true and the key isn't present in this component's
                                                properties, then it will check whether the parent component has
                                                the key.
        @param defaultReturnValue               a value to return if the named property doesn't actually exist
    */
    double getComponentPropertyDouble (const String& keyName,
                                       const bool useParentComponentIfNotFound,
                                       const double defaultReturnValue = 0.0) const throw();

    /** Returns one of the properties as an boolean.

        The result will be true if the string found for this key name can be parsed as a non-zero
        integer.

        @param keyName                          the name of the property to retrieve
        @param useParentComponentIfNotFound     if this is true and the key isn't present in this component's
                                                properties, then it will check whether the parent component has
                                                the key.
        @param defaultReturnValue               a value to return if the named property doesn't actually exist
    */
    bool getComponentPropertyBool (const String& keyName,
                                   const bool useParentComponentIfNotFound,
                                   const bool defaultReturnValue = false) const throw();

    /** Returns one of the properties as an colour.

        @param keyName                          the name of the property to retrieve
        @param useParentComponentIfNotFound     if this is true and the key isn't present in this component's
                                                properties, then it will check whether the parent component has
                                                the key.
        @param defaultReturnValue               a colour to return if the named property doesn't actually exist
    */
    const Colour getComponentPropertyColour (const String& keyName,
                                             const bool useParentComponentIfNotFound,
                                             const Colour& defaultReturnValue = Colours::black) const throw();

    /** Sets a named property as a string.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
        @see removeComponentProperty
    */
    void setComponentProperty (const String& keyName, const String& value) throw();

    /** Sets a named property to an integer.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
        @see removeComponentProperty
    */
    void setComponentProperty (const String& keyName, const int value) throw();

    /** Sets a named property to a double.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
        @see removeComponentProperty
    */
    void setComponentProperty (const String& keyName, const double value) throw();

    /** Sets a named property to a boolean.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
        @see removeComponentProperty
    */
    void setComponentProperty (const String& keyName, const bool value) throw();

    /** Sets a named property to a colour.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param newColour    the new colour to set it to
        @see removeComponentProperty
    */
    void setComponentProperty (const String& keyName, const Colour& newColour) throw();

    /** Deletes a named component property.

        @param keyName      the name of the property to delete. (This mustn't be an empty string)
        @see setComponentProperty, getComponentProperty
    */
    void removeComponentProperty (const String& keyName) throw();

    /** Returns the complete set of properties that have been set for this component.

        If no properties have been set, this will return a null pointer.

        @see getComponentProperty, setComponentProperty
    */
    PropertySet* getComponentProperties() const throw()           { return propertySet_; }

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
    const Colour findColour (const int colourId, const bool inheritFromParent = false) const throw();

    /** Registers a colour to be used for a particular purpose.

        Changing a colour will cause a synchronous callback to the colourChanged()
        method, which your component can override if it needs to do something when
        colours are altered.

        For more details about colour IDs, see the comments for findColour().

        @see findColour, isColourSpecified, colourChanged, LookAndFeel::findColour, LookAndFeel::setColour
    */
    void setColour (const int colourId, const Colour& colour);

    /** If a colour has been set with setColour(), this will remove it.

        This allows you to make a colour revert to its default state.
    */
    void removeColour (const int colourId);

    /** Returns true if the specified colour ID has been explicitly set for this
        component using the setColour() method.
    */
    bool isColourSpecified (const int colourId) const throw();

    /** This looks for any colours that have been specified for this component,
        and copies them to the specified target component.
    */
    void copyAllExplicitColoursTo (Component& target) const throw();

    /** This method is called when a colour is changed by the setColour() method.

        @see setColour, findColour
    */
    virtual void colourChanged();

    //==============================================================================
    /** Returns the underlying native window handle for this component.

        This is platform-dependent and strictly for power-users only!
    */
    void* getWindowHandle() const throw();

    /** When created, each component is given a number to uniquely identify it.

        The number is incremented each time a new component is created, so it's a more
        unique way of identifying a component than using its memory location (which
        may be reused after the component is deleted, of course).
    */
    uint32 getComponentUID() const throw()                { return componentUID; }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    friend class ComponentPeer;
    friend class InternalDragRepeater;

    static Component* currentlyFocusedComponent;
    static Component* componentUnderMouse;

    //==============================================================================
    String componentName_;
    Component* parentComponent_;
    uint32 componentUID;
    Rectangle bounds_;
    unsigned short numDeepMouseListeners;
    Array <Component*> childComponentList_;
    LookAndFeel* lookAndFeel_;
    MouseCursor cursor_;
    ImageEffectFilter* effect_;
    Image* bufferedImage_;
    VoidArray* mouseListeners_;
    VoidArray* keyListeners_;
    VoidArray* componentListeners_;
    PropertySet* propertySet_;

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
        bool draggingFlag               : 1;
        bool mouseOverFlag              : 1;
        bool mouseInsideFlag            : 1;
        bool currentlyModalFlag         : 1;
        bool isDisabledFlag             : 1;
        bool childCompFocusedFlag       : 1;
    };

    union
    {
        uint32 componentFlags_;
        ComponentFlags flags;
    };

    //==============================================================================
    void internalMouseEnter (int x, int y, const int64 time);
    void internalMouseExit  (int x, int y, const int64 time);
    void internalMouseDown  (int x, int y);
    void internalMouseUp    (const int oldModifiers, int x, int y, const int64 time);
    void internalMouseDrag  (int x, int y, const int64 time);
    void internalMouseMove  (int x, int y, const int64 time);
    void internalMouseWheel (const int intAmountX, const int intAmountY, const int64 time);
    void internalBroughtToFront();
    void internalFocusGain (const FocusChangeType cause);
    void internalFocusLoss (const FocusChangeType cause);
    void internalChildFocusChange (FocusChangeType cause);
    void internalModalInputAttempt();
    void internalModifierKeysChanged();
    void internalChildrenChanged();
    void internalHierarchyChanged();
    void internalFilesDropped (const int x, const int y, const StringArray& files);
    void internalUpdateMouseCursor (const bool forcedUpdate) throw();
    void sendMovedResizedMessages (const bool wasMoved, const bool wasResized);
    void repaintParent() throw();
    void sendFakeMouseMove() const;
    void takeKeyboardFocus (const FocusChangeType cause);
    void grabFocusInternal (const FocusChangeType cause, const bool canTryParent = true);
    static void giveAwayFocus();
    void sendEnablementChangeMessage();
    static void* runModalLoopCallback (void*);
    void subtractObscuredRegions (RectangleList& result,
                                  const int deltaX, const int deltaY,
                                  const Rectangle& clipRect,
                                  const Component* const compToAvoid) const throw();
    void clipObscuredRegions (Graphics& g, const Rectangle& clipRect,
                              const int deltaX, const int deltaY) const throw();

    // how much of the component is not off the edges of its parents
    const Rectangle getUnclippedArea() const;
    void sendVisibilityChangeMessage();

    // components aren't allowed to have copy constructors, as this would mess up parent
    // hierarchies. You might need to give your subclasses a private dummy constructor like
    // this one to avoid compiler warnings.
    Component (const Component&);

    const Component& operator= (const Component&);

protected:
    /** @internal */
    virtual void internalRepaint (int x, int y, int w, int h);

    virtual ComponentPeer* createNewPeer (int styleFlags, void* nativeWindowToAttachTo);

    /** Overridden from the MessageListener parent class.

        You can override this if you really need to, but be sure to pass your unwanted messages up
        to this base class implementation, as the Component class needs to send itself messages
        to work properly.
    */
    void handleMessage (const Message&);
};


#endif   // __JUCE_COMPONENT_JUCEHEADER__
