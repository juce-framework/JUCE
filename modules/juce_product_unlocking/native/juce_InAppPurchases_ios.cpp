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
struct InAppPurchases::Pimpl
{
    /** AppStore implementation of hosted content download. */
    struct DownloadImpl  : public Download
    {
        explicit DownloadImpl (SKDownload* downloadToUse)
            : download (downloadToUse)
        {
            [download retain];
        }

        ~DownloadImpl() override
        {
            [download release];
        }

        String getProductId()      const override  { return nsStringToJuce (download.contentIdentifier); }
        String getContentVersion() const override  { return nsStringToJuce (download.contentVersion); }
        Status getStatus()         const override  { return SKDownloadStateToDownloadStatus (download.state); }

      #if JUCE_IOS
        int64 getContentLength()   const override  { return download.contentLength; }
      #else
        int64 getContentLength()   const override
        {
            if (@available (macOS 10.15, *))
                return download.expectedContentLength;

            return download.contentLength.longLongValue;
        }
      #endif

        SKDownload* download;
    };

    /** Represents a pending request initialised with [SKProductRequest start]. */
    struct PendingProductInfoRequest
    {
        enum class Type
        {
            query = 0,
            purchase
        };

        Type type;
        NSUniquePtr<SKProductsRequest> request;
    };

    /** Represents a pending request started from [SKReceiptRefreshRequest start]. */
    struct PendingReceiptRefreshRequest
    {
        String subscriptionsSharedSecret;
        NSUniquePtr<SKReceiptRefreshRequest> request;
    };

    /** Represents a transaction with pending downloads. Only after all downloads
        are finished, the transaction is marked as finished. */
    struct PendingDownloadsTransaction
    {
        PendingDownloadsTransaction (SKPaymentTransaction* t)  : transaction (t)
        {
            addDownloadsFromSKTransaction (transaction);
        }

        void addDownloadsFromSKTransaction (SKPaymentTransaction* transactionToUse)
        {
            for (SKDownload* download in transactionToUse.downloads)
                downloads.add (new DownloadImpl (download));
        }

        bool canBeMarkedAsFinished() const
        {
            for (SKDownload* d in transaction.downloads)
            {
                SKDownloadState state = d.state;

                if (state != SKDownloadStateFinished
                     && state != SKDownloadStateFailed
                     && state != SKDownloadStateCancelled)
                {
                    return false;
                }
            }

            return true;
        }

        OwnedArray<DownloadImpl> downloads;
        SKPaymentTransaction* const transaction;
    };

    //==============================================================================
    explicit Pimpl (InAppPurchases& p)
        : owner (p)
    {
        Class::setThis (delegate.get(), this);
        [[SKPaymentQueue defaultQueue] addTransactionObserver: delegate.get()];
    }

    ~Pimpl()
    {
        [[SKPaymentQueue defaultQueue] removeTransactionObserver: delegate.get()];
    }

    //==============================================================================
    bool isInAppPurchasesSupported() const     { return true; }

    void getProductsInformation (const StringArray& productIdentifiers)
    {
        auto productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers: [NSSet setWithArray: createNSArrayFromStringArray (productIdentifiers)]];

        pendingProductInfoRequests.emplace_back (new PendingProductInfoRequest { PendingProductInfoRequest::Type::query,
                                                                                 NSUniquePtr<SKProductsRequest> (productsRequest) });

        productsRequest.delegate = delegate.get();
        [productsRequest start];
    }

    void purchaseProduct (const String& productIdentifier, const String&, bool)
    {
        if (! [SKPaymentQueue canMakePayments])
        {
            owner.listeners.call ([&] (Listener& l) { l.productPurchaseFinished ({}, false, NEEDS_TRANS ("Payments not allowed")); });
            return;
        }

        auto productIdentifiers = [NSArray arrayWithObject: juceStringToNS (productIdentifier)];
        auto productsRequest    = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];

        pendingProductInfoRequests.emplace_back (new PendingProductInfoRequest { PendingProductInfoRequest::Type::purchase,
                                                                                 NSUniquePtr<SKProductsRequest> (productsRequest) });

        productsRequest.delegate = delegate.get();
        [productsRequest start];
    }

    void restoreProductsBoughtList (bool includeDownloadInfo, const String& subscriptionsSharedSecret)
    {
        if (includeDownloadInfo)
        {
            [[SKPaymentQueue defaultQueue] restoreCompletedTransactions];
        }
        else
        {
            auto receiptRequest = [[SKReceiptRefreshRequest alloc] init];

            pendingReceiptRefreshRequests.emplace_back (new PendingReceiptRefreshRequest { subscriptionsSharedSecret,
                                                                                           NSUniquePtr<SKReceiptRefreshRequest> ([receiptRequest retain]) });
            receiptRequest.delegate = delegate.get();
            [receiptRequest start];
        }
    }

    void consumePurchase (const String&, const String&) {}

    //==============================================================================
    void startDownloads (const Array<Download*>& downloads)
    {
        [[SKPaymentQueue defaultQueue] startDownloads: downloadsToSKDownloads (removeInvalidDownloads (downloads))];
    }

    void pauseDownloads (const Array<Download*>& downloads)
    {
        [[SKPaymentQueue defaultQueue] pauseDownloads: downloadsToSKDownloads (removeInvalidDownloads (downloads))];
    }

    void resumeDownloads (const Array<Download*>& downloads)
    {
        [[SKPaymentQueue defaultQueue] resumeDownloads: downloadsToSKDownloads (removeInvalidDownloads (downloads))];
    }

    void cancelDownloads (const Array<Download*>& downloads)
    {
        [[SKPaymentQueue defaultQueue] cancelDownloads: downloadsToSKDownloads (removeInvalidDownloads (downloads))];
    }

    //==============================================================================
    void notifyProductsInfoReceived (NSArray<SKProduct*>* products)
    {
        Array<Product> productsToReturn;

        for (SKProduct* skProduct in products)
            productsToReturn.add (SKProductToIAPProduct (skProduct));

        owner.listeners.call ([&] (Listener& l) { l.productsInfoReturned (productsToReturn); });
    }

    void startPurchase (NSArray<SKProduct*>* products)
    {
        if ([products count] > 0)
        {
            // Only one product can be bought at once!
            jassert ([products count] == 1);

            auto* product = products[0];
            auto payment = [SKPayment paymentWithProduct: product];
            [[SKPaymentQueue defaultQueue] addPayment: payment];
        }
        else
        {
            owner.listeners.call ([] (Listener& l) { l.productPurchaseFinished ({}, false, NEEDS_TRANS ("Your app is not setup for payments")); });
        }
    }

    //==============================================================================
    Array<Download*> removeInvalidDownloads (const Array<Download*>& downloadsToUse)
    {
        Array<Download*> downloads (downloadsToUse);

        for (int i = downloads.size(); --i >= 0;)
        {
            auto hasPendingDownload = hasDownloadInPendingDownloadsTransaction (*downloads[i]);

            // Invalid download passed, it does not exist in pending downloads list
            jassert (hasPendingDownload);

            if (! hasPendingDownload)
                downloads.remove (i);
        }

        return downloads;
    }

    bool hasDownloadInPendingDownloadsTransaction (const Download& download)
    {
        for (auto* pdt : pendingDownloadsTransactions)
            for (auto* pendingDownload : pdt->downloads)
                if (pendingDownload == &download)
                    return true;

        return false;
    }

    //==============================================================================
    void processTransactionFinish (SKPaymentTransaction* transaction, bool success)
    {
        auto orderId      = nsStringToJuce (transaction.transactionIdentifier);
        auto packageName  = nsStringToJuce ([[NSBundle mainBundle] bundleIdentifier]);
        auto productId    = nsStringToJuce (transaction.payment.productIdentifier);
        auto purchaseTime = Time (1000 * (int64) transaction.transactionDate.timeIntervalSince1970)
                              .toString (true, true, true, true);

        Purchase purchase { orderId, productId, packageName, purchaseTime, {} };

        Array<Download*> downloads;

        // If transaction failed or there are no downloads, finish the transaction immediately, otherwise
        // finish the transaction only after all downloads are finished.
        if (transaction.transactionState == SKPaymentTransactionStateFailed
             || transaction.downloads == nil
             || [transaction.downloads count] == 0)
        {
            [[SKPaymentQueue defaultQueue]  finishTransaction: transaction];
        }
        else
        {
            // On application startup or when the app is resumed we may receive multiple
            // "purchased" callbacks with the same underlying transaction. Sadly, only
            // the last set of downloads will be valid.
            auto* pdt = getPendingDownloadsTransactionForSKTransaction (transaction);

            if (pdt == nullptr)
            {
                pdt = pendingDownloadsTransactions.add (new PendingDownloadsTransaction (transaction));
            }
            else
            {
                pdt->downloads.clear();
                pdt->addDownloadsFromSKTransaction (transaction);
            }

            for (auto* download : pdt->downloads)
                downloads.add (download);
        }

        if (transaction.transactionState == SKPaymentTransactionStateRestored)
            restoredPurchases.add ({ purchase, downloads });
        else
            owner.listeners.call ([&] (Listener& l) { l.productPurchaseFinished ({ purchase, downloads }, success,
                                                                                 SKPaymentTransactionStateToString (transaction.transactionState)); });
    }

    PendingDownloadsTransaction* getPendingDownloadsTransactionForSKTransaction (SKPaymentTransaction* transaction)
    {
        for (auto* pdt : pendingDownloadsTransactions)
            if (pdt->transaction == transaction)
                return pdt;

        return nullptr;
    }

    //==============================================================================
    PendingDownloadsTransaction* getPendingDownloadsTransactionSKDownloadFor (SKDownload* download)
    {
        for (auto* pdt : pendingDownloadsTransactions)
            for (auto* pendingDownload : pdt->downloads)
                if (pendingDownload->download == download)
                    return pdt;

        jassertfalse;
        return nullptr;
    }

    Download* getPendingDownloadFor (SKDownload* download)
    {
        if (auto* pdt = getPendingDownloadsTransactionSKDownloadFor (download))
            for (auto* pendingDownload : pdt->downloads)
                if (pendingDownload->download == download)
                    return pendingDownload;

        jassertfalse;
        return nullptr;
    }

    void processDownloadFinish (Download* pendingDownload, SKDownload* download)
    {
        if (auto* pdt = getPendingDownloadsTransactionSKDownloadFor (download))
        {
            SKDownloadState state = download.state;

            auto contentURL = state == SKDownloadStateFinished
                                ? URL (nsStringToJuce (download.contentURL.absoluteString))
                                : URL();

            owner.listeners.call ([&] (Listener& l) { l.productDownloadFinished (*pendingDownload, contentURL); });

            if (pdt->canBeMarkedAsFinished())
            {
                // All downloads finished, mark transaction as finished too.
                [[SKPaymentQueue defaultQueue]  finishTransaction: pdt->transaction];

                pendingDownloadsTransactions.removeObject (pdt);
            }
        }
    }

    //==============================================================================
    void processReceiptRefreshResponseWithSubscriptionsSharedSecret (const String& secret)
    {
        const auto succeeded = [&]
        {
            auto receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];

            if (receiptURL == nullptr)
                return false;

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
            auto receiptData = [NSData dataWithContentsOfURL: receiptURL];
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            if (receiptData == nullptr)
                return false;

            fetchReceiptDetailsFromAppStore (receiptData, secret);
            return true;
        }();

        if (! succeeded)
            owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Receipt fetch failed")); });
    }

    void fetchReceiptDetailsFromAppStore (NSData* receiptData, const String& secret)
    {
        auto requestContents = [NSMutableDictionary dictionaryWithCapacity: (NSUInteger) (secret.isNotEmpty() ? 2 : 1)];
        [requestContents setObject: [receiptData base64EncodedStringWithOptions:0] forKey: @"receipt-data"];

        if (secret.isNotEmpty())
            [requestContents setObject: juceStringToNS (secret) forKey: @"password"];

        NSError* error;
        auto requestData = [NSJSONSerialization dataWithJSONObject: requestContents
                                                           options: 0
                                                             error: &error];
        if (requestData == nil)
        {
            sendReceiptFetchFail();
            return;
        }

        verifyReceipt ("https://buy.itunes.apple.com/verifyReceipt", requestData);
    }

    void verifyReceipt (const String& endpoint, NSData* requestData)
    {
        const auto nsurl = [NSURL URLWithString: juceStringToNS (endpoint)];

        if (nsurl == nullptr)
            return;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
        const auto storeRequest = [NSMutableURLRequest requestWithURL: nsurl];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        [storeRequest setHTTPMethod: @"POST"];
        [storeRequest setHTTPBody: requestData];

        constexpr auto sandboxURL = "https://sandbox.itunes.apple.com/verifyReceipt";

        const auto shouldRetryWithSandboxUrl = [isProduction = (endpoint != sandboxURL)] (NSDictionary* receiptDetails)
        {
            if (isProduction)
                if (const auto* status = getAs<NSNumber> (receiptDetails[@"status"]))
                    return [status intValue] == 21007;

            return false;
        };

        [[[NSURLSession sharedSession] dataTaskWithRequest: storeRequest
                                         completionHandler: ^(NSData* responseData, NSURLResponse*, NSError* connectionError)
                                                            {
                                                                if (connectionError == nullptr)
                                                                {
                                                                    NSError* err = nullptr;

                                                                    if (NSDictionary* receiptDetails = [NSJSONSerialization JSONObjectWithData: responseData options: 0 error: &err])
                                                                    {
                                                                        if (shouldRetryWithSandboxUrl (receiptDetails))
                                                                        {
                                                                            verifyReceipt (sandboxURL, requestData);
                                                                            return;
                                                                        }

                                                                        processReceiptDetails (receiptDetails);
                                                                        return;
                                                                    }
                                                                }

                                                                sendReceiptFetchFailAsync();
                                                            }] resume];
    }

    void processReceiptDetails (NSDictionary* receiptDetails)
    {
        if (auto receipt = getAs<NSDictionary> (receiptDetails[@"receipt"]))
        {
            if (auto bundleId = getAs<NSString> (receipt[@"bundle_id"]))
            {
                if (auto inAppPurchases = getAs<NSArray> (receipt[@"in_app"]))
                {
                    Array<Listener::PurchaseInfo> purchases;

                    for (id inAppPurchaseData in inAppPurchases)
                    {
                        auto* purchaseData = getAs<NSDictionary> (inAppPurchaseData);

                        if (purchaseData == nullptr)
                            return sendReceiptFetchFailAsync();

                        // Ignore products that were cancelled.
                        if (purchaseData[@"cancellation_date"] != nil)
                            continue;

                        if (auto transactionId = getAs<NSString> (purchaseData[@"original_transaction_id"]))
                        {
                            if (auto productId = getAs<NSString> (purchaseData[@"product_id"]))
                            {
                                auto purchaseTime = getPurchaseDateMs (purchaseData[@"purchase_date_ms"]);

                                if (purchaseTime <= 0)
                                    return sendReceiptFetchFailAsync();

                                purchases.add ({ { nsStringToJuce (transactionId),
                                                   nsStringToJuce (productId),
                                                   nsStringToJuce (bundleId),
                                                   Time (purchaseTime).toString (true, true, true, true),
                                                   {} }, {} });
                            }
                        }
                    }

                    MessageManager::callAsync ([this, purchases]
                    {
                        owner.listeners.call ([&] (Listener& l)
                        {
                            l.purchasesListRestored (purchases, true, NEEDS_TRANS ("Success"));
                        });
                    });

                    return;
                }
            }
        }

        sendReceiptFetchFailAsync();
    }

    void sendReceiptFetchFail()
    {
        owner.listeners.call ([] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Receipt fetch failed")); });
    }

    void sendReceiptFetchFailAsync()
    {
        MessageManager::callAsync ([this] { sendReceiptFetchFail(); });
    }

    static int64 getPurchaseDateMs (id date)
    {
        if (auto dateAsNumber = getAs<NSNumber> (date))
            return [dateAsNumber longLongValue];

        if (auto dateAsString = getAs<NSString> (date))
        {
            auto formatter = [[NSNumberFormatter alloc] init];
            [formatter setNumberStyle: NSNumberFormatterDecimalStyle];
            auto dateAsNumber = [formatter numberFromString: dateAsString];
            [formatter release];
            return [dateAsNumber longLongValue];
        }

        return -1;
    }

    //==============================================================================
    static Product SKProductToIAPProduct (SKProduct* skProduct)
    {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [numberFormatter setFormatterBehavior: NSNumberFormatterBehavior10_4];
        [numberFormatter setNumberStyle: NSNumberFormatterCurrencyStyle];
        [numberFormatter setLocale: skProduct.priceLocale];

        auto identifier   = nsStringToJuce (skProduct.productIdentifier);
        auto title        = nsStringToJuce (skProduct.localizedTitle);
        auto description  = nsStringToJuce (skProduct.localizedDescription);
        auto priceLocale  = nsStringToJuce ([skProduct.priceLocale objectForKey: NSLocaleLanguageCode]);
        auto price        = nsStringToJuce ([numberFormatter stringFromNumber: skProduct.price]);

        [numberFormatter release];

        return { identifier, title, description, price, priceLocale };
    }

    static String SKPaymentTransactionStateToString (SKPaymentTransactionState state)
    {
        switch (state)
        {
            case SKPaymentTransactionStatePurchasing: return NEEDS_TRANS ("Purchasing");
            case SKPaymentTransactionStatePurchased:  return NEEDS_TRANS ("Success");
            case SKPaymentTransactionStateFailed:     return NEEDS_TRANS ("Failure");
            case SKPaymentTransactionStateRestored:   return NEEDS_TRANS ("Restored");
            case SKPaymentTransactionStateDeferred:   return NEEDS_TRANS ("Deferred");
            default:                    jassertfalse; return NEEDS_TRANS ("Unknown status");
        }

    }

    static Download::Status SKDownloadStateToDownloadStatus (SKDownloadState state)
    {
        switch (state)
        {
            case SKDownloadStateWaiting:    return Download::Status::waiting;
            case SKDownloadStateActive:     return Download::Status::active;
            case SKDownloadStatePaused:     return Download::Status::paused;
            case SKDownloadStateFinished:   return Download::Status::finished;
            case SKDownloadStateFailed:     return Download::Status::failed;
            case SKDownloadStateCancelled:  return Download::Status::cancelled;
            default:          jassertfalse; return Download::Status::waiting;
        }
    }

    static NSArray<SKDownload*>* downloadsToSKDownloads (const Array<Download*>& downloads)
    {
        NSMutableArray<SKDownload*>* skDownloads = [NSMutableArray arrayWithCapacity: (NSUInteger) downloads.size()];

        for (const auto& d : downloads)
            if (auto impl = dynamic_cast<DownloadImpl*> (d))
                [skDownloads addObject: impl->download];

        return skDownloads;
    }

    template <typename ObjCType>
    static ObjCType* getAs (id o)
    {
        if (o == nil || ! [o isKindOfClass: [ObjCType class]])
            return nil;

        return (ObjCType*) o;
    }

private:
    auto findPendingProductInfoRequest (SKRequest* r) const
    {
        const auto request = getAs<SKProductsRequest> (r);
        return std::find_if (pendingProductInfoRequests.begin(),
                             pendingProductInfoRequests.end(),
                             [&] (const auto& pending) { return pending->request.get() == request; });
    }

    auto findPendingReceiptRefreshRequest (SKRequest* r) const
    {
        const auto request = getAs<SKReceiptRefreshRequest> (r);
        return std::find_if (pendingReceiptRefreshRequests.begin(),
                             pendingReceiptRefreshRequests.end(),
                             [&] (const auto& pending) { return pending->request.get() == request; });
    }

    struct Class final : public ObjCClass<NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>>
    {
        //==============================================================================
        Class()
            : ObjCClass ("SKDelegateAndPaymentObserverBase_")
        {
            addIvar<Pimpl*> ("self");

            addMethod (@selector (productsRequest:didReceiveResponse:), [] (id self, SEL, SKProductsRequest* request, SKProductsResponse* response)
            {
                auto& t = getThis (self);

                if (const auto iter = t.findPendingProductInfoRequest (request); iter != t.pendingProductInfoRequests.end())
                {
                    switch (iter->get()->type)
                    {
                        case PendingProductInfoRequest::Type::query:
                            t.notifyProductsInfoReceived (response.products);
                            break;
                        case PendingProductInfoRequest::Type::purchase:
                            t.startPurchase (response.products);
                            break;
                    }

                    t.pendingProductInfoRequests.erase (iter);
                }
                else
                {
                    // Unknown request received!
                    jassertfalse;
                }
            });

            addMethod (@selector (requestDidFinish:), [] (id self, SEL, SKRequest* request)
            {
                auto& t = getThis (self);

                if (const auto iter = t.findPendingReceiptRefreshRequest (request); iter != t.pendingReceiptRefreshRequests.end())
                {
                    t.processReceiptRefreshResponseWithSubscriptionsSharedSecret (iter->get()->subscriptionsSharedSecret);
                    t.pendingReceiptRefreshRequests.erase (iter);
                }

                if (const auto iter = t.findPendingProductInfoRequest (request); iter != t.pendingProductInfoRequests.end())
                {
                    t.pendingProductInfoRequests.erase (iter);
                }
            });

            addMethod (@selector (request:didFailWithError:), [] (id self, SEL, SKRequest* request, NSError* error)
            {
                auto& t = getThis (self);

                const auto errorDetails = [&]
                {
                    if (error == nil)
                        return String();

                    const auto description = nsStringToJuce ([error localizedDescription]);

                    if (description.isEmpty())
                        return String();

                    return ": " + description;
                }();

                if (const auto iter = t.findPendingReceiptRefreshRequest (request); iter != t.pendingReceiptRefreshRequests.end())
                {
                    t.owner.listeners.call ([&] (Listener& l)
                    {
                        l.purchasesListRestored ({}, false, NEEDS_TRANS ("Receipt fetch failed") + errorDetails);
                    });
                    t.pendingReceiptRefreshRequests.erase (iter);
                    return;
                }

                if (const auto iter = t.findPendingProductInfoRequest (request); iter != t.pendingProductInfoRequests.end())
                {
                    switch (iter->get()->type)
                    {
                        case PendingProductInfoRequest::Type::query:
                            t.owner.listeners.call ([&] (Listener& l)
                            {
                                l.purchasesListRestored ({}, false, NEEDS_TRANS ("Product query failed") + errorDetails);
                            });
                            break;
                        case PendingProductInfoRequest::Type::purchase:
                            t.owner.listeners.call ([&] (Listener& l)
                            {
                                l.productPurchaseFinished ({}, false, NEEDS_TRANS ("Purchase request failed") + errorDetails);
                            });
                            break;
                    }

                    t.pendingProductInfoRequests.erase (iter);
                    return;
                }
            });

            addMethod (@selector (paymentQueue:updatedTransactions:), [] (id self, SEL, SKPaymentQueue*, NSArray<SKPaymentTransaction*>* transactions)
            {
                auto& t = getThis (self);

                for (SKPaymentTransaction* transaction in transactions)
                {
                    switch (transaction.transactionState)
                    {
                        case SKPaymentTransactionStatePurchasing: break;
                        case SKPaymentTransactionStateDeferred:   break;
                        case SKPaymentTransactionStateFailed:     t.processTransactionFinish (transaction, false); break;
                        case SKPaymentTransactionStatePurchased:  t.processTransactionFinish (transaction, true);  break;
                        case SKPaymentTransactionStateRestored:   t.processTransactionFinish (transaction, true);  break;
                        default:                                  jassertfalse; break;  // Unexpected transaction state
                    }
                }
            });

            addMethod (@selector (paymentQueue:restoreCompletedTransactionsFailedWithError:), [] (id self, SEL, SKPaymentQueue*, NSError* error)
            {
                auto& t = getThis (self);
                t.owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, nsStringToJuce (error.localizedDescription)); });
            });

            addMethod (@selector (paymentQueueRestoreCompletedTransactionsFinished:), [] (id self, SEL, SKPaymentQueue*)
            {
                auto& t = getThis (self);
                t.owner.listeners.call ([&t] (Listener& l) { l.purchasesListRestored (t.restoredPurchases, true, NEEDS_TRANS ("Success")); });
                t.restoredPurchases.clear();
            });

            addMethod (@selector (paymentQueue:updatedDownloads:), [] (id self, SEL, SKPaymentQueue*, NSArray<SKDownload*>* downloads)
            {
                auto& t = getThis (self);

                for (SKDownload* download in downloads)
                {
                    if (auto* pendingDownload = t.getPendingDownloadFor (download))
                    {
                        switch (download.state)
                        {
                            case SKDownloadStateWaiting:
                                break;
                            case SKDownloadStatePaused:
                                t.owner.listeners.call ([&] (Listener& l) { l.productDownloadPaused (*pendingDownload); });
                                break;
                            case SKDownloadStateActive:
                                t.owner.listeners.call ([&] (Listener& l)
                                                        {
                                                            l.productDownloadProgressUpdate (*pendingDownload,
                                                                                             download.progress,
                                                                                             RelativeTime (download.timeRemaining));
                                                        });
                                break;
                            case SKDownloadStateFinished:
                            case SKDownloadStateFailed:
                            case SKDownloadStateCancelled:
                                t.processDownloadFinish (pendingDownload, download);
                                break;

                            default:
                                // Unexpected download state
                                jassertfalse;
                                break;
                        }
                    }
                }
            });

            registerClass();
        }

        //==============================================================================
        static Pimpl& getThis (id self)           { return *getIvar<Pimpl*> (self, "self"); }
        static void setThis (id self, Pimpl* s)   { object_setInstanceVariable (self, "self", s); }

        static Class& get()
        {
            static Class c;
            return c;
        }
    };

    //==============================================================================
    InAppPurchases& owner;

    NSUniquePtr<NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>> delegate { [Class::get().createInstance() init] };

    std::vector<std::unique_ptr<PendingProductInfoRequest>> pendingProductInfoRequests;
    std::vector<std::unique_ptr<PendingReceiptRefreshRequest>> pendingReceiptRefreshRequests;

    OwnedArray<PendingDownloadsTransaction> pendingDownloadsTransactions;
    Array<Listener::PurchaseInfo> restoredPurchases;
};

} // namespace juce
