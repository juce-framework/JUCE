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

AudioProcessorEditor::AudioProcessorEditor (AudioProcessor& p) noexcept  : processor (p)
{
    initialise();
}

AudioProcessorEditor::AudioProcessorEditor (AudioProcessor* p) noexcept  : processor (*p)
{
    // the filter must be valid..
    jassert (p != nullptr);
    initialise();
}

AudioProcessorEditor::~AudioProcessorEditor()
{
    splashScreen.deleteAndZero();

    // if this fails, then the wrapper hasn't called editorBeingDeleted() on the
    // filter for some reason..
    jassert (processor.getActiveEditor() != this);
    removeComponentListener (resizeListener.get());
}

void AudioProcessorEditor::setControlHighlight (ParameterControlHighlightInfo) {}
int AudioProcessorEditor::getControlParameterIndex (Component&)                { return -1; }

bool AudioProcessorEditor::supportsHostMIDIControllerPresence (bool)           { return true; }
void AudioProcessorEditor::hostMIDIControllerIsAvailable (bool)                {}

void AudioProcessorEditor::initialise()
{
    /*
      ==========================================================================
       In accordance with the terms of the JUCE 7 End-Use License Agreement, the
       JUCE Code in SECTION A cannot be removed, changed or otherwise rendered
       ineffective unless you have a JUCE Indie or Pro license, or are using
       JUCE under the GPL v3 license.

       End User License Agreement: www.juce.com/juce-7-licence
      ==========================================================================
    */

    // BEGIN SECTION A

    splashScreen = new JUCESplashScreen (*this);

    // END SECTION A

    attachConstrainer (&defaultConstrainer);
    resizeListener.reset (new AudioProcessorEditorListener (*this));
    addComponentListener (resizeListener.get());
}

//==============================================================================
void AudioProcessorEditor::setResizable (bool allowHostToResize, bool useBottomRightCornerResizer)
{
    resizableByHost = allowHostToResize;

    const auto hasResizableCorner = (resizableCorner.get() != nullptr);

    if (useBottomRightCornerResizer != hasResizableCorner)
    {
        if (useBottomRightCornerResizer)
            attachResizableCornerComponent();
        else
            resizableCorner = nullptr;
    }
}

void AudioProcessorEditor::setResizeLimits (int newMinimumWidth,
                                            int newMinimumHeight,
                                            int newMaximumWidth,
                                            int newMaximumHeight) noexcept
{
    if (constrainer != nullptr && constrainer != &defaultConstrainer)
    {
        // if you've set up a custom constrainer then these settings won't have any effect..
        jassertfalse;
        return;
    }

    resizableByHost = (newMinimumWidth != newMaximumWidth || newMinimumHeight != newMaximumHeight);

    defaultConstrainer.setSizeLimits (newMinimumWidth, newMinimumHeight,
                                      newMaximumWidth, newMaximumHeight);

    if (constrainer == nullptr)
        setConstrainer (&defaultConstrainer);

    if (resizableCorner != nullptr)
        attachResizableCornerComponent();

    setBoundsConstrained (getBounds());
}

void AudioProcessorEditor::setConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        attachConstrainer (newConstrainer);

        if (constrainer != nullptr)
            resizableByHost = (newConstrainer->getMinimumWidth() != newConstrainer->getMaximumWidth()
                                || newConstrainer->getMinimumHeight() != newConstrainer->getMaximumHeight());

        if (resizableCorner != nullptr)
            attachResizableCornerComponent();
    }
}

void AudioProcessorEditor::attachConstrainer (ComponentBoundsConstrainer* newConstrainer)
{
    if (constrainer != newConstrainer)
    {
        constrainer = newConstrainer;
        updatePeer();
    }
}

void AudioProcessorEditor::attachResizableCornerComponent()
{
    resizableCorner = std::make_unique<ResizableCornerComponent> (this, constrainer);
    Component::addChildComponent (resizableCorner.get());
    resizableCorner->setAlwaysOnTop (true);
    editorResized (true);
}

void AudioProcessorEditor::setBoundsConstrained (Rectangle<int> newBounds)
{
    if (constrainer == nullptr)
    {
        setBounds (newBounds);
        return;
    }

    auto currentBounds = getBounds();

    constrainer->setBoundsForComponent (this,
                                        newBounds,
                                        newBounds.getY() != currentBounds.getY() && newBounds.getBottom() == currentBounds.getBottom(),
                                        newBounds.getX() != currentBounds.getX() && newBounds.getRight()  == currentBounds.getRight(),
                                        newBounds.getY() == currentBounds.getY() && newBounds.getBottom() != currentBounds.getBottom(),
                                        newBounds.getX() == currentBounds.getX() && newBounds.getRight()  != currentBounds.getRight());
}

void AudioProcessorEditor::editorResized (bool wasResized)
{
    // The host needs to be able to rescale the plug-in editor and applying your own transform will
    // obliterate it! If you want to scale the whole of your UI use Desktop::setGlobalScaleFactor(),
    // or, for applying other transforms, consider putting the component you want to transform
    // in a child of the editor and transform that instead.
    jassert (getTransform() == hostScaleTransform);

    if (wasResized)
    {
        bool resizerHidden = false;

        if (auto* peer = getPeer())
            resizerHidden = peer->isFullScreen() || peer->isKioskMode();

        if (resizableCorner != nullptr)
        {
            resizableCorner->setVisible (! resizerHidden);

            const int resizerSize = 18;
            resizableCorner->setBounds (getWidth() - resizerSize,
                                        getHeight() - resizerSize,
                                        resizerSize, resizerSize);
        }
    }
}

void AudioProcessorEditor::updatePeer()
{
    if (isOnDesktop())
        if (auto* peer = getPeer())
            peer->setConstrainer (constrainer);
}

void AudioProcessorEditor::setScaleFactor (float newScale)
{
    hostScaleTransform = AffineTransform::scale (newScale);
    setTransform (hostScaleTransform);

    editorResized (true);
}

//==============================================================================
typedef ComponentPeer* (*createUnityPeerFunctionType) (Component&);
createUnityPeerFunctionType juce_createUnityPeerFn = nullptr;

ComponentPeer* AudioProcessorEditor::createNewPeer ([[maybe_unused]] int styleFlags,
                                                    [[maybe_unused]] void* nativeWindow)
{
    if (juce_createUnityPeerFn != nullptr)
        return juce_createUnityPeerFn (*this);

    return Component::createNewPeer (styleFlags, nativeWindow);
}

bool AudioProcessorEditor::wantsLayerBackedView() const
{
   #if JUCE_MODULE_AVAILABLE_juce_opengl && JUCE_MAC
    if (@available (macOS 10.14, *))
        return true;

    return false;
   #else
    return true;
   #endif
}

} // namespace juce
