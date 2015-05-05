# JUCE with QtCreator exporter

This fork of JUCE includes an additional exporter to create QtCreator projects. No further changes were made. New JUCE releases will be regularly merged to keep the sources up to date.

Instructions:

* Get the sources from this repository and compile the IntroJucer as usual.
* Open a JUCE project with the patched Introjucer and add a new exporter for QtCreator. Save the project.
* Open the created project file `Builds/QtCreator/YOURPROJECT.pro` in the QtCreator.
* Choose a compiler if you are asked so and let QMake configure the project.
* Open the project settings and disable the `Shadow-Build` feature for debug and release mode.

Known problems:

* Windows has a maximum length of 260 characters for relative paths. JUCE projects make heavy use of relative paths and these may become too long and compilation will fail. You can try to map long base paths to an own drive letter. From a command line run

    subst P:\ C:\Your\Project\Folder\Is\Quite\Long

and open your project files in QtCreator directly from the new drive P:\YourProject\Builds\Qtcreator\...


# The JUCE Library

JUCE (Jules' Utility Class Extensions) is an all-encompassing 
C++ framework for developing cross-platform software.

It contains pretty much everything you're likely to need to create
most applications, and is particularly well-suited for building 
highly-customised GUIs, and for handling graphics and sound.

Most JUCE modules are shared under the GNU Public Licence 
(GPLv2, v3, and the AGPLv3). This means that the code can 
be freely copied and distributed, and costs nothing to use 
in other GPL applications. One module (the juce_core module) 
is permissively licensed under the ISC.

For more information, visit the website:
http://www.juce.com