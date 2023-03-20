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

namespace juce::detail
{

/**
    Sets up a native control to be hosted on top of a JUCE component.
*/
class NativeModalWrapperComponent : public Component
{
public:
    void parentHierarchyChanged() final
    {
        auto* newPeer = dynamic_cast<UIViewComponentPeer*> (getPeer());

        if (std::exchange (peer, newPeer) == newPeer)
            return;

        if (peer == nullptr)
            return;

        if (isIPad())
        {
            getViewController().preferredContentSize = peer->view.frame.size;

            if (auto* popoverController = getViewController().popoverPresentationController)
            {
                popoverController.sourceView = peer->view;
                popoverController.sourceRect = CGRectMake (0.f, getHeight() - 10.f, getWidth(), 10.f);
                popoverController.canOverlapSourceViewRect = YES;
                popoverController.delegate = popoverDelegate.get();
            }
        }

        if (auto* parentController = peer->controller)
            [parentController showViewController: getViewController() sender: parentController];

        peer->toFront (false);
    }

    void displayNativeWindowModally (Component* parent)
    {
        setOpaque (false);

        if (parent != nullptr)
        {
            [getViewController() setModalPresentationStyle: UIModalPresentationPageSheet];

            setBounds (parent->getLocalBounds());

            setAlwaysOnTop (true);
            parent->addAndMakeVisible (this);
        }
        else
        {
            if (SystemStats::isRunningInAppExtensionSandbox())
            {
                // Opening a native top-level window in an AUv3 is not allowed (sandboxing). You need to specify a
                // parent component (for example your editor) to parent the native file chooser window. To do this
                // specify a parent component in the FileChooser's constructor!
                jassertfalse;
                return;
            }

            auto chooserBounds = Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
            setBounds (chooserBounds);

            setAlwaysOnTop (true);
            setVisible (true);
            addToDesktop (0);
        }
    }

private:
    virtual UIViewController* getViewController() const = 0;

    static bool isIPad()
    {
        return [UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPad;
    }

    struct PopoverDelegateClass : public ObjCClass<NSObject<UIPopoverPresentationControllerDelegate>>
    {
        PopoverDelegateClass()
            : ObjCClass ("PopoverDelegateClass_")
        {
            addMethod (@selector (popoverPresentationController:willRepositionPopoverToRect:inView:), [] (id, SEL, UIPopoverPresentationController*, CGRect* rect, UIView*)
            {
                auto screenBounds = [UIScreen mainScreen].bounds;

                rect->origin.x = 0.f;
                rect->origin.y = screenBounds.size.height - 10.f;
                rect->size.width = screenBounds.size.width;
                rect->size.height = 10.f;
            });

            registerClass();
        }
    };

    UIViewComponentPeer* peer = nullptr;
    NSUniquePtr<NSObject<UIPopoverPresentationControllerDelegate>> popoverDelegate { []
    {
        static PopoverDelegateClass cls;
        return cls.createInstance();
    }() };
};

} // namespace juce::detail
