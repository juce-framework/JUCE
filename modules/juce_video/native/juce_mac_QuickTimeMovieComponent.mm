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

#if JUCE_QUICKTIME

struct NonInterceptingQTMovieViewClass  : public ObjCClass <QTMovieView>
{
    NonInterceptingQTMovieViewClass()  : ObjCClass <QTMovieView> ("JUCEQTMovieView_")
    {
        addMethod (@selector (hitTest:),            hitTest,           "@@:", @encode (NSPoint));
        addMethod (@selector (acceptsFirstMouse:),  acceptsFirstMouse, "c@:@");

        registerClass();
    }

private:
    static NSView* hitTest (id self, SEL, NSPoint point)
    {
        if (! [(QTMovieView*) self isControllerVisible])
            return nil;

        objc_super s = { self, [QTMovieView class] };
        return objc_msgSendSuper (&s, @selector (hitTest:), point);
    }

    static BOOL acceptsFirstMouse (id, SEL, NSEvent*)
    {
        return YES;
    }
};

//==============================================================================
#define theMovie (static_cast<QTMovie*> (movie))

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : movie (0)
{
    setOpaque (true);
    setVisible (true);

    static NonInterceptingQTMovieViewClass cls;
    QTMovieView* view = [cls.createInstance() initWithFrame: NSMakeRect (0, 0, 100.0f, 100.0f)];
    setView (view);
    [view setNextResponder: [view superview]];
    [view setWantsLayer: YES]; // prevents the view failing to redraw correctly when paused.
    [view release];
}

QuickTimeMovieComponent::~QuickTimeMovieComponent()
{
    closeMovie();
    setView (nil);
}

bool QuickTimeMovieComponent::isQuickTimeAvailable() noexcept
{
    return true;
}

static QTMovie* openMovieFromStream (InputStream* movieStream, File& movieFile)
{
    // unfortunately, QTMovie objects can only be created on the main thread..
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    QTMovie* movie = nil;

    if (FileInputStream* const fin = dynamic_cast<FileInputStream*> (movieStream))
    {
        movieFile = fin->getFile();
        movie = [QTMovie movieWithFile: juceStringToNS (movieFile.getFullPathName())
                                 error: nil];
    }
    else
    {
        MemoryBlock temp;
        movieStream->readIntoMemoryBlock (temp);

        static const char* const suffixesToTry[] = { ".mov", ".mp3", ".avi", ".m4a" };

        for (int i = 0; i < numElementsInArray (suffixesToTry); ++i)
        {
            movie = [QTMovie movieWithDataReference: [QTDataReference dataReferenceWithReferenceToData: [NSData dataWithBytes: temp.getData()
                                                                                                                       length: temp.getSize()]
                                                                                                  name: [NSString stringWithUTF8String: suffixesToTry[i]]
                                                                                              MIMEType: nsEmptyString()]
                                              error: nil];

            if (movie != 0)
                break;
        }
    }

    return movie;
}

bool QuickTimeMovieComponent::loadMovie (const File& file, const bool showController)
{
    return loadMovie (file.createInputStream(), showController);
}

bool QuickTimeMovieComponent::loadMovie (InputStream* movieStream, const bool showController)
{
    const ScopedPointer<InputStream> movieStreamDeleter (movieStream);

    closeMovie();

    if (getPeer() == nullptr)
    {
        // To open a movie, this component must be visible inside a functioning window, so that
        // the QT control can be assigned to the window.
        jassertfalse;
        return false;
    }

    if (movieStream == nullptr)
        return false;

    movie = openMovieFromStream (movieStream, movieFile);

    [theMovie retain];
    QTMovieView* view = (QTMovieView*) getView();
    [view setMovie: theMovie];

    controllerVisible = showController;
    [view setControllerVisible: controllerVisible];
    setLooping (looping);

    return movie != nil;
}

bool QuickTimeMovieComponent::loadMovie (const URL& movieURL, const bool showController)
{
    // unfortunately, QTMovie objects can only be created on the main thread..
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    closeMovie();

    if (getPeer() == nullptr)
    {
        // To open a movie, this component must be visible inside a functioning window, so that
        // the QT control can be assigned to the window.
        jassertfalse;
        return false;
    }

    NSURL* url = [NSURL URLWithString: juceStringToNS (movieURL.toString (true))];
    NSError* err;
    if ([QTMovie canInitWithURL: url])
        movie = [QTMovie movieWithURL: url error: &err];

    [theMovie retain];
    QTMovieView* view = (QTMovieView*) getView();
    [view setMovie: theMovie];

    controllerVisible = showController;
    [view setControllerVisible: controllerVisible];
    setLooping (looping);

    return movie != nil;
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();
    QTMovieView* view = (QTMovieView*) getView();
    [view setMovie: nil];
    [theMovie release];
    movie = 0;
    movieFile = File();
}

bool QuickTimeMovieComponent::isMovieOpen() const
{
    return movie != nil;
}

File QuickTimeMovieComponent::getCurrentMovieFile() const
{
    return movieFile;
}

void QuickTimeMovieComponent::play()
{
    [theMovie play];
}

void QuickTimeMovieComponent::stop()
{
    [theMovie stop];
}

bool QuickTimeMovieComponent::isPlaying() const
{
    return movie != 0  && [theMovie rate] != 0;
}

void QuickTimeMovieComponent::setPosition (const double seconds)
{
    if (movie != 0)
    {
        QTTime t;
        t.timeValue = (long long) (100000.0 * seconds);
        t.timeScale = 100000;
        t.flags = 0;

        [theMovie setCurrentTime: t];
    }
}

double QuickTimeMovieComponent::getPosition() const
{
    if (movie == 0)
        return 0.0;

    QTTime t = [theMovie currentTime];
    return t.timeValue / (double) t.timeScale;
}

void QuickTimeMovieComponent::setSpeed (const float newSpeed)
{
    [theMovie setRate: newSpeed];
}

double QuickTimeMovieComponent::getMovieDuration() const
{
    if (movie == 0)
        return 0.0;

    QTTime t = [theMovie duration];
    return t.timeValue / (double) t.timeScale;
}

void QuickTimeMovieComponent::setLooping (const bool shouldLoop)
{
    looping = shouldLoop;

    [theMovie setAttribute: [NSNumber numberWithBool: shouldLoop]
                    forKey: QTMovieLoopsAttribute];
}

bool QuickTimeMovieComponent::isLooping() const
{
    return looping;
}

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    [theMovie setVolume: newVolume];
}

float QuickTimeMovieComponent::getMovieVolume() const
{
    return movie != 0 ? [theMovie volume] : 0.0f;
}

void QuickTimeMovieComponent::getMovieNormalSize (int& width, int& height) const
{
    width = 0;
    height = 0;

    if (movie != 0)
    {
        NSSize s = [[theMovie attributeForKey: QTMovieNaturalSizeAttribute] sizeValue];
        width = (int) s.width;
        height = (int) s.height;
    }
}

void QuickTimeMovieComponent::paint (Graphics& g)
{
    if (movie == 0)
        g.fillAll (Colours::black);
}

bool QuickTimeMovieComponent::isControllerVisible() const
{
    return controllerVisible;
}

//==============================================================================
void QuickTimeMovieComponent::goToStart()
{
    setPosition (0.0);
}

void QuickTimeMovieComponent::setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                                               RectanglePlacement placement)
{
    int normalWidth, normalHeight;
    getMovieNormalSize (normalWidth, normalHeight);

    const Rectangle<int> normalSize (normalWidth, normalHeight);

    if (! (spaceToFitWithin.isEmpty() || normalSize.isEmpty()))
        setBounds (placement.appliedTo (normalSize, spaceToFitWithin));
    else
        setBounds (spaceToFitWithin);
}


//==============================================================================
#if ! (JUCE_MAC && JUCE_64BIT)

bool juce_OpenQuickTimeMovieFromStream (InputStream* movieStream, Movie& result, Handle&)
{
    if (movieStream == nullptr)
        return false;

    File file;
    QTMovie* movie = openMovieFromStream (movieStream, file);

    if (movie != nil)
        result = [movie quickTimeMovie];

    return movie != nil;
}

#endif
#endif
