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
class PaintElementRectangle     : public ColouredElement
{
public:
    PaintElementRectangle (PaintRoutine* pr)
        : ColouredElement (pr, "Rectangle", true, false)
    {
    }

    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const override
    {
        return PaintElement::getCurrentBounds (parentArea); // bypass the ColouredElement implementation
    }

    void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& parentArea, const bool undoable) override
    {
        PaintElement::setCurrentBounds (newBounds, parentArea, undoable); // bypass the ColouredElement implementation
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
    {
        Component tempParentComp;
        tempParentComp.setBounds (parentArea);

        fillType.setFillType (g, getDocument(), parentArea);

        const Rectangle<int> r (position.getRectangle (parentArea, layout));
        g.fillRect (r);

        if (isStrokePresent)
        {
            strokeType.fill.setFillType (g, getDocument(), parentArea);

            g.drawRect (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                        roundToInt (getStrokeType().stroke.getStrokeThickness()));
        }
    }

    void getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected) override
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
          << "    int x = " << x << ", y = " << y << ", width = " << w << ", height = " << h << ";\n";

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
            s << "    g.fillRect (x, y, width, height);\n";
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            s << "    ";
            strokeType.fill.fillInGeneratedCode ("stroke", position, code, s);
            s << "    g.drawRect (x, y, width, height, " << roundToInt (strokeType.stroke.getStrokeThickness()) << ");\n\n";
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

    static const char* getTagName() noexcept        { return "RECT"; }

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
        Path path;
        path.addRectangle (getCurrentAbsoluteBounds());
        convertToNewPathElement (path);
    }

private:
    String customPaintCode;

    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementRectangle* const e)
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
        PaintElementRectangle* const element;
    };
};
