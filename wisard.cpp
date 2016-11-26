//
//  wisard.cpp
//
//
//  Created by Maurizio Giordano on 18/01/13.
//
// the WISARD implementation for background extraction
//

#include "pixel_lib.hpp"
#include "wnet_lib.hpp"
#include "wisard.hpp"
#include <iostream>
#include <string>
using namespace std;

int mypowers[32] = {
    1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2028, 4096, 8192, 16384, 32768, 65536, 131072 ,
    262144 , 524288 , 1048576 , 2097152 , 4194304 , 8388608 , 16777216 , 33554432 , 67108864 , 134217728 , 268435456 , 536870912 , 1073741824 ,
    2147483648 ,
};

extern int cachesize;

extern int status;
extern const uchar BLACK;
extern const uchar SHADOW;

cache_entry_t *cacheinit(int nram) {
    cache_entry_t *top, *newp;
    // create first element;
    newp = (cache_entry_t *)malloc(sizeof(cache_entry_t));
    newp->cr = -1;
    newp->cg = -1;
    newp->cb = -1;
    newp->tuple = (int *)malloc(nram * sizeof(int));
    newp->next = newp;
    newp->prev = newp;
    top = newp;
    
    // add other element;
    for (int i=1; i < cachesize; i++) {
        newp = (cache_entry_t *)malloc(sizeof(cache_entry_t));
        newp->cr = -1;
        newp->cg = -1;
        newp->cb = -1;
        newp->tuple = (int *)malloc(nram * sizeof(int));
        newp->next = top;
        newp->prev = top->prev;
        (top->prev)->next = newp;
        top->prev = newp;
    }
    return top;
}

void printCache(cache_entry_t *cache) {
    cache_entry_t *p;
    if (cache == (cache_entry_t *)NULL) {
        printf("Cache empyty\n");
        return;
    }
    printf("{");
    for (p=cache;p->next != cache; p=p->next ) {
        printf("[%d,%d,%d] ", p->cr, p->cg, p->cb);
    }
    printf("}\n");
}

// net structure (+utilities)
wisard_t *net_create(int n_bit, int im_width, int im_height, string colormode, int nt) {
    register int i, j;
    int npixels;
    int nloc= mypowers[n_bit];
    
    wisard_t *n;
    n=(wisard_t *)malloc(sizeof(wisard_t));
    n->n_bit = n_bit;
    
    // mode delta (differenze)
    if (colormode == "RGB" || colormode == "LAB") {
        n->nt = nt;
        // compute number of rams
        if ((n->nt * 3) % n_bit == 0)
            n->n_ram = (int)((n->nt * 3) / n_bit);
        else
            n->n_ram = (int)((n->nt * 3) / n_bit) + 1;
        // compute neighbour mapping
        n->neigh_map = neigh_mapping(n->n_ram, n->n_bit, n->nt, 3);
        n->neigh_w = 3;
        n->neigh_h = nt;
    } else if (colormode == "HSV") {
        n->nt = nt;
        // compute number of rams
        if ((n->nt * 2) % n_bit == 0)
            n->n_ram = (int)((n->nt * 2) / n_bit);
        else
            n->n_ram = (int)((n->nt * 2) / n_bit) + 1;
        // compute neighbour mapping
        n->neigh_map = neigh_mapping(n->n_ram, n->n_bit, n->nt, 2);
        n->neigh_w = 2;
        n->neigh_h = nt;
    } else {
        cout << "Error: color " << colormode << " mode not supported!" << endl;
    }
    // creates the neural network list.
    // each neuron is a hash-table, and a
    // discriminator is a list of neurons
    npixels = im_width * im_height;
    wentry_t ***net;
    net = (wentry_t ***)malloc(npixels * sizeof(wentry_t **));
    // alloc tuple cache
    cache_entry_t **cache;
    cache = (cache_entry_t **)malloc(npixels * sizeof(cache_entry_t *));
    for (i = 0; i < npixels; i++) {
        net[i] = (wentry_t**)malloc(n->n_ram * sizeof(wentry_t*));
        cache[i] = (cache_entry_t *)cacheinit(n->n_ram);
        for (j=0; j < n->n_ram; j++) {
            net[i][j] = wram_create();
        }
    }

    n->net = net;
    n->cachearray = cache;

    return n;
}

// Start wisard procedures
void neighmap_print(pix_t **input_map, int w, int h) {
    int i;
    
    printf("[");
    for (i=0; i<h;i++) {
        pixels_print(input_map[i],w);
        if (i != h-1) printf(",\n");
    }
    printf("]");
}


// returns a random element of a sequence,
// consuming it. Heavily optimized version
pix_t select(pix_t *seq, int *dim) {
    int pos;
    pix_t elem;
    if (*dim == 1) {
        //printf("LAST ");
        *dim = *dim - 1;
        return seq[*dim];
    } else if (*dim > 1) {
        pos = rand() % (*dim-1);
        //printf("POS %d ",pos);
        elem = seq[pos];
        seq[pos] = seq[*dim-1];
        seq[*dim-1] = elem;
        *dim = *dim - 1;
        //free(seq[*dim-1]);
        return elem;
    } else {
        return newpix(-1,-1);
    }
}

// random neighbor_mapping creation:
pix_t **neigh_mapping(int n_ram, int n_bit, int w, int h) {
    register int i,j;
    pix_t *pixels, *tmp;
    pix_t **input_map;
    int npixels;
    
    // init source pixels (ram_index, bit_index)
    npixels = n_ram * n_bit;
    pixels = (pix_t*)malloc(npixels * sizeof(pix_t));
    for (i = 0; i < n_ram; i++)
        for (j=0; j < n_bit; j++)
            pixels[i * n_bit + j] = newpix(i,mypowers[j]);
    
    // create as many full neuron mappings as
    // possible with the existing pixels
    input_map = (pix_t**)malloc(h * sizeof(pix_t*));
    for (i=0; i<h;i++) {
        tmp = (pix_t*)malloc(w * sizeof(pix_t));
        for (j=0;j<w;j++) {
            //tmp[j] = newpix(pixels[i * bits + j].x,pixels[i * bits + j].y);
            tmp[j] = select(pixels,&npixels);
        }
        input_map[i] = tmp;
    }
    return input_map;
}


// create a dynamic array of int (ram entries)
int *wnet_create(int size) {
    int i;
    int *m;
    m=(int *)malloc(size * sizeof(int));
    return m;
}

// discriminator print utility functions
void wnet_print(int *m, int size) {
    int i;
    
    printf("{");
    for (i=0;i<size;i++) {
        if (m[i]>0) {
            printf("%d:%d", i, m[i]);
            if (i!=size-1) printf(" ");
        }
    }
    printf("}");
}

void wnet_print2(int *m, int size) {
    int i;
    
    printf("{");
    for (i=0;i<size;i++) {
        printf("%d:%d", i, m[i]);
        if (i!=size-1) printf(" ");
    }
    printf("}");
}

// wisard print function
void print_net(int ***net, int dim, int n_ram, int n_loc) {
    int x,y;
    printf("[");
    for (x = 0; x < dim; x++) {
        if (x!=0) printf(" ");
        printf("<");
        for (y = 0; y < n_ram; y++)
            wnet_print(net[x][y], n_loc);
        printf(">");
        if (x!=dim -1) printf(",\n");
    }
    printf("]");

}

// count non-zero ram entries
int wnet_len(int *m, int size) {
    int i,cnt=0;
    
    for (i=0;i<size;i++) {
        if (m[i]>0) cnt++;
    }
    return cnt;
}

void net_free(int ***net, int dim, int n_ram) {
    int i, j;
    for (i = 0; i < dim; i++) {
        for (j=0; j < n_ram; j++) {
            free(net[i][j]);
            net[i][j] = NULL;
        }
        free(net[i]);
    }
    free(net);
}

