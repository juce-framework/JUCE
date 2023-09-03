namespace juce {
    void MessageManager::doPlatformSpecificShutdown() {}
    bool detail::dispatchNextMessageOnSystemQueue(bool returnIfNoPendingMessages) {
        return false;
    }
}