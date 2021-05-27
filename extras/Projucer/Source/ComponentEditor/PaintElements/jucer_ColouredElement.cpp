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

#include "../../Application/jucer_Headers.h"
#include "jucer_ColouredElement.h"
#include "jucer_GradientPointComponent.h"
#include "../Properties/jucer_PositionPropertyBase.h"
#include "../Properties/jucer_ColourPropertyComponent.h"
#include "jucer_PaintElementUndoableAction.h"
#include "jucer_PaintElementPath.h"
#include "jucer_ImageResourceProperty.h"


//==============================================================================
class ElementFillModeProperty   : public ChoicePropertyComponent
{
public:
    ElementFillModeProperty (ColouredElement* const e, const bool isForStroke_)
        : ChoicePropertyComponent ("fill mode"), listener (e),
          isForStroke (isForStroke_)
    {
        listener.setPropertyToRefresh (*this);

        choices.add ("Solid Colour");
        choices.add ("Linear Gradient");
        choices.add ("Radial Gradient");
        choices.add ("Image Brush");
    }

    void setIndex (int newIndex)
    {
        JucerFillType fill (isForStroke ? listener.owner->getStrokeType().fill
                                        : listener.owner->getFillType());

        switch (newIndex)
        {
            case 0:  fill.mode = JucerFillType::solidColour; break;
            case 1:  fill.mode = JucerFillType::linearGradient; break;
            case 2:  fill.mode = JucerFillType::radialGradient; break;
            case 3:  fill.mode = JucerFillType::imageBrush; break;
            default: jassertfalse; break;
        }

        if (! isForStroke)
            listener.owner->setFillType (fill, true);
        else
            listener.owner->setStrokeFill (fill, true);
    }

    int getIndex() const
    {
        switch (isForStroke ? listener.owner->getStrokeType().fill.mode
                            : listener.owner->getFillType().mode)
        {
            case JucerFillType::solidColour:    return 0;
            case JucerFillType::linearGradient: return 1;
            case JucerFillType::radialGradient: return 2;
            case JucerFillType::imageBrush:     return 3;
            default:                            jassertfalse; break;
        }

        return 0;
    }

private:
    ElementListener<ColouredElement> listener;
    const bool isForStroke;
};

//==============================================================================
class ElementFillColourProperty  : public JucerColourPropertyComponent
{
public:
    enum ColourType
    {
        solidColour,
        gradientColour1,
        gradientColour2
    };

    ElementFillColourProperty (const String& name,
                               ColouredElement* const owner_,
                               const ColourType type_,
                               const bool isForStroke_)
        : JucerColourPropertyComponent (name, false),
          listener (owner_),
          type (type_),
          isForStroke (isForStroke_)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setColour (Colour newColour) override
    {
        listener.owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        JucerFillType fill (isForStroke ? listener.owner->getStrokeType().fill
                                        : listener.owner->getFillType());

        switch (type)
        {
            case solidColour:       fill.colour = newColour; break;
            case gradientColour1:   fill.gradCol1 = newColour; break;
            case gradientColour2:   fill.gradCol2 = newColour; break;
            default:                jassertfalse; break;
        }

        if (! isForStroke)
            listener.owner->setFillType (fill, true);
        else
            listener.owner->setStrokeFill (fill, true);
    }

    Colour getColour() const override
    {
        const JucerFillType fill (isForStroke ? listener.owner->getStrokeType().fill
                                              : listener.owner->getFillType());

        switch (type)
        {
            case solidColour:       return fill.colour; break;
            case gradientColour1:   return fill.gradCol1; break;
            case gradientColour2:   return fill.gradCol2; break;
            default:                jassertfalse; break;
        }

        return Colours::black;
    }

    void resetToDefault() override
    {
        jassertfalse; // option shouldn't be visible
    }

private:
    ElementListener<ColouredElement> listener;
    const ColourType type;
    const bool isForStroke;
};

//==============================================================================
class ElementFillPositionProperty   : public PositionPropertyBase
{
public:
    ElementFillPositionProperty (ColouredElement* const owner_,
                                 const String& name,
                                 ComponentPositionDimension dimension_,
                                 const bool isStart_,
                                 const bool isForStroke_)
     : PositionPropertyBase (owner_, name, dimension_, false, false,
                             owner_->getDocument()->getComponentLayout()),
       listener (owner_),
       isStart (isStart_),
       isForStroke (isForStroke_)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        JucerFillType fill (isForStroke ? listener.owner->getStrokeType().fill
                                        : listener.owner->getFillType());

        if (isStart)
            fill.gradPos1 = newPos;
        else
            fill.gradPos2 = newPos;

        if (! isForStroke)
            listener.owner->setFillType (fill, true);
        else
            listener.owner->setStrokeFill (fill, true);
    }

    RelativePositionedRectangle getPosition() const
    {
        const JucerFillType fill (isForStroke ? listener.owner->getStrokeType().fill
                                              : listener.owner->getFillType());

        return isStart ? fill.gradPos1
                       : fill.gradPos2;
    }

private:
    ElementListener<ColouredElement> listener;
    const bool isStart, isForStroke;
};

//==============================================================================
class EnableStrokeProperty : public BooleanPropertyComponent
{
public:
    explicit EnableStrokeProperty (ColouredElement* const owner_)
        : BooleanPropertyComponent ("outline", "Outline enabled", "No outline"),
          listener (owner_)
    {
        listener.setPropertyToRefresh (*this);
    }

    //==============================================================================
    void setState (bool newState)           { listener.owner->enableStroke (newState, true); }
    bool getState() const                   { return listener.owner->isStrokeEnabled(); }

    ElementListener<ColouredElement> listener;
};

//==============================================================================
class StrokeThicknessProperty   : public SliderPropertyComponent
{
public:
    explicit StrokeThicknessProperty (ColouredElement* const owner_)
        : SliderPropertyComponent ("outline thickness", 0.1, 200.0, 0.1, 0.3),
          listener (owner_)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setValue (double newValue)
    {
        listener.owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        listener.owner->setStrokeType (PathStrokeType ((float) newValue,
                                                       listener.owner->getStrokeType().stroke.getJointStyle(),
                                                       listener.owner->getStrokeType().stroke.getEndStyle()),
                                       true);
    }

    double getValue() const                 { return listener.owner->getStrokeType().stroke.getStrokeThickness(); }

    ElementListener<ColouredElement> listener;
};

//==============================================================================
class StrokeJointProperty : public ChoicePropertyComponent
{
public:
    explicit StrokeJointProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent ("joint style"),
          listener (owner_)
    {
        listener.setPropertyToRefresh (*this);

        choices.add ("mitered");
        choices.add ("curved");
        choices.add ("beveled");
    }

    void setIndex (int newIndex)
    {
        const PathStrokeType::JointStyle joints[] = { PathStrokeType::mitered,
                                                      PathStrokeType::curved,
                                                      PathStrokeType::beveled };

        if (! isPositiveAndBelow (newIndex, numElementsInArray (joints)))
        {
            jassertfalse;
            return;
        }

        listener.owner->setStrokeType (PathStrokeType (listener.owner->getStrokeType().stroke.getStrokeThickness(),
                                                       joints [newIndex],
                                                       listener.owner->getStrokeType().stroke.getEndStyle()),
                                       true);
    }

    int getIndex() const
    {
        switch (listener.owner->getStrokeType().stroke.getJointStyle())
        {
            case PathStrokeType::mitered:   return 0;
            case PathStrokeType::curved:    return 1;
            case PathStrokeType::beveled:   return 2;
            default:                        jassertfalse; break;
        }

        return 0;
    }

    ElementListener<ColouredElement> listener;
};

//==============================================================================
class StrokeEndCapProperty   : public ChoicePropertyComponent
{
public:
    explicit StrokeEndCapProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent ("end-cap style"),
          listener (owner_)
    {
        listener.setPropertyToRefresh (*this);

        choices.add ("butt");
        choices.add ("square");
        choices.add ("round");
    }

    void setIndex (int newIndex)
    {
        const PathStrokeType::EndCapStyle ends[] = { PathStrokeType::butt,
                                                     PathStrokeType::square,
                                                     PathStrokeType::rounded };

        if (! isPositiveAndBelow (newIndex, numElementsInArray (ends)))
        {
            jassertfalse;
            return;
        }

        listener.owner->setStrokeType (PathStrokeType (listener.owner->getStrokeType().stroke.getStrokeThickness(),
                                                       listener.owner->getStrokeType().stroke.getJointStyle(),
                                                       ends [newIndex]),
                                       true);
    }

    int getIndex() const
    {
        switch (listener.owner->getStrokeType().stroke.getEndStyle())
        {
            case PathStrokeType::butt:      return 0;
            case PathStrokeType::square:    return 1;
            case PathStrokeType::rounded:   return 2;
            default:                        jassertfalse; break;
        }

        return 0;
    }

    ElementListener<ColouredElement> listener;
};

//==============================================================================
class ImageBrushResourceProperty    : public ImageResourceProperty <ColouredElement>
{
public:
    ImageBrushResourceProperty (ColouredElement* const e, const bool isForStroke_)
        : ImageResourceProperty <ColouredElement> (e, isForStroke_ ? "stroke image"
                                                                   : "fill image"),
          isForStroke (isForStroke_)
    {
    }

    //==============================================================================
    void setResource (const String& newName)
    {
        if (element != nullptr)
        {
            if (isForStroke)
            {
                JucerFillType type (element->getStrokeType().fill);
                type.imageResourceName = newName;

                element->setStrokeFill (type, true);
            }
            else
            {
                JucerFillType type (element->getFillType());
                type.imageResourceName = newName;

                element->setFillType (type, true);
            }
        }
    }

    String getResource() const
    {
        if (element == nullptr)
            return {};

        if (isForStroke)
            return element->getStrokeType().fill.imageResourceName;

        return element->getFillType().imageResourceName;
    }

private:
    bool isForStroke;
};

//==============================================================================
class ImageBrushPositionProperty    : public PositionPropertyBase
{
public:
    ImageBrushPositionProperty (ColouredElement* const owner_,
                                const String& name,
                                ComponentPositionDimension dimension_,
                                const bool isForStroke_)
        : PositionPropertyBase (owner_, name, dimension_, false, false,
                                owner_->getDocument()->getComponentLayout()),
          listener (owner_),
          isForStroke (isForStroke_)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        if (isForStroke)
        {
            JucerFillType type (listener.owner->getStrokeType().fill);
            type.imageAnchor = newPos;
            listener.owner->setStrokeFill (type, true);
        }
        else
        {
            JucerFillType type (listener.owner->getFillType());
            type.imageAnchor = newPos;
            listener.owner->setFillType (type, true);
        }
    }

    RelativePositionedRectangle getPosition() const
    {
        if (isForStroke)
            return listener.owner->getStrokeType().fill.imageAnchor;

        return listener.owner->getFillType().imageAnchor;
    }

private:
    ElementListener<ColouredElement> listener;
    const bool isForStroke;
};

//==============================================================================
class ImageBrushOpacityProperty  : public SliderPropertyComponent
{
public:
    ImageBrushOpacityProperty (ColouredElement* const e, const bool isForStroke_)
        : SliderPropertyComponent ("opacity", 0.0, 1.0, 0.001),
          listener (e),
          isForStroke (isForStroke_)
    {
        listener.setPropertyToRefresh (*this);
    }

    void setValue (double newValue)
    {
        if (listener.owner != nullptr)
        {
            listener.owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

            if (isForStroke)
            {
                JucerFillType type (listener.owner->getStrokeType().fill);
                type.imageOpacity = newValue;

                listener.owner->setStrokeFill (type, true);
            }
            else
            {
                JucerFillType type (listener.owner->getFillType());
                type.imageOpacity = newValue;

                listener.owner->setFillType (type, true);
            }
        }
    }

    double getValue() const
    {
        if (listener.owner == nullptr)
            return 0;

        if (isForStroke)
            return listener.owner->getStrokeType().fill.imageOpacity;

        return listener.owner->getFillType().imageOpacity;
    }

private:
    ElementListener<ColouredElement> listener;

    bool isForStroke;
};


//==============================================================================
ColouredElement::ColouredElement (PaintRoutine* owner_,
                                  const String& name,
                                  const bool showOutline_,
                                  const bool showJointAndEnd_)
    : PaintElement (owner_, name),
      isStrokePresent (false),
      showOutline (showOutline_),
      showJointAndEnd (showJointAndEnd_)
{
}

ColouredElement::~ColouredElement()
{
}

//==============================================================================
void ColouredElement::getEditableProperties (Array <PropertyComponent*>& props, bool multipleSelected)
{
    PaintElement::getEditableProperties (props, multipleSelected);

    if (! multipleSelected)
        getColourSpecificProperties (props);
}

void ColouredElement::getColourSpecificProperties (Array <PropertyComponent*>& props)
{
    props.add (new ElementFillModeProperty (this, false));

    switch (getFillType().mode)
    {
    case JucerFillType::solidColour:
        props.add (new ElementFillColourProperty ("colour", this, ElementFillColourProperty::solidColour, false));
        break;

    case JucerFillType::linearGradient:
    case JucerFillType::radialGradient:
        props.add (new ElementFillColourProperty ("colour 1", this, ElementFillColourProperty::gradientColour1, false));
        props.add (new ElementFillPositionProperty (this, "x1", PositionPropertyBase::componentX, true, false));
        props.add (new ElementFillPositionProperty (this, "y1", PositionPropertyBase::componentY, true, false));
        props.add (new ElementFillColourProperty ("colour 2", this, ElementFillColourProperty::gradientColour2, false));
        props.add (new ElementFillPositionProperty (this, "x2", PositionPropertyBase::componentX, false, false));
        props.add (new ElementFillPositionProperty (this, "y2", PositionPropertyBase::componentY, false, false));
        break;

    case JucerFillType::imageBrush:
        props.add (new ImageBrushResourceProperty (this, false));
        props.add (new ImageBrushPositionProperty (this, "anchor x", PositionPropertyBase::componentX, false));
        props.add (new ImageBrushPositionProperty (this, "anchor y", PositionPropertyBase::componentY, false));
        props.add (new ImageBrushOpacityProperty (this, false));
        break;

    default:
        jassertfalse;
        break;
    }

    if (showOutline)
    {
        props.add (new EnableStrokeProperty (this));

        if (isStrokePresent)
        {
            props.add (new StrokeThicknessProperty (this));

            if (showJointAndEnd)
            {
                props.add (new StrokeJointProperty (this));
                props.add (new StrokeEndCapProperty (this));
            }

            props.add (new ElementFillModeProperty (this, true));

            switch (getStrokeType().fill.mode)
            {
                case JucerFillType::solidColour:
                    props.add (new ElementFillColourProperty ("colour", this, ElementFillColourProperty::solidColour, true));
                    break;

                case JucerFillType::linearGradient:
                case JucerFillType::radialGradient:
                    props.add (new ElementFillColourProperty ("colour 1", this, ElementFillColourProperty::gradientColour1, true));
                    props.add (new ElementFillPositionProperty (this, "x1", PositionPropertyBase::componentX, true, true));
                    props.add (new ElementFillPositionProperty (this, "y1", PositionPropertyBase::componentY, true, true));
                    props.add (new ElementFillColourProperty ("colour 2", this, ElementFillColourProperty::gradientColour2, true));
                    props.add (new ElementFillPositionProperty (this, "x2", PositionPropertyBase::componentX, false, true));
                    props.add (new ElementFillPositionProperty (this, "y2", PositionPropertyBase::componentY, false, true));
                    break;

                case JucerFillType::imageBrush:
                    props.add (new ImageBrushResourceProperty (this, true));
                    props.add (new ImageBrushPositionProperty (this, "stroke anchor x", PositionPropertyBase::componentX, true));
                    props.add (new ImageBrushPositionProperty (this, "stroke anchor y", PositionPropertyBase::componentY, true));
                    props.add (new ImageBrushOpacityProperty (this, true));
                    break;

                default:
                    jassertfalse;
                    break;
            }
        }
    }
}

//==============================================================================
const JucerFillType& ColouredElement::getFillType() noexcept
{
    return fillType;
}

class FillTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    FillTypeChangeAction (ColouredElement* const element, const JucerFillType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->getFillType();
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setFillType (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setFillType (oldState, false);
        return true;
    }

private:
    JucerFillType newState, oldState;
};

void ColouredElement::setFillType (const JucerFillType& newType, const bool undoable)
{
    if (fillType != newType)
    {
        if (undoable)
        {
            perform (new FillTypeChangeAction (this, newType),
                     "Change fill type");
        }
        else
        {
            repaint();

            if (fillType.mode != newType.mode)
            {
                owner->getSelectedElements().changed();
                siblingComponentsChanged();
            }

            fillType = newType;
            changed();
        }
    }
}

//==============================================================================
bool ColouredElement::isStrokeEnabled() const noexcept
{
    return isStrokePresent && showOutline;
}

class StrokeEnableChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeEnableChangeAction (ColouredElement* const element, const bool newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->isStrokeEnabled();
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->enableStroke (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->enableStroke (oldState, false);
        return true;
    }

private:
    bool newState, oldState;
};

void ColouredElement::enableStroke (bool enable, const bool undoable)
{
    enable = enable && showOutline;

    if (isStrokePresent != enable)
    {
        if (undoable)
        {
            perform (new StrokeEnableChangeAction (this, enable),
                     "Change stroke mode");
        }
        else
        {
            repaint();
            isStrokePresent = enable;

            siblingComponentsChanged();
            owner->changed();
            owner->getSelectedElements().changed();
        }
    }
}

//==============================================================================
const StrokeType& ColouredElement::getStrokeType() noexcept
{
    return strokeType;
}

class StrokeTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeTypeChangeAction (ColouredElement* const element, const PathStrokeType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_),
          oldState (element->getStrokeType().stroke)
    {
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setStrokeType (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setStrokeType (oldState, false);
        return true;
    }

private:
    PathStrokeType newState, oldState;
};

void ColouredElement::setStrokeType (const PathStrokeType& newType, const bool undoable)
{
    if (strokeType.stroke != newType)
    {
        if (undoable)
        {
            perform (new StrokeTypeChangeAction (this, newType),
                     "Change stroke type");
        }
        else
        {
            repaint();
            strokeType.stroke = newType;
            changed();
        }
    }
}

class StrokeFillTypeChangeAction  : public PaintElementUndoableAction <ColouredElement>
{
public:
    StrokeFillTypeChangeAction (ColouredElement* const element, const JucerFillType& newState_)
        : PaintElementUndoableAction <ColouredElement> (element),
          newState (newState_)
    {
        oldState = element->getStrokeType().fill;
    }

    bool perform()
    {
        showCorrectTab();
        getElement()->setStrokeFill (newState, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getElement()->setStrokeFill (oldState, false);
        return true;
    }

private:
    JucerFillType newState, oldState;
};

void ColouredElement::setStrokeFill (const JucerFillType& newType, const bool undoable)
{
    if (strokeType.fill != newType)
    {
        if (undoable)
        {
            perform (new StrokeFillTypeChangeAction (this, newType),
                     "Change stroke fill type");
        }
        else
        {
            repaint();

            if (strokeType.fill.mode != newType.mode)
            {
                siblingComponentsChanged();
                owner->getSelectedElements().changed();
            }

            strokeType.fill = newType;
            changed();
        }
    }
}

//==============================================================================
void ColouredElement::createSiblingComponents()
{
    {
        GradientPointComponent* g1 = new GradientPointComponent (this, false, true);
        siblingComponents.add (g1);

        GradientPointComponent* g2 = new GradientPointComponent (this, false, false);
        siblingComponents.add (g2);

        getParentComponent()->addAndMakeVisible (g1);
        getParentComponent()->addAndMakeVisible (g2);

        g1->updatePosition();
        g2->updatePosition();
    }

    if (isStrokePresent && showOutline)
    {
        GradientPointComponent* g1 = new GradientPointComponent (this, true, true);
        siblingComponents.add (g1);

        GradientPointComponent* g2 = new GradientPointComponent (this, true, false);
        siblingComponents.add (g2);

        getParentComponent()->addAndMakeVisible (g1);
        getParentComponent()->addAndMakeVisible (g2);

        g1->updatePosition();
        g2->updatePosition();
    }
}

Rectangle<int> ColouredElement::getCurrentBounds (const Rectangle<int>& parentArea) const
{
    int borderSize = 0;

    if (isStrokePresent)
        borderSize = (int) strokeType.stroke.getStrokeThickness() / 2 + 1;

    return position.getRectangle (parentArea, getDocument()->getComponentLayout())
                   .expanded (borderSize);
}

void ColouredElement::setCurrentBounds (const Rectangle<int>& newBounds,
                                        const Rectangle<int>& parentArea,
                                        const bool undoable)
{
    Rectangle<int> r (newBounds);

    if (isStrokePresent)
    {
        r = r.expanded (-((int) strokeType.stroke.getStrokeThickness() / 2 + 1));

        r.setSize (jmax (1, r.getWidth()), jmax (1, r.getHeight()));
    }

    RelativePositionedRectangle pr (position);
    pr.updateFrom (r.getX() - parentArea.getX(),
                   r.getY() - parentArea.getY(),
                   r.getWidth(), r.getHeight(),
                   Rectangle<int> (0, 0, parentArea.getWidth(), parentArea.getHeight()),
                   getDocument()->getComponentLayout());
    setPosition (pr, undoable);

    updateBounds (parentArea);
}

//==============================================================================
void ColouredElement::addColourAttributes (XmlElement* const e) const
{
    e->setAttribute ("fill", fillType.toString());
    e->setAttribute ("hasStroke", isStrokePresent);

    if (isStrokePresent && showOutline)
    {
        e->setAttribute ("stroke", strokeType.toString());
        e->setAttribute ("strokeColour", strokeType.fill.toString());
    }
}

bool ColouredElement::loadColourAttributes (const XmlElement& xml)
{
    fillType.restoreFromString (xml.getStringAttribute ("fill", String()));

    isStrokePresent = showOutline && xml.getBoolAttribute ("hasStroke", false);

    strokeType.restoreFromString (xml.getStringAttribute ("stroke", String()));
    strokeType.fill.restoreFromString (xml.getStringAttribute ("strokeColour", String()));

    return true;
}

//==============================================================================
void ColouredElement::convertToNewPathElement (const Path& path)
{
    if (! path.isEmpty())
    {
        PaintElementPath newElement (getOwner());
        newElement.setToPath (path);
        newElement.setFillType (fillType, false);
        newElement.enableStroke (isStrokeEnabled(), false);
        newElement.setStrokeType (getStrokeType().stroke, false);
        newElement.setStrokeFill (getStrokeType().fill, false);

        std::unique_ptr<XmlElement> xml (newElement.createXml());

        PaintElement* e = getOwner()->addElementFromXml (*xml, getOwner()->indexOfElement (this), true);

        getOwner()->getSelectedElements().selectOnly (e);
        getOwner()->removeElement (this, true);
    }
}
