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

/** Creates a floating carbon window that can be used to hold a carbon UI.

    This is a handy class that's designed to be inlined where needed, e.g.
    in the audio plugin hosting code.
*/
class CarbonViewWrapperComponent  : public Component,
                                    public ComponentMovementWatcher,
                                    public Timer
{
public:
    CarbonViewWrapperComponent()
        : ComponentMovementWatcher (this),
          wrapperWindow (0),
          embeddedView (0),
          recursiveResize (false)
    {
    }

    virtual ~CarbonViewWrapperComponent()
    {
        jassert (embeddedView == 0); // must call deleteWindow() in the subclass's destructor!
    }

    virtual HIViewRef attachView (WindowRef windowRef, HIViewRef rootView) = 0;
    virtual void removeView (HIViewRef embeddedView) = 0;
    virtual void mouseDown (int x, int y) {}
    virtual void paint() {}

    virtual bool getEmbeddedViewSize (int& w, int& h)
    {
        if (embeddedView == 0)
            return false;

        HIRect bounds;
        HIViewGetBounds (embeddedView, &bounds);
        w = jmax (1, roundFloatToInt (bounds.size.width));
        h = jmax (1, roundFloatToInt (bounds.size.height));
        return true;
    }

    void createWindow()
    {
        if (wrapperWindow == 0)
        {
            Rect r;
            r.left = getScreenX();
            r.top = getScreenY();
            r.right = r.left + getWidth();
            r.bottom = r.top + getHeight();

            CreateNewWindow (kDocumentWindowClass,
                             (WindowAttributes) (kWindowStandardHandlerAttribute | kWindowCompositingAttribute
                                                  | kWindowNoShadowAttribute | kWindowNoTitleBarAttribute),
                             &r, &wrapperWindow);

            jassert (wrapperWindow != 0);
            if (wrapperWindow == 0)
                return;

            NSWindow* carbonWindow = [[NSWindow alloc] initWithWindowRef: wrapperWindow];
            NSWindow* ownerWindow = [((NSView*) getWindowHandle()) window];

            [ownerWindow addChildWindow: carbonWindow
                                ordered: NSWindowAbove];

            embeddedView = attachView (wrapperWindow, HIViewGetRoot (wrapperWindow));

            EventTypeSpec  windowEventTypes[] = { { kEventClassWindow, kEventWindowGetClickActivation },
                                                  { kEventClassWindow, kEventWindowHandleDeactivate } };

            EventHandlerUPP upp = NewEventHandlerUPP (carbonEventCallback);
            InstallWindowEventHandler (wrapperWindow, upp,
                                       sizeof (windowEventTypes) / sizeof (EventTypeSpec),
                                       windowEventTypes, this, &eventHandlerRef);

            setOurSizeToEmbeddedViewSize();
            setEmbeddedWindowToOurSize();
        }
    }

    void deleteWindow()
    {
        removeView (embeddedView);
        embeddedView = 0;

        if (wrapperWindow != 0)
        {
            RemoveEventHandler (eventHandlerRef);
            DisposeWindow (wrapperWindow);
            wrapperWindow = 0;
        }
    }

    //==============================================================================
    void setOurSizeToEmbeddedViewSize()
    {
        int w, h;
        if (getEmbeddedViewSize (w, h))
        {
            if (w != getWidth() || h != getHeight())
            {
                startTimer (50);

                setSize (w, h);
                if (getParentComponent() != 0)
                    getParentComponent()->setSize (w, h);
            }
            else
            {
                startTimer (jlimit (50, 500, getTimerInterval() + 20));
            }
        }
        else
        {
            stopTimer();
        }
    }

    void setEmbeddedWindowToOurSize()
    {
        if (! recursiveResize)
        {
            recursiveResize = true;

            if (embeddedView != 0)
            {
                HIRect r;
                r.origin.x = 0;
                r.origin.y = 0;
                r.size.width = (float) getWidth();
                r.size.height = (float) getHeight();
                HIViewSetFrame (embeddedView, &r);
            }

            if (wrapperWindow != 0)
            {
                Rect wr;
                wr.left = getScreenX();
                wr.top = getScreenY();
                wr.right = wr.left + getWidth();
                wr.bottom = wr.top + getHeight();

                SetWindowBounds (wrapperWindow, kWindowContentRgn, &wr);
                ShowWindow (wrapperWindow);
            }

            recursiveResize = false;
        }
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/)
    {
        setEmbeddedWindowToOurSize();
    }

    void componentPeerChanged()
    {
        deleteWindow();
        createWindow();
    }

    void componentVisibilityChanged (Component&)
    {
        if (isShowing())
            createWindow();
        else
            deleteWindow();

        setEmbeddedWindowToOurSize();
    }

    void timerCallback()
    {
        setOurSizeToEmbeddedViewSize();
    }

    OSStatus carbonEventHandler (EventHandlerCallRef nextHandlerRef,
                                 EventRef event)
    {
        switch (GetEventKind (event))
        {
            case kEventWindowHandleDeactivate:
                ActivateWindow (wrapperWindow, TRUE);
                break;

            case kEventWindowGetClickActivation:
            {
                getTopLevelComponent()->toFront (false);

                ClickActivationResult howToHandleClick = kActivateAndHandleClick;

                SetEventParameter (event, kEventParamClickActivation, typeClickActivationResult,
                                   sizeof (ClickActivationResult), &howToHandleClick);
            }
            break;
        }

        return noErr;
    }

    static pascal OSStatus carbonEventCallback (EventHandlerCallRef nextHandlerRef,
                                                EventRef event, void* userData)
    {
        return ((CarbonViewWrapperComponent*) userData)->carbonEventHandler (nextHandlerRef, event);
    }

protected:
    WindowRef wrapperWindow;
    HIViewRef embeddedView;
    bool recursiveResize;

    EventHandlerRef eventHandlerRef;
};
