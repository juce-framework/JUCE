/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_SCOPEDVALUESETTER_H_INCLUDED
#define JUCE_SCOPEDVALUESETTER_H_INCLUDED


//==============================================================================
/**
    Helper class providing an RAII-based mechanism for temporarily setting and
    then re-setting a value.

    E.g. @code
    int x = 1;

    {
        ScopedValueSetter setter (x, 2);

        // x is now 2
    }

    // x is now 1 again

    {
        ScopedValueSetter setter (x, 3, 4);

        // x is now 3
    }

    // x is now 4
    @endcode

*/
template <typename ValueType>
class ScopedValueSetter
{
public:
    /** Creates a ScopedValueSetter that will immediately change the specified value to the
        given new value, and will then reset it to its original value when this object is deleted.
    */
    ScopedValueSetter (ValueType& valueToSet,
                       ValueType newValue)
        : value (valueToSet),
          originalValue (valueToSet)
    {
        valueToSet = newValue;
    }

    /** Creates a ScopedValueSetter that will immediately change the specified value to the
        given new value, and will then reset it to be valueWhenDeleted when this object is deleted.
    */
    ScopedValueSetter (ValueType& valueToSet,
                       ValueType newValue,
                       ValueType valueWhenDeleted)
        : value (valueToSet),
          originalValue (valueWhenDeleted)
    {
        valueToSet = newValue;
    }

    ~ScopedValueSetter()
    {
        value = originalValue;
    }

private:
    //==============================================================================
    ValueType& value;
    const ValueType originalValue;

    JUCE_DECLARE_NON_COPYABLE (ScopedValueSetter)
};


#endif   // JUCE_SCOPEDVALUESETTER_H_INCLUDED
