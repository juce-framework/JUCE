/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../../juce_Config.h"

#if JUCE_QUICKTIME

#ifdef _MSC_VER
  #pragma warning (disable: 4514)
#endif

#ifdef _WIN32
 #include <windows.h>

 #ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable : 4100)
 #endif

 /* If you've got an include error here, you probably need to install the QuickTime SDK and
    add its header directory to your include path.

    Alternatively, if you don't need any QuickTime services, just turn off the JUC_QUICKTIME
    flag in juce_Config.h
 */
 #include <Movies.h>
 #include <QTML.h>
 #include <QuickTimeComponents.h>
 #include <MediaHandlers.h>
 #include <ImageCodec.h>

 #ifdef _MSC_VER
   #pragma warning (pop)
 #endif

 // If you've got QuickTime 7 installed, then these COM objects should be found in
 // the "\Program Files\Quicktime" directory. You'll need to add this directory to
 // your include search path to make these import statements work.
 #import <QTOLibrary.dll>
 #import <QTOControl.dll>

 using namespace QTOLibrary;
 using namespace QTOControlLib;
#else
 #include <Carbon/Carbon.h>
 #include <QuickTime/Movies.h>
 #include <QuickTime/QuickTimeComponents.h>
 #include <QuickTime/MediaHandlers.h>
#endif

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_QuickTimeMovieComponent.h"
#include "../../../../juce_core/io/files/juce_FileInputStream.h"

bool juce_OpenQuickTimeMovieFromStream (InputStream* input, Movie& movie, Handle& dataHandle);

static bool hasLoadedQT = false;
static bool isQTAvailable = false;


//==============================================================================
struct QTMovieCompInternal
{
    QTMovieCompInternal()
        : dataHandle (0)
    {
#if JUCE_MAC
        movie = 0;
        controller = 0;
#endif
    }

    ~QTMovieCompInternal()
    {
        clearHandle();
    }

#if JUCE_MAC
    Movie movie;
    MovieController controller;
#else
    IQTControlPtr qtControlInternal;
    IQTMoviePtr qtMovieInternal;
#endif

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

#if JUCE_WIN32

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
    if (! hasLoadedQT)
    {
        hasLoadedQT = true;

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
    QTWinBaseClass::parentHierarchyChanged();
}

void QuickTimeMovieComponent::visibilityChanged()
{
    createControlIfNeeded();
    QTWinBaseClass::visibilityChanged();
}

void QuickTimeMovieComponent::paint (Graphics& g)
{
    if (! isControlCreated())
        g.fillAll (Colours::black);
}

#endif

//==============================================================================
#if JUCE_MAC

#include "../../../../juce_core/misc/juce_PlatformUtilities.h"
#include "../../../events/juce_MessageManager.h"
#include "../../graphics/geometry/juce_RectangleList.h"

static VoidArray activeQTWindows (2);

struct MacClickEventData
{
    ::Point where;
    long when;
    long modifiers;
};

void OfferMouseClickToQuickTime (WindowRef window,
                                 ::Point where, long when, long modifiers,
                                 Component* topLevelComp)
{
    if (hasLoadedQT)
    {
        for (int i = activeQTWindows.size(); --i >= 0;)
        {
            QuickTimeMovieComponent* const qtw = (QuickTimeMovieComponent*) activeQTWindows[i];

            if (qtw->isVisible() && topLevelComp->isParentOf (qtw))
            {
                MacClickEventData data;
                data.where = where;
                data.when = when;
                data.modifiers = modifiers;

                qtw->handleMCEvent (&data);
            }
        }
    }
}

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : internal (new QTMovieCompInternal()),
      associatedWindow (0),
      controllerVisible (false),
      controllerAssignedToWindow (false),
      reentrant (false)
{
    if (! hasLoadedQT)
    {
        hasLoadedQT = true;
        isQTAvailable = EnterMovies() == noErr;
    }

    setOpaque (true);
    setVisible (true);

    activeQTWindows.add (this);
}

QuickTimeMovieComponent::~QuickTimeMovieComponent()
{
    closeMovie();

    activeQTWindows.removeValue ((void*) this);

    QTMovieCompInternal* const i = (QTMovieCompInternal*) internal;
    delete i;

    if (activeQTWindows.size() == 0 && isQTAvailable)
    {
        isQTAvailable = false;
        hasLoadedQT = false;

        ExitMovies();
    }
}

bool QuickTimeMovieComponent::isQuickTimeAvailable() throw()
{
    if (! hasLoadedQT)
    {
        hasLoadedQT = true;
        isQTAvailable = EnterMovies() == noErr;
    }

    return isQTAvailable;
}

bool QuickTimeMovieComponent::loadMovie (InputStream* movieStream,
                                         const bool controllerVisible_)
{
    if (! isQTAvailable)
        return false;

    closeMovie();
    movieFile = File::nonexistent;

    if (getPeer() == 0)
    {
        // To open a movie, this component must be visible inside a functioning window, so that
        // the QT control can be assigned to the window.
        jassertfalse
        return false;
    }

    controllerVisible = controllerVisible_;

    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    GrafPtr savedPort;
    GetPort (&savedPort);
    bool result = false;

    if (juce_OpenQuickTimeMovieFromStream (movieStream, qmci->movie, qmci->dataHandle))
    {
        qmci->controller = 0;

        void* window = getWindowHandle();

        if (window != associatedWindow && window != 0)
            associatedWindow = window;

        assignMovieToWindow();

        SetMovieActive (qmci->movie, true);
        SetMovieProgressProc (qmci->movie, (MovieProgressUPP) -1, 0);

        startTimer (1000 / 50); // this needs to be quite a high frequency for smooth playback
        result = true;

        repaint();
    }

    MacSetPort (savedPort);

    return result;
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();

    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->controller != 0)
    {
        DisposeMovieController (qmci->controller);
        qmci->controller = 0;
    }

    if (qmci->movie != 0)
    {
        DisposeMovie (qmci->movie);
        qmci->movie = 0;
    }

    qmci->clearHandle();

    stopTimer();
    movieFile = File::nonexistent;
}

bool QuickTimeMovieComponent::isMovieOpen() const
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;
    return qmci->movie != 0 && qmci->controller != 0;
}

const File QuickTimeMovieComponent::getCurrentMovieFile() const
{
    return movieFile;
}

static GrafPtr getPortForWindow (void* window)
{
    if (window == 0)
        return 0;

    return (GrafPtr) GetWindowPort ((WindowRef) window);
}

void QuickTimeMovieComponent::assignMovieToWindow()
{
    if (reentrant)
        return;

    reentrant = true;

    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;
    if (qmci->controller != 0)
    {
        DisposeMovieController (qmci->controller);
        qmci->controller = 0;
    }

    controllerAssignedToWindow = false;

    void* window = getWindowHandle();
    GrafPtr port = getPortForWindow (window);

    if (port != 0)
    {
        GrafPtr savedPort;
        GetPort (&savedPort);

        SetMovieGWorld (qmci->movie, (CGrafPtr) port, 0);
        MacSetPort (port);

        Rect r;
        r.top = 0;
        r.left = 0;
        r.right  = (short) jmax (1, getWidth());
        r.bottom = (short) jmax (1, getHeight());
        SetMovieBox (qmci->movie, &r);

        // create the movie controller
        qmci->controller = NewMovieController (qmci->movie, &r,
                                               controllerVisible ? mcTopLeftMovie
                                                                 : mcNotVisible);

        if (qmci->controller != 0)
        {
            MCEnableEditing (qmci->controller, true);

            MCDoAction (qmci->controller, mcActionSetUseBadge, (void*) false);
            MCDoAction (qmci->controller, mcActionSetLoopIsPalindrome, (void*) false);
            setLooping (looping);

            MCDoAction (qmci->controller, mcActionSetFlags,
                        (void*) (pointer_sized_int) (mcFlagSuppressMovieFrame | (controllerVisible ? 0 : (mcFlagSuppressStepButtons | mcFlagSuppressSpeakerButton))));

            MCSetControllerBoundsRect (qmci->controller, &r);

            controllerAssignedToWindow = true;

            resized();
        }

        MacSetPort (savedPort);
    }
    else
    {
        SetMovieGWorld (qmci->movie, 0, 0);
    }

    reentrant = false;
}

void QuickTimeMovieComponent::play()
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        StartMovie (qmci->movie);
}

void QuickTimeMovieComponent::stop()
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        StopMovie (qmci->movie);
}

bool QuickTimeMovieComponent::isPlaying() const
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    return qmci->movie != 0 && GetMovieRate (qmci->movie) != 0;
}

void QuickTimeMovieComponent::setPosition (const double seconds)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->controller != 0)
    {
        TimeRecord time;
        time.base = GetMovieTimeBase (qmci->movie);
        time.scale = 100000;
        const uint64 t = (uint64) (100000.0 * seconds);
        time.value.lo = (UInt32) (t & 0xffffffff);
        time.value.hi = (UInt32) (t >> 32);

        SetMovieTime (qmci->movie, &time);
        timerCallback(); // to call MCIdle
    }
    else
    {
        jassertfalse    // no movie is open, so can't set the position.
    }
}

double QuickTimeMovieComponent::getPosition() const
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
    {
        TimeRecord time;
        GetMovieTime (qmci->movie, &time);

        return ((int64) (((uint64) time.value.hi << 32) | (uint64) time.value.lo))
                    / (double) time.scale;
    }

    return 0.0;
}

void QuickTimeMovieComponent::setSpeed (const float newSpeed)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        SetMovieRate (qmci->movie, (Fixed) (newSpeed * (Fixed) 0x00010000L));
}

double QuickTimeMovieComponent::getMovieDuration() const
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        return GetMovieDuration (qmci->movie) / (double) GetMovieTimeScale (qmci->movie);

    return 0.0;
}

void QuickTimeMovieComponent::setLooping (const bool shouldLoop)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;
    looping = shouldLoop;

    if (qmci->controller != 0)
        MCDoAction (qmci->controller, mcActionSetLooping, (void*) shouldLoop);
}

bool QuickTimeMovieComponent::isLooping() const
{
    return looping;
}

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        SetMovieVolume (qmci->movie, jlimit ((short) 0, (short) 0x100, (short) (newVolume * 0x0100)));
}

float QuickTimeMovieComponent::getMovieVolume() const
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
        return jmax (0.0f, GetMovieVolume (qmci->movie) / (float) 0x0100);

    return 0.0f;
}

void QuickTimeMovieComponent::getMovieNormalSize (int& width, int& height) const
{
    width = 0;
    height = 0;

    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie != 0)
    {
        Rect r;
        GetMovieNaturalBoundsRect (qmci->movie, &r);
        width = r.right - r.left;
        height = r.bottom - r.top;
    }
}

void QuickTimeMovieComponent::paint (Graphics& g)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->movie == 0 || qmci->controller == 0)
    {
        g.fillAll (Colours::black);
        return;
    }

    GrafPtr savedPort;
    GetPort (&savedPort);

    MacSetPort (getPortForWindow (getWindowHandle()));
    MCDraw (qmci->controller, (WindowRef) getWindowHandle());

    MacSetPort (savedPort);

    ComponentPeer* const peer = getPeer();

    if (peer != 0)
    {
        peer->addMaskedRegion (getScreenX() - peer->getScreenX(),
                               getScreenY() - peer->getScreenY(),
                               getWidth(), getHeight());
    }

    timerCallback();
}

static const Rectangle getMoviePos (Component* const c)
{
    return Rectangle (c->getScreenX() - c->getTopLevelComponent()->getScreenX(),
                      c->getScreenY() - c->getTopLevelComponent()->getScreenY(),
                      jmax (1, c->getWidth()),
                      jmax (1, c->getHeight()));
}

void QuickTimeMovieComponent::moved()
{
    resized();
}

void QuickTimeMovieComponent::resized()
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->controller != 0 && isShowing())
    {
        checkWindowAssociation();

        GrafPtr port = getPortForWindow (getWindowHandle());

        if (port != 0)
        {
            GrafPtr savedPort;
            GetPort (&savedPort);

            SetMovieGWorld (qmci->movie, (CGrafPtr) port, 0);
            MacSetPort (port);

            lastPositionApplied = getMoviePos (this);

            Rect r;
            r.left   = (short) lastPositionApplied.getX();
            r.top    = (short) lastPositionApplied.getY();
            r.right  = (short) lastPositionApplied.getRight();
            r.bottom = (short) lastPositionApplied.getBottom();

            if (MCGetVisible (qmci->controller))
                MCSetControllerBoundsRect (qmci->controller, &r);
            else
                SetMovieBox (qmci->movie, &r);

            if (! isPlaying())
                timerCallback();

            MacSetPort (savedPort);

            repaint();
        }
    }
}

void QuickTimeMovieComponent::visibilityChanged()
{
    checkWindowAssociation();
    QTWinBaseClass::visibilityChanged();
}

void QuickTimeMovieComponent::timerCallback()
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->controller != 0)
    {
        if (isTimerRunning())
            startTimer (getTimerInterval());

        MCIdle (qmci->controller);

        if (lastPositionApplied != getMoviePos (this))
            resized();
    }
}

void QuickTimeMovieComponent::checkWindowAssociation()
{
    void* const window = getWindowHandle();

    if (window != associatedWindow
         || (window != 0 && ! controllerAssignedToWindow))
    {
        associatedWindow = window;
        assignMovieToWindow();
    }
}

void QuickTimeMovieComponent::parentHierarchyChanged()
{
    checkWindowAssociation();
}

void QuickTimeMovieComponent::handleMCEvent (void* ev)
{
    QTMovieCompInternal* const qmci = (QTMovieCompInternal*) internal;

    if (qmci->controller != 0 && isShowing())
    {
        MacClickEventData* data = (MacClickEventData*) ev;

        data->where.h -= getTopLevelComponent()->getScreenX();
        data->where.v -= getTopLevelComponent()->getScreenY();

        Boolean b = false;
        MCPtInController (qmci->controller, data->where, &b);

        if (b)
        {
            const int oldTimeBeforeWaitCursor = MessageManager::getInstance()->getTimeBeforeShowingWaitCursor();
            MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (0);

            MCClick (qmci->controller,
                     (WindowRef) getWindowHandle(),
                     data->where,
                     data->when,
                     data->modifiers);

            MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (oldTimeBeforeWaitCursor);
        }
    }
}

#endif

//==============================================================================
// (methods common to both platforms..)

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

    UniChar* temp = (UniChar*) juce_malloc (sizeof (UniChar) * len + 4);

    for (int i = 0; i <= len; ++i)
        temp[i] = t[i];

    CFStringRef result = CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
    juce_free (temp);

    return result;
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

#if JUCE_MAC
    SetPort (0);
#else
    MacSetPort (0);
#endif

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
            setBounds (roundDoubleToInt (x), roundDoubleToInt (y),
                       roundDoubleToInt (w), roundDoubleToInt (h));
        }
    }
    else
    {
        setBounds (spaceToFitWithin);
    }
}


END_JUCE_NAMESPACE

#endif
