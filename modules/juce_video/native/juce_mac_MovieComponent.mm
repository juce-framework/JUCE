/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

struct MovieComponent::Pimpl
{
    bool open (MovieComponent& parent, const String& newPath)
    {
        JUCE_AUTORELEASEPOOL
        {
            close();

            NSString* videoFile = [NSString stringWithUTF8String: newPath.toUTF8()];
            NSURL* url = [NSURL fileURLWithPath: videoFile];

            AVAsset* asset = [AVAsset assetWithURL: url];
            duration = CMTimeGetSeconds (asset.duration);
            nativeSize = [[[asset tracksWithMediaType: AVMediaTypeVideo] objectAtIndex: 0] naturalSize];

            if (duration <= 0)
                return false;

            auto frame = CGRectMake (0, 0, nativeSize.width, nativeSize.height);

            view = [[NSView alloc] initWithFrame: NSRectFromCGRect (frame)];
            [view setHidden: NO];
            [view setNeedsDisplay: YES];
            [view setWantsLayer: YES];
            [view makeBackingLayer];

            parent.setView (view);
            path = newPath;

            player = [[AVPlayer alloc] initWithURL: url];
            player.actionAtItemEnd = AVPlayerActionAtItemEndPause;
            player.masterClock = CMClockGetHostTimeClock();

            if ([player respondsToSelector: @selector (setAutomaticallyWaitsToMinimizeStalling:)])
                [player performSelector: @selector (setAutomaticallyWaitsToMinimizeStalling:)
                             withObject: nil];

            [player pause];

            playerLayer = [[AVPlayerLayer playerLayerWithPlayer: player] retain];
            [playerLayer setFrame: frame];
            [playerLayer setHidden: NO];

            [[view layer] addSublayer: playerLayer];

            parent.resized();
        }

        return true;
    }

    void close()
    {
        [playerLayer release];
        playerLayer = nil;

        [player release];
        player = nil;

        [view release];
        view = nil;

        playing = false;
        duration = 0;
        nativeSize = { 0, 0 };
        path = {};
    }

    String path;
    NSView* view = nil;
    AVPlayer* player = nil;
    AVPlayerLayer* playerLayer = nil;
    double duration = 0;
    CGSize nativeSize = { 0, 0 };
    bool playing = false;
};

MovieComponent::MovieComponent()  : pimpl (new Pimpl()) {}

MovieComponent::~MovieComponent()
{
    closeMovie();
    pimpl = nullptr;
}

bool MovieComponent::loadMovie (const File& file)
{
    return file.existsAsFile()
            && pimpl->open (*this, file.getFullPathName());
}

bool MovieComponent::loadMovie (const URL& file)
{
    return pimpl->open (*this, file.toString (true));
}

void MovieComponent::closeMovie()
{
    setView (nullptr);
    pimpl->close();
}

bool MovieComponent::isMovieOpen() const
{
    return pimpl->player != nil && pimpl->player.status != AVPlayerStatusFailed;
}

String MovieComponent::getCurrentMoviePath() const
{
    return pimpl->path;
}

void MovieComponent::play()
{
    pimpl->playing = true;
    [pimpl->player play];
}

void MovieComponent::stop()
{
    pimpl->playing = false;
    [pimpl->player pause];
}

double MovieComponent::getDuration() const
{
    return pimpl->duration;
}

double MovieComponent::getPosition() const
{
    return CMTimeGetSeconds ([pimpl->player currentTime]);
}

void MovieComponent::setPosition (double seconds)
{
    auto delta = std::abs (seconds - getPosition());
    auto newTime = CMTimeMakeWithSeconds (seconds, 600);

    if (! pimpl->playing)
    {
        [pimpl->player seekToTime: newTime
                  toleranceBefore: kCMTimeZero
                   toleranceAfter: kCMTimeZero];
    }
    else if (delta > 0.035)
    {
        auto masterClock = CMClockGetTime (CMClockGetHostTimeClock());

        try
        {
            [pimpl->player setRate: 1.0f
                              time: newTime
                        atHostTime: masterClock];
        }
        catch (...)
        {
            // We should never end up in here, unless somehow automaticallyWaitsToMinimizeStalling
            // got reset to YES
            jassertfalse;

            if (delta > 0.3)
                [pimpl->player seekToTime: newTime
                          toleranceBefore: kCMTimeZero
                           toleranceAfter: kCMTimeZero];
        }
    }
}

void MovieComponent::setVolume (float newVolume)
{
    pimpl->player.volume = static_cast<float> (newVolume);
}

float MovieComponent::getVolume() const
{
    return (float) pimpl->player.volume;
}

Rectangle<int> MovieComponent::getNativeSize() const
{
    return { 0, 0, roundToInt (pimpl->nativeSize.width), roundToInt (pimpl->nativeSize.height) };
}

void MovieComponent::setBoundsWithCorrectAspectRatio (Rectangle<int> spaceToFitWithin, RectanglePlacement placement)
{
    auto nativeSize = getNativeSize();

    setBounds ((spaceToFitWithin.isEmpty() || nativeSize.isEmpty())
                 ? spaceToFitWithin
                 : placement.appliedTo (nativeSize, spaceToFitWithin));
}

void MovieComponent::resized()
{
    JUCE_AUTORELEASEPOOL
    {
        auto frame = CGRectMake (0, 0, (CGFloat) getWidth(), (CGFloat) getHeight());
        [pimpl->view setFrame: NSRectFromCGRect (frame)];
        [pimpl->playerLayer setFrame: frame];
    }
}
