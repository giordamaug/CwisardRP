//
//  wisard.hpp
//  
//
//  Created by Maurizio Giordano on 18/01/13.
//
//

#ifndef _wisard_h
#define _wisard_h

#define PI 3.1415926535
// magic cache size is 10
#define setblack(x) { x = (char)0; }
#define setwhite(x) { x = (char)255; }

#include "wnet_lib.hpp"
#include <stdio.h>
#include <string>
#include <vector>

#define MAXCACHESIZE 1024

using namespace std;

typedef unsigned char uchar;
typedef struct wisard wisard_t;
typedef struct cache_entry cache_entry_t;

struct cache_entry {
    int *tuple;
    int cr;
    int cg;
    int cb;
    cache_entry_t *next;
    cache_entry_t *prev;
};

struct wisard {
    wentry_t ***net;
    cache_entry_t **cachearray;
    pix_t **input_map;
    pix_t **neigh_map;
    char **neigh_img;
    int *in_tuples;
    int n_ram;
    int n_bit;
    int nt;
    int neigh_w;
    int neigh_h;
} ;

wisard_t *net_create(int, int, int, string, int);
pix_t **neigh_mapping(int, int, int, int);
cache_entry_t *cacheinit(int);
void printCache(cache_entry_t *cache);
#endif
