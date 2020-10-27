/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

class ContentSharer::ContentSharerNativeImpl    : public ContentSharer::Pimpl,
                                                  private Component
{
public:
    ContentSharerNativeImpl (ContentSharer& cs)
        : owner (cs)
    {
        static PopoverDelegateClass cls;
        popoverDelegate.reset ([cls.createInstance() init]);
    }

    ~ContentSharerNativeImpl() override
    {
        exitModalState (0);
    }

    void shareFiles (const Array<URL>& files) override
    {
        auto urls = [NSMutableArray arrayWithCapacity: (NSUInteger) files.size()];

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
                [urls addObject: [NSURL fileURLWithPath: nativeFilePath]];
        }

        share (urls);
    }

    void shareText (const String& text) override
    {
        auto array = [NSArray arrayWithObject: juceStringToNS (text)];
        share (array);
    }

private:
    void share (NSArray* items)
    {
        if ([items count] == 0)
        {
            jassertfalse;
            owner.sharingFinished (false, "No valid items found for sharing.");
            return;
        }

        controller.reset ([[UIActivityViewController alloc] initWithActivityItems: items
                                                            applicationActivities: nil]);

        controller.get().excludedActivityTypes = nil;

        controller.get().completionWithItemsHandler = ^ (UIActivityType type, BOOL completed,
                                                         NSArray* returnedItems, NSError* error)
        {
            ignoreUnused (type);
            ignoreUnused (returnedItems);

            succeeded = completed;

            if (error != nil)
                errorDescription = nsStringToJuce ([error localizedDescription]);

            exitModalState (0);
        };

        controller.get().modalTransitionStyle = UIModalTransitionStyleCoverVertical;

        auto bounds = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
        setBounds (bounds);

        setAlwaysOnTop (true);
        setVisible (true);
        addToDesktop (0);

        enterModalState (true,
                         ModalCallbackFunction::create ([this] (int)
                         {
                             owner.sharingFinished (succeeded, errorDescription);
                         }),
                         false);
    }

    static bool isIPad()
    {
        return [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
    }

    //==============================================================================
    void parentHierarchyChanged() override
    {
        auto* newPeer = dynamic_cast<UIViewComponentPeer*> (getPeer());

        if (peer != newPeer)
        {
            peer = newPeer;

            if (isIPad())
            {
                controller.get().preferredContentSize = peer->view.frame.size;

                auto screenBounds = [UIScreen mainScreen].bounds;

                auto* popoverController = controller.get().popoverPresentationController;
                popoverController.sourceView = peer->view;
                popoverController.sourceRect = CGRectMake (0.f, screenBounds.size.height - 10.f, screenBounds.size.width, 10.f);
                popoverController.canOverlapSourceViewRect = YES;
                popoverController.delegate = popoverDelegate.get();
            }

            if (auto* parentController = peer->controller)
                [parentController showViewController: controller.get() sender: parentController];
        }
    }

    //==============================================================================
    struct PopoverDelegateClass    : public ObjCClass<NSObject<UIPopoverPresentationControllerDelegate>>
    {
        PopoverDelegateClass()  : ObjCClass<NSObject<UIPopoverPresentationControllerDelegate>> ("PopoverDelegateClass_")
        {
            addMethod (@selector (popoverPresentationController:willRepositionPopoverToRect:inView:), willRepositionPopover, "v@:@@@");

            registerClass();
        }

        //==============================================================================
        static void willRepositionPopover (id, SEL, UIPopoverPresentationController*, CGRect* rect, UIView*)
        {
            auto screenBounds = [UIScreen mainScreen].bounds;

            rect->origin.x = 0.f;
            rect->origin.y = screenBounds.size.height - 10.f;
            rect->size.width = screenBounds.size.width;
            rect->size.height = 10.f;
        }
    };

    ContentSharer& owner;
    UIViewComponentPeer* peer = nullptr;
    std::unique_ptr<UIActivityViewController, NSObjectDeleter> controller;
    std::unique_ptr<NSObject<UIPopoverPresentationControllerDelegate>, NSObjectDeleter> popoverDelegate;

    bool succeeded = false;
    String errorDescription;
};

//==============================================================================
ContentSharer::Pimpl* ContentSharer::createPimpl()
{
    return new ContentSharerNativeImpl (*this);
}

} // namespace juce
