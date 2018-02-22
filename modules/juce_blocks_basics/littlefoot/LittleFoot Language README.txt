@verbatim

This is a brief description of the LittleFoot language syntax

Littlefoot basically looks like C, but has no pointers, and the only types are:

- int   (32-bit signed integer)
- float (32-bit float)
- bool

The top-level syntax of a program is a list of global variables and global functions. Order
of declaration isn't important, you can use functions and variables that are declared later
in the file without needing to pre-declare anything.

Comments are the same format as C/C++/java/etc

So for example:


    // global variables. These are initialised to 0 or false when the program is loaded, and
    // you can't currently provide any other initial values
    int foo, bar;

    int getTheNextNumber()
    {
        return addTwoNumbers (++foo, 2.0) * 3;
    }

    float addTwoNumbers (int x, float y)
    {
        return float (x) + y;
    }

The usual control-flow operators are provided, all with C++ style syntax:

    if/else
    for
    while
    do...while
    continue
    break
    return

(There isn't currently a switch statement though)

Arithmetic ops are the usual suspects, (with the standard operator precedence):

    +, -, *, /, %
    ||, &&, |, &, ~, ^
    ++, --, +=, -=, *=, /=, %=, |=, &=, ^=
    ==, !=, <, >, <=, >=, !
    <<, >>, <<=, >>=, >>>
    Ternary operator (x ? y : z)

Local variables are declared in C++-style syntax:

    void foo()
    {
        int x = 123;
        float y = 12.0, z = 1.0e5;
        bool b = y > 20.0;
    }

Casts of primitive types are done with function-style syntax, e.g.

    int x = int (123.0);
    float f = float (getIntegerValue());

The program communicates with the host computer by using a shared area of memory
called the heap which the host can change. There are some built-in functions
available for the program to use to read from the heap:

    int getHeapByte (int byteIndex);                    // reads a single byte from the heap
    int getHeapInt (int byteIndex);                     // reads 4 bytes from the heap as an integer
    int getHeapBits (int startBitIndex, int numBits);   // reads a sequence of bits from the heap and returns it as an integer
    void setHeapByte (int byteIndex, int newValue);     // writes a single byte to the heap
    void setHeapInt (int byteIndex, int newValue);      // writes 4 bytes to the heap

Depending on the context, there will also be some built-in functions that the
program can use to do what it needs to do. Currently in the standard Pad BLOCK program,
you have the following functions available:

    int makeARGB (int alpha, int red, int green, int blue);         // combines a set of 8-bit ARGB values into a 32-bit colour
    int blendARGB (int baseColour, int overlaidColour);             // blends the overlaid ARGB colour onto the base one and returns the new colour
    void fillPixel (int rgb, int x, int y);                         // sets a LED colour on the display
    void fillRect (int rgb, int x, int y, int width, int height);   // fills a rectangle on the display

A BLOCKs program needs to provide a repaint() function which the block will call
at approximately 25Hz to draw the display. For example, here's a simple program that
draws a moving rectangle:

    int rectangleX;

    void repaint()
    {
        fillRect (0xff000044, 0, 0, 15, 15); // fill the display with dark blue
        fillRect (0xffffffff, rectangleX, 5, 4, 4); // draw a white rectangle

        rectangleX = (rectangleX + 1) % 15; // animate our position and make it wrap
    }

The host can also send simple event messages to the program, and to receive these you must
provide a function called "handleMessage", e.g.

    void handleMessage (int param1, int param2)
    {
        // do something with the two integer parameters that the app has sent...
    }

@endverbatim

