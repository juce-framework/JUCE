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

class JuceInitialisationData {
  __juce__platform: string[] = [];
  __juce__functions: string[] = [];
  __juce__registeredGlobalEventIds: string[] = [];
  __juce__sliders: string[] = [];
  __juce__toggles: string[] = [];
  __juce__comboBoxes: string[] = [];
}

type Listener = (_: unknown) => void;

class ListenerList {
  listeners: Map<number, Listener> = new Map();
  listenerId: number = 0;

  addListener(fn: Listener): number {
    const newListenerId = this.listenerId++;
    this.listeners.set(newListenerId, fn);
    return newListenerId;
  }

  removeListener(id: number): void {
    if (this.listeners.has(id)) {
      this.listeners.delete(id);
    }
  }

  callListeners(payload: unknown): void {
    for (const [, value] of this.listeners) {
      value(payload);
    }
  }
}

type ListenerHandle = [eventId: string, id: number];

class EventListenerList {
  eventListeners: Map<string, ListenerList> = new Map();

  addEventListener(
    eventId: string,
    fn: (payload: unknown) => void,
  ): ListenerHandle {
    let list = this.eventListeners.get(eventId);
    if (!list) {
      list = new ListenerList();
      this.eventListeners.set(eventId, list);
    }

    const id = list.addListener(fn);

    return [eventId, id];
  }

  removeEventListener([eventId, id]: ListenerHandle): void {
    this.eventListeners.get(eventId)?.removeListener(id);
  }

  emitEvent(eventId: string, object: unknown): void {
    this.eventListeners.get(eventId)?.callListeners(object);
  }
}

class Backend {
  listeners: EventListenerList = new EventListenerList();

  /**
   * Registers a function to listen to events emitted by the backend. The backend can
   * emit such events by calling WebBrowserComponent::emitEventIfBrowserIsVisible.
   *
   * @param eventId The identifier of the emitted event.
   * @param fn      The function to be called when the event is received.
   */
  addEventListener(
    eventId: string,
    fn: (payload: unknown) => void,
  ): ListenerHandle {
    return this.listeners.addEventListener(eventId, fn);
  }

  removeEventListener(handle: ListenerHandle): void {
    this.listeners.removeEventListener(handle);
  }

  /**
   * Sends an event from the frontend to the native backend.
   *
   * @param eventId The identifier of the event to emit.
   * @param object  The payload to send along with the event.
   */
  emitEvent(eventId: string, object: unknown): void {
    window.__JUCE__.postMessage(
      JSON.stringify({ eventId: eventId, payload: object }),
    );
  }

  /**
   * Internal function called by the JUCE backend implementation.
   *
   * User Javascript code running inside the WebView should not call this
   * function.
   *
   * @param eventId The identifier of the event to emit.
   * @param object  The JSON-encoded payload to send along with the event.
   */
  emitByBackend(eventId: string, object: string): void {
    this.listeners.emitEvent(eventId, JSON.parse(object));
  }
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

if (
  typeof window.__JUCE__ !== 'undefined' &&
  typeof window.__JUCE__.getAndroidUserScripts !== 'undefined' &&
  typeof window.inAndroidUserScriptEval === 'undefined'
) {
  window.inAndroidUserScriptEval = true;
  (0, eval)(window.__JUCE__.getAndroidUserScripts());
  delete window.inAndroidUserScriptEval;
}

if (typeof window.__JUCE__ === 'undefined') {
  console.warn(
    "The 'window.__JUCE__' object is undefined." +
      ' Native integration features will not work.' +
      " Defining a placeholder 'window.__JUCE__' object.",
  );

  window.__JUCE__ = {} as JuceGlobal;
}

if (typeof window.__JUCE__.postMessage === 'undefined') {
  window.__JUCE__.postMessage = function () {};
}

if (typeof window.__JUCE__.initialisationData === 'undefined') {
  window.__JUCE__.initialisationData = new JuceInitialisationData();
}

if (typeof window.__JUCE__.backend === 'undefined') {
  window.__JUCE__.backend = new Backend();
}
