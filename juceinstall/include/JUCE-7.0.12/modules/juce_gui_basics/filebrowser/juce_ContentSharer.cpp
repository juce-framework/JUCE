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

ScopedMessageBox ContentSharer::shareFilesScoped (const Array<URL>& files,
                                                  Callback callback,
                                                  Component* parent)
{
    auto impl = detail::ScopedContentSharerInterface::shareFiles (files, parent);
    return detail::ConcreteScopedContentSharerImpl::show (std::move (impl), std::move (callback));
}

ScopedMessageBox ContentSharer::shareTextScoped (const String& text,
                                                 Callback callback,
                                                 Component* parent)
{
    auto impl = detail::ScopedContentSharerInterface::shareText (text, parent);
    return detail::ConcreteScopedContentSharerImpl::show (std::move (impl), std::move (callback));
}

ScopedMessageBox ContentSharer::shareImagesScoped (const Array<Image>& images,
                                                   std::unique_ptr<ImageFileFormat> format,
                                                   Callback callback,
                                                   Component* parent)
{
    auto impl = detail::ScopedContentSharerInterface::shareImages (images, std::move (format), parent);
    return detail::ConcreteScopedContentSharerImpl::show (std::move (impl), std::move (callback));
}

ScopedMessageBox ContentSharer::shareDataScoped (const MemoryBlock& mb,
                                                 Callback callback,
                                                 Component* parent)
{
    auto impl = detail::ScopedContentSharerInterface::shareData (mb, parent);
    return detail::ConcreteScopedContentSharerImpl::show (std::move (impl), std::move (callback));
}

#if ! (JUCE_CONTENT_SHARING && (JUCE_IOS || JUCE_ANDROID))
auto detail::ScopedContentSharerInterface::shareFiles (const Array<URL>&, Component*) -> std::unique_ptr<ScopedContentSharerInterface>
{
    return std::make_unique<detail::ScopedContentSharerInterface>();
}

auto detail::ScopedContentSharerInterface::shareText (const String&, Component*) -> std::unique_ptr<ScopedContentSharerInterface>
{
    return std::make_unique<detail::ScopedContentSharerInterface>();
}
#endif

} // namespace juce
