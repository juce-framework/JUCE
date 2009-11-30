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

#ifndef __JUCER_GRADIENTPOINTCOMPONENT_JUCEHEADER__
#define __JUCER_GRADIENTPOINTCOMPONENT_JUCEHEADER__

#include "jucer_PointComponent.h"
#include "jucer_ColouredElement.h"


//==============================================================================
/**
*/
class GradientPointComponent    : public PointComponent
{
public:
    //==============================================================================
    GradientPointComponent (ColouredElement* const owner_,
                            const bool isStroke_,
                            const bool isStart_)
        : PointComponent (owner_),
          isStroke (isStroke_),
          isStart (isStart_)
    {
    }

    ~GradientPointComponent()
    {
    }

    const RelativePositionedRectangle getPosition()
    {
        ColouredElement* e = dynamic_cast <ColouredElement*> (owner);

        if (isStroke)
        {
            return isStart ? e->getStrokeType().fill.gradPos1
                           : e->getStrokeType().fill.gradPos2;
        }
        else
        {
            return isStart ? e->getFillType().gradPos1
                           : e->getFillType().gradPos2;
        }
    }

    void setPosition (const RelativePositionedRectangle& newPos)
    {
        ColouredElement* e = dynamic_cast <ColouredElement*> (owner);

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

        ColouredElement* e = dynamic_cast <ColouredElement*> (owner);

        JucerFillType f (isStroke ? e->getStrokeType().fill
                                  : e->getFillType());

        setVisible (f.mode == JucerFillType::linearGradient
                     || f.mode == JucerFillType::radialGradient);
    }

private:
    bool isStroke, isStart;
};

#endif   // __JUCER_GRADIENTPOINTCOMPONENT_JUCEHEADER__
