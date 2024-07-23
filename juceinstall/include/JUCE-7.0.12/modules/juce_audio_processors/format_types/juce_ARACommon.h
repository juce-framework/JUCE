/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
