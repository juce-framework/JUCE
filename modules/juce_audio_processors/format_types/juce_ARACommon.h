/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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

template <typename Obj, typename Member, typename... Ts>
constexpr Obj makeARASizedStruct (Member Obj::* member, Ts&&... ts)
{
    return { reinterpret_cast<uintptr_t> (&(static_cast<const Obj*> (nullptr)->*member)) + sizeof (Member),
             std::forward<Ts> (ts)... };
}

} // namespace juce
