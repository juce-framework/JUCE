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
    ImagePimpl (Direct2DImageContext& ownerIn,
                ComSmartPtr<ID2D1DeviceContext1> contextIn,
                ComSmartPtr<ID2D1Bitmap1> bitmapIn,
                const RectangleList<int>& paintAreasIn)
        : Pimpl (ownerIn),
          context (std::move (contextIn)),
          bitmap (std::move (bitmapIn)),
          paintAreas (paintAreasIn)
    {
    }

    Rectangle<int> getFrameSize() const override
    {
        if (bitmap == nullptr)
            return {};

        const auto size = bitmap->GetSize();
        return { (int) size.width, (int) size.height };
    }

    ComSmartPtr<ID2D1DeviceContext1> getDeviceContext() const override
    {
        return context;
    }

    ComSmartPtr<ID2D1Image> getDeviceContextTarget() const override
    {
        return bitmap;
    }

    RectangleList<int> getPaintAreas() const override
    {
        return paintAreas;
    }

private:
    ComSmartPtr<ID2D1DeviceContext1> context;
    ComSmartPtr<ID2D1Bitmap1> bitmap;
    RectangleList<int> paintAreas;

    JUCE_DECLARE_WEAK_REFERENCEABLE (ImagePimpl)
};

//==============================================================================
Direct2DImageContext::Direct2DImageContext (ComSmartPtr<ID2D1DeviceContext1> context,
                                            ComSmartPtr<ID2D1Bitmap1> bitmap,
                                            const RectangleList<int>& paintAreas)
    : pimpl (new ImagePimpl { *this, context, bitmap, paintAreas })
{
   #if JUCE_DIRECT2D_METRICS
    metrics = Direct2DMetricsHub::getInstance()->imageContextMetrics;
   #endif
}

Direct2DImageContext::~Direct2DImageContext() = default;

ComSmartPtr<ID2D1DeviceContext1> Direct2DImageContext::getDeviceContext() const
{
    return getPimpl()->getDeviceContext();
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
