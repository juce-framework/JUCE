/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/*
  ==============================================================================

   In accordance with the terms of the JUCE 7 End-Use License Agreement, the
   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered
   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE
   under the GPL v3 license.

   End User License Agreement: www.juce.com/juce-7-licence

  ==============================================================================
*/

// BEGIN SECTION A

#if ! defined (JUCE_DISPLAY_SPLASH_SCREEN)
 #define JUCE_DISPLAY_SPLASH_SCREEN 1
#endif

#if ! defined (JUCE_USE_DARK_SPLASH_SCREEN)
 #define JUCE_USE_DARK_SPLASH_SCREEN 1
#endif

static constexpr int millisecondsToDisplaySplash = 2000, splashScreenFadeOutTime = 2000,
                     splashScreenLogoWidth = 123, splashScreenLogoHeight = 63;
static uint32 splashDisplayTime = 0;
static bool splashHasStartedFading = false;

static Rectangle<float> getLogoArea (Rectangle<float> parentRect)
{
    return parentRect.reduced (6.0f)
                     .removeFromRight  ((float) splashScreenLogoWidth)
                     .removeFromBottom ((float) splashScreenLogoHeight);
}

//==============================================================================
JUCESplashScreen::JUCESplashScreen (Component& parent)
{
    ignoreUnused (parent);

   #if JUCE_DISPLAY_SPLASH_SCREEN
    if (splashDisplayTime == 0
         || Time::getMillisecondCounter() < splashDisplayTime + (uint32) millisecondsToDisplaySplash)
    {
        content = getSplashScreenLogo();

        setAlwaysOnTop (true);
        parent.addAndMakeVisible (this);
    }
    else
   #endif
    {
        startTimer (1);
    }

    setAccessible (false);
}

std::unique_ptr<Drawable> JUCESplashScreen::getSplashScreenLogo()
{
    const char* svgData = R"JUCESPLASHSCREEN(
        <svg width="120" height="60" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <rect id="a" x=".253" y=".253" width="122" height="62" rx="10"/>
            <mask id="b" fill="#ffffff">
              <use xlink:href="#a" width="100%" height="100%"/>
            </mask>
          </defs>
          <rect width="120" height="60" rx="10" fill=")JUCESPLASHSCREEN"
           #if JUCE_USE_DARK_SPLASH_SCREEN
            "#000000"
           #else
            "#ffffff"
           #endif
            R"JUCESPLASHSCREEN(" opacity=".8" fill-rule="evenodd"/>
          <path d="M57.404 39.48V25.688h-4V39.48c0 2.432-1.408 4.032-3.52 4.032-1.184 0-2.08-.48-3.36-2.176l-2.88 2.496c1.952 2.56 3.84 3.424 6.24 3.424 4.384 0 7.52-3.104 7.52-7.776zm21.104-1.184V25.688h-4v12.448c0 3.264-1.92 5.376-4.672 5.376s-4.672-2.112-4.672-5.376V25.688h-4v12.608c0 5.568 4.032 8.96 8.672 8.96 4.64 0 8.672-3.392 8.672-8.96zM99.324 44.6l-2.368-2.976c-2.016 1.408-3.328 1.888-4.896 1.888-3.872 0-7.008-3.136-7.008-7.168s3.136-7.168 7.008-7.168c1.632 0 2.816.416 4.928 1.888l2.336-2.912c-2.208-1.792-4.576-2.72-7.264-2.72-6.048 0-11.104 4.832-11.104 10.912 0 6.048 4.992 10.912 11.104 10.912 2.464 0 4.544-.608 7.264-2.656zm15.472 2.4v-3.616h-9.28V38.04h8.928v-3.616h-8.928v-5.12h9.28v-3.616h-13.28V47zM66.12 21l-2.28-6.88-2 5.05-2-5.05L57.56 21h.84l1.51-4.66 1.93 4.88 1.93-4.88L65.28 21zm5.097 0h-.8c-.13-.23-.17-.57-.17-.82-.35.45-1.04.9-1.82.9-.95 0-1.62-.52-1.62-1.35 0-.84.56-1.67 3.37-2.12 0-.57-.43-.95-1.17-.95-.64 0-1.07.23-1.45.52l-.48-.57c.53-.44 1.21-.71 1.99-.71.84 0 1.91.32 1.91 1.72v2.21c0 .36.06.82.24 1.17zm-1.04-2.27v-.44c-2.04.35-2.57.86-2.57 1.44 0 .47.51.67.98.67.72 0 1.59-.6 1.59-1.67zM77.063 21h-.8v-.82c-.32.5-.97.9-1.76.9-1.39 0-2.52-1.18-2.52-2.59s1.13-2.59 2.52-2.59c.79 0 1.44.4 1.76.9V14h.8zm-.8-2.51c0-1.02-.75-1.83-1.74-1.83s-1.74.81-1.74 1.83.75 1.83 1.74 1.83 1.74-.81 1.74-1.83zm6.877-.03c0 .18-.02.35-.02.35H78.9c.14.85.87 1.51 1.76 1.51.52 0 1.07-.23 1.46-.65l.57.52c-.52.59-1.26.89-2.03.89-1.43 0-2.59-1.17-2.59-2.59 0-1.43 1.15-2.59 2.58-2.59 1.47 0 2.49 1.16 2.49 2.56zm-.84-.36c-.18-.9-.78-1.44-1.65-1.44-.87 0-1.58.61-1.74 1.44zm11.263-2.12h-.84l-1.27 3.38-1.63-3.66-1.63 3.66-1.27-3.38h-.84l2.06 5.3 1.68-3.84 1.68 3.84zm1.837-1.52c0-.3-.25-.55-.55-.55-.3 0-.55.25-.55.55 0 .3.25.55.55.55.3 0 .55-.25.55-.55zM95.25 21v-5.02h-.8V21zm3.897-4.31v-.71h-1.34v-1.64h-.8v1.64h-1.01v.71h1.01V21h.8v-4.31zm5.026 4.31v-2.88c0-1.42-.93-2.22-2.08-2.22-.69 0-1.26.37-1.47.73V14h-.8v7h.8v-2.83c0-.95.61-1.51 1.39-1.51.78 0 1.36.56 1.36 1.51V21z" fill=")JUCESPLASHSCREEN"
           #if JUCE_USE_DARK_SPLASH_SCREEN
            "#ffffff"
           #else
            "#000000"
           #endif
            R"JUCESPLASHSCREEN(" mask="url(#b)" transform="translate(-1.253 -1.253)" fill-rule="evenodd"/>
          <g transform="matrix(.13126 0 0 .13126 4.943 10.657)">
            <ellipse cx="142.2" cy="142.2" rx="132.82" ry="132.74" fill="#ffffff"/>
            <path d="M142.2 284.4C63.79 284.4 0 220.61 0 142.2S63.79 0 142.2 0s142.2 63.79 142.2 142.2-63.79 142.2-142.2 142.2zm0-265.48c-68.06 0-123.43 55.3-123.43 123.28S74.14 265.48 142.2 265.48s123.43-55.3 123.43-123.28S210.26 18.92 142.2 18.92z" fill="#8dc63f"/>
          </g>
          <path d="M25.695 32.623c1.117 2.803 2.33 5.597 3.838 8.386a1.912 1.912 0 002.78.657 15.201 15.201 0 003.888-3.999 1.91 1.91 0 00-1.05-2.874c-2.97-.903-5.728-2.011-8.419-3.178a.788.788 0 00-1.037 1.008z" fill="#f3bd48"/>
          <path d="M27.2 30.264c2.825 1.225 5.716 2.376 8.845 3.282a1.922 1.922 0 002.424-1.508 15.226 15.226 0 00-.05-5.693 1.916 1.916 0 00-2.773-1.313c-2.814 1.486-5.622 2.683-8.424 3.79a.788.788 0 00-.023 1.442z" fill="#f09f53"/>
          <path d="M24.31 32.999a.788.788 0 00-1.444-.023c-1.222 2.817-2.373 5.696-3.281 8.81a1.918 1.918 0 001.524 2.427 15.238 15.238 0 005.644-.122 1.918 1.918 0 001.313-2.768c-1.465-2.78-2.658-5.555-3.756-8.324z" fill="#ecdc13"/>
          <path d="M23.042 25.773a.788.788 0 001.444.02c1.235-2.845 2.394-5.756 3.304-8.91a1.92 1.92 0 00-1.51-2.42 15.226 15.226 0 00-5.74.077 1.916 1.916 0 00-1.312 2.77c1.495 2.825 2.7 5.647 3.814 8.463z" fill="#a95a96"/>
          <path d="M11.94 24.077c3.103.933 5.971 2.092 8.771 3.313a.788.788 0 001.04-1.007c-1.163-2.936-2.423-5.865-4.01-8.794a1.916 1.916 0 00-2.78-.657 15.204 15.204 0 00-4.098 4.283 1.916 1.916 0 001.077 2.862z" fill="#2b8ec1"/>
          <path d="M21.54 32.446a.788.788 0 00-1.007-1.04c-2.863 1.137-5.72 2.363-8.569 3.908a1.912 1.912 0 00-.646 2.79 15.213 15.213 0 004.141 3.938 1.912 1.912 0 002.858-1.064c.912-3.02 2.038-5.808 3.224-8.532z" fill="#add14c"/>
          <path d="M25.77 26.424a.788.788 0 001.008 1.038c2.877-1.142 5.748-2.381 8.613-3.938a1.914 1.914 0 00.66-2.76 15.202 15.202 0 00-4.133-4.048 1.912 1.912 0 00-2.877 1.05c-.923 3.058-2.067 5.892-3.271 8.658z" fill="#e74253"/>
          <path d="M20.014 28.64c-2.84-1.23-5.744-2.388-8.886-3.296A1.918 1.918 0 008.7 26.87a15.24 15.24 0 00.159 5.73 1.911 1.911 0 002.756 1.257c2.796-1.477 5.588-2.669 8.373-3.771a.788.788 0 00.025-1.447z" fill="#01b699"/>
        </svg>
    )JUCESPLASHSCREEN";

    auto svgXml = parseXML (svgData);
    jassert (svgXml != nullptr);
    return Drawable::createFromSVG (*svgXml);
}

void JUCESplashScreen::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    Point<float> bottomRight (0.9f * r.getWidth(),
                              0.9f * r.getHeight());

    ColourGradient cg (Colour (0x00000000), Line<float> (0.0f, r.getHeight(), r.getWidth(), 0.0f)
                                              .findNearestPointTo (bottomRight),
                       Colour (0xff000000), bottomRight, false);
    cg.addColour (0.25f, Colour (0x10000000));
    cg.addColour (0.50f, Colour (0x30000000));
    cg.addColour (0.75f, Colour (0x70000000));
    g.setFillType (cg);
    g.fillAll();

    content->drawWithin (g, getLogoArea (r), RectanglePlacement::centred, 1.0f);

    if (splashDisplayTime == 0)
        splashDisplayTime = Time::getMillisecondCounter();

    if (! isTimerRunning())
        startTimer (millisecondsToDisplaySplash);
}

void JUCESplashScreen::timerCallback()
{
   #if JUCE_DISPLAY_SPLASH_SCREEN
    if (isVisible() && ! splashHasStartedFading)
    {
        splashHasStartedFading = true;
        fader.animateComponent (this, getBounds(), 0.0f, splashScreenFadeOutTime, false, 0, 0);
    }

    if (splashHasStartedFading && ! fader.isAnimating())
   #endif
        delete this;
}

void JUCESplashScreen::parentSizeChanged()
{
    if (auto* p = getParentComponent())
        setBounds (p->getLocalBounds().removeFromBottom (splashScreenLogoHeight * 3)
                                      .removeFromRight  (splashScreenLogoWidth  * 3));
}

void JUCESplashScreen::parentHierarchyChanged()
{
    toFront (false);
}

bool JUCESplashScreen::hitTest (int x, int y)
{
    if (! splashHasStartedFading)
        return getLogoArea (getLocalBounds().toFloat()).contains ((float) x, (float) y);

    return false;
}

void JUCESplashScreen::mouseUp (const MouseEvent&)
{
    URL juceWebsite ("https://juce.com");
    juceWebsite.launchInDefaultBrowser();
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> JUCESplashScreen::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::splashScreen);
}

// END SECTION A

} // namespace juce
