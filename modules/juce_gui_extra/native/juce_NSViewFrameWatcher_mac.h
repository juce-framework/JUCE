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
