/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_AffineTransform.h"


//==============================================================================
AffineTransform::AffineTransform() throw()
    : mat00 (1.0f),
      mat01 (0),
      mat02 (0),
      mat10 (0),
      mat11 (1.0f),
      mat12 (0)
{
}

AffineTransform::AffineTransform (const AffineTransform& other) throw()
  : mat00 (other.mat00),
    mat01 (other.mat01),
    mat02 (other.mat02),
    mat10 (other.mat10),
    mat11 (other.mat11),
    mat12 (other.mat12)
{
}

AffineTransform::AffineTransform (const float mat00_,
                                  const float mat01_,
                                  const float mat02_,
                                  const float mat10_,
                                  const float mat11_,
                                  const float mat12_) throw()
 :  mat00 (mat00_),
    mat01 (mat01_),
    mat02 (mat02_),
    mat10 (mat10_),
    mat11 (mat11_),
    mat12 (mat12_)
{
}

const AffineTransform& AffineTransform::operator= (const AffineTransform& other) throw()
{
    mat00 = other.mat00;
    mat01 = other.mat01;
    mat02 = other.mat02;
    mat10 = other.mat10;
    mat11 = other.mat11;
    mat12 = other.mat12;

    return *this;
}

bool AffineTransform::operator== (const AffineTransform& other) const throw()
{
    return mat00 == other.mat00
        && mat01 == other.mat01
        && mat02 == other.mat02
        && mat10 == other.mat10
        && mat11 == other.mat11
        && mat12 == other.mat12;
}

bool AffineTransform::operator!= (const AffineTransform& other) const throw()
{
    return ! operator== (other);
}

//==============================================================================
bool AffineTransform::isIdentity() const throw()
{
    return (mat01 == 0)
        && (mat02 == 0)
        && (mat10 == 0)
        && (mat12 == 0)
        && (mat00 == 1.0f)
        && (mat11 == 1.0f);
}

const AffineTransform AffineTransform::identity;

//==============================================================================
const AffineTransform AffineTransform::followedBy (const AffineTransform& other) const throw()
{
    return AffineTransform (other.mat00 * mat00 + other.mat01 * mat10,
                            other.mat00 * mat01 + other.mat01 * mat11,
                            other.mat00 * mat02 + other.mat01 * mat12 + other.mat02,
                            other.mat10 * mat00 + other.mat11 * mat10,
                            other.mat10 * mat01 + other.mat11 * mat11,
                            other.mat10 * mat02 + other.mat11 * mat12 + other.mat12);
}

const AffineTransform AffineTransform::followedBy (const float omat00,
                                                   const float omat01,
                                                   const float omat02,
                                                   const float omat10,
                                                   const float omat11,
                                                   const float omat12) const throw()
{
    return AffineTransform (omat00 * mat00 + omat01 * mat10,
                            omat00 * mat01 + omat01 * mat11,
                            omat00 * mat02 + omat01 * mat12 + omat02,
                            omat10 * mat00 + omat11 * mat10,
                            omat10 * mat01 + omat11 * mat11,
                            omat10 * mat02 + omat11 * mat12 + omat12);
}

//==============================================================================
const AffineTransform AffineTransform::translated (const float dx,
                                                   const float dy) const throw()
{
    return AffineTransform (mat00, mat01, mat02 + dx,
                            mat10, mat11, mat12 + dy);
}

const AffineTransform AffineTransform::translation (const float dx,
                                                    const float dy) throw()
{
    return AffineTransform (1.0f, 0, dx,
                            0, 1.0f, dy);
}

const AffineTransform AffineTransform::rotated (const float rad) const throw()
{
    const float cosRad = cosf (rad);
    const float sinRad = sinf (rad);

    return followedBy (cosRad, -sinRad, 0,
                       sinRad, cosRad, 0);
}

const AffineTransform AffineTransform::rotation (const float rad) throw()
{
    const float cosRad = cosf (rad);
    const float sinRad = sinf (rad);

    return AffineTransform (cosRad, -sinRad, 0,
                            sinRad, cosRad, 0);
}

const AffineTransform AffineTransform::rotated (const float angle,
                                                const float pivotX,
                                                const float pivotY) const throw()
{
    return translated (-pivotX, -pivotY)
            .rotated (angle)
            .translated (pivotX, pivotY);
}

const AffineTransform AffineTransform::rotation (const float angle,
                                                 const float pivotX,
                                                 const float pivotY) throw()
{
    return translation (-pivotX, -pivotY)
            .rotated (angle)
            .translated (pivotX, pivotY);
}

const AffineTransform AffineTransform::scaled (const float factorX,
                                               const float factorY) const throw()
{
    return AffineTransform (factorX * mat00, factorX * mat01, factorX * mat02,
                            factorY * mat10, factorY * mat11, factorY * mat12);
}

const AffineTransform AffineTransform::scale (const float factorX,
                                              const float factorY) throw()
{
    return AffineTransform (factorX, 0, 0,
                            0, factorY, 0);
}

const AffineTransform AffineTransform::sheared (const float shearX,
                                                const float shearY) const throw()
{
    return followedBy (1.0f, shearX, 0,
                       shearY, 1.0f, 0);
}

const AffineTransform AffineTransform::inverted() const throw()
{
    double determinant = (mat00 * mat11 - mat10 * mat01);

    if (determinant != 0.0)
    {
        determinant = 1.0 / determinant;

        const float dst00 = (float) (mat11 * determinant);
        const float dst10 = (float) (-mat10 * determinant);
        const float dst01 = (float) (-mat01 * determinant);
        const float dst11 = (float) (mat00 * determinant);

        return AffineTransform (dst00, dst01, -mat02 * dst00 - mat12 * dst01,
                                dst10, dst11, -mat02 * dst10 - mat12 * dst11);
    }
    else
    {
        // singularity..
        return *this;
    }
}

bool AffineTransform::isSingularity() const throw()
{
    return (mat00 * mat11 - mat10 * mat01) == 0.0;
}

bool AffineTransform::isOnlyTranslation() const throw()
{
    return (mat01 == 0)
        && (mat10 == 0)
        && (mat00 == 1.0f)
        && (mat11 == 1.0f);
}

//==============================================================================
void AffineTransform::transformPoint (float& x,
                                      float& y) const throw()
{
    const float oldX = x;
    x = mat00 * oldX + mat01 * y + mat02;
    y = mat10 * oldX + mat11 * y + mat12;
}

void AffineTransform::transformPoint (double& x,
                                      double& y) const throw()
{
    const double oldX = x;
    x = mat00 * oldX + mat01 * y + mat02;
    y = mat10 * oldX + mat11 * y + mat12;
}


END_JUCE_NAMESPACE
