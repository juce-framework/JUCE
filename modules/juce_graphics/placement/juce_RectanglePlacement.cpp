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

bool RectanglePlacement::operator== (const RectanglePlacement& other) const noexcept
{
    return flags == other.flags;
}

bool RectanglePlacement::operator!= (const RectanglePlacement& other) const noexcept
{
    return flags != other.flags;
}

void RectanglePlacement::applyTo (double& x, double& y, double& w, double& h,
                                  const double dx, const double dy, const double dw, const double dh) const noexcept
{
    if (w == 0.0 || h == 0.0)
        return;

    if ((flags & stretchToFit) != 0)
    {
        x = dx;
        y = dy;
        w = dw;
        h = dh;
    }
    else
    {
        double scale = (flags & fillDestination) != 0 ? jmax (dw / w, dh / h)
                                                      : jmin (dw / w, dh / h);

        if ((flags & onlyReduceInSize) != 0)
            scale = jmin (scale, 1.0);

        if ((flags & onlyIncreaseInSize) != 0)
            scale = jmax (scale, 1.0);

        w *= scale;
        h *= scale;

        if ((flags & xLeft) != 0)
            x = dx;
        else if ((flags & xRight) != 0)
            x = dx + dw - w;
        else
            x = dx + (dw - w) * 0.5;

        if ((flags & yTop) != 0)
            y = dy;
        else if ((flags & yBottom) != 0)
            y = dy + dh - h;
        else
            y = dy + (dh - h) * 0.5;
    }
}

AffineTransform RectanglePlacement::getTransformToFit (const Rectangle<float>& source, const Rectangle<float>& destination) const noexcept
{
    if (source.isEmpty())
        return AffineTransform();

    float newX = destination.getX();
    float newY = destination.getY();

    float scaleX = destination.getWidth() / source.getWidth();
    float scaleY = destination.getHeight() / source.getHeight();

    if ((flags & stretchToFit) == 0)
    {
        scaleX = (flags & fillDestination) != 0 ? jmax (scaleX, scaleY)
                                                : jmin (scaleX, scaleY);

        if ((flags & onlyReduceInSize) != 0)
            scaleX = jmin (scaleX, 1.0f);

        if ((flags & onlyIncreaseInSize) != 0)
            scaleX = jmax (scaleX, 1.0f);

        scaleY = scaleX;

        if ((flags & xRight) != 0)
            newX += destination.getWidth() - source.getWidth() * scaleX;             // right
        else if ((flags & xLeft) == 0)
            newX += (destination.getWidth() - source.getWidth() * scaleX) / 2.0f;    // centre

        if ((flags & yBottom) != 0)
            newY += destination.getHeight() - source.getHeight() * scaleX;             // bottom
        else if ((flags & yTop) == 0)
            newY += (destination.getHeight() - source.getHeight() * scaleX) / 2.0f;    // centre
    }

    return AffineTransform::translation (-source.getX(), -source.getY())
                .scaled (scaleX, scaleY)
                .translated (newX, newY);
}

} // namespace juce
