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

// Various useful functions, mainly for formatting c++ code.

class JucerDocument;
class ComponentLayout;

//==============================================================================
struct RelativePositionedRectangle
{
    //==============================================================================
    PositionedRectangle rect;
    int64 relativeToX;
    int64 relativeToY;
    int64 relativeToW;
    int64 relativeToH;

    //==============================================================================
    RelativePositionedRectangle();
    RelativePositionedRectangle (const RelativePositionedRectangle&);
    const RelativePositionedRectangle& operator= (const RelativePositionedRectangle&);
    ~RelativePositionedRectangle();

    //==============================================================================
    bool operator== (const RelativePositionedRectangle& other) const throw();
    bool operator!= (const RelativePositionedRectangle& other) const throw();

    const Rectangle getRectangle (const Rectangle& parentArea, const ComponentLayout* layout) const;
    void getRectangleDouble (double& x, double& y, double& w, double& h,
                             const Rectangle& parentArea, const ComponentLayout* layout) const;
    void updateFromComponent (const Component& comp, const ComponentLayout* layout);
    void updateFrom (double newX, double newY, double newW, double newH,
                     const Rectangle& parentArea, const ComponentLayout* layout);

    void applyToXml (XmlElement& e) const;
    void restoreFromXml (const XmlElement& e, const RelativePositionedRectangle& defaultPos);

    void getRelativeTargetBounds (const Rectangle& parentArea,
                                  const ComponentLayout* layout,
                                  int& x, int& xw, int& y, int& yh, int& w, int& h) const;
};

//==============================================================================
const String replaceCEscapeChars (const String& s);
const String makeValidCppIdentifier (String s,
                                     const bool capitalise,
                                     const bool removeColons,
                                     const bool allowTemplates);

const String quotedString (const String& s);
// replaces any recognised embedded strings like %%getName()%% ready for display on screen
const String replaceStringTranslations (String s, JucerDocument* document);

const String valueToFloat (const double v);
const String castToFloat (const String& expression);
const String boolToString (const bool b);
const String justificationToCode (const Justification& justification);

//==============================================================================
const String indentCode (const String& code, const int numSpaces);

int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

//==============================================================================
const String colourToHex (const Colour& col);
const String colourToCode (const Colour& col);

void setColourXml (XmlElement& xml, const tchar* const attName, const Colour& colour);
const Colour getColourXml (const XmlElement& xml, const tchar* const attName, const Colour& defaultColour);

//==============================================================================
const String positionToString (const RelativePositionedRectangle& pos);

void positionToXY (const RelativePositionedRectangle& position,
                   double& x, double& y,
                   const Rectangle& parentArea,
                   const ComponentLayout* layout);

void positionToCode (const RelativePositionedRectangle& position,
                     const ComponentLayout* layout,
                     String& x, String& y, String& w, String& h);

//==============================================================================
void drawResizableBorder (Graphics& g,
                          int w, int h,
                          const BorderSize borderSize,
                          const bool isMouseOver);

void drawMouseOverCorners (Graphics& g, int w, int h);
