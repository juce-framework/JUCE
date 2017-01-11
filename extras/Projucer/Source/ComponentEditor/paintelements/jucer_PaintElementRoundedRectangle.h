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

#ifndef JUCER_PAINTELEMENTROUNDEDRECTANGLE_H_INCLUDED
#define JUCER_PAINTELEMENTROUNDEDRECTANGLE_H_INCLUDED

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

    void draw (Graphics& g, const ComponentLayout* layout, const Rectangle<int>& parentArea)
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

    void getEditableProperties (Array<PropertyComponent*>& props)
    {
        props.add (new CornerSizeProperty (this));

        ColouredElement::getEditableProperties (props);

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
    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode)
    {
        if (fillType.isInvisible() && (strokeType.isInvisible() || ! isStrokePresent))
            return;
        
        String x, y, w, h, s;
        positionToCode (position, code.document->getComponentLayout(), x, y, w, h);
        s << "{\n"
          << "    float x = " << castToFloat (x) << ", y = " << castToFloat (y) << ", "
          <<           "width = " << castToFloat (w) << ", height = " << castToFloat (h) << ";\n"
          << "    //[UserPaintCustomArguments] Customize the painting arguments here..\n"
          << customPaintCode
          << "    //[/UserPaintCustomArguments]\n";
  
        if (! fillType.isInvisible())
        {
            s << "    ";
            fillType.fillInGeneratedCode (position, code, s);
            s << "    g.fillRoundedRectangle (x, y, width, height, " << CodeHelpers::floatLiteral (cornerSize, 3)  << ");\n";
        }

        if (isStrokePresent && ! strokeType.isInvisible())
        {
            s << "    ";
            strokeType.fill.fillInGeneratedCode (position, code, s);
            s << "    g.drawRoundedRectangle (x, y, width, height, " << CodeHelpers::floatLiteral (cornerSize, 3)  << ", " << CodeHelpers::floatLiteral (strokeType.stroke.getStrokeThickness(), 3) << ");\n";
        }
        
        s << "}\n\n";
        
        paintMethodCode += s;
    }

    void applyCustomPaintSnippets (StringArray& snippets)
    {
        customPaintCode.clear();
        
        if (! snippets.isEmpty() && (! fillType.isInvisible() || (isStrokePresent && ! strokeType.isInvisible())))
        {
            customPaintCode = snippets[0];
            snippets.remove(0);
        }
    }

    static const char* getTagName() noexcept        { return "ROUNDRECT"; }

    XmlElement* createXml() const
    {
        XmlElement* const e = new XmlElement (getTagName());

        position.applyToXml (*e);
        e->setAttribute ("cornerSize", cornerSize);
        addColourAttributes (e);

        return e;
    }

    bool loadFromXml (const XmlElement& xml)
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


#endif   // JUCER_PAINTELEMENTROUNDEDRECTANGLE_H_INCLUDED
