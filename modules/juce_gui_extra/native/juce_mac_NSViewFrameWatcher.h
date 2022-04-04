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

#if JUCE_MAC

namespace juce
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
const auto nsViewFrameChangedSelector = @selector (frameChanged:);
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

struct NSViewCallbackInterface
{
    virtual ~NSViewCallbackInterface() = default;
    virtual void frameChanged() = 0;
};

//==============================================================================
struct NSViewFrameChangeCallbackClass   : public ObjCClass<NSObject>
{
    NSViewFrameChangeCallbackClass()
        : ObjCClass ("JUCE_NSViewCallback_")
    {
        addIvar<NSViewCallbackInterface*> ("target");

        addMethod (nsViewFrameChangedSelector, frameChanged);

        registerClass();
    }

    static void setTarget (id self, NSViewCallbackInterface* c)
    {
        object_setInstanceVariable (self, "target", c);
    }

private:
    static void frameChanged (id self, SEL, NSNotification*)
    {
        if (auto* target = getIvar<NSViewCallbackInterface*> (self, "target"))
            target->frameChanged();
    }

    JUCE_DECLARE_NON_COPYABLE (NSViewFrameChangeCallbackClass)
};

//==============================================================================
class NSViewFrameWatcher : private NSViewCallbackInterface
{
public:
    NSViewFrameWatcher (NSView* viewToWatch, std::function<void()> viewResizedIn)
        : viewResized (std::move (viewResizedIn)), callback (makeCallbackForView (viewToWatch))
    {
    }

    ~NSViewFrameWatcher() override
    {
        [[NSNotificationCenter defaultCenter] removeObserver: callback];
        [callback release];
        callback = nil;
    }

    JUCE_DECLARE_NON_COPYABLE (NSViewFrameWatcher)
    JUCE_DECLARE_NON_MOVEABLE (NSViewFrameWatcher)

private:
    id makeCallbackForView (NSView* view)
    {
        static NSViewFrameChangeCallbackClass cls;
        auto* result = [cls.createInstance() init];
        NSViewFrameChangeCallbackClass::setTarget (result, this);

        [[NSNotificationCenter defaultCenter]  addObserver: result
                                                  selector: nsViewFrameChangedSelector
                                                      name: NSViewFrameDidChangeNotification
                                                    object: view];

        return result;
    }

    void frameChanged() override { viewResized(); }

    std::function<void()> viewResized;
    id callback;
};

} // namespace juce

#endif
