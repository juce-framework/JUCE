/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../jucer_Headers.h"
#include "jucer_PaintElementPath.h"
#include "../../properties/jucer_PositionPropertyBase.h"
#include "jucer_PaintElementUndoableAction.h"


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
        jassert (p != 0);
        return p;
    }

    bool changeTo (const PathPoint& value) const
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != 0);

        PathPoint* const p = path->getPoint (index);
        jassert (p != 0);

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
        : ChoicePropertyComponent (T("winding rule")),
          owner (owner_)
    {
        choices.add (T("Non-zero winding"));
        choices.add (T("Even/odd winding"));

        owner->getDocument()->addChangeListener (this);
    }

    ~PathWindingModeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    void setIndex (const int newIndex)      { owner->setNonZeroWinding (newIndex == 0, true); }
    int getIndex() const                    { return owner->isNonZeroWinding() ? 0 : 1; }

    void changeListenerCallback (void*)     { refresh(); }

private:
    PaintElementPath* const owner;
};


//==============================================================================
PaintElementPath::PaintElementPath (PaintRoutine* owner)
    : ColouredElement (owner, T("Path"), true, true),
      nonZeroWinding (true)
{
}

PaintElementPath::~PaintElementPath()
{
}

//==============================================================================
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

const Rectangle PaintElementPath::getCurrentBounds (const Rectangle& parentArea) const
{
    updateStoredPath (getDocument()->getComponentLayout(), parentArea);

    float x, y, w, h;
    path.getBounds (x, y, w, h);

    const int borderSize = getBorderSize();

    return Rectangle ((int) x - borderSize,
                      (int) y - borderSize,
                      (int) w + borderSize * 2,
                      (int) h + borderSize * 2);
}

void PaintElementPath::setCurrentBounds (const Rectangle& b,
                                         const Rectangle& parentArea,
                                         const bool undoable)
{
    Rectangle newBounds (b);
    newBounds.setSize (jmax (1, newBounds.getWidth()),
                       jmax (1, newBounds.getHeight()));

    const Rectangle current (getCurrentBounds (parentArea));

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

            perform (new ChangePointAction (destPoint, i, p), T("Move path"));
        }
    }
}

void PaintElementPath::rescalePoint (RelativePositionedRectangle& pos, int dx, int dy,
                                     double scaleX, double scaleY,
                                     double scaleStartX, double scaleStartY,
                                     const Rectangle& parentArea) const
{
    double x, y, w, h;
    pos.getRectangleDouble (x, y, w, h, parentArea, getDocument()->getComponentLayout());

    x = (x - scaleStartX) * scaleX + scaleStartX + dx;
    y = (y - scaleStartY) * scaleY + scaleStartY + dy;

    pos.updateFrom (x, y, w, h, parentArea, getDocument()->getComponentLayout());
}

//==============================================================================
static void drawArrow (Graphics& g, float x1, float y1, float x2, float y2)
{
    g.drawArrow (x1, y1, (x1 + x2) * 0.5f, (y1 + y2) * 0.5f, 1.0f, 8.0f, 10.0f);
    g.drawLine (x1 + (x2 - x1) * 0.49f, y1 + (y2 - y1) * 0.49f, x2, y2);
}

void PaintElementPath::draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
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

void PaintElementPath::drawExtraEditorGraphics (Graphics& g, const Rectangle& relativeTo)
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
                double x1, y1, x2, y2;

                if (numPoints > 2)
                {
                    positionToXY (p->pos [1], x1, y1, relativeTo, layout);
                    positionToXY (p->pos [2], x2, y2, relativeTo, layout);
                    drawArrow (g, (float) x1, (float) y1, (float) x2, (float) y2);
                }

                if (numPoints > 1)
                {
                    positionToXY (p->pos [0], x1, y1, relativeTo, layout);
                    positionToXY (p->pos [1], x2, y2, relativeTo, layout);
                    drawArrow (g, (float) x1, (float) y1, (float) x2, (float) y2);
                }

                positionToXY (p->pos [0], x2, y2, relativeTo, layout);

                const PathPoint* const nextPoint = points [i - 1];

                if (nextPoint != 0)
                {
                    positionToXY (nextPoint->pos [nextPoint->getNumPoints() - 1], x1, y1, relativeTo, layout);
                    drawArrow (g, (float) x1, (float) y1, (float) x2, (float) y2);
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

    if (points [mouseDownOnSegment] != 0)
    {
        mouseDownSelectSegmentStatus = owner->getSelectedPoints().addToSelectionOnMouseDown (points [mouseDownOnSegment], e.mods);
    }
    else
    {
        ColouredElement::mouseDown (e);
    }
}

void PaintElementPath::mouseDrag (const MouseEvent& e)
{
    if (mouseDownOnSegment < 0)
        ColouredElement::mouseDrag (e);
}

void PaintElementPath::mouseUp (const MouseEvent& e)
{
    if (points [mouseDownOnSegment] == 0)
    {
        ColouredElement::mouseUp (e);
    }
    else
    {
        owner->getSelectedPoints().addToSelectionOnMouseUp (points [mouseDownOnSegment],
                                                            e.mods, false, mouseDownSelectSegmentStatus);
    }
}

//==============================================================================
void PaintElementPath::changed()
{
    ColouredElement::changed();
    lastPathBounds = Rectangle();
}

void PaintElementPath::pointListChanged()
{
    changed();
    siblingComponentsChanged();
}

//==============================================================================
void PaintElementPath::getEditableProperties (Array <PropertyComponent*>& properties)
{
    properties.add (new PathWindingModeProperty (this));
    getColourSpecificProperties (properties);
}

//==============================================================================
static const String positionToPairOfValues (const RelativePositionedRectangle& position,
                                            const ComponentLayout* layout)
{
    String x, y, w, h;
    positionToCode (position, layout, x, y, w, h);
    return castToFloat (x) + T(", ") + castToFloat (y);
}

void PaintElementPath::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
{
    if (fillType.isInvisible() && (strokeType.isInvisible() || ! isStrokePresent))
        return;

    const String pathVariable (T("internalPath") + String (code.getUniqueSuffix()));

    const ComponentLayout* layout = code.document->getComponentLayout();

    code.privateMemberDeclarations
        << "Path " << pathVariable << ";\n";

    String r;
    bool somePointsAreRelative = false;

    if (! nonZeroWinding)
        r << pathVariable << ".setUsingNonZeroWinding (false);\n";

    for (int i = 0; i < points.size(); ++i)
    {
        const PathPoint* const p = points.getUnchecked(i);

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
            jassertfalse
            break;
        }
    }

    r << '\n';

    if (somePointsAreRelative)
        code.getCallbackCode (String::empty, T("void"), T("resized()"), false)
            << pathVariable << ".clear();\n" << r;
    else
        code.constructorCode << r;

    if (! fillType.isInvisible())
    {
        fillType.fillInGeneratedCode (code, paintMethodCode);

        paintMethodCode << "g.fillPath (" << pathVariable << ");\n";
    }

    if (isStrokePresent && ! strokeType.isInvisible())
    {
        String s;

        strokeType.fill.fillInGeneratedCode (code, s);
        s << "g.strokePath (" << pathVariable << ", " << strokeType.getPathStrokeCode() << ");\n";

        paintMethodCode += s;
    }

    paintMethodCode += "\n";
}

//==============================================================================
XmlElement* PaintElementPath::createXml() const
{
    XmlElement* e = new XmlElement (getTagName());
    position.applyToXml (*e);
    addColourAttributes (e);
    e->setAttribute (T("nonZeroWinding"), nonZeroWinding);

    e->addTextElement (pathToString());

    return e;
}

bool PaintElementPath::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (getTagName()))
    {
        position.restoreFromXml (xml, position);
        loadColourAttributes (xml);
        nonZeroWinding = xml.getBoolAttribute (T("nonZeroWinding"), true);

        restorePathFromString (xml.getAllSubText());

        return true;
    }
    else
    {
        jassertfalse
        return false;
    }
}

//==============================================================================
void PaintElementPath::createSiblingComponents()
{
    ColouredElement::createSiblingComponents();

    int i;
    for (i = 0; i < points.size(); ++i)
    {
        const PathPoint* const p = points.getUnchecked(i);

        switch (p->type)
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
            jassertfalse
            break;
        }
    }

    for (i = 0; i < siblingComponents.size(); ++i)
    {
        getParentComponent()->addAndMakeVisible (siblingComponents.getUnchecked(i));
        siblingComponents.getUnchecked(i)->updatePosition();
    }
}


const String PaintElementPath::pathToString() const
{
    String s;

    for (int i = 0; i < points.size(); ++i)
    {
        const PathPoint* const p = points.getUnchecked(i);

        switch (p->type)
        {
        case Path::Iterator::startNewSubPath:
            s << "s " << positionToString (p->pos [0]) << ' ';
            break;
        case Path::Iterator::lineTo:
            s << "l " << positionToString (p->pos [0]) << ' ';
            break;
        case Path::Iterator::quadraticTo:
            s << "q " << positionToString (p->pos [0])
              << ' ' << positionToString (p->pos [1]) << ' ';
            break;
        case Path::Iterator::cubicTo:
            s << "c " << positionToString (p->pos [0])
              << ' ' << positionToString (p->pos [1]) << ' '
              << ' ' << positionToString (p->pos [2]) << ' ';
            break;
        case Path::Iterator::closePath:
            s << "x ";
            break;
        default:
            jassertfalse
            break;
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
        PathPoint* p = new PathPoint (this);

        if (tokens[i] == T("s"))
        {
            p->type = Path::Iterator::startNewSubPath;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            i += 2;
        }
        else if (tokens[i] == T("l"))
        {
            p->type = Path::Iterator::lineTo;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            i += 2;
        }
        else if (tokens[i] == T("q"))
        {
            p->type = Path::Iterator::quadraticTo;
            p->pos [0] = RelativePositionedRectangle();
            p->pos [0].rect = PositionedRectangle (tokens [i + 1] + " " + tokens [i + 2]);
            p->pos [1] = RelativePositionedRectangle();
            p->pos [1].rect = PositionedRectangle (tokens [i + 3] + " " + tokens [i + 4]);
            i += 4;
        }
        else if (tokens[i] == T("c"))
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
        else if (tokens[i] == T("x"))
        {
            p->type = Path::Iterator::closePath;
        }
        else
        {
            delete p;
            continue;
        }

        points.add (p);
    }
}

void PaintElementPath::setToPath (const Path& p)
{
    points.clear();

    Path::Iterator i (p);

    while (i.next())
    {
        PathPoint* p = new PathPoint (this);
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
            delete p;
            continue;
        }

        points.add (p);
    }
}

void PaintElementPath::updateStoredPath (const ComponentLayout* layout, const Rectangle& relativeTo) const
{
    if (lastPathBounds != relativeTo && ! relativeTo.isEmpty())
    {
        lastPathBounds = relativeTo;

        path.clear();

        for (int i = 0; i < points.size(); ++i)
        {
            const PathPoint* const p = points.getUnchecked(i);
            double x1, y1, x2, y2, x3, y3;

            switch (p->type)
            {
            case Path::Iterator::startNewSubPath:
                positionToXY (p->pos [0], x1, y1, relativeTo, layout);
                path.startNewSubPath ((float) x1, (float) y1);
                break;

            case Path::Iterator::lineTo:
                positionToXY (p->pos [0], x1, y1, relativeTo, layout);
                path.lineTo ((float) x1, (float) y1);
                break;

            case Path::Iterator::quadraticTo:
                positionToXY (p->pos [0], x1, y1, relativeTo, layout);
                positionToXY (p->pos [1], x2, y2, relativeTo, layout);
                path.quadraticTo ((float) x1, (float) y1, (float) x2, (float) y2);
                break;

            case Path::Iterator::cubicTo:
                positionToXY (p->pos [0], x1, y1, relativeTo, layout);
                positionToXY (p->pos [1], x2, y2, relativeTo, layout);
                positionToXY (p->pos [2], x3, y3, relativeTo, layout);
                path.cubicTo ((float) x1, (float) y1, (float) x2, (float) y2, (float) x3, (float) y3);
                break;

            case Path::Iterator::closePath:
                path.closeSubPath();
                break;

            default:
                jassertfalse
                break;
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
            perform (new ChangeWindingAction (this, nonZero), T("Change path winding rule"));
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
        else if (points.getUnchecked (i)->type == Path::Iterator::startNewSubPath)
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
            else if (p->type == Path::Iterator::startNewSubPath)
            {
                jassert (closed);

                PathPoint* p = addPoint (i - 1, undoable);

                PathPoint p2 (*p);
                p2.type = Path::Iterator::closePath;
                perform (new ChangePointAction (p, p2), T("Close subpath"));
                return;
            }
        }

        jassert (closed);

        PathPoint* p = addPoint (points.size() - 1, undoable);
        PathPoint p2 (*p);
        p2.type = Path::Iterator::closePath;
        perform (new ChangePointAction (p, p2), T("Close subpath"));
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
        jassert (path != 0);

        PathPoint* const p = path->addPoint (pointIndexToAddItAfter, false);
        jassert (p != 0);

        indexAdded = path->indexOfPoint (p);
        jassert (indexAdded >= 0);
        return true;
    }

    bool undo()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != 0);

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
        perform (action, T("Add path point"));
        return points [action->indexAdded];
    }
    else
    {
        double x1 = 20.0, y1 = 20.0, x2, y2;

        ComponentLayout* layout = getDocument()->getComponentLayout();
        const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

        if (points [pointIndexToAddItAfter] != 0)
            positionToXY (points [pointIndexToAddItAfter]->pos [points [pointIndexToAddItAfter]->getNumPoints() - 1], x1, y1,
                          area, layout);
        else if (points[0] != 0)
            positionToXY (points[0]->pos [0], x1, y1,
                          area, layout);

        x2 = x1 + 50.0;
        y2 = y1 + 50.0;

        if (points [pointIndexToAddItAfter + 1] != 0)
        {
            if (points [pointIndexToAddItAfter + 1]->type == Path::Iterator::closePath
                 || points [pointIndexToAddItAfter + 1]->type == Path::Iterator::startNewSubPath)
            {
                int i = pointIndexToAddItAfter;
                while (i > 0)
                    if (points [--i]->type == Path::Iterator::startNewSubPath)
                        break;

                if (i != pointIndexToAddItAfter)
                    positionToXY (points [i]->pos [0], x2, y2,
                                  area, layout);
            }
            else
            {
                positionToXY (points [pointIndexToAddItAfter + 1]->pos [0], x2, y2,
                              area, layout);
            }
        }
        else
        {
            int i = pointIndexToAddItAfter + 1;
            while (i > 0)
                if (points [--i]->type == Path::Iterator::startNewSubPath)
                    break;

            positionToXY (points [i]->pos [0], x2, y2,
                          area, layout);
        }

        PathPoint* const p = new PathPoint (this);

        p->type = Path::Iterator::lineTo;
        p->pos[0].rect.setX ((x1 + x2) * 0.5f);
        p->pos[0].rect.setY ((y1 + y2) * 0.5f);

        points.insert (pointIndexToAddItAfter + 1, p);

        pointListChanged();

        return p;
    }
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
        jassert (path != 0);

        path->deletePoint (indexToRemove, false);
        return path != 0;
    }

    bool undo()
    {
        showCorrectTab();

        PaintElementPath* const path = getElement();
        jassert (path != 0);

        PathPoint* p = path->addPoint (indexToRemove - 1, false);
        *p = oldValue;

        return path != 0;
    }

    int indexToRemove;

private:
    PathPoint oldValue;
};

void PaintElementPath::deletePoint (int pointIndex, const bool undoable)
{
    if (undoable)
    {
        perform (new DeletePointAction (this, pointIndex), T("Delete path point"));
    }
    else
    {
        PathPoint* const p = points [pointIndex];

        if (p != 0 && pointIndex > 0)
        {
            owner->getSelectedPoints().deselect (p);
            owner->getSelectedPoints().changed (true);

            points.remove (pointIndex);
            pointListChanged();
        }
    }
}

//==============================================================================
bool PaintElementPath::getPoint (int index, int pointNumber, double& x, double& y, const Rectangle& parentArea) const
{
    const PathPoint* const p = points [index];

    if (p == 0)
        return false;

    jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
    jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

    positionToXY (p->pos [pointNumber], x, y, parentArea, getDocument()->getComponentLayout());
    return true;
}

int PaintElementPath::findSegmentAtXY (int x, int y) const
{
    double x1, y1, x2, y2, x3, y3, lastX = 0.0, lastY = 0.0, subPathStartX = 0.0, subPathStartY = 0.0;

    ComponentLayout* const layout = getDocument()->getComponentLayout();
    const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());

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
            positionToXY (p->pos [0], lastX, lastY, area, layout);
            subPathStartX = lastX;
            subPathStartY = lastY;
            subpathStartIndex = i;
            break;

        case Path::Iterator::lineTo:
            positionToXY (p->pos [0], x1, y1, area, layout);

            segmentPath.addLineSegment ((float) lastX, (float) lastY, (float) x1, (float) y1, thickness);
            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x1;
            lastY = y1;
            break;

        case Path::Iterator::quadraticTo:
            positionToXY (p->pos [0], x1, y1, area, layout);
            positionToXY (p->pos [1], x2, y2, area, layout);

            segmentPath.startNewSubPath ((float) lastX, (float) lastY);
            segmentPath.quadraticTo ((float) x1, (float) y1, (float) x2, (float) y2);
            PathStrokeType (thickness).createStrokedPath (segmentPath, segmentPath);

            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x2;
            lastY = y2;
            break;

        case Path::Iterator::cubicTo:
            positionToXY (p->pos [0], x1, y1, area, layout);
            positionToXY (p->pos [1], x2, y2, area, layout);
            positionToXY (p->pos [2], x3, y3, area, layout);

            segmentPath.startNewSubPath ((float) lastX, (float) lastY);
            segmentPath.cubicTo ((float) x1, (float) y1, (float) x2, (float) y2, (float) x3, (float) y3);
            PathStrokeType (thickness).createStrokedPath (segmentPath, segmentPath);

            if (segmentPath.contains ((float) x, (float) y))
                return i;

            lastX = x3;
            lastY = y3;
            break;

        case Path::Iterator::closePath:
            segmentPath.addLineSegment ((float) lastX, (float) lastY, (float) subPathStartX, (float) subPathStartY, thickness);
            if (segmentPath.contains ((float) x, (float) y))
                return subpathStartIndex;

            lastX = subPathStartX;
            lastY = subPathStartY;
            break;

        default:
            jassertfalse
            break;
        }
    }

    return -1;
}

//==============================================================================
void PaintElementPath::movePoint (int index, int pointNumber,
                                  double newX, double newY,
                                  const Rectangle& parentArea,
                                  const bool undoable)
{
    PathPoint* const p = points [index];

    if (p != 0)
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
            perform (new ChangePointAction (p, index, newPoint), T("Move path point"));
        }
        else
        {
            *p = newPoint;
            changed();
        }
    }
}

const RelativePositionedRectangle PaintElementPath::getPoint (int index, int pointNumber) const
{
    PathPoint* const p = points [index];

    if (p != 0)
    {
        jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
        jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

        return p->pos [pointNumber];
    }

    jassertfalse
    return RelativePositionedRectangle();
}

void PaintElementPath::setPoint (int index, int pointNumber, const RelativePositionedRectangle& newPos, const bool undoable)
{
    PathPoint* const p = points [index];

    if (p != 0)
    {
        PathPoint newPoint (*p);

        jassert (pointNumber < 3 || p->type == Path::Iterator::cubicTo);
        jassert (pointNumber < 2 || p->type == Path::Iterator::cubicTo || p->type == Path::Iterator::quadraticTo);

        if (newPoint.pos [pointNumber] != newPos)
        {
            newPoint.pos [pointNumber] = newPos;

            if (undoable)
            {
                perform (new ChangePointAction (p, index, newPoint), T("Change path point position"));
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
        jassertfalse
    }
}

//==============================================================================
class PathPointTypeProperty : public ChoicePropertyComponent,
                              public ChangeListener
{
public:
    PathPointTypeProperty (PaintElementPath* const owner_,
                           const int index_)
        : ChoicePropertyComponent (T("point type")),
          owner (owner_),
          index (index_)
    {
        choices.add (T("Start of sub-path"));
        choices.add (T("Line"));
        choices.add (T("Quadratic"));
        choices.add (T("Cubic"));

        owner->getDocument()->addChangeListener (this);
    }

    ~PathPointTypeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    void setIndex (const int newIndex)
    {
        Path::Iterator::PathElementType type = Path::Iterator::startNewSubPath;

        switch (newIndex)
        {
        case 0:
            type = Path::Iterator::startNewSubPath;
            break;

        case 1:
            type = Path::Iterator::lineTo;
            break;

        case 2:
            type = Path::Iterator::quadraticTo;
            break;

        case 3:
            type = Path::Iterator::cubicTo;
            break;

        default:
            jassertfalse
            break;
        }

        const Rectangle area (((PaintRoutineEditor*) owner->getParentComponent())->getComponentArea());

        owner->getPoint (index)->changePointType (type, area, true);
    }

    int getIndex() const
    {
        const PathPoint* const p = owner->getPoint (index);
        jassert (p != 0);

        switch (p->type)
        {
        case Path::Iterator::startNewSubPath:
            return 0;

        case Path::Iterator::lineTo:
            return 1;

        case Path::Iterator::quadraticTo:
            return 2;

        case Path::Iterator::cubicTo:
            return 3;

        case Path::Iterator::closePath:
            break;

        default:
            jassertfalse
            break;
        }

        return 0;
    }

    void changeListenerCallback (void*)
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

    //==============================================================================
    void setPosition (const RelativePositionedRectangle& newPos)
    {
        owner->setPoint (index, pointNumber, newPos, true);
    }

    const RelativePositionedRectangle getPosition() const
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
        : ChoicePropertyComponent (T("openness")),
          owner (owner_),
          index (index_)
    {
        owner->getDocument()->addChangeListener (this);

        choices.add (T("Subpath is closed"));
        choices.add (T("Subpath is open-ended"));
    }

    ~PathPointClosedProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

    void setIndex (const int newIndex)
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
        : ButtonPropertyComponent (T("new point"), false),
          owner (owner_),
          index (index_)
    {
    }

    ~AddNewPointProperty()  {}

    void buttonClicked()
    {
        owner->addPoint (index, true);
    }

    const String getButtonText() const      { return T("Add new point"); }

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

const PathPoint& PathPoint::operator= (const PathPoint& other)
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
    if (type == Path::Iterator::cubicTo)
        return 3;

    if (type == Path::Iterator::quadraticTo)
        return 2;

    if (type == Path::Iterator::closePath)
        return 0;

    return 1;
}

const PathPoint PathPoint::withChangedPointType (const Path::Iterator::PathElementType newType,
                                                 const Rectangle& parentArea) const
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
            PathPoint* lastPoint = owner->points [index - 1];

            jassert (lastPoint != 0)
            if (lastPoint != 0)
            {
                lastPoint->pos [lastPoint->getNumPoints() - 1]
                            .getRectangleDouble (lastX, lastY, w, h, parentArea, owner->getDocument()->getComponentLayout());
            }
            else
            {
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
                                 const Rectangle& parentArea, const bool undoable)
{
    if (newType != type)
    {
        if (undoable)
        {
            owner->perform (new ChangePointAction (this, withChangedPointType (newType, parentArea)),
                            T("Change path point type"));
        }
        else
        {
            *this = withChangedPointType (newType, parentArea);
            owner->pointListChanged();
        }
    }
}

void PathPoint::getEditableProperties (Array <PropertyComponent*>& properties)
{
    const int index = owner->points.indexOf (this);
    jassert (index >= 0);

    switch (type)
    {
    case Path::Iterator::startNewSubPath:
        properties.add (new PathPointPositionProperty (owner, index, 0, T("x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("y"), PositionPropertyBase::componentY));

        properties.add (new PathPointClosedProperty (owner, index));
        properties.add (new AddNewPointProperty (owner, index));
        break;

    case Path::Iterator::lineTo:
        properties.add (new PathPointTypeProperty (owner, index));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("y"), PositionPropertyBase::componentY));
        properties.add (new AddNewPointProperty (owner, index));
        break;

    case Path::Iterator::quadraticTo:
        properties.add (new PathPointTypeProperty (owner, index));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("control pt x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("control pt y"), PositionPropertyBase::componentY));
        properties.add (new PathPointPositionProperty (owner, index, 1, T("x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 1, T("y"), PositionPropertyBase::componentY));
        properties.add (new AddNewPointProperty (owner, index));
        break;

    case Path::Iterator::cubicTo:
        properties.add (new PathPointTypeProperty (owner, index));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("control pt1 x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 0, T("control pt1 y"), PositionPropertyBase::componentY));
        properties.add (new PathPointPositionProperty (owner, index, 1, T("control pt2 x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 1, T("control pt2 y"), PositionPropertyBase::componentY));
        properties.add (new PathPointPositionProperty (owner, index, 2, T("x"), PositionPropertyBase::componentX));
        properties.add (new PathPointPositionProperty (owner, index, 2, T("y"), PositionPropertyBase::componentY));
        properties.add (new AddNewPointProperty (owner, index));
        break;

    case Path::Iterator::closePath:
        break;

    default:
        jassertfalse
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
    const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
    jassert (getParentComponent() != 0);

    double x, y;
    path->getPoint (index, pointNumber, x, y, area);

    setCentrePosition (roundDoubleToInt (x),
                       roundDoubleToInt (y));
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
        g.drawRect (0, 0, getWidth(), getHeight());
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

    owner->getDocument()->getUndoManager().beginNewTransaction();
}

void PathPointComponent::mouseDrag (const MouseEvent& e)
{
    if (! e.mods.isPopupMenu())
    {
        if (selected && ! dragging)
            dragging = ! e.mouseWasClicked();

        if (dragging)
        {
            const Rectangle area (((PaintRoutineEditor*) getParentComponent())->getComponentArea());
            int x = dragX + e.getDistanceFromDragStartX() - area.getX();
            int y = dragY + e.getDistanceFromDragStartY() - area.getY();

            JucerDocument* const document = owner->getDocument();

            if (document != 0)
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

void PathPointComponent::changeListenerCallback (void* source)
{
    ElementSiblingComponent::changeListenerCallback (source);

    const bool nowSelected = routine->getSelectedPoints().isSelected (path->points [index]);

    if (nowSelected != selected)
    {
        selected = nowSelected;
        repaint();

        if (getParentComponent() != 0)
            getParentComponent()->repaint();
    }
}
