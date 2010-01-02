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

#include "juce_SplashScreen.h"
#include "../../../events/juce_MessageManager.h"
#include "../../graphics/imaging/juce_ImageCache.h"
#include "../juce_Desktop.h"


//==============================================================================
SplashScreen::SplashScreen()
    : backgroundImage (0)
{
    setOpaque (true);
}

SplashScreen::~SplashScreen()
{
    ImageCache::releaseOrDelete (backgroundImage);
}

//==============================================================================
void SplashScreen::show (const String& title,
                         Image* const backgroundImage_,
                         const int minimumTimeToDisplayFor,
                         const bool useDropShadow,
                         const bool removeOnMouseClick)
{
    backgroundImage = backgroundImage_;

    jassert (backgroundImage_ != 0);

    if (backgroundImage_ != 0)
    {
        setOpaque (! backgroundImage_->hasAlphaChannel());

        show (title,
              backgroundImage_->getWidth(),
              backgroundImage_->getHeight(),
              minimumTimeToDisplayFor,
              useDropShadow,
              removeOnMouseClick);
    }
}

void SplashScreen::show (const String& title,
                         const int width,
                         const int height,
                         const int minimumTimeToDisplayFor,
                         const bool useDropShadow,
                         const bool removeOnMouseClick)
{
    setName (title);
    setAlwaysOnTop (true);
    setVisible (true);
    centreWithSize (width, height);

    addToDesktop (useDropShadow ? ComponentPeer::windowHasDropShadow : 0);
    toFront (false);

    MessageManager::getInstance()->runDispatchLoopUntil (300);

    repaint();

    originalClickCounter = removeOnMouseClick
                                ? Desktop::getMouseButtonClickCounter()
                                : INT_MAX;

    earliestTimeToDelete = Time::getCurrentTime() + RelativeTime::milliseconds (minimumTimeToDisplayFor);

    startTimer (50);
}

//==============================================================================
void SplashScreen::paint (Graphics& g)
{
    if (backgroundImage != 0)
    {
        g.setOpacity (1.0f);

        g.drawImage (backgroundImage,
                     0, 0, getWidth(), getHeight(),
                     0, 0, backgroundImage->getWidth(), backgroundImage->getHeight());
    }
}

void SplashScreen::timerCallback()
{
    if (Time::getCurrentTime() > earliestTimeToDelete
         || Desktop::getMouseButtonClickCounter() > originalClickCounter)
    {
        delete this;
    }
}

END_JUCE_NAMESPACE
