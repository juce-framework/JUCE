/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

namespace GraphicsHelpers
{
    jobject createPaint (Graphics::ResamplingQuality quality)
    {
        jint constructorFlags = 1 /*ANTI_ALIAS_FLAG*/
                                | 4 /*DITHER_FLAG*/
                                | 128 /*SUBPIXEL_TEXT_FLAG*/;

        if (quality > Graphics::lowResamplingQuality)
            constructorFlags |= 2; /*FILTER_BITMAP_FLAG*/

        return getEnv()->NewObject (Paint, Paint.constructor, constructorFlags);
    }

    const jobject createMatrix (JNIEnv* env, const AffineTransform& t)
    {
        jobject m = env->NewObject (Matrix, Matrix.constructor);

        jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                             t.mat10, t.mat11, t.mat12,
                             0.0f, 0.0f, 1.0f };

        jfloatArray javaArray = env->NewFloatArray (9);
        env->SetFloatArrayRegion (javaArray, 0, 9, values);

        env->CallVoidMethod (m, Matrix.setValues, javaArray);
        env->DeleteLocalRef (javaArray);

        return m;
    }
}

ImagePixelData::Ptr NativeImageType::create (Image::PixelFormat format, int width, int height, bool clearImage) const
{
    return SoftwareImageType().create (format, width, height, clearImage);
}
