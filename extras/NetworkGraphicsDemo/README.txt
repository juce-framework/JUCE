
    JUCE Network graphics demo
    ==========================

This example is an app that we threw together to run as a demo of JUCE on our
booth at CES 2016.

It allows a collection of heterogenous devices on a local network to collectively
draw parts of a large animated vector graphic image that is being generated and
broadcast by one of the machines (the 'master').

Each 'slave' device can be positioned within the overall canvas so that if you put
all of their displays on a wall next to each other and line things up correctly, they
all form a bigger display. At CES we set up a bunch of machines including a Mac, a
Linux box, a PC, and some tablets and phones running iOS and Android, to show how the
same app can run uniformly across all these platforms.

We wrote a few simple animations to run in this context, mainly around the idea of
flocking bird-type objects, because this creates a nice effect as they fly between
each monitor, which helps to create the illusion of a bigger picture.

Any touchs/mouse-clicks on each slave display are sent back to the master, so that
it can react to these. In our demos we used this to cause the flock to swarm towards
the place that was being touched.

To run it, there's a command line flag "master" that will cause the app to run in
master-mode. Other instances running without this will act as slaves. The master
will open a special window that lets you modify the positions of all the networked
devices by dragging/mouse-wheeling. In this window you can also press cursor
left/right to select the demo to run.

These are some interesting bits of code in here - notably a special
LowLevelGraphicsContext class which allows the content generator to simply
draw to a Graphics object as you normally would, but instead of painting to the
screen, everything that is drawn gets serialised into binary packets and broadcast
over UDP to the slaves, which each render their own part of it.

The content should be easy to hack - have a look in Demos.h. Have fun tweaking it
with your own code and ideas!
