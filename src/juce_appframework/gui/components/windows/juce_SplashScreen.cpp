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

#include "juce_SplashScreen.h"
#include "../../../events/juce_MessageManager.h"
#include "../../graphics/imaging/juce_ImageCache.h"


//==============================================================================
SplashScreen::SplashScreen()
    : backgroundImage (0),
      isImageInCache (false)
{
    setOpaque (true);
}

SplashScreen::~SplashScreen()
{
    if (isImageInCache)
        ImageCache::release (backgroundImage);
    else
        delete backgroundImage;
}

//==============================================================================
void SplashScreen::show (const String& title,
                         Image* const backgroundImage_,
                         const int minimumTimeToDisplayFor,
                         const bool useDropShadow)
{
    backgroundImage = backgroundImage_;

    jassert (backgroundImage_ != 0);

    if (backgroundImage_ != 0)
    {
        isImageInCache = ImageCache::isImageInCache (backgroundImage_);

        setOpaque (! backgroundImage_->hasAlphaChannel());

        show (title,
              backgroundImage_->getWidth(),
              backgroundImage_->getHeight(),
              minimumTimeToDisplayFor,
              useDropShadow);
    }
}

void SplashScreen::show (const String& title,
                         const int width,
                         const int height,
                         const int minimumTimeToDisplayFor,
                         const bool useDropShadow)
{
    setName (title);
    setAlwaysOnTop (true);
    setVisible (true);
    centreWithSize (width, height);

    addToDesktop (useDropShadow ? (ComponentPeer::windowAppearsOnTaskbar | ComponentPeer::windowHasDropShadow)
                                : ComponentPeer::windowAppearsOnTaskbar);
    toFront (false);

    MessageManager::getInstance()->dispatchPendingMessages();

    repaint();

    earliestTimeToDelete = Time::getCurrentTime() + RelativeTime::milliseconds (minimumTimeToDisplayFor);
    startTimer (200);
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
    if (Time::getCurrentTime() > earliestTimeToDelete)
    {
        delete this;
    }
}

END_JUCE_NAMESPACE
