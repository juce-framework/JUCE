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

#ifndef JUCER_PAINTELEMENTRECTANGLE_H_INCLUDED
#define JUCER_PAINTELEMENTRECTANGLE_H_INCLUDED

#include "jucer_ColouredElement.h"


//==============================================================================
class PaintElementRectangle     : public ColouredElement
{
public:
    PaintElementRectangle (PaintRoutine* pr)
        : ColouredElement (pr, "Rectangle", true, false)
    {
    }

    Rectangle<int> getCurrentBounds (const Rectangle<int>& parentArea) const
    {
        return PaintElement::getCurrentBounds (parentArea); // bypass the ColouredElement implementation
    }

    void setCurrentBounds (const Rectangle<int>& newBounds, const Rectangle<int>& parentArea, const bool undoable)
    {
        PaintElement::setCurrentBounds (newBounds, parentArea, undoable); // bypass the ColouredElement implementation
    }

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
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

    void getEditableProperties (Array <PropertyComponent*>& props)
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
            s << "g.fillRect (" << x << ", " << y << ", " << w << ", " << h << ");\n\n";

            paintMethodCode += s;
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            strokeType.fill.fillInGeneratedCode (code, paintMethodCode);
            s << "g.drawRect (" << x << ", " << y << ", " << w << ", " << h << ", "
              << roundToInt (strokeType.stroke.getStrokeThickness()) << ");\n\n";

            paintMethodCode += s;
        }
    }

    static const char* getTagName() noexcept        { return "RECT"; }

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
        Path path;
        path.addRectangle (getCurrentAbsoluteBounds());
        convertToNewPathElement (path);
    }

private:
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


#endif   // JUCER_PAINTELEMENTRECTANGLE_H_INCLUDED
