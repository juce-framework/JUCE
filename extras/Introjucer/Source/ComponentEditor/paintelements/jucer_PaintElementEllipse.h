/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_PAINTELEMENTELLIPSE_H_INCLUDED
#define JUCER_PAINTELEMENTELLIPSE_H_INCLUDED

#include "jucer_ColouredElement.h"


//==============================================================================
class PaintElementEllipse   : public ColouredElement
{
public:
    PaintElementEllipse (PaintRoutine* pr)
        : ColouredElement (pr, "Ellipse", true, false)
    {
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
    {
        fillType.setFillType (g, getDocument(), parentArea);

        Rectangle<int> r (position.getRectangle (parentArea, layout));
        g.fillEllipse ((float) r.getX(), (float) r.getY(), (float) r.getWidth(), (float) r.getHeight());

        if (isStrokePresent)
        {
            strokeType.fill.setFillType (g, getDocument(), parentArea);

            g.drawEllipse ((float) r.getX(), (float) r.getY(), (float) r.getWidth(), (float) r.getHeight(),
                           getStrokeType().stroke.getStrokeThickness());
        }
    }

    void getEditableProperties (Array<PropertyComponent*>& props)
    {
        ColouredElement::getEditableProperties (props);
        props.add (new ShapeToPathProperty (this));
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
              << CodeHelpers::floatLiteral (strokeType.stroke.getStrokeThickness(), 3) << ");\n\n";

            paintMethodCode += s;
        }
    }

    static const char* getTagName() noexcept        { return "ELLIPSE"; }

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

        jassertfalse;
        return false;
    }

    void convertToPath()
    {
        double x, y, w, h;
        getCurrentAbsoluteBoundsDouble (x, y, w, h);

        Path path;
        path.addEllipse ((float) x, (float) y, (float) w, (float) h);

        convertToNewPathElement (path);
    }

private:
    //==============================================================================
    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementEllipse* const e)
            : ButtonPropertyComponent ("path", false),
              element (e)
        {
        }

        void buttonClicked()
        {
            element->convertToPath();
        }

        String getButtonText() const
        {
            return "convert to a path";
        }

    private:
        PaintElementEllipse* const element;
    };
};


#endif   // JUCER_PAINTELEMENTELLIPSE_H_INCLUDED
