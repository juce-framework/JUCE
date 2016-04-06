/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_GRADIENTPOINTCOMPONENT_H_INCLUDED
#define JUCER_GRADIENTPOINTCOMPONENT_H_INCLUDED

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

    RelativePositionedRectangle getPosition()
    {
        ColouredElement* e = dynamic_cast<ColouredElement*> (owner);

        if (isStroke)
            return isStart ? e->getStrokeType().fill.gradPos1
                           : e->getStrokeType().fill.gradPos2;

        return isStart ? e->getFillType().gradPos1
                       : e->getFillType().gradPos2;
    }

    void setPosition (const RelativePositionedRectangle& newPos)
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

    void updatePosition()
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


#endif   // JUCER_GRADIENTPOINTCOMPONENT_H_INCLUDED
