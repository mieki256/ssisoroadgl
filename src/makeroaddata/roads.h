#ifndef __ROADS_H__
#define __ROADS_H__

typedef struct roaddata
{
  double cx;  // center x
  double cy;  // center y
  double rx0; // road edge x0
  double ry0; // road edge y0
  double rx1; // road edge x1
  double ry1; // road edge y1
  double lx0; // white line x0
  double ly0; // white line y0
  double lx1; // white line x1
  double ly1; // white line y1
  int tfg;    // tree flag
  int col;    // tree color index
  double tx;  // tree x
  double ty;  // tree y
  double r;   // tree size
} ROADDATA;

#endif
