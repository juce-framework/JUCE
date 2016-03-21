#ifndef juce_ios_Windowing_h
#define juce_ios_Windowing_h

@interface JuceAppStartupDelegate : NSObject <UIApplicationDelegate>
{
}

@property (strong, nonatomic) UIWindow *window;

- (void) applicationDidFinishLaunching: (UIApplication*) application;
- (void) applicationWillTerminate: (UIApplication*) application;
- (void) applicationDidEnterBackground: (UIApplication*) application;
- (void) applicationWillEnterForeground: (UIApplication*) application;
- (void) applicationDidBecomeActive: (UIApplication*) application;
- (void) applicationWillResignActive: (UIApplication*) application;

@end



#endif /* juce_ios_Windowing_h */
