/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_QUICKTIME

using namespace QTOLibrary;
using namespace QTOControlLib;

bool juce_OpenQuickTimeMovieFromStream (InputStream* input, Movie& movie, Handle& dataHandle);

static bool isQTAvailable = false;


//==============================================================================
struct QTMovieCompInternal
{
    QTMovieCompInternal()
        : dataHandle (0)
    {
    }

    ~QTMovieCompInternal()
    {
        clearHandle();
    }

    IQTControlPtr qtControlInternal;
    IQTMoviePtr qtMovieInternal;

    Handle dataHandle;

    void clearHandle()
    {
        if (dataHandle != 0)
        {
            DisposeHandle (dataHandle);
            dataHandle = 0;
        }
    }
};

#define qtControl  (((QTMovieCompInternal*) internal)->qtControlInternal)
#define qtMovie    (((QTMovieCompInternal*) internal)->qtMovieInternal)

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : movieLoaded (false),
      controllerVisible (true)
{
    internal = new QTMovieCompInternal();
    setMouseEventsAllowed (false);
}

QuickTimeMovieComponent::~QuickTimeMovieComponent()
{
    closeMovie();
    qtControl = 0;

    deleteControl();

    delete internal;
    internal = 0;
}

bool QuickTimeMovieComponent::isQuickTimeAvailable() throw()
{
    if (! isQTAvailable)
    {
        isQTAvailable = (InitializeQTML (0) == noErr)
                           && (EnterMovies() == noErr);
    }

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
            qtControl = (IQTControl*) queryInterface (&qtInterfaceIID);

            if (qtControl != 0)
            {
                qtControl->Release(); // it has one ref too many at this point

                qtControl->QuickTimeInitialize();
                qtControl->PutSizing (qtMovieFitsControl);

                if (movieFile != File::nonexistent)
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
    movieFile = File::nonexistent;
    movieLoaded = false;
    qtMovie = 0;
    controllerVisible = isControllerVisible;
    createControlIfNeeded();

    if (isControlCreated())
    {
        if (qtControl != 0)
        {
            qtControl->Put_MovieHandle (0);
            ((QTMovieCompInternal*) internal)->clearHandle();

            Movie movie;
            if (juce_OpenQuickTimeMovieFromStream (movieStream, movie, ((QTMovieCompInternal*) internal)->dataHandle))
            {
                qtControl->Put_MovieHandle ((long) (pointer_sized_int) movie);

                qtMovie = qtControl->GetMovie();

                if (qtMovie != 0)
                    qtMovie->PutMovieControllerType (isControllerVisible ? qtMovieControllerTypeStandard
                                                                         : qtMovieControllerTypeNone);
            }

            if (movie == 0)
                ((QTMovieCompInternal*) internal)->clearHandle();
        }

        movieLoaded = (qtMovie != 0);
    }
    else
    {
        // You're trying to open a movie when the control hasn't yet been created, probably because
        // you've not yet added this component to a Window and made the whole component hierarchy visible.
        jassertfalse
    }

    delete movieStream;
    return movieLoaded;
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();
    movieFile = File::nonexistent;
    movieLoaded = false;
    qtMovie = 0;

    if (qtControl != 0)
        qtControl->Put_MovieHandle (0);

    ((QTMovieCompInternal*) internal)->clearHandle();
}

const File QuickTimeMovieComponent::getCurrentMovieFile() const
{
    return movieFile;
}

bool QuickTimeMovieComponent::isMovieOpen() const
{
    return movieLoaded;
}

double QuickTimeMovieComponent::getMovieDuration() const
{
    if (qtMovie != 0)
        return qtMovie->GetDuration() / (double) qtMovie->GetTimeScale();

    return 0.0;
}

void QuickTimeMovieComponent::getMovieNormalSize (int& width, int& height) const
{
    if (qtMovie != 0)
    {
        struct QTRECT r = qtMovie->GetNaturalRect();

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
    if (qtMovie != 0)
        qtMovie->Play();
}

void QuickTimeMovieComponent::stop()
{
    if (qtMovie != 0)
        qtMovie->Stop();
}

bool QuickTimeMovieComponent::isPlaying() const
{
    return qtMovie != 0 && qtMovie->GetRate() != 0.0f;
}

void QuickTimeMovieComponent::setPosition (const double seconds)
{
    if (qtMovie != 0)
        qtMovie->PutTime ((long) (seconds * qtMovie->GetTimeScale()));
}

double QuickTimeMovieComponent::getPosition() const
{
    if (qtMovie != 0)
        return qtMovie->GetTime() / (double) qtMovie->GetTimeScale();

    return 0.0;
}

void QuickTimeMovieComponent::setSpeed (const float newSpeed)
{
    if (qtMovie != 0)
        qtMovie->PutRate (newSpeed);
}

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    if (qtMovie != 0)
    {
        qtMovie->PutAudioVolume (newVolume);
        qtMovie->PutAudioMute (newVolume <= 0);
    }
}

float QuickTimeMovieComponent::getMovieVolume() const
{
    if (qtMovie != 0)
        return qtMovie->GetAudioVolume();

    return 0.0f;
}

void QuickTimeMovieComponent::setLooping (const bool shouldLoop)
{
    if (qtMovie != 0)
        qtMovie->PutLoop (shouldLoop);
}

bool QuickTimeMovieComponent::isLooping() const
{
    return qtMovie != 0 && qtMovie->GetLoop();
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

        CharacterFunctions::copy ((char*) suffix, fileName, 128);

        StringPtr name = suffix;
        err = PtrAndHand (name, dataRef, name[0] + 1);

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
    const int len = s.length();
    const juce_wchar* const t = (const juce_wchar*) s;

    HeapBlock <UniChar> temp (len + 2);
    for (int i = 0; i <= len; ++i)
        temp[i] = t[i];

    return CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
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
    if (input == 0)
        return false;

    dataHandle = 0;
    bool ok = false;

    QTNewMoviePropertyElement props[5];
    zeromem (props, sizeof (props));
    int prop = 0;

    DataReferenceRecord dr;
    props[prop].propClass = kQTPropertyClass_DataLocation;
    props[prop].propID = kQTDataLocationPropertyID_DataReference;
    props[prop].propValueSize = sizeof (dr);
    props[prop].propValueAddress = (void*) &dr;
    ++prop;

    FileInputStream* const fin = dynamic_cast <FileInputStream*> (input);

    if (fin != 0)
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
        const char* const suffixesToTry[] = { "\04.mov", "\04.mp3",
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
    const bool ok = loadMovie ((InputStream*) movieFile_.createInputStream(), isControllerVisible);
    movieFile = movieFile_;
    return ok;
}

bool QuickTimeMovieComponent::loadMovie (const URL& movieURL,
                                         const bool isControllerVisible)
{
    return loadMovie ((InputStream*) movieURL.createInputStream (false), isControllerVisible);
}

void QuickTimeMovieComponent::goToStart()
{
    setPosition (0.0);
}

void QuickTimeMovieComponent::setBoundsWithCorrectAspectRatio (const Rectangle& spaceToFitWithin,
                                                               const RectanglePlacement& placement)
{
    int normalWidth, normalHeight;
    getMovieNormalSize (normalWidth, normalHeight);

    if (normalWidth > 0 && normalHeight > 0 && ! spaceToFitWithin.isEmpty())
    {
        double x = 0.0, y = 0.0, w = normalWidth, h = normalHeight;

        placement.applyTo (x, y, w, h,
                           spaceToFitWithin.getX(), spaceToFitWithin.getY(),
                           spaceToFitWithin.getWidth(), spaceToFitWithin.getHeight());

        if (w > 0 && h > 0)
        {
            setBounds (roundToInt (x), roundToInt (y),
                       roundToInt (w), roundToInt (h));
        }
    }
    else
    {
        setBounds (spaceToFitWithin);
    }
}

#endif
