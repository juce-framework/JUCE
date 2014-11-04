/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_MAC_CARBONVIEWWRAPPERCOMPONENT_H_INCLUDED
#define JUCE_MAC_CARBONVIEWWRAPPERCOMPONENT_H_INCLUDED


//==============================================================================
/**
    Creates a floating carbon window that can be used to hold a carbon UI.

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
          keepPluginWindowWhenHidden (false),
          wrapperWindow (0),
          carbonWindow (0),
          embeddedView (0),
          recursiveResize (false),
          repaintChildOnCreation (true)
    {
    }

    ~CarbonViewWrapperComponent()
    {
        jassert (embeddedView == 0); // must call deleteWindow() in the subclass's destructor!
    }

    virtual HIViewRef attachView (WindowRef windowRef, HIViewRef rootView) = 0;
    virtual void removeView (HIViewRef embeddedView) = 0;
    virtual void handleMouseDown (int, int) {}
    virtual void handlePaint() {}

    virtual bool getEmbeddedViewSize (int& w, int& h)
    {
        if (embeddedView == 0)
            return false;

        HIRect bounds;
        HIViewGetBounds (embeddedView, &bounds);
        w = jmax (1, roundToInt (bounds.size.width));
        h = jmax (1, roundToInt (bounds.size.height));
        return true;
    }

    void createWindow()
    {
        if (wrapperWindow == 0)
        {
            Rect r;
            r.left   = (short) getScreenX();
            r.top    = (short) getScreenY();
            r.right  = (short) (r.left + getWidth());
            r.bottom = (short) (r.top + getHeight());

            CreateNewWindow (kDocumentWindowClass,
                             (WindowAttributes) (kWindowStandardHandlerAttribute | kWindowCompositingAttribute
                                                  | kWindowNoShadowAttribute | kWindowNoTitleBarAttribute),
                             &r, &wrapperWindow);

            jassert (wrapperWindow != 0);
            if (wrapperWindow == 0)
                return;

            carbonWindow = [[NSWindow alloc] initWithWindowRef: wrapperWindow];

            [getOwnerWindow() addChildWindow: carbonWindow
                                     ordered: NSWindowAbove];

            embeddedView = attachView (wrapperWindow, HIViewGetRoot (wrapperWindow));

            // Check for the plugin creating its own floating window, and if there is one,
            // we need to reparent it to make it visible..
            if (NSWindow* floatingChildWindow = [[carbonWindow childWindows] objectAtIndex: 0])
                [getOwnerWindow() addChildWindow: floatingChildWindow
                                         ordered: NSWindowAbove];

            EventTypeSpec windowEventTypes[] =
            {
                { kEventClassWindow, kEventWindowGetClickActivation },
                { kEventClassWindow, kEventWindowHandleDeactivate },
                { kEventClassWindow, kEventWindowBoundsChanging },
                { kEventClassMouse,  kEventMouseDown },
                { kEventClassMouse,  kEventMouseMoved },
                { kEventClassMouse,  kEventMouseDragged },
                { kEventClassMouse,  kEventMouseUp },
                { kEventClassWindow, kEventWindowDrawContent },
                { kEventClassWindow, kEventWindowShown },
                { kEventClassWindow, kEventWindowHidden }
            };

            EventHandlerUPP upp = NewEventHandlerUPP (carbonEventCallback);
            InstallWindowEventHandler (wrapperWindow, upp,
                                       sizeof (windowEventTypes) / sizeof (EventTypeSpec),
                                       windowEventTypes, this, &eventHandlerRef);

            setOurSizeToEmbeddedViewSize();
            setEmbeddedWindowToOurSize();

            creationTime = Time::getCurrentTime();
        }
    }

    void deleteWindow()
    {
        removeView (embeddedView);
        embeddedView = 0;

        if (wrapperWindow != 0)
        {
            NSWindow* ownerWindow = getOwnerWindow();

            if ([[ownerWindow childWindows] count] > 0)
            {
                [ownerWindow removeChildWindow: carbonWindow];
                [carbonWindow close];
            }

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

                if (Component* p = getParentComponent())
                    p->setSize (w, h);
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
                r.size.width  = (float) getWidth();
                r.size.height = (float) getHeight();
                HIViewSetFrame (embeddedView, &r);
            }

            if (wrapperWindow != 0)
            {
                jassert (getTopLevelComponent()->getDesktopScaleFactor() == 1.0f);
                Rectangle<int> screenBounds (getScreenBounds() * Desktop::getInstance().getGlobalScaleFactor());

                Rect wr;
                wr.left   = (short) screenBounds.getX();
                wr.top    = (short) screenBounds.getY();
                wr.right  = (short) screenBounds.getRight();
                wr.bottom = (short) screenBounds.getBottom();

                SetWindowBounds (wrapperWindow, kWindowContentRgn, &wr);

                // This group stuff is mainly a workaround for Mackie plugins like FinalMix..
                WindowGroupRef group = GetWindowGroup (wrapperWindow);
                WindowRef attachedWindow;

                if (GetIndexedWindow (group, 2, kWindowGroupContentsReturnWindows, &attachedWindow) == noErr)
                {
                    SelectWindow (attachedWindow);
                    ActivateWindow (attachedWindow, TRUE);
                    HideWindow (wrapperWindow);
                }

                ShowWindow (wrapperWindow);
            }

            recursiveResize = false;
        }
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        setEmbeddedWindowToOurSize();
    }

    // (overridden to intercept movements of the top-level window)
    void componentMovedOrResized (Component& component, bool wasMoved, bool wasResized) override
    {
        ComponentMovementWatcher::componentMovedOrResized (component, wasMoved, wasResized);

        if (&component == getTopLevelComponent())
            setEmbeddedWindowToOurSize();
    }

    void componentPeerChanged() override
    {
        deleteWindow();
        createWindow();
    }

    void componentVisibilityChanged() override
    {
        if (isShowing())
            createWindow();
        else if (! keepPluginWindowWhenHidden)
            deleteWindow();

        setEmbeddedWindowToOurSize();
    }

    static void recursiveHIViewRepaint (HIViewRef view)
    {
        HIViewSetNeedsDisplay (view, true);
        HIViewRef child = HIViewGetFirstSubview (view);

        while (child != 0)
        {
            recursiveHIViewRepaint (child);
            child = HIViewGetNextView (child);
        }
    }

    void timerCallback() override
    {
        if (isShowing())
        {
            setOurSizeToEmbeddedViewSize();

            // To avoid strange overpainting problems when the UI is first opened, we'll
            // repaint it a few times during the first second that it's on-screen..
            if (repaintChildOnCreation && (Time::getCurrentTime() - creationTime).inMilliseconds() < 1000)
                recursiveHIViewRepaint (HIViewGetRoot (wrapperWindow));
        }
    }

    void setRepaintsChildHIViewWhenCreated (bool b) noexcept
    {
        repaintChildOnCreation = b;
    }

    OSStatus carbonEventHandler (EventHandlerCallRef /*nextHandlerRef*/, EventRef event)
    {
        switch (GetEventKind (event))
        {
            case kEventWindowHandleDeactivate:
                ActivateWindow (wrapperWindow, TRUE);
                return noErr;

            case kEventWindowGetClickActivation:
            {
                getTopLevelComponent()->toFront (false);
                [carbonWindow makeKeyAndOrderFront: nil];

                ClickActivationResult howToHandleClick = kActivateAndHandleClick;

                SetEventParameter (event, kEventParamClickActivation, typeClickActivationResult,
                                   sizeof (ClickActivationResult), &howToHandleClick);

                if (embeddedView != 0)
                    HIViewSetNeedsDisplay (embeddedView, true);

                return noErr;
            }
        }

        return eventNotHandledErr;
    }

    static pascal OSStatus carbonEventCallback (EventHandlerCallRef nextHandlerRef, EventRef event, void* userData)
    {
        return ((CarbonViewWrapperComponent*) userData)->carbonEventHandler (nextHandlerRef, event);
    }

    bool keepPluginWindowWhenHidden;

protected:
    WindowRef wrapperWindow;
    NSWindow* carbonWindow;
    HIViewRef embeddedView;
    bool recursiveResize, repaintChildOnCreation;
    Time creationTime;

    EventHandlerRef eventHandlerRef;

    NSWindow* getOwnerWindow() const    { return [((NSView*) getWindowHandle()) window]; }
};

#endif   // JUCE_MAC_CARBONVIEWWRAPPERCOMPONENT_H_INCLUDED
