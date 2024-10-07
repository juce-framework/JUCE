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

namespace juce
{

//==============================================================================
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
                                      [[maybe_unused]] const String& upgradeProductIdentifier,
                                      [[maybe_unused]] bool creditForUnusedSubscription)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->purchaseProduct (productIdentifier, upgradeProductIdentifier, creditForUnusedSubscription);
   #else
    Listener::PurchaseInfo purchaseInfo { Purchase { "", productIdentifier, {}, {}, {} }, {} };

    listeners.call ([&] (Listener& l) { l.productPurchaseFinished (purchaseInfo, false, "In-app purchases unavailable"); });
   #endif
}

void InAppPurchases::restoreProductsBoughtList ([[maybe_unused]] bool includeDownloadInfo, [[maybe_unused]] const String& subscriptionsSharedSecret)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->restoreProductsBoughtList (includeDownloadInfo, subscriptionsSharedSecret);
   #else
    listeners.call ([] (Listener& l) { l.purchasesListRestored ({}, false, "In-app purchases unavailable"); });
   #endif
}

void InAppPurchases::consumePurchase (const String& productIdentifier, [[maybe_unused]] const String& purchaseToken)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->consumePurchase (productIdentifier, purchaseToken);
   #else
    listeners.call ([&] (Listener& l) { l.productConsumed (productIdentifier, false, "In-app purchases unavailable"); });
   #endif
}

void InAppPurchases::addListener (Listener* l)      { listeners.add (l); }
void InAppPurchases::removeListener (Listener* l)   { listeners.remove (l); }

void InAppPurchases::startDownloads  ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->startDownloads (downloads);
   #endif
}

void InAppPurchases::pauseDownloads  ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->pauseDownloads (downloads);
   #endif
}

void InAppPurchases::resumeDownloads ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->resumeDownloads (downloads);
   #endif
}

void InAppPurchases::cancelDownloads ([[maybe_unused]] const Array<Download*>& downloads)
{
   #if JUCE_ANDROID || JUCE_IOS || JUCE_MAC
    pimpl->cancelDownloads (downloads);
   #endif
}

} // namespace juce
