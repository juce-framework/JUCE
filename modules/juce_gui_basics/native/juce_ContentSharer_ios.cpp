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

class NativeScopedContentSharerInterface final : public detail::ScopedContentSharerInterface,
                                                 public detail::NativeModalWrapperComponent
{
public:
    NativeScopedContentSharerInterface (Component* parentIn, NSUniquePtr<NSArray> itemsIn)
        : parent (parentIn), items (std::move (itemsIn)) {}

    void runAsync (std::function<void (bool, const String&)> callback) override
    {
        if ([items.get() count] == 0)
        {
            jassertfalse;
            NullCheckedInvocation::invoke (callback, false, "No valid items found for sharing.");
            return;
        }

        controller.reset ([[UIActivityViewController alloc] initWithActivityItems: items.get()
                                                            applicationActivities: nil]);

        controller.get().excludedActivityTypes = nil;

        controller.get().completionWithItemsHandler = ^([[maybe_unused]] UIActivityType type, BOOL completed,
                                                        [[maybe_unused]] NSArray* returnedItems, NSError* error)
        {
            const auto errorDescription = error != nil ? nsStringToJuce ([error localizedDescription])
                                                       : String();
            exitModalState (0);

            NullCheckedInvocation::invoke (callback, completed && errorDescription.isEmpty(), errorDescription);
        };

        displayNativeWindowModally (parent);

        enterModalState (true, nullptr, false);
    }

    void close() override
    {
        [controller.get() dismissViewControllerAnimated: YES completion: nil];
    }

private:
    UIViewController* getViewController() const override { return controller.get(); }

    Component* parent = nullptr;
    NSUniquePtr<UIActivityViewController> controller;
    NSUniquePtr<NSArray> items;
};

auto detail::ScopedContentSharerInterface::shareFiles (const Array<URL>& files, Component* parent) -> std::unique_ptr<ScopedContentSharerInterface>
{
    NSUniquePtr<NSMutableArray> urls ([[NSMutableArray arrayWithCapacity: (NSUInteger) files.size()] retain]);

    for (const auto& f : files)
    {
        NSString* nativeFilePath = nil;

        if (f.isLocalFile())
        {
            nativeFilePath = juceStringToNS (f.getLocalFile().getFullPathName());
        }
        else
        {
            auto filePath = f.toString (false);

            auto* fileDirectory = filePath.contains ("/")
                                ? juceStringToNS (filePath.upToLastOccurrenceOf ("/", false, false))
                                : [NSString string];

            auto fileName = juceStringToNS (filePath.fromLastOccurrenceOf ("/", false, false)
                                            .upToLastOccurrenceOf (".", false, false));

            auto fileExt = juceStringToNS (filePath.fromLastOccurrenceOf (".", false, false));

            if ([fileDirectory length] == NSUInteger (0))
                nativeFilePath = [[NSBundle mainBundle] pathForResource: fileName
                                                                 ofType: fileExt];
            else
                nativeFilePath = [[NSBundle mainBundle] pathForResource: fileName
                                                                 ofType: fileExt
                                                            inDirectory: fileDirectory];
        }

        if (nativeFilePath != nil)
            [urls.get() addObject: [NSURL fileURLWithPath: nativeFilePath]];
    }

    return std::make_unique<NativeScopedContentSharerInterface> (parent, std::move (urls));
}

auto detail::ScopedContentSharerInterface::shareText (const String& text, Component* parent) -> std::unique_ptr<ScopedContentSharerInterface>
{
    NSUniquePtr<NSArray> array ([[NSArray arrayWithObject: juceStringToNS (text)] retain]);
    return std::make_unique<NativeScopedContentSharerInterface> (parent, std::move (array));
}

} // namespace juce
