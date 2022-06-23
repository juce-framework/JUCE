/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Application.h"

//==============================================================================
class UserAvatarComponent  : public Component,
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

        if (! isGPL)
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

    bool isDisplaingGPLLogo() const noexcept  { return isGPL; }

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
    static Image createGPLAvatarImage()
    {
        if (auto logo = Drawable::createFromImageData (BinaryData::gpl_logo_svg, BinaryData::gpl_logo_svgSize))
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

        isGPL = ProjucerApplication::getApp().getLicenseController().getCurrentState().isGPL();

        if (interactive)
        {
            auto formattedUserString = [state]() -> String
            {
                if (state.isSignedIn())
                    return (state.isGPL() ? "" : (state.username + " - ")) + state.getLicenseTypeString();

                return "Not logged in";
            }();

            setTooltip (formattedUserString);
        }

        currentAvatar = isGPL ? gplAvatarImage
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
    Image standardAvatarImage, signedOutAvatarImage, gplAvatarImage { createGPLAvatarImage() }, currentAvatar;
    bool isGPL = false, interactive = false;
};
