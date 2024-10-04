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
    A drawable object which acts as a container for a set of other Drawables.

    Note that although this is a Component, it takes ownership of its child components
    and will delete them, so that you can use it as a self-contained graphic object.
    The intention is that you should not add your own components to it, only add other
    Drawable objects.

    @see Drawable

    @tags{GUI}
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
    ~DrawableComposite() override;

    //==============================================================================
    /** Sets the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setContentArea
    */
    void setBoundingBox (Parallelogram<float> newBoundingBox);

    /** Sets the rectangle that defines the target position of the content rectangle when the drawable is rendered.
        @see setContentArea
    */
    void setBoundingBox (Rectangle<float> newBoundingBox);

    /** Returns the parallelogram that defines the target position of the content rectangle when the drawable is rendered.
        @see setBoundingBox
    */
    Parallelogram<float> getBoundingBox() const noexcept            { return bounds; }

    /** Changes the bounding box transform to match the content area, so that any sub-items will
        be drawn at their untransformed positions.
    */
    void resetBoundingBoxToContentArea();

    /** Returns the main content rectangle.
        @see contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    Rectangle<float> getContentArea() const noexcept                { return contentArea; }

    /** Changes the main content area.
        @see setBoundingBox, contentLeftMarkerName, contentRightMarkerName, contentTopMarkerName, contentBottomMarkerName
    */
    void setContentArea (Rectangle<float> newArea);

    /** Resets the content area and the bounding transform to fit around the area occupied
        by the child components.
    */
    void resetContentAreaAndBoundingBoxToFitChildren();

    //==============================================================================
    /** @internal */
    std::unique_ptr<Drawable> createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    void childBoundsChanged (Component*) override;
    /** @internal */
    void childrenChanged() override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    Path getOutlineAsPath() const override;

private:
    //==============================================================================
    Parallelogram<float> bounds;
    Rectangle<float> contentArea;
    bool updateBoundsReentrant = false;

    void updateBoundsToFitChildren();

    DrawableComposite& operator= (const DrawableComposite&);
    JUCE_LEAK_DETECTOR (DrawableComposite)
};

} // namespace juce
