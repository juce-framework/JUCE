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

#if JUCE_IOS
 using BaseClass = UIViewComponent;
#else
 using BaseClass = NSViewComponent;
#endif

struct VideoComponent::Pimpl   : public BaseClass
{
    Pimpl()
    {
        setVisible (true);

       #if JUCE_MAC && JUCE_32BIT
        auto view = [[NSView alloc] init];  // 32-bit builds don't have AVPlayerView, so need to use a layer
        controller = [[AVPlayerLayer alloc] init];
        setView (view);
        [view setNextResponder: [view superview]];
        [view setWantsLayer: YES];
        [view setLayer: controller];
        [view release];
       #elif JUCE_MAC
        controller = [[AVPlayerView alloc] init];
        setView (controller);
        [controller setNextResponder: [controller superview]];
        [controller setWantsLayer: YES];
       #else
        controller = [[AVPlayerViewController alloc] init];
        setView ([controller view]);
       #endif
    }

    ~Pimpl()
    {
        close();
        setView (nil);
        [controller release];
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
        auto r = load ([NSURL URLWithString: juceStringToNS (url.toString (true))]);

        if (r.wasOk())
            currentURL = url;

        return r;
    }

    Result load (NSURL* url)
    {
        if (url != nil)
        {
            close();

            if (auto* player = [AVPlayer playerWithURL: url])
            {
                [controller setPlayer: player];
                return Result::ok();
            }
        }

        return Result::fail ("Couldn't open movie");
    }

    void close()
    {
        stop();
        [controller setPlayer: nil];
        currentFile = File();
        currentURL = {};
    }

    bool isOpen() const noexcept        { return getAVPlayer() != nil; }
    bool isPlaying() const noexcept     { return getSpeed() != 0; }

    void play() noexcept                { [getAVPlayer() play]; }
    void stop() noexcept                { [getAVPlayer() pause]; }

    void setPosition (double newPosition)
    {
        if (auto* p = getAVPlayer())
        {
            CMTime t = { (CMTimeValue) (100000.0 * newPosition),
                         (CMTimeScale) 100000, kCMTimeFlags_Valid };

            [p seekToTime: t
          toleranceBefore: kCMTimeZero
           toleranceAfter: kCMTimeZero];
        }
    }

    double getPosition() const
    {
        if (auto* p = getAVPlayer())
            return toSeconds ([p currentTime]);

        return 0.0;
    }

    void setSpeed (double newSpeed)
    {
        [getAVPlayer() setRate: (float) newSpeed];
    }

    double getSpeed() const
    {
        if (auto* p = getAVPlayer())
            return [p rate];

        return 0.0;
    }

    Rectangle<int> getNativeSize() const
    {
        if (auto* player = getAVPlayer())
        {
            auto s = [[player currentItem] presentationSize];
            return { (int) s.width, (int) s.height };
        }

        return {};
    }

    double getDuration() const
    {
        if (auto* player = getAVPlayer())
            return toSeconds ([[player currentItem] duration]);

        return 0.0;
    }

    void setVolume (float newVolume)
    {
        [getAVPlayer() setVolume: newVolume];
    }

    float getVolume() const
    {
        if (auto* p = getAVPlayer())
            return [p volume];

        return 0.0f;
    }

    File currentFile;
    URL currentURL;

private:
   #if JUCE_IOS
    AVPlayerViewController* controller = nil;
   #elif JUCE_32BIT
    AVPlayerLayer* controller = nil;
   #else
    AVPlayerView* controller = nil;
   #endif

    AVPlayer* getAVPlayer() const noexcept   { return [controller player]; }

    static double toSeconds (const CMTime& t) noexcept
    {
        return t.timescale != 0 ? (t.value / (double) t.timescale) : 0.0;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};
