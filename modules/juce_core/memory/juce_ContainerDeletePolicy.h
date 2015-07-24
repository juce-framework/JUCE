/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_CONTAINERDELETEPOLICY_H_INCLUDED
#define JUCE_CONTAINERDELETEPOLICY_H_INCLUDED

//==============================================================================
/**
    Used by container classes as an indirect way to delete an object of a
    particular type.

    The generic implementation of this class simply calls 'delete', but you can
    create a specialised version of it for a particular class if you need to
    delete that type of object in a more appropriate way.

    @see ScopedPointer, OwnedArray
*/
template <typename ObjectType>
struct ContainerDeletePolicy
{
    static void destroy (ObjectType* object)
    {
        // If the line below triggers a compiler error, it means that you are using
        // an incomplete type for ObjectType (for example, a type that is declared
        // but not defined). This is a problem because then the following delete is
        // undefined behaviour. The purpose of the sizeof is to capture this situation.
        // If this was caused by a ScopedPointer to a forward-declared type, move the
        // implementation of all methods trying to use the ScopedPointer (e.g. the destructor
        // of the class owning it) into cpp files where they can see to the definition
        // of ObjectType. This should fix the error.
        ignoreUnused (sizeof (ObjectType));

        delete object;
    }
};


#endif   // JUCE_CONTAINERDELETEPOLICY_H_INCLUDED
