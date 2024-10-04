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

/** @internal */
bool juce_handleXEmbedEvent (ComponentPeer*, void*);
/** @internal */
unsigned long juce_getCurrentFocusWindow (ComponentPeer*);

#if JUCE_LINUX || JUCE_BSD || DOXYGEN

//==============================================================================
/**
    A Linux-specific class that can embed a foreign X11 widget.

    Use this class to embed a foreign X11 widget from other toolkits such as
    GTK+ or QT.

    There are two ways to initiate the Xembed protocol. Either the client creates
    a window and passes this to the host (client initiated) or the host
    creates a window in which the client can reparent it's client widget
    (host initiated). XEmbedComponent supports both protocol types.

    This is how you embed a GTK+ widget: if you are using the client
    initiated version of the protocol, then create a new gtk widget with
    gtk_plug_new (0). Then query the window id of the plug via gtk_plug_get_id().
    Pass this id to the constructor of this class.

    If you are using the host initiated version of the protocol, then first create
    the XEmbedComponent using the default constructor. Use getHostWindowID to get
    the window id of the host, use this to construct your gtk plug via gtk_plug_new.

    A similar approach can be used to embed QT widgets via QT's QX11EmbedWidget
    class.

    Other toolkits or raw X11 widgets should follow the X11 embed protocol:
    https://specifications.freedesktop.org/xembed-spec/xembed-spec-latest.html

    @tags{GUI}
*/
class XEmbedComponent : public Component
{
public:
    //==============================================================================

    /** Creates a JUCE component wrapping a foreign widget

        Use this constructor if you are using the host initiated version
        of the XEmbedProtocol. When using this version of the protocol
        you must call getHostWindowID() and pass this id to the foreign toolkit.
    */
    XEmbedComponent (bool wantsKeyboardFocus = true,
                     bool allowForeignWidgetToResizeComponent = false);

    /** Create a JUCE component wrapping the foreign widget with id wID

        Use this constructor if you are using the client initiated version
        of the XEmbedProtocol.
    */
    XEmbedComponent (unsigned long wID, bool wantsKeyboardFocus = true,
                     bool allowForeignWidgetToResizeComponent = false);


    /** Destructor. */
    ~XEmbedComponent() override;

    /** Use this method to retrieve the host's window id when using the
        host initiated version of the XEmbedProtocol
    */
    unsigned long getHostWindowID();

    /** Removes the client window from the host. */
    void removeClient();

    /** Forces the embedded window to match the current size of this component. */
    void updateEmbeddedBounds();

protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    void focusGainedWithDirection (FocusChangeType, FocusChangeDirection) override;
    void focusLost (FocusChangeType) override;
    void broughtToFront() override;

private:
    friend bool juce::juce_handleXEmbedEvent (ComponentPeer*, void*);
    friend unsigned long juce_getCurrentFocusWindow (ComponentPeer*);

    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;
};

#endif

} // namespace juce
