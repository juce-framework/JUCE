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

struct Direct2DImageContext::ImagePimpl : public Direct2DGraphicsContext::Pimpl
{
public:
    static constexpr auto opaque = false;

    ImagePimpl (Direct2DImageContext& ownerIn, Direct2DPixelData::Ptr targetIn)
        : Pimpl (ownerIn, opaque),
          target (targetIn)
    {
        if (target != nullptr)
            adapter = target->getAdapter();
        else
            jassertfalse;
    }

    Rectangle<int> getFrameSize() override
    {
        const auto targetBitmap = getBitmap();

        if (targetBitmap == nullptr)
            return {};

        auto size = targetBitmap->GetSize();
        return Rectangle<float> { size.width, size.height }.getSmallestIntegerContainer();
    }

    ComSmartPtr<ID2D1Image> getDeviceContextTarget() const override
    {
        return getBitmap();
    }

    HRESULT finishFrame() override
    {
        const auto result = Pimpl::finishFrame();

        if (target != nullptr)
            target->flushToSoftwareBackup();

        return result;
    }

private:
    ComSmartPtr<ID2D1Bitmap1> getBitmap() const
    {
        if (target == nullptr)
            return {};

        return target->getAdapterD2D1Bitmap();
    }

    void updatePaintAreas() override
    {
        paintAreas = getFrameSize();
    }

    Direct2DPixelData::Ptr target;

    JUCE_DECLARE_WEAK_REFERENCEABLE (ImagePimpl)
};

//==============================================================================
Direct2DImageContext::Direct2DImageContext (Direct2DPixelData::Ptr targetIn)
    : pimpl (new ImagePimpl { *this, targetIn })
{
   #if JUCE_DIRECT2D_METRICS
    metrics = Direct2DMetricsHub::getInstance()->imageContextMetrics;
   #endif

    startFrame (1.0f);
}

Direct2DImageContext::~Direct2DImageContext()
{
    endFrame();
}

Direct2DGraphicsContext::Pimpl* Direct2DImageContext::getPimpl() const noexcept
{
    return pimpl.get();
}

void Direct2DImageContext::clearTargetBuffer()
{
    // The bitmap was already cleared when it was created; do nothing here
}

} // namespace juce
