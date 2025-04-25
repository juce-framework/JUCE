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

struct Direct2DGraphicsContext::Pimpl : private DxgiAdapterListener
{
public:
    explicit Pimpl (Direct2DGraphicsContext& ownerIn);
    ~Pimpl() override;

    virtual SavedState* startFrame();
    virtual HRESULT finishFrame();

    virtual bool prepare();
    virtual void teardown();
    virtual bool checkPaintReady();

    virtual RectangleList<int> getPaintAreas() const = 0;
    virtual Rectangle<int> getFrameSize() const = 0;
    virtual ComSmartPtr<ID2D1DeviceContext1> getDeviceContext() const = 0;
    virtual ComSmartPtr<ID2D1Image> getDeviceContextTarget() const = 0;

    SavedState* getCurrentSavedState() const;
    SavedState* pushFirstSavedState (Rectangle<int> initialClipRegion);

    SavedState* pushSavedState();
    SavedState* popSavedState();

    void popAllSavedStates();

    void setDeviceContextTransform (AffineTransform transform);
    void resetDeviceContextTransform();

    DxgiAdapter::Ptr getDefaultAdapter() const
    {
        return directX->adapters.getDefaultAdapter();
    }

    auto getDirect2DFactory() const
    {
        return directX->getD2DFactory();
    }

    auto getDirectWriteFactory() const
    {
        return directWrite->getDWriteFactory();
    }

    auto getDirectWriteFactory4() const
    {
        return directWrite->getDWriteFactory4();
    }

    auto& getFontCollection() const
    {
        return directWrite->getFonts();
    }

    uint64_t getFrameId() const
    {
        return owner.getFrameId();
    }

    Direct2DMetrics::Ptr getMetrics() const
    {
        return owner.metrics;
    }

    bool fillSpriteBatch (const RectangleList<float>& list);

    DirectWriteGlyphRun glyphRun;

private:
    static void resetTransform (ID2D1DeviceContext1* context);
    static void setTransform (ID2D1DeviceContext1* context, AffineTransform newTransform);

    DxgiAdapter::Ptr findAdapter() const;

    void adapterCreated (DxgiAdapter::Ptr newAdapter) override;
    void adapterRemoved (DxgiAdapter::Ptr expiringAdapter) override;

    Direct2DGraphicsContext& owner;
    SharedResourcePointer<DirectX> directX;
    SharedResourcePointer<Direct2DFactories> directWrite;

    std::optional<Direct2DDeviceResources> deviceResources;

    std::vector<std::unique_ptr<Direct2DGraphicsContext::SavedState>> savedClientStates;

   #if JUCE_DIRECT2D_METRICS
    int64 paintStartTicks = 0;
   #endif

    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)
};

} // namespace juce
