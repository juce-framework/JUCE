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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_RelativeCoordinate.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
namespace RelativeCoordinateHelpers
{
    static bool isOrigin (const String& name)
    {
        return name.isEmpty()
                || name == RelativeCoordinate::Strings::parentLeft
                || name == RelativeCoordinate::Strings::parentTop;
    }

    static const String getOriginAnchorName (const bool isHorizontal) throw()
    {
        return isHorizontal ? RelativeCoordinate::Strings::parentLeft
                            : RelativeCoordinate::Strings::parentTop;
    }

    static const String getExtentAnchorName (const bool isHorizontal) throw()
    {
        return isHorizontal ? RelativeCoordinate::Strings::parentRight
                            : RelativeCoordinate::Strings::parentBottom;
    }

    static const String getObjectName (const String& fullName)
    {
        return fullName.upToFirstOccurrenceOf (".", false, false);
    }

    static const String getEdgeName (const String& fullName)
    {
        return fullName.fromFirstOccurrenceOf (".", false, false);
    }

    static const RelativeCoordinate findCoordinate (const String& name, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
    {
        return nameFinder != 0 ? nameFinder->findNamedCoordinate (getObjectName (name), getEdgeName (name))
                               : RelativeCoordinate();
    }

    //==============================================================================
    struct RecursionException  : public std::runtime_error
    {
        RecursionException()   : std::runtime_error ("Recursive RelativeCoordinate expression")
        {
        }
    };

    //==============================================================================
    static void skipWhitespace (const String& s, int& i)
    {
        while (CharacterFunctions::isWhitespace (s[i]))
            ++i;
    }

    static void skipComma (const String& s, int& i)
    {
        skipWhitespace (s, i);
        if (s[i] == ',')
            ++i;
    }

    static const String readAnchorName (const String& s, int& i)
    {
        skipWhitespace (s, i);

        if (CharacterFunctions::isLetter (s[i]) || s[i] == '_')
        {
            int start = i;
            while (CharacterFunctions::isLetterOrDigit (s[i]) || s[i] == '_' || s[i] == '.')
                ++i;

            return s.substring (start, i);
        }

        return String::empty;
    }

    static double readNumber (const String& s, int& i)
    {
        skipWhitespace (s, i);

        int start = i;
        if (CharacterFunctions::isDigit (s[i]) || s[i] == '.' || s[i] == '-')
            ++i;

        while (CharacterFunctions::isDigit (s[i]) || s[i] == '.')
            ++i;

        if ((s[i] == 'e' || s[i] == 'E')
             && (CharacterFunctions::isDigit (s[i + 1])
                  || s[i + 1] == '-'
                  || s[i + 1] == '+'))
        {
            i += 2;

            while (CharacterFunctions::isDigit (s[i]))
                ++i;
        }

        const double value = s.substring (start, i).getDoubleValue();
        while (CharacterFunctions::isWhitespace (s[i]) || s[i] == ',')
            ++i;

        return value;
    }

    static const RelativeCoordinate readNextCoordinate (const String& s, int& i, const bool isHorizontal)
    {
        String anchor1 (readAnchorName (s, i));
        double value = 0;

        if (anchor1.isNotEmpty())
        {
            skipWhitespace (s, i);

            if (s[i] == '+')
                value = readNumber (s, ++i);
            else if (s[i] == '-')
                value = -readNumber (s, ++i);

            return RelativeCoordinate (value, anchor1);
        }
        else
        {
            value = readNumber (s, i);
            skipWhitespace (s, i);

            if (s[i] == '%')
            {
                value /= 100.0;
                skipWhitespace (s, ++i);
                String anchor2;

                if (s[i] == '*')
                {
                    anchor1 = readAnchorName (s, ++i);

                    if (anchor1.isEmpty())
                        anchor1 = getOriginAnchorName (isHorizontal);

                    skipWhitespace (s, i);

                    if (s[i] == '-' && s[i + 1] == '>')
                    {
                        i += 2;
                        anchor2 = readAnchorName (s, i);
                    }
                    else
                    {
                        anchor2 = anchor1;
                        anchor1 = getOriginAnchorName (isHorizontal);
                    }
                }
                else
                {
                    anchor1 = getOriginAnchorName (isHorizontal);
                    anchor2 = getExtentAnchorName (isHorizontal);
                }

                return RelativeCoordinate (value, anchor1, anchor2);
            }

            return RelativeCoordinate (value, isHorizontal);
        }
    }

    static const String limitedAccuracyString (const double n)
    {
        if (! (n < -0.001 || n > 0.001)) // to detect NaN and inf as well as for rounding
            return "0";

        return String (n, 3).trimCharactersAtEnd ("0").trimCharactersAtEnd (".");
    }

    static bool couldBeMistakenForPathCommand (const String& s)
    {
        switch (s[0])
        {
        case 'a':
        case 'm':
        case 'l':
        case 'z':
        case 'q':
        case 'c':
            return s[1] == 0 || CharacterFunctions::isWhitespace (s[1]);

        default:
            break;
        }

        return false;
    }
}

//==============================================================================
const String RelativeCoordinate::Strings::parent ("parent");
const String RelativeCoordinate::Strings::left ("left");
const String RelativeCoordinate::Strings::right ("right");
const String RelativeCoordinate::Strings::top ("top");
const String RelativeCoordinate::Strings::bottom ("bottom");
const String RelativeCoordinate::Strings::parentLeft ("parent.left");
const String RelativeCoordinate::Strings::parentTop ("parent.top");
const String RelativeCoordinate::Strings::parentRight ("parent.right");
const String RelativeCoordinate::Strings::parentBottom ("parent.bottom");

//==============================================================================
RelativeCoordinate::RelativeCoordinate()
    : value (0)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistanceFromOrigin, const bool horizontal_)
    : anchor1 (RelativeCoordinateHelpers::getOriginAnchorName (horizontal_)),
      value (absoluteDistanceFromOrigin)
{
}

RelativeCoordinate::RelativeCoordinate (const double absoluteDistance, const String& source)
    : anchor1 (source.trim()),
      value (absoluteDistance)
{
    jassert (anchor1.isNotEmpty());
}

RelativeCoordinate::RelativeCoordinate (const double relativeProportion, const String& pos1, const String& pos2)
    : anchor1 (pos1.trim()),
      anchor2 (pos2.trim()),
      value (relativeProportion)
{
    jassert (anchor1.isNotEmpty());
    jassert (anchor2.isNotEmpty());
}

RelativeCoordinate::RelativeCoordinate (const String& s, const bool isHorizontal)
    : value (0)
{
    int i = 0;
    *this = RelativeCoordinateHelpers::readNextCoordinate (s, i, isHorizontal);
}

RelativeCoordinate::~RelativeCoordinate()
{
}

bool RelativeCoordinate::operator== (const RelativeCoordinate& other) const throw()
{
    return value == other.value && anchor1 == other.anchor1 && anchor2 == other.anchor2;
}

bool RelativeCoordinate::operator!= (const RelativeCoordinate& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate1() const
{
    return RelativeCoordinate (0.0, anchor1);
}

const RelativeCoordinate RelativeCoordinate::getAnchorCoordinate2() const
{
    return RelativeCoordinate (0.0, anchor2);
}

double RelativeCoordinate::resolveAnchor (const String& anchorName, const NamedCoordinateFinder* nameFinder, int recursionCounter)
{
    if (RelativeCoordinateHelpers::isOrigin (anchorName))
        return 0.0;

    return RelativeCoordinateHelpers::findCoordinate (anchorName, nameFinder).resolve (nameFinder, recursionCounter + 1);
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder* nameFinder, int recursionCounter) const
{
    if (recursionCounter > 150)
    {
        jassertfalse
        throw RelativeCoordinateHelpers::RecursionException();
    }

    const double pos1 = resolveAnchor (anchor1, nameFinder, recursionCounter);

    return isProportional() ? pos1 + (resolveAnchor (anchor2, nameFinder, recursionCounter) - pos1) * value
                            : pos1 + value;
}

double RelativeCoordinate::resolve (const NamedCoordinateFinder* nameFinder) const
{
    try
    {
        return resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}

    return 0.0;
}

bool RelativeCoordinate::isRecursive (const NamedCoordinateFinder* nameFinder) const
{
    try
    {
        (void) resolve (nameFinder, 0);
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {
        return true;
    }

    return false;
}

void RelativeCoordinate::moveToAbsolute (double newPos, const NamedCoordinateFinder* nameFinder)
{
    try
    {
        const double pos1 = resolveAnchor (anchor1, nameFinder, 0);

        if (isProportional())
        {
            const double size = resolveAnchor (anchor2, nameFinder, 0) - pos1;

            if (size != 0)
                value = (newPos - pos1) / size;
        }
        else
        {
            value = newPos - pos1;
        }
    }
    catch (RelativeCoordinateHelpers::RecursionException&)
    {}
}

void RelativeCoordinate::toggleProportionality (const NamedCoordinateFinder* nameFinder, bool isHorizontal)
{
    const double oldValue = resolve (nameFinder);

    anchor1 = RelativeCoordinateHelpers::getOriginAnchorName (isHorizontal);
    anchor2 = isProportional() ? String::empty
                               : RelativeCoordinateHelpers::getExtentAnchorName (isHorizontal);

    moveToAbsolute (oldValue, nameFinder);
}

bool RelativeCoordinate::references (const String& coordName, const NamedCoordinateFinder* nameFinder) const
{
    using namespace RelativeCoordinateHelpers;

    if (isOrigin (anchor1) && ! isProportional())
        return isOrigin (coordName);

    return anchor1 == coordName
            || anchor2 == coordName
            || findCoordinate (anchor1, nameFinder).references (coordName, nameFinder)
            || (isProportional() && findCoordinate (anchor2, nameFinder).references (coordName, nameFinder));
}

bool RelativeCoordinate::isDynamic() const
{
    return anchor2.isNotEmpty() || ! RelativeCoordinateHelpers::isOrigin (anchor1);
}

//==============================================================================
const String RelativeCoordinate::toString() const
{
    using namespace RelativeCoordinateHelpers;

    if (isProportional())
    {
        const String percent (limitedAccuracyString (value * 100.0));

        if (isOrigin (anchor1))
        {
            if (anchor2 == "parent.right" || anchor2 == "parent.bottom")
                return percent + "%";
            else
                return percent + "% * " + anchor2;
        }
        else
            return percent + "% * " + anchor1 + " -> " + anchor2;
    }
    else
    {
        if (isOrigin (anchor1))
            return limitedAccuracyString (value);
        else if (value > 0)
            return anchor1 + " + " + limitedAccuracyString (value);
        else if (value < 0)
            return anchor1 + " - " + limitedAccuracyString (-value);
        else
            return anchor1;
    }
}

//==============================================================================
const double RelativeCoordinate::getEditableNumber() const
{
    return isProportional() ? value * 100.0 : value;
}

void RelativeCoordinate::setEditableNumber (const double newValue)
{
    value = isProportional() ? newValue / 100.0 : newValue;
}

//==============================================================================
void RelativeCoordinate::changeAnchor1 (const String& newAnchorName, const NamedCoordinateFinder* nameFinder)
{
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor1 = newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::changeAnchor2 (const String& newAnchorName, const NamedCoordinateFinder* nameFinder)
{
    jassert (isProportional());
    jassert (newAnchorName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_."));

    const double oldValue = resolve (nameFinder);
    anchor2 = newAnchorName;
    moveToAbsolute (oldValue, nameFinder);
}

void RelativeCoordinate::renameAnchorIfUsed (const String& oldName, const String& newName, const NamedCoordinateFinder* nameFinder)
{
    using namespace RelativeCoordinateHelpers;
    jassert (oldName.isNotEmpty());
    jassert (newName.toLowerCase().containsOnly ("abcdefghijklmnopqrstuvwxyz0123456789_"));

    if (newName.isEmpty())
    {
        if (getObjectName (anchor1) == oldName
             || getObjectName (anchor2) == oldName)
        {
            value = resolve (nameFinder);
            anchor1 = String::empty;
            anchor2 = String::empty;
        }
    }
    else
    {
        if (getObjectName (anchor1) == oldName)
            anchor1 = newName + "." + getEdgeName (anchor1);

        if (getObjectName (anchor2) == oldName)
            anchor2 = newName + "." + getEdgeName (anchor2);
    }
}

//==============================================================================
RelativePoint::RelativePoint()
    : x (0, true), y (0, false)
{
}

RelativePoint::RelativePoint (const Point<float>& absolutePoint)
    : x (absolutePoint.getX(), true), y (absolutePoint.getY(), false)
{
}

RelativePoint::RelativePoint (const RelativeCoordinate& x_, const RelativeCoordinate& y_)
    : x (x_), y (y_)
{
}

RelativePoint::RelativePoint (const String& s)
{
    int i = 0;
    x = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    y = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
}

bool RelativePoint::operator== (const RelativePoint& other) const throw()
{
    return x == other.x && y == other.y;
}

bool RelativePoint::operator!= (const RelativePoint& other) const throw()
{
    return ! operator== (other);
}

const Point<float> RelativePoint::resolve (const RelativeCoordinate::NamedCoordinateFinder* nameFinder) const
{
    return Point<float> ((float) x.resolve (nameFinder),
                         (float) y.resolve (nameFinder));
}

void RelativePoint::moveToAbsolute (const Point<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    x.moveToAbsolute (newPos.getX(), nameFinder);
    y.moveToAbsolute (newPos.getY(), nameFinder);
}

const String RelativePoint::toString() const
{
    return x.toString() + ", " + y.toString();
}

void RelativePoint::renameAnchorIfUsed (const String& oldName, const String& newName, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    x.renameAnchorIfUsed (oldName, newName, nameFinder);
    y.renameAnchorIfUsed (oldName, newName, nameFinder);
}

bool RelativePoint::isDynamic() const
{
    return x.isDynamic() || y.isDynamic();
}


//==============================================================================
RelativeRectangle::RelativeRectangle()
{
}

RelativeRectangle::RelativeRectangle (const Rectangle<float>& rect, const String& componentName)
    : left (rect.getX(), true),
      right (rect.getWidth(), componentName + "." + RelativeCoordinate::Strings::left),
      top (rect.getY(), false),
      bottom (rect.getHeight(), componentName + "." + RelativeCoordinate::Strings::top)
{
}

RelativeRectangle::RelativeRectangle (const String& s)
{
    int i = 0;
    left = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    top = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
    RelativeCoordinateHelpers::skipComma (s, i);
    right = RelativeCoordinateHelpers::readNextCoordinate (s, i, true);
    RelativeCoordinateHelpers::skipComma (s, i);
    bottom = RelativeCoordinateHelpers::readNextCoordinate (s, i, false);
}

bool RelativeRectangle::operator== (const RelativeRectangle& other) const throw()
{
    return left == other.left && top == other.top && right == other.right && bottom == other.bottom;
}

bool RelativeRectangle::operator!= (const RelativeRectangle& other) const throw()
{
    return ! operator== (other);
}

const Rectangle<float> RelativeRectangle::resolve (const RelativeCoordinate::NamedCoordinateFinder* nameFinder) const
{
    const double l = left.resolve (nameFinder);
    const double r = right.resolve (nameFinder);
    const double t = top.resolve (nameFinder);
    const double b = bottom.resolve (nameFinder);

    return Rectangle<float> ((float) l, (float) t, (float) (r - l), (float) (b - t));
}

void RelativeRectangle::moveToAbsolute (const Rectangle<float>& newPos, const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    left.moveToAbsolute (newPos.getX(), nameFinder);
    right.moveToAbsolute (newPos.getRight(), nameFinder);
    top.moveToAbsolute (newPos.getY(), nameFinder);
    bottom.moveToAbsolute (newPos.getBottom(), nameFinder);
}

const String RelativeRectangle::toString() const
{
    return left.toString() + ", " + top.toString() + ", " + right.toString() + ", " + bottom.toString();
}

void RelativeRectangle::renameAnchorIfUsed (const String& oldName, const String& newName,
                                               const RelativeCoordinate::NamedCoordinateFinder* nameFinder)
{
    left.renameAnchorIfUsed (oldName, newName, nameFinder);
    right.renameAnchorIfUsed (oldName, newName, nameFinder);
    top.renameAnchorIfUsed (oldName, newName, nameFinder);
    bottom.renameAnchorIfUsed (oldName, newName, nameFinder);
}


//==============================================================================
RelativePointPath::RelativePointPath()
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
}

RelativePointPath::RelativePointPath (const RelativePointPath& other)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    parseString (other.toString());
}

RelativePointPath::RelativePointPath (const String& s)
    : usesNonZeroWinding (true),
      containsDynamicPoints (false)
{
    parseString (s);
}

void RelativePointPath::parseString (const String& s)
{
    int i = 0;
    juce_wchar marker = 'm';
    int numValues = 2;
    RelativePoint points [3];

    for (;;)
    {
        RelativeCoordinateHelpers::skipWhitespace (s, i);
        const juce_wchar firstChar = s[i];

        if (firstChar == 0)
            break;

        const juce_wchar secondChar = s[i + 1];

        if (secondChar == 0 || CharacterFunctions::isWhitespace (secondChar))
        {
            if (firstChar == 'm' || firstChar == 'l')
            {
                ++i;
                marker = firstChar;
                numValues = 1;
            }
            else if (firstChar == 'q')
            {
                ++i;
                marker = firstChar;
                numValues = 2;
            }
            else if (firstChar == 'c')
            {
                ++i;
                marker = firstChar;
                numValues = 3;
            }
            else if (firstChar == 'z')
            {
                ++i;
                marker = 'm';
                numValues = 2;
                elements.add (new CloseSubPath());
                continue;
            }
            else if (firstChar == 'a')
            {
                ++i;
                usesNonZeroWinding = false;
                continue;
            }
        }

        if (firstChar == '#')
            ++i;

        for (int j = 0; j < numValues; ++j)
        {
            const RelativeCoordinate x (RelativeCoordinateHelpers::readNextCoordinate (s, i, true));
            const RelativeCoordinate y (RelativeCoordinateHelpers::readNextCoordinate (s, i, false));
            points[j] = RelativePoint (x, y);
            containsDynamicPoints = containsDynamicPoints || points[j].isDynamic();
        }

        switch (marker)
        {
            case 'm':   elements.add (new StartSubPath (points[0])); break;
            case 'l':   elements.add (new LineTo (points[0])); break;
            case 'q':   elements.add (new QuadraticTo (points[0], points[1])); break;
            case 'c':   elements.add (new CubicTo (points[0], points[1], points[2])); break;
            default: jassertfalse; break;  // illegal string format?
        }
    }
}

RelativePointPath::~RelativePointPath()
{
}

void RelativePointPath::swapWith (RelativePointPath& other) throw()
{
    elements.swapWithArray (other.elements);
    swapVariables (usesNonZeroWinding, other.usesNonZeroWinding);
}

void RelativePointPath::createPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder)
{
    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked(i)->addToPath (path, coordFinder);
}

bool RelativePointPath::containsAnyDynamicPoints() const
{
    return containsDynamicPoints;
}

const String RelativePointPath::toString() const
{
    ElementType lastType = nullElement;
    MemoryOutputStream out;

    if (! usesNonZeroWinding)
        out << 'a';

    for (int i = 0; i < elements.size(); ++i)
    {
        if (out.getDataSize() > 0)
            out << ' ';

        const ElementBase* const e = elements.getUnchecked(i);
        e->write (out, lastType);
        lastType = e->type;
    }

    return out.toUTF8();
}

//==============================================================================
RelativePointPath::ElementBase::ElementBase (const ElementType type_) : type (type_)
{
}

//==============================================================================
RelativePointPath::StartSubPath::StartSubPath (const RelativePoint& pos)
    : ElementBase (startSubPathElement), startPos (pos)
{
}

void RelativePointPath::StartSubPath::write (OutputStream& out, ElementType lastTypeWritten) const
{
    const String p (startPos.toString());

    if (lastTypeWritten != startSubPathElement)
        out << "m ";
    else if (RelativeCoordinateHelpers::couldBeMistakenForPathCommand (p))
        out << '#';

    out << p;
}

void RelativePointPath::StartSubPath::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    const Point<float> p (startPos.resolve (coordFinder));
    path.startNewSubPath (p.getX(), p.getY());
}

RelativePoint* RelativePointPath::StartSubPath::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &startPos;
}

//==============================================================================
RelativePointPath::CloseSubPath::CloseSubPath()
    : ElementBase (closeSubPathElement)
{
}

void RelativePointPath::CloseSubPath::write (OutputStream& out, ElementType lastTypeWritten) const
{
    if (lastTypeWritten != closeSubPathElement)
        out << 'z';
}

void RelativePointPath::CloseSubPath::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder*) const
{
    path.closeSubPath();
}

RelativePoint* RelativePointPath::CloseSubPath::getControlPoints (int& numPoints)
{
    numPoints = 0;
    return 0;
}

//==============================================================================
RelativePointPath::LineTo::LineTo (const RelativePoint& endPoint_)
    : ElementBase (lineToElement), endPoint (endPoint_)
{
}

void RelativePointPath::LineTo::write (OutputStream& out, ElementType lastTypeWritten) const
{
    const String p (endPoint.toString());

    if (lastTypeWritten != lineToElement)
        out << "l ";
    else if (RelativeCoordinateHelpers::couldBeMistakenForPathCommand (p))
        out << '#';

    out << p;
}

void RelativePointPath::LineTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    const Point<float> p (endPoint.resolve (coordFinder));
    path.lineTo (p.getX(), p.getY());
}

RelativePoint* RelativePointPath::LineTo::getControlPoints (int& numPoints)
{
    numPoints = 1;
    return &endPoint;
}

//==============================================================================
RelativePointPath::QuadraticTo::QuadraticTo (const RelativePoint& controlPoint, const RelativePoint& endPoint)
    : ElementBase (quadraticToElement)
{
    controlPoints[0] = controlPoint;
    controlPoints[1] = endPoint;
}

void RelativePointPath::QuadraticTo::write (OutputStream& out, ElementType lastTypeWritten) const
{
    const String p1 (controlPoints[0].toString());

    if (lastTypeWritten != quadraticToElement)
        out << "q ";
    else if (RelativeCoordinateHelpers::couldBeMistakenForPathCommand (p1))
        out << '#';

    out << p1 << ' ' << controlPoints[1].toString();
}

void RelativePointPath::QuadraticTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    const Point<float> p1 (controlPoints[0].resolve (coordFinder));
    const Point<float> p2 (controlPoints[1].resolve (coordFinder));
    path.quadraticTo (p1.getX(), p1.getY(), p2.getX(), p2.getY());
}

RelativePoint* RelativePointPath::QuadraticTo::getControlPoints (int& numPoints)
{
    numPoints = 2;
    return controlPoints;
}

//==============================================================================
RelativePointPath::CubicTo::CubicTo (const RelativePoint& controlPoint1, const RelativePoint& controlPoint2, const RelativePoint& endPoint)
    : ElementBase (cubicToElement)
{
    controlPoints[0] = controlPoint1;
    controlPoints[1] = controlPoint2;
    controlPoints[2] = endPoint;
}

void RelativePointPath::CubicTo::write (OutputStream& out, ElementType lastTypeWritten) const
{
    const String p1 (controlPoints[0].toString());

    if (lastTypeWritten != cubicToElement)
        out << "c ";
    else if (RelativeCoordinateHelpers::couldBeMistakenForPathCommand (p1))
        out << '#';

    out << p1 << ' ' << controlPoints[1].toString() << ' ' << controlPoints[2].toString();
}

void RelativePointPath::CubicTo::addToPath (Path& path, RelativeCoordinate::NamedCoordinateFinder* coordFinder) const
{
    const Point<float> p1 (controlPoints[0].resolve (coordFinder));
    const Point<float> p2 (controlPoints[1].resolve (coordFinder));
    const Point<float> p3 (controlPoints[2].resolve (coordFinder));
    path.cubicTo (p1.getX(), p1.getY(), p2.getX(), p2.getY(), p3.getX(), p3.getY());
}

RelativePoint* RelativePointPath::CubicTo::getControlPoints (int& numPoints)
{
    numPoints = 3;
    return controlPoints;
}


END_JUCE_NAMESPACE
