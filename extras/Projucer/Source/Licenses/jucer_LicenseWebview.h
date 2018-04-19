/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class LicenseWebview : public DialogWindow
{
public:
    LicenseWebview (ModalComponentManager::Callback* callbackToUse, const String& request)
        : DialogWindow ("Log-in to Projucer", Colour (0xfff1f1f1), true, true)
    {
        LicenseWebviewContent* content;

        setUsingNativeTitleBar (true);
        setContentOwned (content = new LicenseWebviewContent (*this, callbackToUse), true);

        centreWithSize (getWidth(), getHeight());

        content->goToURL (request);
    }

    void goToURL (const String& request) { reinterpret_cast<LicenseWebviewContent*> (getContentComponent())->goToURL (request); }

    void setPageCallback (const std::function<void (const String&, const HashMap<String, String>&)>& cb)
    {
        reinterpret_cast<LicenseWebviewContent*> (getContentComponent())->pageCallback = cb;
    }

    void setNewWindowCallback (const std::function<void (const String&)>& cb)
    {
        reinterpret_cast<LicenseWebviewContent*> (getContentComponent())->newWindowCallback = cb;
    }

    void closeButtonPressed() override    { exitModalState (-1); }

private:
    class LicenseWebviewContent  : public Component
    {
        //==============================================================================
        struct RedirectWebBrowserComponent : public WebBrowserComponent
        {
            RedirectWebBrowserComponent (LicenseWebviewContent& controller) : WebBrowserComponent (false), owner (controller) {}
            virtual ~RedirectWebBrowserComponent() {}

            bool pageAboutToLoad           (const String& url) override { return owner.pageAboutToLoad (url); }
            void pageFinishedLoading       (const String& url) override { owner.pageFinishedLoading (url); }
            void newWindowAttemptingToLoad (const String& url) override { owner.newWindowAttemptingToLoad (url); }
            bool pageLoadHadNetworkError   (const String& err) override { return owner.pageLoadHadNetworkError (err); }

            LicenseWebviewContent& owner;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RedirectWebBrowserComponent)
        };

        //==============================================================================
        struct Header  : public Component,
                         private LicenseController::StateChangedCallback
        {
            Header()  : avatarButton ("User Settings", &getIcons().user)
            {
                setOpaque (true);
                addChildComponent (avatarButton);

                avatarButton.onClick = [this] { showAvatarWindow(); };

                if (auto* licenseController = ProjucerApplication::getApp().licenseController.get())
                {
                    licenseController->addLicenseStatusChangedCallback (this);
                    licenseStateChanged (licenseController->getState());
                }
            }

            virtual ~Header()
            {
                if (auto* licenseController = ProjucerApplication::getApp().licenseController.get())
                    licenseController->removeLicenseStatusChangedCallback (this);
            }

            void resized() override
            {
                auto r = getLocalBounds().reduced (30, 20);
                avatarButton.setBounds (r.removeFromRight (r.getHeight()));
            }

            void paint (Graphics& g) override
            {
                auto r = getLocalBounds().reduced (30, 20);
                g.fillAll (Colour (backgroundColour));

                if (juceLogo != nullptr)
                    juceLogo->drawWithin (g, r.toFloat(), RectanglePlacement::xLeft + RectanglePlacement::yMid, 1.0);
            }

            void licenseStateChanged (const LicenseState& state) override
            {
                avatarButton.iconImage = state.avatar;
                avatarButton.setVisible (state.type != LicenseState::Type::notLoggedIn && state.type != LicenseState::Type::GPL);
                avatarButton.repaint();
            }

            void showAvatarWindow()
            {
                if (auto* licenseController = ProjucerApplication::getApp().licenseController.get())
                {
                    auto type = licenseController->getState().type;

                    auto* content = new UserSettingsPopup (true);
                    content->setSize (200, (type == LicenseState::Type::noLicenseChosenYet ? 100 : 150));

                    CallOutBox::launchAsynchronously (content, avatarButton.getScreenBounds(), nullptr);
                }
            }

            const uint32 backgroundColour = 0xff414141;
            std::unique_ptr<Drawable> juceLogo { Drawable::createFromImageData (BinaryData::jucelogowithtext_svg,
                                                                                BinaryData::jucelogowithtext_svgSize) };
            IconButton avatarButton;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Header)
        };

        //==============================================================================
    public:
        LicenseWebviewContent (LicenseWebview& parentWindowToUse, ModalComponentManager::Callback* callbackToUse)
            : parentWindow (parentWindowToUse), modalCallback (callbackToUse), webview (*this)
        {
            addAndMakeVisible (header);
            addAndMakeVisible (webview);

            setOpaque (true);
            setSize (978, 718);

           #if JUCE_WINDOWS // windows needs the webcomponent be visible
            parentWindow.enterModalState (true, modalCallback.release(), true);
           #endif
        }

        void goToURL (const String& request)
        {
            lastURL = request;
            webview.goToURL (lastURL);
        }

        void paint (Graphics& g) override     { g.fillAll (Colours::lightblue); }

        void resized() override
        {
            auto r = getLocalBounds();
            header.setBounds (r.removeFromTop (78));
            webview.setBounds (r);
        }

        bool pageAboutToLoad (const String& page)
        {
            URL url (page);

            if (page == "about:blank" || page.startsWith ("file://") || page.startsWith ("data:text/html"))
            {
                if (page != lastErrorPageURI)
                    lastURL = page;

                return true;
            }
            else if (url.getScheme() == "projucer")
            {
                HashMap<String, String> params;

                auto n = url.getParameterNames().size();

                for (int i = 0; i < n; ++i)
                    params.set (url.getParameterNames()[i], url.getParameterValues()[i]);

                String cmd (url.getDomain());

                if (n == 0 && cmd.containsChar (L'='))
                {
                    // old-style callback
                    StringArray domainTokens (StringArray::fromTokens (cmd, "=", ""));
                    cmd = domainTokens[0];

                    params.set (cmd, domainTokens[1]);
                }

                if (pageCallback)
                    pageCallback (cmd, params);

                return false;
            }

            bool isValid = (url.getDomain().endsWith ("roli.com") || url.getDomain().endsWith ("juce.com"));

            if (isValid)
                lastURL = page;

            return true;
        }

        void pageFinishedLoading (const String& page)
        {
            URL url (page);

            if ((isValidURL (url)
                 || page.startsWith ("file://") || page.startsWith ("data:text/html"))
                 && ! parentWindow.isCurrentlyModal())
                parentWindow.enterModalState (true, modalCallback.release(), true);
        }

        void newWindowAttemptingToLoad (const String& page)
        {
            URL url (page);
            bool isGitHub = url.getDomain().endsWith ("github.com");

            if (url.getDomain().endsWith ("roli.com")
             || url.getDomain().endsWith ("juce.com")
             || isGitHub)
            {
                url.launchInDefaultBrowser();

                if (newWindowCallback && ! isGitHub)
                    newWindowCallback (page);
            }
        }

        bool pageLoadHadNetworkError (const String&)
        {
            String errorPageSource = String (BinaryData::offlinepage_html, BinaryData::offlinepage_htmlSize)
                                         .replace ("__URL_PLACEHOLDER__", lastURL);

           #if JUCE_WINDOWS
            auto tmpFile = File::createTempFile (".html");
            tmpFile.replaceWithText (errorPageSource, true);

            lastErrorPageURI = "file://" + tmpFile.getFullPathName();
           #else
            lastErrorPageURI = "data:text/html;base64," + Base64::toBase64 (errorPageSource);
           #endif

            goToURL (lastErrorPageURI);

            return false;
        }

        static bool isValidURL (const URL& url)     { return (url.getDomain().endsWith ("roli.com") || url.getDomain().endsWith ("juce.com")); }

        //==============================================================================
        LicenseWebview& parentWindow;
        std::unique_ptr<ModalComponentManager::Callback> modalCallback;
        Header header;
        RedirectWebBrowserComponent webview;
        std::function<void (const String&, const HashMap<String, String>&)> pageCallback;
        std::function<void (const String&)> newWindowCallback;
        String lastURL, lastErrorPageURI;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseWebviewContent)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseWebview)
};
