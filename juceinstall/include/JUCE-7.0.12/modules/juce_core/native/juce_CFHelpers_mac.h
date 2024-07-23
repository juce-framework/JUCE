/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

/* This file contains a few helper functions that are used internally but which
   need to be kept away from the public headers because they use obj-C symbols.
*/
namespace juce
{

template <typename CFType>
struct CFObjectDeleter
{
    void operator() (CFType object) const noexcept
    {
        if (object != nullptr)
            CFRelease (object);
    }
};

template <typename CFType>
using CFUniquePtr = std::unique_ptr<std::remove_pointer_t<CFType>, CFObjectDeleter<CFType>>;

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
