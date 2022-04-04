/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/**
    An implementation of the OnlineUnlockStatus class which talks to the
    Tracktion Marketplace server.

    For details about how to use this class, see the docs for the base
    class: OnlineUnlockStatus. Basically, you need to inherit from it, and
    implement all the pure virtual methods to tell it about your product.

    @see OnlineUnlockStatus, OnlineUnlockForm, KeyGeneration

    @tags{ProductUnlocking}
*/
class JUCE_API  TracktionMarketplaceStatus   : public OnlineUnlockStatus
{
public:
    TracktionMarketplaceStatus();

    /** @internal */
    bool doesProductIDMatch (const String& returnedIDFromServer) override;
    /** @internal */
    URL getServerAuthenticationURL() override;
    /** @internal */
    String getWebsiteName() override;
    /** @internal */
    String readReplyFromWebserver (const String& email, const String& password) override;
    /** @internal */
    void userCancelled() override;

private:
    CriticalSection streamCreationLock;
    std::unique_ptr<WebInputStream> stream;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TracktionMarketplaceStatus)
};

} // namespace juce
