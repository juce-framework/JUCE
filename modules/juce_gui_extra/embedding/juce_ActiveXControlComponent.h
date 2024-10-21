/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_WINDOWS || DOXYGEN

//==============================================================================
/**
    A Windows-specific class that can create and embed an ActiveX control inside
    itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use createControl() to instantiate an ActiveX control. The control
    will then be moved and resized to follow the movements of this component.

    Of course, since the control is a heavyweight window, it'll obliterate any
    JUCE components that may overlap this component, but that's life.

    @tags{GUI}
*/
class JUCE_API  ActiveXControlComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    ActiveXControlComponent();

    /** Destructor. */
    ~ActiveXControlComponent() override;

    /** Tries to create an ActiveX control and embed it in this peer.

        The peer controlIID is a pointer to an IID structure - it's treated
        as a void* because when including the JUCE headers, you might not always
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
        as a void* because when including the JUCE headers, you might not always
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
    /** Set an instance of IDispatch where dispatch events should be delivered to
    */
    void setEventHandler (void* eventHandler);

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

    /** @internal */
    intptr_t offerEventToActiveXControl (void*);
    static intptr_t offerEventToActiveXControlStatic (void*);

private:
    class Pimpl;
    std::unique_ptr<Pimpl> control;
    bool mouseEventsAllowed = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActiveXControlComponent)
};

#endif

} // namespace juce
