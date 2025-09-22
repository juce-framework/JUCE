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

#if JUCE_INTERNAL_HAS_AU

#include <juce_audio_processors_headless/format_types/juce_AudioUnitPluginFormatImpl.h>

namespace juce
{

class AudioUnitPluginWindowCocoa final : public AudioProcessorEditor
{
    // Audio Unit plugins expect hosts to listen to their view bounds, and to resize
    // the plugin window/view appropriately.
   #if JUCE_IOS
    using JUCE_IOS_MAC_VIEW = UIView;
    using ViewComponentBaseClass = UIViewComponent;
   #else
    using JUCE_IOS_MAC_VIEW = NSView;
    using ViewComponentBaseClass = NSViewComponent;
   #endif

    struct AutoResizingNSViewComponent final : public ViewComponentBaseClass,
                                               private AsyncUpdater
    {
        void childBoundsChanged (Component*) override  { triggerAsyncUpdate(); }
        void handleAsyncUpdate() override              { resizeToFitView(); }
    };

public:
    AudioUnitPluginWindowCocoa (AudioUnitPluginInstanceHeadless& p, bool createGenericViewIfNeeded)
        : AudioProcessorEditor (&p),
          plugin (p)
    {
        addAndMakeVisible (wrapper);

        setOpaque (true);
        setVisible (true);
        setSize (100, 100);

        createView (createGenericViewIfNeeded);
    }

    ~AudioUnitPluginWindowCocoa() override
    {
        if (wrapper.getView() != nil)
        {
            wrapper.setVisible (false);
            removeChildComponent (&wrapper);
            wrapper.setView (nil);
            plugin.editorBeingDeleted (this);
        }
    }

    void embedViewController (JUCE_IOS_MAC_VIEW* pluginView, [[maybe_unused]] const CGSize& size)
    {
        wrapper.setView (pluginView);
        waitingForViewCallback = false;

      #if JUCE_MAC
        if (pluginView != nil)
            wrapper.resizeToFitView();
      #else
        [pluginView setBounds: CGRectMake (0.f, 0.f, static_cast<int> (size.width), static_cast<int> (size.height))];
        wrapper.setSize (static_cast<int> (size.width), static_cast<int> (size.height));
      #endif
    }

    bool isValid() const        { return wrapper.getView() != nil || waitingForViewCallback; }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void resized() override
    {
        wrapper.setSize (getWidth(), getHeight());
    }

    void childBoundsChanged (Component*) override
    {
        setSize (wrapper.getWidth(), wrapper.getHeight());
    }

private:
    AudioUnitPluginInstanceHeadless& plugin;
    AutoResizingNSViewComponent wrapper;

    typedef void (^ViewControllerCallbackBlock)(AUViewControllerBase *);

    bool waitingForViewCallback = false;

    bool createView ([[maybe_unused]] bool createGenericViewIfNeeded)
    {
        JUCE_IOS_MAC_VIEW* pluginView = nil;
        UInt32 dataSize = 0;
        Boolean isWritable = false;

       #if JUCE_MAC
        if (AudioUnitGetPropertyInfo (plugin.getAudioUnitHandle(), kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, &dataSize, &isWritable) == noErr
             && dataSize != 0
             && AudioUnitGetPropertyInfo (plugin.getAudioUnitHandle(), kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr)
        {
            HeapBlock<AudioUnitCocoaViewInfo> info;
            info.calloc (dataSize, 1);

            if (AudioUnitGetProperty (plugin.getAudioUnitHandle(), kAudioUnitProperty_CocoaUI, kAudioUnitScope_Global,
                                      0, info, &dataSize) == noErr)
            {
                NSString* viewClassName = (NSString*) (info->mCocoaAUViewClass[0]);
                CFUniquePtr<CFStringRef> path (CFURLCopyPath (info->mCocoaAUViewBundleLocation));
                NSString* unescapedPath = (NSString*) CFURLCreateStringByReplacingPercentEscapes (nullptr, path.get(), CFSTR (""));
                NSBundle* viewBundle = [NSBundle bundleWithPath: [unescapedPath autorelease]];
                Class viewClass = [viewBundle classNamed: viewClassName];

                if ([viewClass conformsToProtocol: @protocol (AUCocoaUIBase)]
                     && [viewClass instancesRespondToSelector: @selector (interfaceVersion)]
                     && [viewClass instancesRespondToSelector: @selector (uiViewForAudioUnit: withSize:)])
                {
                    id factory = [[[viewClass alloc] init] autorelease];
                    pluginView = [factory uiViewForAudioUnit: plugin.getAudioUnitHandle()
                                                    withSize: NSMakeSize (getWidth(), getHeight())];
                }

                for (int i = (dataSize - sizeof (CFURLRef)) / sizeof (CFStringRef); --i >= 0;)
                    CFRelease (info->mCocoaAUViewClass[i]);

                CFRelease (info->mCocoaAUViewBundleLocation);
            }
        }
       #endif

        dataSize = 0;
        isWritable = false;

        if (AudioUnitGetPropertyInfo (plugin.getAudioUnitHandle(), kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global,
                                          0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (ViewControllerCallbackBlock))
        {
            waitingForViewCallback = true;
            auto callback = ^(AUViewControllerBase* controller) { this->requestViewControllerCallback (controller); };

            if (noErr == AudioUnitSetProperty (plugin.getAudioUnitHandle(), kAudioUnitProperty_RequestViewController, kAudioUnitScope_Global, 0, &callback, dataSize))
                return true;

            waitingForViewCallback = false;
        }

       #if JUCE_MAC
        if (createGenericViewIfNeeded && (pluginView == nil))
        {
            {
                // This forces CoreAudio.component to be loaded, otherwise the AUGenericView will assert
                AudioComponentDescription desc;
                String name, version, manufacturer;
                AudioUnitFormatHelpers::getComponentDescFromIdentifier ("AudioUnit:Output/auou,genr,appl",
                                                                        desc, name, version, manufacturer);
            }

            pluginView = [[AUGenericView alloc] initWithAudioUnit: plugin.getAudioUnitHandle()];
        }
       #endif

        wrapper.setView (pluginView);

        if (pluginView != nil)
            wrapper.resizeToFitView();

        return pluginView != nil;
    }

    void requestViewControllerCallback (AUViewControllerBase* controller)
    {
        const auto viewSize = [&controller]
        {
            auto size = CGSizeZero;

            size = [controller preferredContentSize];

            if (approximatelyEqual (size.width, 0.0) || approximatelyEqual (size.height, 0.0))
                size = controller.view.frame.size;

            return CGSizeMake (jmax ((CGFloat) 20.0f, size.width),
                               jmax ((CGFloat) 20.0f, size.height));
        }();

        if (! MessageManager::getInstance()->isThisTheMessageThread())
        {
            struct AsyncViewControllerCallback final : public CallbackMessage
            {
                AudioUnitPluginWindowCocoa* owner;
                JUCE_IOS_MAC_VIEW* controllerView;
                CGSize size;

                AsyncViewControllerCallback (AudioUnitPluginWindowCocoa* plugInWindow, JUCE_IOS_MAC_VIEW* inView,
                                             const CGSize& preferredSize)
                    : owner (plugInWindow), controllerView ([inView retain]), size (preferredSize)
                {}

                void messageCallback() override
                {
                    owner->embedViewController (controllerView, size);
                    [controllerView release];
                }
            };

            (new AsyncViewControllerCallback (this, [controller view], viewSize))->post();
        }
        else
        {
            embedViewController ([controller view], viewSize);
        }
    }
};

class AudioUnitPluginInstance final : public AudioUnitPluginInstanceHeadless
{
public:
    using AudioUnitPluginInstanceHeadless::AudioUnitPluginInstanceHeadless;

    bool hasEditor() const override
    {
       #if JUCE_MAC
        return true;
       #else
        UInt32 dataSize;
        Boolean isWritable;

        return (AudioUnitGetPropertyInfo (getAudioUnitHandle(), kAudioUnitProperty_RequestViewController,
                                          kAudioUnitScope_Global, 0, &dataSize, &isWritable) == noErr
                && dataSize == sizeof (uintptr_t) && isWritable != 0);
       #endif
    }

    AudioProcessorEditor* createEditor() override
    {
        std::unique_ptr<AudioProcessorEditor> w (new AudioUnitPluginWindowCocoa (*this, false));

        if (! static_cast<AudioUnitPluginWindowCocoa*> (w.get())->isValid())
            w.reset (new AudioUnitPluginWindowCocoa (*this, true)); // use AUGenericView as a fallback

        return w.release();
    }
};

void AudioUnitPluginFormat::createPluginInstance (const PluginDescription& desc,
                                                  double rate,
                                                  int blockSize,
                                                  PluginCreationCallback callback)
{
    createAudioUnitPluginInstance<AudioUnitPluginInstance> (*this, desc, rate, blockSize, callback);
}

} // namespace juce

#endif
