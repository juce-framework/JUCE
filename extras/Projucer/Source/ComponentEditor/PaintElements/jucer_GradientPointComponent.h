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

#pragma once

#include "jucer_PointComponent.h"
#include "jucer_ColouredElement.h"

//==============================================================================
class GradientPointComponent    : public PointComponent
{
public:
    GradientPointComponent (ColouredElement* const owner_,
                            const bool isStroke_,
                            const bool isStart_)
        : PointComponent (owner_),
          isStroke (isStroke_),
          isStart (isStart_)
    {
    }

    RelativePositionedRectangle getPosition() override
    {
        ColouredElement* e = dynamic_cast<ColouredElement*> (owner);

        if (isStroke)
            return isStart ? e->getStrokeType().fill.gradPos1
                           : e->getStrokeType().fill.gradPos2;

        return isStart ? e->getFillType().gradPos1
                       : e->getFillType().gradPos2;
    }

    void setPosition (const RelativePositionedRectangle& newPos) override
    {
        ColouredElement* e = dynamic_cast<ColouredElement*> (owner);

        if (isStroke)
        {
            JucerFillType f (e->getStrokeType().fill);

            if (isStart)
                f.gradPos1 = newPos;
            else
                f.gradPos2 = newPos;

            e->setStrokeFill (f, true);
        }
        else
        {
            JucerFillType f (e->getFillType());

            if (isStart)
                f.gradPos1 = newPos;
            else
                f.gradPos2 = newPos;

            e->setFillType (f, true);
        }
    }

    void updatePosition() override
    {
        PointComponent::updatePosition();

        ColouredElement* e = dynamic_cast<ColouredElement*> (owner);

        JucerFillType f (isStroke ? e->getStrokeType().fill
                                  : e->getFillType());

        setVisible (f.mode == JucerFillType::linearGradient
                     || f.mode == JucerFillType::radialGradient);
    }

private:
    bool isStroke, isStart;
};
