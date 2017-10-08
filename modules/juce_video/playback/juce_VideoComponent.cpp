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

#if JUCE_MAC || JUCE_IOS || JUCE_MSVC

#if JUCE_MAC || JUCE_IOS
 #include "../native/juce_mac_Video.h"
#elif JUCE_WINDOWS
 #include "../native/juce_win32_Video.h"
#endif

//==============================================================================
VideoComponent::VideoComponent()  : pimpl (new Pimpl())
{
    addAndMakeVisible (pimpl);
}

VideoComponent::~VideoComponent()
{
    pimpl = nullptr;
}

Result VideoComponent::load (const File& file)
{
    Result r = pimpl->load (file);
    resized();
    return r;
}

Result VideoComponent::load (const URL& url)
{
    Result r = pimpl->load (url);
    resized();
    return r;
}

void VideoComponent::closeVideo()
{
    pimpl->close();
    resized();
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
void VideoComponent::setAudioVolume (float newVolume)       { pimpl->setVolume (newVolume); }
float VideoComponent::getAudioVolume() const                { return pimpl->getVolume(); }

void VideoComponent::resized()
{
    Rectangle<int> r = getLocalBounds();

    if (isVideoOpen() && ! r.isEmpty())
    {
        Rectangle<int> nativeSize = getVideoNativeSize();

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
