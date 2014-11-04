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

SplashScreen::SplashScreen (const String& title, const Image& image, bool useDropShadow)
    : Component (title),
      backgroundImage (image),
      clickCountToDelete (0)
{
    // You must supply a valid image here!
    jassert (backgroundImage.isValid());

    setOpaque (! backgroundImage.hasAlphaChannel());
    makeVisible (image.getWidth(), image.getHeight(), useDropShadow);
}

SplashScreen::SplashScreen (const String& title, int width, int height, bool useDropShadow)
    : Component (title),
      clickCountToDelete (0)
{
    makeVisible (width, height, useDropShadow);
}

void SplashScreen::makeVisible (int w, int h, bool useDropShadow)
{
    clickCountToDelete = Desktop::getInstance().getMouseButtonClickCounter();
    creationTime = Time::getCurrentTime();

    setAlwaysOnTop (true);
    setVisible (true);
    centreWithSize (w, h);
    addToDesktop (useDropShadow ? ComponentPeer::windowHasDropShadow : 0);
    toFront (false);
}

SplashScreen::~SplashScreen() {}

void SplashScreen::deleteAfterDelay (RelativeTime timeout, bool removeOnMouseClick)
{
    // Note that this method must be safe to call from non-GUI threads
    if (! removeOnMouseClick)
        clickCountToDelete = std::numeric_limits<int>::max();

    minimumVisibleTime = timeout;

    startTimer (50);
}

void SplashScreen::paint (Graphics& g)
{
    g.setOpacity (1.0f);

    g.drawImage (backgroundImage,
                 0, 0, getWidth(), getHeight(),
                 0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
}

void SplashScreen::timerCallback()
{
    if (Time::getCurrentTime() > creationTime + minimumVisibleTime
         || Desktop::getInstance().getMouseButtonClickCounter() > clickCountToDelete)
        delete this;
}
