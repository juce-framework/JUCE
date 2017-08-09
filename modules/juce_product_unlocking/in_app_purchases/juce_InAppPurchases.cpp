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


InAppPurchases::InAppPurchases() : pimpl (new Pimpl (*this)) {}
InAppPurchases::~InAppPurchases() {}

bool InAppPurchases::isInAppPurchasesSupported() const
{
    return pimpl->isInAppPurchasesSupported();
}

void InAppPurchases::getProductsInformation (const StringArray& productIdentifiers)
{
    pimpl->getProductsInformation (productIdentifiers);
}

void InAppPurchases::purchaseProduct (const String& productIdentifier,
                                      bool isSubscription,
                                      const StringArray& upgradeProductIdentifiers,
                                      bool creditForUnusedSubscription)
{
    pimpl->purchaseProduct (productIdentifier, isSubscription,
                            upgradeProductIdentifiers, creditForUnusedSubscription);
}

void InAppPurchases::restoreProductsBoughtList (bool includeDownloadInfo, const String& subscriptionsSharedSecret)
{
    pimpl->restoreProductsBoughtList (includeDownloadInfo, subscriptionsSharedSecret);
}

void InAppPurchases::consumePurchase (const String& productIdentifier, const String& purchaseToken)
{
    pimpl->consumePurchase (productIdentifier, purchaseToken);
}

void InAppPurchases::addListener (Listener* l)      { listeners.add (l); }
void InAppPurchases::removeListener (Listener* l)   { listeners.remove (l); }

void InAppPurchases::startDownloads  (const Array<Download*>& downloads)    { pimpl->startDownloads (downloads); }
void InAppPurchases::pauseDownloads  (const Array<Download*>& downloads)    { pimpl->pauseDownloads (downloads); }
void InAppPurchases::resumeDownloads (const Array<Download*>& downloads)    { pimpl->resumeDownloads (downloads); }
void InAppPurchases::cancelDownloads (const Array<Download*>& downloads)    { pimpl->cancelDownloads (downloads); }
