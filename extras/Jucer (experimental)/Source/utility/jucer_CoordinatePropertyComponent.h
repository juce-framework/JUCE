/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_COORDINATEPROPERTYCOMPONENT_H_1128AA3D__
#define __JUCER_COORDINATEPROPERTYCOMPONENT_H_1128AA3D__


//==============================================================================
class CoordinatePropertyComponent  : public PropertyComponent,
                                     public ButtonListener,
                                     public Value::Listener
{
public:
    //==============================================================================
    CoordinatePropertyComponent (ComponentDocument& document_, const String& name,
                                 const Value& coordValue_, bool isHorizontal_)
        : PropertyComponent (name, 40), document (document_),
          coordValue (coordValue_),
          textValue (Value (new CoordEditableValueSource (coordValue_, isHorizontal_))),
         isHorizontal (isHorizontal_)
    {
        addAndMakeVisible (label = new Label (String::empty, String::empty));

        label->setEditable (true, true, false);
        label->setColour (Label::backgroundColourId, Colours::white);
        label->setColour (Label::outlineColourId, findColour (ComboBox::outlineColourId));
        label->getTextValue().referTo (textValue);

        addAndMakeVisible (proportionButton = new TextButton ("%"));
        proportionButton->addButtonListener (this);

        addAndMakeVisible (anchorButton1 = new TextButton (String::empty));
        anchorButton1->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnTop | Button::ConnectedOnRight | Button::ConnectedOnBottom);
        anchorButton1->setTriggeredOnMouseDown (true);
        anchorButton1->addButtonListener (this);

        addAndMakeVisible (anchorButton2 = new TextButton (String::empty));
        anchorButton2->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnTop | Button::ConnectedOnRight | Button::ConnectedOnBottom);
        anchorButton2->setTriggeredOnMouseDown (true);
        anchorButton2->addButtonListener (this);

        coordValue.addListener (this);
        valueChanged (coordValue);
    }

    ~CoordinatePropertyComponent()
    {
        coordValue.removeListener (this);
        deleteAllChildren();
    }

    void resized()
    {
        const Rectangle<int> r (getLookAndFeel().getPropertyComponentContentPosition (*this));

        label->setBounds (r.getX(), r.getY(), r.getWidth() / 2, r.getHeight() / 2);
        proportionButton->setBounds (r.getX() + r.getWidth() / 2, r.getY(),
                                     r.getWidth() / 2, r.getHeight() / 2);

        if (anchorButton2->isVisible())
        {
            anchorButton1->setBounds (r.getX(), r.getY() + r.getHeight() / 2, r.getWidth() / 2, r.getHeight() / 2);
            anchorButton2->setBounds (r.getX() + r.getWidth() / 2, r.getY() + r.getHeight() / 2, r.getWidth() / 2, r.getHeight() / 2);
        }
        else
        {
            anchorButton1->setBounds (r.getX(), r.getY() + r.getHeight() / 2, r.getWidth(), r.getHeight() / 2);
        }
    }

    void refresh() {}

    void buttonClicked (Button* button)
    {
        Coordinate coord (getCoordinate());

        if (button == proportionButton)
        {
            coord.toggleProportionality (document);
            coordValue = coord.toString();
        }
        else if (button == anchorButton1)
        {
            const String marker (pickMarker (anchorButton1, coord.getAnchor1(), true));

            if (marker.isNotEmpty())
            {
                coord.changeAnchor1 (marker, document);
                coordValue = coord.toString();
            }
        }
        else if (button == anchorButton2)
        {
            const String marker (pickMarker (anchorButton2, coord.getAnchor2(), false));

            if (marker.isNotEmpty())
            {
                coord.changeAnchor2 (marker, document);
                coordValue = coord.toString();
            }
        }
    }

    void valueChanged (Value&)
    {
        Coordinate coord (getCoordinate());

        anchorButton1->setButtonText (coord.getAnchor1());

        anchorButton2->setVisible (coord.isProportional());
        anchorButton2->setButtonText (coord.getAnchor2());
        resized();
    }

    const Coordinate getCoordinate() const
    {
        return Coordinate (coordValue.toString(), isHorizontal);
    }

    virtual const String pickMarker (TextButton* button, const String& currentMarker, bool isAnchor1) = 0;

protected:
    ComponentDocument& document;
    Value coordValue, textValue;
    Label* label;
    TextButton* proportionButton;
    TextButton* anchorButton1;
    TextButton* anchorButton2;
    bool isHorizontal;

    //==============================================================================
    class CoordEditableValueSource   : public Value::ValueSource,
                                       public Value::Listener
    {
    public:
        CoordEditableValueSource (const Value& sourceValue_, bool isHorizontal_)
           : sourceValue (sourceValue_), isHorizontal (isHorizontal_)
        {
            sourceValue.addListener (this);
        }

        ~CoordEditableValueSource() {}

        const var getValue() const
        {
            Coordinate coord (sourceValue.toString(), isHorizontal);

            if (coord.isProportional())
                return String (coord.getEditableValue()) + "%";

            return coord.getEditableValue();
        }

        void setValue (const var& newValue)
        {
            Coordinate coord (sourceValue.toString(), isHorizontal);
            coord.setEditableValue ((double) newValue);

            const String newVal (coord.toString());
            if (sourceValue != newVal)
                sourceValue = newVal;
        }

        void valueChanged (Value&)
        {
            sendChangeMessage (true);
        }

        //==============================================================================
        juce_UseDebuggingNewOperator

    protected:
        Value sourceValue;
        bool isHorizontal;

        CoordEditableValueSource (const CoordEditableValueSource&);
        const CoordEditableValueSource& operator= (const CoordEditableValueSource&);
    };
};



#endif  // __JUCER_COORDINATEPROPERTYCOMPONENT_H_1128AA3D__
