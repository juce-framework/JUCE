/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_JUSTIFICATIONPROPERTY_JUCEHEADER__
#define __JUCER_JUSTIFICATIONPROPERTY_JUCEHEADER__


//==============================================================================
/**
*/
class JustificationProperty  : public ChoicePropertyComponent
{
public:
    //==============================================================================
    JustificationProperty (const String& name, const bool onlyHorizontalOptions)
        : ChoicePropertyComponent (name)
    {
        if (onlyHorizontalOptions)
        {
            choices.add ("centre");
            choices.add ("left");
            choices.add ("right");
        }
        else
        {
            choices.add ("centred");
            choices.add ("centred left");
            choices.add ("centred right");
            choices.add ("centred top");
            choices.add ("centred bottom");
            choices.add ("top left");
            choices.add ("top right");
            choices.add ("bottom left");
            choices.add ("bottom right");
        }
    }

    ~JustificationProperty()
    {
    }

    //==============================================================================
    virtual void setJustification (const Justification& newJustification) = 0;
    virtual const Justification getJustification() const = 0;


    //==============================================================================
    void setIndex (int newIndex)
    {
        const int types[] = { Justification::centred,
                              Justification::centredLeft,
                              Justification::centredRight,
                              Justification::centredTop,
                              Justification::centredBottom,
                              Justification::topLeft,
                              Justification::topRight,
                              Justification::bottomLeft,
                              Justification::bottomRight };

        if (((unsigned int) newIndex) < (unsigned int) numElementsInArray (types)
             && types [newIndex] != getJustification().getFlags())
        {
            setJustification (Justification (types [newIndex]));
        }
    }

    int getIndex() const
    {
        const int types[] = { Justification::centred,
                              Justification::centredLeft,
                              Justification::centredRight,
                              Justification::centredTop,
                              Justification::centredBottom,
                              Justification::topLeft,
                              Justification::topRight,
                              Justification::bottomLeft,
                              Justification::bottomRight };

        const int flags = getJustification().getFlags();

        for (int i = numElementsInArray (types); --i >= 0;)
            if (types[i] == flags)
                return i;

        return -1;
    }
};


#endif   // __JUCER_JUSTIFICATIONPROPERTY_JUCEHEADER__
