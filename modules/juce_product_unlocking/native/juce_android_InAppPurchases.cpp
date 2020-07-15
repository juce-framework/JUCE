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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getSku,               "getSku",               "()Ljava/lang/String;") \
  METHOD (getTitle,             "getTitle",             "()Ljava/lang/String;") \
  METHOD (getDescription,       "getDescription",       "()Ljava/lang/String;") \
  METHOD (getPrice,             "getPrice",             "()Ljava/lang/String;") \
  METHOD (getPriceCurrencyCode, "getPriceCurrencyCode", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (SkuDetails, "com/android/billingclient/api/SkuDetails")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  STATICMETHOD (newBuilder, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParams, "com/android/billingclient/api/BillingFlowParams")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (build,                       "build",                       "()Lcom/android/billingclient/api/BillingFlowParams;")                                             \
  METHOD (setOldSku,                   "setOldSku",                   "(Ljava/lang/String;Ljava/lang/String;)Lcom/android/billingclient/api/BillingFlowParams$Builder;") \
  METHOD (setReplaceSkusProrationMode, "setReplaceSkusProrationMode", "(I)Lcom/android/billingclient/api/BillingFlowParams$Builder;")                                    \
  METHOD (setSkuDetails,               "setSkuDetails",               "(Lcom/android/billingclient/api/SkuDetails;)Lcom/android/billingclient/api/BillingFlowParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsBuilder, "com/android/billingclient/api/BillingFlowParams$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getOrderId,       "getOrderId",       "()Ljava/lang/String;") \
  METHOD (getSku,           "getSku",           "()Ljava/lang/String;") \
  METHOD (getPackageName,   "getPackageName",   "()Ljava/lang/String;") \
  METHOD (getPurchaseTime,  "getPurchaseTime",  "()J")                  \
  METHOD (getPurchaseToken, "getPurchaseToken", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (AndroidPurchase, "com/android/billingclient/api/Purchase")
#undef JNI_CLASS_MEMBERS

//==============================================================================
struct InAppPurchases::Pimpl
{
    Pimpl (InAppPurchases& parent)
        : owner (parent),
          billingClient (LocalRef<jobject> (getEnv()->NewObject (JuceBillingClient,
                                                                 JuceBillingClient.constructor,
                                                                 getAppContext().get(),
                                                                 (jlong) this)))
    {
    }

    ~Pimpl()
    {
        getEnv()->CallVoidMethod (billingClient, JuceBillingClient.endConnection);
    }

    //==============================================================================
    bool isInAppPurchasesSupported() const
    {
        return isReady() && getEnv()->CallBooleanMethod (billingClient, JuceBillingClient.isBillingSupported);
    }

    void getProductsInformation (const StringArray& productIdentifiers)
    {
        skuDetailsQueryCallbackQueue.emplace ([this] (LocalRef<jobject> skuDetailsList)
        {
            if (skuDetailsList != nullptr)
            {
                auto* env = getEnv();
                Array<InAppPurchases::Product> products;

                for (int i = 0; i < env->CallIntMethod (skuDetailsList, JavaList.size); ++i)
                    products.add (buildProduct (LocalRef<jobject> (env->CallObjectMethod (skuDetailsList, JavaList.get, i))));

                owner.listeners.call ([&] (Listener& l) { l.productsInfoReturned (products); });
            }
        });

        querySkuDetailsAsync (convertToLowerCase (productIdentifiers));
    }

    void purchaseProduct (const String& productIdentifier,
                          const String& subscriptionIdentifier,
                          bool creditForUnusedSubscription)
    {
        skuDetailsQueryCallbackQueue.emplace ([=] (LocalRef<jobject> skuDetailsList)
        {
            if (skuDetailsList != nullptr)
            {
                auto* env = getEnv();

                if (env->CallIntMethod (skuDetailsList, JavaList.size) > 0)
                {
                    LocalRef<jobject> skuDetails (env->CallObjectMethod (skuDetailsList, JavaList.get, 0));

                    if (subscriptionIdentifier.isNotEmpty())
                        changeExistingSubscription (skuDetails, subscriptionIdentifier, creditForUnusedSubscription);
                    else
                        purchaseProductWithSkuDetails (skuDetails);
                }
            }
        });

        querySkuDetailsAsync (convertToLowerCase ({ productIdentifier }));
    }

    void restoreProductsBoughtList (bool, const juce::String&)
    {
        purchasesListQueryCallbackQueue.emplace ([this] (LocalRef<jobject> purchasesList)
        {
            if (purchasesList != nullptr)
            {
                auto* env = getEnv();
                Array<InAppPurchases::Listener::PurchaseInfo> purchases;

                for (int i = 0; i < env->CallIntMethod (purchasesList, JavaArrayList.size); ++i)
                {
                    LocalRef<jobject> purchase (env->CallObjectMethod (purchasesList, JavaArrayList.get, i));
                    purchases.add ({ buildPurchase (purchase), {} });
                }

                owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored (purchases, true, NEEDS_TRANS ("Success")); });
            }
            else
            {
                owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Failure")); });
            }
        });

        getProductsBoughtAsync();
    }

    void consumePurchase (const String& productIdentifier, const String& purchaseToken)
    {
        if (purchaseToken.isEmpty())
        {
            skuDetailsQueryCallbackQueue.emplace ([=] (LocalRef<jobject> skuDetailsList)
            {
                if (skuDetailsList != nullptr)
                {
                    auto* env = getEnv();

                    if (env->CallIntMethod (skuDetailsList, JavaList.size) > 0)
                    {
                        LocalRef<jobject> sku (env->CallObjectMethod (skuDetailsList, JavaList.get, 0));

                        auto token = juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (sku, AndroidPurchase.getSku)));

                        if (token.isNotEmpty())
                        {
                            consumePurchaseWithToken (productIdentifier, token);
                            return;
                        }
                    }
                }

                notifyListenersAboutConsume (productIdentifier, false, NEEDS_TRANS ("Item unavailable"));
            });

            querySkuDetailsAsync (convertToLowerCase ({ productIdentifier }));
        }

        consumePurchaseWithToken (productIdentifier, purchaseToken);
    }

    //==============================================================================
    void startDownloads (const Array<Download*>& downloads)
    {
        // Not available on this platform.
        ignoreUnused (downloads);
        jassertfalse;
    }

    void pauseDownloads (const Array<Download*>& downloads)
    {
        // Not available on this platform.
        ignoreUnused (downloads);
        jassertfalse;
    }

    void resumeDownloads (const Array<Download*>& downloads)
    {
        // Not available on this platform.
        ignoreUnused (downloads);
        jassertfalse;
    }

    void cancelDownloads (const Array<Download*>& downloads)
    {
        // Not available on this platform.
        ignoreUnused (downloads);
        jassertfalse;
    }

private:
    #define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
      METHOD (constructor,                  "<init>",                     "(Landroid/content/Context;J)V")                                              \
      METHOD (endConnection,                "endConnection",              "()V")                                                                        \
      METHOD (isReady,                      "isReady",                    "()Z")                                                                        \
      METHOD (isBillingSupported,           "isBillingSupported",         "()Z")                                                                        \
      METHOD (querySkuDetails,              "querySkuDetails",            "([Ljava/lang/String;)V")                                                     \
      METHOD (launchBillingFlow,            "launchBillingFlow",          "(Landroid/app/Activity;Lcom/android/billingclient/api/BillingFlowParams;)V") \
      METHOD (queryPurchases,               "queryPurchases",             "()V")                                                                        \
      METHOD (consumePurchase,              "consumePurchase",            "(Ljava/lang/String;Ljava/lang/String;)V")                                    \
                                                                                                                                                        \
      CALLBACK (skuDetailsQueryCallback,    "skuDetailsQueryCallback",    "(JLjava/util/List;)V")                                                       \
      CALLBACK (purchasesListQueryCallback, "purchasesListQueryCallback", "(JLjava/util/List;)V")                                                       \
      CALLBACK (purchaseCompletedCallback,  "purchaseCompletedCallback",  "(JLcom/android/billingclient/api/Purchase;I)V")                              \
      CALLBACK (purchaseConsumedCallback,   "purchaseConsumedCallback",   "(JLjava/lang/String;I)V")

    DECLARE_JNI_CLASS (JuceBillingClient, "com/rmsl/juce/JuceBillingClient")
    #undef JNI_CLASS_MEMBERS

    static void JNICALL skuDetailsQueryCallback (JNIEnv*, jobject, jlong host, jobject skuDetailsList)
    {
        if (auto* myself = reinterpret_cast<Pimpl*> (host))
            myself->updateSkuDetails (skuDetailsList);
    }

    static void JNICALL purchasesListQueryCallback (JNIEnv*, jobject, jlong host, jobject purchasesList)
    {
        if (auto* myself = reinterpret_cast<Pimpl*> (host))
            myself->updatePurchasesList (purchasesList);
    }

    static void JNICALL purchaseCompletedCallback (JNIEnv*, jobject, jlong host, jobject purchase, int responseCode)
    {
        if (auto* myself = reinterpret_cast<Pimpl*> (host))
            myself->purchaseCompleted (purchase, responseCode);
    }

    static void JNICALL purchaseConsumedCallback (JNIEnv*, jobject, jlong host, jstring productIdentifier, int responseCode)
    {
        if (auto* myself = reinterpret_cast<Pimpl*> (host))
            myself->purchaseConsumed (productIdentifier, responseCode);
    }

    //==============================================================================
    bool isReady() const
    {
        return getEnv()->CallBooleanMethod (billingClient, JuceBillingClient.isReady);
    }

    bool checkIsReady() const
    {
        for (int i = 0; i < 10; ++i)
        {
            if (isReady())
                return true;

            Thread::sleep (500);
        }

        return false;
    }

    //==============================================================================
    static StringArray convertToLowerCase (const StringArray& stringsToConvert)
    {
        StringArray lowerCase;

        for (auto& s : stringsToConvert)
            lowerCase.add (s.toLowerCase());

        return lowerCase;
    }

    void querySkuDetailsAsync (const StringArray& productIdentifiers)
    {
        Thread::launch ([=]
        {
            if (! checkIsReady())
                return;

            MessageManager::callAsync ([=]
            {
                getEnv()->CallVoidMethod (billingClient,
                                          JuceBillingClient.querySkuDetails,
                                          juceStringArrayToJava (productIdentifiers).get());
            });
        });
    }

    void getProductsBoughtAsync()
    {
        Thread::launch ([=]
        {
            if (! checkIsReady())
                return;

            MessageManager::callAsync ([=]
            {
                getEnv()->CallVoidMethod (billingClient,
                                          JuceBillingClient.queryPurchases);
            });
        });
    }

    //==============================================================================
    void notifyListenersAboutPurchase (const InAppPurchases::Purchase& purchase, bool success, const String& statusDescription)
    {
        owner.listeners.call ([&] (Listener& l) { l.productPurchaseFinished ({ purchase, {} }, success, statusDescription); });
    }

    void notifyListenersAboutConsume (const String& productIdentifier, bool success, const String& statusDescription)
    {
        owner.listeners.call ([&] (Listener& l) { l.productConsumed (productIdentifier, success, statusDescription); });
    }

    LocalRef<jobject> createBillingFlowParamsBuilder (LocalRef<jobject> skuDetails)
    {
        auto* env = getEnv();

        auto builder = LocalRef<jobject> (env->CallStaticObjectMethod (BillingFlowParams, BillingFlowParams.newBuilder));

        return LocalRef<jobject> (env->CallObjectMethod (builder.get(),
                                  BillingFlowParamsBuilder.setSkuDetails,
                                  skuDetails.get()));
    }

    void launchBillingFlowWithParameters (LocalRef<jobject> params)
    {
        LocalRef<jobject> activity (getCurrentActivity());

        if (activity == nullptr)
            activity = getMainActivity();

        getEnv()->CallVoidMethod (billingClient,
                                  JuceBillingClient.launchBillingFlow,
                                  activity.get(),
                                  params.get());
    }

    void changeExistingSubscription (LocalRef<jobject> skuDetails, const String& subscriptionIdentifier, bool creditForUnusedSubscription)
    {
        if (! isReady())
        {
            notifyListenersAboutPurchase ({}, false, NEEDS_TRANS ("In-App purchases unavailable"));
            return;
        }

        purchasesListQueryCallbackQueue.emplace ([=] (LocalRef<jobject> purchasesList)
        {
            if (purchasesList != nullptr)
            {
                auto* env = getEnv();

                for (int i = 0; i < env->CallIntMethod (purchasesList, JavaArrayList.size); ++i)
                {
                    auto purchase = buildPurchase (LocalRef<jobject> (env->CallObjectMethod (purchasesList.get(), JavaArrayList.get, i)));

                    if (purchase.productId == subscriptionIdentifier)
                    {
                        auto builder = createBillingFlowParamsBuilder (skuDetails);

                        builder = LocalRef<jobject> (env->CallObjectMethod (builder.get(),
                                                                            BillingFlowParamsBuilder.setOldSku,
                                                                            javaString (subscriptionIdentifier).get(),
                                                                            javaString (purchase.purchaseToken).get()));

                        if (! creditForUnusedSubscription)
                            builder = LocalRef<jobject> (env->CallObjectMethod (builder.get(),
                                                                                BillingFlowParamsBuilder.setReplaceSkusProrationMode,
                                                                                3 /*IMMEDIATE_WITHOUT_PRORATION*/));

                        launchBillingFlowWithParameters (LocalRef<jobject> (env->CallObjectMethod (builder.get(),
                                                                                                   BillingFlowParamsBuilder.build)));
                    }
                }
            }

            notifyListenersAboutPurchase ({}, false, NEEDS_TRANS ("Unable to get subscription details"));
        });

        getProductsBoughtAsync();
    }

    void purchaseProductWithSkuDetails (LocalRef<jobject> skuDetails)
    {
        if (! isReady())
        {
            notifyListenersAboutPurchase ({}, false, NEEDS_TRANS ("In-App purchases unavailable"));
            return;
        }

        launchBillingFlowWithParameters (LocalRef<jobject> (getEnv()->CallObjectMethod (createBillingFlowParamsBuilder (skuDetails).get(),
                                                                                        BillingFlowParamsBuilder.build)));
    }

    void consumePurchaseWithToken (const String& productIdentifier, const String& purchaseToken)
    {
        if (! isReady())
        {
            notifyListenersAboutConsume (productIdentifier, false, NEEDS_TRANS ("In-App purchases unavailable"));
            return;
        }

        getEnv()->CallObjectMethod (billingClient,
                                    JuceBillingClient.consumePurchase,
                                    LocalRef<jstring> (javaString (productIdentifier)).get(),
                                    LocalRef<jstring> (javaString (purchaseToken)).get());
    }

    //==============================================================================
    static InAppPurchases::Purchase buildPurchase (LocalRef<jobject> purchase)
    {
        if (purchase == nullptr)
            return {};

        auto* env = getEnv();

        return { juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (purchase, AndroidPurchase.getOrderId))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (purchase, AndroidPurchase.getSku))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (purchase, AndroidPurchase.getPackageName))),
                 Time (env->CallLongMethod (purchase, AndroidPurchase.getPurchaseTime)).toString (true, true, true, true),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (purchase, AndroidPurchase.getPurchaseToken))) };
    }

    static InAppPurchases::Product buildProduct (LocalRef<jobject> productSkuDetails)
    {
        if (productSkuDetails == nullptr)
            return {};

        auto* env = getEnv();

        return { juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (productSkuDetails, SkuDetails.getSku))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (productSkuDetails, SkuDetails.getTitle))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (productSkuDetails, SkuDetails.getDescription))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (productSkuDetails, SkuDetails.getPrice))),
                 juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (productSkuDetails, SkuDetails.getPriceCurrencyCode))) };
    }

    static String getStatusDescriptionFromResponseCode (int responseCode)
    {
        switch (responseCode)
        {
            case 0:   return NEEDS_TRANS ("Success");
            case 1:   return NEEDS_TRANS ("Cancelled by user");
            case 2:   return NEEDS_TRANS ("Service unavailable");
            case 3:   return NEEDS_TRANS ("Billing unavailable");
            case 4:   return NEEDS_TRANS ("Item unavailable");
            case 5:   return NEEDS_TRANS ("Internal error");
            case 6:   return NEEDS_TRANS ("Generic error");
            case 7:   return NEEDS_TRANS ("Item already owned");
            case 8:   return NEEDS_TRANS ("Item not owned");
            default:  return NEEDS_TRANS ("Unknown status");
        }
    }

    static bool wasSuccessful (int responseCode)
    {
        return responseCode == 0;
    }

    void purchaseCompleted (jobject purchase, int responseCode)
    {
        notifyListenersAboutPurchase (buildPurchase (LocalRef<jobject> (purchase)),
                                      wasSuccessful (responseCode),
                                      getStatusDescriptionFromResponseCode (responseCode));
    }

    void purchaseConsumed (jstring productIdentifier, int responseCode)
    {
        notifyListenersAboutConsume (juceString (LocalRef<jstring> (productIdentifier)),
                                     wasSuccessful (responseCode),
                                     getStatusDescriptionFromResponseCode (responseCode));
    }

    void updateSkuDetails (jobject skuDetailsList)
    {
        jassert (! skuDetailsQueryCallbackQueue.empty());
        skuDetailsQueryCallbackQueue.front() (LocalRef<jobject> (skuDetailsList));
        skuDetailsQueryCallbackQueue.pop();
    }

    void updatePurchasesList (jobject purchasesList)
    {
        jassert (! purchasesListQueryCallbackQueue.empty());
        purchasesListQueryCallbackQueue.front() (LocalRef<jobject> (purchasesList));
        purchasesListQueryCallbackQueue.pop();
    }

    //==============================================================================
    InAppPurchases& owner;
    GlobalRef billingClient;

    std::queue<std::function<void (LocalRef<jobject>)>> skuDetailsQueryCallbackQueue,
                                                        purchasesListQueryCallbackQueue;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};


InAppPurchases::Pimpl::JuceBillingClient_Class InAppPurchases::Pimpl::JuceBillingClient;

} // namespace juce
