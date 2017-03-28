/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

#pragma once

/** @internal */
bool juce_handleXEmbedEvent (ComponentPeer*, void*);
/** @internal */
unsigned long juce_getCurrentFocusWindow (ComponentPeer*);

#if JUCE_LINUX || DOXYGEN

//==============================================================================
/**
    A Linux-specific class that can embed a foreign X11 widget.

    Use this class to embed a foreign X11 widget from other toolkits such as
    GTK+ or QT.

    For GTK+, create a gtk_plug container and pass the plug's id
    (gtk_plug_get_id) to the constructor of this class.

    For QT, use the QX11EmbedWidget class and pass the widget's
    id (containerWinId()) to the constructor of this class.

    Other toolkits or raw X11 widgets should follow the X11 embed protocol:
    https://specifications.freedesktop.org/xembed-spec/xembed-spec-latest.html
*/
class XEmbedComponent : public Component
{
public:
    //==============================================================================

    /** Create a JUCE component wrapping the foreign widget with id wID */
    XEmbedComponent (unsigned long wID, bool wantsKeyboardFocus = true);

    /** Destructor. */
    ~XEmbedComponent();

protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;
    void broughtToFront() override;

private:
    friend bool juce::juce_handleXEmbedEvent (ComponentPeer*, void*);
    friend unsigned long juce_getCurrentFocusWindow (ComponentPeer*);

    class Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;
};

#endif
