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

DrawablePath::DrawablePath()
{
}

DrawablePath::DrawablePath (const DrawablePath& other)
    : DrawableShape (other)
{
    if (other.relativePath != nullptr)
        setPath (*other.relativePath);
    else
        setPath (other.path);
}

DrawablePath::~DrawablePath()
{
}

Drawable* DrawablePath::createCopy() const
{
    return new DrawablePath (*this);
}

//==============================================================================
void DrawablePath::setPath (const Path& newPath)
{
    path = newPath;
    pathChanged();
}

const Path& DrawablePath::getPath() const
{
    return path;
}

const Path& DrawablePath::getStrokePath() const
{
    return strokePath;
}

void DrawablePath::applyRelativePath (const RelativePointPath& newRelativePath, Expression::Scope* scope)
{
    Path newPath;
    newRelativePath.createPath (newPath, scope);

    if (path != newPath)
    {
        path.swapWithPath (newPath);
        pathChanged();
    }
}

//==============================================================================
class DrawablePath::RelativePositioner  : public RelativeCoordinatePositionerBase
{
public:
    RelativePositioner (DrawablePath& comp)
        : RelativeCoordinatePositionerBase (comp),
          owner (comp)
    {
    }

    bool registerCoordinates()
    {
        bool ok = true;

        jassert (owner.relativePath != nullptr);
        const RelativePointPath& path = *owner.relativePath;

        for (int i = 0; i < path.elements.size(); ++i)
        {
            RelativePointPath::ElementBase* const e = path.elements.getUnchecked(i);

            int numPoints;
            RelativePoint* const points = e->getControlPoints (numPoints);

            for (int j = numPoints; --j >= 0;)
                ok = addPoint (points[j]) && ok;
        }

        return ok;
    }

    void applyToComponentBounds()
    {
        jassert (owner.relativePath != nullptr);

        ComponentScope scope (getComponent());
        owner.applyRelativePath (*owner.relativePath, &scope);
    }

    void applyNewBounds (const Rectangle<int>&)
    {
        jassertfalse; // drawables can't be resized directly!
    }

private:
    DrawablePath& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RelativePositioner)
};

void DrawablePath::setPath (const RelativePointPath& newRelativePath)
{
    if (newRelativePath.containsAnyDynamicPoints())
    {
        if (relativePath == nullptr || newRelativePath != *relativePath)
        {
            relativePath = new RelativePointPath (newRelativePath);

            RelativePositioner* const p = new RelativePositioner (*this);
            setPositioner (p);
            p->apply();
        }
    }
    else
    {
        relativePath = nullptr;
        applyRelativePath (newRelativePath, nullptr);
    }
}

//==============================================================================
const Identifier DrawablePath::valueTreeType ("Path");

const Identifier DrawablePath::ValueTreeWrapper::nonZeroWinding ("nonZeroWinding");
const Identifier DrawablePath::ValueTreeWrapper::point1 ("p1");
const Identifier DrawablePath::ValueTreeWrapper::point2 ("p2");
const Identifier DrawablePath::ValueTreeWrapper::point3 ("p3");

//==============================================================================
DrawablePath::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : FillAndStrokeState (state_)
{
    jassert (state.hasType (valueTreeType));
}

ValueTree DrawablePath::ValueTreeWrapper::getPathState()
{
    return state.getOrCreateChildWithName (path, nullptr);
}

bool DrawablePath::ValueTreeWrapper::usesNonZeroWinding() const
{
    return state [nonZeroWinding];
}

void DrawablePath::ValueTreeWrapper::setUsesNonZeroWinding (bool b, UndoManager* undoManager)
{
    state.setProperty (nonZeroWinding, b, undoManager);
}

void DrawablePath::ValueTreeWrapper::readFrom (const RelativePointPath& p, UndoManager* undoManager)
{
    setUsesNonZeroWinding (p.usesNonZeroWinding, undoManager);

    ValueTree pathTree (getPathState());
    pathTree.removeAllChildren (undoManager);

    for (int i = 0; i < p.elements.size(); ++i)
        pathTree.addChild (p.elements.getUnchecked(i)->createTree(), -1, undoManager);
}

void DrawablePath::ValueTreeWrapper::writeTo (RelativePointPath& p) const
{
    p.usesNonZeroWinding = usesNonZeroWinding();
    RelativePoint points[3];

    const ValueTree pathTree (state.getChildWithName (path));
    const int num = pathTree.getNumChildren();
    for (int i = 0; i < num; ++i)
    {
        const Element e (pathTree.getChild(i));

        const int numCps = e.getNumControlPoints();
        for (int j = 0; j < numCps; ++j)
            points[j] = e.getControlPoint (j);

        RelativePointPath::ElementBase* newElement = nullptr;
        const Identifier t (e.getType());

        if      (t == Element::startSubPathElement)  newElement = new RelativePointPath::StartSubPath (points[0]);
        else if (t == Element::closeSubPathElement)  newElement = new RelativePointPath::CloseSubPath();
        else if (t == Element::lineToElement)        newElement = new RelativePointPath::LineTo (points[0]);
        else if (t == Element::quadraticToElement)   newElement = new RelativePointPath::QuadraticTo (points[0], points[1]);
        else if (t == Element::cubicToElement)       newElement = new RelativePointPath::CubicTo (points[0], points[1], points[2]);
        else                                         jassertfalse;

        p.addElement (newElement);
    }
}

//==============================================================================
const Identifier DrawablePath::ValueTreeWrapper::Element::mode ("mode");
const Identifier DrawablePath::ValueTreeWrapper::Element::startSubPathElement ("Move");
const Identifier DrawablePath::ValueTreeWrapper::Element::closeSubPathElement ("Close");
const Identifier DrawablePath::ValueTreeWrapper::Element::lineToElement ("Line");
const Identifier DrawablePath::ValueTreeWrapper::Element::quadraticToElement ("Quad");
const Identifier DrawablePath::ValueTreeWrapper::Element::cubicToElement ("Cubic");

const char* DrawablePath::ValueTreeWrapper::Element::cornerMode = "corner";
const char* DrawablePath::ValueTreeWrapper::Element::roundedMode = "round";
const char* DrawablePath::ValueTreeWrapper::Element::symmetricMode = "symm";

DrawablePath::ValueTreeWrapper::Element::Element (const ValueTree& state_)
    : state (state_)
{
}

DrawablePath::ValueTreeWrapper::Element::~Element()
{
}

DrawablePath::ValueTreeWrapper DrawablePath::ValueTreeWrapper::Element::getParent() const
{
    return ValueTreeWrapper (state.getParent().getParent());
}

DrawablePath::ValueTreeWrapper::Element DrawablePath::ValueTreeWrapper::Element::getPreviousElement() const
{
    return Element (state.getSibling (-1));
}

int DrawablePath::ValueTreeWrapper::Element::getNumControlPoints() const noexcept
{
    const Identifier i (state.getType());
    if (i == startSubPathElement || i == lineToElement) return 1;
    if (i == quadraticToElement) return 2;
    if (i == cubicToElement) return 3;
    return 0;
}

RelativePoint DrawablePath::ValueTreeWrapper::Element::getControlPoint (const int index) const
{
    jassert (index >= 0 && index < getNumControlPoints());
    return RelativePoint (state [index == 0 ? point1 : (index == 1 ? point2 : point3)].toString());
}

Value DrawablePath::ValueTreeWrapper::Element::getControlPointValue (int index, UndoManager* undoManager)
{
    jassert (index >= 0 && index < getNumControlPoints());
    return state.getPropertyAsValue (index == 0 ? point1 : (index == 1 ? point2 : point3), undoManager);
}

void DrawablePath::ValueTreeWrapper::Element::setControlPoint (const int index, const RelativePoint& point, UndoManager* undoManager)
{
    jassert (index >= 0 && index < getNumControlPoints());
    state.setProperty (index == 0 ? point1 : (index == 1 ? point2 : point3), point.toString(), undoManager);
}

RelativePoint DrawablePath::ValueTreeWrapper::Element::getStartPoint() const
{
    const Identifier i (state.getType());

    if (i == startSubPathElement)
        return getControlPoint (0);

    jassert (i == lineToElement || i == quadraticToElement || i == cubicToElement || i == closeSubPathElement);

    return getPreviousElement().getEndPoint();
}

RelativePoint DrawablePath::ValueTreeWrapper::Element::getEndPoint() const
{
    const Identifier i (state.getType());
    if (i == startSubPathElement || i == lineToElement)  return getControlPoint (0);
    if (i == quadraticToElement)                         return getControlPoint (1);
    if (i == cubicToElement)                             return getControlPoint (2);

    jassert (i == closeSubPathElement);
    return RelativePoint();
}

float DrawablePath::ValueTreeWrapper::Element::getLength (Expression::Scope* scope) const
{
    const Identifier i (state.getType());

    if (i == lineToElement || i == closeSubPathElement)
        return getEndPoint().resolve (scope).getDistanceFrom (getStartPoint().resolve (scope));

    if (i == cubicToElement)
    {
        Path p;
        p.startNewSubPath (getStartPoint().resolve (scope));
        p.cubicTo (getControlPoint (0).resolve (scope), getControlPoint (1).resolve (scope), getControlPoint (2).resolve (scope));
        return p.getLength();
    }

    if (i == quadraticToElement)
    {
        Path p;
        p.startNewSubPath (getStartPoint().resolve (scope));
        p.quadraticTo (getControlPoint (0).resolve (scope), getControlPoint (1).resolve (scope));
        return p.getLength();
    }

    jassert (i == startSubPathElement);
    return 0;
}

String DrawablePath::ValueTreeWrapper::Element::getModeOfEndPoint() const
{
    return state [mode].toString();
}

void DrawablePath::ValueTreeWrapper::Element::setModeOfEndPoint (const String& newMode, UndoManager* undoManager)
{
    if (state.hasType (cubicToElement))
        state.setProperty (mode, newMode, undoManager);
}

void DrawablePath::ValueTreeWrapper::Element::convertToLine (UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i == quadraticToElement || i == cubicToElement)
    {
        ValueTree newState (lineToElement);
        Element e (newState);
        e.setControlPoint (0, getEndPoint(), undoManager);
        state = newState;
    }
}

void DrawablePath::ValueTreeWrapper::Element::convertToCubic (Expression::Scope* scope, UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i == lineToElement || i == quadraticToElement)
    {
        ValueTree newState (cubicToElement);
        Element e (newState);

        const RelativePoint start (getStartPoint());
        const RelativePoint end (getEndPoint());
        const Point<float> startResolved (start.resolve (scope));
        const Point<float> endResolved (end.resolve (scope));
        e.setControlPoint (0, startResolved + (endResolved - startResolved) * 0.3f, undoManager);
        e.setControlPoint (1, startResolved + (endResolved - startResolved) * 0.7f, undoManager);
        e.setControlPoint (2, end, undoManager);

        state = newState;
    }
}

void DrawablePath::ValueTreeWrapper::Element::convertToPathBreak (UndoManager* undoManager)
{
    const Identifier i (state.getType());

    if (i != startSubPathElement)
    {
        ValueTree newState (startSubPathElement);
        Element e (newState);
        e.setControlPoint (0, getEndPoint(), undoManager);
        state = newState;
    }
}

namespace DrawablePathHelpers
{
    static Point<float> findCubicSubdivisionPoint (float proportion, const Point<float> points[4])
    {
        const Point<float> mid1 (points[0] + (points[1] - points[0]) * proportion),
                           mid2 (points[1] + (points[2] - points[1]) * proportion),
                           mid3 (points[2] + (points[3] - points[2]) * proportion);

        const Point<float> newCp1 (mid1 + (mid2 - mid1) * proportion),
                           newCp2 (mid2 + (mid3 - mid2) * proportion);

        return newCp1 + (newCp2 - newCp1) * proportion;
    }

    static Point<float> findQuadraticSubdivisionPoint (float proportion, const Point<float> points[3])
    {
        const Point<float> mid1 (points[0] + (points[1] - points[0]) * proportion),
                           mid2 (points[1] + (points[2] - points[1]) * proportion);

        return mid1 + (mid2 - mid1) * proportion;
    }
}

float DrawablePath::ValueTreeWrapper::Element::findProportionAlongLine (Point<float> targetPoint, Expression::Scope* scope) const
{
    using namespace DrawablePathHelpers;
    const Identifier pointType (state.getType());
    float bestProp = 0;

    if (pointType == cubicToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getControlPoint (1)), rp4 (getEndPoint());

        const Point<float> points[] = { rp1.resolve (scope), rp2.resolve (scope), rp3.resolve (scope), rp4.resolve (scope) };

        float bestDistance = std::numeric_limits<float>::max();

        for (int i = 110; --i >= 0;)
        {
            float prop = i > 10 ? ((i - 10) / 100.0f) : (bestProp + ((i - 5) / 1000.0f));
            const Point<float> centre (findCubicSubdivisionPoint (prop, points));
            const float distance = centre.getDistanceFrom (targetPoint);

            if (distance < bestDistance)
            {
                bestProp = prop;
                bestDistance = distance;
            }
        }
    }
    else if (pointType == quadraticToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (scope), rp2.resolve (scope), rp3.resolve (scope) };

        float bestDistance = std::numeric_limits<float>::max();

        for (int i = 110; --i >= 0;)
        {
            float prop = i > 10 ? ((i - 10) / 100.0f) : (bestProp + ((i - 5) / 1000.0f));
            const Point<float> centre (findQuadraticSubdivisionPoint ((float) prop, points));
            const float distance = centre.getDistanceFrom (targetPoint);

            if (distance < bestDistance)
            {
                bestProp = prop;
                bestDistance = distance;
            }
        }
    }
    else if (pointType == lineToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getEndPoint());
        const Line<float> line (rp1.resolve (scope), rp2.resolve (scope));
        bestProp = line.findNearestProportionalPositionTo (targetPoint);
    }

    return bestProp;
}

ValueTree DrawablePath::ValueTreeWrapper::Element::insertPoint (Point<float> targetPoint, Expression::Scope* scope, UndoManager* undoManager)
{
    ValueTree newTree;
    const Identifier pointType (state.getType());

    if (pointType == cubicToElement)
    {
        float bestProp = findProportionAlongLine (targetPoint, scope);

        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getControlPoint (1)), rp4 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (scope), rp2.resolve (scope), rp3.resolve (scope), rp4.resolve (scope) };

        const Point<float> mid1 (points[0] + (points[1] - points[0]) * bestProp),
                           mid2 (points[1] + (points[2] - points[1]) * bestProp),
                           mid3 (points[2] + (points[3] - points[2]) * bestProp);

        const Point<float> newCp1 (mid1 + (mid2 - mid1) * bestProp),
                           newCp2 (mid2 + (mid3 - mid2) * bestProp);

        const Point<float> newCentre (newCp1 + (newCp2 - newCp1) * bestProp);

        setControlPoint (0, mid1, undoManager);
        setControlPoint (1, newCp1, undoManager);
        setControlPoint (2, newCentre, undoManager);
        setModeOfEndPoint (roundedMode, undoManager);

        Element newElement (newTree = ValueTree (cubicToElement));
        newElement.setControlPoint (0, newCp2, nullptr);
        newElement.setControlPoint (1, mid3, nullptr);
        newElement.setControlPoint (2, rp4, nullptr);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (pointType == quadraticToElement)
    {
        float bestProp = findProportionAlongLine (targetPoint, scope);

        RelativePoint rp1 (getStartPoint()), rp2 (getControlPoint (0)), rp3 (getEndPoint());
        const Point<float> points[] = { rp1.resolve (scope), rp2.resolve (scope), rp3.resolve (scope) };

        const Point<float> mid1 (points[0] + (points[1] - points[0]) * bestProp),
                           mid2 (points[1] + (points[2] - points[1]) * bestProp);

        const Point<float> newCentre (mid1 + (mid2 - mid1) * bestProp);

        setControlPoint (0, mid1, undoManager);
        setControlPoint (1, newCentre, undoManager);
        setModeOfEndPoint (roundedMode, undoManager);

        Element newElement (newTree = ValueTree (quadraticToElement));
        newElement.setControlPoint (0, mid2, nullptr);
        newElement.setControlPoint (1, rp3, nullptr);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (pointType == lineToElement)
    {
        RelativePoint rp1 (getStartPoint()), rp2 (getEndPoint());
        const Line<float> line (rp1.resolve (scope), rp2.resolve (scope));
        const Point<float> newPoint (line.findNearestPointTo (targetPoint));

        setControlPoint (0, newPoint, undoManager);

        Element newElement (newTree = ValueTree (lineToElement));
        newElement.setControlPoint (0, rp2, nullptr);

        state.getParent().addChild (newTree, state.getParent().indexOf (state) + 1, undoManager);
    }
    else if (pointType == closeSubPathElement)
    {
    }

    return newTree;
}

void DrawablePath::ValueTreeWrapper::Element::removePoint (UndoManager* undoManager)
{
    state.getParent().removeChild (state, undoManager);
}

//==============================================================================
void DrawablePath::refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    refreshFillTypes (v, builder.getImageProvider());
    setStrokeType (v.getStrokeType());

    RelativePointPath newRelativePath;
    v.writeTo (newRelativePath);
    setPath (newRelativePath);
}

ValueTree DrawablePath::createValueTree (ComponentBuilder::ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    writeTo (v, imageProvider, nullptr);

    if (relativePath != nullptr)
        v.readFrom (*relativePath, nullptr);
    else
        v.readFrom (RelativePointPath (path), nullptr);

    return tree;
}
