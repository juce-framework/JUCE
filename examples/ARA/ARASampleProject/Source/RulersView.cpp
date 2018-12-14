#include "RulersView.h"

//==============================================================================
RulersView::RulersView (ARADocument* doc)
    : document (doc),
      musicalContext (nullptr)
{
    document->addListener (this);
}

RulersView::~RulersView()
{
    detachFromMusicalContext();
    detachFromDocument();
}

void RulersView::detachFromDocument()
{
    if (document == nullptr)
        return;

    document->removeListener (this);

    document = nullptr;
}

void RulersView::detachFromMusicalContext()
{
    if (musicalContext == nullptr)
        return;

    musicalContext->removeListener (this);

    musicalContext = nullptr;
}

//==============================================================================
void RulersView::paint (juce::Graphics& g)
{
    if (musicalContext == nullptr)
    {
        if (document->getMusicalContexts().empty())
            return;

        musicalContext = static_cast<ARAMusicalContext*> (document->getMusicalContexts().front());
        musicalContext->addListener (this);
    }

    g.setColour (Colours::darkgrey);
    g.drawRect (getLocalBounds(), 3);

    g.setColour (Colours::white);
    g.setFont (Font (12.0f));
    g.drawText ("Not implemented yet: showing seconds, bars with beat division, chords", getLocalBounds(), Justification::centred);
}

//==============================================================================

void RulersView::willRemoveMusicalContextFromDocument (ARADocument* doc, ARAMusicalContext* context)
{
    jassert (document == doc);

    if (musicalContext == context)
    {
        detachFromMusicalContext();
        repaint();
    }
}

void RulersView::didReorderMusicalContextsInDocument (ARADocument* doc)
{
    jassert (document == doc);

    if (musicalContext != document->getMusicalContexts().front())
    {
        detachFromMusicalContext();
        repaint();
    }
}

void RulersView::willDestroyDocument (ARADocument* doc)
{
    jassert (document == doc);

    detachFromDocument();
}

void RulersView::doUpdateMusicalContextContent (ARAMusicalContext* context, ARAContentUpdateScopes scopeFlags)
{
    jassert (musicalContext == context);

    repaint();
}
