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

#ifndef __JUCER_PAINTELEMENTELLIPSE_JUCEHEADER__
#define __JUCER_PAINTELEMENTELLIPSE_JUCEHEADER__

#include "jucer_ColouredElement.h"


//==============================================================================
/**
*/
class PaintElementEllipse   : public ColouredElement
{
public:
    //==============================================================================
    PaintElementEllipse (PaintRoutine* owner)
        : ColouredElement (owner, T("Ellipse"), true, false)
    {
    }

    ~PaintElementEllipse() {}

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
    {
        fillType.setFillType (g, getDocument(), parentArea);

        Rectangle r (position.getRectangle (parentArea, layout));
        g.fillEllipse ((float) r.getX(), (float) r.getY(), (float) r.getWidth(), (float) r.getHeight());

        if (isStrokePresent)
        {
            strokeType.fill.setFillType (g, getDocument(), parentArea);

            g.drawEllipse ((float) r.getX(), (float) r.getY(), (float) r.getWidth(), (float) r.getHeight(),
                           getStrokeType().stroke.getStrokeThickness());
        }
    }

    void getEditableProperties (Array <PropertyComponent*>& properties)
    {
        ColouredElement::getEditableProperties (properties);
        properties.add (new ShapeToPathProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        if (! fillType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            fillType.fillInGeneratedCode (code, paintMethodCode);
            s << "g.fillEllipse ("
              << castToFloat (x) << ", "
              << castToFloat (y) << ", "
              << castToFloat (w) << ", "
              << castToFloat (h) << ");\n\n";

            paintMethodCode += s;
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            strokeType.fill.fillInGeneratedCode (code, paintMethodCode);
            s << "g.drawEllipse ("
              << castToFloat (x) << ", "
              << castToFloat (y) << ", "
              << castToFloat (w) << ", "
              << castToFloat (h) << ", "
              << valueToFloat (strokeType.stroke.getStrokeThickness()) << ");\n\n";

            paintMethodCode += s;
        }
    }

    static const tchar* getTagName() throw()        { return T("ELLIPSE"); }

    XmlElement* createXml() const
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        addColourAttributes (e);

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            loadColourAttributes (xml);

            return true;
        }
        else
        {
            jassertfalse
            return false;
        }
    }

    void convertToPath()
    {
        double x, y, w, h;
        getCurrentAbsoluteBoundsDouble (x, y, w, h);

        Path path;
        path.addEllipse ((float) x, (float) y, (float) w, (float) h);

        convertToNewPathElement (path);
    }


    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementEllipse* const element_)
            : ButtonPropertyComponent (T("path"), false),
              element (element_)
        {
        }

        void buttonClicked()
        {
            element->convertToPath();
        }

        const String getButtonText() const
        {
            return T("convert to a path");
        }

    private:
        PaintElementEllipse* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTELLIPSE_JUCEHEADER__
