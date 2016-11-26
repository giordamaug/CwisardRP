//
//  pixel_lib.cpp
//  
//
//  Created by Maurizio Giordano on 18/01/13.
//
// a library for image creation, initialization and print
// support RGB color images, greyscale images
// include also:
//  - random generator
//  - vector print function: [v1,v2, ...,vn]
//  - pixels print: [(x1,x2),(x2,y2), ...,(xn,yn)]
//

#include <time.h>
#include <stdlib.h>
//#include <highgui.h>
#include "pixel_lib.hpp"


// pixels utility functions
pix_t newpix(int x, int y) {
    return (pix_t){.x = x, .y = y};
}

pix_t cpypix(pix_t p) {
    return (pix_t){.x = p.x, .y = p.y};
}

void setpix(pix_t *p, int x, int y) {
    p->x = x; p->y = y;
}

void pix_print(pix_t p) {
    printf("(%d,%d)", p.x, p.y);
}

// color utility functions
color_t newcolor(int r, int g, int b) {
    return (color_t){.r = r, .g = g, .b = b};
}

color_t cpycolor(color_t c) {
    return (color_t){.r = c.r, .g = c.g, .b = c.b};
}

void setcolor(color_t *c, int r1, int g1, int b1) {
    c->r = r1;
    c->g = g1;
    c->b = b1;
}

void setcol(color_t c, int r1, int g1, int b1) {
    c.r = r1;
    c.g = g1;
    c.b = b1; 
}

void color_print(color_t c) {
    printf("(%d,%d,%d)", c.r, c.g, c.b);
}

// array of pixel print function
void pixels_print(pix_t *p, int dim) {
    int i;
    if (p == NULL) {
        printf("[]\n");
    } else {
        printf("[");
        for(i=0;i<dim;i++) {
            printf("(%d,%d)", p[i].x, p[i].y);
            
            //pix_print(p[i]);
            if (i < dim -1) printf(",");
        }
        printf("]");
    }
}

// create Black and White image functions (+ utilities)
char **create_bwimage(int x, int y) {
    char **image;
    int i;
    
    image = (char **)malloc(x * sizeof(char *));
    for (i=0; i<x; i++)
        image[i] = (char *)malloc(y * sizeof(char));
    return image;
}

void bwimage_print(char **image, int x, int y) {
    int i,j;
    int val;
    printf("(");
    for (i = 0; i < x; i++) {
        if (i!=0) printf(" ");
        for (j = 0; j < y; j++) {
            val = (char)image[j][i] == (char)255 ? 1 : 0;
            printf("%d", val);
            if (j != y -1) printf(",");
        }
        //else printf("\n");
        if (i != x -1 ) printf("\n");
    }
    printf(")");
}

void bwimage_fg(char **image, int x, int y) {
    int i,j;
    for (i = 0; i < x; i++) {
        for (j = 0; j < y; j++) {
            image[i][j] = 255;
        }
    }
}

void bwimage_rnd(char **image, int x, int y) {
    int i,j;
    for (i = 0; i < x; i++) {
        for (j = 0; j < y; j++) {
            image[i][j] = rand() % (2);
        }
    }
}

// create Colored (RGB) image functions (+ utilities)
color_t **create_colorimage_white(int w, int h) {
    color_t **image;
    register int i,j;
    
    image = (color_t **)malloc(h * sizeof(color_t *));
    for (i=0; i<h; i++) {
        image[i] = (color_t *)malloc(w * sizeof(color_t));
        for (j = 0; j < w; j++) {
            image[i][j].r = 255;
            image[i][j].g = 255;
            image[i][j].b = 255;
        } 
    }
    return image;
}

// init color image to white
color_t **set_colorimage_white(color_t **image, int w, int h) {
    register int i,j;
    
    for (i=0; i<h; i++) {
        for (j = 0; j < w; j++) {
            image[i][j].r = 255;
            image[i][j].g = 255;
            image[i][j].b = 255;
        }
    }
    return image;
}

// init color image to white
color_t **set_colorimage_black(color_t **image, int w, int h) {
    register int i,j;
    
    for (i=0; i<h; i++) {
        for (j = 0; j < w; j++) {
            image[i][j].r = 0;
            image[i][j].g = 0;
            image[i][j].b = 0;
        }
    }
    return image;
}


color_t **set_colorimage_rnd(color_t **image, int w, int h) {
    register int i,j;
    
    for (i=0; i<h; i++) {
        for (j = 0; j < w; j++) {
            image[i][j].r = rand() % (256);
            image[i][j].g = rand() % (256);
            image[i][j].b = rand() % (256);
        }
    }
    return image;
}

color_t **create_colorimage_black(int w, int h) {
    color_t **image;
    register int i,j;
    
    image = (color_t **)malloc(h * sizeof(color_t *));
    for (i=0; i<h; i++) {
        image[i] = (color_t *)malloc(w * sizeof(color_t));
        for (j = 0; j < w; j++) {
            image[i][j].r = 0;
            image[i][j].g = 0;
            image[i][j].b = 0;
        }
    }
    return image;
}

color_t **create_colorimage_rnd(int w, int h) {
    color_t **image;
    register int i,j;
    
    image = (color_t **)malloc(h * sizeof(color_t *));
    for (i=0; i<h; i++) {
        image[i] = (color_t *)malloc(w * sizeof(color_t));
        for (j = 0; j < w; j++) {
            image[i][j].r = rand() % (256);
            image[i][j].g = rand() % (256);
            image[i][j].b = rand() % (256);
        }
    }
    return image;
}



void colorimage_print(color_t **image, int w, int h) {
    int i,j;
    printf("(");
    for (i = 0; i < h; i++) {
        if (i!=0) printf(" ");
        for (j = 0; j < w; j++) {
            color_print(image[i][j]);
            if (j != w -1 ) printf(",");
        }
        if (i != h -1 ) printf("\n");
    }
    printf(")");
}

void colorimage_rnd(color_t **image, int x, int y) {
    int i,j;
    for (i = 0; i < x; i++) {
        for (j = 0; j < y; j++) {
            image[i][j].r = rand() % (256);
            image[i][j].g = rand() % (256);
            image[i][j].b = rand() % (256);
        }
    }
}

void free_colorimage(color_t **image, int x, int y) {
    color_t p;
    int i,j;
    for (i = y-1; i >= 0; i--) {
            free(image[i]);
    }
    free(image); image = NULL;  
}

void array_print(unsigned int *vector, int x) {
    int i;
    printf("(");
    for (i = 0; i < x; i++) {
        printf("%d", vector[i]);
        if (i != x -1) printf(",");
    }
    printf(")");
}

void intuple_print(int *vector, int x) {
    int i;
    printf("[");
    for (i = 0; i < x; i++) {
        if (vector[i]) printf("%d", vector[i]);
        if (i != x -1) printf(".");
    }
    printf("]\n");
}

// random number generator
unsigned int randr(unsigned int min, unsigned int max) {
    double scaled = (double)rand()/RAND_MAX;
    return (max - min +1)*scaled + min;
}
