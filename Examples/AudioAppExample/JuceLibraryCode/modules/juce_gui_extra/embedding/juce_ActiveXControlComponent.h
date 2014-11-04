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

#ifndef JUCE_ACTIVEXCONTROLCOMPONENT_H_INCLUDED
#define JUCE_ACTIVEXCONTROLCOMPONENT_H_INCLUDED

#if JUCE_WINDOWS || DOXYGEN

//==============================================================================
/**
    A Windows-specific class that can create and embed an ActiveX control inside
    itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use createControl() to instantiate an ActiveX control. The control
    will then be moved and resized to follow the movements of this component.

    Of course, since the control is a heavyweight window, it'll obliterate any
    juce components that may overlap this component, but that's life.
*/
class JUCE_API  ActiveXControlComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    ActiveXControlComponent();

    /** Destructor. */
    ~ActiveXControlComponent();

    /** Tries to create an ActiveX control and embed it in this peer.

        The peer controlIID is a pointer to an IID structure - it's treated
        as a void* because when including the Juce headers, you might not always
        have included windows.h first, in which case IID wouldn't be defined.

        e.g. @code
        const IID myIID = __uuidof (QTControl);
        myControlComp->createControl (&myIID);
        @endcode
    */
    bool createControl (const void* controlIID);

    /** Deletes the ActiveX control, if one has been created.
    */
    void deleteControl();

    /** Returns true if a control is currently in use. */
    bool isControlOpen() const noexcept                 { return control != nullptr; }

    /** Does a QueryInterface call on the embedded control object.

        This allows you to cast the control to whatever type of COM object you need.

        The iid parameter is a pointer to an IID structure - it's treated
        as a void* because when including the Juce headers, you might not always
        have included windows.h first, in which case IID wouldn't be defined, but
        you should just pass a pointer to an IID.

        e.g. @code
        const IID iid = __uuidof (IOleWindow);

        IOleWindow* oleWindow = (IOleWindow*) myControlComp->queryInterface (&iid);

        if (oleWindow != nullptr)
        {
            HWND hwnd;
            oleWindow->GetWindow (&hwnd);

            ...

            oleWindow->Release();
        }
        @endcode
    */
    void* queryInterface (const void* iid) const;

    /** Set this to false to stop mouse events being allowed through to the control.
    */
    void setMouseEventsAllowed (bool eventsCanReachControl);

    /** Returns true if mouse events are allowed to get through to the control.
    */
    bool areMouseEventsAllowed() const noexcept                 { return mouseEventsAllowed; }

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

private:
    class Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> control;
    bool mouseEventsAllowed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActiveXControlComponent)
};

#endif

#endif   // JUCE_ACTIVEXCONTROLCOMPONENT_H_INCLUDED
