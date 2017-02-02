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
            player.automaticallyWaitsToMinimizeStalling = NO;
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
        [pimpl->view setFrame: frame];
        [pimpl->playerLayer setFrame: frame];
    }
}
