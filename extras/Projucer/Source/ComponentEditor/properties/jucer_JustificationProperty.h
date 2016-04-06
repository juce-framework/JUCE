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

#ifndef JUCER_JUSTIFICATIONPROPERTY_H_INCLUDED
#define JUCER_JUSTIFICATIONPROPERTY_H_INCLUDED


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


#endif   // JUCER_JUSTIFICATIONPROPERTY_H_INCLUDED
