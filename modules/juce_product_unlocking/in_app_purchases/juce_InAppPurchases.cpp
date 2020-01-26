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

namespace juce
{

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (InAppPurchases)

InAppPurchases::InAppPurchases()
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    : pimpl (new Pimpl (*this))
   #endif
{}

InAppPurchases::~InAppPurchases() { clearSingletonInstance(); }

bool InAppPurchases::isInAppPurchasesSupported() const
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    return pimpl->isInAppPurchasesSupported();
   #else
    return false;
   #endif
}

void InAppPurchases::getProductsInformation (const StringArray& productIdentifiers)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->getProductsInformation (productIdentifiers);
   #else
    Array<Product> products;
    for (auto productId : productIdentifiers)
        products.add (Product { productId, {}, {}, {}, {}  });

    listeners.call ([&] (Listener& l) { l.productsInfoReturned (products); });
   #endif
}

void InAppPurchases::purchaseProduct (const String& productIdentifier,
                                      const String& upgradeProductIdentifier,
                                      bool creditForUnusedSubscription)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->purchaseProduct (productIdentifier, upgradeProductIdentifier, creditForUnusedSubscription);
   #else
    Listener::PurchaseInfo purchaseInfo { Purchase { "", productIdentifier, {}, {}, {} }, {} };

    listeners.call ([&] (Listener& l) { l.productPurchaseFinished (purchaseInfo, false, "In-app purchases unavailable"); });
    ignoreUnused (isSubscription, upgradeProductIdentifiers, creditForUnusedSubscription);
   #endif
}

void InAppPurchases::restoreProductsBoughtList (bool includeDownloadInfo, const String& subscriptionsSharedSecret)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->restoreProductsBoughtList (includeDownloadInfo, subscriptionsSharedSecret);
   #else
    listeners.call ([] (Listener& l) { l.purchasesListRestored ({}, false, "In-app purchases unavailable"); });
    ignoreUnused (includeDownloadInfo, subscriptionsSharedSecret);
   #endif
}

void InAppPurchases::consumePurchase (const String& productIdentifier, const String& purchaseToken)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->consumePurchase (productIdentifier, purchaseToken);
   #else
    listeners.call ([&] (Listener& l) { l.productConsumed (productIdentifier, false, "In-app purchases unavailable"); });
    ignoreUnused (purchaseToken);
   #endif
}

void InAppPurchases::addListener (Listener* l)      { listeners.add (l); }
void InAppPurchases::removeListener (Listener* l)   { listeners.remove (l); }

void InAppPurchases::startDownloads  (const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->startDownloads (downloads);
   #else
    ignoreUnused (downloads);
   #endif
}

void InAppPurchases::pauseDownloads  (const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->pauseDownloads (downloads);
   #else
    ignoreUnused (downloads);
   #endif
}

void InAppPurchases::resumeDownloads (const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->resumeDownloads (downloads);
   #else
    ignoreUnused (downloads);
   #endif
}

void InAppPurchases::cancelDownloads (const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->cancelDownloads (downloads);
   #else
    ignoreUnused (downloads);
   #endif
}

} // namespace juce
