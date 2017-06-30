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

struct VideoComponent::Pimpl   : public NSViewComponent
{
    Pimpl()
    {
        setVisible (true);

        AVPlayerView* view = [[AVPlayerView alloc] init];
        setView (view);
        [view setNextResponder: [view superview]];
        [view setWantsLayer: YES];
        [view release];
    }

    ~Pimpl()
    {
        close();
        setView (nil);
    }

    Result load (const File& file)
    {
        auto r = load (createNSURLFromFile (file));

        if (r.wasOk())
            currentFile = file;

        return r;
    }

    Result load (const URL& url)
    {
        Result r = load ([NSURL URLWithString: juceStringToNS (url.toString (true))]);

        if (r.wasOk())
            currentURL = url;

        return r;
    }

    Result load (NSURL* url)
    {
        if (url != nil)
        {
            close();

            if (AVPlayer* player = [AVPlayer playerWithURL: url])
            {
                [getAVPlayerView() setPlayer: player];
                return Result::ok();
            }
        }

        return Result::fail ("Couldn't open movie");
    }

    void close()
    {
        stop();
        [getAVPlayerView() setPlayer: nil];
        currentFile = File();
        currentURL = URL();
    }

    bool isOpen() const
    {
        return getAVPlayer() != nil;
    }

    bool isPlaying() const
    {
        return getSpeed() != 0;
    }

    void play()
    {
        [getAVPlayer() play];
    }

    void stop()
    {
        [getAVPlayer() pause];
    }

    void setPosition (double newPosition)
    {
        if (AVPlayer* p = getAVPlayer())
        {
            CMTime t = { (CMTimeValue) (100000.0 * newPosition),
                         (CMTimeScale) 100000, kCMTimeFlags_Valid };

            [p seekToTime: t];
        }
    }

    double getPosition() const
    {
        if (AVPlayer* p = getAVPlayer())
            return toSeconds ([p currentTime]);

        return 0.0;
    }

    void setSpeed (double newSpeed)
    {
        [getAVPlayer() setRate: newSpeed];
    }

    double getSpeed() const
    {
        if (AVPlayer* p = getAVPlayer())
            return [p rate];

        return 0.0;
    }

    Rectangle<int> getNativeSize() const
    {
        if (AVPlayer* player = getAVPlayer())
        {
            CGSize s = [[player currentItem] presentationSize];
            return Rectangle<int> ((int) s.width, (int) s.height);
        }

        return Rectangle<int>();
    }

    double getDuration() const
    {
        if (AVPlayer* player = getAVPlayer())
            return toSeconds ([[player currentItem] duration]);

        return 0.0;
    }

    void setVolume (float newVolume)
    {
        [getAVPlayer() setVolume: newVolume];
    }

    float getVolume() const
    {
        if (AVPlayer* p = getAVPlayer())
            return [p volume];

        return 0.0f;
    }

    File currentFile;
    URL currentURL;

private:
    AVPlayerView* getAVPlayerView() const   { return (AVPlayerView*) getView(); }
    AVPlayer* getAVPlayer() const           { return [getAVPlayerView() player]; }

    static double toSeconds (const CMTime& t) noexcept
    {
        return t.timescale != 0 ? (t.value / (double) t.timescale) : 0.0;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};
