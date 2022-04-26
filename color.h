/*A union is a handy shortcut for translating between data types in an explicit way 
without a lot of casting. A union is the size of the largest object in it.  Each entry is 
an object.  In COLOR_T we have the union of a long (four bytes) and four char, such that the
bytes of the long correspond to RGBX: Red, Green, Blue, eXtra (Max intensity in our use). 
This lets us easily operate on the notion of the color as a combined product of the hue
intensity and value, or with the individual elements of the composite.

Another way this concept could be handled is with a class.  This model may be explored
in a later version to compare the relative merits of each model.
*/

#ifndef COLOR_H  //These preprocessor directives let you include without worrying about dupes.
#define COLOR_H  //Once defined it doesn't need to be re-included.

typedef union COLOR_T
{
  unsigned long l;    //long representation.  It's bytes overlap the memory of the chars.
  byte c[4]; //byte representation. 0-255 intensity for R G B and a spare 
} COLOR;

#endif //COLOR_H
