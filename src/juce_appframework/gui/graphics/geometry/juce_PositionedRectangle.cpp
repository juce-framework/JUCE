/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PositionedRectangle.h"


//==============================================================================
PositionedRectangle::PositionedRectangle() throw()
    : x (0.0),
      y (0.0),
      w (0.0),
      h (0.0),
      xMode (anchorAtLeftOrTop | absoluteFromParentTopLeft),
      yMode (anchorAtLeftOrTop | absoluteFromParentTopLeft),
      wMode (absoluteSize),
      hMode (absoluteSize)
{
}

PositionedRectangle::PositionedRectangle (const PositionedRectangle& other) throw()
    : x (other.x),
      y (other.y),
      w (other.w),
      h (other.h),
      xMode (other.xMode),
      yMode (other.yMode),
      wMode (other.wMode),
      hMode (other.hMode)
{
}

const PositionedRectangle& PositionedRectangle::operator= (const PositionedRectangle& other) throw()
{
    if (this != &other)
    {
        x = other.x;
        y = other.y;
        w = other.w;
        h = other.h;
        xMode = other.xMode;
        yMode = other.yMode;
        wMode = other.wMode;
        hMode = other.hMode;
    }

    return *this;
}

PositionedRectangle::~PositionedRectangle() throw()
{
}

const bool PositionedRectangle::operator== (const PositionedRectangle& other) const throw()
{
    return x == other.x
        && y == other.y
        && w == other.w
        && h == other.h
        && xMode == other.xMode
        && yMode == other.yMode
        && wMode == other.wMode
        && hMode == other.hMode;
}

const bool PositionedRectangle::operator!= (const PositionedRectangle& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
PositionedRectangle::PositionedRectangle (const String& stringVersion) throw()
{
    StringArray tokens;
    tokens.addTokens (stringVersion, false);

    decodePosString (tokens [0], xMode, x);
    decodePosString (tokens [1], yMode, y);
    decodeSizeString (tokens [2], wMode, w);
    decodeSizeString (tokens [3], hMode, h);
}

const String PositionedRectangle::toString() const throw()
{
    String s;
    s.preallocateStorage (12);

    addPosDescription (s, xMode, x);
    s << T(' ');
    addPosDescription (s, yMode, y);
    s << T(' ');
    addSizeDescription (s, wMode, w);
    s << T(' ');
    addSizeDescription (s, hMode, h);

    return s;
}

//==============================================================================
const Rectangle PositionedRectangle::getRectangle (const Rectangle& target) const throw()
{
    jassert (! target.isEmpty());

    double x_, y_, w_, h_;
    applyPosAndSize (x_, w_, x, w, xMode, wMode, target.getX(), target.getWidth());
    applyPosAndSize (y_, h_, y, h, yMode, hMode, target.getY(), target.getHeight());

    return Rectangle (roundDoubleToInt (x_), roundDoubleToInt (y_),
                      roundDoubleToInt (w_), roundDoubleToInt (h_));
}

void PositionedRectangle::getRectangleDouble (const Rectangle& target,
                                              double& x_, double& y_,
                                              double& w_, double& h_) const throw()
{
    jassert (! target.isEmpty());

    applyPosAndSize (x_, w_, x, w, xMode, wMode, target.getX(), target.getWidth());
    applyPosAndSize (y_, h_, y, h, yMode, hMode, target.getY(), target.getHeight());
}

void PositionedRectangle::applyToComponent (Component& comp) const throw()
{
    comp.setBounds (getRectangle (Rectangle (0, 0, comp.getParentWidth(), comp.getParentHeight())));
}

//==============================================================================
void PositionedRectangle::updateFrom (const Rectangle& rectangle,
                                      const Rectangle& target) throw()
{
    updatePosAndSize (x, w, rectangle.getX(), rectangle.getWidth(), xMode, wMode, target.getX(), target.getWidth());
    updatePosAndSize (y, h, rectangle.getY(), rectangle.getHeight(), yMode, hMode, target.getY(), target.getHeight());
}

void PositionedRectangle::updateFromDouble (const double newX, const double newY,
                                            const double newW, const double newH,
                                            const Rectangle& target) throw()
{
    updatePosAndSize (x, w, newX, newW, xMode, wMode, target.getX(), target.getWidth());
    updatePosAndSize (y, h, newY, newH, yMode, hMode, target.getY(), target.getHeight());
}

void PositionedRectangle::updateFromComponent (const Component& comp) throw()
{
    if (comp.getParentComponent() == 0 && ! comp.isOnDesktop())
        updateFrom (comp.getBounds(), Rectangle());
    else
        updateFrom (comp.getBounds(), Rectangle (0, 0, comp.getParentWidth(), comp.getParentHeight()));
}

//==============================================================================
PositionedRectangle::AnchorPoint PositionedRectangle::getAnchorPointX() const throw()
{
    return (AnchorPoint) (xMode & (anchorAtLeftOrTop | anchorAtRightOrBottom | anchorAtCentre));
}

PositionedRectangle::PositionMode PositionedRectangle::getPositionModeX() const throw()
{
    return (PositionMode) (xMode & (absoluteFromParentTopLeft
                                     | absoluteFromParentBottomRight
                                     | absoluteFromParentCentre
                                     | proportionOfParentSize));
}

PositionedRectangle::AnchorPoint PositionedRectangle::getAnchorPointY() const throw()
{
    return (AnchorPoint) (yMode & (anchorAtLeftOrTop | anchorAtRightOrBottom | anchorAtCentre));
}

PositionedRectangle::PositionMode PositionedRectangle::getPositionModeY() const throw()
{
    return (PositionMode) (yMode & (absoluteFromParentTopLeft
                                     | absoluteFromParentBottomRight
                                     | absoluteFromParentCentre
                                     | proportionOfParentSize));
}

PositionedRectangle::SizeMode PositionedRectangle::getWidthMode() const throw()
{
    return (SizeMode) wMode;
}

PositionedRectangle::SizeMode PositionedRectangle::getHeightMode() const throw()
{
    return (SizeMode) hMode;
}

void PositionedRectangle::setModes (const AnchorPoint xAnchor,
                                    const PositionMode xMode_,
                                    const AnchorPoint yAnchor,
                                    const PositionMode yMode_,
                                    const SizeMode widthMode,
                                    const SizeMode heightMode,
                                    const Rectangle& target) throw()
{
    if (xMode != (xAnchor | xMode_) || wMode != widthMode)
    {
        double tx, tw;
        applyPosAndSize (tx, tw, x, w, xMode, wMode, target.getX(), target.getWidth());

        xMode = (uint8) (xAnchor | xMode_);
        wMode = (uint8) widthMode;

        updatePosAndSize (x, w, tx, tw, xMode, wMode, target.getX(), target.getWidth());
    }

    if (yMode != (yAnchor | yMode_) || hMode != heightMode)
    {
        double ty, th;
        applyPosAndSize (ty, th, y, h, yMode, hMode, target.getY(), target.getHeight());

        yMode = (uint8) (yAnchor | yMode_);
        hMode = (uint8) heightMode;

        updatePosAndSize (y, h, ty, th, yMode, hMode, target.getY(), target.getHeight());
    }
}

bool PositionedRectangle::isPositionAbsolute() const throw()
{
    return xMode == absoluteFromParentTopLeft
        && yMode == absoluteFromParentTopLeft
        && wMode == absoluteSize
        && hMode == absoluteSize;
}

//==============================================================================
void PositionedRectangle::addPosDescription (String& s, const uint8 mode, const double value) const throw()
{
    if ((mode & proportionOfParentSize) != 0)
    {
        s << (roundDoubleToInt (value * 100000.0) / 1000.0) << T('%');
    }
    else
    {
        s << (roundDoubleToInt (value * 100.0) / 100.0);

        if ((mode & absoluteFromParentBottomRight) != 0)
            s << T('R');
        else if ((mode & absoluteFromParentCentre) != 0)
            s << T('C');
    }

    if ((mode & anchorAtRightOrBottom) != 0)
        s << T('r');
    else if ((mode & anchorAtCentre) != 0)
        s << T('c');
}

void PositionedRectangle::addSizeDescription (String& s, const uint8 mode, const double value) const throw()
{
    if (mode == proportionalSize)
        s << (roundDoubleToInt (value * 100000.0) / 1000.0) << T('%');
    else if (mode == parentSizeMinusAbsolute)
        s << (roundDoubleToInt (value * 100.0) / 100.0) << T('M');
    else
        s << (roundDoubleToInt (value * 100.0) / 100.0);
}

void PositionedRectangle::decodePosString (const String& s, uint8& mode, double& value) throw()
{
    if (s.containsChar (T('r')))
        mode = anchorAtRightOrBottom;
    else if (s.containsChar (T('c')))
        mode = anchorAtCentre;
    else
        mode = anchorAtLeftOrTop;

    if (s.containsChar (T('%')))
    {
        mode |= proportionOfParentSize;
        value = s.removeCharacters (T("%rcRC")).getDoubleValue() / 100.0;
    }
    else
    {
        if (s.containsChar (T('R')))
            mode |= absoluteFromParentBottomRight;
        else if (s.containsChar (T('C')))
            mode |= absoluteFromParentCentre;
        else
            mode |= absoluteFromParentTopLeft;

        value = s.removeCharacters (T("rcRC")).getDoubleValue();
    }
}

void PositionedRectangle::decodeSizeString (const String& s, uint8& mode, double& value) throw()
{
    if (s.containsChar (T('%')))
    {
        mode = proportionalSize;
        value = s.upToFirstOccurrenceOf (T("%"), false, false).getDoubleValue() / 100.0;
    }
    else if (s.containsChar (T('M')))
    {
        mode = parentSizeMinusAbsolute;
        value = s.getDoubleValue();
    }
    else
    {
        mode = absoluteSize;
        value = s.getDoubleValue();
    }
}

void PositionedRectangle::applyPosAndSize (double& xOut, double& wOut,
                                           const double x, const double w,
                                           const uint8 xMode, const uint8 wMode,
                                           const int parentPos,
                                           const int parentSize) const throw()
{
    if (wMode == proportionalSize)
        wOut = roundDoubleToInt (w * parentSize);
    else if (wMode == parentSizeMinusAbsolute)
        wOut = jmax (0, parentSize - roundDoubleToInt (w));
    else
        wOut = roundDoubleToInt (w);

    if ((xMode & proportionOfParentSize) != 0)
        xOut = parentPos + x * parentSize;
    else if ((xMode & absoluteFromParentBottomRight) != 0)
        xOut = (parentPos + parentSize) - x;
    else if ((xMode & absoluteFromParentCentre) != 0)
        xOut = x + (parentPos + parentSize / 2);
    else
        xOut = x + parentPos;

    if ((xMode & anchorAtRightOrBottom) != 0)
        xOut -= wOut;
    else if ((xMode & anchorAtCentre) != 0)
        xOut -= wOut / 2;
}

void PositionedRectangle::updatePosAndSize (double& xOut, double& wOut,
                                            double x, const double w,
                                            const uint8 xMode, const uint8 wMode,
                                            const int parentPos,
                                            const int parentSize) const throw()
{
    if (wMode == proportionalSize)
    {
        if (parentSize > 0)
            wOut = w / parentSize;
    }
    else if (wMode == parentSizeMinusAbsolute)
        wOut = parentSize - w;
    else
        wOut = w;

    if ((xMode & anchorAtRightOrBottom) != 0)
        x += w;
    else if ((xMode & anchorAtCentre) != 0)
        x += w / 2;

    if ((xMode & proportionOfParentSize) != 0)
    {
        if (parentSize > 0)
            xOut = (x - parentPos) / parentSize;
    }
    else if ((xMode & absoluteFromParentBottomRight) != 0)
        xOut = (parentPos + parentSize) - x;
    else if ((xMode & absoluteFromParentCentre) != 0)
        xOut = x - (parentPos + parentSize / 2);
    else
        xOut = x - parentPos;
}

END_JUCE_NAMESPACE
