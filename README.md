# JUCE and the ARA API

This repository is a fork of the [JUCE 5 develop branch](https://github.com/WeAreROLI/JUCE) with additions that enable it to generate [ARA plugins](https://www.celemony.com/en/service1/about-celemony/technologies) in the VST3 or AudioUnit format.
The fork is currently being maintained by [Celemony](https://www.celemony.com) and [SoundRadix](https://www.soundradix.com), with the goal of it being picked up eventually by [ROLI](https://www.juce.com) for main line JUCE.

In order to use this branch you'll need access to the [ARA SDK](http://www.celemony.com/en/service1/about-celemony/technologies) - if you're a developer and would like access to the latest ARA SDK release, send an email to [ara@celemony.com](mailto:ara@celemony.com?Subject=JUCE%20ARA%20integration).

Current ARA SDK compatibility version: 1.9.6.019

For feedback and questions, please contact Celemony via [ara@celemony.com](mailto:ara@celemony.com?Subject=JUCE%20ARA%20integration).

### Getting started:

We've made modifications to the Projucer to enable it to work with the ARA 2.0 SDK. These modifications allow building VST3 and AudioUnit plugins that can be loaded as ARA plugins by a compatible host. 

To create an ARA plugin, clone the [ARA branch](https://github.com/Celemony/JUCE_ARA/tree/ARA) of this repository and build the [Projucer](https://github.com/Celemony/JUCE_ARA/tree/ARA/extras/Projucer)(projects for Visual Studio and Xcode exist within the repository.)

Once you have the ARA version of the Projucer built you can use it to specify your global ARA path like so:

<img src="https://i.imgur.com/fRjU8kB.png"/>

This ARA SDK path will be used for new ARA plugin projects, but can also be configured per project.

Once the SDK has been located we can create a new Audio Plug-In project like so

<img src="https://i.imgur.com/cqsEA18.png"/>

We specifically want to create an ARA plugin. This is not one of the main types in the template window but can be selected like so:

<img src="https://i.imgur.com/mY6Z1XL.png"/>

The generated plugin will have, in addition to the standard PluginProcessor and PluginEditor classes, a PluginARADocumentController file containing a class that overrides the ARA::PlugIn::DocumentController type.

<img src="https://i.imgur.com/JqNIe2b.png"/>

This version of the Projucer supports VST3 and AudioUnit, so make sure at least one of these plugin formats is checked
<img src="https://i.imgur.com/7wWWPuK.png"/>

We can also edit other ARA plugin factory properties such as available content types and transformation flags as well as the plugin factory identifier.

<img src="https://i.imgur.com/mJoXIxG.png"/>

This should be enough to generate an empty ARA plugin. For a more complete example see the [.jucer file located here](https://github.com/Celemony/JUCE_ARA/tree/ARA/examples/ARA).

# The JUCE 5 Library

**BY DOWNLOADING, INSTALLING OR USING ANY PART OF THE JUCE LIBRARY, YOU AGREE
TO THE [JUCE 5 END-USER LICENSE AGREEMENT](https://www.juce.com/juce-5-licence)
AND [JUCE 5 PRIVACY POLICY](https://www.juce.com/juce-5-privacy-policy), WHICH
ARE BINDING AGREEMENTS BETWEEN YOU AND ROLI, LTD. IF YOU DO NOT AGREE TO THE
TERMS, DO NOT USE THE JUCE LIBRARY.**

JUCE is an all-encompassing C++ framework for developing cross-platform
software. JUCE is used by hundreds of companies to develop powerful,
cross-platform audio, interactive, embedded or graphic applications.

We now have tier-leveled license terms for JUCE 5, with different terms for
each available license: JUCE Personal (for developers or startup businesses
with revenue under 50K USD "Revenue Limit"; free), JUCE Indie (for small
businesses with under 200K Revenue Limit; $35/month), JUCE Pro (no Revenue
Limit; $65/month), and JUCE Educational (no Revenue Limit; free for bona fide
educational institutes). All licenses allow you to commercially release
applications so long as you do not exceed the Revenue Limit and pay applicable
Fees. Once your business hits the Revenue Limit for your JUCE license, you will
either have to upgrade your JUCE license or release your Applications under the
[GNU General Public License v.3](https://www.gnu.org/licenses/gpl-3.0.en.html),
which means, among other things, that your code can be freely copied and
distributed.

You agree to give notice to the end-users of your Applications that we may
track the IP addresses associated with their use of the Applications using JUCE
solely for our internal purposes in providing JUCE, unless you are a paying
JUCE customer and opt-out of such tracking. You agree to fully comply with all
laws, including relating to the collection of information from children and the
[Children’s Online Privacy Protection Act
(COPPA)](https://www.ftc.gov/enforcement/rules/rulemaking-regulatory-reform-proceedings/childrens-online-privacy-protection-rule).

JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE, ARE DISCLAIMED.

The juce_audio_basics, juce_audio_devices, juce_blocks_basics, juce_core and
juce_events modules are permissively licensed under the terms of the [ISC
license](http://www.isc.org/downloads/software-support-policy/isc-license).

For more information, visit the website:
[www.juce.com](https://www.juce.com)

FULL JUCE TERMS:
- [JUCE 5 END-USER LICENSE AGREEMENT](https://www.juce.com/juce-5-licence)
- [JUCE 5 PRIVACY POLICY](https://www.juce.com/juce-5-privacy-policy)
![alt text](https://d30pueezughrda.cloudfront.net/juce/JUCE_banner.png "JUCE")

JUCE is an open-source cross-platform C++ application framework used for rapidly
developing high quality desktop and mobile applications, including VST, AU (and AUv3),
RTAS and AAX audio plug-ins. JUCE can be easily integrated with existing projects or can
be used as a project generation tool via the [Projucer](https://juce.com/discover/projucer),
which supports exporting projects for Xcode (macOS and iOS), Visual Studio, Android Studio,
Code::Blocks, CLion and Linux Makefiles as well as containing a source code editor and
live-coding engine which can be used for rapid prototyping.

## Getting Started
The JUCE repository contains a [master](https://github.com/weareroli/JUCE/tree/master)
and [develop](https://github.com/weareroli/JUCE/tree/develop) branch. The develop branch
contains the latest bugfixes and features and is periodically merged into the master
branch in stable [tagged releases](https://github.com/WeAreROLI/JUCE/releases)
(the latest release containing pre-built binaries can be also downloaded from the
[JUCE website](https://shop.juce.com/get-juce)).

The repository doesn't contain a pre-built Projucer so you will need to build it
for your platform - Xcode, Visual Studio and Linux Makefile projects are located in
[extras/Projucer/Builds](/extras/Projucer/Builds)
(the minumum system requirements are listed in the __System Requirements__ section below).
The Projucer can then be used to create new JUCE projects, view tutorials and run examples.
It is also possible to include the JUCE modules source code in an existing project directly,
or build them into a static or dynamic library which can be linked into a project.

For further help getting started, please refer to the JUCE
[documentation](https://juce.com/learn/documentation) and
[tutorials](https://juce.com/learn/tutorials).

## System Requirements
#### Building JUCE Projects
- __macOS__: macOS 10.11 and Xcode 7.3.1
- __Windows__: Windows 8.1 and Visual Studio 2013 64-bit
- __Linux__: GCC 5.0

#### Minimum Deployment Targets
- __macOS__: macOS 10.7
- __Windows__: Windows Vista
- __Linux__: Mainstream Linux distributions

## Contributing
For bug reports and features requests, please visit the [JUCE Forum](https://forum.juce.com/) -
the JUCE developers are active there and will read every post and respond accordingly. When
submitting a bug report, please ensure that it follows the
[issue template](/.github/ISSUE_TEMPLATE.txt).
We don't accept third party GitHub pull requests directly due to copyright restrictions
but if you would like to contribute any changes please contact us.

## License
The core JUCE modules (juce_audio_basics, juce_audio_devices, juce_blocks_basics, juce_core
and juce_events) are permissively licensed under the terms of the
[ISC license](http://www.isc.org/downloads/software-support-policy/isc-license/).
Other modules are covered by a
[GPL/Commercial license](https://www.gnu.org/licenses/gpl-3.0.en.html).

There are multiple commercial licensing tiers for JUCE 5, with different terms for each:
- JUCE Personal (developers or startup businesses with revenue under 50K USD) - free
- JUCE Indie (small businesses with revenue under 200K USD) - $35/month
- JUCE Pro (no revenue limit) - $65/month
- JUCE Eductational (no revenue limit) - free for bona fide educational institutes

For full terms see [LICENSE.md](LICENSE.md).