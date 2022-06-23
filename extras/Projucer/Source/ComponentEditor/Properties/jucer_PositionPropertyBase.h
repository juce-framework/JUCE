/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../UI/jucer_PaintRoutineEditor.h"
#include "../UI/jucer_ComponentLayoutEditor.h"

//==============================================================================
/**
    Base class for a property that edits the x, y, w, or h of a PositionedRectangle.
*/
class PositionPropertyBase  : public PropertyComponent,
                              protected ChangeListener
{
public:
    enum ComponentPositionDimension
    {
        componentX          = 0,
        componentY          = 1,
        componentWidth      = 2,
        componentHeight     = 3
    };

    PositionPropertyBase (Component* comp,
                          const String& name,
                          ComponentPositionDimension dimension_,
                          const bool includeAnchorOptions_,
                          const bool allowRelativeOptions_,
                          ComponentLayout* layout_)
        : PropertyComponent (name),
          layout (layout_),
          button ("mode"),
          component (comp),
          dimension (dimension_),
          includeAnchorOptions (includeAnchorOptions_),
          allowRelativeOptions (allowRelativeOptions_)
    {
        addAndMakeVisible (button);
        button.setTriggeredOnMouseDown (true);
        button.setConnectedEdges (TextButton::ConnectedOnLeft | TextButton::ConnectedOnRight);
        button.onClick = [this]
        {
            SafePointer<PositionPropertyBase> safeThis { this };
            showMenu (layout, [safeThis] (bool shouldRefresh)
            {
                if (safeThis == nullptr)
                    return;

                if (shouldRefresh)
                    safeThis->refresh(); // (to clear the text editor if it's got focus)
            });
        };

        textEditor.reset (new PositionPropLabel (*this));
        addAndMakeVisible (textEditor.get());
    }

    String getText() const
    {
        RelativePositionedRectangle rpr (getPosition());
        PositionedRectangle& p = rpr.rect;
        String s;

        switch (dimension)
        {
        case componentX:
            if (p.getPositionModeX() == PositionedRectangle::proportionOfParentSize)
                s << valueToString (p.getX() * 100.0) << '%';
            else
                s << valueToString (p.getX());
            break;

        case componentY:
            if (p.getPositionModeY() == PositionedRectangle::proportionOfParentSize)
                s << valueToString (p.getY() * 100.0) << '%';
            else
                s << valueToString (p.getY());
            break;

        case componentWidth:
            if (p.getWidthMode() == PositionedRectangle::proportionalSize)
                s << valueToString (p.getWidth() * 100.0) << '%';
            else
                s << valueToString (p.getWidth());
            break;

        case componentHeight:
            if (p.getHeightMode() == PositionedRectangle::proportionalSize)
                s << valueToString (p.getHeight() * 100.0) << '%';
            else
                s << valueToString (p.getHeight());
            break;

        default:
            jassertfalse;
            break;
        };

        return s;
    }

    static String valueToString (const double n)
    {
        return String (roundToInt (n * 1000.0) / 1000.0);
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

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

    void showMenu (ComponentLayout* compLayout, std::function<void (bool)> callback)
    {
        RelativePositionedRectangle rpr (getPosition());
        PositionedRectangle p (rpr.rect);

        PositionedRectangle::AnchorPoint xAnchor = p.getAnchorPointX();
        PositionedRectangle::AnchorPoint yAnchor = p.getAnchorPointY();
        PositionedRectangle::PositionMode xMode = p.getPositionModeX();
        PositionedRectangle::PositionMode yMode = p.getPositionModeY();
        PositionedRectangle::SizeMode sizeW = p.getWidthMode();
        PositionedRectangle::SizeMode sizeH = p.getHeightMode();

        String relCompName ("parent");

        if (Component* const relComp = compLayout != nullptr ? compLayout->getComponentRelativePosTarget (component, (int) dimension)
                                                             : nullptr)
            relCompName = compLayout->getComponentMemberVariableName (relComp);

        jassert (relCompName.isNotEmpty());

        PopupMenu m;

        if (dimension == componentX || dimension == componentY)
        {
            const PositionedRectangle::PositionMode posMode = (dimension == componentX) ? xMode : yMode;

            m.addItem (10, ((dimension == componentX) ? "Absolute distance from left of "
                                                      : "Absolute distance from top of ") + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentTopLeft);

            m.addItem (11, ((dimension == componentX) ? "Absolute distance from right of "
                                                      : "Absolute distance from bottom of ") + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentBottomRight);

            m.addItem (12, "Absolute distance from centre of " + relCompName,
                       true, posMode == PositionedRectangle::absoluteFromParentCentre);

            m.addItem (13, ((dimension == componentX) ? "Percentage of width of "
                                                      : "Percentage of height of ") + relCompName,
                       true, posMode == PositionedRectangle::proportionOfParentSize);

            m.addSeparator();

            if (includeAnchorOptions)
            {
                const PositionedRectangle::AnchorPoint anchor = (dimension == componentX) ? xAnchor : yAnchor;

                m.addItem (14, (dimension == componentX) ? "Anchored at left of component"
                                                         : "Anchored at top of component",
                           true, anchor == PositionedRectangle::anchorAtLeftOrTop);

                m.addItem (15, "Anchored at centre of component", true, anchor == PositionedRectangle::anchorAtCentre);

                m.addItem (16, (dimension == componentX) ? "Anchored at right of component"
                                                         : "Anchored at bottom of component",
                           true, anchor == PositionedRectangle::anchorAtRightOrBottom);
            }
        }
        else
        {
            const PositionedRectangle::SizeMode sizeMode = (dimension == componentWidth) ? sizeW : sizeH;

            m.addItem (20, (dimension == componentWidth) ? "Absolute width"
                                                         : "Absolute height",
                       true, sizeMode == PositionedRectangle::absoluteSize);

            m.addItem (21, ((dimension == componentWidth) ? "Percentage of width of "
                                                          : "Percentage of height of ") + relCompName,
                       true, sizeMode == PositionedRectangle::proportionalSize);

            m.addItem (22, ((dimension == componentWidth) ? "Subtracted from width of "
                                                          : "Subtracted from height of ") + relCompName,
                       true, sizeMode == PositionedRectangle::parentSizeMinusAbsolute);
        }


        if (allowRelativeOptions && compLayout != nullptr)
        {
            m.addSeparator();
            m.addSubMenu ("Relative to", compLayout->getRelativeTargetMenu (component, (int) dimension));
        }

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (&button),
                         [compLayout, callback, xAnchor, yAnchor, xMode, yMode, sizeW, sizeH, p, rpr,
                          ref = SafePointer<PositionPropertyBase> { this }] (int menuResult) mutable
        {
            if (menuResult == 0 || ref == nullptr)
            {
                callback (false);
                return;
            }

            switch (menuResult)
            {
            case 10:
                if (ref->dimension == componentX)
                    xMode = PositionedRectangle::absoluteFromParentTopLeft;
                else
                    yMode = PositionedRectangle::absoluteFromParentTopLeft;
                break;

            case 11:
                if (ref->dimension == componentX)
                    xMode = PositionedRectangle::absoluteFromParentBottomRight;
                else
                    yMode = PositionedRectangle::absoluteFromParentBottomRight;
                break;

            case 12:
                if (ref->dimension == componentX)
                    xMode = PositionedRectangle::absoluteFromParentCentre;
                else
                    yMode = PositionedRectangle::absoluteFromParentCentre;
                break;

            case 13:
                if (ref->dimension == componentX)
                    xMode = PositionedRectangle::proportionOfParentSize;
                else
                    yMode = PositionedRectangle::proportionOfParentSize;
                break;

            case 14:
                if (ref->dimension == componentX)
                    xAnchor = PositionedRectangle::anchorAtLeftOrTop;
                else
                    yAnchor = PositionedRectangle::anchorAtLeftOrTop;
                break;

            case 15:
                if (ref->dimension == componentX)
                    xAnchor = PositionedRectangle::anchorAtCentre;
                else
                    yAnchor = PositionedRectangle::anchorAtCentre;
                break;

            case 16:
                if (ref->dimension == componentX)
                    xAnchor = PositionedRectangle::anchorAtRightOrBottom;
                else
                    yAnchor = PositionedRectangle::anchorAtRightOrBottom;
                break;

            case 20:
                if (ref->dimension == componentWidth)
                    sizeW = PositionedRectangle::absoluteSize;
                else
                    sizeH = PositionedRectangle::absoluteSize;
                break;

            case 21:
                if (ref->dimension == componentWidth)
                    sizeW = PositionedRectangle::proportionalSize;
                else
                    sizeH = PositionedRectangle::proportionalSize;
                break;

            case 22:
                if (ref->dimension == componentWidth)
                    sizeW = PositionedRectangle::parentSizeMinusAbsolute;
                else
                    sizeH = PositionedRectangle::parentSizeMinusAbsolute;
                break;

            default:
                if (ref->allowRelativeOptions && compLayout != nullptr)
                    compLayout->processRelativeTargetMenuResult (ref->component, (int) ref->dimension, menuResult);
                break;
            }

            const auto parentArea = [&]() -> Rectangle<int>
            {
                if (ref->component->findParentComponentOfClass<ComponentLayoutEditor>() != nullptr)
                    return { ref->component->getParentWidth(), ref->component->getParentHeight() };

                if (auto pre = dynamic_cast<PaintRoutineEditor*> (ref->component->getParentComponent()))
                    return pre->getComponentArea();

                jassertfalse;
                return {};
            }();

            int x, xw, y, yh, w, h;
            rpr.getRelativeTargetBounds (parentArea, compLayout, x, xw, y, yh, w, h);

            PositionedRectangle xyRect (p);
            PositionedRectangle whRect (p);

            xyRect.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                             Rectangle<int> (x, y, xw, yh));

            whRect.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                             Rectangle<int> (x, y, w, h));

            p.setModes (xAnchor, xMode, yAnchor, yMode, sizeW, sizeH,
                        Rectangle<int> (x, y, xw, yh));

            p.setX (xyRect.getX());
            p.setY (xyRect.getY());
            p.setWidth (whRect.getWidth());
            p.setHeight (whRect.getHeight());

            if (p != rpr.rect)
            {
                rpr.rect = p;
                ref->setPosition (rpr);
            }

            callback (true);
        });
    }

    void resized()
    {
        const Rectangle<int> r (getLookAndFeel().getPropertyComponentContentPosition (*this));

        button.changeWidthToFitText (r.getHeight());
        button.setTopRightPosition (r.getRight(), r.getY());

        textEditor->setBounds (r.getX(), r.getY(), button.getX() - r.getX(), r.getHeight());
    }

    void refresh()
    {
        textEditor->setText (getText(), dontSendNotification);
    }

    void textWasEdited()
    {
        const String newText (textEditor->getText());
        if (getText() != newText)
            setText (newText);
    }

    //==============================================================================
    virtual void setPosition (const RelativePositionedRectangle& newPos) = 0;

    virtual RelativePositionedRectangle getPosition() const = 0;

protected:
    class PositionPropLabel  : public Label
    {
        PositionPropertyBase& owner;

    public:
        PositionPropLabel (PositionPropertyBase& owner_)
            : Label (String(), String()),
              owner (owner_)
        {
            setEditable (true, true, false);
            lookAndFeelChanged();
        }

        TextEditor* createEditorComponent() override
        {
            TextEditor* ed = Label::createEditorComponent();
            ed->setInputRestrictions (14, "0123456789.-%");

            return ed;
        }

        void textWasEdited() override
        {
            owner.textWasEdited();
        }

        void lookAndFeelChanged() override
        {
            setColour (backgroundColourId, findColour (widgetBackgroundColourId));
            setColour (textColourId, findColour (widgetTextColourId));
        }
    };

    ComponentLayout* layout;
    std::unique_ptr<PositionPropLabel> textEditor;
    TextButton button;

    Component* component;
    ComponentPositionDimension dimension;
    const bool includeAnchorOptions, allowRelativeOptions;
};
