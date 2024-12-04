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
