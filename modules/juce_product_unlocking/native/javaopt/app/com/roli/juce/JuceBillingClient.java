package com.roli.juce;

import com.android.billingclient.api.*;

public class JuceBillingClient implements PurchasesUpdatedListener {
    private native void skuDetailsQueryCallback(long host, java.util.List<SkuDetails> skuDetails);
    private native void purchasesListQueryCallback(long host, java.util.List<Purchase> purchases);
    private native void purchaseCompletedCallback(long host, Purchase purchase, int responseCode);
    private native void purchaseConsumedCallback(long host, String productIdentifier, int responseCode);

    public JuceBillingClient(android.content.Context context, long hostToUse) {
        host = hostToUse;

        billingClient = BillingClient.newBuilder(context)
                .enablePendingPurchases()
                .setListener(this)
                .build();

        billingClient.startConnection(null);
    }

    public void endConnection() {
        billingClient.endConnection();
    }

    public boolean isReady() {
        return billingClient.isReady();
    }

    public boolean isBillingSupported() {
        return billingClient.isFeatureSupported(BillingClient.FeatureType.SUBSCRIPTIONS).getResponseCode()
                == BillingClient.BillingResponseCode.OK;
    }

    public void querySkuDetails(final String[] skusToQuery) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public void run() {
                final java.util.List<String> skuList = java.util.Arrays.asList(skusToQuery);

                SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder()
                        .setSkusList(skuList)
                        .setType(BillingClient.SkuType.INAPP);

                billingClient.querySkuDetailsAsync(params.build(), new SkuDetailsResponseListener() {
                    @Override
                    public void onSkuDetailsResponse(BillingResult billingResult, final java.util.List<SkuDetails> inAppSkuDetails) {
                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                            SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder()
                                    .setSkusList(skuList)
                                    .setType(BillingClient.SkuType.SUBS);

                            billingClient.querySkuDetailsAsync(params.build(), new SkuDetailsResponseListener() {
                                @Override
                                public void onSkuDetailsResponse(BillingResult billingResult, java.util.List<SkuDetails> subsSkuDetails) {
                                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                                        subsSkuDetails.addAll(inAppSkuDetails);
                                        skuDetailsQueryCallback(host, subsSkuDetails);
                                    }
                                }
                            });
                        }
                    }
                });
            }
        });
    }

    public void launchBillingFlow(final android.app.Activity activity, final BillingFlowParams params) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public void run() {
                BillingResult r = billingClient.launchBillingFlow(activity, params);
            }
        });
    }

    public void queryPurchases() {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public void run() {
                Purchase.PurchasesResult inAppPurchases = billingClient.queryPurchases(BillingClient.SkuType.INAPP);
                Purchase.PurchasesResult subsPurchases = billingClient.queryPurchases(BillingClient.SkuType.SUBS);

                if (inAppPurchases.getResponseCode() == BillingClient.BillingResponseCode.OK
                        && subsPurchases.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    java.util.List<Purchase> purchaseList = inAppPurchases.getPurchasesList();
                    purchaseList.addAll(subsPurchases.getPurchasesList());

                    purchasesListQueryCallback(host, purchaseList);
                    return;
                }

                purchasesListQueryCallback(host, null);
            }
        });
    }

    public void consumePurchase(final String productIdentifier, final String purchaseToken) {
        executeOnBillingClientConnection(new Runnable() {
            @Override
            public void run() {
                ConsumeParams consumeParams = ConsumeParams.newBuilder()
                        .setPurchaseToken(purchaseToken)
                        .build();

                billingClient.consumeAsync(consumeParams, new ConsumeResponseListener() {
                    @Override
                    public void onConsumeResponse(BillingResult billingResult, String purchaseToken) {
                        purchaseConsumedCallback(host, productIdentifier, billingResult.getResponseCode());
                    }
                });
            }
        });
    }

    @Override
    public void onPurchasesUpdated(BillingResult result, java.util.List<Purchase> purchases) {
        int responseCode = result.getResponseCode();

        if (purchases != null) {
            for (Purchase purchase : purchases) {
                handlePurchase(purchase, responseCode);
            }
        } else {
            purchaseCompletedCallback(host, null, responseCode);
        }
    }

    private void executeOnBillingClientConnection(Runnable runnable) {
        if (billingClient.isReady()) {
            runnable.run();
        } else {
            connectAndExecute(runnable);
        }
    }

    private void connectAndExecute(final Runnable executeOnSuccess) {
        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResponse) {
                if (billingResponse.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                    if (executeOnSuccess != null) {
                        executeOnSuccess.run();
                    }
                }
            }

            @Override
            public void onBillingServiceDisconnected() {
            }
        });
    }

    private void handlePurchase(final Purchase purchase, final int responseCode) {
        purchaseCompletedCallback(host, purchase, responseCode);

        if (responseCode == BillingClient.BillingResponseCode.OK
                && purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED
                && !purchase.isAcknowledged()) {
            executeOnBillingClientConnection(new Runnable() {
                @Override
                public void run() {
                    AcknowledgePurchaseParams acknowledgePurchaseParams = AcknowledgePurchaseParams.newBuilder().setPurchaseToken(purchase.getPurchaseToken()).build();
                    billingClient.acknowledgePurchase(acknowledgePurchaseParams, null);
                }
            });
        }
    }

    private long host = 0;
    private BillingClient billingClient;
}
