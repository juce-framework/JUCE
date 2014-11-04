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

#ifndef JUCE_DRAWABLECOMPOSITE_H_INCLUDED
#define JUCE_DRAWABLECOMPOSITE_H_INCLUDED


//==============================================================================
/**
    A drawable object which acts as a container for a set of other Drawables.

    @see Drawable
*/
class JUCE_API  DrawableComposite  : public Drawable
{
public:
    //==============================================================================
    /** Creates a composite Drawable. */
    DrawableComposite();

    /** Creates a copy of a DrawableComposite. */
    DrawableComposite (const DrawableComposite&);

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
    const RelativeParallelogram& getBoundingBox() const noexcept            { return bounds; }

    /** Changes the bounding box transform to match the content area, so that any sub-items will
        be drawn at their untransformed positions.
    */
    void resetBoundingBoxToContentArea();

    /** Returns the main content rectangle.
        The content area is actually defined by the markers named "left", "right", "top" and
        "bottom", but this method is a shortcut that returns them all at once.
        @see contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    RelativeRectangle getContentArea() const;

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
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    Rectangle<float> getDrawableBounds() const;
    /** @internal */
    void childBoundsChanged (Component*) override;
    /** @internal */
    void childrenChanged() override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    MarkerList* getMarkers (bool xAxis) override;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableComposite's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        ValueTree getChildList() const;
        ValueTree getChildListCreating (UndoManager* undoManager);

        RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager);
        void resetBoundingBoxToContentArea (UndoManager* undoManager);

        RelativeRectangle getContentArea() const;
        void setContentArea (const RelativeRectangle& newArea, UndoManager* undoManager);

        MarkerList::ValueTreeWrapper getMarkerList (bool xAxis) const;
        MarkerList::ValueTreeWrapper getMarkerListCreating (bool xAxis, UndoManager* undoManager);

        static const Identifier topLeft, topRight, bottomLeft;

    private:
        static const Identifier childGroupTag, markerGroupTagX, markerGroupTagY;
    };

private:
    //==============================================================================
    RelativeParallelogram bounds;
    MarkerList markersX, markersY;
    bool updateBoundsReentrant;

    friend class Drawable::Positioner<DrawableComposite>;
    bool registerCoordinates (RelativeCoordinatePositionerBase&);
    void recalculateCoordinates (Expression::Scope*);

    void updateBoundsToFitChildren();

    DrawableComposite& operator= (const DrawableComposite&);
    JUCE_LEAK_DETECTOR (DrawableComposite)
};


#endif   // JUCE_DRAWABLECOMPOSITE_H_INCLUDED
