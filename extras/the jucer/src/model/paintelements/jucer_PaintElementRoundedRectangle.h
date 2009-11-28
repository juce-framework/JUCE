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

#ifndef __JUCER_PAINTELEMENTROUNDEDRECTANGLE_JUCEHEADER__
#define __JUCER_PAINTELEMENTROUNDEDRECTANGLE_JUCEHEADER__

#include "jucer_ColouredElement.h"


//==============================================================================
/**
*/
class PaintElementRoundedRectangle  : public ColouredElement
{
public:
    //==============================================================================
    PaintElementRoundedRectangle (PaintRoutine* owner)
        : ColouredElement (owner, T("Rounded Rectangle"), true, false)
    {
        cornerSize = 10.0;
    }

    ~PaintElementRoundedRectangle() {}

    //==============================================================================
    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle& parentArea)
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

    void getEditableProperties (Array <PropertyComponent*>& properties)
    {
        properties.add (new CornerSizeProperty (this));

        ColouredElement::getEditableProperties (properties);

        properties.add (new ShapeToPathProperty (this));
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
                         T("Change rounded rectangle corner size"));
            }
            else
            {
                cornerSize = newSize;
                changed();
            }
        }
    }

    double getCornerSize() const throw()                         { return cornerSize; }

    //==============================================================================
    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        if (! fillType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            fillType.fillInGeneratedCode (code, paintMethodCode);
            s << "g.fillRoundedRectangle ("
              << castToFloat (x) << ", "
              << castToFloat (y) << ", "
              << castToFloat (w) << ", "
              << castToFloat (h) << ", "
              << valueToFloat (cornerSize) << ");\n\n";

            paintMethodCode += s;
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            String x, y, w, h, s;
            positionToCode (position, code.document->getComponentLayout(), x, y, w, h);

            strokeType.fill.fillInGeneratedCode (code, paintMethodCode);
            s << "g.drawRoundedRectangle ("
              << castToFloat (x) << ", "
              << castToFloat (y) << ", "
              << castToFloat (w) << ", "
              << castToFloat (h) << ", "
              << valueToFloat (cornerSize) << ", "
              << valueToFloat (strokeType.stroke.getStrokeThickness()) << ");\n\n";

            paintMethodCode += s;
        }
    }

    static const tchar* getTagName() throw()        { return T("ROUNDRECT"); }

    XmlElement* createXml() const
    {
        XmlElement* const e = new XmlElement (getTagName());

        position.applyToXml (*e);
        e->setAttribute (T("cornerSize"), cornerSize);
        addColourAttributes (e);

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
    {
        if (xml.hasTagName (getTagName()))
        {
            position.restoreFromXml (xml, position);
            cornerSize = xml.getDoubleAttribute (T("cornerSize"), 10.0);
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
        path.addRoundedRectangle ((float) x, (float) y, (float) w, (float) h, (float) cornerSize);

        convertToNewPathElement (path);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    double cornerSize;

    //==============================================================================
    class CornerSizeProperty  : public SliderPropertyComponent,
                                public ChangeListener
    {
    public:
        CornerSizeProperty (PaintElementRoundedRectangle* const owner_)
            : SliderPropertyComponent (T("corner size"), 1.0, 200.0, 0.5, 0.4),
              owner (owner_)
        {
            owner->getDocument()->addChangeListener (this);
        }

        ~CornerSizeProperty()
        {
            owner->getDocument()->removeChangeListener (this);
        }

        void setValue (const double newValue)
        {
            owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            owner->setCornerSize (newValue, true);
        }

        const double getValue() const           { return owner->getCornerSize(); }

        void changeListenerCallback (void*)     { refresh(); }

    private:
        PaintElementRoundedRectangle* const owner;
    };

    //==============================================================================
    class ShapeToPathProperty  : public ButtonPropertyComponent
    {
    public:
        ShapeToPathProperty (PaintElementRoundedRectangle* const element_)
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
        PaintElementRoundedRectangle* const element;
    };
};


#endif   // __JUCER_PAINTELEMENTROUNDEDRECTANGLE_JUCEHEADER__
