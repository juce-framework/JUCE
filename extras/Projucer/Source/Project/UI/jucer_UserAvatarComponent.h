/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Application.h"

class UserAvatarComponent  : public Component,
                             public SettableTooltipClient,
                             public ChangeBroadcaster,
                             private LicenseController::LicenseStateListener
{
public:
    UserAvatarComponent (bool tooltip, bool signIn)
        : displayTooltip (tooltip),
          signInOnClick (signIn)
    {
        ProjucerApplication::getApp().getLicenseController().addListener (this);
        licenseStateChanged();
    }

    ~UserAvatarComponent() override
    {
        ProjucerApplication::getApp().getLicenseController().removeListener (this);
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();

        if (! isGPL)
        {
            bounds = bounds.removeFromRight (bounds.getHeight());

            Path ellipse;
            ellipse.addEllipse (bounds.toFloat());

            g.reduceClipRegion (ellipse);
        }

        g.drawImage (userAvatarImage, bounds.toFloat(), RectanglePlacement::fillDestination);
    }

    void mouseUp (const MouseEvent&) override
    {
        if (signInOnClick)
        {
            PopupMenu menu;
            menu.addCommandItem (ProjucerApplication::getApp().commandManager.get(), CommandIDs::loginLogout);

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (this));
        }
    }

    bool isDisplaingGPLLogo() const noexcept  { return isGPL; }

private:
    Image createDefaultAvatarImage()
    {
        Image image (Image::ARGB, 250, 250, true);
        Graphics g (image);

        g.setColour (findColour (defaultButtonBackgroundColourId));
        g.fillAll();

        g.setColour (findColour (defaultIconColourId));

        auto path = getIcons().user;
        g.fillPath (path, RectanglePlacement (RectanglePlacement::centred)
                            .getTransformToFit (path.getBounds(), image.getBounds().reduced (image.getHeight() / 5).toFloat()));

        return image;
    }

    void licenseStateChanged() override
    {
        auto state = ProjucerApplication::getApp().getLicenseController().getCurrentState();

        isGPL = ProjucerApplication::getApp().getLicenseController().getCurrentState().isGPL();

        if (displayTooltip)
        {
            auto formattedUserString = [state]() -> String
            {
                if (state.isValid())
                    return (state.isGPL() ? "" : (state.username + " - ")) + state.getLicenseTypeString();

                return "Not logged in";
            }();

            setTooltip (formattedUserString);
        }

        userAvatarImage = state.isValid() ? state.avatar : defaultAvatarImage;
        repaint();
        sendChangeMessage();
    }

    void lookAndFeelChanged() override
    {
        defaultAvatarImage = createDefaultAvatarImage();
        licenseStateChanged();
        repaint();
    }

    Image userAvatarImage, defaultAvatarImage { createDefaultAvatarImage() };
    bool isGPL = false, displayTooltip = false, signInOnClick = false;
};
