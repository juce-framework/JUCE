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

class Direct2DPixelData : public ImagePixelData,
                          private DxgiAdapterListener
{
public:
    using Ptr = ReferenceCountedObjectPtr<Direct2DPixelData>;

    static Ptr make (Image::PixelFormat formatToUse,
                     int w,
                     int h,
                     bool clearImageIn,
                     DxgiAdapter::Ptr adapterIn);

    static Ptr fromDirect2DBitmap (ComSmartPtr<ID2D1Bitmap1> bitmap);

    ~Direct2DPixelData() override;

    std::unique_ptr<LowLevelGraphicsContext> createLowLevelContext() override;

    void initialiseBitmapData (Image::BitmapData& bitmap, int x, int y, Image::BitmapData::ReadWriteMode mode) override;

    ImagePixelData::Ptr clone() override;

    void applyGaussianBlurEffect (float radius, Image& result) override;
    void applySingleChannelBoxBlurEffect (int radius, Image& result) override;

    std::unique_ptr<ImageType> createType() const override;

    DxgiAdapter::Ptr getAdapter() const { return adapter; }
    ComSmartPtr<ID2D1Bitmap1> getAdapterD2D1Bitmap();

    void flushToSoftwareBackup();

private:
    Direct2DPixelData (Image::PixelFormat, int, int, bool, DxgiAdapter::Ptr);

    int getPixelStride() const { return pixelFormat == Image::SingleChannel ? 1 : 4; }
    int getLineStride() const { return (getPixelStride() * jmax (1, width) + 3) & ~3; }

    void adapterCreated (DxgiAdapter::Ptr) override;
    void adapterRemoved (DxgiAdapter::Ptr) override;

    void initBitmapDataReadOnly (Image::BitmapData&, int, int);

    ComSmartPtr<ID2D1Bitmap1> createAdapterBitmap() const;
    void createDeviceResources();

    SharedResourcePointer<DirectX> directX;
    const bool clearImage;
    Image backup;
    DxgiAdapter::Ptr adapter;
    ComSmartPtr<ID2D1DeviceContext1> context;
    ComSmartPtr<ID2D1Bitmap1> nativeBitmap;

    JUCE_LEAK_DETECTOR (Direct2DPixelData)
};

} // namespace juce
