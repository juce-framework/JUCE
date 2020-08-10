/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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
