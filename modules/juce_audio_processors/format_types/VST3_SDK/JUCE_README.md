This list details modifications made to the VST3 SDK in order to facilitate
inclusion in JUCE.

- `#warning` directives were removed from fstring.cpp, as these cannot be
  silenced with a `pragma GCC diagnostic ignored "-Wcpp"` when building with
  g++.
