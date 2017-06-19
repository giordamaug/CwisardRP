//
//  main.cpp
//
//
//  Created by Maurizio Giordano on 18/02/14.
//
// the WISARD implementation for background extraction
//

// C++ headers
#include <getopt.h>
#include <dirent.h>
#include <sys/time.h>

#include <iostream>
#include <string>
#include <stdexcept>

// speific include libraries
#include "pixel_lib.hpp"
#include "wisard.hpp"
#include "wnet_lib.hpp"
#include "colorproc.hpp"
#include "cv_util.hpp"
#include "comparator.hpp"

// OpenCv include
#include <opencv2/highgui/highgui.hpp>

#ifdef OMP
#define partype "OpenMP"
#else
#define partype "Sequential"
#endif
#define getch() wgetch(stdscr)

using namespace cv;
using namespace std;

long int wcounter = 0;
long int hits = 0;
long int misses = 0;
int status = 0;
int cachesize = 10;
char ch;

string title = "CwisarDRP - Change Detection by WiSARDrp";


// Stats definitions
unsigned int tp, fp, fn, tn, nbShadowErrors;

string removeExtension(const string filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == string::npos) return filename;
    return filename.substr(0, lastdot);
}


int getdir(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    int cnt=0;
    
    if((dp = opendir(dir.c_str())) == NULL) {
        cout << "Error(" << -1 << ") wrong dir " << dir << endl;
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (dirp->d_name[0] != '.') {  /* ignore hidden files */
            files.push_back(string(dirp->d_name));
            cnt++;
        }
    }
    closedir(dp);
    
    if (cnt == 0) {
        cout << "Error(" << -2 << ") empty dir " << dir << endl;
        return -2;
    }

    return cnt;
}

void rgb2rgb(Mat in, Mat &out) {
    ;
}

void rgb2lab(Mat in, Mat &out) {
    cvtColor(in, out, CV_BGR2Lab);
}

void lab2rgb(Mat in, Mat &out) {
    cvtColor(in, out, CV_Lab2BGR);
}

void rgb2hsv(Mat in, Mat &out) {
    cvtColor(in, out, CV_BGR2HSV);
}

void hsv2rgb(Mat in, Mat &out) {
    cvtColor(in, out, CV_HSV2BGR);
}

void (*convert)(Mat, Mat &);
void (*backconvert)(Mat, Mat &);

int main(int argc,char **argv) {
    DIR *dp;
    
    register int n, i, j, k, neuron;
    int sum, den;
    
    // image processing globals
    int frameindex, iter;
    int wait=20;
    uchar tune;
    uchar *data, *odata, *roidata;
    uchar R,G,B;

    vector<Mat> imglist = vector<Mat>();
    // define input, output and gt images variables
    Mat frame, outframe, gtimg, roiimg;

    // wisard globals
    int n_ram, npixels, n_bit=16, nt=128;
    wisard_t *wiznet = (wisard_t *)NULL;
    wentry_t **discr;
    cache_entry_t *cache = (cache_entry_t *)NULL;
    
    pix_t **neigh_map, ***neigh_map_array;

    // command line option flags and globals
    int gtflag=0, verboseflag=0;
    bool roiflag= false;
    bool dumpflag = false;
    bool dmprangeflag = false;
    bool statsflag = false;
    int startframe=1, endframe=1000000;
    int dmpstart, dmpstop, statsstart, statsstop;
    
    // command line parsing globals
    char buffer[512];
    string s;
    int c;
    stringstream st;

    // timing globals
    struct timeval tv1, tv2, tv3, tv4;
    struct timeval tm1, tm2, tm3, tm4, tm5;
    struct timeval tg1, tg2, tg3, tg4;
    double t_f = 0.0, t_in = 0.0, t_out = 0.0, t_cl = 0.0, t_tr = 0.0, t_g = 0.0, t_tot = 0.0;
    double meanfps = 0.0, fps = 0.0;
    double recall, precision;
    
    // train/classify globals
    int traindecr=1, trainincr=1;
    stringstream ss;
    string token;
    float threshold2=0.50, response;
    int threshold1=30;

    // filter globals;
    bool filterflag = false, postfilterflag = false, postfilterflag2 = false;
    int filterX, filterY;
    CvSize BlurSize = cvSize(3,3);
    int erosion_elem = 0;
    int erosion_type=0, erosion_size = 0;
    int dilation_elem = 0;
    int dilation_type = 0, dilation_size = 0;
    int erosion_elem2 = 0;
    int erosion_type2=0, erosion_size2 = 0;
    int dilation_elem2 = 0;
    int dilation_type2 = 0, dilation_size2 = 0;
    int const max_elem = 2;
    int const max_kernel_size = 21;
    Mat erosionElement;
    Mat dilationElement;
    Mat erosionElement2;
    Mat dilationElement2;
    
    // set default directory for input
    string dumpdirname, indirname = ".";
    string gtdirname = ".";
    string roifilename = "./ROI.bmp";
    string colormode = "RGB";

    string filtername = "None";
    string postfiltername = "None";
    string postfiltername2 = "None";
    string colorname;
    string extension = "png";
    
    // color processing function & globals
    cache_entry_t *(*makeTupleCached)(cache_entry_t *, unsigned char, unsigned char, unsigned char, int, int, int, pix_t **);
    makeTupleCached = &makeTupleCachedRGB;
    convert = &rgb2rgb;
    backconvert = &rgb2rgb;
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    
    // parse command line arguments
    static struct option long_options[] =
    {
        /* These options set a flag. */
        //{"verbose",   no_argument,       &verboseflag, 'v'},
        /* These options don't set a flag.
         We distinguish them by their indices. */
        {"verbose", no_argument,       0, 'v'},
        {"roimode", no_argument,       0, 'R'},
        {"erodila", no_argument,       0, 'O'},
        {"dilaero", no_argument,       0, 'M'},
        {"blur", no_argument,       0, 'B'},
        {"path",  required_argument, 0, 'P'},
        {"help", no_argument,       0, 'h'},
        {"gtdir",  required_argument, 0, 'g'},
        {"roiimage",  required_argument,       0, 'r'},
        {"dir",  required_argument, 0, 'd'},
        {"dumpdir",  required_argument, 0, 'D'},
        {"colormode",  required_argument, 0, 'c'},
        {"stats",  required_argument, 0, 'S'},
        {"dumprange",  required_argument, 0, 'q'},
        {"nbits",    required_argument, 0, 'n'},
        {"ntics",    required_argument, 0, 'z'},
        {"startframe",    required_argument, 0, 's'},
        {"stopframe",    required_argument, 0, 'f'},
        {"train",    required_argument, 0, 'T'},
        {"classify",    required_argument, 0, 'C'},
        {"cachesize",    required_argument, 0, 'm'},
        {"framewait",    required_argument, 0, 'w'},
        {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    
    while ((c = getopt_long (argc, argv, "C:T:B:D:S:O:M:r:P:m:d:g:r:n:z:s:c:q:f:w:hv", long_options, &option_index)) != -1)
        switch (c)
    {
        // help
        case 'h':
			cout << "Usage:" << endl;
			cout << "    -h|--help\t\t\t\t: print this help" << argv[0] << endl;
			cout << "    -v|--verbose\t\t\t: verbose mode (print runtime info)" << endl;
			cout << "    -P|--path <dirpath>\t\t\t: input files directory (with gt and roi)" << endl;
			cout << "    -T|--train <decr>:<incr>\t\t: train policy [1:1]" << endl;
			cout << "    -C|--classify <thr1>:<thr2>\t\t: classification thresholds [1:0.75]" << endl;
			cout << "    -m|--cachesize <size>\t\t: chache size [10]" << endl;
			cout << "    -S|--stats <from>:<to>\t\t: compute TP, FP, TN, FN from_frame:to_frame" << endl;
			cout << "    -B|--blur <npix>:<npix>\t\t: input image gaussian blur filter [3:3]" << endl;
			cout << "    -r|--roiimage <filepath>\t\t: ROI image pathname [./ROI.bmp]" << endl;
			cout << "    -d|--dir <dirpath>\t\t\t: input images directory pathname [.]" << endl;
			cout << "    -g|--gtdir <dirpath>\t\t: groundtruth images directory pathname [.]" << endl;
			cout << "    -O|--erodila <type>:<size>:<type>:<size>: erosion-dilation type and size [1:1:1:1]" << endl;
            cout << "    -M|--dilaero <type>:<size>:<type>:<size>: dilation-erosion type and size [1:1:1:1]" << endl;
			cout << "    -D|--dump <dirpath>\t\t\t: dump bg images in dirpath" << endl;
			cout << "    -q|--dumprange <from>:<to>\t\t: range of frames to dump [startframe:endframe]" << endl;
			cout << "    -n|--nbits <nbits>\t\t\t: number of bits (positive inteneger upto 64) [16]" << endl;
            cout << "    -z|--scale <ntics>\t\t\t: number of tics in color discretizing (positive inteneger upto 256) [128]" << endl;
            cout << "    -c|--colormode RGB|HSV|LAB: set color mode [RGB]" << endl;
			cout << "    -s|--startframe <FrameIdx>\t\t: start frame index [0]" << endl;
			cout << "    -f|--endframe <FrameIdx>\t\t: end frame index [1000]" << endl;
			exit(-1);
            break;
        // capture mode
        case 'v':
            verboseflag = 1;
            break;
        // preprocessing filter
            // color mode (only RGB at the moment)
        case 'c':
            if (strcmp(optarg,"RGB") == 0) {
                colormode = "RGB";
                makeTupleCached = &makeTupleCachedRGB;
                convert = &rgb2rgb;
                backconvert = &rgb2rgb;
            } else if (strcmp(optarg,"HSV") == 0) {
                colormode = "HSV";
                makeTupleCached = &makeTupleCachedHSV;
                convert = &rgb2hsv;
                backconvert = &hsv2rgb;
            } else if (strcmp(optarg,"LAB") == 0) {
                colormode = "LAB";
                makeTupleCached = &makeTupleCachedRGB;
                convert = &rgb2lab;
                backconvert = &lab2rgb;
            } else {
                cout << "Supported color mode : RGB, HSV and Lab" << endl;
                exit(-1);
            }
            break;
        case 'B':
            if (!filterflag && sscanf(optarg, "%d:%d", &filterX,&filterY) == 2 && filterX > 0 && filterY > 0) {
                filterflag = true;
                st.str("");
                st << "GAUSS(" << filterX << "x" << filterY << ")";
                filtername = st.str();
                BlurSize = cvSize(filterX,filterY);
            } else {
                cerr << "Blur resolution must be \"<int>:<int>\"" << endl;
                exit(-1);
            }
            break;
        // portprocessinf filter
        case 'O':
            if (!postfilterflag && sscanf(optarg, "%d:%d:%d:%d", &erosion_type,&erosion_size,&dilation_type,&dilation_size) == 4
                && erosion_type > -1 && erosion_type < 3 && dilation_type > -1 && dilation_type < 3
                && erosion_size > -1 && dilation_size > -1) {
                postfilterflag = true;
            } else {
                cerr << "Erode-Dilation resolution must be \"<int>:<int>\"" << endl;
                exit(-1);
            }
            break;
        // portprocessinf filter
        case 'M':
            if (!postfilterflag2 && sscanf(optarg, "%d:%d:%d:%d", &erosion_type2,&erosion_size2,&dilation_type2,&dilation_size2) == 4
                && erosion_type2 > -1 && erosion_type2 < 3 && dilation_type2 > -1 && dilation_type2 < 3
                && erosion_size2 > -1 && dilation_size2 > -1) {
                postfilterflag2 = true;
            } else {
                cerr << "Dilation-Erosion resolution must be \"<int>:<int>\"" << endl;
                exit(-1);
            }
            break;
        // package dir pathname
        case 'P':
            dp = opendir (optarg);
            if (dp == NULL) {
                cout << "Could not open package dir" << endl;
                exit(-1);
            }
            st.str("");
            st << optarg;
            st << "/input";
            indirname = st.str();
            st.str("");
            st << optarg;
            st << "/groundtruth";
            gtdirname = st.str();
            roiflag = true;
            st.str("");;
            st << optarg;
            st << "/ROI.bmp";
            roifilename = st.str();
            break;
        // dump dir pathname
        case 'D':
            dumpflag = true;
            dumpdirname = optarg;
            dp = opendir (dumpdirname.c_str());
            if (dp == NULL) {
                cout << "Could not open dump dir" << endl;
                exit(-1);
            }
            break;
        // dump range
        case 'q':
            if (sscanf(optarg, "%d:%d", &dmpstart,&dmpstop) != 2 || dmpstart <=0 || dmpstop <= 0) {
                cout << "Wrong dump frame range (<fromFrame>:<toFrame>)" << endl;
                exit(-1);
            }
            dmprangeflag = true;
            break;
        // stats range
        case 'S':
            if (sscanf(optarg, "%d:%d", &statsstart,&statsstop) != 2 || statsstart <=0 || statsstop <= 0 || statsstop < statsstart) {
                cout << "Wrong stats range format (<fromFrame>:<toFrame>)" << endl;
                exit(-1);
            }
            statsflag = true;
            break;
        // threshold
        case 'C':
            if (sscanf(optarg, "%d:%f", &threshold1, &threshold2) != 2 || threshold1 < 0 || threshold2 <= 0.0) {
                cout << "Thresholds must be <non-negative int>:<non-negative real>" << endl;
                exit(-1);
            }
            break;
        // train policy
        case 'T':
            if (sscanf(optarg, "%d:%d", &traindecr,&trainincr) != 2 || traindecr <0 || trainincr <0) {
                cout << "Wrong train threshold values (<non-negative int>:<non-negative int>)" << endl;
                exit(-1);
            }
            break;
        // wait time between display
        case 'w':
            wait = atoi(optarg);
            if (wait > 1000) {
                cout << "Wait train is too large" << endl;
                exit(-1);
            }
            break;
        // wait time between display
        case 'm':
            cachesize = atoi(optarg);
            if (cachesize > MAXCACHESIZE) {
                cout << "Cache size too large" << endl;
                exit(-1);
            }
            break;
        // start frame index
        case 's':
            startframe = atoi(optarg);
            if (startframe <= 0) {
                cout << "Start frame must be positive" << endl;
                exit(-1);
            }
            break;
        // end frame index
        case 'f':
            endframe = atoi(optarg);
            if (endframe < 0) {
                cout << "End frame must be non zero" << endl;
                exit(-1);
            }
            break;
        // input dir pathname
        case 'd':
            indirname = optarg;
            break;
        // input ground truth pathname
        case 'g':
            gtflag = 1;
            gtdirname = optarg;
            break;
        // input ROI pathname
        case 'r':
            roiflag = true;
            roifilename = optarg;
            break;
        // number of wisard bits
        case 'n':
            n_bit = atoi(optarg);
            if (n_bit > 32) {
                cout << "number of bit must be at most 32" << endl;
                exit(-1);
            }
            break;
            // number of wisard bits
        case 'z':
            nt = atoi(optarg);
            if (nt > 256) {
                cout << "number of tics in scaling must be at most 256" << endl;
                exit(-1);
            }
            break;
        case '?':
            if (optopt == 'B'  || optopt == 'D' || optopt == 'S' || optopt == 'O' || optopt == 'M' || optopt == 'C' || optopt == 'T' ||
                optopt == 'g' || optopt == 'r' || optopt == 'P' || optopt == 'd' || optopt == 'm' || optopt == 'c' ||
                optopt == 'n' || optopt == 'z' || optopt == 'i' || optopt == 's' || optopt == 'q' || optopt == 'f' || optopt == 'w')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            exit(-1);
    }
    
    // option arguments dependency check
    if (dmprangeflag && !dumpflag){
        cout << "Dump range can be set only in Dump mode (-D|--dumpdir <dirpath>)" << endl;
        exit(-1);
    }
    if (dumpflag) {
        if (dmpstart < startframe) dmpstart = startframe;
        if (dmpstop > endframe) dmpstop = endframe;
    }
    // check stats range
    if (statsflag) {
        if (statsstart < startframe) statsstart = startframe;
        if (statsstop > endframe) statsstop = endframe;
    }

    // fill colorname name and params
    stringstream sstm;
    sstm << colormode << " (NT=" << nt << ")";
    colorname = sstm.str();

    // set postfilter parameters
    if (postfilterflag) {
        st.str("");
        st << "ERO(" << erosion_type << ":" << erosion_size << ") DILA(" << dilation_type << ":" << dilation_size << ")";
        postfiltername = st.str();
        erosionElement = getStructuringElement( erosion_type, Size( 2*erosion_size + 1, 2*erosion_size+1 ), Point( erosion_size, erosion_size ) );
        dilationElement = getStructuringElement( dilation_type, Size( 2*dilation_size + 1, 2*dilation_size+1 ), Point( dilation_size, dilation_size ) );
    }
    // set postfilter (by max) parameters
    if (postfilterflag2) {
        st.str("");
        st << "ERO(" << erosion_type2 << ":" << erosion_size2 << ") DILA(" << dilation_type2 << ":" << dilation_size2 << ")";
        postfiltername2 = st.str();
        erosionElement2 = getStructuringElement( erosion_type2, Size( 2*erosion_size2 + 1, 2*erosion_size2+1 ), Point( erosion_size2, erosion_size2 ) );
        dilationElement2 = getStructuringElement( dilation_type2, Size( 2*dilation_size2 + 1, 2*dilation_size2+1 ), Point( dilation_size2, dilation_size2 ) );
    }

    if (verboseflag) {
        cout << "Concurrency\t= " << partype << endl;
        cout << "N.Bit \t\t= " << n_bit << endl;
        cout << "Color \t\t= " << colormode << endl;
        cout << "Scaling tics:\t= " << nt << endl;
        cout << "Thresholds \t= " << threshold1 << ":" << threshold2 << endl;
        cout << "Start Fr.\t= " << startframe << endl;
        cout << "Stop Fr.\t= " << endframe << endl;
        cout << "Train decr/incr = " << traindecr << ":" << trainincr << endl;
        cout << "Input Dir \t= " << indirname << endl;
        cout << "GroundTruth Dir = " <<  gtdirname << endl;
        cout << "PostFilter \t= " << postfiltername << endl;
        cout << "PostFilter (2) \t= " << postfiltername2 << endl;
        if (roiflag) cout << "ROI filename \t= " <<  roifilename << endl;
    }
    
    int height,width,step,channels;
    int dcnt, gtcnt;
    vector<string> dlist = vector<string>();
    vector<string> gtlist = vector<string>();
    string filename, gtname;
    
    // set random seed
    srand(time(NULL));
    
    // set start frame index
    iter = startframe;
    frameindex = iter -1;
    
    // read image file list in given directory
    strcpy(buffer,indirname.c_str());
    
    if ((dcnt = getdir(buffer, dlist)) < 0) return -1;
    
    if (statsflag) {
        strcpy(buffer,gtdirname.c_str());
        // read groundtruth file list in given directory
        if ((gtcnt = getdir(buffer, gtlist)) < 0) return -1;
    }
    
    if (startframe > dcnt -1 ) {
        cout << "Start frame must be lesser than input dir files no." << endl;
        exit(-1);
    }
    
    den = (int) (256 / nt);
    if (endframe > dcnt) endframe = dcnt;
    frameindex = iter -1;
    
    // GET FIRST FRAME TO INITIALIZE WISARD
    gettimeofday(&tg1, NULL);
    filename =  indirname + "/" + dlist[frameindex];
    frame=imread(filename, 1);
    if(!frame.data ) {
        cout << "Could not load image file: " << dlist[frameindex] << endl;
        exit (0);
    }
    gettimeofday(&tg2, NULL);
    t_g += (double) (tg2.tv_usec - tg1.tv_usec)/1000000 + (double) (tg2.tv_sec - tg1.tv_sec);
    // load ROI image if stats are enabled
    if (roiflag) {
        roiimg=imread(roifilename, 0);
        if(!roiimg.data ) {
            cout << "Could not load ROI file: " << roifilename << endl;
            exit(0);
        }
    }
    
    /* inizialize wisard net */
    // get the image data
    height    = frame.rows;
    width     = frame.cols;
    channels  = frame.channels();
    // create the output image with same size
    outframe = frame.clone();
    npixels = width * height;
    wiznet = net_create(n_bit,width,height,colormode, nt);
    cout << "Processing a " << width << "x" << height;
    cout << " image with " << channels << " channels (" << npixels << " pixels)" << endl;
#ifdef OMP
    // alloc and init pixel info matrix mapping
    neigh_map_array = (pix_t ***) malloc(npixels * sizeof(pix_t **));
    if (colormode == "RGB" || colormode == "LAB")
        for (n=0;n<npixels;n++) {
            neigh_map_array[n] = neigh_mapping(wiznet->n_ram, wiznet->n_bit, wiznet->nt, 3);
        }
    else { // HSV
        for (n=0;n<npixels;n++) {
            neigh_map_array[n] = neigh_mapping(wiznet->n_ram, wiznet->n_bit, wiznet->nt, 2);
        }
    }
#endif
    
    /* TRAIN/CLASSIFY LOOP */
    while(iter <= endframe) {
        gettimeofday(&tg1, NULL);
        filename =  indirname + "/" + dlist[frameindex];
        frame=imread(filename, 1);
        if(!frame.data ) {
            cout << "Could not load image file: " << dlist[frameindex] << endl;
            exit(0);
        }
        //convert(frame, frame);
        gettimeofday(&tg2, NULL);
        t_g += (double) (tg2.tv_usec - tg1.tv_usec)/1000000 + (double) (tg2.tv_sec - tg1.tv_sec);
        gettimeofday(&tv1, NULL);
        // apply pre-processing filter
        if (filterflag) GaussianBlur(frame, frame, BlurSize, 0.0);
        gettimeofday(&tv2, NULL);
        t_f += (double) (tv2.tv_usec - tv1.tv_usec)/1000000 + (double) (tv2.tv_sec - tv1.tv_sec);
        // TRAIN WISARD
#ifdef OMP
#pragma omp parallel for schedule(dynamic,1) shared(den,neigh_map_array,wiznet,threshold1,threshold2,width,height,nt,frame,outframe,roiimg) private(sum,neuron,i,j,discr,neigh_map,cache,data,odata,roidata,R,G,B)
#else
        neigh_map = wiznet->neigh_map;
#endif
        for (j=0; j<height; j++) {
            data= frame.ptr<uchar>(j);
            odata= outframe.ptr<uchar>(j);
            if (roiflag) roidata= roiimg.ptr<uchar>(j);
            for (i=0; i<width; i++) {
                gettimeofday(&tm3, NULL);
                B = *data++; G = *data++; R = *data++;
                if (roiflag) {    // skip pixels out of roi
                    if (roidata[i] == (uchar)0) {
                        *odata++ = (uchar)85; // + tune;
                        *odata++ = (uchar)85; // + tune;
                        *odata++ = (uchar)85; // + tune;
                        gettimeofday(&tm4, NULL);
                        t_out += (double) (tm4.tv_usec - tm3.tv_usec)/1000000 + (double) (tm4.tv_sec - tm3.tv_sec);
                        continue;
                    }
                }
#ifdef OMP
                neigh_map = neigh_map_array[j*width + i];
#endif
                cache = wiznet->cachearray[j*width + i]; // get cache for pixel
                discr = wiznet->net[j*width + i]; // get discriminator of pixel
                // compute pixel info into binary matrix data
                gettimeofday(&tm1, NULL);
                cache = makeTupleCached(cache,R,G,B,den,nt,wiznet->n_ram,neigh_map);
                wiznet->cachearray[j*width + i] = cache;
                gettimeofday(&tm2, NULL);
                t_in += (double) (tm2.tv_usec - tm1.tv_usec)/1000000 + (double) (tm2.tv_sec - tm1.tv_sec);
                // CLASSIFY
                for (neuron=0, sum=0;neuron<wiznet->n_ram;neuron++) {
                    if (wram_get(discr[neuron],cache->tuple[neuron]) > threshold1) {
                        sum++;
                    }
                }
                gettimeofday(&tm3, NULL);
                t_cl += (double) (tm3.tv_usec - tm2.tv_usec)/1000000 + (double) (tm3.tv_sec - tm2.tv_sec);
                // OUTPUT
                if ((float)sum/(float)wiznet->n_ram >= threshold2) {
                    // set bgimage to black
                    *odata++ = (uchar)0; // + tune;
                    *odata++ = (uchar)0; // + tune;
                    *odata++ = (uchar)0; // + tune;
                    gettimeofday(&tm4, NULL);
                } else {  // if pixel is not BG ... set it to FG
                    *odata++ = (uchar)255;
                    *odata++ = (uchar)255;
                    *odata++ = (uchar)255;
                    gettimeofday(&tm4, NULL);
                }
                t_out += (double) (tm4.tv_usec - tm3.tv_usec)/1000000 + (double) (tm4.tv_sec - tm3.tv_sec);
                // TRAIN
                for (neuron=0;neuron<wiznet->n_ram;neuron++) {
                    //wram_set_or_incr(discr[neuron], cache->tuple[neuron],(wvalue_t)1,(wvalue_t)1);
                    wram_decr_all_but_key(discr[neuron], cache->tuple[neuron],(wvalue_t)trainincr,(wvalue_t)traindecr);
                }
                gettimeofday(&tm5, NULL);
                t_tr += (double) (tm5.tv_usec - tm4.tv_usec)/1000000 + (double) (tm5.tv_sec - tm4.tv_sec);
            }
        }
        gettimeofday(&tv3, NULL);
        // apply post filtering
        if (postfilterflag) {
            erode(outframe,outframe,erosionElement);
            dilate(outframe,outframe,dilationElement);
        }
        if (postfilterflag2) {
            dilate(outframe,outframe,dilationElement2);
            erode(outframe,outframe,erosionElement2);
        }
        gettimeofday(&tv4, NULL);
        t_f += (double) (tv4.tv_usec - tv3.tv_usec)/1000000 + (double) (tv4.tv_sec - tv3.tv_sec);
        fps = 1.0 / ((double) (tv4.tv_usec - tv1.tv_usec)/1000000 + (double) (tv4.tv_sec - tv1.tv_sec));
        meanfps += fps;
        
        // save the bg image into a file
        if (dumpflag && iter >= dmpstart && iter <= dmpstop) {
            s = removeExtension (dlist[frameindex]);
            filename = dumpdirname + "/b" + s + "." + extension;
            imwrite(filename,outframe,compression_params);
        }
        // measure outputs
        gettimeofday(&tg3, NULL);
        if (statsflag && iter >= statsstart && iter <= statsstop) {
            filename = gtdirname + "/" + gtlist[frameindex];
            gtimg=imread(filename, 1);
            if(!gtimg.data ) {
                cout << "Could not load groundtruth images: " << gtlist[frameindex] << endl;
                exit(-1);
            }
            compare(outframe, gtimg, roiimg);
            // display
            imglist.clear();
            backconvert(frame, frame);
            imglist.push_back(frame);
            imglist.push_back(outframe);
            imglist.push_back(gtimg);
        } else {
            // display (no groundtruth)
            imglist.clear();
            backconvert(frame, frame);
            imglist.push_back(frame);
            imglist.push_back(outframe);
        }
        gettimeofday(&tg4, NULL);
        t_g += (double) (tg4.tv_usec - tg3.tv_usec)/1000000 + (double) (tg4.tv_sec - tg3.tv_sec);
        t_tot = t_f + t_in + t_out + t_cl + t_tr + t_g;
        recall = tp / (double)(tp + fn);
        precision = tp / (double)(tp + fp);
        cvShowManyImages(title, wait, wiznet->n_bit, colorname, partype, dlist[frameindex], traindecr, trainincr, threshold1, threshold2, filtername, postfiltername, wcounter, cachesize, tp, fp, fn, tn, nbShadowErrors, recall, precision, (tn / (double)(tn + fp)), (2.0 * (recall * precision) / (recall + precision)), fps, meanfps / iter, t_g * 100 / t_tot, t_tr * 100 / t_tot, t_cl * 100 / t_tot, t_in * 100 / t_tot, t_out * 100 / t_tot, t_f * 100 / t_tot, hits * 100.0 / (misses + hits), imglist);
        iter++;
        frameindex = iter -1;
        
    } // end WHILE

    // print stats
    if (statsflag) {
        cout << "TP\tFP\tFN\tTN\t\tSE" << endl;
        cout << tp << "\t" << fp << "\t" << fn << "\t" << tn << "\t" << nbShadowErrors << endl;
        float recall = tp / (float)(tp + fn);
        cout << "Recall \t\t= " << recall << endl;
        cout << "Specificity \t= " << (tn / (float)(tn + fp)) << endl;
        cout << "Fpr \t\t= " << (fp / (float)(fp + tn)) << endl;
        cout << "Fnr \t\t= " << (fn / (float)(tn + fp)) << endl;
        cout << "Pbc \t\t= " << (100.0 * (float)(fn + fp) / (float)(tp + fp + fn + tn)) << endl;
        float precision = tp / (float)(tp + fp);
        cout << "Precision \t= " << precision << endl;
        cout << "FMeasure \t= " << (2.0 * (recall * precision) / (recall + precision)) << endl;
    }
}
