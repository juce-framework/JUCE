/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/*
  ==============================================================================

   In accordance with the terms of the JUCE 5 End-Use License Agreement, the
   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered
   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE
   under the GPL v3 license.

   End User License Agreement: www.juce.com/juce-5-licence
  ==============================================================================
*/

// BEGIN SECTION A

#if ! defined (JUCE_REPORT_APP_USAGE)
 #define JUCE_REPORT_APP_USAGE 1
#endif

#if ! defined (JUCE_DISPLAY_SPLASH_SCREEN)
 #define JUCE_DISPLAY_SPLASH_SCREEN 1
#endif

#if ! defined (JUCE_USE_DARK_SPLASH_SCREEN)
 #define JUCE_USE_DARK_SPLASH_SCREEN 1
#endif

static const int millisecondsToDisplaySplash = 2000, splashScreenFadeOutTime = 2000;
static const int splashScreenLogoWidth = 123, splashScreenLogoHeight = 63;
static uint32 splashDisplayTime = 0;
static bool appUsageReported = false;


Rectangle<float> getLogoArea (Rectangle<float> parentRect)
{
    return parentRect.reduced (6.0f)
                     .removeFromRight  ((float) splashScreenLogoWidth)
                     .removeFromBottom ((float) splashScreenLogoHeight);
}

//==============================================================================
struct ReportingThread;

struct ReportingThreadContainer  : public ChangeListener,
                                   public DeletedAtShutdown
{
    void sendReport (String, String&, StringPairArray&);
    void changeListenerCallback (ChangeBroadcaster*) override;

    ScopedPointer<ReportingThread> reportingThread;

    juce_DeclareSingleton_SingleThreaded_Minimal (ReportingThreadContainer)
};

juce_ImplementSingleton_SingleThreaded (ReportingThreadContainer)

//==============================================================================
struct ReportingThread  : public Thread,
                          private ChangeBroadcaster
{
    ReportingThread (ReportingThreadContainer& container,
                     String& address,
                     String& userAgent,
                     StringPairArray& parameters)
        : Thread ("JUCE app usage reporting"),
          threadContainer (container),
          headers ("User-Agent: " + userAgent)
    {
        StringArray postData;

        for (auto& key : parameters.getAllKeys())
            if (parameters[key].isNotEmpty())
                postData.add (key + "=" + URL::addEscapeChars (parameters[key], true));

        url = URL (address).withPOSTData (postData.joinIntoString ("&"));

        addChangeListener (&threadContainer);
    }

    ~ReportingThread()
    {
        removeChangeListener (&threadContainer);

        if (webStream != nullptr)
            webStream->cancel();

        stopThread (2000);
    }

    void run() override
    {
        webStream = new WebInputStream (url, true);
        webStream->withExtraHeaders (headers);
        webStream->connect (nullptr);

        sendChangeMessage();
    }

private:
    ReportingThreadContainer& threadContainer;
    URL url;
    String headers;
    ScopedPointer<WebInputStream> webStream;
};

//==============================================================================
void ReportingThreadContainer::sendReport (String address, String& userAgent, StringPairArray& parameters)
{
    reportingThread = new ReportingThread (*this, address, userAgent, parameters);

    reportingThread->startThread();
}

void ReportingThreadContainer::changeListenerCallback (ChangeBroadcaster*)
{
    reportingThread = nullptr;
}

//==============================================================================
JUCESplashScreen::JUCESplashScreen (Component& parent)
{
    ignoreUnused (hasStartedFading);
    ignoreUnused (parent);

   #if JUCE_REPORT_APP_USAGE
    if (! appUsageReported)
    {
        const ScopedTryLock appUsageReportingLock (appUsageReporting);

        if (appUsageReportingLock.isLocked() && ! appUsageReported)
        {
            const auto deviceDescription = SystemStats::getDeviceDescription();
            const auto deviceString = SystemStats::getDeviceIdentifiers().joinIntoString (":");
            const auto deviceIdentifier = String::toHexString (deviceString.hashCode64());
            const auto osName = SystemStats::getOperatingSystemName();

            StringPairArray data;

            data.set ("v",   "1");
            data.set ("tid", "UA-19759318-3");
            data.set ("cid", deviceIdentifier);
            data.set ("t",   "event");
            data.set ("ec",  "info");
            data.set ("ea",  "appStarted");

            data.set ("cd1", SystemStats::getJUCEVersion());
            data.set ("cd2", osName);
            data.set ("cd3", deviceDescription);
            data.set ("cd4", deviceIdentifier);

            String appType, appName, appVersion, appManufacturer;

           #if defined(JucePlugin_Name)
            appType         = "Plugin";
            appName         = JucePlugin_Name;
            appVersion      = JucePlugin_VersionString;
            appManufacturer = JucePlugin_Manufacturer;
           #else
            if (JUCEApplicationBase::isStandaloneApp())
            {
                appType = "Application";

                if (auto* app = JUCEApplicationBase::getInstance())
                {
                    appName    = app->getApplicationName();
                    appVersion = app->getApplicationVersion();
                }
            }
            else
            {
                appType = "Library";
            }
           #endif

            data.set ("cd5", appType);
            data.set ("cd6", appName);
            data.set ("cd7", appVersion);
            data.set ("cd8", appManufacturer);

            data.set ("an", appName);
            data.set ("av", appVersion);

            auto agentCPUVendor = SystemStats::getCpuVendor();

            if (agentCPUVendor.isEmpty())
                agentCPUVendor = "CPU";

            auto agentOSName = osName.replaceCharacter ('.', '_')
                                     .replace ("iOS", "iPhone OS");
           #if JUCE_IOS
            agentOSName << " like Mac OS X";
           #endif

            String userAgent;
            userAgent << "Mozilla/5.0 ("
                      << deviceDescription << ";"
                      << agentCPUVendor << " " << agentOSName << ";"
                      << SystemStats::getDisplayLanguage() << ")";

            ReportingThreadContainer::getInstance()->sendReport ("https://www.google-analytics.com/collect", userAgent, data);

            appUsageReported = true;
        }
    }
   #else
    ignoreUnused (appUsageReported);
   #endif

   #if JUCE_DISPLAY_SPLASH_SCREEN
    if (splashDisplayTime == 0
         || Time::getMillisecondCounter() < splashDisplayTime + (uint32) millisecondsToDisplaySplash)
    {
        content = getSplashScreenLogo();

        setAlwaysOnTop (true);
        parent.addAndMakeVisible (this);
    }
    else
   #endif
    {
        startTimer (1);
    }
}

JUCESplashScreen::~JUCESplashScreen()
{
}

Drawable* JUCESplashScreen::getSplashScreenLogo()
{
    const char* svgData = R"JUCESPLASHSCREEN(
      <?xml version="1.0" encoding="UTF-8"?>
      <svg width="123px" height="63px" viewBox="0 0 123 63" version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
        <defs>
          <rect id="path-1" x="0.253112033" y="0.253112033" width="122" height="62" rx="10"></rect>
        </defs>
        <g id="Page-1" stroke="none" stroke-width="1" fill="none" fill-rule="evenodd" opacity="0.8">
          <mask id="mask-2" fill="white">
            <use xlink:href="#path-1"></use>
          </mask>
          <rect stroke="#B144C8" stroke-width="2" x="1.25311203" y="1.25311203" width="120" height="60" rx="10" fill=")JUCESPLASHSCREEN"
   #if JUCE_USE_DARK_SPLASH_SCREEN
    "#000000"
   #else
    "#FFFFFF"
   #endif
    "\"></rect>"
    R"JUCESPLASHSCREEN(
        <g id="Slice" mask="url(#mask-2)" fill="#B144C8" transform="translate(6.000000, 14.000000)">
          <path d="M17.728338,35.4569627 C7.9530089,35.4569627 0,27.5039538 0,17.7286247 C0,7.95303496 7.9530089,2.60592054e-05 17.728338,2.60592054e-05 C27.5039277,2.60592054e-05 35.4569366,7.95303496 35.4569366,17.7286247 C35.4569366,27.5039538 27.5039277,35.4569627 17.728338,35.4569627 Z M17.728338,1.18222797 C8.60474962,1.18222797 1.18230615,8.60493204 1.18230615,17.7285204 C1.18230615,26.8521088 8.60474962,34.2745523 17.728338,34.2745523 C26.852187,34.2745523 34.2746305,26.8521088 34.2746305,17.7285204 C34.2746305,8.60493204 26.852187,1.18222797 17.728338,1.18222797 Z" id="Combined-Shape"></path>
          <path d="M31.7163462,17.1373674 C32.0191542,17.1373674 32.3177926,17.0276581 32.5291328,16.8108455 C32.7621021,16.5724038 32.869466,16.2576086 32.8308984,15.9313473 C32.5035948,13.1505695 31.4109323,10.5157233 29.6709592,8.31215687 C29.4580554,8.04244409 29.1630652,7.89364603 28.8404523,7.89364603 C28.5558858,7.89364603 28.2796582,8.01325778 28.062585,8.23007037 L19.5130809,16.7798351 C19.3812213,16.9119552 19.4745133,17.1373674 19.6610972,17.1373674 L31.7163462,17.1373674 Z" id="Fill-6"></path>
          <path d="M28.8404002,27.5631082 L28.8404002,27.5631082 C29.1630131,27.5631082 29.4580033,27.4148313 29.6711676,27.1451185 C31.4108802,24.941031 32.5038033,22.3064453 32.8308463,19.5256675 C32.8694139,19.1994062 32.76205,18.884611 32.5290807,18.6461693 C32.3177405,18.4293567 32.019102,18.3196475 31.7162941,18.3196475 L19.6610451,18.3196475 C19.4747217,18.3196475 19.3811692,18.5453202 19.5130288,18.6771798 L28.0627935,27.2269445 C28.2796061,27.443757 28.5558336,27.5631082 28.8404002,27.5631082" id="Fill-8"></path>
          <path d="M6.61653649,7.89375026 C6.29392353,7.89375026 5.99893332,8.04228773 5.78576902,8.31200051 C4.04605647,10.5158275 2.95313339,13.1506738 2.62609037,15.9314516 C2.58752274,16.2577128 2.69488667,16.572508 2.92785596,16.8109498 C3.13919612,17.0277623 3.43783461,17.1374716 3.74064258,17.1374716 L15.7958916,17.1374716 C15.9822149,17.1374716 16.0757675,16.9117989 15.9439079,16.7799393 L7.39414318,8.23017461 C7.17733059,8.01336202 6.90110301,7.89375026 6.61653649,7.89375026" id="Fill-10"></path>
          <path d="M15.7957874,18.3197256 L3.74053834,18.3197256 C3.43773038,18.3197256 3.13935247,18.4294349 2.92775173,18.6462475 C2.69504302,18.8846892 2.5874185,19.1994844 2.62598613,19.5257457 C2.95328975,22.3065235 4.04595223,24.9411091 5.78592538,27.1449361 C5.99882908,27.4146489 6.29407988,27.5631864 6.61643225,27.5631864 C6.90073818,27.5631864 7.17670517,27.4440958 7.39351776,27.2278044 L7.39403894,27.2272832 L15.9438036,18.6772579 C16.0756632,18.5451378 15.9823713,18.3197256 15.7957874,18.3197256" id="Fill-12"></path>
          <path d="M17.1374455,3.74079894 C17.1374455,3.43773038 17.0277363,3.13909188 16.8109237,2.92775173 C16.572482,2.69478243 16.2574262,2.5874185 15.9314255,2.62598613 C13.1506477,2.95328975 10.5158015,4.04595223 8.31223504,5.78592538 C8.04252227,5.99882908 7.8937242,6.29407988 7.8937242,6.61643225 C7.8937242,6.90099877 8.01307536,7.17748694 8.23014855,7.39429953 L16.7799132,15.9438036 C16.9117728,16.0756632 17.1374455,15.9823713 17.1374455,15.7957874 L17.1374455,3.74079894 Z" id="Fill-14"></path>
          <path d="M27.5631603,6.61648437 L27.5631603,6.61648437 C27.5631603,6.294132 27.4148834,5.9988812 27.1451707,5.7857169 C24.9410831,4.04600435 22.3064974,2.95308127 19.5257196,2.62603825 C19.1994583,2.58747062 18.8846631,2.69483455 18.6459608,2.92780385 C18.4294088,3.139144 18.3196996,3.43778249 18.3196996,3.74085105 L18.3196996,15.7958395 C18.3196996,15.9821628 18.5453723,16.0757153 18.6772319,15.9438558 L27.2269966,7.39409106 C27.4438092,7.17727847 27.5631603,6.90131148 27.5631603,6.61648437" id="Fill-16"></path>
          <path d="M7.89380238,28.840348 C7.89380238,29.162961 8.04233985,29.4579512 8.31205263,29.6711155 C10.5158796,31.4110887 13.1507259,32.5040117 15.9315037,32.8307942 C16.2575044,32.8693618 16.5725601,32.7619979 16.8110019,32.5290286 C17.0278145,32.3176884 17.1375237,32.0190499 17.1375237,31.7162419 L17.1375237,19.6609929 C17.1375237,19.4746696 16.9115904,19.3811171 16.7799914,19.5129766 L8.23022672,28.0627413 C8.01315354,28.2798145 7.89380238,28.5557815 7.89380238,28.840348" id="Fill-18"></path>
          <path d="M18.3197778,31.7163462 C18.3197778,32.0191542 18.429487,32.3175321 18.646039,32.5291328 C18.8847413,32.7618415 19.1995365,32.869466 19.5257978,32.8308984 C22.3065756,32.5038554 24.9411613,31.4111929 27.1449883,29.6709592 C27.414701,29.4580554 27.5632385,29.1630652 27.5632385,28.8404523 C27.5632385,28.5561463 27.4441479,28.2801794 27.2278565,28.0633668 L18.6773101,19.5133415 C18.5451899,19.3812213 18.3197778,19.4745133 18.3197778,19.6613578 L18.3197778,31.7163462 Z" id="Fill-20"></path>
        </g>
        <path d="M57.404,39.48 L57.404,25.688 L53.404,25.688 L53.404,39.48 C53.404,41.912 51.996,43.512 49.884,43.512 C48.7,43.512 47.804,43.032 46.524,41.336 L43.644,43.832 C45.596,46.392 47.484,47.256 49.884,47.256 C54.268,47.256 57.404,44.152 57.404,39.48 Z M78.508,38.296 L78.508,25.688 L74.508,25.688 L74.508,38.136 C74.508,41.4 72.588,43.512 69.836,43.512 C67.084,43.512 65.164,41.4 65.164,38.136 L65.164,25.688 L61.164,25.688 L61.164,38.296 C61.164,43.864 65.196,47.256 69.836,47.256 C74.476,47.256 78.508,43.864 78.508,38.296 Z M99.324,44.6 L96.956,41.624 C94.94,43.032 93.628,43.512 92.06,43.512 C88.188,43.512 85.052,40.376 85.052,36.344 C85.052,32.312 88.188,29.176 92.06,29.176 C93.692,29.176 94.876,29.592 96.988,31.064 L99.324,28.152 C97.116,26.36 94.748,25.432 92.06,25.432 C86.012,25.432 80.956,30.264 80.956,36.344 C80.956,42.392 85.948,47.256 92.06,47.256 C94.524,47.256 96.604,46.648 99.324,44.6 Z M114.796,47 L114.796,43.384 L105.516,43.384 L105.516,38.04 L114.444,38.04 L114.444,34.424 L105.516,34.424 L105.516,29.304 L114.796,29.304 L114.796,25.688 L101.516,25.688 L101.516,47 L114.796,47 Z" id="JUCE" fill="#B144C8" mask="url(#mask-2)"></path>
        <path d="M66.1199999,21 L63.8399999,14.12 L61.8399999,19.17 L59.8399999,14.12 L57.5599999,21 L58.3999999,21 L59.9099999,16.34 L61.8399999,21.22 L63.7699999,16.34 L65.2799999,21 L66.1199999,21 Z M71.2166666,21 L70.4166666,21 C70.2866666,20.77 70.2466666,20.43 70.2466666,20.18 C69.8966666,20.63 69.2066666,21.08 68.4266666,21.08 C67.4766666,21.08 66.8066666,20.56 66.8066666,19.73 C66.8066666,18.89 67.3666666,18.06 70.1766666,17.61 C70.1766666,17.04 69.7466666,16.66 69.0066666,16.66 C68.3666666,16.66 67.9366666,16.89 67.5566666,17.18 L67.0766666,16.61 C67.6066666,16.17 68.2866666,15.9 69.0666666,15.9 C69.9066666,15.9 70.9766666,16.22 70.9766666,17.62 L70.9766666,19.83 C70.9766666,20.19 71.0366666,20.65 71.2166666,21 Z M70.1766666,18.73 L70.1766666,18.29 C68.1366666,18.64 67.6066666,19.15 67.6066666,19.73 C67.6066666,20.2 68.1166666,20.4 68.5866666,20.4 C69.3066666,20.4 70.1766666,19.8 70.1766666,18.73 Z M77.0633333,21 L76.2633333,21 L76.2633333,20.18 C75.9433333,20.68 75.2933333,21.08 74.5033333,21.08 C73.1133333,21.08 71.9833333,19.9 71.9833333,18.49 C71.9833333,17.08 73.1133333,15.9 74.5033333,15.9 C75.2933333,15.9 75.9433333,16.3 76.2633333,16.8 L76.2633333,14 L77.0633333,14 L77.0633333,21 Z M76.2633333,18.49 C76.2633333,17.47 75.5133333,16.66 74.5233333,16.66 C73.5333333,16.66 72.7833333,17.47 72.7833333,18.49 C72.7833333,19.51 73.5333333,20.32 74.5233333,20.32 C75.5133333,20.32 76.2633333,19.51 76.2633333,18.49 Z M83.14,18.46 C83.14,18.64 83.12,18.81 83.12,18.81 L78.9,18.81 C79.04,19.66 79.77,20.32 80.66,20.32 C81.18,20.32 81.73,20.09 82.12,19.67 L82.69,20.19 C82.17,20.78 81.43,21.08 80.66,21.08 C79.23,21.08 78.07,19.91 78.07,18.49 C78.07,17.06 79.22,15.9 80.65,15.9 C82.12,15.9 83.14,17.06 83.14,18.46 Z M82.3,18.1 C82.12,17.2 81.52,16.66 80.65,16.66 C79.78,16.66 79.07,17.27 78.91,18.1 L82.3,18.1 Z M93.5633333,15.98 L92.7233333,15.98 L91.4533333,19.36 L89.8233333,15.7 L88.1933333,19.36 L86.9233333,15.98 L86.0833333,15.98 L88.1433333,21.28 L89.8233333,17.44 L91.5033333,21.28 L93.5633333,15.98 Z M95.4,14.46 C95.4,14.16 95.15,13.91 94.85,13.91 C94.55,13.91 94.3,14.16 94.3,14.46 C94.3,14.76 94.55,15.01 94.85,15.01 C95.15,15.01 95.4,14.76 95.4,14.46 Z M95.25,21 L95.25,15.98 L94.45,15.98 L94.45,21 L95.25,21 Z M99.1466667,16.69 L99.1466667,15.98 L97.8066667,15.98 L97.8066667,14.34 L97.0066667,14.34 L97.0066667,15.98 L95.9966667,15.98 L95.9966667,16.69 L97.0066667,16.69 L97.0066667,21 L97.8066667,21 L97.8066667,16.69 L99.1466667,16.69 Z M104.173333,21 L104.173333,18.12 C104.173333,16.7 103.243333,15.9 102.093333,15.9 C101.403333,15.9 100.833333,16.27 100.623333,16.63 L100.623333,14 L99.8233334,14 L99.8233334,21 L100.623333,21 L100.623333,18.17 C100.623333,17.22 101.233333,16.66 102.013333,16.66 C102.793333,16.66 103.373333,17.22 103.373333,18.17 L103.373333,21 L104.173333,21 Z" id="Made-with" fill="#B144C8" mask="url(#mask-2)"></path>
      </g>
    </svg>
    )JUCESPLASHSCREEN";

    ScopedPointer<XmlElement> svgXml (XmlDocument::parse (svgData));
    return Drawable::createFromSVG (*svgXml);
}

void JUCESplashScreen::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    Point<float> bottomRight (0.9f * r.getWidth(),
                              0.9f * r.getHeight());

    ColourGradient cg (Colour (0x00000000), Line<float> (0.0f, r.getHeight(), r.getWidth(), 0.0f)
                                              .findNearestPointTo (bottomRight),
                       Colour (0xff000000), bottomRight, false);
    cg.addColour (0.25f, Colour (0x10000000));
    cg.addColour (0.50f, Colour (0x30000000));
    cg.addColour (0.75f, Colour (0x70000000));
    g.setFillType (cg);
    g.fillAll();

    content->drawWithin (g, getLogoArea (r), RectanglePlacement::centred, 1.0f);

    if (splashDisplayTime == 0)
        splashDisplayTime = Time::getMillisecondCounter();

    if (! isTimerRunning())
        startTimer (millisecondsToDisplaySplash);
}

void JUCESplashScreen::timerCallback()
{
   #if JUCE_DISPLAY_SPLASH_SCREEN
    if (isVisible() && ! hasStartedFading)
    {
        hasStartedFading = true;
        fader.animateComponent (this, getBounds(), 0.0f, splashScreenFadeOutTime, false, 0, 0);
    }

    if (hasStartedFading && ! fader.isAnimating())
   #endif
        delete this;
}

void JUCESplashScreen::parentSizeChanged()
{
    if (auto* p = getParentComponent())
        setBounds (p->getLocalBounds().removeFromBottom (splashScreenLogoHeight * 3)
                                      .removeFromRight  (splashScreenLogoWidth  * 3));
}

void JUCESplashScreen::parentHierarchyChanged()
{
    toFront (false);
}

bool JUCESplashScreen::hitTest (int x, int y)
{
    return getLogoArea (getLocalBounds().toFloat()).contains ((float) x, (float) y);
}

void JUCESplashScreen::mouseUp (const MouseEvent&)
{
    URL juceWebsite ("https://juce.com");
    juceWebsite.launchInDefaultBrowser();
}

// END SECTION A

} // namespace juce
