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

namespace juce
{

//==============================================================================
/**
    Used by container classes as an indirect way to delete an object of a
    particular type.

    The generic implementation of this class simply calls 'delete', but you can
    create a specialised version of it for a particular class if you need to
    delete that type of object in a more appropriate way.

    @see OwnedArray

    @tags{Core}
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
        // If this was caused by a OwnedArray of a forward-declared type, move the
        // implementation of all methods trying to use the OwnedArray (e.g. the destructor
        // of the class owning it) into cpp files where they can see to the definition
        // of ObjectType. This should fix the error.
        [[maybe_unused]] constexpr auto size = sizeof (ObjectType);

        delete object;
    }
};

} // namespace juce
