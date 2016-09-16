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

using namespace QTOLibrary;
using namespace QTOControlLib;

bool juce_OpenQuickTimeMovieFromStream (InputStream* input, Movie& movie, Handle& dataHandle);

static bool isQTAvailable = false;


//==============================================================================
class QuickTimeMovieComponent::Pimpl
{
public:
    Pimpl() : dataHandle (0)
    {
    }

    ~Pimpl()
    {
        clearHandle();
    }

    void clearHandle()
    {
        if (dataHandle != 0)
        {
            DisposeHandle (dataHandle);
            dataHandle = 0;
        }
    }

    IQTControlPtr qtControl;
    IQTMoviePtr qtMovie;
    Handle dataHandle;
};

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : movieLoaded (false),
      controllerVisible (true)
{
    pimpl = new Pimpl();
    setMouseEventsAllowed (false);
}

QuickTimeMovieComponent::~QuickTimeMovieComponent()
{
    closeMovie();
    pimpl->qtControl = 0;
    deleteControl();
    pimpl = nullptr;
}

bool QuickTimeMovieComponent::isQuickTimeAvailable() noexcept
{
    if (! isQTAvailable)
        isQTAvailable = (InitializeQTML (0) == noErr) && (EnterMovies() == noErr);

    return isQTAvailable;
}

//==============================================================================
void QuickTimeMovieComponent::createControlIfNeeded()
{
    if (isShowing() && ! isControlCreated())
    {
        const IID qtIID = __uuidof (QTControl);

        if (createControl (&qtIID))
        {
            const IID qtInterfaceIID = __uuidof (IQTControl);
            pimpl->qtControl = (IQTControl*) queryInterface (&qtInterfaceIID);

            if (pimpl->qtControl != nullptr)
            {
                pimpl->qtControl->Release(); // it has one ref too many at this point

                pimpl->qtControl->QuickTimeInitialize();
                pimpl->qtControl->PutSizing (qtMovieFitsControl);

                if (movieFile != File())
                    loadMovie (movieFile, controllerVisible);
            }
        }
    }
}

bool QuickTimeMovieComponent::isControlCreated() const
{
    return isControlOpen();
}

bool QuickTimeMovieComponent::loadMovie (InputStream* movieStream,
                                         const bool isControllerVisible)
{
    const ScopedPointer<InputStream> movieStreamDeleter (movieStream);

    movieFile = File();
    movieLoaded = false;
    pimpl->qtMovie = 0;
    controllerVisible = isControllerVisible;
    createControlIfNeeded();

    if (isControlCreated())
    {
        if (pimpl->qtControl != 0)
        {
            pimpl->qtControl->Put_MovieHandle (0);
            pimpl->clearHandle();

            Movie movie;
            if (juce_OpenQuickTimeMovieFromStream (movieStream, movie, pimpl->dataHandle))
            {
                pimpl->qtControl->Put_MovieHandle ((long) (pointer_sized_int) movie);

                pimpl->qtMovie = pimpl->qtControl->GetMovie();

                if (pimpl->qtMovie != 0)
                    pimpl->qtMovie->PutMovieControllerType (isControllerVisible ? qtMovieControllerTypeStandard
                                                                                : qtMovieControllerTypeNone);
            }

            if (movie == 0)
                pimpl->clearHandle();
        }

        movieLoaded = (pimpl->qtMovie != 0);
    }
    else
    {
        // You're trying to open a movie when the control hasn't yet been created, probably because
        // you've not yet added this component to a Window and made the whole component hierarchy visible.
        jassertfalse;
    }

    return movieLoaded;
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();
    movieFile = File();
    movieLoaded = false;
    pimpl->qtMovie = 0;

    if (pimpl->qtControl != 0)
        pimpl->qtControl->Put_MovieHandle (0);

    pimpl->clearHandle();
}

File QuickTimeMovieComponent::getCurrentMovieFile() const
{
    return movieFile;
}

bool QuickTimeMovieComponent::isMovieOpen() const
{
    return movieLoaded;
}

double QuickTimeMovieComponent::getMovieDuration() const
{
    if (pimpl->qtMovie != 0)
        return pimpl->qtMovie->GetDuration() / (double) pimpl->qtMovie->GetTimeScale();

    return 0.0;
}

void QuickTimeMovieComponent::getMovieNormalSize (int& width, int& height) const
{
    if (pimpl->qtMovie != 0)
    {
        struct QTRECT r = pimpl->qtMovie->GetNaturalRect();

        width = r.right - r.left;
        height = r.bottom - r.top;
    }
    else
    {
        width = height = 0;
    }
}

void QuickTimeMovieComponent::play()
{
    if (pimpl->qtMovie != 0)
        pimpl->qtMovie->Play();
}

void QuickTimeMovieComponent::stop()
{
    if (pimpl->qtMovie != 0)
        pimpl->qtMovie->Stop();
}

bool QuickTimeMovieComponent::isPlaying() const
{
    return pimpl->qtMovie != 0 && pimpl->qtMovie->GetRate() != 0.0f;
}

void QuickTimeMovieComponent::setPosition (const double seconds)
{
    if (pimpl->qtMovie != 0)
        pimpl->qtMovie->PutTime ((long) (seconds * pimpl->qtMovie->GetTimeScale()));
}

double QuickTimeMovieComponent::getPosition() const
{
    if (pimpl->qtMovie != 0)
        return pimpl->qtMovie->GetTime() / (double) pimpl->qtMovie->GetTimeScale();

    return 0.0;
}

void QuickTimeMovieComponent::setSpeed (const float newSpeed)
{
    if (pimpl->qtMovie != 0)
        pimpl->qtMovie->PutRate (newSpeed);
}

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    if (pimpl->qtMovie != 0)
    {
        pimpl->qtMovie->PutAudioVolume (newVolume);
        pimpl->qtMovie->PutAudioMute (newVolume <= 0);
    }
}

float QuickTimeMovieComponent::getMovieVolume() const
{
    if (pimpl->qtMovie != 0)
        return pimpl->qtMovie->GetAudioVolume();

    return 0.0f;
}

void QuickTimeMovieComponent::setLooping (const bool shouldLoop)
{
    if (pimpl->qtMovie != 0)
        pimpl->qtMovie->PutLoop (shouldLoop);
}

bool QuickTimeMovieComponent::isLooping() const
{
    return pimpl->qtMovie != 0 && pimpl->qtMovie->GetLoop();
}

bool QuickTimeMovieComponent::isControllerVisible() const
{
    return controllerVisible;
}

void QuickTimeMovieComponent::parentHierarchyChanged()
{
    createControlIfNeeded();
    QTCompBaseClass::parentHierarchyChanged();
}

void QuickTimeMovieComponent::visibilityChanged()
{
    createControlIfNeeded();
    QTCompBaseClass::visibilityChanged();
}

void QuickTimeMovieComponent::paint (Graphics& g)
{
    if (! isControlCreated())
        g.fillAll (Colours::black);
}


//==============================================================================
static Handle createHandleDataRef (Handle dataHandle, const char* fileName)
{
    Handle dataRef = 0;
    OSStatus err = PtrToHand (&dataHandle, &dataRef, sizeof (Handle));
    if (err == noErr)
    {
        Str255 suffix;

       #if JUCE_MSVC
        #pragma warning (push)
        #pragma warning (disable: 4244 4996)
       #endif

        suffix[0] = strlen (fileName);
        strncpy ((char*) suffix + 1, fileName, 128);

       #if JUCE_MSVC
        #pragma warning (pop)
       #endif

        err = PtrAndHand (suffix, dataRef, suffix[0] + 1);

        if (err == noErr)
        {
            long atoms[3];
            atoms[0] = EndianU32_NtoB (3 * sizeof (long));
            atoms[1] = EndianU32_NtoB (kDataRefExtensionMacOSFileType);
            atoms[2] = EndianU32_NtoB (MovieFileType);

            err = PtrAndHand (atoms, dataRef, 3 * sizeof (long));

            if (err == noErr)
                return dataRef;
        }

        DisposeHandle (dataRef);
    }

    return 0;
}

static CFStringRef juceStringToCFString (const String& s)
{
    return CFStringCreateWithCString (kCFAllocatorDefault, s.toUTF8(), kCFStringEncodingUTF8);
}

static bool openMovie (QTNewMoviePropertyElement* props, int prop, Movie& movie)
{
    Boolean trueBool = true;
    props[prop].propClass = kQTPropertyClass_MovieInstantiation;
    props[prop].propID = kQTMovieInstantiationPropertyID_DontResolveDataRefs;
    props[prop].propValueSize = sizeof (trueBool);
    props[prop].propValueAddress = &trueBool;
    ++prop;

    props[prop].propClass = kQTPropertyClass_MovieInstantiation;
    props[prop].propID = kQTMovieInstantiationPropertyID_AsyncOK;
    props[prop].propValueSize = sizeof (trueBool);
    props[prop].propValueAddress = &trueBool;
    ++prop;

    Boolean isActive = true;
    props[prop].propClass = kQTPropertyClass_NewMovieProperty;
    props[prop].propID = kQTNewMoviePropertyID_Active;
    props[prop].propValueSize = sizeof (isActive);
    props[prop].propValueAddress = &isActive;
    ++prop;

    MacSetPort (0);

    jassert (prop <= 5);
    OSStatus err = NewMovieFromProperties (prop, props, 0, 0, &movie);

    return err == noErr;
}

bool juce_OpenQuickTimeMovieFromStream (InputStream* input, Movie& movie, Handle& dataHandle)
{
    if (input == nullptr)
        return false;

    dataHandle = 0;
    bool ok = false;

    QTNewMoviePropertyElement props[5] = { 0 };
    int prop = 0;

    DataReferenceRecord dr;
    props[prop].propClass = kQTPropertyClass_DataLocation;
    props[prop].propID = kQTDataLocationPropertyID_DataReference;
    props[prop].propValueSize = sizeof (dr);
    props[prop].propValueAddress = &dr;
    ++prop;

    FileInputStream* const fin = dynamic_cast<FileInputStream*> (input);

    if (fin != nullptr)
    {
        CFStringRef filePath = juceStringToCFString (fin->getFile().getFullPathName());

        QTNewDataReferenceFromFullPathCFString (filePath, (QTPathStyle) kQTNativeDefaultPathStyle, 0,
                                                &dr.dataRef, &dr.dataRefType);


        ok = openMovie (props, prop, movie);

        DisposeHandle (dr.dataRef);
        CFRelease (filePath);
    }
    else
    {
        // sanity-check because this currently needs to load the whole stream into memory..
        jassert (input->getTotalLength() < 50 * 1024 * 1024);

        dataHandle = NewHandle ((Size) input->getTotalLength());
        HLock (dataHandle);
        // read the entire stream into memory - this is a pain, but can't get it to work
        // properly using a custom callback to supply the data.
        input->read (*dataHandle, (int) input->getTotalLength());
        HUnlock (dataHandle);

        // different types to get QT to try. (We should really be a bit smarter here by
        // working out in advance which one the stream contains, rather than just trying
        // each one)
        static const char* const suffixesToTry[] = { "\04.mov", "\04.mp3",
                                                     "\04.avi", "\04.m4a" };

        for (int i = 0; i < numElementsInArray (suffixesToTry) && ! ok; ++i)
        {
            /*  // this fails for some bizarre reason - it can be bodged to work with
                // movies, but can't seem to do it for other file types..
            QTNewMovieUserProcRecord procInfo;
            procInfo.getMovieUserProc = NewGetMovieUPP (readMovieStreamProc);
            procInfo.getMovieUserProcRefcon = this;
            procInfo.defaultDataRef.dataRef = dataRef;
            procInfo.defaultDataRef.dataRefType = HandleDataHandlerSubType;

            props[prop].propClass = kQTPropertyClass_DataLocation;
            props[prop].propID = kQTDataLocationPropertyID_MovieUserProc;
            props[prop].propValueSize = sizeof (procInfo);
            props[prop].propValueAddress = (void*) &procInfo;
            ++prop; */

            dr.dataRef = createHandleDataRef (dataHandle, suffixesToTry [i]);
            dr.dataRefType = HandleDataHandlerSubType;
            ok = openMovie (props, prop, movie);

            DisposeHandle (dr.dataRef);
        }
    }

    return ok;
}

bool QuickTimeMovieComponent::loadMovie (const File& movieFile_,
                                         const bool isControllerVisible)
{
    const bool ok = loadMovie (static_cast<InputStream*> (movieFile_.createInputStream()), isControllerVisible);
    movieFile = movieFile_;
    return ok;
}

bool QuickTimeMovieComponent::loadMovie (const URL& movieURL,
                                         const bool isControllerVisible)
{
    return loadMovie (static_cast<InputStream*> (movieURL.createInputStream (false)), isControllerVisible);
}

void QuickTimeMovieComponent::goToStart()
{
    setPosition (0.0);
}

void QuickTimeMovieComponent::setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                                               RectanglePlacement placement)
{
    int normalWidth, normalHeight;
    getMovieNormalSize (normalWidth, normalHeight);

    const Rectangle<int> normalSize (0, 0, normalWidth, normalHeight);

    if (! (spaceToFitWithin.isEmpty() || normalSize.isEmpty()))
        setBounds (placement.appliedTo (normalSize, spaceToFitWithin));
    else
        setBounds (spaceToFitWithin);
}
