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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

static LibdecorSymbols* getUsableLibdecorSymbols()
{
    auto* symbols = LibdecorSymbols::getInstanceWithoutCreating();

    if (symbols == nullptr || ! symbols->isUsable())
        return nullptr;

    return symbols;
}

// A libdecor error may indicate that its decoration plugin already destroyed these handles.
// Leave them allocated at shutdown rather than risk freeing them twice.
void LibdecorContextDeleter::operator() (libdecor* context) const
{
    if (context == nullptr)
        return;

    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorUnref (context);
}

void LibdecorFrameDeleter::operator() (libdecor_frame* frame) const
{
    if (frame == nullptr)
        return;

    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameUnref (frame);
}

void LibdecorStateDeleter::operator() (libdecor_state* state) const
{
    if (state == nullptr)
        return;

    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorStateFree (state);
}

} // namespace juce

namespace juce::LibdecorAPI
{

void markUnusableAfterError()
{
    if (auto* symbols = LibdecorSymbols::getInstanceWithoutCreating())
        symbols->markUnusableAfterError();
}

bool isUsable()
{
    return getUsableLibdecorSymbols() != nullptr;
}

LibdecorContextHandle createContext (wl_display* display, libdecor_interface* interface)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return LibdecorContextHandle { symbols->libdecorNew (display, interface) };

    return {};
}

int getFd (libdecor* context)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return symbols->libdecorGetFd (context);

    return -1;
}

int dispatch (libdecor* context, int timeoutMs)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return symbols->libdecorDispatch (context, timeoutMs);

    return 0;
}

LibdecorFrameHandle decorate (libdecor* context, wl_surface* surface,
                              libdecor_frame_interface* interface, void* userData)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return LibdecorFrameHandle { symbols->libdecorDecorate (context, surface, interface, userData) };

    return {};
}

void frameSetTitle (libdecor_frame* frame, StringRef title)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetTitle (frame, title);
}

void frameSetAppId (libdecor_frame* frame, StringRef appId)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetAppId (frame, appId);
}

void frameSetMinContentSize (libdecor_frame* frame, Point<int> size)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetMinContentSize (frame, size.x, size.y);
}

void frameSetMaxContentSize (libdecor_frame* frame, Point<int> size)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetMaxContentSize (frame, size.x, size.y);
}

void frameSetMinimized (libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetMinimized (frame);
}

void frameSetFullscreen (libdecor_frame* frame, wl_output* output)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameSetFullscreen (frame, output);
}

void frameUnsetFullscreen (libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameUnsetFullscreen (frame);
}

void frameMove (libdecor_frame* frame, wl_seat* seat, uint32_t serial)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameMove (frame, seat, serial);
}

void frameResize (libdecor_frame* frame, wl_seat* seat, uint32_t serial, libdecor_resize_edge edge)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameResize (frame, seat, serial, edge);
}

void frameMap (libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameMap (frame);
}

void frameCommit (libdecor_frame* frame, libdecor_state* state, libdecor_configuration* configuration)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFrameCommit (frame, state, configuration);
}

xdg_surface* frameGetXdgSurface (libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return symbols->libdecorFrameGetXdgSurface (frame);

    return nullptr;
}

xdg_toplevel* frameGetXdgToplevel (libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return symbols->libdecorFrameGetXdgToplevel (frame);

    return nullptr;
}

void framePopupGrab (libdecor_frame* frame, StringRef seatName)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFramePopupGrab (frame, seatName);
}

void framePopupUngrab (libdecor_frame* frame, StringRef seatName)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        symbols->libdecorFramePopupUngrab (frame, seatName);
}

Point<int> frameTranslateCoordinate (libdecor_frame* frame, Point<int> contentPoint)
{
    if (auto* symbols = getUsableLibdecorSymbols())
    {
        Point<int> result;
        symbols->libdecorFrameTranslateCoordinate (frame,
                                                   contentPoint.x,
                                                   contentPoint.y,
                                                   &result.x,
                                                   &result.y);
        return result;
    }

    return contentPoint;
}

LibdecorStateHandle createState (Point<int> contentSize)
{
    if (auto* symbols = getUsableLibdecorSymbols())
        return LibdecorStateHandle { symbols->libdecorStateNew (contentSize.x, contentSize.y) };

    return {};
}

std::optional<Point<int>> getConfigurationContentSize (libdecor_configuration* configuration,
                                                       libdecor_frame* frame)
{
    if (auto* symbols = getUsableLibdecorSymbols())
    {
        int width = 0;
        int height = 0;

        if (symbols->libdecorConfigurationGetContentSize (configuration, frame, &width, &height)
            && width > 0 && height > 0)
        {
            return Point { width, height };
        }
    }

    return {};
}

std::optional<libdecor_window_state> getConfigurationWindowState (libdecor_configuration* configuration)
{
    if (auto* symbols = getUsableLibdecorSymbols())
    {
        auto state = libdecorWindowStateNone;

        if (symbols->libdecorConfigurationGetWindowState (configuration, &state))
            return state;
    }

    return {};
}

} // namespace juce::LibdecorAPI
