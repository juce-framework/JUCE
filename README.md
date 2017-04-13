# The JUCE Library

<img src="http://rawmaterialsoftware.com/images/jucelogo96.png">

JUCE (Jules' Utility Class Extensions) is an all-encompassing C++ class library
for developing cross-platform software.

It contains pretty much everything you're likely to need to create most
applications, and is particularly well-suited for building highly-customised
GUIs, and for handling graphics and sound.

## Supported Operating Systems and Compilers

JUCE can target the following platforms:

- Mac OSX: Applications and VST/AudioUnit/RTAS/NPAPI plugins can be compiled
with Xcode for OSX 10.4 or later.
- iOS: Native iPhone and iPad apps can be built with Xcode.
- Windows: Applications and VST/RTAS/NPAPI/ActiveX plugins can be built using
MS Visual Studio. The results are all fully compatible with Windows XP, Vista
or Win7.
- Linux: Applications and plugins can be built for any kernel 2.6 or later.
- Android: NEW! Android apps can now be built using Ant and Eclipse, with the
Android NDK v5 or later. (This is still a work in progress, so some features aren't still to be finished).

For all the platforms above, the code that you write is the same, and you don't
need to worry about any platform-specific details. If your C++ is portable,
then you should be able to simply re-compile your app to run it on other OSes.

## Integrating JUCE into a project

Adding JUCE to your app is very simple - the easiest way involves simply
including juce.h in your files and adding a single cpp file to your project.
No need to compile any libraries or worry about dependencies. Ideally I'd like
to have made JUCE an include-only library like the std c++ library.. that's not
actually possible because of the platform-specific nastiness that it has to deal
with, but to be able to add a complete multi-platform library to your app in
only two steps is a pretty good result!

Of course JUCE can also be built as a static library and linked into your
application in the traditional way. Or you can use it in its 'amalgamated'
form, where the entire library (all 350,000 lines of it!) has been cunningly
compressed into just two (large!) source files. Having only two files to deal
with means that you can easily add a local copy of them to a project and check
them into your source control system, avoiding any external dependencies.

To further simplify the process of building across multiple platforms, the
Introjucer will automatically generate all the compiler-specific project files
you need to get the same app running in Xcode, Visual Studio, etc. Just use
the Introjucer's IDE to build your project, and it'll take care of the hassle
involved in keeping several different IDE projects in sync with each other.

## Licence and Purchasing

JUCE is released under the GNU Public Licence, which means it can be freely
copied and distributed, and costs nothing to use in open-source applications.

If you'd like to release a closed-source application that uses JUCE, commercial
licences are available for a fee (visit
http://rawmaterialsoftware.com/jucelicense.php for more information on pricing
and terms.

## Design and coding style

In designing JUCE, I've tried to make it:

- Literate: class, method and variable names are clear, self-explanatory and
consistent. It's vital in such a large library that when you look at a class,
it should be obvious how it works; and when you use a class in your own code,
the result should be as readable as possible. The best way to achieve this is
by using well-written names.
- Coherent: a library needs to be predictable enough that once you've got a
feel for the way things are arranged, you can navigate around it without any
surprises. If you need to do a particular task, you should be able to guess
what the classes to do it might be called and find them quickly. (Having a
single person write the entire library helps with this!)
- Cross-platform: platform-dependent code is all confined to a single area
and kept out away from public view. When you include juce.h, you only include
pure C++ classes, it won't pull in any platform-dependent headers. Wherever
it's possible to use a pure C++ technique instead of native functionality,
I've done so.
- High Quality C++: having been a professional C++ coder for 15 years, and
having studied all the available C++ guru literature, I think I'm finally
starting to get the hang of it! Every class in the library is intended to be
a good example of the best possible coding practices - so if you spot anything
dodgy in there, don't hesitate to let me know!
