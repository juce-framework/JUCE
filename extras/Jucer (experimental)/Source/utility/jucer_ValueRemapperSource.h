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

#ifndef __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__
#define __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__


//==============================================================================
/** A ValueSource that remaps specific values to new values.
*/
class ValueRemapperSource   : public Value::ValueSource,
                              public Value::Listener
{
public:
    ValueRemapperSource (const Value& sourceValue_)
       : sourceValue (sourceValue_)
    {
    }

    ValueRemapperSource (const Value& sourceValue_, const char** mappings)
       : sourceValue (sourceValue_)
    {
        addMappings (mappings);
    }

    ~ValueRemapperSource() {}

    void addMappings (const char** values)
    {
        while (values[0] != 0 && values[1] != 0)
        {
            addMapping (values[0], values[1]);
            values += 2;
        }
    }

    void addMapping (const var& sourceValue, const var& remappedValue)
    {
        mappings.add (sourceValue);
        mappings.add (remappedValue);
    }

    const var getValue() const
    {
        const var sourceVar (sourceValue.getValue());

        for (int i = 0; i < mappings.size(); i += 2)
            if (sourceVar == mappings.getReference(i))
                return mappings.getReference (i + 1);

        return sourceVar;
    }

    void setValue (const var& newValue)
    {
        for (int i = 1; i < mappings.size(); i += 2)
        {
            if (newValue == mappings.getReference(i))
            {
                sourceValue = mappings.getReference (i - 1);
                return;
            }
        }

        sourceValue = newValue;
    }

    void valueChanged (Value&)
    {
        sendChangeMessage (false);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Value sourceValue;
    Array<var> mappings;

    ValueRemapperSource (const ValueRemapperSource&);
    const ValueRemapperSource& operator= (const ValueRemapperSource&);
};


#endif   // __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__
