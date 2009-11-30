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

#include "../../jucer_Headers.h"
#include "jucer_ColouredElement.h"
#include "jucer_GradientPointComponent.h"
#include "../../properties/jucer_PositionPropertyBase.h"
#include "../../properties/jucer_ColourPropertyComponent.h"
#include "jucer_PaintElementUndoableAction.h"
#include "jucer_PaintElementPath.h"
#include "jucer_ImageResourceProperty.h"


//==============================================================================
class ElementFillModeProperty   : public ChoicePropertyComponent,
                                  private ChangeListener
{
public:
    ElementFillModeProperty (ColouredElement* const owner_, const bool isForStroke_)
        : ChoicePropertyComponent (T("fill mode")),
          owner (owner_),
          isForStroke (isForStroke_)
    {
        choices.add (T("Solid Colour"));
        choices.add (T("Linear Gradient"));
        choices.add (T("Radial Gradient"));
        choices.add (T("Image Brush"));

        owner->getDocument()->addChangeListener (this);
    }

    ~ElementFillModeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setIndex (const int newIndex)
    {
        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        switch (newIndex)
        {
        case 0:
            fill.mode = JucerFillType::solidColour;
            break;
        case 1:
            fill.mode = JucerFillType::linearGradient;
            break;
        case 2:
            fill.mode = JucerFillType::radialGradient;
            break;
        case 3:
            fill.mode = JucerFillType::imageBrush;
            break;

        default:
            jassertfalse
            break;
        }

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    int getIndex() const
    {
        switch (isForStroke ? owner->getStrokeType().fill.mode
                            : owner->getFillType().mode)
        {
        case JucerFillType::solidColour:
            return 0;
        case JucerFillType::linearGradient:
            return 1;
        case JucerFillType::radialGradient:
            return 2;
        case JucerFillType::imageBrush:
            return 3;
        default:
            jassertfalse
            break;
        }

        return 0;
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

private:
    ColouredElement* const owner;
    const bool isForStroke;
};

//==============================================================================
class ElementFillColourProperty  : public ColourPropertyComponent,
                                   private ChangeListener
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
        : ColourPropertyComponent (name, false),
          owner (owner_),
          type (type_),
          isForStroke (isForStroke_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~ElementFillColourProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setColour (const Colour& newColour)
    {
        owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        switch (type)
        {
        case solidColour:
            fill.colour = newColour;
            break;

        case gradientColour1:
            fill.gradCol1 = newColour;
            break;

        case gradientColour2:
            fill.gradCol2 = newColour;
            break;

        default:
            jassertfalse
            break;
        }

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    const Colour getColour() const
    {
        const JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                              : owner->getFillType());

        switch (type)
        {
        case solidColour:
            return fill.colour;
            break;

        case gradientColour1:
            return fill.gradCol1;
            break;

        case gradientColour2:
            return fill.gradCol2;
            break;

        default:
            jassertfalse
            break;
        }

        return Colours::black;
    }

    void resetToDefault()
    {
        jassertfalse // option shouldn't be visible
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

private:
    ColouredElement* const owner;
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
       owner (owner_),
       isStart (isStart_),
       isForStroke (isForStroke_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~ElementFillPositionProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    void setPosition (const RelativePositionedRectangle& newPos)
    {
        JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                        : owner->getFillType());

        if (isStart)
            fill.gradPos1 = newPos;
        else
            fill.gradPos2 = newPos;

        if (! isForStroke)
            owner->setFillType (fill, true);
        else
            owner->setStrokeFill (fill, true);
    }

    const RelativePositionedRectangle getPosition() const
    {
        const JucerFillType fill (isForStroke ? owner->getStrokeType().fill
                                              : owner->getFillType());

        return isStart ? fill.gradPos1
                       : fill.gradPos2;
    }

private:
    ColouredElement* const owner;
    const bool isStart, isForStroke;
};

//==============================================================================
class EnableStrokeProperty : public BooleanPropertyComponent,
                             public ChangeListener
{
public:
    EnableStrokeProperty (ColouredElement* const owner_)
        : BooleanPropertyComponent (T("outline"), T("Outline enabled"), T("No outline")),
          owner (owner_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~EnableStrokeProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    //==============================================================================
    void setState (const bool newState)     { owner->enableStroke (newState, true); }
    bool getState() const                   { return owner->isStrokeEnabled(); }

    void changeListenerCallback (void*)     { refresh(); }

private:
    ColouredElement* const owner;
};

//==============================================================================
class StrokeThicknessProperty   : public SliderPropertyComponent,
                                  public ChangeListener
{
public:
    StrokeThicknessProperty (ColouredElement* const owner_)
        : SliderPropertyComponent (T("outline thickness"), 0.1, 200.0, 0.1, 0.3),
          owner (owner_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~StrokeThicknessProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setValue (const double newValue)
    {
        owner->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        owner->setStrokeType (PathStrokeType ((float) newValue,
                                              owner->getStrokeType().stroke.getJointStyle(),
                                              owner->getStrokeType().stroke.getEndStyle()),
                              true);
    }

    const double getValue() const           { return owner->getStrokeType().stroke.getStrokeThickness(); }

    void changeListenerCallback (void*)     { refresh(); }

private:
    ColouredElement* const owner;
};

//==============================================================================
class StrokeJointProperty : public ChoicePropertyComponent,
                            public ChangeListener
{
public:
    StrokeJointProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent (T("joint style")),
          owner (owner_)
    {
        choices.add (T("mitered"));
        choices.add (T("curved"));
        choices.add (T("beveled"));

        owner->getDocument()->addChangeListener (this);
    }

    ~StrokeJointProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setIndex (const int newIndex)
    {
        const PathStrokeType::JointStyle joints[] = { PathStrokeType::mitered,
                                                      PathStrokeType::curved,
                                                      PathStrokeType::beveled };

        jassert (newIndex >= 0 && newIndex < 3);

        owner->setStrokeType (PathStrokeType (owner->getStrokeType().stroke.getStrokeThickness(),
                                              joints [newIndex],
                                              owner->getStrokeType().stroke.getEndStyle()),
                              true);
    }

    int getIndex() const
    {
        switch (owner->getStrokeType().stroke.getJointStyle())
        {
        case PathStrokeType::mitered:
            return 0;
        case PathStrokeType::curved:
            return 1;
        case PathStrokeType::beveled:
            return 2;
        default:
            jassertfalse
            break;
        }

        return 0;
    }

    void changeListenerCallback (void*)     { refresh(); }

private:
    ColouredElement* const owner;
};

//==============================================================================
class StrokeEndCapProperty   : public ChoicePropertyComponent,
                               public ChangeListener
{
public:
    StrokeEndCapProperty (ColouredElement* const owner_)
        : ChoicePropertyComponent (T("end-cap style")),
          owner (owner_)
    {
        choices.add (T("butt"));
        choices.add (T("square"));
        choices.add (T("round"));

        owner->getDocument()->addChangeListener (this);
    }

    ~StrokeEndCapProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setIndex (const int newIndex)
    {
        const PathStrokeType::EndCapStyle ends[] = { PathStrokeType::butt,
                                                     PathStrokeType::square,
                                                     PathStrokeType::rounded };

        jassert (newIndex >= 0 && newIndex < 3);

        owner->setStrokeType (PathStrokeType (owner->getStrokeType().stroke.getStrokeThickness(),
                                              owner->getStrokeType().stroke.getJointStyle(),
                                              ends [newIndex]),
                              true);
    }

    int getIndex() const
    {
        switch (owner->getStrokeType().stroke.getEndStyle())
        {
        case PathStrokeType::butt:
            return 0;
        case PathStrokeType::square:
            return 1;
        case PathStrokeType::rounded:
            return 2;
        default:
            jassertfalse
            break;
        }

        return 0;
    }

    void changeListenerCallback (void*)     { refresh(); }

private:
    ColouredElement* const owner;
};

//==============================================================================
class ImageBrushResourceProperty    : public ImageResourceProperty <ColouredElement>
{
public:
    ImageBrushResourceProperty (ColouredElement* const element_, const bool isForStroke_)
        : ImageResourceProperty <ColouredElement> (element_, isForStroke_ ? T("stroke image")
                                                                          : T("fill image")),
          isForStroke (isForStroke_)
    {
    }

    //==============================================================================
    void setResource (const String& newName)
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

    const String getResource() const
    {
        if (isForStroke)
            return element->getStrokeType().fill.imageResourceName;
        else
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
          owner (owner_),
          isForStroke (isForStroke_)
    {
        owner->getDocument()->addChangeListener (this);
    }

    ~ImageBrushPositionProperty()
    {
        owner->getDocument()->removeChangeListener (this);
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        if (isForStroke)
        {
            JucerFillType type (owner->getStrokeType().fill);
            type.imageAnchor = newPos;
            owner->setStrokeFill (type, true);
        }
        else
        {
            JucerFillType type (owner->getFillType());
            type.imageAnchor = newPos;
            owner->setFillType (type, true);
        }
    }

    const RelativePositionedRectangle getPosition() const
    {
        if (isForStroke)
            return owner->getStrokeType().fill.imageAnchor;
        else
            return owner->getFillType().imageAnchor;
    }

private:
    ColouredElement* const owner;
    const bool isForStroke;
};

//==============================================================================
class ImageBrushOpacityProperty  : public SliderPropertyComponent,
                                   private ChangeListener
{
public:
    ImageBrushOpacityProperty (ColouredElement* const element_, const bool isForStroke_)
        : SliderPropertyComponent (T("opacity"), 0.0, 1.0, 0.001),
          element (element_),
          isForStroke (isForStroke_)
    {
        element->getDocument()->addChangeListener (this);
    }

    ~ImageBrushOpacityProperty()
    {
        element->getDocument()->removeChangeListener (this);
    }

    void setValue (const double newValue)
    {
        element->getDocument()->getUndoManager().undoCurrentTransactionOnly();

        if (isForStroke)
        {
            JucerFillType type (element->getStrokeType().fill);
            type.imageOpacity = newValue;

            element->setStrokeFill (type, true);
        }
        else
        {
            JucerFillType type (element->getFillType());
            type.imageOpacity = newValue;

            element->setFillType (type, true);
        }
    }

    const double getValue() const
    {
        if (isForStroke)
            return element->getStrokeType().fill.imageOpacity;
        else
            return element->getFillType().imageOpacity;
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

private:
    ColouredElement* const element;
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
void ColouredElement::getEditableProperties (Array <PropertyComponent*>& properties)
{
    PaintElement::getEditableProperties (properties);
    getColourSpecificProperties (properties);
}

void ColouredElement::getColourSpecificProperties (Array <PropertyComponent*>& properties)
{
    properties.add (new ElementFillModeProperty (this, false));

    switch (getFillType().mode)
    {
    case JucerFillType::solidColour:
        properties.add (new ElementFillColourProperty (T("colour"), this, ElementFillColourProperty::solidColour, false));
        break;

    case JucerFillType::linearGradient:
    case JucerFillType::radialGradient:
        properties.add (new ElementFillColourProperty (T("colour 1"), this, ElementFillColourProperty::gradientColour1, false));
        properties.add (new ElementFillPositionProperty (this, T("x1"), PositionPropertyBase::componentX, true, false));
        properties.add (new ElementFillPositionProperty (this, T("y1"), PositionPropertyBase::componentY, true, false));
        properties.add (new ElementFillColourProperty (T("colour 2"), this, ElementFillColourProperty::gradientColour2, false));
        properties.add (new ElementFillPositionProperty (this, T("x2"), PositionPropertyBase::componentX, false, false));
        properties.add (new ElementFillPositionProperty (this, T("y2"), PositionPropertyBase::componentY, false, false));
        break;

    case JucerFillType::imageBrush:
        properties.add (new ImageBrushResourceProperty (this, false));
        properties.add (new ImageBrushPositionProperty (this, T("anchor x"), PositionPropertyBase::componentX, false));
        properties.add (new ImageBrushPositionProperty (this, T("anchor y"), PositionPropertyBase::componentY, false));
        properties.add (new ImageBrushOpacityProperty (this, false));
        break;

    default:
        jassertfalse
        break;
    }

    if (showOutline)
    {
        properties.add (new EnableStrokeProperty (this));

        if (isStrokePresent)
        {
            properties.add (new StrokeThicknessProperty (this));

            if (showJointAndEnd)
            {
                properties.add (new StrokeJointProperty (this));
                properties.add (new StrokeEndCapProperty (this));
            }

            properties.add (new ElementFillModeProperty (this, true));

            switch (getStrokeType().fill.mode)
            {
            case JucerFillType::solidColour:
                properties.add (new ElementFillColourProperty (T("colour"), this, ElementFillColourProperty::solidColour, true));
                break;

            case JucerFillType::linearGradient:
            case JucerFillType::radialGradient:
                properties.add (new ElementFillColourProperty (T("colour 1"), this, ElementFillColourProperty::gradientColour1, true));
                properties.add (new ElementFillPositionProperty (this, T("x1"), PositionPropertyBase::componentX, true, true));
                properties.add (new ElementFillPositionProperty (this, T("y1"), PositionPropertyBase::componentY, true, true));
                properties.add (new ElementFillColourProperty (T("colour 2"), this, ElementFillColourProperty::gradientColour2, true));
                properties.add (new ElementFillPositionProperty (this, T("x2"), PositionPropertyBase::componentX, false, true));
                properties.add (new ElementFillPositionProperty (this, T("y2"), PositionPropertyBase::componentY, false, true));
                break;

            case JucerFillType::imageBrush:
                properties.add (new ImageBrushResourceProperty (this, true));
                properties.add (new ImageBrushPositionProperty (this, T("stroke anchor x"), PositionPropertyBase::componentX, true));
                properties.add (new ImageBrushPositionProperty (this, T("stroke anchor y"), PositionPropertyBase::componentY, true));
                properties.add (new ImageBrushOpacityProperty (this, true));
                break;

            default:
                jassertfalse
                break;
            }
        }
    }
}

//==============================================================================
const JucerFillType& ColouredElement::getFillType() throw()
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
                     T("Change fill type"));
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
bool ColouredElement::isStrokeEnabled() const throw()
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
                     T("Change stroke mode"));
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
const StrokeType& ColouredElement::getStrokeType() throw()
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
                     T("Change stroke type"));
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
                     T("Change stroke fill type"));
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
    GradientPointComponent* g1 = new GradientPointComponent (this, false, true);
    siblingComponents.add (g1);

    GradientPointComponent* g2 = new GradientPointComponent (this, false, false);
    siblingComponents.add (g2);

    getParentComponent()->addAndMakeVisible (g1);
    getParentComponent()->addAndMakeVisible (g2);

    g1->updatePosition();
    g2->updatePosition();

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

const Rectangle ColouredElement::getCurrentBounds (const Rectangle& parentArea) const
{
    int border = 0;

    if (isStrokePresent)
        border = (int) strokeType.stroke.getStrokeThickness() / 2 + 1;

    return position.getRectangle (parentArea, getDocument()->getComponentLayout())
                   .expanded (border, border);
}

void ColouredElement::setCurrentBounds (const Rectangle& newBounds,
                                        const Rectangle& parentArea,
                                        const bool undoable)
{
    Rectangle r (newBounds);

    if (isStrokePresent)
    {
        const int border = (int) strokeType.stroke.getStrokeThickness() / 2 + 1;
        r = r.expanded (-border, -border);

        r.setSize (jmax (1, r.getWidth()), jmax (1, r.getHeight()));
    }

    RelativePositionedRectangle pr (position);
    pr.updateFrom (r.getX() - parentArea.getX(),
                   r.getY() - parentArea.getY(),
                   r.getWidth(), r.getHeight(),
                   Rectangle (0, 0, parentArea.getWidth(), parentArea.getHeight()),
                   getDocument()->getComponentLayout());
    setPosition (pr, undoable);

    updateBounds (parentArea);
}

//==============================================================================
void ColouredElement::addColourAttributes (XmlElement* const e) const
{
    e->setAttribute (T("fill"), fillType.toString());
    e->setAttribute (T("hasStroke"), isStrokePresent);

    if (isStrokePresent && showOutline)
    {
        e->setAttribute (T("stroke"), strokeType.toString());
        e->setAttribute (T("strokeColour"), strokeType.fill.toString());
    }
}

bool ColouredElement::loadColourAttributes (const XmlElement& xml)
{
    fillType.restoreFromString (xml.getStringAttribute (T("fill"), String::empty));

    isStrokePresent = showOutline && xml.getBoolAttribute (T("hasStroke"), false);

    strokeType.restoreFromString (xml.getStringAttribute (T("stroke"), String::empty));
    strokeType.fill.restoreFromString (xml.getStringAttribute (T("strokeColour"), String::empty));

    return true;
}

//==============================================================================
void ColouredElement::convertToNewPathElement (const Path& path)
{
    if (! path.isEmpty())
    {
        PaintElementPath* newElement = new PaintElementPath (getOwner());
        newElement->setToPath (path);
        newElement->setFillType (fillType, false);
        newElement->enableStroke (isStrokeEnabled(), false);
        newElement->setStrokeType (getStrokeType().stroke, false);
        newElement->setStrokeFill (getStrokeType().fill, false);

        XmlElement* xml = newElement->createXml();
        delete newElement;

        PaintElement* e = getOwner()->addElementFromXml (*xml, getOwner()->indexOfElement (this), true);
        delete xml;

        getOwner()->getSelectedElements().selectOnly (e);
        getOwner()->removeElement (this, true);
    }
}
