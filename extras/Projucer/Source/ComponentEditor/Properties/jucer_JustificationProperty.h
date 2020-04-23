/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class JustificationProperty  : public ChoicePropertyComponent
{
public:
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

    //==============================================================================
    virtual void setJustification (Justification newJustification) = 0;
    virtual Justification getJustification() const = 0;

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

        const int rawFlags = getJustification().getFlags();

        for (int i = numElementsInArray (types); --i >= 0;)
            if (types[i] == rawFlags)
                return i;

        return -1;
    }
};
