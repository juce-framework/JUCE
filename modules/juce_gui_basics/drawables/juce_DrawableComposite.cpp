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

DrawableComposite::DrawableComposite()
    : bounds (Point<float>(), Point<float> (100.0f, 0.0f), Point<float> (0.0f, 100.0f)),
      updateBoundsReentrant (false)
{
    setContentArea (RelativeRectangle (RelativeCoordinate (0.0),
                                       RelativeCoordinate (100.0),
                                       RelativeCoordinate (0.0),
                                       RelativeCoordinate (100.0)));
}

DrawableComposite::DrawableComposite (const DrawableComposite& other)
    : Drawable (other),
      bounds (other.bounds),
      markersX (other.markersX),
      markersY (other.markersY),
      updateBoundsReentrant (false)
{
    for (int i = 0; i < other.getNumChildComponents(); ++i)
        if (const Drawable* const d = dynamic_cast <const Drawable*> (other.getChildComponent(i)))
            addAndMakeVisible (d->createCopy());
}

DrawableComposite::~DrawableComposite()
{
    deleteAllChildren();
}

Drawable* DrawableComposite::createCopy() const
{
    return new DrawableComposite (*this);
}

//==============================================================================
Rectangle<float> DrawableComposite::getDrawableBounds() const
{
    Rectangle<float> r;

    for (int i = getNumChildComponents(); --i >= 0;)
        if (const Drawable* const d = dynamic_cast <const Drawable*> (getChildComponent(i)))
            r = r.getUnion (d->isTransformed() ? d->getDrawableBounds().transformedBy (d->getTransform())
                                               : d->getDrawableBounds());

    return r;
}

MarkerList* DrawableComposite::getMarkers (bool xAxis)
{
    return xAxis ? &markersX : &markersY;
}

RelativeRectangle DrawableComposite::getContentArea() const
{
    jassert (markersX.getNumMarkers() >= 2 && markersX.getMarker (0)->name == contentLeftMarkerName && markersX.getMarker (1)->name == contentRightMarkerName);
    jassert (markersY.getNumMarkers() >= 2 && markersY.getMarker (0)->name == contentTopMarkerName && markersY.getMarker (1)->name == contentBottomMarkerName);

    return RelativeRectangle (markersX.getMarker(0)->position, markersX.getMarker(1)->position,
                              markersY.getMarker(0)->position, markersY.getMarker(1)->position);
}

void DrawableComposite::setContentArea (const RelativeRectangle& newArea)
{
    markersX.setMarker (contentLeftMarkerName, newArea.left);
    markersX.setMarker (contentRightMarkerName, newArea.right);
    markersY.setMarker (contentTopMarkerName, newArea.top);
    markersY.setMarker (contentBottomMarkerName, newArea.bottom);
}

void DrawableComposite::setBoundingBox (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        if (bounds.isDynamic())
        {
            Drawable::Positioner<DrawableComposite>* const p = new Drawable::Positioner<DrawableComposite> (*this);
            setPositioner (p);
            p->apply();
        }
        else
        {
            setPositioner (nullptr);
            recalculateCoordinates (nullptr);
        }
    }
}

void DrawableComposite::resetBoundingBoxToContentArea()
{
    const RelativeRectangle content (getContentArea());

    setBoundingBox (RelativeParallelogram (RelativePoint (content.left, content.top),
                                           RelativePoint (content.right, content.top),
                                           RelativePoint (content.left, content.bottom)));
}

void DrawableComposite::resetContentAreaAndBoundingBoxToFitChildren()
{
    const Rectangle<float> activeArea (getDrawableBounds());

    setContentArea (RelativeRectangle (RelativeCoordinate (activeArea.getX()),
                                       RelativeCoordinate (activeArea.getRight()),
                                       RelativeCoordinate (activeArea.getY()),
                                       RelativeCoordinate (activeArea.getBottom())));
    resetBoundingBoxToContentArea();
}

bool DrawableComposite::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    return pos.addPoint (bounds.bottomLeft) && ok;
}

void DrawableComposite::recalculateCoordinates (Expression::Scope* scope)
{
    Point<float> resolved[3];
    bounds.resolveThreePoints (resolved, scope);

    const Rectangle<float> content (getContentArea().resolve (scope));

    AffineTransform t (AffineTransform::fromTargetPoints (content.getX(),     content.getY(),      resolved[0].x, resolved[0].y,
                                                          content.getRight(), content.getY(),      resolved[1].x, resolved[1].y,
                                                          content.getX(),     content.getBottom(), resolved[2].x, resolved[2].y));

    if (t.isSingularity())
        t = AffineTransform::identity;

    setTransform (t);
}

void DrawableComposite::parentHierarchyChanged()
{
    DrawableComposite* parent = getParent();
    if (parent != nullptr)
        originRelativeToComponent = parent->originRelativeToComponent - getPosition();
}

void DrawableComposite::childBoundsChanged (Component*)
{
    updateBoundsToFitChildren();
}

void DrawableComposite::childrenChanged()
{
    updateBoundsToFitChildren();
}

void DrawableComposite::updateBoundsToFitChildren()
{
    if (! updateBoundsReentrant)
    {
        const ScopedValueSetter<bool> setter (updateBoundsReentrant, true, false);

        Rectangle<int> childArea;

        for (int i = getNumChildComponents(); --i >= 0;)
            childArea = childArea.getUnion (getChildComponent(i)->getBoundsInParent());

        const Point<int> delta (childArea.getPosition());
        childArea += getPosition();

        if (childArea != getBounds())
        {
            if (! delta.isOrigin())
            {
                originRelativeToComponent -= delta;

                for (int i = getNumChildComponents(); --i >= 0;)
                    if (Component* const c = getChildComponent(i))
                        c->setBounds (c->getBounds() - delta);
            }

            setBounds (childArea);
        }
    }
}

//==============================================================================
const char* const DrawableComposite::contentLeftMarkerName = "left";
const char* const DrawableComposite::contentRightMarkerName = "right";
const char* const DrawableComposite::contentTopMarkerName = "top";
const char* const DrawableComposite::contentBottomMarkerName = "bottom";

//==============================================================================
const Identifier DrawableComposite::valueTreeType ("Group");

const Identifier DrawableComposite::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableComposite::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableComposite::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableComposite::ValueTreeWrapper::childGroupTag ("Drawables");
const Identifier DrawableComposite::ValueTreeWrapper::markerGroupTagX ("MarkersX");
const Identifier DrawableComposite::ValueTreeWrapper::markerGroupTagY ("MarkersY");

//==============================================================================
DrawableComposite::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

ValueTree DrawableComposite::ValueTreeWrapper::getChildList() const
{
    return state.getChildWithName (childGroupTag);
}

ValueTree DrawableComposite::ValueTreeWrapper::getChildListCreating (UndoManager* undoManager)
{
    return state.getOrCreateChildWithName (childGroupTag, undoManager);
}

RelativeParallelogram DrawableComposite::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state.getProperty (topLeft, "0, 0"),
                                  state.getProperty (topRight, "100, 0"),
                                  state.getProperty (bottomLeft, "0, 100"));
}

void DrawableComposite::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

void DrawableComposite::ValueTreeWrapper::resetBoundingBoxToContentArea (UndoManager* undoManager)
{
    const RelativeRectangle content (getContentArea());

    setBoundingBox (RelativeParallelogram (RelativePoint (content.left, content.top),
                                           RelativePoint (content.right, content.top),
                                           RelativePoint (content.left, content.bottom)), undoManager);
}

RelativeRectangle DrawableComposite::ValueTreeWrapper::getContentArea() const
{
    MarkerList::ValueTreeWrapper marksX (getMarkerList (true));
    MarkerList::ValueTreeWrapper marksY (getMarkerList (false));

    return RelativeRectangle (marksX.getMarker (marksX.getMarkerState (0)).position,
                              marksX.getMarker (marksX.getMarkerState (1)).position,
                              marksY.getMarker (marksY.getMarkerState (0)).position,
                              marksY.getMarker (marksY.getMarkerState (1)).position);
}

void DrawableComposite::ValueTreeWrapper::setContentArea (const RelativeRectangle& newArea, UndoManager* undoManager)
{
    MarkerList::ValueTreeWrapper marksX (getMarkerListCreating (true, nullptr));
    MarkerList::ValueTreeWrapper marksY (getMarkerListCreating (false, nullptr));

    marksX.setMarker (MarkerList::Marker (contentLeftMarkerName, newArea.left), undoManager);
    marksX.setMarker (MarkerList::Marker (contentRightMarkerName, newArea.right), undoManager);
    marksY.setMarker (MarkerList::Marker (contentTopMarkerName, newArea.top), undoManager);
    marksY.setMarker (MarkerList::Marker (contentBottomMarkerName, newArea.bottom), undoManager);
}

MarkerList::ValueTreeWrapper DrawableComposite::ValueTreeWrapper::getMarkerList (bool xAxis) const
{
    return state.getChildWithName (xAxis ? markerGroupTagX : markerGroupTagY);
}

MarkerList::ValueTreeWrapper DrawableComposite::ValueTreeWrapper::getMarkerListCreating (bool xAxis, UndoManager* undoManager)
{
    return state.getOrCreateChildWithName (xAxis ? markerGroupTagX : markerGroupTagY, undoManager);
}

//==============================================================================
void DrawableComposite::refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder)
{
    const ValueTreeWrapper wrapper (tree);
    setComponentID (wrapper.getID());

    wrapper.getMarkerList (true).applyTo (markersX);
    wrapper.getMarkerList (false).applyTo (markersY);

    setBoundingBox (wrapper.getBoundingBox());

    builder.updateChildComponents (*this, wrapper.getChildList());
}

ValueTree DrawableComposite::createValueTree (ComponentBuilder::ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    v.setBoundingBox (bounds, nullptr);

    ValueTree childList (v.getChildListCreating (nullptr));

    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        const Drawable* const d = dynamic_cast <const Drawable*> (getChildComponent(i));
        jassert (d != nullptr); // You can't save a mix of Drawables and normal components!

        childList.addChild (d->createValueTree (imageProvider), -1, nullptr);
    }

    v.getMarkerListCreating (true, nullptr).readFrom (markersX, nullptr);
    v.getMarkerListCreating (false, nullptr).readFrom (markersY, nullptr);

    return tree;
}
