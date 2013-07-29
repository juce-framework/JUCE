/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

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
