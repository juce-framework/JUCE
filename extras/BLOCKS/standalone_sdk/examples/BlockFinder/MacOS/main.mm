#include "../BlockFinder.h"

#import <Cocoa/Cocoa.h>

int main (int argc, const char * argv[])
{
    // Initialise the JUCE code.
    juce::ScopedJuceInitialiser_GUI platform;

    // Create our JUCE object.
    BlockFinder finder;

    // Run an event loop.
    while ([[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode beforeDate:[NSDate dateWithTimeIntervalSinceNow:2]]);

    return 0;
}
