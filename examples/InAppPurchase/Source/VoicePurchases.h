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

//==============================================================================
class VoicePurchases      : private InAppPurchases::Listener
{
public:
    //==============================================================================
    struct VoiceProduct
    {
        const char* identifier;
        const char* humanReadable;
        bool isPurchased, priceIsKnown, purchaseInProgress;
        String purchasePrice;
    };

    //==============================================================================
    VoicePurchases (AsyncUpdater& asyncUpdater)
         : guiUpdater (asyncUpdater)
    {
        voiceProducts = Array<VoiceProduct>(
                         {VoiceProduct {"robot", "Robot",   true,   true,  false, "Free" },
                          VoiceProduct {"jules",  "Jules",  false,  false, false, "Retrieving price..." },
                          VoiceProduct {"fabian", "Fabian", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"ed",     "Ed",     false,  false, false, "Retrieving price..." },
                          VoiceProduct {"lukasz", "Lukasz", false,  false, false, "Retrieving price..." },
                          VoiceProduct {"jb",     "JB",     false,  false, false, "Retrieving price..." } });
    }

    ~VoicePurchases()
    {
        inAppPurchases.removeListener (this);
    }

    //==============================================================================
    VoiceProduct getPurchase (int voiceIndex)
    {
        if (! havePurchasesBeenRestored)
        {
            havePurchasesBeenRestored = true;
            inAppPurchases.addListener (this);

            inAppPurchases.restoreProductsBoughtList (true);
        }

        return voiceProducts[voiceIndex];
    }

    void purchaseVoice (int voiceIndex)
    {
        if (havePricesBeenFetched && isPositiveAndBelow (voiceIndex, voiceProducts.size()))
        {
            auto& product = voiceProducts.getReference (voiceIndex);

            if (! product.isPurchased)
            {
                product.purchaseInProgress = true;
                inAppPurchases.purchaseProduct (product.identifier, false);
            }
        }
    }

    StringArray getVoiceNames() const
    {
        StringArray names;

        for (auto& voiceProduct : voiceProducts)
            names.add (voiceProduct.humanReadable);

        return names;
    }

private:
    //==============================================================================
    void productsInfoReturned (const Array<InAppPurchases::Product>& products) override
    {
        if (! inAppPurchases.isInAppPurchasesSupported())
        {
            for (auto idx = 1; idx < voiceProducts.size(); ++idx)
            {
                auto& voiceProduct = voiceProducts.getReference (idx);

                voiceProduct.isPurchased  = false;
                voiceProduct.priceIsKnown = false;
                voiceProduct.purchasePrice = "In-App purcahses unavailable";
            }

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "In-app purchase is unavailable!",
                                              "In-App purchases are not available. This either means you are trying "
                                              "to use IAP on a platform that does not support IAP or you haven't setup "
                                              "your app correctly to work with IAP.",
                                              "OK");
        }
        else
        {
            for (auto product : products)
            {
                auto idx = findVoiceIndexFromIdentifier (product.identifier);

                if (isPositiveAndBelow (idx, voiceProducts.size()))
                {
                    auto& voiceProduct = voiceProducts.getReference (idx);

                    voiceProduct.priceIsKnown = true;
                    voiceProduct.purchasePrice = product.price;
                }
            }

            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Your credit card will be charged!",
                                              "You are running the sample code for JUCE In-App purchases. "
                                              "Although this is only sample code, it will still CHARGE YOUR CREDIT CARD!",
                                              "Understood!");
        }

        guiUpdater.triggerAsyncUpdate();
    }

    void productPurchaseFinished (const PurchaseInfo& info, bool success, const String&) override
    {
        auto idx = findVoiceIndexFromIdentifier (info.purchase.productId);

        if (isPositiveAndBelow (idx, voiceProducts.size()))
        {
            auto& voiceProduct = voiceProducts.getReference (idx);

            voiceProduct.isPurchased = success;
            voiceProduct.purchaseInProgress = false;
            guiUpdater.triggerAsyncUpdate();
        }
    }

    void purchasesListRestored (const Array<PurchaseInfo>& infos, bool success, const String&) override
    {
        if (success)
        {
            for (auto& info : infos)
            {
                auto idx = findVoiceIndexFromIdentifier (info.purchase.productId);

                if (isPositiveAndBelow (idx, voiceProducts.size()))
                {
                    auto& voiceProduct = voiceProducts.getReference (idx);

                    voiceProduct.isPurchased = true;
                }
            }

            guiUpdater.triggerAsyncUpdate();
        }

        if (! havePricesBeenFetched)
        {
            havePricesBeenFetched = true;
            StringArray identifiers;

            for (auto& voiceProduct : voiceProducts)
                identifiers.add (voiceProduct.identifier);

            inAppPurchases.getProductsInformation(identifiers);
        }
    }

    //==============================================================================
    int findVoiceIndexFromIdentifier (String identifier) const
    {
        identifier = identifier.toLowerCase();

        for (auto i = 0; i < voiceProducts.size(); ++i)
            if (String (voiceProducts.getReference (i).identifier) ==  identifier)
                return i;

        return -1;
    }

    //==============================================================================
    AsyncUpdater& guiUpdater;
    bool havePurchasesBeenRestored = false, havePricesBeenFetched = false;
    InAppPurchases inAppPurchases;
    Array<VoiceProduct> voiceProducts;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoicePurchases)
};
