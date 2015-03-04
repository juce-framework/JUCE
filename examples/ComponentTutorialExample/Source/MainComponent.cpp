/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent()
    : lightGrid ("lightGrid") //initialise the ToggleLightGridComponent object
{
    setSize (600, 600);

    // add the light grid to out main component.
    addAndMakeVisible (lightGrid);
}

MainContentComponent::~MainContentComponent()
{
}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xff001F36));
}

void MainContentComponent::resized()
{
    // set the size of the grid to fill the whole window.
    lightGrid.setBounds (getLocalBounds());
}
