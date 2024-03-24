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

extern XContext windowHandleXContext;

/*  Attaches a pointer to a given window, so that it can be retrieved with XFindContext on
    the windowHandleXContext.
*/
class ScopedWindowAssociation
{
public:
    ScopedWindowAssociation() = default;

    ScopedWindowAssociation (void* associatedIn, Window windowIn)
        : associatedPointer ([&]() -> void*
          {
              if (associatedIn == nullptr)
                  return nullptr;

              // If you hit this, there's already a pointer associated with this window.
              const auto display = XWindowSystem::getInstance()->getDisplay();
              jassert (! getAssociatedPointer (display, windowIn).has_value());

              if (X11Symbols::getInstance()->xSaveContext (display,
                                                           static_cast<XID> (windowIn),
                                                           windowHandleXContext,
                                                           unalignedPointerCast<XPointer> (associatedIn)) != 0)
              {
                  jassertfalse;
                  return nullptr;
              }

              return associatedIn;
          }()),
          window (static_cast<XID> (windowIn)) {}

    ScopedWindowAssociation (const ScopedWindowAssociation&) = delete;
    ScopedWindowAssociation& operator= (const ScopedWindowAssociation&) = delete;

    ScopedWindowAssociation (ScopedWindowAssociation&& other) noexcept
        : associatedPointer (std::exchange (other.associatedPointer, nullptr)), window (other.window) {}

    ScopedWindowAssociation& operator= (ScopedWindowAssociation&& other) noexcept
    {
        ScopedWindowAssociation { std::move (other) }.swap (*this);
        return *this;
    }

    ~ScopedWindowAssociation() noexcept
    {
        if (associatedPointer == nullptr)
            return;

        const auto display = XWindowSystem::getInstance()->getDisplay();
        const auto ptr = getAssociatedPointer (display, window);

        if (! ptr.has_value())
        {
            // If you hit this, something else has cleared this association before we were able to.
            jassertfalse;
            return;
        }

        jassert (unalignedPointerCast<XPointer> (associatedPointer) == *ptr);

        if (X11Symbols::getInstance()->xDeleteContext (display, window, windowHandleXContext) != 0)
            jassertfalse;
    }

    bool isValid() const { return associatedPointer != nullptr; }

private:
    static std::optional<XPointer> getAssociatedPointer (Display* display, Window window)
    {
        XPointer ptr{};

        if (X11Symbols::getInstance()->xFindContext (display, window, windowHandleXContext, &ptr) != 0)
            return std::nullopt;

        return ptr;
    }

    void swap (ScopedWindowAssociation& other) noexcept
    {
        std::swap (other.associatedPointer, associatedPointer);
        std::swap (other.window, window);
    }

    void* associatedPointer = nullptr;
    XID window{};
};

} // namespace juce
