/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A drawable object which acts as a container for a set of other Drawables.

    Note that although this is a Component, it takes ownership of its child components
    and will delete them, so that you can use it as a self-contained graphic object.
    The intention is that you should not add your own components to it, only add other
    Drawable objects.

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
    Drawable* createCopy() const override;
    /** @internal */
    void refreshFromValueTree (const ValueTree&, ComponentBuilder&);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const override;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    void childBoundsChanged (Component*) override;
    /** @internal */
    void childrenChanged() override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    MarkerList* getMarkers (bool xAxis) override;
    /** @internal */
    Path getOutlineAsPath() const override;

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
    bool updateBoundsReentrant = false;

    friend class Drawable::Positioner<DrawableComposite>;
    bool registerCoordinates (RelativeCoordinatePositionerBase&);
    void recalculateCoordinates (Expression::Scope*);

    void updateBoundsToFitChildren();

    DrawableComposite& operator= (const DrawableComposite&);
    JUCE_LEAK_DETECTOR (DrawableComposite)
};

} // namespace juce
