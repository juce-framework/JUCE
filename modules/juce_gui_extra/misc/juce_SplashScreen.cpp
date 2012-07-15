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

SplashScreen::SplashScreen()
{
    setOpaque (true);
}

SplashScreen::~SplashScreen()
{
}

//==============================================================================
void SplashScreen::show (const String& title,
                         const Image& backgroundImage_,
                         const int minimumTimeToDisplayFor,
                         const bool useDropShadow,
                         const bool removeOnMouseClick)
{
    backgroundImage = backgroundImage_;

    jassert (backgroundImage_.isValid());

    if (backgroundImage_.isValid())
    {
        setOpaque (! backgroundImage_.hasAlphaChannel());

        show (title,
              backgroundImage_.getWidth(),
              backgroundImage_.getHeight(),
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

   #if JUCE_MODAL_LOOPS_PERMITTED
    MessageManager::getInstance()->runDispatchLoopUntil (300);
   #endif

    repaint();

    originalClickCounter = removeOnMouseClick
                                ? Desktop::getInstance().getMouseButtonClickCounter()
                                : std::numeric_limits<int>::max();

    earliestTimeToDelete = Time::getCurrentTime() + RelativeTime::milliseconds (minimumTimeToDisplayFor);

    startTimer (50);
}

//==============================================================================
void SplashScreen::paint (Graphics& g)
{
    g.setOpacity (1.0f);

    g.drawImage (backgroundImage,
                 0, 0, getWidth(), getHeight(),
                 0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
}

void SplashScreen::timerCallback()
{
    if (Time::getCurrentTime() > earliestTimeToDelete
         || Desktop::getInstance().getMouseButtonClickCounter() != originalClickCounter)
    {
        delete this;
    }
}
