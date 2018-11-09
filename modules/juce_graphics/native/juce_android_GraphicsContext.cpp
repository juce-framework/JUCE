/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace GraphicsHelpers
{
    jobject createPaint (Graphics::ResamplingQuality quality)
    {
        jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                                | 4 /*DITHER_FLAG*/
                                | 128 /*SUBPIXEL_TEXT_FLAG*/;

        if (quality > Graphics::lowResamplingQuality)
            constructorFlags |= 2; /*FILTER_BITMAP_FLAG*/

        return getEnv()->NewObject (AndroidPaint, AndroidPaint.constructor, constructorFlags);
    }

    //
    const jobject createMatrix (JNIEnv* env, const AffineTransform& t)
    {
        jobject m = env->NewObject (AndroidMatrix, AndroidMatrix.constructor);

        jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                             t.mat10, t.mat11, t.mat12,
                             0.0f, 0.0f, 1.0f };

        jfloatArray javaArray = env->NewFloatArray (9);
        env->SetFloatArrayRegion (javaArray, 0, 9, values);

        env->CallVoidMethod (m, AndroidMatrix.setValues, javaArray);
        env->DeleteLocalRef (javaArray);

        return m;
    }
}

ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return SoftwareImageType().create (format, width, height, clearImage);
}

} // namespace juce
