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

/* This file contains a few helper functions that are used internally but which
   need to be kept away from the public headers because they use obj-C symbols.
*/
namespace juce
{

struct CFObjectDeleter
{
    void operator() (CFTypeRef object) const noexcept
    {
        if (object != nullptr)
            CFRelease (object);
    }
};

template <typename CFType>
struct CFRefRemover
{
    static_assert (std::is_pointer_v<CFType>, "CFType must be a Ref type");
    using Type = std::remove_pointer_t<CFType>;
};

template <typename CFType>
using CFRemoveRef = typename CFRefRemover<CFType>::Type;

template <typename CFType>
using CFUniquePtr = std::unique_ptr<CFRemoveRef<CFType>, CFObjectDeleter>;

template <typename CFType>
struct CFObjectHolder
{
    CFObjectHolder() = default;
    explicit CFObjectHolder (CFType obj)  : object (obj) {}

    CFObjectHolder (const CFObjectHolder&) = delete;
    CFObjectHolder (CFObjectHolder&&) = delete;

    CFObjectHolder& operator= (const CFObjectHolder&) = delete;
    CFObjectHolder& operator= (CFObjectHolder&&) = delete;

    ~CFObjectHolder() noexcept
    {
        if (object != nullptr)
            CFRelease (object);
    }

    // Public to facilitate passing the pointer address to functions
    CFType object = nullptr;
};

} // namespace juce
