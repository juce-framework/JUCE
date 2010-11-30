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

#ifndef __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
#define __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__

#include "juce_Drawable.h"


//==============================================================================
/**
    A drawable object which acts as a container for a set of other Drawables.

    @see Drawable
*/
class JUCE_API  DrawableComposite  : public Drawable,
                                     public Expression::EvaluationContext
{
public:
    //==============================================================================
    /** Creates a composite Drawable. */
    DrawableComposite();

    /** Creates a copy of a DrawableComposite. */
    DrawableComposite (const DrawableComposite& other);

    /** Destructor. */
    ~DrawableComposite();

    //==============================================================================
    /** Adds a new sub-drawable to this one.

        This passes in a Drawable pointer for this object to look after. To add a copy
        of a drawable, use the form of this method that takes a Drawable reference instead.

        @param drawable         the object to add - this will be deleted automatically
                                when no longer needed, so the caller mustn't keep any
                                pointers to it.
        @param index            where to insert it in the list of drawables. 0 is the back,
                                -1 is the front, or any value from 0 and getNumDrawables()
                                can be used
        @see removeDrawable
    */
    void insertDrawable (Drawable* drawable, int index = -1);

    /** Adds a new sub-drawable to this one.

        This takes a copy of a Drawable and adds it to this object. To pass in a Drawable
        for this object to look after, use the form of this method that takes a Drawable
        pointer instead.

        @param drawable         the object to add - an internal copy will be made of this object
        @param index            where to insert it in the list of drawables. 0 is the back,
                                -1 is the front, or any value from 0 and getNumDrawables()
                                can be used
        @see removeDrawable
    */
    void insertDrawable (const Drawable& drawable, int index = -1);

    /** Deletes one of the Drawable objects.

        @param index    the index of the drawable to delete, between 0
                        and (getNumDrawables() - 1).
        @param deleteDrawable   if this is true, the drawable that is removed will also
                        be deleted. If false, it'll just be removed.
        @see insertDrawable, getNumDrawables
    */
    void removeDrawable (int index, bool deleteDrawable = true);

    /** Returns the number of drawables contained inside this one.

        @see getDrawable
    */
    int getNumDrawables() const throw();

    /** Returns one of the drawables that are contained in this one.

        Each drawable also has a transform associated with it - you can use getDrawableTransform()
        to find it.

        The pointer returned is managed by this object and will be deleted when no longer
        needed, so be careful what you do with it.

        @see getNumDrawables
    */
    Drawable* getDrawable (int index) const;

    /** Looks for a child drawable with the specified name. */
    Drawable* getDrawableWithName (const String& name) const throw();

    /** Brings one of the Drawables to the front.

        @param index    the index of the drawable to move, between 0
                        and (getNumDrawables() - 1).
        @see insertDrawable, getNumDrawables
    */
    void bringToFront (int index);

    //==============================================================================
    /** Sets the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setContentArea
    */
    void setBoundingBox (const RelativeParallelogram& newBoundingBox);

    /** Returns the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setBoundingBox
    */
    const RelativeParallelogram& getBoundingBox() const throw()             { return bounds; }

    /** Changes the bounding box transform to match the content area, so that any sub-items will
        be drawn at their untransformed positions.
    */
    void resetBoundingBoxToContentArea();

    /** Returns the main content rectangle.
        The content area is actually defined by the markers named "left", "right", "top" and
        "bottom", but this method is a shortcut that returns them all at once.
        @see contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    const RelativeRectangle getContentArea() const;

    /** Changes the main content area.
        The content area is actually defined by the markers named "left", "right", "top" and
        "bottom", but this method is a shortcut that sets them all at once.
        @see setBoundingBox, contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    void setContentArea (const RelativeRectangle& newArea);

    /** Resets the content area and the bounding transform to fit around the area occupied
        by the child components (ignoring any markers).
    */
    void resetContentAreaAndBoundingBoxToFitChildren();

    //==============================================================================
    /** Represents a named marker position.
        @see DrawableComposite::getMarker
    */
    struct Marker
    {
        Marker (const Marker&);
        Marker (const String& name, const RelativeCoordinate& position);
        bool operator!= (const Marker&) const throw();

        String name;
        RelativeCoordinate position;
    };

    int getNumMarkers (bool xAxis) const throw();
    const Marker* getMarker (bool xAxis, int index) const throw();
    void setMarker (const String& name, bool xAxis, const RelativeCoordinate& position);
    void removeMarker (bool xAxis, int index);

    /** The name of the marker that defines the left edge of the content area. */
    static const char* const contentLeftMarkerName;
    /** The name of the marker that defines the right edge of the content area. */
    static const char* const contentRightMarkerName;
    /** The name of the marker that defines the top edge of the content area. */
    static const char* const contentTopMarkerName;
    /** The name of the marker that defines the bottom edge of the content area. */
    static const char* const contentBottomMarkerName;

    //==============================================================================
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }
    /** @internal */
    const Expression getSymbolValue (const String& symbol, const String& member) const;
    /** @internal */
    const Rectangle<float> getDrawableBounds() const;
    /** @internal */
    void markerHasMoved();
    /** @internal */
    void childBoundsChanged (Component*);
    /** @internal */
    void childrenChanged();
    /** @internal */
    void parentHierarchyChanged();

    //==============================================================================
    /** Internally-used class for wrapping a DrawableComposite's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        int getNumDrawables() const;
        ValueTree getDrawableState (int index) const;
        ValueTree getDrawableWithId (const String& objectId, bool recursive) const;
        int indexOfDrawable (const ValueTree& item) const;
        void addDrawable (const ValueTree& newDrawableState, int index, UndoManager* undoManager);
        void moveDrawableOrder (int currentIndex, int newIndex, UndoManager* undoManager);
        void removeDrawable (const ValueTree& child, UndoManager* undoManager);

        const RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager);
        void resetBoundingBoxToContentArea (UndoManager* undoManager);

        const RelativeRectangle getContentArea() const;
        void setContentArea (const RelativeRectangle& newArea, UndoManager* undoManager);

        int getNumMarkers (bool xAxis) const;
        const ValueTree getMarkerState (bool xAxis, int index) const;
        const ValueTree getMarkerState (bool xAxis, const String& name) const;
        bool containsMarker (bool xAxis, const ValueTree& state) const;
        const Marker getMarker (bool xAxis, const ValueTree& state) const;
        void setMarker (bool xAxis, const Marker& marker, UndoManager* undoManager);
        void removeMarker (bool xAxis, const ValueTree& state, UndoManager* undoManager);

        static const Identifier nameProperty, posProperty, topLeft, topRight, bottomLeft;

    private:
        static const Identifier childGroupTag, markerGroupTagX, markerGroupTagY, markerTag;

        ValueTree getChildList() const;
        ValueTree getChildListCreating (UndoManager* undoManager);
        ValueTree getMarkerList (bool xAxis) const;
        ValueTree getMarkerListCreating (bool xAxis, UndoManager* undoManager);
    };

private:
    //==============================================================================
    RelativeParallelogram bounds;
    OwnedArray <Marker> markersX, markersY;
    bool updateBoundsReentrant;

    void refreshTransformFromBounds();
    void updateBoundsToFitChildren();

    DrawableComposite& operator= (const DrawableComposite&);
    JUCE_LEAK_DETECTOR (DrawableComposite);
};


#endif   // __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
