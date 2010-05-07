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
        sourceValue.addListener (this);
    }

    ValueRemapperSource (const Value& sourceValue_, const char** mappings_)
       : sourceValue (sourceValue_)
    {
        addMappings (mappings_);
        sourceValue.addListener (this);
    }

    ~ValueRemapperSource() {}

    void addMappings (const char* const* values)
    {
        while (values[0] != 0 && values[1] != 0)
        {
            addMapping (values[0], values[1]);
            values += 2;
        }
    }

    void addMapping (const var& sourceValue_, const var& remappedValue)
    {
        mappings.add (sourceValue_);
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
        var remappedVal (newValue);

        for (int i = 1; i < mappings.size(); i += 2)
        {
            if (newValue == mappings.getReference(i))
            {
                remappedVal = mappings.getReference (i - 1);
                break;
            }
        }

        if (remappedVal != sourceValue)
            sourceValue = remappedVal;
    }

    void valueChanged (Value&)
    {
        sendChangeMessage (true);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Value sourceValue;
    Array<var> mappings;

    ValueRemapperSource (const ValueRemapperSource&);
    const ValueRemapperSource& operator= (const ValueRemapperSource&);
};

//==============================================================================
/** A ValueSource that converts strings into an ID suitable for a combo box.
*/
class StringListValueSource   : public Value::ValueSource,
                                public Value::Listener
{
public:
    StringListValueSource (const Value& sourceValue_, const StringArray& strings_)
       : sourceValue (sourceValue_), strings (strings_)
    {
        sourceValue.addListener (this);
    }

    ~StringListValueSource() {}

    const var getValue() const              { return jmax (0, strings.indexOf (sourceValue.toString())) + 1; }
    void setValue (const var& newValue)
    {
        const String newVal (strings [((int) newValue) - 1]);

        if (newVal != getValue().toString())  // this test is important, because if a property is missing, it won't
            sourceValue = newVal;             // create it (causing an unwanted undo action) when a control sets it to empty
    }

    void valueChanged (Value&)              { sendChangeMessage (true); }

    static ChoicePropertyComponent* create (const String& title, const Value& value, const StringArray& strings)
    {
        return new ChoicePropertyComponent (Value (new StringListValueSource (value, strings)), title, strings);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Value sourceValue;
    StringArray strings;

    StringListValueSource (const StringListValueSource&);
    const StringListValueSource& operator= (const StringListValueSource&);
};

//==============================================================================
/**
*/
class IntegerValueSource   : public Value::ValueSource,
                             public Value::Listener
{
public:
    IntegerValueSource (const Value& sourceValue_)
       : sourceValue (sourceValue_)
    {
        sourceValue.addListener (this);
    }

    ~IntegerValueSource() {}

    const var getValue() const
    {
        return (int) sourceValue.getValue();
    }

    void setValue (const var& newValue)
    {
        const int newVal = (int) newValue;

        if (newVal != (int) getValue())  // this test is important, because if a property is missing, it won't
            sourceValue = newVal;        // create it (causing an unwanted undo action) when a control sets it to 0
    }

    void valueChanged (Value&)
    {
        sendChangeMessage (true);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    Value sourceValue;

    IntegerValueSource (const IntegerValueSource&);
    const IntegerValueSource& operator= (const IntegerValueSource&);
};


#endif   // __JUCER_VALUEREMAPPERSOURCE_JUCEHEADER__
