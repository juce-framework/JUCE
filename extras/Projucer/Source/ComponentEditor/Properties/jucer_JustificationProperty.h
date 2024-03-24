/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
    void setIndex (int newIndex) override
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

    int getIndex() const override
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
