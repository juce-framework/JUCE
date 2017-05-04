
This folder contains the source from the excellent Box2D physics library. 
For any Box2D-related info, visit their website: http://box2d.org

To create the juce module, the only changes required to the original source-code
were to adjust the include paths to be relative rather than absolute, and to wrap
#ifdefs around a couple of unguarded header files. (Oh, and there were a few
compiler warnings that I cleaned up to avoid bothering people with them)