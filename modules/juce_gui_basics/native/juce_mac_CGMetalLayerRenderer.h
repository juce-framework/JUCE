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

// The CoreGraphicsMetalLayerRenderer requires macOS 10.14 and iOS 12.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunguarded-availability", "-Wunguarded-availability-new")

namespace juce
{

//==============================================================================
template <typename ViewType>
class CoreGraphicsMetalLayerRenderer
{
public:
    //==============================================================================
    static auto create (ViewType* view, bool isOpaque)
    {
        ObjCObjectHandle<id<MTLDevice>> device { MTLCreateSystemDefaultDevice() };
        return rawToUniquePtr (device != nullptr ? new CoreGraphicsMetalLayerRenderer (device, view, isOpaque)
                                                 : nullptr);
    }

    ~CoreGraphicsMetalLayerRenderer()
    {
        if (memoryBlitCommandBuffer != nullptr)
        {
            stopGpuCommandSubmission = true;
            [memoryBlitCommandBuffer.get() waitUntilCompleted];
        }
    }

    void attach (ViewType* view, bool isOpaque)
    {
       #if JUCE_MAC
        view.wantsLayer = YES;
        view.layerContentsPlacement = NSViewLayerContentsPlacementTopLeft;
        view.layer = [CAMetalLayer layer];
       #endif

        auto layer = (CAMetalLayer*) view.layer;

        layer.device = device.get();
        layer.framebufferOnly = NO;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        layer.opaque = isOpaque;
        layer.allowsNextDrawableTimeout = NO;

        attachedView = view;
        doSynchronousRender = true;
    }

    void detach()
    {
       #if JUCE_MAC
        attachedView.wantsLayer = NO;
        attachedView.layer = nil;
       #endif

        attachedView = nullptr;
    }

    bool isAttachedToView (ViewType* view) const
    {
        return view == attachedView && attachedView != nullptr;
    }

    template <typename Callback>
    bool drawRectangleList (ViewType* view,
                            float scaleFactor,
                            Callback&& drawRectWithContext,
                            const RectangleList<float>& dirtyRegions)
    {
        auto layer = (CAMetalLayer*) view.layer;

        if (memoryBlitCommandBuffer != nullptr)
        {
            switch ([memoryBlitCommandBuffer.get() status])
            {
                case MTLCommandBufferStatusNotEnqueued:
                case MTLCommandBufferStatusEnqueued:
                case MTLCommandBufferStatusCommitted:
                case MTLCommandBufferStatusScheduled:
                    // If we haven't finished blitting the CPU texture to the GPU then
                    // report that we have been unable to draw anything.
                    return false;
                case MTLCommandBufferStatusCompleted:
                case MTLCommandBufferStatusError:
                    break;
            }
        }

        layer.contentsScale = scaleFactor;
        const auto drawableSizeTansform = CGAffineTransformMakeScale (layer.contentsScale,
                                                                      layer.contentsScale);
        const auto transformedFrameSize = CGSizeApplyAffineTransform (view.frame.size, drawableSizeTansform);

        if (resources == nullptr || ! CGSizeEqualToSize (layer.drawableSize, transformedFrameSize))
        {
            layer.drawableSize = transformedFrameSize;
            resources = std::make_unique<Resources> (device.get(), layer);
        }

        auto gpuTexture = resources->getGpuTexture();

        if (gpuTexture == nullptr)
        {
            jassertfalse;
            return false;
        }

        auto cgContext = resources->getCGContext();

        for (auto rect : dirtyRegions)
        {
            const auto cgRect = convertToCGRect (rect);

            CGContextSaveGState (cgContext);

            CGContextClipToRect (cgContext, cgRect);
            drawRectWithContext (cgContext, cgRect);

            CGContextRestoreGState (cgContext);
        }

        resources->signalBufferModifiedByCpu();

        auto sharedTexture = resources->getSharedTexture();

        auto encodeBlit = [] (id<MTLCommandBuffer> commandBuffer,
                              id<MTLTexture> source,
                              id<MTLTexture> destination)
        {
            auto blitCommandEncoder = [commandBuffer blitCommandEncoder];
            [blitCommandEncoder copyFromTexture: source
                                    sourceSlice: 0
                                    sourceLevel: 0
                                   sourceOrigin: MTLOrigin{}
                                     sourceSize: MTLSize { source.width, source.height, 1 }
                                      toTexture: destination
                               destinationSlice: 0
                               destinationLevel: 0
                              destinationOrigin: MTLOrigin{}];
            [blitCommandEncoder endEncoding];
        };

        if (doSynchronousRender)
        {
            @autoreleasepool
            {
                id<MTLCommandBuffer> commandBuffer = [commandQueue.get() commandBuffer];

                id<CAMetalDrawable> drawable = [layer nextDrawable];
                encodeBlit (commandBuffer, sharedTexture, drawable.texture);

                [commandBuffer presentDrawable: drawable];
                [commandBuffer commit];
            }

            doSynchronousRender = false;
        }
        else
        {
            // Command buffers are usually considered temporary, and are automatically released by
            // the operating system when the rendering pipeline is finsihed. However, we want to keep
            // this one alive so that we can wait for pipeline completion in the destructor.
            memoryBlitCommandBuffer.reset ([[commandQueue.get() commandBuffer] retain]);

            encodeBlit (memoryBlitCommandBuffer.get(), sharedTexture, gpuTexture);

            [memoryBlitCommandBuffer.get() addScheduledHandler: ^(id<MTLCommandBuffer>)
            {
                // We're on a Metal thread, so we can make a blocking nextDrawable call
                // without stalling the message thread.

                // Check if we can do an early exit.
                if (stopGpuCommandSubmission)
                    return;

                @autoreleasepool
                {
                    id<CAMetalDrawable> drawable = [layer nextDrawable];

                    id<MTLCommandBuffer> presentationCommandBuffer = [commandQueue.get() commandBuffer];

                    encodeBlit (presentationCommandBuffer, gpuTexture, drawable.texture);

                    [presentationCommandBuffer addScheduledHandler: ^(id<MTLCommandBuffer>)
                    {
                        [drawable present];
                    }];

                    [presentationCommandBuffer commit];
                }
            }];

            [memoryBlitCommandBuffer.get() commit];
        }

        return true;
    }

private:
    //==============================================================================
    CoreGraphicsMetalLayerRenderer (ObjCObjectHandle<id<MTLDevice>> mtlDevice,
                                    ViewType* view,
                                    bool isOpaque)
        : device (mtlDevice),
          commandQueue ([device.get() newCommandQueue])
    {
        attach (view, isOpaque);
    }

    //==============================================================================
    static auto alignTo (size_t n, size_t alignment)
    {
        return ((n + alignment - 1) / alignment) * alignment;
    }

    //==============================================================================
    class GpuTexturePool
    {
    public:
        GpuTexturePool (id<MTLDevice> metalDevice, MTLTextureDescriptor* descriptor)
        {
            for (auto& t : textureCache)
                t.reset ([metalDevice newTextureWithDescriptor: descriptor]);
        }

        id<MTLTexture> take() const
        {
            auto iter = std::find_if (textureCache.begin(), textureCache.end(),
                                      [] (const ObjCObjectHandle<id<MTLTexture>>& t) { return [t.get() retainCount] == 1; });
            return iter == textureCache.end() ? nullptr : (*iter).get();
        }

    private:
        std::array<ObjCObjectHandle<id<MTLTexture>>, 3> textureCache;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GpuTexturePool)
        JUCE_DECLARE_NON_MOVEABLE (GpuTexturePool)
    };

    //==============================================================================
    class Resources
    {
    public:
        Resources (id<MTLDevice> metalDevice, CAMetalLayer* layer)
        {
            const auto bytesPerRow = alignTo ((size_t) layer.drawableSize.width * 4, 256);

            const auto allocationSize = cpuRenderMemory.ensureSize (bytesPerRow * (size_t) layer.drawableSize.height);

            buffer.reset ([metalDevice newBufferWithBytesNoCopy: cpuRenderMemory.get()
                                                         length: allocationSize
                                                        options:
                                                                #if JUCE_MAC
                                                                 MTLResourceStorageModeManaged
                                                                #else
                                                                 MTLResourceStorageModeShared
                                                                #endif
                                                    deallocator: nullptr]);

            auto* textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: layer.pixelFormat
                                                                                   width: (NSUInteger) layer.drawableSize.width
                                                                                  height: (NSUInteger) layer.drawableSize.height
                                                                               mipmapped: NO];
            textureDesc.storageMode =
                                     #if JUCE_MAC
                                      MTLStorageModeManaged;
                                     #else
                                      MTLStorageModeShared;
                                     #endif
            textureDesc.usage = MTLTextureUsageShaderRead;

            sharedTexture.reset ([buffer.get() newTextureWithDescriptor: textureDesc
                                                                 offset: 0
                                                            bytesPerRow: bytesPerRow]);

            cgContext.reset (CGBitmapContextCreate (cpuRenderMemory.get(),
                                                    (size_t) layer.drawableSize.width,
                                                    (size_t) layer.drawableSize.height,
                                                    8, // Bits per component
                                                    bytesPerRow,
                                                    CGColorSpaceCreateWithName (kCGColorSpaceSRGB),
                                                    (uint32_t) kCGImageAlphaPremultipliedFirst | (uint32_t) kCGBitmapByteOrder32Host));

            CGContextTranslateCTM (cgContext.get(), 0, layer.drawableSize.height);
            CGContextScaleCTM (cgContext.get(), layer.contentsScale, -layer.contentsScale);

            textureDesc.storageMode = MTLStorageModePrivate;
            gpuTexturePool = std::make_unique<GpuTexturePool> (metalDevice, textureDesc);
        }

        CGContextRef getCGContext() const noexcept       { return cgContext.get(); }
        id<MTLTexture> getSharedTexture() const noexcept { return sharedTexture.get(); }
        id<MTLTexture> getGpuTexture() noexcept          { return gpuTexturePool == nullptr ? nullptr : gpuTexturePool->take(); }

        void signalBufferModifiedByCpu()
        {
           #if JUCE_MAC
            [buffer.get() didModifyRange: { 0, buffer.get().length }];
           #endif
        }

    private:
        class AlignedMemory
        {
        public:
            AlignedMemory() = default;

            void* get()
            {
                return allocation != nullptr ? allocation->data : nullptr;
            }

            size_t ensureSize (size_t newSize)
            {
                const auto alignedSize = alignTo (newSize, pagesize);

                if (alignedSize > size)
                {
                    size = std::max (alignedSize, alignTo ((size_t) (size * growthFactor), pagesize));
                    allocation = std::make_unique<AllocationWrapper> (pagesize, size);
                }

                return size;
            }

        private:
            static constexpr float growthFactor = 1.3f;

            const size_t pagesize = (size_t) getpagesize();

            struct AllocationWrapper
            {
                AllocationWrapper (size_t alignment, size_t allocationSize)
                {
                    if (posix_memalign (&data, alignment, allocationSize) != 0)
                        jassertfalse;
                }

                ~AllocationWrapper()
                {
                    ::free (data);
                }

                void* data = nullptr;
            };

            std::unique_ptr<AllocationWrapper> allocation;
            size_t size = 0;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlignedMemory)
            JUCE_DECLARE_NON_MOVEABLE (AlignedMemory)
        };

        AlignedMemory cpuRenderMemory;

        detail::ContextPtr cgContext;

        ObjCObjectHandle<id<MTLBuffer>> buffer;
        ObjCObjectHandle<id<MTLTexture>> sharedTexture;
        std::unique_ptr<GpuTexturePool> gpuTexturePool;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Resources)
        JUCE_DECLARE_NON_MOVEABLE (Resources)
    };

    //==============================================================================
    ViewType* attachedView = nullptr;
    bool doSynchronousRender = false;

    std::unique_ptr<Resources> resources;

    ObjCObjectHandle<id<MTLDevice>> device;
    ObjCObjectHandle<id<MTLCommandQueue>> commandQueue;
    ObjCObjectHandle<id<MTLCommandBuffer>> memoryBlitCommandBuffer;

    std::atomic<bool> stopGpuCommandSubmission { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsMetalLayerRenderer)
    JUCE_DECLARE_NON_MOVEABLE (CoreGraphicsMetalLayerRenderer)
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

}
