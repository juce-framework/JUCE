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

namespace juce
{

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getProductId,                     "getProductId",                         "()Ljava/lang/String;") \
  METHOD (getTitle,                         "getTitle",                             "()Ljava/lang/String;") \
  METHOD (getDescription,                   "getDescription",                       "()Ljava/lang/String;") \
  METHOD (getOneTimePurchaseOfferDetails,   "getOneTimePurchaseOfferDetails",       "()Lcom/android/billingclient/api/ProductDetails$OneTimePurchaseOfferDetails;") \
  METHOD (getSubscriptionOfferDetails,      "getSubscriptionOfferDetails",          "()Ljava/util/List;")

DECLARE_JNI_CLASS (ProductDetails, "com/android/billingclient/api/ProductDetails")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getFormattedPrice,                "getFormattedPrice",                    "()Ljava/lang/String;") \
  METHOD (getPriceCurrencyCode,             "getPriceCurrencyCode",                 "()Ljava/lang/String;")

DECLARE_JNI_CLASS (OneTimePurchaseOfferDetails, "com/android/billingclient/api/ProductDetails$OneTimePurchaseOfferDetails")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getFormattedPrice,                "getFormattedPrice",                    "()Ljava/lang/String;") \
  METHOD (getPriceCurrencyCode,             "getPriceCurrencyCode",                 "()Ljava/lang/String;")

DECLARE_JNI_CLASS (PricingPhase, "com/android/billingclient/api/ProductDetails$PricingPhase")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getOfferToken,                "getOfferToken",                    "()Ljava/lang/String;") \
  METHOD (getPricingPhases,             "getPricingPhases",                 "()Lcom/android/billingclient/api/ProductDetails$PricingPhases;")

DECLARE_JNI_CLASS (SubscriptionOfferDetails, "com/android/billingclient/api/ProductDetails$SubscriptionOfferDetails")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getPricingPhaseList,             "getPricingPhaseList",                 "()Ljava/util/List;")

DECLARE_JNI_CLASS (PricingPhases, "com/android/billingclient/api/ProductDetails$PricingPhases")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  STATICMETHOD (newBuilder, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$ProductDetailsParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsProductDetailsParams, "com/android/billingclient/api/BillingFlowParams$ProductDetailsParams")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  STATICMETHOD (newBuilder, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParams, "com/android/billingclient/api/BillingFlowParams")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  STATICMETHOD (newBuilder, "newBuilder", "()Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsSubscriptionUpdateParams, "com/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (build,                       "build",                       "()Lcom/android/billingclient/api/BillingFlowParams;") \
  METHOD (setSubscriptionUpdateParams, "setSubscriptionUpdateParams", "(Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams;)Lcom/android/billingclient/api/BillingFlowParams$Builder;") \
  METHOD (setProductDetailsParamsList, "setProductDetailsParamsList", "(Ljava/util/List;)Lcom/android/billingclient/api/BillingFlowParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsBuilder, "com/android/billingclient/api/BillingFlowParams$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (build,                       "build",                       "()Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams;") \
  METHOD (setOldPurchaseToken,         "setOldPurchaseToken",         "(Ljava/lang/String;)Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder;") \
  METHOD (setReplaceProrationMode,     "setReplaceProrationMode",     "(I)Lcom/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsSubscriptionUpdateParamsBuilder, "com/android/billingclient/api/BillingFlowParams$SubscriptionUpdateParams$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (build,                       "build",                       "()Lcom/android/billingclient/api/BillingFlowParams$ProductDetailsParams;") \
  METHOD (setOfferToken,               "setOfferToken",               "(Ljava/lang/String;)Lcom/android/billingclient/api/BillingFlowParams$ProductDetailsParams$Builder;") \
  METHOD (setProductDetails,           "setProductDetails",           "(Lcom/android/billingclient/api/ProductDetails;)Lcom/android/billingclient/api/BillingFlowParams$ProductDetailsParams$Builder;")

DECLARE_JNI_CLASS (BillingFlowParamsProductDetailsParamsBuilder, "com/android/billingclient/api/BillingFlowParams$ProductDetailsParams$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (getOrderId,       "getOrderId",       "()Ljava/lang/String;") \
  METHOD (getPurchaseState, "getPurchaseState", "()I") \
  METHOD (getProducts,      "getProducts",      "()Ljava/util/List;") \
  METHOD (getPackageName,   "getPackageName",   "()Ljava/lang/String;") \
  METHOD (getPurchaseTime,  "getPurchaseTime",  "()J") \
  METHOD (getPurchaseToken, "getPurchaseToken", "()Ljava/lang/String;")

DECLARE_JNI_CLASS (AndroidPurchase, "com/android/billingclient/api/Purchase")
#undef JNI_CLASS_MEMBERS

template <typename Fn>
static void callOnMainThread (Fn&& fn)
{
    if (MessageManager::getInstance()->isThisTheMessageThread())
        fn();
    else
        MessageManager::callAsync (std::forward<Fn> (fn));
}

inline StringArray javaListOfStringToJuceStringArray (const LocalRef<jobject>& javaArray)
{
    if (javaArray.get() == nullptr)
        return {};

    auto* env = getEnv();

    StringArray result;

    const auto size = env->CallIntMethod (javaArray, JavaList.size);

    for (int i = 0; i < size; ++i)
        result.add (juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (javaArray, JavaList.get, i) }.get()));

    return result;
}

//==============================================================================
struct InAppPurchases::Pimpl
{
    Pimpl (InAppPurchases& parent)
        : owner (parent),
          billingClient (LocalRef<jobject> { getEnv()->NewObject (JuceBillingClient,
                                                                  JuceBillingClient.constructor,
                                                                  getAppContext().get(),
                                                                  (jlong) this) })
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
        productDetailsQueryCallbackQueue.emplace ([this] (LocalRef<jobject> productDetailsList)
        {
            if (productDetailsList != nullptr)
            {
                auto* env = getEnv();
                Array<InAppPurchases::Product> products;

                for (int i = 0; i < env->CallIntMethod (productDetailsList, JavaList.size); ++i)
                    products.add (buildProduct (LocalRef<jobject> { env->CallObjectMethod (productDetailsList, JavaList.get, i) }));

                callMemberOnMainThread ([this, products]
                {
                    owner.listeners.call ([&] (Listener& l) { l.productsInfoReturned (products); });
                });
            }
        });

        queryProductDetailsAsync (convertToLowerCase (productIdentifiers));
    }

    void purchaseProduct (const String& productIdentifier,
                          const String& subscriptionIdentifier,
                          bool creditForUnusedSubscription)
    {
        productDetailsQueryCallbackQueue.emplace ([=] (LocalRef<jobject> productDetailsList)
        {
            if (productDetailsList != nullptr)
            {
                auto* env = getEnv();

                if (env->CallIntMethod (productDetailsList, JavaList.size) > 0)
                {
                    GlobalRef productDetails (LocalRef<jobject> { env->CallObjectMethod (productDetailsList, JavaList.get, 0) });

                    callMemberOnMainThread ([this, productDetails, subscriptionIdentifier, creditForUnusedSubscription]
                    {
                        if (subscriptionIdentifier.isNotEmpty())
                            changeExistingSubscription (productDetails, subscriptionIdentifier, creditForUnusedSubscription);
                        else
                            purchaseProductWithProductDetails (productDetails);
                    });
                }
            }
        });

        queryProductDetailsAsync (convertToLowerCase ({ productIdentifier }));
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
                    const LocalRef<jobject> purchase { env->CallObjectMethod (purchasesList, JavaArrayList.get, i) };
                    purchases.add ({ buildPurchase (purchase), {} });
                }

                callMemberOnMainThread ([this, purchases]
                {
                        owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored (purchases, true, NEEDS_TRANS ("Success")); });
                });
            }
            else
            {
                callMemberOnMainThread ([this]
                {
                    owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Failure")); });
                });
            }
        });

        getProductsBoughtAsync();
    }

    void consumePurchase (const String& productIdentifier, const String& purchaseToken)
    {
        if (purchaseToken.isEmpty())
        {
            productDetailsQueryCallbackQueue.emplace ([=] (LocalRef<jobject> productDetailsList)
            {
                if (productDetailsList != nullptr)
                {
                    auto* env = getEnv();

                    if (env->CallIntMethod (productDetailsList, JavaList.size) > 0)
                    {
                        const LocalRef<jobject> product { env->CallObjectMethod (productDetailsList, JavaList.get, 0) };

                        auto token = juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (product, ProductDetails.getProductId) });

                        if (token.isNotEmpty())
                        {
                            consumePurchaseWithToken (productIdentifier, token);
                            return;
                        }
                    }
                }

                callMemberOnMainThread ([this, productIdentifier]
                {
                    notifyListenersAboutConsume (productIdentifier, false, NEEDS_TRANS ("Item unavailable"));
                });
            });

            queryProductDetailsAsync (convertToLowerCase ({ productIdentifier }));
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
      METHOD (constructor,                   "<init>",                      "(Landroid/content/Context;J)V")                                              \
      METHOD (endConnection,                 "endConnection",               "()V")                                                                        \
      METHOD (isReady,                       "isReady",                     "()Z")                                                                        \
      METHOD (isBillingSupported,            "isBillingSupported",          "()Z")                                                                        \
      METHOD (queryProductDetails,           "queryProductDetails",         "([Ljava/lang/String;)V")                                                     \
      METHOD (launchBillingFlow,             "launchBillingFlow",           "(Landroid/app/Activity;Lcom/android/billingclient/api/BillingFlowParams;)V") \
      METHOD (queryPurchases,                "queryPurchases",              "()V")                                                                        \
      METHOD (consumePurchase,               "consumePurchase",             "(Ljava/lang/String;Ljava/lang/String;)V")                                    \
                                                                                                                                                          \
      CALLBACK (productDetailsQueryCallback, "productDetailsQueryCallback", "(JLjava/util/List;)V")                                                       \
      CALLBACK (purchasesListQueryCallback,  "purchasesListQueryCallback",  "(JLjava/util/List;)V")                                                       \
      CALLBACK (purchaseCompletedCallback,   "purchaseCompletedCallback",   "(JLcom/android/billingclient/api/Purchase;I)V")                              \
      CALLBACK (purchaseConsumedCallback,    "purchaseConsumedCallback",    "(JLjava/lang/String;I)V")

    DECLARE_JNI_CLASS (JuceBillingClient, "com/rmsl/juce/JuceBillingClient")
    #undef JNI_CLASS_MEMBERS

    static void JNICALL productDetailsQueryCallback (JNIEnv*, jobject, jlong host, jobject productDetailsList)
    {
        if (auto* myself = reinterpret_cast<Pimpl*> (host))
            myself->updateProductDetails (productDetailsList);
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

    void queryProductDetailsAsync (const StringArray& productIdentifiers)
    {
        Thread::launch ([=]
        {
            if (! checkIsReady())
                return;

            MessageManager::callAsync ([=]
            {
                getEnv()->CallVoidMethod (billingClient,
                                          JuceBillingClient.queryProductDetails,
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

    void launchBillingFlowWithParameters (LocalRef<jobject> params)
    {
        const auto activity = []
        {
            if (auto current = getCurrentActivity())
                return current;

            return getMainActivity();
        }();

        getEnv()->CallVoidMethod (billingClient,
                                  JuceBillingClient.launchBillingFlow,
                                  activity.get(),
                                  params.get());
    }

    void changeExistingSubscription (GlobalRef productDetails, const String& subscriptionIdentifier, bool creditForUnusedSubscription)
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
                    auto purchase = buildPurchase (LocalRef<jobject> { env->CallObjectMethod (purchasesList.get(), JavaArrayList.get, i) });

                    if (purchase.productIds.contains (subscriptionIdentifier))
                    {
                        const LocalRef<jobject> subscriptionBuilder { getEnv()->CallStaticObjectMethod (BillingFlowParamsSubscriptionUpdateParams,
                                                                                                        BillingFlowParamsSubscriptionUpdateParams.newBuilder) };
                        env->CallObjectMethod (subscriptionBuilder.get(),
                                               BillingFlowParamsSubscriptionUpdateParamsBuilder.setOldPurchaseToken,
                                               javaString (purchase.purchaseToken).get());

                        if (! creditForUnusedSubscription)
                        {
                            env->CallObjectMethod (subscriptionBuilder.get(),
                                                   BillingFlowParamsSubscriptionUpdateParamsBuilder.setReplaceProrationMode,
                                                   3 /*IMMEDIATE_WITHOUT_PRORATION*/);
                        }

                        const LocalRef<jobject> subscriptionParams { env->CallObjectMethod (subscriptionBuilder.get(),
                                                                                            BillingFlowParamsSubscriptionUpdateParamsBuilder.build) };

                        const LocalRef<jobject> builder { env->CallStaticObjectMethod (BillingFlowParams, BillingFlowParams.newBuilder) };
                        env->CallObjectMethod (builder.get(),
                                               BillingFlowParamsBuilder.setSubscriptionUpdateParams,
                                               subscriptionParams.get());
                        const LocalRef<jobject> params { env->CallObjectMethod (builder.get(), BillingFlowParamsBuilder.build) };

                        launchBillingFlowWithParameters (params);
                    }
                }
            }

            callMemberOnMainThread ([this]
            {
                notifyListenersAboutPurchase ({}, false, NEEDS_TRANS ("Unable to get subscription details"));
            });
        });

        getProductsBoughtAsync();
    }

    void purchaseProductWithProductDetails (GlobalRef productDetails)
    {
        if (! isReady())
        {
            notifyListenersAboutPurchase ({}, false, NEEDS_TRANS ("In-App purchases unavailable"));
            return;
        }

        auto* env = getEnv();
        const LocalRef<jobject> billingFlowParamsProductDetailsParamsBuilder { env->CallStaticObjectMethod (BillingFlowParamsProductDetailsParams, BillingFlowParamsProductDetailsParams.newBuilder) };
        env->CallObjectMethod (billingFlowParamsProductDetailsParamsBuilder, BillingFlowParamsProductDetailsParamsBuilder.setProductDetails, productDetails.get());

        if (const LocalRef<jobject> subscriptionDetailsList { env->CallObjectMethod (productDetails, ProductDetails.getSubscriptionOfferDetails) })
        {
            if (env->CallIntMethod (subscriptionDetailsList, JavaList.size) > 0)
            {
                const LocalRef<jobject> subscriptionDetails { env->CallObjectMethod (subscriptionDetailsList, JavaList.get, 0) };
                const LocalRef<jobject> offerToken { env->CallObjectMethod (subscriptionDetails, SubscriptionOfferDetails.getOfferToken) };
                env->CallObjectMethod (billingFlowParamsProductDetailsParamsBuilder, BillingFlowParamsProductDetailsParamsBuilder.setOfferToken, offerToken.get());
            }
        }

        const LocalRef<jobject> billingFlowParamsProductDetailsParams { env->CallObjectMethod (billingFlowParamsProductDetailsParamsBuilder, BillingFlowParamsProductDetailsParamsBuilder.build) };

        const LocalRef<jobject> list { env->NewObject (JavaArrayList, JavaArrayList.constructor, 0) };
        env->CallBooleanMethod (list, JavaArrayList.add, billingFlowParamsProductDetailsParams.get());

        const LocalRef<jobject> billingFlowParamsBuilder { env->CallStaticObjectMethod (BillingFlowParams, BillingFlowParams.newBuilder) };
        env->CallObjectMethod (billingFlowParamsBuilder, BillingFlowParamsBuilder.setProductDetailsParamsList, list.get());
        const LocalRef<jobject> params { env->CallObjectMethod (billingFlowParamsBuilder, BillingFlowParamsBuilder.build) };

        launchBillingFlowWithParameters (params);
    }

    void consumePurchaseWithToken (const String& productIdentifier, const String& purchaseToken)
    {
        if (! isReady())
        {
            callMemberOnMainThread ([this, productIdentifier]
            {
                notifyListenersAboutConsume (productIdentifier, false, NEEDS_TRANS ("In-App purchases unavailable"));
            });

            return;
        }

        getEnv()->CallObjectMethod (billingClient,
                                    JuceBillingClient.consumePurchase,
                                    LocalRef<jstring> { javaString (productIdentifier) }.get(),
                                    LocalRef<jstring> { javaString (purchaseToken) }.get());
    }

    //==============================================================================
    static InAppPurchases::Purchase buildPurchase (LocalRef<jobject> purchase)
    {
        if (purchase == nullptr)
            return {};

        auto* env = getEnv();

        if (env->CallIntMethod(purchase, AndroidPurchase.getPurchaseState) != 1 /* PURCHASED */)
            return {};

        return { juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (purchase, AndroidPurchase.getOrderId) }),
                 javaListOfStringToJuceStringArray (LocalRef<jobject> { env->CallObjectMethod (purchase, AndroidPurchase.getProducts) }),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (purchase, AndroidPurchase.getPackageName) }),
                 Time (env->CallLongMethod (purchase, AndroidPurchase.getPurchaseTime)).toString (true, true, true, true),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (purchase, AndroidPurchase.getPurchaseToken) }) };
    }

    static InAppPurchases::Product buildProduct (LocalRef<jobject> productDetails)
    {
        if (productDetails == nullptr)
            return {};

        auto* env = getEnv();

        if (LocalRef<jobject> oneTimePurchase { env->CallObjectMethod (productDetails, ProductDetails.getOneTimePurchaseOfferDetails) })
        {
            return { juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getProductId) }),
                     juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getTitle) }),
                     juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getDescription) }),
                     juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (oneTimePurchase, OneTimePurchaseOfferDetails.getFormattedPrice) }),
                     juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (oneTimePurchase, OneTimePurchaseOfferDetails.getPriceCurrencyCode) }) };
        }

        LocalRef<jobject> subscription { env->CallObjectMethod (productDetails, ProductDetails.getSubscriptionOfferDetails) };

        if (env->CallIntMethod (subscription, JavaList.size) == 0)
            return {};

        // We can only return a single subscription price for this subscription,
        // but the subscription has more than one pricing scheme.
        jassert (env->CallIntMethod (subscription, JavaList.size) == 1);

        const LocalRef<jobject> offerDetails { env->CallObjectMethod (subscription, JavaList.get, 0) };
        const LocalRef<jobject> pricingPhases { env->CallObjectMethod (offerDetails, SubscriptionOfferDetails.getPricingPhases) };
        const LocalRef<jobject> phaseList { env->CallObjectMethod (pricingPhases, PricingPhases.getPricingPhaseList) };

        if (env->CallIntMethod (phaseList, JavaList.size) == 0)
            return {};

        // We can only return a single subscription price for this subscription,
        // but the pricing scheme for this subscription has more than one phase.
        jassert (env->CallIntMethod (phaseList, JavaList.size) == 1);

        const LocalRef<jobject> phase { env->CallObjectMethod (phaseList, JavaList.get, 0) };

        return { juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getProductId) }),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getTitle) }),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (productDetails, ProductDetails.getDescription) }),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (phase, PricingPhase.getFormattedPrice) }),
                 juceString (LocalRef<jstring> { (jstring) env->CallObjectMethod (phase, PricingPhase.getPriceCurrencyCode) }) };
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
        notifyListenersAboutPurchase (buildPurchase (LocalRef<jobject> { purchase }),
                                      wasSuccessful (responseCode),
                                      getStatusDescriptionFromResponseCode (responseCode));
    }

    void purchaseConsumed (jstring productIdentifier, int responseCode)
    {
        notifyListenersAboutConsume (juceString (LocalRef<jstring> { productIdentifier }),
                                     wasSuccessful (responseCode),
                                     getStatusDescriptionFromResponseCode (responseCode));
    }

    void updateProductDetails (jobject productDetailsList)
    {
        jassert (! productDetailsQueryCallbackQueue.empty());
        productDetailsQueryCallbackQueue.front() (LocalRef<jobject> { productDetailsList });
        productDetailsQueryCallbackQueue.pop();
    }

    void updatePurchasesList (jobject purchasesList)
    {
        jassert (! purchasesListQueryCallbackQueue.empty());
        purchasesListQueryCallbackQueue.front() (LocalRef<jobject> { purchasesList });
        purchasesListQueryCallbackQueue.pop();
    }

    //==============================================================================
    InAppPurchases& owner;
    GlobalRef billingClient;

    std::queue<std::function<void (LocalRef<jobject>)>> productDetailsQueryCallbackQueue,
                                                        purchasesListQueryCallbackQueue;

    //==============================================================================
    void callMemberOnMainThread (std::function<void()> callback)
    {
        callOnMainThread ([ref = WeakReference<Pimpl> (this), callback]
        {
            if (ref != nullptr)
                callback();
        });
    }

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE(Pimpl)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

void juce_handleOnResume()
{
    callOnMainThread ([]
    {
        InAppPurchases::getInstance()->restoreProductsBoughtList (false);
    });
}


InAppPurchases::Pimpl::JuceBillingClient_Class InAppPurchases::Pimpl::JuceBillingClient;

} // namespace juce
