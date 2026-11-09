/*!
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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
declare class JuceInitialisationData {
    __juce__platform: string[];
    __juce__functions: string[];
    __juce__registeredGlobalEventIds: string[];
    __juce__sliders: string[];
    __juce__toggles: string[];
    __juce__comboBoxes: string[];
}
type Listener = (_: unknown) => void;
declare class ListenerList {
    listeners: Map<number, Listener>;
    listenerId: number;
    addListener(fn: Listener): number;
    removeListener(id: number): void;
    callListeners(payload: unknown): void;
}
type ListenerHandle = [eventId: string, id: number];
declare class EventListenerList {
    eventListeners: Map<string, ListenerList>;
    addEventListener(eventId: string, fn: (payload: unknown) => void): ListenerHandle;
    removeEventListener([eventId, id]: ListenerHandle): void;
    emitEvent(eventId: string, object: unknown): void;
}
declare class Backend {
    listeners: EventListenerList;
    /**
     * Registers a function to listen to events emitted by the backend. The backend can
     * emit such events by calling WebBrowserComponent::emitEventIfBrowserIsVisible.
     *
     * @param eventId The identifier of the emitted event.
     * @param fn      The function to be called when the event is received.
     */
    addEventListener(eventId: string, fn: (payload: unknown) => void): ListenerHandle;
    removeEventListener(handle: ListenerHandle): void;
    /**
     * Sends an event from the frontend to the native backend.
     *
     * @param eventId The identifier of the event to emit.
     * @param object  The payload to send along with the event.
     */
    emitEvent(eventId: string, object: unknown): void;
    /**
     * Internal function called by the JUCE backend implementation.
     *
     * User Javascript code running inside the WebView should not call this
     * function.
     *
     * @param eventId The identifier of the event to emit.
     * @param object  The JSON-encoded payload to send along with the event.
     */
    emitByBackend(eventId: string, object: string): void;
}
interface JuceGlobal {
    postMessage: (message: string) => void;
    initialisationData: JuceInitialisationData;
    backend: Backend;
    getAndroidUserScripts?: () => string;
}
declare global {
    interface Window {
        __JUCE__: JuceGlobal;
        inAndroidUserScriptEval?: true;
    }
}
export {};
//# sourceMappingURL=check_native_interop.d.ts.map