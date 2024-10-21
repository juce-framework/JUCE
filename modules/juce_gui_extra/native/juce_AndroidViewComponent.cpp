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

class AndroidViewComponent::Pimpl  : public ComponentMovementWatcher
{
public:
    Pimpl (const LocalRef<jobject>& v, Component& comp)
        : ComponentMovementWatcher (&comp),
          view (v),
          owner (comp)
    {
        if (owner.isShowing())
            componentPeerChanged();
    }

    ~Pimpl() override
    {
        removeFromParent();
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        auto* topComp = owner.getTopLevelComponent();

        if (topComp->getPeer() != nullptr)
        {
            auto pos = topComp->getLocalPoint (&owner, Point<int>());

            Rectangle<int> r (pos.x, pos.y, owner.getWidth(), owner.getHeight());
            r *= Desktop::getInstance().getDisplays().getPrimaryDisplay()->scale;

            getEnv()->CallVoidMethod (view, AndroidView.layout, r.getX(), r.getY(),
                                      r.getRight(), r.getBottom());
        }
    }

    void componentPeerChanged() override
    {
        auto* peer = owner.getPeer();

        if (currentPeer != peer)
        {
            removeFromParent();
            currentPeer = peer;

            addToParent();
        }

        enum
        {
            VISIBLE   = 0,
            INVISIBLE = 4
        };

        getEnv()->CallVoidMethod (view, AndroidView.setVisibility, owner.isShowing() ? VISIBLE : INVISIBLE);
     }

    void componentVisibilityChanged() override
    {
        componentPeerChanged();
    }

    void componentBroughtToFront (Component& comp) override
    {
        ComponentMovementWatcher::componentBroughtToFront (comp);
    }

    Rectangle<int> getViewBounds() const
    {
        auto* env = getEnv();

        int width  = env->CallIntMethod (view, AndroidView.getWidth);
        int height = env->CallIntMethod (view, AndroidView.getHeight);

        return Rectangle<int> (width, height);
    }

    GlobalRef view;

private:
    void addToParent()
    {
        if (currentPeer != nullptr)
        {
            jobject peerView = (jobject) currentPeer->getNativeHandle();

            // NB: Assuming a parent is always of ViewGroup type
            auto* env = getEnv();

            env->CallVoidMethod (peerView, AndroidViewGroup.addView, view.get());
            componentMovedOrResized (false, false);
        }
    }

    void removeFromParent()
    {
        auto* env = getEnv();
        auto parentView = env->CallObjectMethod (view, AndroidView.getParent);

        if (parentView != nullptr)
        {
            // Assuming a parent is always of ViewGroup type
            env->CallVoidMethod (parentView, AndroidViewGroup.removeView, view.get());
        }
    }

    Component& owner;
    ComponentPeer* currentPeer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
AndroidViewComponent::AndroidViewComponent()
{
}

AndroidViewComponent::~AndroidViewComponent()
{
    AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
}

void AndroidViewComponent::setView (void* view)
{
    if (view != getView())
    {
        pimpl.reset();

        if (view != nullptr)
        {
            // explicitly create a new local ref here so that we don't
            // delete the users pointer
            auto* env = getEnv();
            auto localref = LocalRef<jobject> (env->NewLocalRef ((jobject) view));

            pimpl.reset (new Pimpl (localref, *this));

            AccessibilityHandler::setNativeChildForComponent (*this, getView());
        }
        else
        {
            AccessibilityHandler::setNativeChildForComponent (*this, nullptr);
        }
    }
}

void* AndroidViewComponent::getView() const
{
    return pimpl == nullptr ? nullptr : (void*) pimpl->view;
}

void AndroidViewComponent::resizeToFitView()
{
    if (pimpl != nullptr)
        setBounds (pimpl->getViewBounds());
}

void AndroidViewComponent::paint (Graphics&) {}

std::unique_ptr<AccessibilityHandler> AndroidViewComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
