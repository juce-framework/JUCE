/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
