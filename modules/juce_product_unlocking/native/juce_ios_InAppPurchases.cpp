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

struct SKDelegateAndPaymentObserver
{
    SKDelegateAndPaymentObserver()  : delegate ([getClass().createInstance() init])
    {
        Class::setThis (delegate.get(), this);
    }

    virtual ~SKDelegateAndPaymentObserver() {}

    virtual void didReceiveResponse (SKProductsRequest*, SKProductsResponse*) = 0;
    virtual void requestDidFinish (SKRequest*) = 0;
    virtual void requestDidFailWithError (SKRequest*, NSError*) = 0;
    virtual void updatedTransactions (SKPaymentQueue*, NSArray<SKPaymentTransaction*>*) = 0;
    virtual void restoreCompletedTransactionsFailedWithError (SKPaymentQueue*, NSError*) = 0;
    virtual void restoreCompletedTransactionsFinished (SKPaymentQueue*) = 0;
    virtual void updatedDownloads (SKPaymentQueue*, NSArray<SKDownload*>*) = 0;

protected:
    std::unique_ptr<NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>, NSObjectDeleter> delegate;

private:
    struct Class   : public ObjCClass<NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>>
    {
        //==============================================================================
        Class()  : ObjCClass<NSObject<SKProductsRequestDelegate, SKPaymentTransactionObserver>> ("SKDelegateAndPaymentObserverBase_")
        {
            addIvar<SKDelegateAndPaymentObserver*> ("self");

            addMethod (@selector (productsRequest:didReceiveResponse:),                       didReceiveResponse,                          "v@:@@");
            addMethod (@selector (requestDidFinish:),                                         requestDidFinish,                            "v@:@");
            addMethod (@selector (request:didFailWithError:),                                 requestDidFailWithError,                     "v@:@@");
            addMethod (@selector (paymentQueue:updatedTransactions:),                         updatedTransactions,                         "v@:@@");
            addMethod (@selector (paymentQueue:restoreCompletedTransactionsFailedWithError:), restoreCompletedTransactionsFailedWithError, "v@:@@");
            addMethod (@selector (paymentQueueRestoreCompletedTransactionsFinished:),         restoreCompletedTransactionsFinished,        "v@:@");
            addMethod (@selector (paymentQueue:updatedDownloads:),                            updatedDownloads,                            "v@:@@");

            registerClass();
        }

        //==============================================================================
        static SKDelegateAndPaymentObserver& getThis (id self)           { return *getIvar<SKDelegateAndPaymentObserver*> (self, "self"); }
        static void setThis (id self, SKDelegateAndPaymentObserver* s)   { object_setInstanceVariable (self, "self", s); }

        //==============================================================================
        static void didReceiveResponse (id self, SEL, SKProductsRequest* request, SKProductsResponse* response)      { getThis (self).didReceiveResponse (request, response); }
        static void requestDidFinish (id self, SEL, SKRequest* request)                                              { getThis (self).requestDidFinish (request); }
        static void requestDidFailWithError (id self, SEL, SKRequest* request, NSError* err)                         { getThis (self).requestDidFailWithError (request, err); }
        static void updatedTransactions (id self, SEL, SKPaymentQueue* queue, NSArray<SKPaymentTransaction*>* trans) { getThis (self).updatedTransactions (queue, trans); }
        static void restoreCompletedTransactionsFailedWithError (id self, SEL, SKPaymentQueue* q, NSError* err)      { getThis (self).restoreCompletedTransactionsFailedWithError (q, err); }
        static void restoreCompletedTransactionsFinished (id self, SEL, SKPaymentQueue* queue)                       { getThis (self).restoreCompletedTransactionsFinished (queue); }
        static void updatedDownloads (id self, SEL, SKPaymentQueue* queue, NSArray<SKDownload*>* downloads)          { getThis (self).updatedDownloads (queue, downloads); }
    };

    //==============================================================================
    static Class& getClass()
    {
        static Class c;
        return c;
    }
};

//==============================================================================
struct InAppPurchases::Pimpl   : public SKDelegateAndPaymentObserver
{
    /** AppStore implementation of hosted content download. */
    struct DownloadImpl  : public Download
    {
        DownloadImpl (SKDownload* downloadToUse)  : download (downloadToUse) {}

        String getProductId()      const override  { return nsStringToJuce (download.contentIdentifier); }
        String getContentVersion() const override  { return nsStringToJuce (download.contentVersion); }

      #if JUCE_IOS
        int64 getContentLength()   const override  { return download.contentLength; }
        Status getStatus()         const override  { return SKDownloadStateToDownloadStatus (download.downloadState); }
      #else
        int64 getContentLength()   const override  { return [download.contentLength longLongValue]; }
        Status getStatus()         const override  { return SKDownloadStateToDownloadStatus (download.state); }
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
        std::unique_ptr<SKProductsRequest, NSObjectDeleter> request;
    };

    /** Represents a pending request started from [SKReceiptRefreshRequest start]. */
    struct PendingReceiptRefreshRequest
    {
        String subscriptionsSharedSecret;
        std::unique_ptr<SKReceiptRefreshRequest, NSObjectDeleter> request;
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
              #if JUCE_IOS
                SKDownloadState state = d.downloadState;
              #else
                SKDownloadState state = d.state;
              #endif
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
    Pimpl (InAppPurchases& p) : owner (p)  { [[SKPaymentQueue defaultQueue] addTransactionObserver:    delegate.get()]; }
    ~Pimpl() noexcept                      { [[SKPaymentQueue defaultQueue] removeTransactionObserver: delegate.get()]; }

    //==============================================================================
    bool isInAppPurchasesSupported() const     { return true; }

    void getProductsInformation (const StringArray& productIdentifiers)
    {
        auto* productsRequest = [[SKProductsRequest alloc] initWithProductIdentifiers: [NSSet setWithArray: createNSArrayFromStringArray (productIdentifiers)]];

        pendingProductInfoRequests.add (new PendingProductInfoRequest { PendingProductInfoRequest::Type::query,
                                                                        std::unique_ptr<SKProductsRequest, NSObjectDeleter> (productsRequest) });

        productsRequest.delegate = delegate.get();
        [productsRequest start];
    }

    void purchaseProduct (const String& productIdentifier, bool, const StringArray&, bool)
    {
        if (! [SKPaymentQueue canMakePayments])
        {
            owner.listeners.call ([&] (Listener& l) { l.productPurchaseFinished ({}, false, NEEDS_TRANS ("Payments not allowed")); });
            return;
        }

        auto* productIdentifiers = [NSArray arrayWithObject: juceStringToNS (productIdentifier)];
        auto* productsRequest    = [[SKProductsRequest alloc] initWithProductIdentifiers:[NSSet setWithArray:productIdentifiers]];

        pendingProductInfoRequests.add (new PendingProductInfoRequest { PendingProductInfoRequest::Type::purchase,
                                                                        std::unique_ptr<SKProductsRequest, NSObjectDeleter> (productsRequest) });

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
            auto* receiptRequest = [[SKReceiptRefreshRequest alloc] init];

            pendingReceiptRefreshRequests.add (new PendingReceiptRefreshRequest { subscriptionsSharedSecret,
                                                                                  std::unique_ptr<SKReceiptRefreshRequest, NSObjectDeleter> ([receiptRequest retain]) });
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
    void didReceiveResponse (SKProductsRequest* request, SKProductsResponse* response) override
    {
        for (auto i = 0; i < pendingProductInfoRequests.size(); ++i)
        {
            auto& pendingRequest = *pendingProductInfoRequests[i];

            if (pendingRequest.request.get() == request)
            {
                if      (pendingRequest.type == PendingProductInfoRequest::Type::query)    notifyProductsInfoReceived (response.products);
                else if (pendingRequest.type == PendingProductInfoRequest::Type::purchase) startPurchase (response.products);
                else break;

                pendingProductInfoRequests.remove (i);
                return;
            }
        }

        // Unknown request received!
        jassertfalse;
    }

    void requestDidFinish (SKRequest* request) override
    {
        if (auto receiptRefreshRequest = getAs<SKReceiptRefreshRequest> (request))
        {
            for (auto i = 0; i < pendingReceiptRefreshRequests.size(); ++i)
            {
                auto& pendingRequest = *pendingReceiptRefreshRequests[i];

                if (pendingRequest.request.get() == receiptRefreshRequest)
                {
                    processReceiptRefreshResponseWithSubscriptionsSharedSecret (pendingRequest.subscriptionsSharedSecret);
                    pendingReceiptRefreshRequests.remove (i);
                    return;
                }
            }
        }
    }

    void requestDidFailWithError (SKRequest* request, NSError* error) override
    {
        if (auto receiptRefreshRequest = getAs<SKReceiptRefreshRequest> (request))
        {
            for (auto i = 0; i < pendingReceiptRefreshRequests.size(); ++i)
            {
                auto& pendingRequest = *pendingReceiptRefreshRequests[i];

                if (pendingRequest.request.get() == receiptRefreshRequest)
                {
                    auto errorDetails = error != nil ? (", " + nsStringToJuce ([error localizedDescription])) : String();
                    owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Receipt fetch failed") + errorDetails); });
                    pendingReceiptRefreshRequests.remove (i);
                    return;
                }
            }
        }
    }

    void updatedTransactions (SKPaymentQueue*, NSArray<SKPaymentTransaction*>* transactions) override
    {
        for (SKPaymentTransaction* transaction in transactions)
        {
            switch (transaction.transactionState)
            {
                case SKPaymentTransactionStatePurchasing: break;
                case SKPaymentTransactionStateDeferred:   break;
                case SKPaymentTransactionStateFailed:     processTransactionFinish (transaction, false); break;
                case SKPaymentTransactionStatePurchased:  processTransactionFinish (transaction, true);  break;
                case SKPaymentTransactionStateRestored:   processTransactionFinish (transaction, true);  break;
                default:                                  jassertfalse; break;  // Unexpected transaction state
            }
        }
    }

    void restoreCompletedTransactionsFailedWithError (SKPaymentQueue*, NSError* error) override
    {
        owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, nsStringToJuce (error.localizedDescription)); });
    }

    void restoreCompletedTransactionsFinished (SKPaymentQueue*) override
    {
        owner.listeners.call ([this] (Listener& l) { l.purchasesListRestored (restoredPurchases, true, NEEDS_TRANS ("Success")); });
        restoredPurchases.clear();
    }

    void updatedDownloads (SKPaymentQueue*, NSArray<SKDownload*>* downloads) override
    {
        for (SKDownload* download in downloads)
        {
            if (auto* pendingDownload = getPendingDownloadFor (download))
            {
              #if JUCE_IOS
                switch (download.downloadState)
              #else
                switch (download.state)
              #endif
                {
                    case SKDownloadStateWaiting: break;
                    case SKDownloadStatePaused:  owner.listeners.call ([&] (Listener& l) { l.productDownloadPaused (*pendingDownload); }); break;
                    case SKDownloadStateActive:  owner.listeners.call ([&] (Listener& l) { l.productDownloadProgressUpdate (*pendingDownload,
                                                                                                                            download.progress,
                                                                                                                            RelativeTime (download.timeRemaining)); }); break;
                    case SKDownloadStateFinished:
                    case SKDownloadStateFailed:
                    case SKDownloadStateCancelled: processDownloadFinish (pendingDownload, download); break;

                    default:  jassertfalse; break;  // Unexpected download state
                }
            }
        }
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
            auto* payment = [SKPayment paymentWithProduct: product];
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
          #if JUCE_IOS
            SKDownloadState state = download.downloadState;
          #else
            SKDownloadState state = download.state;
          #endif

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
        auto* receiptURL = [[NSBundle mainBundle] appStoreReceiptURL];

        if (auto* receiptData = [NSData dataWithContentsOfURL: receiptURL])
            fetchReceiptDetailsFromAppStore (receiptData, secret);
        else
            owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored ({}, false, NEEDS_TRANS ("Receipt fetch failed")); });
    }

    void fetchReceiptDetailsFromAppStore (NSData* receiptData, const String& secret)
    {
        auto* requestContents = [NSMutableDictionary dictionaryWithCapacity: (NSUInteger) (secret.isNotEmpty() ? 2 : 1)];
        [requestContents setObject: [receiptData base64EncodedStringWithOptions:0] forKey: nsStringLiteral ("receipt-data")];

        if (secret.isNotEmpty())
            [requestContents setObject: juceStringToNS (secret) forKey: nsStringLiteral ("password")];

        NSError* error;
        auto* requestData = [NSJSONSerialization dataWithJSONObject: requestContents
                                                            options: 0
                                                              error: &error];
        if (requestData == nil)
        {
            sendReceiptFetchFail();
            return;
        }

       #if JUCE_IN_APP_PURCHASES_USE_SANDBOX_ENVIRONMENT
        auto storeURL = "https://sandbox.itunes.apple.com/verifyReceipt";
       #else
        auto storeURL = "https://buy.itunes.apple.com/verifyReceipt";
       #endif

        // TODO: use juce URL here
        auto* storeRequest = [NSMutableURLRequest requestWithURL: [NSURL URLWithString: nsStringLiteral (storeURL)]];
        [storeRequest setHTTPMethod: nsStringLiteral ("POST")];
        [storeRequest setHTTPBody: requestData];

        auto* task = [[NSURLSession sharedSession] dataTaskWithRequest: storeRequest
                                                     completionHandler:
                                                        ^(NSData* data, NSURLResponse*, NSError* connectionError)
                                                        {
                                                            if (connectionError != nil)
                                                            {
                                                                sendReceiptFetchFail();
                                                            }
                                                            else
                                                            {
                                                                NSError* err;

                                                                if (NSDictionary* receiptDetails = [NSJSONSerialization JSONObjectWithData: data options: 0 error: &err])
                                                                    processReceiptDetails (receiptDetails);
                                                                else
                                                                    sendReceiptFetchFail();
                                                            }
                                                        }];

        [task resume];
    }

    void processReceiptDetails (NSDictionary* receiptDetails)
    {
        if (auto receipt = getAs<NSDictionary> (receiptDetails[nsStringLiteral ("receipt")]))
        {
            if (auto bundleId = getAs<NSString> (receipt[nsStringLiteral ("bundle_id")]))
            {
                if (auto inAppPurchases = getAs<NSArray> (receipt[nsStringLiteral ("in_app")]))
                {
                    Array<Listener::PurchaseInfo> purchases;

                    for (id inAppPurchaseData in inAppPurchases)
                    {
                        if (auto* purchaseData = getAs<NSDictionary> (inAppPurchaseData))
                        {
                            // Ignore products that were cancelled.
                            if (purchaseData[nsStringLiteral ("cancellation_date")] != nil)
                                continue;

                            if (auto transactionId = getAs<NSString> (purchaseData[nsStringLiteral ("original_transaction_id")]))
                            {
                                if (auto productId = getAs<NSString> (purchaseData[nsStringLiteral ("product_id")]))
                                {
                                    auto purchaseTime = getPurchaseDateMs (purchaseData[nsStringLiteral ("purchase_date_ms")]);

                                    if (purchaseTime > 0)
                                    {
                                        purchases.add ({ { nsStringToJuce (transactionId),
                                                           nsStringToJuce (productId),
                                                           nsStringToJuce (bundleId),
                                                           Time (purchaseTime).toString (true, true, true, true),
                                                           {} }, {} });
                                    }
                                    else
                                    {
                                        return sendReceiptFetchFailAsync();
                                    }
                                }
                            }
                        }
                        else
                        {
                            return sendReceiptFetchFailAsync();
                        }
                    }

                    MessageManager::callAsync ([this, purchases] { owner.listeners.call ([&] (Listener& l) { l.purchasesListRestored (purchases, true, NEEDS_TRANS ("Success")); }); });
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
        {
            return [dateAsNumber longLongValue];
        }
        else if (auto dateAsString = getAs<NSString> (date))
        {
            auto* formatter = [[NSNumberFormatter alloc] init];
            [formatter setNumberStyle: NSNumberFormatterDecimalStyle];
            dateAsNumber = [formatter numberFromString: dateAsString];
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
            default:                                  jassertfalse; return NEEDS_TRANS ("Unknown status");
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
            default:                        jassertfalse; return Download::Status::waiting;
        }
    }

    static NSArray<SKDownload*>* downloadsToSKDownloads (const Array<Download*>& downloads)
    {
        NSMutableArray<SKDownload*>* skDownloads = [NSMutableArray arrayWithCapacity: (NSUInteger) downloads.size()];

        for (const auto& d : downloads)
            if (auto impl = dynamic_cast<DownloadImpl*>(d))
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

    //==============================================================================
    InAppPurchases& owner;

    OwnedArray<PendingProductInfoRequest> pendingProductInfoRequests;
    OwnedArray<PendingReceiptRefreshRequest> pendingReceiptRefreshRequests;

    OwnedArray<PendingDownloadsTransaction> pendingDownloadsTransactions;
    Array<Listener::PurchaseInfo> restoredPurchases;
};

} // namespace juce
