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

#include "../../Application/jucer_Headers.h"
#include "jucer_PaintElementPath.h"
#include "../Properties/jucer_PositionPropertyBase.h"
#include "jucer_PaintElementUndoableAction.h"
#include "../jucer_UtilityFunctions.h"

//==============================================================================
class ChangePointAction     : public PaintElementUndoableAction <PaintElementPath>
{
public:
    ChangePointAction (PathPoint* const point,
                       const int pointIndex,
                       const PathPoint& newValue_)
        : PaintElementUndoableAction <PaintElementPath> (point->owner),
          index (pointIndex),
          newValue (newValue_),
          oldValue (*point)
    {
    }

    ChangePointAction (PathPoint* const point,
                       const PathPoint& newValue_)
        : PaintElementUndoableAction <PaintElementPath> (point->owner),
          index (point->owner->indexOfPoint (point)),
          newValue (newValue_),
          oldValue (*point)
    {
    }

    bool perform()
    {
        return changeTo (newValue);
    }

    bool undo()
    {
        return changeTo (oldValue);
    }

private:
    const int index;
    PathPoint newValue, oldValue;

    PathPoint* getPoint() const
    {
        PathPoint* p = getElement()->getPoint (index);
        jassert (p != nullptr);
        return p;
    }

    bool changeTo (const PathPoint& value) const
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != nullptr);

        PathPoint* const p = path->getPoint (index);
        jassert (p != nullptr);

        const bool typeChanged = (p->type != value.type);
        *p = value;
        p->owner = path;

        if (typeChanged)
            path->pointListChanged();

        path->changed();
        return true;
    }
};


//==============================================================================
class PathWindingModeProperty    : public ChoicePropertyComponent,
                                   public ChangeListener
{
public:
    PathWindingModeProperty (PaintElementPath* const owner_)
        : ChoicePropertyComponent ("winding rule"),
          owner (owner_)
    {
        choices.add ("Non-zero winding");
        choices.add ("Even/odd winding");

        owner->getDocument()->addChangeListener (this);
    }

    ~PathWindingModeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setIndex (int newIndex)            { owner->setNonZeroWinding (newIndex == 0, true); }
    int getIndex() const                    { return owner->isNonZeroWinding() ? 0 : 1; }

    void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

private:
    PaintElementPath* const owner;
};


//==============================================================================
PaintElementPath::PaintElementPath (PaintRoutine* pr)
    : ColouredElement (pr, "Path", true, true),
      nonZeroWinding (true)
{
}

PaintElementPath::~PaintElementPath()
{
}

static int randomPos (int size)
{
    return size / 4 + Random::getSystemRandom().nextInt (size / 4) - size / 8;
}

void PaintElementPath::setInitialBounds (int w, int h)
{
    String s;

    int x = randomPos (w);
    int y = randomPos (h);

    s << "s "
      << x << " " << y << " l "
      << (x + 30) << " " << (y + 50) << " l "
      << (x - 30) << " " << (y + 50) << " x";

    restorePathFromString (s);
}

//==============================================================================
int PaintElementPath::getBorderSize() const
{
    return isStrokePresent ? 1 + roundFloatToInt (strokeType.stroke.getStrokeThickness())
                           : 0;
}

Rectangle<int> PaintElementPath::getCurrentBounds (const Rectangle<int>& parentArea) const
{
    updateStoredPath (getDocument()->getComponentLayout(), parentArea);

    Rectangle<float> r (path.getBounds());

    const int borderSize = getBorderSize();

    return Rectangle<int> ((int) r.getX() - borderSize,
                           (int) r.getY() - borderSize,
                           (int) r.getWidth() + borderSize * 2,
                           (int) r.getHeight() + borderSize * 2);
}

void PaintElementPath::setCurrentBounds (const Rectangle<int>& b,
                                         const Rectangle<int>& parentArea,
                                         const bool /*undoable*/)
{
    Rectangle<int> newBounds (b);
    newBounds.setSize (jmax (1, newBounds.getWidth()),
                       jmax (1, newBounds.getHeight()));

    const Rectangle<int> current (getCurrentBounds (parentArea));

    if (newBounds != current)
    {
        const int borderSize = getBorderSize();

        const int dx = newBounds.getX() - current.getX();
        const int dy = newBounds.getY() - current.getY();

        const double scaleStartX = current.getX() + borderSize;
        const double scaleStartY = current.getY() + borderSize;
        const double scaleX = (newBounds.getWidth() - borderSize * 2) / (double) (current.getWidth() - borderSize * 2);
        const double scaleY = (newBounds.getHeight() - borderSize * 2) / (double) (current.getHeight() - borderSize * 2);

        for (int i = 0; i < points.size(); ++i)
        {
            PathPoint* const destPoint = points.getUnchecked(i);
            PathPoint p (*destPoint);

            for (int j = p.getNumPoints(); --j >= 0;)
                rescalePoint (p.pos[j], dx, dy,
                              scaleX, scaleY,
                              scaleStartX, scaleStartY,
                              parentArea);

            perform (new ChangePointAction (destPoint, i, p), "Move path");
        }
    }
}

void PaintElementPath::rescalePoint (RelativePositionedRectangle& pos, int dx, int dy,
                                     double scaleX, double scaleY,
                                     double scaleStartX, double scaleStartY,
                                     const Rectangle<int>& parentArea) const
{
    double x, y, w, h;
    pos.getRectangleDouble (x, y, w, h, parentArea, getDocument()->getComponentLayout());

    x = (x - scaleStartX) * scaleX + scaleStartX + dx;
    y = (y - scaleStartY) * scaleY + scaleStartY + dy;

    pos.updateFrom (x, y, w, h, parentArea, getDocument()->getComponentLayout());
}

//==============================================================================
static void drawArrow (Graphics& g, const Point<float> p1, const Point<float> p2)
{
    g.drawArrow (Line<float> (p1.x, p1.y, (p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f), 1.0f, 8.0f, 10.0f);
    g.drawLine (p1.x + (p2.x - p1.x) * 0.49f, p1.y + (p2.y - p1.y) * 0.49f, p2.x, p2.y);
}

void PaintElementPath::draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
{
    updateStoredPath (layout, parentArea);
    path.setUsingNonZeroWinding (nonZeroWinding);

    fillType.setFillType (g, getDocument(), parentArea);
    g.fillPath (path);

    if (isStrokePresent)
    {
        strokeType.fill.setFillType (g, getDocument(), parentArea);
        g.strokePath (path, getStrokeType().stroke);
    }
}

void PaintElementPath::drawExtraEditorGraphics (Graphics& g, const Rectangle<int>& relativeTo)
{
    ComponentLayout* layout = getDocument()->getComponentLayout();

    for (int i = 0; i < points.size(); ++i)
    {
        PathPoint* const p = points.getUnchecked (i);

        const int numPoints = p->getNumPoints();

        if (numPoints > 0)
        {
            if (owner->getSelectedPoints().isSelected (p))
            {
                g.setColour (Colours::red);
                Point<float> p1, p2;

                if (numPoints > 2)
                {
                    p1 = p->pos[1].toXY (relativeTo, layout);
                    p2 = p->pos[2].toXY (relativeTo, layout);
                    drawArrow (g, p1, p2);
                }

                if (numPoints > 1)
                {
                    p1 = p->pos[0].toXY (relativeTo, layout);
                    p2 = p->pos[1].toXY (relativeTo, layout);
                    drawArrow (g, p1, p2);
                }

                p2 = p->pos[0].toXY (relativeTo, layout);

                if (const PathPoint* const nextPoint = points [i - 1])
                {
                    p1 = nextPoint->pos [nextPoint->getNumPoints() - 1].toXY (relativeTo, layout);
                    drawArrow (g, p1, p2);
                }
            }
        }
    }
}

void PaintElementPath::resized()
{
    ColouredElement::resized();
}

void PaintElementPath::parentSizeChanged()
{
    repaint();
}

//==============================================================================
void PaintElementPath::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu() || ! owner->getSelectedElements().isSelected (this))
        mouseDownOnSegment = -1;
    else
        mouseDownOnSegment = findSegmentAtXY (getX() + e.x, getY() + e.y);

    if (points [mouseDownOnSegment] != nullptr)
        mouseDownSelectSegmentStatus = owner->getSelectedPoints().addToSelectionOnMouseDown (points [mouseDownOnSegment], e.mods);
    else
        ColouredElement::mouseDown (e);
}

void PaintElementPath::mouseDrag (const MouseEvent& e)
{
    if (mouseDownOnSegment < 0)
        ColouredElement::mouseDrag (e);
}

void PaintElementPath::mouseUp (const MouseEvent& e)
{
    if (points [mouseDownOnSegment] == 0)
        ColouredElement::mouseUp (e);
    else
        owner->getSelectedPoints().addToSelectionOnMouseUp (points [mouseDownOnSegment],
                                                            e.mods, false, mouseDownSelectSegmentStatus);
}

//==============================================================================
void PaintElementPath::changed()
{
    ColouredElement::changed();
    lastPathBounds = Rectangle<int>();
}

void PaintElementPath::pointListChanged()
{
    changed();
    siblingComponentsChanged();
}

//==============================================================================
void PaintElementPath::getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected)
{
    if (multipleSelected)
        return;

    props.add (new PathWindingModeProperty (this));
    getColourSpecificProperties (props);
}

//==============================================================================
static String positionToPairOfValues (const RelativePositionedRectangle& position,
                                      const ComponentLayout* layout)
{
    String x, y, w, h;
    positionToCode (position, layout, x, y, w, h);
    return castToFloat (x) + ", " + castToFloat (y);
}

void PaintElementPath::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
{
    if (fillType.isInvisible() && (strokeType.isInvisible() || ! isStrokePresent))
        return;

    const String pathVariable ("internalPath" + String (code.getUniqueSuffix()));

    const ComponentLayout* layout = code.document->getComponentLayout();

    code.privateMemberDeclarations
        << "Path " << pathVariable << ";\n";

    String r;
    bool somePointsAreRelative = false;

    if (! nonZeroWinding)
        r << pathVariable << ".setUsingNonZeroWinding (false);\n";

    for (auto* p : points)
    {
        switch (p->type)
        {
        case Path::Iterator::startNewSubPath:
            r << pathVariable << ".startNewSubPath (" << positionToPairOfValues (p->pos[0], layout) << ");\n";
            somePointsAreRelative = somePointsAreRelative || ! p->pos[0].rect.isPositionAbsolute();
            break;

        case Path::Iterator::lineTo:
            r << pathVariable << ".lineTo (" << positionToPairOfValues (p->pos[0], layout) << ");\n";
            somePointsAreRelative = somePointsAreRelative || ! p->pos[0].rect.isPositionAbsolute();
            break;

        case Path::Iterator::quadraticTo:
            r << pathVariable << ".quadraticTo (" << positionToPairOfValues (p->pos[0], layout)
              << ", " << positionToPairOfValues (p->pos[1], layout) << ");\n";

            somePointsAreRelative = somePointsAreRelative || ! p->pos[0].rect.isPositionAbsolute();
            somePointsAreRelative = somePointsAreRelative || ! p->pos[1].rect.isPositionAbsolute();
            break;

        case Path::Iterator::cubicTo:
            r << pathVariable << ".cubicTo (" << positionToPairOfValues (p->pos[0], layout)
              << ", " << positionToPairOfValues (p->pos[1], layout)
              << ", " << positionToPairOfValues (p->pos[2], layout) << ");\n";

            somePointsAreRelative = somePointsAreRelative || ! p->pos[0].rect.isPositionAbsolute();
            somePointsAreRelative = somePointsAreRelative || ! p->pos[1].rect.isPositionAbsolute();
            somePointsAreRelative = somePointsAreRelative || ! p->pos[2].rect.isPositionAbsolute();
            break;

        case Path::Iterator::closePath:
            r << pathVariable << ".closeSubPath();\n";
            break;

        default:
            jassertfalse;
            break;
        }
    }

    r << '\n';

    if (somePointsAreRelative)
        code.getCallbackCode (String(), "void", "resized()", false)
            << pathVariable << ".clear();\n" << r;
    else
        code.constructorCode << r;

    String s;
    s << "{\n"
      << "    float x = 0, y = 0;\n";

    if (! fillType.isInvisible())
        s << "    " << fillType.generateVariablesCode ("fill");

    if (isStrokePresent && ! strokeType.isInvisible())
        s << "    " << strokeType.fill.generateVariablesCode ("stroke");

    s << "    //[UserPaintCustomArguments] Customize the painting arguments here..\n"
      << customPaintCode
      << "    //[/UserPaintCustomArguments]\n";

    RelativePositionedRectangle zero;

    if (! fillType.isInvisible())
    {
        s << "    ";
        fillType.fillInGeneratedCode ("fill", zero, code, s);
        s << "    g.fillPath (" << pathVariable << ", AffineTransform::translation(x, y));\n";
    }

    if (isStrokePresent && ! strokeType.isInvisible())
    {
        s << "    ";
        strokeType.fill.fillInGeneratedCode ("stroke", zero, code, s);
        s << "    g.strokePath (" << pathVariable << ", " << strokeType.getPathStrokeCode() << ", AffineTransform::translation(x, y));\n";
    }

    s << "}\n\n";

    paintMethodCode += s;
}

void PaintElementPath::applyCustomPaintSnippets (StringArray& snippets)
{
    customPaintCode.clear();

    if (! snippets.isEmpty() && (! fillType.isInvisible() || (isStrokePresent && ! strokeType.isInvisible())))
    {
        customPaintCode = snippets[0];
        snippets.remove(0);
    }
}

//==============================================================================
XmlElement* PaintElementPath::createXml() const
{
    XmlElement* e = new XmlElement (getTagName());
    position.applyToXml (*e);
    addColourAttributes (e);
    e->setAttribute ("nonZeroWinding", nonZeroWinding);
    e->addTextElement (pathToString());

    return e;
}

bool PaintElementPath::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (getTagName()))
    {
        position.restoreFromXml (xml, position);
        loadColourAttributes (xml);
        nonZeroWinding = xml.getBoolAttribute ("nonZeroWinding", true);

        restorePathFromString (xml.getAllSubText().trim());

        return true;
    }

    jassertfalse;
    return false;
}

//==============================================================================
void PaintElementPath::createSiblingComponents()
{
    ColouredElement::createSiblingComponents();

    for (int i = 0; i < points.size(); ++i)
    {
        switch (points.getUnchecked(i)->type)
        {
            case Path::Iterator::startNewSubPath:
                siblingComponents.add (new PathPointComponent (this, i, 0));
                break;
            case Path::Iterator::lineTo:
                siblingComponents.add (new PathPointComponent (this, i, 0));
                break;
            case Path::Iterator::quadraticTo:
                siblingComponents.add (new PathPointComponent (this, i, 0));
                siblingComponents.add (new PathPointComponent (this, i, 1));
                break;
            case Path::Iterator::cubicTo:
                siblingComponents.add (new PathPointComponent (this, i, 0));
                siblingComponents.add (new PathPointComponent (this, i, 1));
                siblingComponents.add (new PathPointComponent (this, i, 2));
                break;
            case Path::Iterator::closePath:
                break;

            default:
                jassertfalse; break;
        }
    }

    for (int i = 0; i < siblingComponents.size(); ++i)
    {
        getParentComponent()->addAndMakeVisible (siblingComponents.getUnchecked(i));
        siblingComponents.getUnchecked(i)->updatePosition();
    }
}


String PaintElementPath::pathToString() const
{
    String s;

    for (int i = 0; i < points.size(); ++i)
    {
        const PathPoint* const p = points.getUnchecked(i);

        switch (p->type)
        {
            case Path::Iterator::startNewSubPath:
                s << "s " << p->pos[0].toString() << ' ';
                break;
            case Path::Iterator::lineTo:
                s << "l " << p->pos[0].toString() << ' ';
                break;
            case Path::Iterator::quadraticTo:
                s << "q " << p->pos[0].toString()
                  << ' '  << p->pos[1].toString() << ' ';
                break;
            case Path::Iterator::cubicTo:
                s << "c " << p->pos[0].toString()
                  << ' '  << p->pos[1].toString() << ' '
                  << ' '  << p->pos[2].toString() << ' ';
                break;
            case Path::Iterator::closePath:
                s << "x ";
                break;

            default:
                jassertfalse; break;
        }
    }

    return s.trimEnd();
}

void PaintElementPath::restorePathFromString (const String& s)
{
    points.clear();

    StringArray tokens;
    tokens.addTokens (s, false);
    tokens.trim();
    tokens.removeEmptyStrings();

    for (int i = 0; i < tokens.size(); ++i)
    {
        ScopedPointer<PathPoint> p (new PathPoint (this));

        if (tokens[i] == "s")
        {
            p->type = Path::Iterator::startNewSubPath;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            i += 2;
        }
        else if (tokens[i] == "l")
        {
            p->type = Path::Iterator::lineTo;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            i += 2;
        }
        else if (tokens[i] == "q")
        {
            p->type = Path::Iterator::quadraticTo;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            p->pos [1] = RelativePositionedRectangle();
            p->pos [1].rect = PositionedRectangle (tokens [i + 3] + " " + tokens [i + 4]);
            i += 4;
        }
        else if (tokens[i] == "c")
        {
            p->type = Path::Iterator::cubicTo;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            p->pos [1] = RelativePositionedRectangle();
            p->pos [1].rect = PositionedRectangle (tokens [i + 3] + " " + tokens [i + 4]);
            p->pos [2] = RelativePositionedRectangle();
            p->pos [2].rect = PositionedRectangle (tokens [i + 5] + " " + tokens [i + 6]);
            i += 6;
        }
        else if (tokens[i] == "x")
        {
            p->type = Path::Iterator::closePath;
        }
        else
            continue;

        points.add (p.release());
    }
}

void PaintElementPath::setToPath (const Path& newPath)
{
    points.clear();

    Path::Iterator i (newPath);

    while (i.next())
    {
        ScopedPointer<PathPoint> p (new PathPoint (this));
        p->type = i.elementType;

        if (i.elementType == Path::Iterator::startNewSubPath)
        {
            p->pos [0].rect.setX (i.x1);
            p->pos [0].rect.setY (i.y1);
        }
        else if (i.elementType == Path::Iterator::lineTo)
        {
            p->pos [0].rect.setX (i.x1);
            p->pos [0].rect.setY (i.y1);
        }
        else if (i.elementType == Path::Iterator::quadraticTo)
        {
            p->pos [0].rect.setX (i.x1);
            p->pos [0].rect.setY (i.y1);
            p->pos [1].rect.setX (i.x2);
            p->pos [1].rect.setY (i.y2);
        }
        else if (i.elementType == Path::Iterator::cubicTo)
        {
            p->pos [0].rect.setX (i.x1);
            p->pos [0].rect.setY (i.y1);
            p->pos [1].rect.setX (i.x2);
            p->pos [1].rect.setY (i.y2);
            p->pos [2].rect.setX (i.x3);
            p->pos [2].rect.setY (i.y3);
        }
        else if (i.elementType == Path::Iterator::closePath)
        {
        }
        else
        {
            continue;
        }

        points.add (p.release());
    }
}

void PaintElementPath::updateStoredPath (const ComponentLayout* layout, const Rectangle<int>& relativeTo) const
{
    if (lastPathBounds != relativeTo && ! relativeTo.isEmpty())
    {
        lastPathBounds = relativeTo;
        path.clear();

        for (int i = 0; i < points.size(); ++i)
        {
            const PathPoint* const p = points.getUnchecked(i);

            switch (p->type)
            {
                case Path::Iterator::startNewSubPath:
                    path.startNewSubPath (p->pos[0].toXY (relativeTo, layout));
                    break;

                case Path::Iterator::lineTo:
                    path.lineTo (p->pos[0].toXY (relativeTo, layout));
                    break;

                case Path::Iterator::quadraticTo:
                    path.quadraticTo (p->pos[0].toXY (relativeTo, layout),
                                      p->pos[1].toXY (relativeTo, layout));
                    break;

                case Path::Iterator::cubicTo:
                    path.cubicTo (p->pos[0].toXY (relativeTo, layout),
                                  p->pos[1].toXY (relativeTo, layout),
                                  p->pos[2].toXY (relativeTo, layout));
                    break;

                case Path::Iterator::closePath:
                    path.closeSubPath();
                    break;

                default:
                    jassertfalse; break;
            }
        }
    }
}

//==============================================================================
class ChangeWindingAction     : public PaintElementUndoableAction <PaintElementPath>
{
public:
    ChangeWindingAction (PaintElementPath* const path, const bool newValue_)
        : PaintElementUndoableAction <PaintElementPath> (path),
          newValue (newValue_),
          oldValue (path->isNonZeroWinding())
    {
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setNonZeroWinding (newValue, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setNonZeroWinding (oldValue, false);
        return true;
    }

private:
    bool newValue, oldValue;
};

void PaintElementPath::setNonZeroWinding (const bool nonZero, const bool undoable)
{
    if (nonZero != nonZeroWinding)
    {
        if (undoable)
        {
            perform (new ChangeWindingAction (this, nonZero), "Change path winding rule");
        }
        else
        {
            nonZeroWinding = nonZero;
            changed();
        }
    }
}

bool PaintElementPath::isSubpathClosed (int index) const
{
    for (int i = index + 1; i < points.size(); ++i)
    {
        if (points.getUnchecked (i)->type == Path::Iterator::closePath)
            return true;

        if (points.getUnchecked (i)->type == Path::Iterator::startNewSubPath)
            break;
    }

    return false;
}

//==============================================================================
void PaintElementPath::setSubpathClosed (int index, const bool closed, const bool undoable)
{
    if (closed != isSubpathClosed (index))
    {
        for (int i = index + 1; i < points.size(); ++i)
        {
            PathPoint* p = points.getUnchecked (i);

            if (p->type == Path::Iterator::closePath)
            {
                jassert (! closed);

                deletePoint (i, undoable);
                return;
            }

            if (p->type == Path::Iterator::startNewSubPath)
            {
                jassert (closed);

                PathPoint* pp = addPoint (i - 1, undoable);

                PathPoint p2 (*pp);
                p2.type = Path::Iterator::closePath;
                perform (new ChangePointAction (pp, p2), "Close subpath");
                return;
            }
        }

        jassert (closed);

        PathPoint* p = addPoint (points.size() - 1, undoable);
        PathPoint p2 (*p);
        p2.type = Path::Iterator::closePath;
        perform (new ChangePointAction (p, p2), "Close subpath");
    }
}

//==============================================================================
class AddPointAction   : public PaintElementUndoableAction <PaintElementPath>
{
public:
    AddPointAction (PaintElementPath* path, int pointIndexToAddItAfter_)
        : PaintElementUndoableAction <PaintElementPath> (path),
          indexAdded (-1),
          pointIndexToAddItAfter (pointIndexToAddItAfter_)
    {
    }

    bool perform()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != nullptr);

        PathPoint* const p = path->addPoint (pointIndexToAddItAfter, false);
        jassert (p != nullptr);

        indexAdded = path->indexOfPoint (p);
        jassert (indexAdded >= 0);
        return true;
    }

    bool undo()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != nullptr);

        path->deletePoint (indexAdded, false);
        return true;
    }

    int indexAdded;

private:
    int pointIndexToAddItAfter;
};

PathPoint* PaintElementPath::addPoint (int pointIndexToAddItAfter, const bool undoable)
{
    if (undoable)
    {
        AddPointAction* action = new AddPointAction (this, pointIndexToAddItAfter);
        perform (action, "Add path point");
        return points [action->indexAdded];
    }

    double x1 = 20.0, y1 = 20.0, x2, y2;

    ComponentLayout* layout = getDocument()->getComponentLayout();
    const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    if (points [pointIndexToAddItAfter] != nullptr)
        points [pointIndexToAddItAfter]->pos [points [pointIndexToAddItAfter]->getNumPoints() - 1].getXY (x1, y1, area, layout);
    else if (points[0] != nullptr)
        points[0]->pos[0].getXY (x1, y1, area, layout);

    x2 = x1 + 50.0;
    y2 = y1 + 50.0;

    if (points [pointIndexToAddItAfter + 1] != nullptr)
    {
        if (points [pointIndexToAddItAfter + 1]->type == Path::Iterator::closePath
             || points [pointIndexToAddItAfter + 1]->type == Path::Iterator::startNewSubPath)
        {
            int i = pointIndexToAddItAfter;
            while (i > 0)
                if (points [--i]->type == Path::Iterator::startNewSubPath)
                    break;

            if (i != pointIndexToAddItAfter)
                points [i]->pos[0].getXY (x2, y2, area, layout);
        }
        else
        {
            points [pointIndexToAddItAfter + 1]->pos[0].getXY (x2, y2, area, layout);
        }
    }
    else
    {
        int i = pointIndexToAddItAfter + 1;
        while (i > 0)
            if (points [--i]->type == Path::Iterator::startNewSubPath)
                break;

        points[i]->pos[0].getXY (x2, y2, area, layout);
    }

    PathPoint* const p = new PathPoint (this);

    p->type = Path::Iterator::lineTo;
    p->pos[0].rect.setX ((x1 + x2) * 0.5f);
    p->pos[0].rect.setY ((y1 + y2) * 0.5f);

    points.insert (pointIndexToAddItAfter + 1, p);

    pointListChanged();
    return p;
}

//==============================================================================
class DeletePointAction   : public PaintElementUndoableAction <PaintElementPath>
{
public:
    DeletePointAction (PaintElementPath* const path, const int indexToRemove_)
        : PaintElementUndoableAction <PaintElementPath> (path),
          indexToRemove (indexToRemove_),
          oldValue (*path->getPoint (indexToRemove))
    {
    }

    bool perform()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != nullptr);

        path->deletePoint (indexToRemove, false);
        return path != nullptr;
    }

    bool undo()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != nullptr);

        PathPoint* p = path->addPoint (indexToRemove - 1, false);
        *p = oldValue;

        return path != nullptr;
    }

    int indexToRemove;

private:
    PathPoint oldValue;
};

void PaintElementPath::deletePoint (int pointIndex, const bool undoable)
{
    if (undoable)
    {
        perform (new DeletePointAction (this, pointIndex), "Delete path point");
    }
    else
    {
        PathPoint* const p = points [pointIndex];

        if (p != nullptr && pointIndex > 0)
        {
            owner->getSelectedPoints().deselect (p);
            owner->getSelectedPoints().changed (true);

            points.remove (pointIndex);
            pointListChanged();
        }
    }
}

//==============================================================================
bool PaintElementPath::getPoint (int index, int pointNumber, double& x, double& y, const Rectangle<int>& parentArea) const
{
    const PathPoint* const p = points [index];

    if (p == nullptr)
    {
        x = y = 0;
        return false;
    }

    jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
    jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

    p->pos [pointNumber].getXY (x, y, parentArea, getDocument()->getComponentLayout());
    return true;
}

int PaintElementPath::findSegmentAtXY (int x, int y) const
{
    double x1, y1, x2, y2, x3, y3, lastX = 0.0, lastY = 0.0, subPathStartX = 0.0, subPathStartY = 0.0;

    ComponentLayout* const layout = getDocument()->getComponentLayout();
    const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

    int subpathStartIndex = 0;

    float thickness = 10.0f;
    if (isStrokePresent)
        thickness = jmax (thickness, strokeType.stroke.getStrokeThickness());

    for (int i = 0; i < points.size(); ++i)
    {
        Path segmentPath;
        PathPoint* const p = points.getUnchecked (i);

        switch (p->type)
        {
        case Path::Iterator::startNewSubPath:
            p->pos[0].getXY (lastX, lastY, area, layout);
            subPathStartX = lastX;
            subPathStartY = lastY;
            subpathStartIndex = i;
            break;

        case Path::Iterator::lineTo:
            p->pos[0].getXY (x1, y1, area, layout);

            segmentPath.addLineSegment (Line<float> ((float) lastX, (float) lastY, (float) x1, (float) y1), thickness);
            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x1;
            lastY = y1;
            break;

        case Path::Iterator::quadraticTo:
            p->pos[0].getXY (x1, y1, area, layout);
            p->pos[1].getXY (x2, y2, area, layout);

            segmentPath.startNewSubPath ((float) lastX, (float) lastY);
            segmentPath.quadraticTo ((float) x1, (float) y1, (float) x2, (float) y2);
            PathStrokeType (thickness).createStrokedPath (segmentPath, segmentPath);

            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x2;
            lastY = y2;
            break;

        case Path::Iterator::cubicTo:
            p->pos[0].getXY (x1, y1, area, layout);
            p->pos[1].getXY (x2, y2, area, layout);
            p->pos[2].getXY (x3, y3, area, layout);

            segmentPath.startNewSubPath ((float) lastX, (float) lastY);
            segmentPath.cubicTo ((float) x1, (float) y1, (float) x2, (float) y2, (float) x3, (float) y3);
            PathStrokeType (thickness).createStrokedPath (segmentPath, segmentPath);

            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x3;
            lastY = y3;
            break;

        case Path::Iterator::closePath:
            segmentPath.addLineSegment (Line<float> ((float) lastX, (float) lastY, (float) subPathStartX, (float) subPathStartY), thickness);
            if (segmentPath.contains ((float) x, (float) y))
                return subpathStartIndex;

            lastX = subPathStartX;
            lastY = subPathStartY;
            break;

        default:
            jassertfalse; break;
        }
    }

    return -1;
}

//==============================================================================
void PaintElementPath::movePoint (int index, int pointNumber,
                                  double newX, double newY,
                                  const Rectangle<int>& parentArea,
                                  const bool undoable)
{
    if (PathPoint* const p = points [index])
    {
        PathPoint newPoint (*p);
        jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
        jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

        RelativePositionedRectangle& pr = newPoint.pos [pointNumber];

        double x, y, w, h;
        pr.getRectangleDouble (x, y, w, h, parentArea, getDocument()->getComponentLayout());
        pr.updateFrom (newX, newY, w, h, parentArea, getDocument()->getComponentLayout());

        if (undoable)
        {
            perform (new ChangePointAction (p, index, newPoint), "Move path point");
        }
        else
        {
            *p = newPoint;
            changed();
        }
    }
}

RelativePositionedRectangle PaintElementPath::getPoint (int index, int pointNumber) const
{
    if (PathPoint* const p = points [index])
    {
        jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
        jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

        return p->pos [pointNumber];
    }

    jassertfalse;
    return RelativePositionedRectangle();
}

void PaintElementPath::setPoint (int index, int pointNumber, const RelativePositionedRectangle& newPos, const bool undoable)
{
    if (PathPoint* const p = points [index])
    {
        PathPoint newPoint (*p);

        jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
        jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

        if (newPoint.pos [pointNumber] != newPos)
        {
            newPoint.pos [pointNumber] = newPos;

            if (undoable)
            {
                perform (new ChangePointAction (p, index, newPoint), "Change path point position");
            }
            else
            {
                *p = newPoint;
                changed();
            }
        }
    }
    else
    {
        jassertfalse;
    }
}

//==============================================================================
class PathPointTypeProperty : public ChoicePropertyComponent,
                              public ChangeListener
{
public:
    PathPointTypeProperty (PaintElementPath* const owner_,
                           const int index_)
        : ChoicePropertyComponent ("point type"),
          owner (owner_),
          index (index_)
    {
        choices.add ("Start of sub-path");
        choices.add ("Line");
        choices.add ("Quadratic");
        choices.add ("Cubic");

        owner->getDocument()->addChangeListener (this);
    }

    ~PathPointTypeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setIndex (int newIndex)
    {
        Path::Iterator::PathElementType type = Path::Iterator::startNewSubPath;

        switch (newIndex)
        {
            case 0:     type = Path::Iterator::startNewSubPath; break;
            case 1:     type = Path::Iterator::lineTo; break;
            case 2:     type = Path::Iterator::quadraticTo; break;
            case 3:     type = Path::Iterator::cubicTo; break;
            default:    jassertfalse; break;
        }

        const Rectangle<int> area (((PaintRoutineEditor*) owner->getParentComponent())->getComponentArea());
        owner->getPoint (index)->changePointType (type, area, true);
    }

    int getIndex() const
    {
        const PathPoint* const p = owner->getPoint (index);
        jassert (p != nullptr);

        switch (p->type)
        {
            case Path::Iterator::startNewSubPath:   return 0;
            case Path::Iterator::lineTo:            return 1;
            case Path::Iterator::quadraticTo:       return 2;
            case Path::Iterator::cubicTo:           return 3;
            case Path::Iterator::closePath:         break;
            default:                                jassertfalse; break;
        }

        return 0;
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

private:
    PaintElementPath* const owner;
    const int index;
};

//==============================================================================
class PathPointPositionProperty   : public PositionPropertyBase
{
public:
    PathPointPositionProperty (PaintElementPath* const owner_,
                               const int index_, const int pointNumber_,
                               const String& name,
                               ComponentPositionDimension dimension_)
        : PositionPropertyBase (owner_, name, dimension_, false, false,
                                owner_->getDocument()->getComponentLayout()),
          owner (owner_),
          index (index_),
          pointNumber (pointNumber_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~PathPointPositionProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        owner->setPoint (index, pointNumber, newPos, true);
    }

    RelativePositionedRectangle getPosition() const
    {
        return owner->getPoint (index, pointNumber);
    }

private:
    PaintElementPath* const owner;
    const int index, pointNumber;
};

//==============================================================================
class PathPointClosedProperty   : public ChoicePropertyComponent,
                                  private ChangeListener
{
public:
    PathPointClosedProperty (PaintElementPath* const owner_, const int index_)
        : ChoicePropertyComponent ("openness"),
          owner (owner_),
          index (index_)
    {
        owner->getDocument()->addChangeListener (this);

        choices.add ("Subpath is closed");
        choices.add ("Subpath is open-ended");
    }

    ~PathPointClosedProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    void setIndex (int newIndex)
    {
        owner->setSubpathClosed (index, newIndex == 0, true);
    }

    int getIndex() const
    {
        return owner->isSubpathClosed (index) ? 0 : 1;
    }

private:
    PaintElementPath* const owner;
    const int index;
};

//==============================================================================
class AddNewPointProperty   : public ButtonPropertyComponent
{
public:
    AddNewPointProperty (PaintElementPath* const owner_, const int index_)
        : ButtonPropertyComponent ("new point", false),
          owner (owner_),
          index (index_)
    {
    }

    void buttonClicked()
    {
        owner->addPoint (index, true);
    }

    String getButtonText() const      { return "Add new point"; }

private:
    PaintElementPath* const owner;
    const int index;
};


//==============================================================================
PathPoint::PathPoint (PaintElementPath* const owner_)
    : owner (owner_)
{
}

PathPoint::PathPoint (const PathPoint& other)
    : owner (other.owner),
      type (other.type)
{
    pos [0] = other.pos [0];
    pos [1] = other.pos [1];
    pos [2] = other.pos [2];
}

PathPoint& PathPoint::operator= (const PathPoint& other)
{
    owner = other.owner;
    type = other.type;
    pos [0] = other.pos [0];
    pos [1] = other.pos [1];
    pos [2] = other.pos [2];
    return *this;
}

PathPoint::~PathPoint()
{
}

int PathPoint::getNumPoints() const
{
    if (type == Path::Iterator::cubicTo)        return 3;
    if (type == Path::Iterator::quadraticTo)    return 2;
    if (type == Path::Iterator::closePath)      return 0;

    return 1;
}

PathPoint PathPoint::withChangedPointType (const Path::Iterator::PathElementType newType,
                                           const Rectangle<int>& parentArea) const
{
    PathPoint p (*this);

    if (newType != p.type)
    {
        int oldNumPoints = getNumPoints();
        p.type = newType;
        int numPoints = p.getNumPoints();

        if (numPoints != oldNumPoints)
        {
            double lastX, lastY;
            double x, y, w, h;

            p.pos [numPoints - 1] = p.pos [oldNumPoints - 1];
            p.pos [numPoints - 1].getRectangleDouble (x, y, w, h, parentArea, owner->getDocument()->getComponentLayout());

            const int index = owner->points.indexOf (this);

            if (PathPoint* lastPoint = owner->points [index - 1])
            {
                lastPoint->pos [lastPoint->getNumPoints() - 1]
                            .getRectangleDouble (lastX, lastY, w, h, parentArea, owner->getDocument()->getComponentLayout());
            }
            else
            {
                jassertfalse;
                lastX = x;
                lastY = y;
            }

            for (int i = 0; i < numPoints - 1; ++i)
            {
                p.pos[i] = p.pos [numPoints - 1];

                p.pos[i].updateFrom (lastX + (x - lastX) * (i + 1) / numPoints,
                                     lastY + (y - lastY) * (i + 1) / numPoints,
                                     w, h,
                                     parentArea,
                                     owner->getDocument()->getComponentLayout());
            }
        }
    }

    return p;
}

void PathPoint::changePointType (const Path::Iterator::PathElementType newType,
                                 const Rectangle<int>& parentArea, const bool undoable)
{
    if (newType != type)
    {
        if (undoable)
        {
            owner->perform (new ChangePointAction (this, withChangedPointType (newType, parentArea)),
                            "Change path point type");
        }
        else
        {
            *this = withChangedPointType (newType, parentArea);
            owner->pointListChanged();
        }
    }
}

void PathPoint::getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected)
{
    if (multipleSelected)
        return;

    auto index = owner->points.indexOf (this);
    jassert (index >= 0);

    switch (type)
    {
        case Path::Iterator::startNewSubPath:
            props.add (new PathPointPositionProperty (owner, index, 0, "x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 0, "y", PositionPropertyBase::componentY));

            props.add (new PathPointClosedProperty (owner, index));
            props.add (new AddNewPointProperty (owner, index));
            break;

        case Path::Iterator::lineTo:
            props.add (new PathPointTypeProperty (owner, index));
            props.add (new PathPointPositionProperty (owner, index, 0, "x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 0, "y", PositionPropertyBase::componentY));
            props.add (new AddNewPointProperty (owner, index));
            break;

        case Path::Iterator::quadraticTo:
            props.add (new PathPointTypeProperty (owner, index));
            props.add (new PathPointPositionProperty (owner, index, 0, "control pt x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 0, "control pt y", PositionPropertyBase::componentY));
            props.add (new PathPointPositionProperty (owner, index, 1, "x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 1, "y", PositionPropertyBase::componentY));
            props.add (new AddNewPointProperty (owner, index));
            break;

        case Path::Iterator::cubicTo:
            props.add (new PathPointTypeProperty (owner, index));
            props.add (new PathPointPositionProperty (owner, index, 0, "control pt1 x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 0, "control pt1 y", PositionPropertyBase::componentY));
            props.add (new PathPointPositionProperty (owner, index, 1, "control pt2 x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 1, "control pt2 y", PositionPropertyBase::componentY));
            props.add (new PathPointPositionProperty (owner, index, 2, "x", PositionPropertyBase::componentX));
            props.add (new PathPointPositionProperty (owner, index, 2, "y", PositionPropertyBase::componentY));
            props.add (new AddNewPointProperty (owner, index));
            break;

        case Path::Iterator::closePath:
            break;

        default:
            jassertfalse;
            break;
    }
}

void PathPoint::deleteFromPath()
{
    owner->deletePoint (owner->points.indexOf (this), true);
}

//==============================================================================
PathPointComponent::PathPointComponent (PaintElementPath* const path_,
                                        const int index_,
                                        const int pointNumber_)
    : ElementSiblingComponent (path_),
      path (path_),
      routine (path_->getOwner()),
      index (index_),
      pointNumber (pointNumber_),
      selected (false)
{
    setSize (11, 11);
    setRepaintsOnMouseActivity (true);

    selected = routine->getSelectedPoints().isSelected (path_->points [index]);
    routine->getSelectedPoints().addChangeListener (this);
}

PathPointComponent::~PathPointComponent()
{
    routine->getSelectedPoints().removeChangeListener (this);
}

void PathPointComponent::updatePosition()
{
    const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
    jassert (getParentComponent() != nullptr);

    double x, y;
    path->getPoint (index, pointNumber, x, y, area);

    setCentrePosition (roundToInt (x),
                       roundToInt (y));
}

void PathPointComponent::showPopupMenu()
{
}

void PathPointComponent::paint (Graphics& g)
{
    if (isMouseOverOrDragging())
        g.fillAll (Colours::red);

    if (selected)
    {
        g.setColour (Colours::red);
        g.drawRect (getLocalBounds());
    }

    g.setColour (Colours::white);
    g.fillRect (getWidth() / 2 - 3, getHeight() / 2 - 3, 7, 7);

    g.setColour (Colours::black);

    if (pointNumber < path->getPoint (index)->getNumPoints() - 1)
        g.drawRect (getWidth() / 2 - 2, getHeight() / 2 - 2, 5, 5);
    else
        g.fillRect (getWidth() / 2 - 2, getHeight() / 2 - 2, 5, 5);
}

void PathPointComponent::mouseDown (const MouseEvent& e)
{
    dragging = false;

    if (e.mods.isPopupMenu())
    {
        showPopupMenu();
        return; // this may be deleted now..
    }

    dragX = getX() + getWidth() / 2;
    dragY = getY() + getHeight() / 2;

    mouseDownSelectStatus = routine->getSelectedPoints().addToSelectionOnMouseDown (path->points [index], e.mods);

    owner->getDocument()->beginTransaction();
}

void PathPointComponent::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
    {
        if (selected && ! dragging)
            dragging = e.mouseWasDraggedSinceMouseDown();

        if (dragging)
        {
            const Rectangle<int> area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
            int x = dragX + e.getDistanceFromDragStartX() - area.getX();
            int y = dragY + e.getDistanceFromDragStartY() - area.getY();

            if (JucerDocument* const document = owner->getDocument())
            {
                x = document->snapPosition (x);
                y = document->snapPosition (y);
            }

            owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();
            path->movePoint (index, pointNumber, x + area.getX(), y + area.getY(), area, true);
        }
    }
}

void PathPointComponent::mouseUp (const MouseEvent& e)
{
    routine->getSelectedPoints().addToSelectionOnMouseUp (path->points [index],
                                                          e.mods, dragging,
                                                          mouseDownSelectStatus);
}

void PathPointComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    ElementSiblingComponent::changeListenerCallback (source);

    const bool nowSelected = routine->getSelectedPoints().isSelected (path->points [index]);

    if (nowSelected != selected)
    {
        selected = nowSelected;
        repaint();

        if (Component* parent = getParentComponent())
            parent->repaint();
    }
}
