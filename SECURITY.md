# Security Policy

## Reporting a vulnerability

**Please do not report security vulnerabilities through public GitHub issues,
discussions, or the forum.**

Report suspected vulnerabilities by email to [security@juce.com](mailto:security@juce.com).

To help us understand and resolve the issue quickly, please include as much of
the following as you can:

- The type of issue (e.g. buffer overflow, use-after-free, path traversal,
  injection)
- The JUCE version(s) and module(s) affected (e.g. `juce_graphics`,
  `juce_audio_formats`), including whether the issue lies in a bundled
  third-party component
- The platform(s) on which you observed or believe the issue applies
  (macOS, Windows, iOS, Android, Linux)
- Full paths to the affected source files, if known
- Step-by-step instructions or a minimal code sample to reproduce the issue
- Proof-of-concept or exploit code, if available
- The impact you believe the issue has, including how an attacker might
  exploit it in a shipped application

## What to expect

- We will acknowledge your report within **2 business days**.
- We will give you an initial assessment (accepted, declined, or more
  information needed) within **10 business days**.
- We will investigate, keep you informed of our progress, and tell you the
  outcome of our assessment.
- We follow a **coordinated disclosure** model: we ask that you do not publish
  details of the issue until a fix or mitigation is available. We aim to
  resolve confirmed vulnerabilities within **90 days** of the report and will
  agree a disclosure date with you.
- We will credit you in the published advisory if you wish. We do not
  currently operate a paid bug bounty programme.

We will not pursue legal action against researchers who act in good faith:
who make a genuine effort to avoid privacy violations and service disruption,
who do not exploit an issue beyond what is needed to demonstrate it, and who
give us reasonable time to remediate before public disclosure.

## Security advisories and updates

Fixes for confirmed vulnerabilities are published in JUCE releases and
announced in the Security Advisories section of the JUCE Forum:
[https://forum.juce.com/c/security-advisories](https://forum.juce.com/c/security-advisories).

Supported versions, security-support periods for each major JUCE version, how
to receive security notifications, and our full vulnerability disclosure policy
are maintained at [https://juce.com/security](https://juce.com/security).

## Bundled third-party components

JUCE includes source code from third-party projects (for example libpng,
zlib, libjpeg, FLAC, and Ogg/Vorbis). A complete inventory of these
components, with their versions and locations in the source tree, is
maintained in the SPDX software bill of materials at
[JUCE.spdx.json](JUCE.spdx.json). If you find a vulnerability that
originates in one of these components, please report it to us at the address
above — we will assess its impact in JUCE, coordinate with the upstream
maintainers, and update the bundled copy as needed. If you have already
reported it upstream, please tell us so we can track the fix.
