/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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
