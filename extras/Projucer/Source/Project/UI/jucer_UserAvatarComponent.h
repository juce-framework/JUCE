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

#pragma once

#include "../../Application/jucer_Application.h"

//==============================================================================
class UserAvatarComponent final : public Component,
                                  public SettableTooltipClient,
                                  public ChangeBroadcaster,
                                  private LicenseController::LicenseStateListener
{
public:
    UserAvatarComponent (bool isInteractive)
        : interactive (isInteractive)
    {
        ProjucerApplication::getApp().getLicenseController().addListener (this);
        lookAndFeelChanged();
    }

    ~UserAvatarComponent() override
    {
        ProjucerApplication::getApp().getLicenseController().removeListener (this);
    }

    void paint (Graphics& g) override
    {
        auto bounds = getLocalBounds();

        if (! isAGPL)
        {
            bounds = bounds.removeFromRight (bounds.getHeight());

            Path ellipse;
            ellipse.addEllipse (bounds.toFloat());

            g.reduceClipRegion (ellipse);
        }

        g.drawImage (currentAvatar, bounds.toFloat(), RectanglePlacement::fillDestination);
    }

    void mouseUp (const MouseEvent&) override
    {
        triggerClick();
    }

    void triggerClick()
    {
        if (interactive)
        {
            PopupMenu menu;
            menu.addCommandItem (ProjucerApplication::getApp().commandManager.get(), CommandIDs::loginLogout);

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (this));
        }
    }

    bool isDisplaingAGPLLogo() const noexcept  { return isAGPL; }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return interactive ? std::make_unique<AccessibilityHandler> (*this,
                                                                     AccessibilityRole::button,
                                                                     AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                                       [this] { triggerClick(); }))
                           : nullptr;
    }

private:
    //==============================================================================
    static Image createAGPLAvatarImage()
    {
        if (auto logo = Drawable::createFromImageData (BinaryData::agplv3_logo_svg, BinaryData::agplv3_logo_svgSize))
        {
            auto bounds = logo->getDrawableBounds();

            Image image (Image::ARGB, roundToInt (bounds.getWidth()), roundToInt (bounds.getHeight()), true);
            Graphics g (image);
            logo->draw (g, 1.0f);

            return image;
        }

        jassertfalse;
        return {};
    }

    Image createStandardAvatarImage()
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

    //==============================================================================
    void licenseStateChanged() override
    {
        auto state = ProjucerApplication::getApp().getLicenseController().getCurrentState();
        isAGPL = state.isAGPL();

        if (interactive)
        {
            auto formattedUserString = [state]() -> String
            {
                if (state.isSignedIn())
                    return (state.isAGPL() ? "" : (state.username + " - ")) + state.getLicenseTypeString();

                return "Not logged in";
            }();

            setTooltip (formattedUserString);
        }

        currentAvatar = isAGPL ? agplAvatarImage
                               : state.isSignedIn() ? standardAvatarImage : signedOutAvatarImage;

        repaint();
        sendChangeMessage();
    }

    void lookAndFeelChanged() override
    {
        standardAvatarImage = createStandardAvatarImage();
        signedOutAvatarImage = createStandardAvatarImage();

        if (interactive)
            signedOutAvatarImage.multiplyAllAlphas (0.4f);

        licenseStateChanged();
        repaint();
    }

    //==============================================================================
    Image standardAvatarImage, signedOutAvatarImage, agplAvatarImage { createAGPLAvatarImage() }, currentAvatar;
    bool isAGPL = false, interactive = false;
};
