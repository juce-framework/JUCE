# JUCE's Doxygen documentation

JUCE's doxygen setup aims to be pretty vanilla Doxygen and can be run like so:

```
doxygen Doxyfile
```

However, `build.py` can be run to generate additional documentation indexes for JUCE modules:

```
python3 build.py
```

## Styles

`HTML_STYLESHEET` is left empty, meaning that Doxygen will generate its default styles.

JUCE specific overrides then live in `css/doxygen-juce-overrides.css`.

Code highlighting is done with [highlight.js](highlightjs.org).
