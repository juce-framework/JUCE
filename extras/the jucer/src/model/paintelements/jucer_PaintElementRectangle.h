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

#ifndef __JUCER_PAINTELEMENTRECTANGLE_JUCEHEADER__
#define __JUCER_PAINTELEMENTRECTANGLE_JUCEHEADER__

#include "jucer_ColouredElement.h"


//==============================================================================
/**
*/
class PaintElementRectangle     : public ColouredElement
{
public:
    //==============================================================================
    PaintElementRectangle (PaintRoutine* owner)
        : ColouredElement (owner, T("Rectangle"), true, false)
    {
    }

    ~PaintElementRectangle()
    {
    }

    const Rectangle getCurrentBounds (const Rectangle& parentArea) const
    {
        return PaintElement::getCurrentBounds (parentArea); // bypass the ColouredElement implementation
    }

    void setCurrentBounds (const Rectangle& newBounds, const Rectangle& parentArea, const bool undoable)
    {
        PaintElement::setCurrentBounds (newBounds, parentArea, undoable); // bypass the ColouredElement implementation
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
    {
        Component parentComponent;
        parentComponent.setBounds (parentArea);

        fillType.setFillType (g, getDocument(), parentArea);

        const Rectangle r (position.getRectangle (parentArea, layout));
        g.fillRect (r);

        if (isStrokePresent)
        {
            strokeType.fill.setFillType (g, getDocument(), parentArea);

            g.drawRect (r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                        roundDoubleToInt (getStrokeType().stroke.getStrokeThickness()));
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
            s << "g.fillRect (" << x << ", " << y << ", " << w << ", " << h << ");\n\n";

            paintMethodCode += s;
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            strokeType.fill.fillInGeneratedCode (code, paintMethodCode);
            s << "g.drawRect (" << x << ", " << y << ", " << w << ", " << h << ", "
              << roundDoubleToInt (strokeType.stroke.getStrokeThickness()) << ");\n\n";

            paintMethodCode += s;
        }
    }

    static const tchar* getTagName() throw()        { return T("RECT"); }

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
        const Rectangle r (getCurrentAbsoluteBounds());

        Path path;
        path.addRectangle ((float) r.getX(), (float) r.getY(), (float) r.getWidth(), (float) r.getHeight());

        convertToNewPathElement (path);
    }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementRectangle* const element_)
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
        PaintElementRectangle* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTRECTANGLE_JUCEHEADER__
