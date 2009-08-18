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

#include "../jucer_Headers.h"
#include "../model/jucer_JucerDocument.h"


//==============================================================================
const String replaceCEscapeChars (const String& s)
{
    const int len = s.length();

    String r;
    r.preallocateStorage (len + 2);
    bool lastWasHexEscapeCode = false;

    for (int i = 0; i < len; ++i)
    {
        const tchar c = s[i];

        switch (c)
        {
        case '\t':
            r << T("\\t");
            lastWasHexEscapeCode = false;
            break;
        case '\r':
            r << T("\\r");
            lastWasHexEscapeCode = false;
            break;
        case '\n':
            r <<  T("\\n");
            lastWasHexEscapeCode = false;
            break;
        case '\\':
            r << T("\\\\");
            lastWasHexEscapeCode = false;
            break;
        case '\'':
            r << T("\\\'");
            lastWasHexEscapeCode = false;
            break;
        case '\"':
            r << T("\\\"");
            lastWasHexEscapeCode = false;
            break;

        default:
            if (c < 128 &&
                 ! (lastWasHexEscapeCode
                     && String (T("0123456789abcdefABCDEF")).containsChar (c))) // (have to avoid following a hex escape sequence with a valid hex digit)
            {
                r << c;
                lastWasHexEscapeCode = false;
            }
            else
            {
                lastWasHexEscapeCode = true;
                r << T("\\x") << String::toHexString ((int) c);
            }

            break;
        }
    }

    return r;
}

const String quotedString (const String& s)
{
    if (s.isEmpty())
        return T("String::empty");

    const int embeddedIndex = s.indexOfIgnoreCase (T("%%"));

    if (embeddedIndex >= 0)
    {
        String s1 (s.substring (0, embeddedIndex));
        String s2 (s.substring (embeddedIndex + 2));
        String code;

        const int closeIndex = s2.indexOf (T("%%"));

        if (closeIndex > 0)
        {
            code = s2.substring (0, closeIndex).trim();
            s2 = s2.substring (closeIndex + 2);
        }

        if (code.isNotEmpty())
        {
            String result;

            if (s1.isNotEmpty())
                result << quotedString (s1) << T(" + ");

            result << code;

            if (s2.isNotEmpty())
                result << T(" + ") << quotedString (s2);

            return result;
        }
    }

    return T("T(\"") + replaceCEscapeChars (s) + T("\")");
}

const String replaceStringTranslations (String s, JucerDocument* document)
{
    s = s.replace (T("%%getName()%%"), document->getComponentName());
    s = s.replace (T("%%getButtonText()%%"), document->getComponentName());

    return s;
}

const String castToFloat (const String& expression)
{
    if (expression.containsOnly (T("0123456789.f")))
    {
        String s (expression.getFloatValue());

        if (s.containsChar (T('.')))
            return s + T("f");

        return s + T(".0f");
    }

    return T("(float) (") + expression + T(")");
}

const String indentCode (const String& code, const int numSpaces)
{
    if (numSpaces == 0)
        return code;

    const String space (String::repeatedString (T(" "), numSpaces));

    StringArray lines;
    lines.addLines (code);

    for (int i = 1; i < lines.size(); ++i)
    {
        String s (lines[i].trimEnd());
        if (s.isNotEmpty())
            s = space + s;

        lines.set (i, s);
    }

    return lines.joinIntoString (T("\n"));
}

//==============================================================================
const String makeValidCppIdentifier (String s,
                                     const bool capitalise,
                                     const bool removeColons,
                                     const bool allowTemplates)
{
    if (removeColons)
        s = s.replaceCharacters (T(".,;:/@"), T("______"));
    else
        s = s.replaceCharacters (T(".,;/@"), T("_____"));

    int i;
    for (i = s.length(); --i > 0;)
        if (CharacterFunctions::isLetter (s[i])
             && CharacterFunctions::isLetter (s[i - 1])
             && CharacterFunctions::isUpperCase (s[i])
             && ! CharacterFunctions::isUpperCase (s[i - 1]))
            s = s.substring (0, i) + T(" ") + s.substring (i);

    String allowedChars (T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_ 0123456789"));
    if (allowTemplates)
        allowedChars += T("<>");

    if (! removeColons)
        allowedChars += T(":");

    StringArray words;
    words.addTokens (s.retainCharacters (allowedChars), false);
    words.trim();

    String n (words[0]);

    if (capitalise)
        n = n.toLowerCase();

    for (i = 1; i < words.size(); ++i)
    {
        if (capitalise && words[i].length() > 1)
            n << words[i].substring (0, 1).toUpperCase()
              << words[i].substring (1).toLowerCase();
        else
            n << words[i];
    }

    if (CharacterFunctions::isDigit (n[0]))
        n = T("_") + n;

    // make sure it's not a reserved c++ keyword..
    static const tchar* const reservedWords[] =
    {
        T("auto"), T("const"), T("double"), T("float"), T("int"), T("short"), T("struct"),
        T("return"), T("static"), T("union"), T("while"), T("asm"), T("dynamic_cast"),
        T("unsigned"), T("break"), T("continue"), T("else"), T("for"), T("long"), T("signed"),
        T("switch"), T("void"), T("case"), T("default"), T("enum"), T("goto"), T("register"),
        T("sizeof"), T("typedef"), T("volatile"), T("char"), T("do"), T("extern"), T("if"),
        T("namespace"), T("reinterpret_cast"), T("try"), T("bool"), T("explicit"), T("new"),
        T("static_cast"), T("typeid"), T("catch"), T("false"), T("operator"), T("template"),
        T("typename"), T("class"), T("friend"), T("private"), T("this"), T("using"), T("const_cast"),
        T("inline"), T("public"), T("throw"), T("virtual"), T("delete"), T("mutable"), T("protected"),
        T("true"), T("wchar_t"), T("and"), T("bitand"), T("compl"), T("not_eq"), T("or_eq"),
        T("xor_eq"), T("and_eq"), T("bitor"), T("not"), T("or"), T("xor"), T("cin"), T("endl"),
        T("INT_MIN"), T("iomanip"), T("main"), T("npos"), T("std"), T("cout"), T("include"),
        T("INT_MAX"), T("iostream"), T("MAX_RAND"), T("NULL"), T("string")
    };

    for (i = 0; i < numElementsInArray (reservedWords); ++i)
        if (n == reservedWords[i])
            n << '_';

    return n;
}

//==============================================================================
int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex)
{
    startIndex = jmax (0, startIndex);

    while (startIndex < lines.size())
    {
        if (lines[startIndex].trimStart().startsWithIgnoreCase (text))
            return startIndex;

        ++startIndex;
    }

    return -1;
}

//==============================================================================
const String valueToFloat (const double v)
{
    String s ((double) (float) v, 4);

    if (s.containsChar (T('.')))
        s << 'f';
    else
        s << ".0f";

    return s;
}

const String boolToString (const bool b)
{
    return b ? T("true") : T("false");
}

//==============================================================================
const String colourToHex (const Colour& col)
{
    return String::toHexString ((int) col.getARGB());
}

//==============================================================================
const String colourToCode (const Colour& col)
{
    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL
    #define COL(col)  #col,
    static const char* colourNames[] =
    {
        #include "jucer_Colours.h"
        0
    };
    #undef COL

    for (int i = 0; i < numElementsInArray (colourNames) - 1; ++i)
        if (col == colours[i])
            return T("Colours::") + String (colourNames[i]);

    return T("Colour (0x") + colourToHex (col) + T(')');
}

void setColourXml (XmlElement& xml, const tchar* const attName, const Colour& colour)
{
    xml.setAttribute (attName, colourToHex (colour));
}

const Colour getColourXml (const XmlElement& xml, const tchar* const attName, const Colour& defaultColour)
{
    return Colour (xml.getStringAttribute (attName, colourToHex (defaultColour)).getHexValue32());
}

//==============================================================================
const String positionToString (const RelativePositionedRectangle& pos)
{
    StringArray toks;
    toks.addTokens (pos.rect.toString(), false);

    return toks[0] + T(" ") + toks[1];
}

void positionToXY (const RelativePositionedRectangle& position,
                   double& x, double& y,
                   const Rectangle& parentArea,
                   const ComponentLayout* layout)
{
    double w, h;
    position.getRectangleDouble (x, y, w, h, parentArea, layout);
}

void positionToCode (const RelativePositionedRectangle& position,
                     const ComponentLayout* layout,
                     String& x, String& y, String& w, String& h)
{
    // these are the code sections for the positions of the relative comps
    String xrx, xry, xrw, xrh;
    Component* const relCompX = layout != 0 ? layout->findComponentWithId (position.relativeToX) : 0;
    if (relCompX != 0)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompX), layout, xrx, xry, xrw, xrh);

    String yrx, yry, yrw, yrh;
    Component* const relCompY = layout != 0 ? layout->findComponentWithId (position.relativeToY) : 0;
    if (relCompY != 0)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompY), layout, yrx, yry, yrw, yrh);

    String wrx, wry, wrw, wrh;
    Component* const relCompW = (layout != 0 && position.rect.getWidthMode() != PositionedRectangle::absoluteSize)
                                    ? layout->findComponentWithId (position.relativeToW) : 0;
    if (relCompW != 0)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompW), layout, wrx, wry, wrw, wrh);

    String hrx, hry, hrw, hrh;
    Component* const relCompH = (layout != 0 && position.rect.getHeightMode() != PositionedRectangle::absoluteSize)
                                    ? layout->findComponentWithId (position.relativeToH) : 0;
    if (relCompH != 0)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompH), layout, hrx, hry, hrw, hrh);

    // width
    if (position.rect.getWidthMode() == PositionedRectangle::proportionalSize)
    {
        if (wrw.isNotEmpty())
            w << "roundFloatToInt ((" << wrw << ") * " << valueToFloat (position.rect.getWidth()) << ")";
        else
            w << "proportionOfWidth (" << valueToFloat (position.rect.getWidth()) << ")";
    }
    else if (position.rect.getWidthMode() == PositionedRectangle::parentSizeMinusAbsolute)
    {
        if (wrw.isNotEmpty())
            w << "(" << wrw << ") - " << roundDoubleToInt (position.rect.getWidth());
        else
            w << "getWidth() - " << roundDoubleToInt (position.rect.getWidth());
    }
    else
    {
        if (wrw.isNotEmpty())
            w << "(" << wrw << ") + ";

        w << roundDoubleToInt (position.rect.getWidth());
    }

    // height
    if (position.rect.getHeightMode() == PositionedRectangle::proportionalSize)
    {
        if (hrh.isNotEmpty())
            h << "roundFloatToInt ((" << hrh << ") * " << valueToFloat (position.rect.getHeight()) << ")";
        else
            h << "proportionOfHeight (" << valueToFloat (position.rect.getHeight()) << ")";
    }
    else if (position.rect.getHeightMode() == PositionedRectangle::parentSizeMinusAbsolute)
    {
        if (hrh.isNotEmpty())
            h << "(" << hrh << ") - " << roundDoubleToInt (position.rect.getHeight());
        else
            h << "getHeight() - " << roundDoubleToInt (position.rect.getHeight());
    }
    else
    {
        if (hrh.isNotEmpty())
            h << "(" << hrh << ") + ";

        h << roundDoubleToInt (position.rect.getHeight());
    }

    // x-pos
    if (position.rect.getPositionModeX() == PositionedRectangle::proportionOfParentSize)
    {
        if (xrx.isNotEmpty() && xrw.isNotEmpty())
            x << "(" << xrx << ") + roundFloatToInt ((" << xrw << ") * " << valueToFloat (position.rect.getX()) << ")";
        else
            x << "proportionOfWidth (" << valueToFloat (position.rect.getX()) << ")";
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentTopLeft)
    {
        if (xrx.isNotEmpty())
            x << "(" << xrx << ") + ";

        x << roundDoubleToInt (position.rect.getX());
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentBottomRight)
    {
        if (xrx.isNotEmpty())
            x << "(" << xrx << ") + (" << xrw << ")";
        else
            x << "getWidth()";

        const int d = roundDoubleToInt (position.rect.getX());
        if (d != 0)
            x << " - " << d;
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentCentre)
    {
        if (xrx.isNotEmpty())
            x << "(" << xrx << ") + (" << xrw << ") / 2";
        else
            x << "(getWidth() / 2)";

        const int d = roundDoubleToInt (position.rect.getX());
        if (d != 0)
            x << " + " << d;
    }

    if (w != T("0"))
    {
        if (position.rect.getAnchorPointX() == PositionedRectangle::anchorAtRightOrBottom)
            x << " - " << w;
        else if (position.rect.getAnchorPointX() == PositionedRectangle::anchorAtCentre)
            x << " - ((" << w << ") / 2)";
    }

    // y-pos
    if (position.rect.getPositionModeY() == PositionedRectangle::proportionOfParentSize)
    {
        if (yry.isNotEmpty() && yrh.isNotEmpty())
            y << "(" << yry << ") + roundFloatToInt ((" << yrh << ") * " << valueToFloat (position.rect.getY()) << ")";
        else
            y << "proportionOfHeight (" << valueToFloat (position.rect.getY()) << ")";
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentTopLeft)
    {
        if (yry.isNotEmpty())
            y << "(" << yry << ") + ";

        y << roundDoubleToInt (position.rect.getY());
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentBottomRight)
    {
        if (yry.isNotEmpty())
            y << "(" << yry << ") + (" << yrh << ")";
        else
            y << "getHeight()";

        const int d = roundDoubleToInt (position.rect.getY());
        if (d != 0)
            y << " - " << d;
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentCentre)
    {
        if (yry.isNotEmpty())
            y << "(" << yry << ") + (" << yrh << ") / 2";
        else
            y << "(getHeight() / 2)";

        const int d = roundDoubleToInt (position.rect.getY());
        if (d != 0)
            y << " + " << d;
    }

    if (h != T("0"))
    {
        if (position.rect.getAnchorPointY() == PositionedRectangle::anchorAtRightOrBottom)
            y << " - " << h;
        else if (position.rect.getAnchorPointY() == PositionedRectangle::anchorAtCentre)
            y << " - ((" << h << ") / 2)";
    }
}

const String justificationToCode (const Justification& justification)
{
    switch (justification.getFlags())
    {
    case Justification::centred:
        return "Justification::centred";

    case Justification::centredLeft:
        return "Justification::centredLeft";

    case Justification::centredRight:
        return "Justification::centredRight";

    case Justification::centredTop:
        return "Justification::centredTop";

    case Justification::centredBottom:
        return "Justification::centredBottom";

    case Justification::topLeft:
        return "Justification::topLeft";

    case Justification::topRight:
        return "Justification::topRight";

    case Justification::bottomLeft:
        return "Justification::bottomLeft";

    case Justification::bottomRight:
        return "Justification::bottomRight";

    case Justification::left:
        return "Justification::left";

    case Justification::right:
        return "Justification::right";

    case Justification::horizontallyCentred:
        return "Justification::horizontallyCentred";

    case Justification::top:
        return "Justification::top";

    case Justification::bottom:
        return "Justification::bottom";

    case Justification::verticallyCentred:
        return "Justification::verticallyCentred";

    case Justification::horizontallyJustified:
        return "Justification::horizontallyJustified";

    default:
        jassertfalse
        break;
    }

    return T("Justification (") + String (justification.getFlags()) + T(")");
}

//==============================================================================
void drawResizableBorder (Graphics& g,
                          int w, int h,
                          const BorderSize borderSize,
                          const bool isMouseOver)
{
    g.setColour (Colours::orange.withAlpha (isMouseOver ? 0.4f : 0.3f));

    g.fillRect (0, 0, w, borderSize.getTop());
    g.fillRect (0, 0, borderSize.getLeft(), h);
    g.fillRect (0, h - borderSize.getBottom(), w, borderSize.getBottom());
    g.fillRect (w - borderSize.getRight(), 0, borderSize.getRight(), h);

    g.drawRect (borderSize.getLeft() - 1, borderSize.getTop() - 1,
                w - borderSize.getRight() - borderSize.getLeft() + 2,
                h - borderSize.getTop() - borderSize.getBottom() + 2);
}

void drawMouseOverCorners (Graphics& g, int w, int h)
{
    RectangleList r (Rectangle (0, 0, w, h));
    r.subtract (Rectangle (1, 1, w - 2, h - 2));

    const int size = jmin (w / 3, h / 3, 12);
    r.subtract (Rectangle (size, 0, w - size - size, h));
    r.subtract (Rectangle (0, size, w, h - size - size));

    g.setColour (Colours::darkgrey);

    for (int i = r.getNumRectangles(); --i >= 0;)
        g.fillRect (r.getRectangle (i));
}



//==============================================================================
RelativePositionedRectangle::RelativePositionedRectangle()
    : relativeToX (0),
      relativeToY (0),
      relativeToW (0),
      relativeToH (0)
{
}

RelativePositionedRectangle::RelativePositionedRectangle (const RelativePositionedRectangle& other)
    : rect (other.rect),
      relativeToX (other.relativeToX),
      relativeToY (other.relativeToY),
      relativeToW (other.relativeToW),
      relativeToH (other.relativeToH)
{
}

const RelativePositionedRectangle& RelativePositionedRectangle::operator= (const RelativePositionedRectangle& other)
{
    rect = other.rect;
    relativeToX = other.relativeToX;
    relativeToY = other.relativeToY;
    relativeToW = other.relativeToW;
    relativeToH = other.relativeToH;
    return *this;
}

RelativePositionedRectangle::~RelativePositionedRectangle()
{
}

//==============================================================================
bool RelativePositionedRectangle::operator== (const RelativePositionedRectangle& other) const throw()
{
    return rect == other.rect
        && relativeToX == other.relativeToX
        && relativeToY == other.relativeToY
        && relativeToW == other.relativeToW
        && relativeToH == other.relativeToH;
}

bool RelativePositionedRectangle::operator!= (const RelativePositionedRectangle& other) const throw()
{
    return ! operator== (other);
}

void RelativePositionedRectangle::getRelativeTargetBounds (const Rectangle& parentArea,
                                                           const ComponentLayout* layout,
                                                           int& x, int& xw, int& y, int& yh,
                                                           int& w, int& h) const
{
    Component* rx = 0;
    Component* ry = 0;
    Component* rw = 0;
    Component* rh = 0;

    if (layout != 0)
    {
        rx = layout->findComponentWithId (relativeToX);
        ry = layout->findComponentWithId (relativeToY);
        rw = layout->findComponentWithId (relativeToW);
        rh = layout->findComponentWithId (relativeToH);
    }

    x = parentArea.getX() + (rx != 0 ? rx->getX() : 0);
    y = parentArea.getY() + (ry != 0 ? ry->getY() : 0);
    w = rw != 0 ? rw->getWidth() : parentArea.getWidth();
    h = rh != 0 ? rh->getHeight() : parentArea.getHeight();
    xw = rx != 0 ? rx->getWidth() : parentArea.getWidth();
    yh = ry != 0 ? ry->getHeight() : parentArea.getHeight();
}

const Rectangle RelativePositionedRectangle::getRectangle (const Rectangle& parentArea,
                                                           const ComponentLayout* layout) const
{
    int x, xw, y, yh, w, h;
    getRelativeTargetBounds (parentArea, layout, x, xw, y, yh, w, h);

    const Rectangle xyRect ((xw <= 0 || yh <= 0) ? Rectangle()
                                                 : rect.getRectangle (Rectangle (x, y, xw, yh)));

    const Rectangle whRect ((w <= 0 || h <= 0) ? Rectangle()
                                               : rect.getRectangle (Rectangle (x, y, w, h)));

    return Rectangle (xyRect.getX(), xyRect.getY(),
                      whRect.getWidth(), whRect.getHeight());
}

void RelativePositionedRectangle::getRectangleDouble (double& x, double& y, double& w, double& h,
                                                      const Rectangle& parentArea,
                                                      const ComponentLayout* layout) const
{
    int rx, rxw, ry, ryh, rw, rh;
    getRelativeTargetBounds (parentArea, layout, rx, rxw, ry, ryh, rw, rh);

    double dummy1, dummy2;
    rect.getRectangleDouble (Rectangle (rx, ry, rxw, ryh), x, y, dummy1, dummy2);
    rect.getRectangleDouble (Rectangle (rx, ry, rw, rh), dummy1, dummy2, w, h);
}

void RelativePositionedRectangle::updateFromComponent (const Component& comp,
                                                       const ComponentLayout* layout)
{
    int x, xw, y, yh, w, h;
    getRelativeTargetBounds (Rectangle (0, 0, comp.getParentWidth(), comp.getParentHeight()),
                             layout, x, xw, y, yh, w, h);

    PositionedRectangle xyRect (rect), whRect (rect);
    xyRect.updateFrom (comp.getBounds(), Rectangle (x, y, xw, yh));
    whRect.updateFrom (comp.getBounds(), Rectangle (x, y, w, h));

    rect.setX (xyRect.getX());
    rect.setY (xyRect.getY());
    rect.setWidth (whRect.getWidth());
    rect.setHeight (whRect.getHeight());
}

void RelativePositionedRectangle::updateFrom (double newX, double newY, double newW, double newH,
                                              const Rectangle& parentArea, const ComponentLayout* layout)
{
    int x, xw, y, yh, w, h;
    getRelativeTargetBounds (parentArea, layout, x, xw, y, yh, w, h);

    PositionedRectangle xyRect (rect), whRect (rect);
    xyRect.updateFromDouble (newX, newY, newW, newH, Rectangle (x, y, xw, yh));
    whRect.updateFromDouble (newX, newY, newW, newH, Rectangle (x, y, w, h));

    rect.setX (xyRect.getX());
    rect.setY (xyRect.getY());
    rect.setWidth (whRect.getWidth());
    rect.setHeight (whRect.getHeight());
}

void RelativePositionedRectangle::applyToXml (XmlElement& e) const
{
    e.setAttribute (T("pos"), rect.toString());

    if (relativeToX != 0)
        e.setAttribute (T("posRelativeX"), String::toHexString (relativeToX));
    if (relativeToY != 0)
        e.setAttribute (T("posRelativeY"), String::toHexString (relativeToY));
    if (relativeToW != 0)
        e.setAttribute (T("posRelativeW"), String::toHexString (relativeToW));
    if (relativeToH != 0)
        e.setAttribute (T("posRelativeH"), String::toHexString (relativeToH));
}

void RelativePositionedRectangle::restoreFromXml (const XmlElement& e,
                                                  const RelativePositionedRectangle& defaultPos)
{
    rect = PositionedRectangle (e.getStringAttribute (T("pos"), defaultPos.rect.toString()));
    relativeToX = e.getStringAttribute (T("posRelativeX"), String::toHexString (defaultPos.relativeToX)).getHexValue64();
    relativeToY = e.getStringAttribute (T("posRelativeY"), String::toHexString (defaultPos.relativeToY)).getHexValue64();
    relativeToW = e.getStringAttribute (T("posRelativeW"), String::toHexString (defaultPos.relativeToW)).getHexValue64();
    relativeToH = e.getStringAttribute (T("posRelativeH"), String::toHexString (defaultPos.relativeToH)).getHexValue64();
}
