/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_GradientBrush.h"
#include "../contexts/juce_LowLevelGraphicsContext.h"


//==============================================================================
GradientBrush::GradientBrush (const Colour& colour1,
                              const float x1,
                              const float y1,
                              const Colour& colour2,
                              const float x2,
                              const float y2,
                              const bool isRadial) throw()
    : gradient (colour1, x1, y1,
                colour2, x2, y2,
                isRadial)
{
}

GradientBrush::GradientBrush (const ColourGradient& gradient_) throw()
    : gradient (gradient_)
{
}

GradientBrush::~GradientBrush() throw()
{
}

Brush* GradientBrush::createCopy() const throw()
{
    return new GradientBrush (gradient);
}

void GradientBrush::applyTransform (const AffineTransform& transform) throw()
{
    gradient.transform = gradient.transform.followedBy (transform);
}

void GradientBrush::multiplyOpacity (const float multiple) throw()
{
    gradient.multiplyOpacity (multiple);
}

bool GradientBrush::isInvisible() const throw()
{
    return gradient.isInvisible();
}

//==============================================================================
void GradientBrush::paintPath (LowLevelGraphicsContext& context,
                               const Path& path, const AffineTransform& transform) throw()
{
    context.fillPathWithGradient (path, transform, gradient, EdgeTable::Oversampling_4times);
}

void GradientBrush::paintRectangle (LowLevelGraphicsContext& context,
                                    int x, int y, int w, int h) throw()
{
    context.fillRectWithGradient (x, y, w, h, gradient);
}

void GradientBrush::paintAlphaChannel (LowLevelGraphicsContext& context,
                                       const Image& alphaChannelImage, int imageX, int imageY,
                                       int x, int y, int w, int h) throw()
{
    context.saveState();

    if (context.reduceClipRegion (x, y, w, h))
        context.fillAlphaChannelWithGradient (alphaChannelImage, imageX, imageY, gradient);

    context.restoreState();
}

END_JUCE_NAMESPACE
