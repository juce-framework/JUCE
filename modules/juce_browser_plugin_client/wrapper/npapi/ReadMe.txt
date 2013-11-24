
This folder contains the minimum set of header files that is needed to build the Juce NPAPI 
wrapper on Windows. (OSX already provides these headers so they're not used in a Mac build)

I've included them in the Juce tree because there seem to be so many subtle variations of them
floating around that finding a version that's compatible with the juce wrapper is actually a lot harder
than it sounds! These particular files are taken from the XulRunner tree, where they live in
xulrunner-sdk/sdk/include. To get the rest of the XulRunner code, please visit:
https://developer.mozilla.org/En/XULRunner

Obviously these files are in no way related to Juce, and aren't covered by the Juce license. If you're 
going to use them in your own projects, then please check their own licensing details to satisfy 
yourself that you're allowed to do what you want to do with them.
