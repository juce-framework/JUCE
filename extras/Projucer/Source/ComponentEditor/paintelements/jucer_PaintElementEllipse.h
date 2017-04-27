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

#pragma once

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
