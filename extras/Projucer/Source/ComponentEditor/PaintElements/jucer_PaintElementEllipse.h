/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
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

    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ColouredElement::getEditableProperties (props, multipleSelected);
        props.add (new ShapeToPathProperty (this));
    }

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) override
    {
        if (fillType.isInvisible() && (strokeType.isInvisible() || ! isStrokePresent))
            return;

        String x, y, w, h, s;
        positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

        s << "{\n"
          << "    float x = " << castToFloat (x) << ", y = " << castToFloat (y) << ", "
          <<           "width = " << castToFloat (w) << ", height = " << castToFloat (h) << ";\n";

        if (! fillType.isInvisible())
            s << "    " << fillType.generateVariablesCode ("fill");

        if (isStrokePresent && ! strokeType.isInvisible())
            s << "    " << strokeType.fill.generateVariablesCode ("stroke");

        s << "    //[UserPaintCustomArguments] Customize the painting arguments here..\n"
          << customPaintCode
          << "    //[/UserPaintCustomArguments]\n";

        if (! fillType.isInvisible())
        {
            s << "    ";
            fillType.fillInGeneratedCode ("fill", position, code, s);
            s << "    g.fillEllipse (x, y, width, height);\n";
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            s << "    ";
            strokeType.fill.fillInGeneratedCode ("stroke", position, code, s);
            s << "    g.drawEllipse (x, y, width, height, " << CodeHelpers::floatLiteral (strokeType.stroke.getStrokeThickness(), 3) << ");\n";
        }

        s << "}\n\n";

        paintMethodCode += s;
    }

    void applyCustomPaintSnippets (StringArray& snippets) override
    {
        customPaintCode.clear();

        if (! snippets.isEmpty() && (! fillType.isInvisible() || (isStrokePresent && ! strokeType.isInvisible())))
        {
            customPaintCode = snippets[0];
            snippets.remove (0);
        }
    }

    static const char* getTagName() noexcept        { return "ELLIPSE"; }

    XmlElement* createXml() const override
    {
        XmlElement* e = new XmlElement (getTagName());
        position.applyToXml (*e);
        addColourAttributes (e);

        return e;
    }

    bool loadFromXml (const XmlElement& xml) override
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
    String customPaintCode;

    //==============================================================================
    struct ShapeToPathProperty  : public ButtonPropertyComponent
    {
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

        PaintElementEllipse* element;
    };
};
