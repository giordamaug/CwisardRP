//
//  cv_util.c
//  
//
//  Created by Maurizio Giordano on 22/01/13.
//  a set of opencv utility functions:
//  display multiple images in the same window
//

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
using namespace std;
using namespace cv;

void cvShowManyImages(string title, int wait, int n_bit,  string colormode,  string partype, string framename, int tdecr, int tincr, int threshold1, float threshold2, string filtertype, string postfiltertype, long int wcnt, int cachesize, unsigned int tp, unsigned int fp, unsigned int fn,  unsigned int tn, unsigned int nbShadowErrors, double recall, double precision, double specificity, double fmeasure, double fps, double mfps,
    double timeG, double timeT, double timeC, double timeM, double timeO, double timeP, double histperc,
    std::vector<cv::Mat> imglist) {
    
    int nArgs = imglist.size();
    // start point of text
    char text[256];
    Point pt1;
    Scalar blue(250,0,0);
    Scalar red(0,0,255);
    Scalar lightblue(217, 50, 70);
    Scalar yellow(255, 255, 0);
    Scalar darkred(0, 0, 127);
    Scalar green(127, 255, 0);
    Scalar darkgreen(69, 96, 0);
    Scalar brown(117, 87, 16);
    
    // Text variables
	double hscale = 1.0;
	double vscale = 0.8;
	double shear = 0.2;
	int thickness = 1;
	int line_type = 4;
    // set font
	int fontface = CV_FONT_HERSHEY_PLAIN;
    
    int size;
    int off, base;
    int i;
    int m, n;
    int x, y;
    
    // w - Maximum number of images in a row
    // h - Maximum number of images in a column
    int w, h;
    
    // scale - How much we have to resize the image
    float scale;
    int max;
    
    // If the number of arguments is lesser than 0 or greater than 12
    // return without displaying
    if(nArgs <= 0) {
        printf("Number of arguments too small....\n");
        return;
    }
    else if(nArgs > 12) {
        printf("Number of arguments too large....\n");
        return;
    }
    // Determine the size of the image,
    // and the number of rows/cols
    // from number of arguments
    else if (nArgs == 1) {
        w = h = 1;
        size = 300;
    }
    else if (nArgs == 2) {
        w = 2; h = 1;
        size = 600;
    }
    else if (nArgs == 3) {
        w = 3; h = 1;
        size = 600;
    }
    else if (nArgs == 3 || nArgs == 4) {
        w = 2; h = 2;
        size = 300;
    }
    else if (nArgs == 5 || nArgs == 6) {
        w = 3; h = 2;
        size = 200;
    }
    else if (nArgs == 7 || nArgs == 8) {
        w = 4; h = 2;
        size = 200;
    }
    else {
        w = 4; h = 3;
        size = 150;
    }
    
    // Create a new 3 channel image
    Mat DispImage(60 + size*h, 100 + size*w, CV_8UC3, Scalar::all(180));
    
    // Loop for nArgs number of arguments
    vector<Mat>::iterator iter = imglist.begin();
    vector<Mat>::iterator end = imglist.end();
    for (i = 0, m = 20, n = 20; iter != end; iter++, i++, m += (20 + size)) {
        
        // Get the Pointer to the IplImage
        Mat img = *iter;
        
        // Check whether it is NULL or not
        // If it is NULL, release the image, and return
        if(img.data == 0) {
            cerr << "Invalid arguments img " << i << endl;
            return;
        }
        
        // Find the width and height of the image
        x = img.cols;
        y = img.rows;
        
        // Find whether height or width is greater in order to resize the image
        max = (x > y)? x: y;
        
        // Find the scaling factor to resize the image
        scale = (float) ( (float) max / size );
        
        // Used to Align the images
        if( i % w == 0 && m!= 20) {
            m = 20;
            n+= 20 + size;
        }
        
        // Set the image ROI to display the current image
        try {
            Rect ROI = Rect(m, n, (int)( x/scale ), (int)( y/scale ));
            resize(img, img, Size((int)( x/scale ), (int)( y/scale )), 50, 50, INTER_CUBIC);
            img.copyTo(DispImage(ROI));
        } catch (const std::exception &e) {
            std::cerr << e.what();
            cout << "Error in Display ... wor king in silent mode" << endl;
        }
    }
    
    // put text
    base = 20;
    off = (int)( y/scale ) + 50;
    pt1 = Point(base,off);
    sprintf(text, "VIDEO INFO:");
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    off +=20;
    pt1 = Point(base,off);
    sprintf(text, "Display Scale: %f", 1/scale);
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Frame: %s", framename.c_str());
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Image Size: %dx%d", x,y);
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Pre Filter: %s", filtertype.c_str());
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Post Filter: %s", postfiltertype.c_str());
    putText(DispImage,text,pt1,fontface,1.0,darkgreen);
    
    // second column wisard status
    base = 350;
    off = (int)( y/scale ) + 50;
    pt1 = Point(base,off);
    sprintf(text, "WISARD STATUS:", tp);
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=20;
    pt1 = Point(base,off);
    sprintf(text, "Train decr/incr: %d:%d", tdecr, tincr);
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "N. Bits: %d", n_bit);
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Thresholds: %d:%.2f", threshold1,threshold2);
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Color mode: %s", colormode.c_str());
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Concurrency: %s", partype.c_str());
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Cache Size: %d", cachesize);
    putText(DispImage,text,pt1,fontface,1.0,darkred);
    
    // third column (Stats)
    base = 650;
    off = (int)( y/scale ) + 50;
    pt1 = Point(base,off);
    sprintf(text, "TIMING:", tp);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=20;
    pt1 = Point(base,off);
    sprintf(text, "GRAPHICS = %2.2f %%", timeG);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "TRAIN = %2.2f %%", timeT);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "CLASSIFY = %2.2f %%", timeC);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "MKTUPLE = %2.2f %%", timeM);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "OUTPUT = %2.2f %%", timeO);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "FILTER = %2.2f %%", timeP);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Wnet Size: %d K", (int)wcnt >> 10);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "FPS (Mean) = %.2f (%.2f)", fps, mfps);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Cache Hits = %2.2f %%", histperc);
    putText(DispImage,text,pt1,fontface,1.0,blue);
    
    // third column (Stats)
    base = 1000;
    off = (int)( y/scale ) + 50;
    pt1 = Point(base,off);
    sprintf(text, "METRICS:", tp);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=20;
    pt1 = Point(base,off);
    sprintf(text, "TP = %d", tp);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "FP = %d", fp);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "FN = %d", fn);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "TN = %d", tn);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "SE = %d", nbShadowErrors);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Recall   = %1.4f", recall);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Precisio = %1.4f", precision);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Specific = %1.4f", specificity);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    off +=15;
    pt1 = Point(base,off);
    sprintf(text, "Fmeasure = %1.4f", fmeasure);
    putText(DispImage,text,pt1,fontface,1.0,brown);
    
    // Create a new window, and show the Single Big Image
    imshow( title, DispImage);
    
    cvWaitKey(wait);
    
}