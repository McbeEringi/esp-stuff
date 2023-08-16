#include <Arduino.h>

/*

Y
↑

7-------------------------------------------------
6                                                |
5                                                |-----
4                                                |-----
3                                                |-----
2                                                |-----
1                                                |
0 1 2 3 4 5 6 7  8 9 a b c d e f  g h i j k l m n  →X

*/

// >>> + - - -

// +
// reg A   B   C   D   E   F   G   H
// pin 3   22  6   19  9   16  12  13
// fun x4  x0  x5  x1  x6  x2  x7  x3

// -
// reg A   B   C   D   E   F   G   H   A   B   C   D   E   F   G   H
// pin 1   2   4   5   7   8   10  11  14  15  17  18  20  21  23  24
// fun y4g y4r y5g y5r y6g y6r y7g y7r y3r y3g y2r y2g y1r y1g y0r y0g

void setup(){}
void loop(){}