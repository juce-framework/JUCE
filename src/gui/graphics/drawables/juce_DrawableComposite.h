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
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    const ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
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

        ValueTree getChildList() const;
        ValueTree getChildListCreating (UndoManager* undoManager);

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
