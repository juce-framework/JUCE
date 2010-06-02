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
                                     public RelativeCoordinate::NamedCoordinateFinder
{
public:
    //==============================================================================
    /** Creates a composite Drawable. */
    DrawableComposite();

    /** Creates a copy of a DrawableComposite. */
    DrawableComposite (const DrawableComposite& other);

    /** Destructor. */
    virtual ~DrawableComposite();

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
    int getNumDrawables() const throw()                                         { return drawables.size(); }

    /** Returns one of the drawables that are contained in this one.

        Each drawable also has a transform associated with it - you can use getDrawableTransform()
        to find it.

        The pointer returned is managed by this object and will be deleted when no longer
        needed, so be careful what you do with it.

        @see getNumDrawables
    */
    Drawable* getDrawable (int index) const throw()                             { return drawables [index]; }

    /** Looks for a child drawable with the specified name. */
    Drawable* getDrawableWithName (const String& name) const throw();

    /** Brings one of the Drawables to the front.

        @param index    the index of the drawable to move, between 0
                        and (getNumDrawables() - 1).
        @see insertDrawable, getNumDrawables
    */
    void bringToFront (int index);

    /** Sets the transform to be applied to this drawable, by defining the positions
        where three anchor points should end up in the target rendering space.

        @param targetPositionForOrigin  the position that the local coordinate (0, 0) should be
                                        mapped onto when rendering this object.
        @param targetPositionForX1Y0    the position that the local coordinate (1, 0) should be
                                        mapped onto when rendering this object.
        @param targetPositionForX0Y1    the position that the local coordinate (0, 1) should be
                                        mapped onto when rendering this object.
    */
    void setTransform (const RelativePoint& targetPositionForOrigin,
                       const RelativePoint& targetPositionForX1Y0,
                       const RelativePoint& targetPositionForX0Y1);

    /** Returns the position to which the local coordinate (0, 0) should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForOrigin() const throw()          { return controlPoints[0]; }

    /** Returns the position to which the local coordinate (1, 0) should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForX1Y0() const throw()            { return controlPoints[1]; }

    /** Returns the position to which the local coordinate (0, 1) should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForX0Y1() const throw()            { return controlPoints[2]; }

    //==============================================================================
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

    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    const Rectangle<float> getBounds() const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    void invalidatePoints();
    /** @internal */
    const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }
    /** @internal */
    const RelativeCoordinate findNamedCoordinate (const String& objectName, const String& edge) const;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableComposite's state into a ValueTree. */
    class ValueTreeWrapper   : public ValueTreeWrapperBase
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

        const RelativePoint getTargetPositionForOrigin() const;
        void setTargetPositionForOrigin (const RelativePoint& newPoint, UndoManager* undoManager);

        const RelativePoint getTargetPositionForX1Y0() const;
        void setTargetPositionForX1Y0 (const RelativePoint& newPoint, UndoManager* undoManager);

        const RelativePoint getTargetPositionForX0Y1() const;
        void setTargetPositionForX0Y1 (const RelativePoint& newPoint, UndoManager* undoManager);

        int getNumMarkers (bool xAxis) const;
        const ValueTree getMarkerState (bool xAxis, int index) const;
        const ValueTree getMarkerState (bool xAxis, const String& name) const;
        bool containsMarker (bool xAxis, const ValueTree& state) const;
        const Marker getMarker (bool xAxis, const ValueTree& state) const;
        void setMarker (bool xAxis, const Marker& marker, UndoManager* undoManager);
        void removeMarker (bool xAxis, const ValueTree& state, UndoManager* undoManager);

        static const Identifier nameProperty, posProperty;

    private:
        static const Identifier topLeft, topRight, bottomLeft, childGroupTag, markerGroupTagX,
                                markerGroupTagY, markerTag;

        ValueTree getChildList() const;
        ValueTree getChildListCreating (UndoManager* undoManager);
        ValueTree getMarkerList (bool xAxis) const;
        ValueTree getMarkerListCreating (bool xAxis, UndoManager* undoManager);
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <Drawable> drawables;
    RelativePoint controlPoints[3];
    OwnedArray <Marker> markersX, markersY;

    const Rectangle<float> getUntransformedBounds() const;
    const AffineTransform calculateTransform() const;

    DrawableComposite& operator= (const DrawableComposite&);
};


#endif   // __JUCE_DRAWABLECOMPOSITE_JUCEHEADER__
