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

namespace juce
{

#if ! (JUCE_LINUX || JUCE_PROJUCER_LIVE_BUILD)

#if JUCE_MAC || JUCE_IOS
 #include "../native/juce_mac_Video.h"
#elif JUCE_WINDOWS
 #include "../native/juce_win32_Video.h"
#elif JUCE_ANDROID
 #include "../native/juce_android_Video.h"
#endif

//==============================================================================
VideoComponent::VideoComponent (bool useNativeControlsIfAvailable)
    : pimpl (new Pimpl (*this, useNativeControlsIfAvailable))
{
    addAndMakeVisible (pimpl.get());
}

VideoComponent::~VideoComponent()
{
    pimpl.reset();
}

Result VideoComponent::load (const File& file)
{
   #if JUCE_ANDROID || JUCE_IOS
    ignoreUnused (file);
    jassertfalse;
    return Result::fail ("load() is not supported on this platform. Use loadAsync() instead.");
   #else
    auto r = pimpl->load (file);
    resized();
    return r;
   #endif
}

Result VideoComponent::load (const URL& url)
{
   #if JUCE_ANDROID || JUCE_IOS
    // You need to use loadAsync on Android & iOS.
    ignoreUnused (url);
    jassertfalse;
    return Result::fail ("load() is not supported on this platform. Use loadAsync() instead.");
   #else
    auto r = pimpl->load (url);
    resized();
    return r;
   #endif
}

void VideoComponent::loadAsync (const URL& url, std::function<void (const URL&, Result)> callback)
{
    if (callback == nullptr)
    {
        jassertfalse;
        return;
    }

   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->loadAsync (url, callback);
   #else
    auto result = load (url);
    callback (url, result);
   #endif
}

void VideoComponent::closeVideo()
{
    pimpl->close();
    // Closing on Android is async and resized() will be called internally by pimpl once
    // close operation finished.
   #if ! JUCE_ANDROID// TODO JUCE_IOS too?
    resized();
   #endif
}

bool VideoComponent::isVideoOpen() const                    { return pimpl->isOpen(); }

File VideoComponent::getCurrentVideoFile() const            { return pimpl->currentFile; }
URL VideoComponent::getCurrentVideoURL() const              { return pimpl->currentURL; }

double VideoComponent::getVideoDuration() const             { return pimpl->getDuration(); }
Rectangle<int> VideoComponent::getVideoNativeSize() const   { return pimpl->getNativeSize(); }

void VideoComponent::play()                                 { pimpl->play(); }
void VideoComponent::stop()                                 { pimpl->stop(); }

bool VideoComponent::isPlaying() const                      { return pimpl->isPlaying(); }

void VideoComponent::setPlayPosition (double newPos)        { pimpl->setPosition (newPos); }
double VideoComponent::getPlayPosition() const              { return pimpl->getPosition(); }

void VideoComponent::setPlaySpeed (double newSpeed)         { pimpl->setSpeed (newSpeed); }
double VideoComponent::getPlaySpeed() const                 { return pimpl->getSpeed(); }

void VideoComponent::setAudioVolume (float newVolume)       { pimpl->setVolume (newVolume); }
float VideoComponent::getAudioVolume() const                { return pimpl->getVolume(); }

void VideoComponent::resized()
{
    auto r = getLocalBounds();

    if (isVideoOpen() && ! r.isEmpty())
    {
        auto nativeSize = getVideoNativeSize();

        if (nativeSize.isEmpty())
        {
            // if we've just opened the file and are still waiting for it to
            // figure out the size, start our timer..
            if (! isTimerRunning())
                startTimer (50);
        }
        else
        {
            r = RectanglePlacement (RectanglePlacement::centred).appliedTo (nativeSize, r);
            stopTimer();
        }
    }
    else
    {
        stopTimer();
    }

    pimpl->setBounds (r);
}

void VideoComponent::timerCallback()
{
    resized();
}

#endif

} // namespace juce
