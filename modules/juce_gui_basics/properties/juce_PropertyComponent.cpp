/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

PropertyComponent::PropertyComponent (const String& name, const int preferredHeight_)
    : Component (name), preferredHeight (preferredHeight_)
{
    jassert (name.isNotEmpty());
}

PropertyComponent::~PropertyComponent() {}

void PropertyComponent::paint (Graphics& g)
{
    LookAndFeel& lf = getLookAndFeel();

    lf.drawPropertyComponentBackground (g, getWidth(), getHeight(), *this);
    lf.drawPropertyComponentLabel      (g, getWidth(), getHeight(), *this);
}

void PropertyComponent::resized()
{
    if (Component* const c = getChildComponent(0))
        c->setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this));
}

void PropertyComponent::enablementChanged()
{
    repaint();
}
