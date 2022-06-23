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
class CoreGraphicsMetalLayerRenderer
{
public:
    //==============================================================================
    CoreGraphicsMetalLayerRenderer (CAMetalLayer* layer, const Component& comp)
    {
        device.reset (MTLCreateSystemDefaultDevice());

        layer.device = device.get();
        layer.framebufferOnly = NO;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
        layer.opaque = comp.isOpaque();
        layer.allowsNextDrawableTimeout = NO;

        commandQueue.reset ([device.get() newCommandQueue]);

        memoryBlitEvent.reset ([device.get() newSharedEvent]);
    }

    ~CoreGraphicsMetalLayerRenderer()
    {
        stopGpuCommandSubmission = true;
        [memoryBlitCommandBuffer.get() waitUntilCompleted];
    }

    template <typename Callback>
    bool drawRectangleList (CAMetalLayer* layer,
                            float scaleFactor,
                            CGRect viewFrame,
                            const Component& comp,
                            Callback&& drawRectWithContext,
                            const RectangleList<float>& dirtyRegions)
    {
        if (resources != nullptr)
        {
            // If we haven't finished blitting the CPU texture to the GPU then
            // report that we have been unable to draw anything.
            if (memoryBlitEvent.get().signaledValue != memoryBlitCounter + 1)
                return false;

            ++memoryBlitCounter;
        }

        layer.contentsScale = scaleFactor;
        const auto drawableSizeTansform = CGAffineTransformMakeScale (layer.contentsScale,
                                                                      layer.contentsScale);
        const auto transformedFrameSize = CGSizeApplyAffineTransform (viewFrame.size, drawableSizeTansform);

        const auto componentHeight = comp.getHeight();

        if (! CGSizeEqualToSize (layer.drawableSize, transformedFrameSize))
        {
            layer.drawableSize = transformedFrameSize;
            resources = std::make_unique<Resources> (device.get(), layer, componentHeight);
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

        memoryBlitCommandBuffer.reset ([commandQueue.get() commandBuffer]);

        // Command buffers are usually considered temporary, and are automatically released by
        // the operating system when the rendering pipeline is finsihed. However, we want to keep
        // this one alive so that we can wait for pipeline completion in the destructor.
        [memoryBlitCommandBuffer.get() retain];

        auto blitCommandEncoder = [memoryBlitCommandBuffer.get() blitCommandEncoder];
        [blitCommandEncoder copyFromTexture: sharedTexture
                                sourceSlice: 0
                                sourceLevel: 0
                               sourceOrigin: MTLOrigin{}
                                 sourceSize: MTLSize { sharedTexture.width, sharedTexture.height, 1 }
                                  toTexture: gpuTexture
                           destinationSlice: 0
                           destinationLevel: 0
                          destinationOrigin: MTLOrigin{}];
        [blitCommandEncoder endEncoding];

        // Signal that the GPU has finished using the CPU texture
        [memoryBlitCommandBuffer.get() encodeSignalEvent: memoryBlitEvent.get()
                                                   value: memoryBlitCounter + 1];

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

                auto presentationBlitCommandEncoder = [presentationCommandBuffer blitCommandEncoder];
                [presentationBlitCommandEncoder copyFromTexture: gpuTexture
                                                    sourceSlice: 0
                                                    sourceLevel: 0
                                                   sourceOrigin: MTLOrigin{}
                                                     sourceSize: MTLSize { gpuTexture.width, gpuTexture.height, 1 }
                                                      toTexture: drawable.texture
                                               destinationSlice: 0
                                               destinationLevel: 0
                                              destinationOrigin: MTLOrigin{}];
                [presentationBlitCommandEncoder endEncoding];

                [presentationCommandBuffer addScheduledHandler: ^(id<MTLCommandBuffer>)
                {
                    [drawable present];
                }];

                [presentationCommandBuffer commit];
            }
        }];

        [memoryBlitCommandBuffer.get() commit];

        return true;
    }

private:
    //==============================================================================
    static auto alignTo (size_t n, size_t alignment)
    {
        return ((n + alignment - 1) / alignment) * alignment;
    }

    //==============================================================================
    struct TextureDeleter
    {
        void operator() (id<MTLTexture> texture) const noexcept
        {
            [texture setPurgeableState: MTLPurgeableStateEmpty];
            [texture release];
        }
    };

    using TextureUniquePtr = std::unique_ptr<std::remove_pointer_t<id<MTLTexture>>, TextureDeleter>;

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
                                      [] (const TextureUniquePtr& t) { return [t.get() retainCount] == 1; });
            return iter == textureCache.end() ? nullptr : (*iter).get();
        }

    private:
        std::array<TextureUniquePtr, 3> textureCache;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GpuTexturePool)
        JUCE_DECLARE_NON_MOVEABLE (GpuTexturePool)
    };

    //==============================================================================
    class Resources
    {
    public:
        Resources (id<MTLDevice> metalDevice, CAMetalLayer* layer, int componentHeight)
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

            CGContextScaleCTM (cgContext.get(), layer.contentsScale, layer.contentsScale);
            CGContextConcatCTM (cgContext.get(), CGAffineTransformMake (1, 0, 0, -1, 0, componentHeight));

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
        TextureUniquePtr sharedTexture;
        std::unique_ptr<GpuTexturePool> gpuTexturePool;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Resources)
        JUCE_DECLARE_NON_MOVEABLE (Resources)
    };

    //==============================================================================
    std::unique_ptr<Resources> resources;

    ObjCObjectHandle<id<MTLDevice>> device;
    ObjCObjectHandle<id<MTLCommandQueue>> commandQueue;
    ObjCObjectHandle<id<MTLCommandBuffer>> memoryBlitCommandBuffer;
    ObjCObjectHandle<id<MTLSharedEvent>> memoryBlitEvent;

    uint64_t memoryBlitCounter = 0;
    std::atomic<bool> stopGpuCommandSubmission { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsMetalLayerRenderer)
    JUCE_DECLARE_NON_MOVEABLE (CoreGraphicsMetalLayerRenderer)
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

}
