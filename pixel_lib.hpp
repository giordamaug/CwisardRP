//
//  pixel_lib.hpp
//  
//
//  Created by Maurizio Giordano on 18/01/13.
//
// Header file of pixel_lib.c
//

#ifndef _pixel_lib_h
#define _pixel_lib_h
//#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>

// pixels coordimate structure (x,y)
typedef struct pixel pix_t;

struct pixel {
    int x;
    int y;
};

// pixels color structure (r,g,b)
typedef struct color {
    int r;
    int g;
    int b;
} color_t;

// pixel utility functions
pix_t newpix(int x, int y);
pix_t cpypix(pix_t p);
void pix_print(pix_t p);
void setpix(pix_t *p, int x, int y);

// color utility functions
color_t newcolor(int r, int g, int b);
color_t cpycolor(color_t c);
void color_print(color_t c);
void setcolor(color_t *c, int r, int g, int b);
void setcol(color_t c, int r, int g, int b);

// array of pixel print function
void pixels_print(pix_t *p, int dim);
void array_print(unsigned int *vector, int x);
void intuple_print(int *vector, int x);

// create Black and White image functions (+ utilities)
char **create_bwimage(int x, int y);
void bwimage_print(char **image, int x, int y);
void bwimage_fg(char **image, int x, int y);
void bwimage_rnd(char **image, int x, int y);

// create Colored (RGB) image functions (+ utilities)
color_t **create_colorimage_white(int x, int y);
color_t **set_colorimage_white(color_t **, int, int);
color_t **set_colorimage_black(color_t **, int x, int y);
color_t **set_colorimage_rnd(color_t **, int x, int y);
color_t **create_colorimage_black(int x, int y);
color_t **create_colorimage_rnd(int x, int y);
void free_colorimage(color_t **image, int x, int y);
void colorimage_print(color_t **image, int x, int y);
void colorimage_rnd(color_t **image, int x, int y);

// random number generator
unsigned int randr(unsigned int min, unsigned int max);
#endif


