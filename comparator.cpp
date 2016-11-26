//
//  comparator.c
//  
//
//  Created by Maurizio Giordano on 22/02/13.
//
//

#include <stdio.h>
#include "comparator.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

extern unsigned int tp, fp, fn, tn, nbShadowErrors;
const uchar BLACK = 0;
const uchar WHITE = 255;
const uchar UNKNOWN = 170;
const uchar SHADOW = 50;

void compare(Mat binary,  Mat gtimage, Mat roiimage) {
	register int x, y;
    
    for( y=0; y<binary.rows; y++ ) {
        uchar* ptr = (uchar*) binary.ptr<uchar>(y);
        uchar* gtr = (uchar*) gtimage.ptr<uchar>(y);
        uchar* roitr = (uchar*) roiimage.ptr<uchar>(y);
        for( x=0; x<binary.cols; x++ ) {
            //if (gtr[3*x+2]) printf("IN=%d GT=(%d,%d,%d)\n", ptr[3*x], gtr[3*x+2], gtr[3*x+1], gtr[3*x]);
            if (gtr[3*x] != UNKNOWN && roitr[x] != BLACK) {
                if (ptr[3*x] == WHITE) { // Model thinks pixel is foreground
                    if (gtr[3*x] == WHITE) {
                        ++tp; // and it is
                    } else {
                        ++fp; // but it's not
                    }
                } else { // Model thinks pixel is background
                    if (gtr[3*x] == WHITE) {
                        ++fn; // but it's not
                    } else {
                        ++tn; // and it is
                    }
                }
                if (gtr[3*x] == SHADOW) {
                    if (ptr[3*x] == WHITE) {
                        ++nbShadowErrors;
                    }
                }
            }
            
        }
    }
}