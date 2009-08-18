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

#ifndef __JUCER_POSITIONPROPERTYBASE_JUCEHEADER__
#define __JUCER_POSITIONPROPERTYBASE_JUCEHEADER__

#include "../ui/jucer_PaintRoutineEditor.h"
#include "../ui/jucer_ComponentLayoutEditor.h"

//==============================================================================
/**
    Base class for a property that edits the x, y, w, or h of a PositionedRectangle.
*/
class PositionPropertyBase  : public PropertyComponent,
                              protected ChangeListener,
                              private ButtonListener
{
public:
    enum ComponentPositionDimension
    {
        componentX          = 0,
        componentY          = 1,
        componentWidth      = 2,
        componentHeight     = 3
    };

    PositionPropertyBase (Component* component_,
                          const String& name,
                          ComponentPositionDimension dimension_,
                          const bool includeAnchorOptions_,
                          const bool allowRelativeOptions_,
                          ComponentLayout* layout_)
        : PropertyComponent (name),
          layout (layout_),
          component (component_),
          dimension (dimension_),
          includeAnchorOptions (includeAnchorOptions_),
          allowRelativeOptions (allowRelativeOptions_)
    {
        addAndMakeVisible (button = new TextButton (T("mode")));
        button->addButtonListener (this);
        button->setTriggeredOnMouseDown (true);
        button->setConnectedEdges (TextButton::ConnectedOnLeft | TextButton::ConnectedOnRight);

        addAndMakeVisible (textEditor = new PositionPropLabel (*this));
    }

    ~PositionPropertyBase()
    {
        deleteAllChildren();
    }

    const String getText() const
    {
        RelativePositionedRectangle rpr (getPosition());
        PositionedRectangle& p = rpr.rect;
        String s;

        switch (dimension)
        {
        case componentX:
            if (p.getPositionModeX() == PositionedRectangle::proportionOfParentSize)
                s << valueToString (p.getX() * 100.0) << T('%');
            else
                s << valueToString (p.getX());
            break;

        case componentY:
            if (p.getPositionModeY() == PositionedRectangle::proportionOfParentSize)
                s << valueToString (p.getY() * 100.0) << T('%');
            else
                s << valueToString (p.getY());
            break;

        case componentWidth:
            if (p.getWidthMode() == PositionedRectangle::proportionalSize)
                s << valueToString (p.getWidth() * 100.0) << T('%');
            else
                s << valueToString (p.getWidth());
            break;

        case componentHeight:
            if (p.getHeightMode() == PositionedRectangle::proportionalSize)
                s << valueToString (p.getHeight() * 100.0) << T('%');
            else
                s << valueToString (p.getHeight());
            break;

        default:
            jassertfalse;
            break;
        };

        return s;
    }

    static const String valueToString (const double n)
    {
        return String (roundDoubleToInt (n * 1000.0) / 1000.0);
    }

    void setText (const String& newText)
    {
        RelativePositionedRectangle rpr (getPosition());
        PositionedRectangle p (rpr.rect);
        const double value = newText.getDoubleValue();

        switch (dimension)
        {
        case componentX:
            if (p.getPositionModeX() == PositionedRectangle::proportionOfParentSize)
                p.setX (value / 100.0);
            else
                p.setX (value);
            break;

        case componentY:
            if (p.getPositionModeY() == PositionedRectangle::proportionOfParentSize)
                p.setY (value / 100.0);
            else
                p.setY (value);
            break;

        case componentWidth:
            if (p.getWidthMode() == PositionedRectangle::proportionalSize)
                p.setWidth (value / 100.0);
            else
                p.setWidth (value);
            break;

        case componentHeight:
            if (p.getHeightMode() == PositionedRectangle::proportionalSize)
                p.setHeight (value / 100.0);
            else
                p.setHeight (value);
            break;

        default:
            jassertfalse;
            break;
        };

        if (p != rpr.rect)
        {
            rpr.rect = p;
            setPosition (rpr);
        }
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

    void showMenu (ComponentLayout* layout)
    {
        RelativePositionedRectangle rpr (getPosition());
        PositionedRectangle p (rpr.rect);

        PositionedRectangle::AnchorPoint xAnchor = p.getAnchorPointX();
        PositionedRectangle::AnchorPoint yAnchor = p.getAnchorPointY();
        PositionedRectangle::PositionMode xMode = p.getPositionModeX();
        PositionedRectangle::PositionMode yMode = p.getPositionModeY();
        PositionedRectangle::SizeMode sizeW = p.getWidthMode();
        PositionedRectangle::SizeMode sizeH = p.getHeightMode();

        String relCompName (T("parent"));

        Component* const relComp = layout != 0 ? layout->getComponentRelativePosTarget (component, (int) dimension) : 0;

        if (relComp != 0)
            relCompName = layout->getComponentMemberVariableName (relComp);

        jassert (relCompName.isNotEmpty());

        PopupMenu m;

        if (dimension == componentX || dimension == componentY)
        {
            const PositionedRectangle::PositionMode posMode = (dimension == componentX) ? xMode : yMode;

            m.addItem (10, ((dimension == componentX) ? T("Absolute distance from left of ")
                                                      : T("Absolute distance from top of ")) + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentTopLeft);

            m.addItem (11, ((dimension == componentX) ? T("Absolute distance from right of ")
                                                      : T("Absolute distance from bottom of ")) + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentBottomRight);

            m.addItem (12, T("Absolute distance from centre of ") + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentCentre);

            m.addItem (13, ((dimension == componentX) ? T("Percentage of width of ")
                                                      : T("Percentage of height of ")) + relCompName,
                       true, posMode == PositionedRectangle::proportionOfParentSize);

            m.addSeparator();

            if (includeAnchorOptions)
            {
                const PositionedRectangle::AnchorPoint anchor = (dimension == componentX) ? xAnchor : yAnchor;

                m.addItem (14, (dimension == componentX) ? T("Anchored at left of component")
                                                         : T("Anchored at top of component"),
                           true, anchor == PositionedRectangle::anchorAtLeftOrTop);

                m.addItem (15, T("Anchored at centre of component"), true, anchor == PositionedRectangle::anchorAtCentre);

                m.addItem (16, (dimension == componentX) ? T("Anchored at right of component")
                                                         : T("Anchored at bottom of component"),
                           true, anchor == PositionedRectangle::anchorAtRightOrBottom);
            }
        }
        else
        {
            const PositionedRectangle::SizeMode sizeMode = (dimension == componentWidth) ? sizeW : sizeH;

            m.addItem (20, (dimension == componentWidth) ? T("Absolute width")
                                                         : T("Absolute height"),
                       true, sizeMode == PositionedRectangle::absoluteSize);

            m.addItem (21, ((dimension == componentWidth) ? T("Percentage of width of ")
                                                          : T("Percentage of height of ")) + relCompName,
                       true, sizeMode == PositionedRectangle::proportionalSize);

            m.addItem (22, ((dimension == componentWidth) ? T("Subtracted from width of ")
                                                          : T("Subtracted from height of ")) + relCompName,
                       true, sizeMode == PositionedRectangle::parentSizeMinusAbsolute);
        }


        if (allowRelativeOptions && layout != 0)
        {
            m.addSeparator();
            m.addSubMenu (T("Relative to"), layout->getRelativeTargetMenu (component, (int) dimension));
        }

        const int menuResult = m.showAt (button);
        switch (menuResult)
        {
        case 10:
            if (dimension == componentX)
                xMode = PositionedRectangle::absoluteFromParentTopLeft;
            else
                yMode = PositionedRectangle::absoluteFromParentTopLeft;
            break;

        case 11:
            if (dimension == componentX)
                xMode = PositionedRectangle::absoluteFromParentBottomRight;
            else
                yMode = PositionedRectangle::absoluteFromParentBottomRight;
            break;

        case 12:
            if (dimension == componentX)
                xMode = PositionedRectangle::absoluteFromParentCentre;
            else
                yMode = PositionedRectangle::absoluteFromParentCentre;
            break;

        case 13:
            if (dimension == componentX)
                xMode = PositionedRectangle::proportionOfParentSize;
            else
                yMode = PositionedRectangle::proportionOfParentSize;
            break;

        case 14:
            if (dimension == componentX)
                xAnchor = PositionedRectangle::anchorAtLeftOrTop;
            else
                yAnchor = PositionedRectangle::anchorAtLeftOrTop;
            break;

        case 15:
            if (dimension == componentX)
                xAnchor = PositionedRectangle::anchorAtCentre;
            else
                yAnchor = PositionedRectangle::anchorAtCentre;
            break;

        case 16:
            if (dimension == componentX)
                xAnchor = PositionedRectangle::anchorAtRightOrBottom;
            else
                yAnchor = PositionedRectangle::anchorAtRightOrBottom;
            break;

        case 20:
            if (dimension == componentWidth)
                sizeW = PositionedRectangle::absoluteSize;
            else
                sizeH = PositionedRectangle::absoluteSize;

            break;

        case 21:
            if (dimension == componentWidth)
                sizeW = PositionedRectangle::proportionalSize;
            else
                sizeH = PositionedRectangle::proportionalSize;

            break;

        case 22:
            if (dimension == componentWidth)
                sizeW = PositionedRectangle::parentSizeMinusAbsolute;
            else
                sizeH = PositionedRectangle::parentSizeMinusAbsolute;

            break;

        default:
            if (allowRelativeOptions && layout != 0)
                layout->processRelativeTargetMenuResult (component, (int) dimension, menuResult);

            break;
        }

        Rectangle parentArea;

        if (component->findParentComponentOfClass ((ComponentLayoutEditor*) 0) != 0)
        {
            parentArea.setSize (component->getParentWidth(), component->getParentHeight());
        }
        else if (dynamic_cast <PaintRoutineEditor*> (component->getParentComponent()) != 0)
        {
            parentArea = dynamic_cast <PaintRoutineEditor*> (component->getParentComponent())->getComponentArea();
        }
        else
        {
            jassertfalse
        }

        int x, xw, y, yh, w, h;
        rpr.getRelativeTargetBounds (parentArea, layout, x, xw, y, yh, w, h);

        PositionedRectangle xyRect (p);
        PositionedRectangle whRect (p);

        xyRect.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                         Rectangle (x, y, xw, yh));

        whRect.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                         Rectangle (x, y, w, h));

        p.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                    Rectangle (x, y, xw, yh));

        p.setX (xyRect.getX());
        p.setY (xyRect.getY());
        p.setWidth (whRect.getWidth());
        p.setHeight (whRect.getHeight());

        if (p != rpr.rect)
        {
            rpr.rect = p;
            setPosition (rpr);
        }
    }

    void resized()
    {
        const Rectangle r (getLookAndFeel().getPropertyComponentContentPosition (*this));

        button->changeWidthToFitText (r.getHeight());
        button->setTopRightPosition (r.getRight(), r.getY());

        textEditor->setBounds (r.getX(), r.getY(), button->getX() - r.getX(), r.getHeight());
    }

    void refresh()
    {
        textEditor->setText (getText(), false);
    }

    void buttonClicked (Button*)
    {
        showMenu (layout);
        refresh(); // (to clear the text editor if it's got focus)
    }

    void textWasEdited()
    {
        const String newText (textEditor->getText());
        if (getText() != newText)
            setText (newText);
    }

    //==============================================================================
    virtual void setPosition (const RelativePositionedRectangle& newPos) = 0;

    virtual const RelativePositionedRectangle getPosition() const = 0;

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    class PositionPropLabel  : public Label
    {
        PositionPropertyBase& owner;
        int maxChars;
        bool isMultiline;

    public:
        PositionPropLabel (PositionPropertyBase& owner_)
            : Label (String::empty, String::empty),
              owner (owner_)
        {
            setEditable (true, true, false);

            setColour (backgroundColourId, Colours::white);
            setColour (textColourId, Colours::black);
            setColour (outlineColourId, findColour (ComboBox::outlineColourId));

            setColour (TextEditor::textColourId, Colours::black);
            setColour (TextEditor::backgroundColourId, Colours::white);
            setColour (TextEditor::outlineColourId, findColour (ComboBox::outlineColourId));
        }

        ~PositionPropLabel()
        {
        }

        TextEditor* createEditorComponent()
        {
            TextEditor* textEditor = Label::createEditorComponent();
            textEditor->setInputRestrictions (14, T("0123456789.-%"));

            return textEditor;
        }

        void textWasEdited()
        {
            owner.textWasEdited();
        }
    };

    ComponentLayout* layout;
    PositionPropLabel* textEditor;
    TextButton* button;

    Component* component;
    ComponentPositionDimension dimension;
    const bool includeAnchorOptions, allowRelativeOptions;
};


#endif   // __JUCER_POSITIONPROPERTYBASE_JUCEHEADER__
