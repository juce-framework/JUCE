
/*
    This file includes the entire juce source tree via the amalgamated file.

    You could add the amalgamated file directly to your project, but doing it
    like this allows you to put your app's config settings in the
    juce_AppConfig.h file and have them applied to both the juce headers and
    the source code.
*/

#include "juce_AppConfig.h"

// This is where all the juce code gets included, via this amalgamated file..
#include "../../../juce_amalgamated.cpp"


/* NB. A handy tip is that if you're doing a lot of debugging into the juce code, then stepping through
   the amalgamated file can be slow or impossible for the debugger. But if you use the following line
   instead of the one above, then it makes it a lot easier..
*/
//#include "../../../src/juce_amalgamated_template.cpp"
