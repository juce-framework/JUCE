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

namespace juce
{

#if ! (JUCE_LINUX || JUCE_BSD)

#if JUCE_MAC || JUCE_IOS
 #include "../native/juce_Video_mac.h"
#elif JUCE_WINDOWS
 #include "../native/juce_Video_windows.h"
#elif JUCE_ANDROID
 #include "../native/juce_Video_android.h"
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
    return loadInternal (file, false);
}

Result VideoComponent::load (const URL& url)
{
    return loadInternal (url, false);
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
    auto result = loadInternal (url, true);
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

template <class FileOrURL>
Result VideoComponent::loadInternal (const FileOrURL& fileOrUrl, bool loadAsync)
{
   #if JUCE_ANDROID || JUCE_IOS
    ignoreUnused (fileOrUrl, loadAsync);
    // You need to use loadAsync on Android & iOS.
    jassertfalse;
    return Result::fail ("load() is not supported on this platform. Use loadAsync() instead.");
   #else
    auto result = pimpl->load (fileOrUrl);

    if (loadAsync)
        startTimer (50);
    else
        resized();

    return result;
   #endif
}

#endif

} // namespace juce
