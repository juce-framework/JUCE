The JUCE API Reference
======================

From here, you can generate an offline HTML version of the JUCE API Reference.

Dependencies
------------

- doxygen
- python
- graphviz (to generate inheritance diagrams)

Make sure that all the dependencies can be found on your PATH.

Building
--------

- cd into this directory on the command line
- run `make`

Doxygen will create a new subfolder "doc". Open doc/index.html in your browser
to access the generated HTML documentation.

