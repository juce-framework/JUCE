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

namespace juce
{

//==============================================================================
ScopedMessageBox::ScopedMessageBox() = default;

ScopedMessageBox::ScopedMessageBox (std::shared_ptr<detail::ScopedMessageBoxImpl> i)
    : impl (std::move (i)) {}

ScopedMessageBox::~ScopedMessageBox() noexcept
{
    close();
}

ScopedMessageBox::ScopedMessageBox (ScopedMessageBox&& other) noexcept
    : impl (std::exchange (other.impl, nullptr)) {}

ScopedMessageBox& ScopedMessageBox::operator= (ScopedMessageBox&& other) noexcept
{
    ScopedMessageBox temp (std::move (other));
    std::swap (temp.impl, impl);
    return *this;
}

void ScopedMessageBox::close()
{
    if (impl != nullptr)
        impl->close();

    impl.reset();
}

} // namespace juce
