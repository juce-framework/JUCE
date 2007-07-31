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
#endif

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_QuickTimeMovieComponent.h"


//==============================================================================
#if JUCE_WIN32

struct QTMovieCompInternal
{
    IQTControlPtr qtControlInternal;
    IQTMoviePtr qtMovieInternal;
};

#define qtControl  (((QTMovieCompInternal*) internal)->qtControlInternal)
#define qtMovie    (((QTMovieCompInternal*) internal)->qtMovieInternal)

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : movieLoaded (false),
      controllerVisible (true)
{
    internal = new QTMovieCompInternal();
}

QuickTimeMovieComponent::~QuickTimeMovieComponent()
{
    closeMovie();
    qtControl = 0;

    deleteControl();

    delete internal;
    internal = 0;
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

bool QuickTimeMovieComponent::loadMovie (const File& movieFile_,
                                         const bool isControllerVisible)
{
    movieFile = movieFile_;
    movieLoaded = false;
    qtMovie = 0;
    controllerVisible = isControllerVisible;
    createControlIfNeeded();

    if (isControlCreated())
    {
        if (qtControl != 0)
        {
            qtControl->PutURL ((const WCHAR*) movieFile_.getFullPathName());
            qtMovie = qtControl->GetMovie();

            if (qtMovie != 0)
                qtMovie->PutMovieControllerType (isControllerVisible ? qtMovieControllerTypeStandard
                                                                     : qtMovieControllerTypeNone);
        }

        movieLoaded = (qtMovie != 0);
        return movieLoaded;
    }
    else
    {
        // You're trying to open a movie when the control hasn't yet been created, probably because
        // you've not yet added this component to a Window and made the whole component hierarchy visible.
        jassertfalse
        return false;
    }
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();
    movieFile = File::nonexistent;
    movieLoaded = false;
    qtMovie = 0;

    if (qtControl != 0)
        qtControl->PutURL (L"");
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

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    if (qtMovie != 0)
        qtMovie->PutAudioVolume (newVolume);
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

static bool isQTAvailable = false;
static bool hasLoadedQT = false;
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
struct InternalData
{
    Movie movie;
    MovieController controller;
};

//==============================================================================
QuickTimeMovieComponent::QuickTimeMovieComponent()
    : internal (new InternalData()),
      associatedWindow (0),
      controllerVisible (false),
      controllerAssignedToWindow (false),
      reentrant (false)
{
    InternalData* const id = (InternalData*) internal;
    id->movie = 0;
    id->controller = 0;

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

    InternalData* const id = (InternalData*) internal;
    delete id;

    if (activeQTWindows.size() == 0 && isQTAvailable)
    {
        isQTAvailable = false;
        hasLoadedQT = false;

        ExitMovies();
    }
}

bool QuickTimeMovieComponent::loadMovie (const File& f,
                                         const bool controllerVisible_)
{
    if (! (isQTAvailable && f.existsAsFile()))
        return false;

    closeMovie();

    if (getPeer() == 0)
    {
        // To open a movie, this component must be visible inside a functioning window, so that
        // the QT control can be assigned to the window.
        jassertfalse
        return false;
    }

    controllerVisible = controllerVisible_;

    InternalData* const id = (InternalData*) internal;

    GrafPtr savedPort;
    GetPort (&savedPort);
    bool result = false;

    FSSpec fsSpec;

    PlatformUtilities::makeFSSpecFromPath (&fsSpec, f.getFullPathName());

    short refNum = -1;
    OSErr err;

    if ((err = OpenMovieFile (&fsSpec, &refNum, fsRdWrPerm)) == noErr
         || (err = OpenMovieFile (&fsSpec, &refNum, fsRdPerm)) == noErr)
    {
        id->controller = 0;

        short resID = 0;

        if (NewMovieFromFile (&id->movie, refNum, &resID, 0, newMovieActive, 0) == noErr
             && id->movie != 0)
        {
            void* window = getWindowHandle();

            if (window != associatedWindow && window != 0)
                associatedWindow = window;

            assignMovieToWindow();

            SetMovieActive (id->movie, true);
            SetMovieProgressProc (id->movie, (MovieProgressUPP) -1, 0);

            movieFile = f;
            startTimer (1000 / 50); // this needs to be quite a high frequency for smooth playback
            result = true;

            repaint();
        }
    }

    MacSetPort (savedPort);

    return result;
}

void QuickTimeMovieComponent::closeMovie()
{
    stop();

    InternalData* const id = (InternalData*) internal;

    if (id->controller != 0)
    {
        DisposeMovieController (id->controller);
        id->controller = 0;
    }

    if (id->movie != 0)
    {
        DisposeMovie (id->movie);
        id->movie = 0;
    }

    stopTimer();
    movieFile = File::nonexistent;
}

bool QuickTimeMovieComponent::isMovieOpen() const
{
    InternalData* const id = (InternalData*) internal;
    return id->movie != 0 && id->controller != 0;
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

    InternalData* const id = (InternalData*) internal;
    if (id->controller != 0)
    {
        DisposeMovieController (id->controller);
        id->controller = 0;
    }

    controllerAssignedToWindow = false;

    void* window = getWindowHandle();
    GrafPtr port = getPortForWindow (window);

    if (port != 0)
    {
        GrafPtr savedPort;
        GetPort (&savedPort);

        SetMovieGWorld (id->movie, (CGrafPtr) port, 0);
        MacSetPort (port);

        Rect r;
        r.top = 0;
        r.left = 0;
        r.right  = (short) jmax (1, getWidth());
        r.bottom = (short) jmax (1, getHeight());
        SetMovieBox (id->movie, &r);

        // create the movie controller
        id->controller = NewMovieController (id->movie, &r,
                                             controllerVisible ? mcTopLeftMovie
                                                               : mcNotVisible);

        if (id->controller != 0)
        {
            MCEnableEditing (id->controller, true);

            MCDoAction (id->controller, mcActionSetUseBadge, (void*) false);
            MCDoAction (id->controller, mcActionSetLoopIsPalindrome, (void*) false);
            setLooping (looping);

            MCDoAction (id->controller, mcActionSetFlags,
                        (void*) (pointer_sized_int) (mcFlagSuppressMovieFrame | (controllerVisible ? 0 : (mcFlagSuppressStepButtons | mcFlagSuppressSpeakerButton))));

            MCSetControllerBoundsRect (id->controller, &r);

            controllerAssignedToWindow = true;

            resized();
        }

        MacSetPort (savedPort);
    }

    reentrant = false;
}

void QuickTimeMovieComponent::play()
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
        StartMovie (id->movie);
}

void QuickTimeMovieComponent::stop()
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
        StopMovie (id->movie);
}

bool QuickTimeMovieComponent::isPlaying() const
{
    InternalData* const id = (InternalData*) internal;

    return id->movie != 0 && GetMovieRate (id->movie) != 0;
}

void QuickTimeMovieComponent::setPosition (const double seconds)
{
    InternalData* const id = (InternalData*) internal;

    if (id->controller != 0)
    {
        TimeRecord time;
        time.base = GetMovieTimeBase (id->movie);
        time.scale = 100000;
        const uint64 t = (uint64) (100000.0 * seconds);
        time.value.lo = (UInt32) (t & 0xffffffff);
        time.value.hi = (UInt32) (t >> 32);

        SetMovieTime (id->movie, &time);
        timerCallback(); // to call MCIdle
    }
    else
    {
        jassertfalse    // no movie is open, so can't set the position.
    }
}

double QuickTimeMovieComponent::getPosition() const
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
    {
        TimeRecord time;
        GetMovieTime (id->movie, &time);

        return ((int64) (((uint64) time.value.hi << 32) | (uint64) time.value.lo))
                    / (double) time.scale;
    }

    return 0.0;
}

double QuickTimeMovieComponent::getMovieDuration() const
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
        return GetMovieDuration (id->movie) / (double) GetMovieTimeScale (id->movie);

    return 0.0;
}

void QuickTimeMovieComponent::setLooping (const bool shouldLoop)
{
    InternalData* const id = (InternalData*) internal;
    looping = shouldLoop;

    if (id->controller != 0)
        MCDoAction (id->controller, mcActionSetLooping, (void*) shouldLoop);
}

bool QuickTimeMovieComponent::isLooping() const
{
    return looping;
}

void QuickTimeMovieComponent::setMovieVolume (const float newVolume)
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
        SetMovieVolume (id->movie, jlimit ((short) 0, (short) 0x100, (short) (newVolume * 0x0100)));
}

float QuickTimeMovieComponent::getMovieVolume() const
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
        return jmax (0.0f, GetMovieVolume (id->movie) / (float) 0x0100);

    return 0.0f;
}

void QuickTimeMovieComponent::getMovieNormalSize (int& width, int& height) const
{
    width = 0;
    height = 0;

    InternalData* const id = (InternalData*) internal;

    if (id->movie != 0)
    {
        Rect r;
        GetMovieNaturalBoundsRect (id->movie, &r);
        width = r.right - r.left;
        height = r.bottom - r.top;
    }
}

void QuickTimeMovieComponent::paint (Graphics& g)
{
    InternalData* const id = (InternalData*) internal;

    if (id->movie == 0 || id->controller == 0)
    {
        g.fillAll (Colours::black);
        return;
    }

    GrafPtr savedPort;
    GetPort (&savedPort);

    MacSetPort (getPortForWindow (getWindowHandle()));
    MCDraw (id->controller, (WindowRef) getWindowHandle());

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
    InternalData* const id = (InternalData*) internal;

    if (id->controller != 0 && isShowing())
    {
        checkWindowAssociation();

        GrafPtr port = getPortForWindow (getWindowHandle());

        if (port != 0)
        {
            GrafPtr savedPort;
            GetPort (&savedPort);

            SetMovieGWorld (id->movie, (CGrafPtr) port, 0);
            MacSetPort (port);

            lastPositionApplied = getMoviePos (this);

            Rect r;
            r.left   = (short) lastPositionApplied.getX();
            r.top    = (short) lastPositionApplied.getY();
            r.right  = (short) lastPositionApplied.getRight();
            r.bottom = (short) lastPositionApplied.getBottom();

            if (MCGetVisible (id->controller))
                MCSetControllerBoundsRect (id->controller, &r);
            else
                SetMovieBox (id->movie, &r);

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
    InternalData* const id = (InternalData*) internal;

    if (id->controller != 0)
    {
        if (isTimerRunning())
            startTimer (getTimerInterval());

        MCIdle (id->controller);

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
    InternalData* const id = (InternalData*) internal;

    if (id->controller != 0 && isShowing())
    {
        MacClickEventData* data = (MacClickEventData*) ev;

        data->where.h -= getTopLevelComponent()->getScreenX();
        data->where.v -= getTopLevelComponent()->getScreenY();

        Boolean b = false;
        MCPtInController (id->controller, data->where, &b);

        if (b)
        {
            const int oldTimeBeforeWaitCursor = MessageManager::getInstance()->getTimeBeforeShowingWaitCursor();
            MessageManager::getInstance()->setTimeBeforeShowingWaitCursor (0);

            MCClick (id->controller,
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
