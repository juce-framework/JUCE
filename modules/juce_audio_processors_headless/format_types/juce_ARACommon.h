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

namespace ARA
{
    struct ARAFactory;
}

namespace juce
{

/** Encapsulates an ARAFactory pointer and makes sure that it remains in a valid state
    for the lifetime of the ARAFactoryWrapper object.

    @tags{ARA}
*/
class ARAFactoryWrapper
{
public:
    ARAFactoryWrapper() = default;

    /** @internal

        Used by the framework to encapsulate ARAFactory pointers loaded from plugins.
    */
    explicit ARAFactoryWrapper (std::shared_ptr<const ARA::ARAFactory> factoryIn) : factory (std::move (factoryIn)) {}

    /** Returns the contained ARAFactory pointer, which can be a nullptr.

        The validity of the returned pointer is only guaranteed for the lifetime of this wrapper.
    */
    const ARA::ARAFactory* get() const noexcept { return factory.get(); }

private:
    std::shared_ptr<const ARA::ARAFactory> factory;
};

/** Represents the result of AudioPluginFormatManager::createARAFactoryAsync().

    If the operation fails then #araFactory will contain `nullptr`, and #errorMessage may
    contain a reason for the failure.

    The araFactory member ensures that the module necessary for the correct functioning
    of the factory will remain loaded.

    @tags{ARA}
*/
struct ARAFactoryResult
{
    ARAFactoryWrapper araFactory;
    String errorMessage;
};

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmissing-field-initializers")

template <typename Obj, typename Member, typename... Ts>
constexpr Obj makeARASizedStruct (Member Obj::* member, Ts&&... ts)
{
    return { reinterpret_cast<uintptr_t> (&(static_cast<const Obj*> (nullptr)->*member)) + sizeof (Member),
             std::forward<Ts> (ts)... };
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
