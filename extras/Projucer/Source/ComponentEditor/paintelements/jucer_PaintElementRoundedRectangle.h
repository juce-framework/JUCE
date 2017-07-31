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
#include "../jucer_UtilityFunctions.h"

//==============================================================================
class PaintElementRoundedRectangle  : public ColouredElement
{
public:
    PaintElementRoundedRectangle (PaintRoutine* pr)
        : ColouredElement (pr, "Rounded Rectangle", true, false)
    {
        cornerSize = 10.0;
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea) override
    {
        double x, y, w, h;
        position.getRectangleDouble (x, y, w, h, parentArea, layout);

        fillType.setFillType (g, getDocument(), parentArea);
        g.fillRoundedRectangle ((float) x, (float) y, (float) w, (float) h, (float) cornerSize);

        if (isStrokePresent)
        {
            strokeType.fill.setFillType (g, getDocument(), parentArea);

            g.drawRoundedRectangle ((float) x, (float) y, (float) w, (float) h, (float) cornerSize,
                                    getStrokeType().stroke.getStrokeThickness());
        }
    }

    void getEditableProperties (Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        props.add (new CornerSizeProperty (this));

        ColouredElement::getEditableProperties (props, multipleSelected);

        props.add (new ShapeToPathProperty (this));
    }

    //==============================================================================
    class SetCornerSizeAction   : public PaintElementUndoableAction <PaintElementRoundedRectangle>
    {
    public:
        SetCornerSizeAction (PaintElementRoundedRectangle* const element, const double newSize_)
            : PaintElementUndoableAction <PaintElementRoundedRectangle> (element),
              newSize (newSize_)
        {
            oldSize = element->getCornerSize();
        }

        bool perform()
        {
            showCorrectTab();
            getElement()->setCornerSize (newSize, false);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getElement()->setCornerSize (oldSize, false);
            return true;
        }

    private:
        double newSize, oldSize;
    };

    void setCornerSize (const double newSize, const bool undoable)
    {
        if (newSize != cornerSize)
        {
            if (undoable)
            {
                perform (new SetCornerSizeAction (this, newSize),
                         "Change rounded rectangle corner size");
            }
            else
            {
                cornerSize = newSize;
                changed();
            }
        }
    }

    double getCornerSize() const noexcept                         { return cornerSize; }

    //==============================================================================
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
            s << "    g.fillRoundedRectangle (x, y, width, height, " << CodeHelpers::floatLiteral (cornerSize, 3)  << ");\n";
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            s << "    ";
            strokeType.fill.fillInGeneratedCode ("stroke", position, code, s);
            s << "    g.drawRoundedRectangle (x, y, width, height, " << CodeHelpers::floatLiteral (cornerSize, 3)
              << ", " << CodeHelpers::floatLiteral (strokeType.stroke.getStrokeThickness(), 3) << ");\n";
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

    static const char* getTagName() noexcept        { return "ROUNDRECT"; }

    XmlElement* createXml() const override
    {
        XmlElement* const e = new XmlElement (getTagName());

        position.applyToXml (*e);
        e->setAttribute ("cornerSize", cornerSize);
        addColourAttributes (e);

        return e;
    }

    bool loadFromXml (const XmlElement& xml) override
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            cornerSize = xml.getDoubleAttribute ("cornerSize", 10.0);
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
        path.addRoundedRectangle ((float) x, (float) y, (float) w, (float) h, (float) cornerSize);

        convertToNewPathElement (path);
    }

private:
    double cornerSize;
    String customPaintCode;

    //==============================================================================
    class CornerSizeProperty  : public SliderPropertyComponent,
                                public ChangeListener
    {
    public:
        CornerSizeProperty (PaintElementRoundedRectangle* const owner_)
            : SliderPropertyComponent ("corner size", 1.0, 200.0, 0.5, 0.4),
              owner (owner_)
        {
            owner->getDocument()->addChangeListener (this);
        }

        ~CornerSizeProperty()
        {
            owner->getDocument()->removeChangeListener (this);
        }

        void setValue (double newValue)
        {
            owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            owner->setCornerSize (newValue, true);
        }

        double getValue() const                 { return owner->getCornerSize(); }

        void changeListenerCallback (ChangeBroadcaster*)     { refresh(); }

    private:
        PaintElementRoundedRectangle* const owner;
    };

    //==============================================================================
    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementRoundedRectangle* const e)
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
        PaintElementRoundedRectangle* const element;
    };
};
