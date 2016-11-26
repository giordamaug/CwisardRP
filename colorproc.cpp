//
//  colorproc.cpp
//  
//
//  Created by Maurizio Giordano on 20/02/14.
//
//

#include "colorproc.hpp"

void codeHSV(float H, float S, float V, int *hi, int *hj, int *si, int *sj, int *vi, int *vj, int Nh) {
    int Ns = Nh / 2;
    int Nv = Nh / 2;
    float DTh = 90.0 / Nh;
    float DTs = 100.0 / (2*Ns - 1);
    float DTv = 100.0 / (2*Nv - 1);
    int vh, vs, vv;
    if ((H >= DTh) && (H < 360 - 3 * DTh)) {
        vh = (int)(H/(DTh*2) + 0.5);
        if (vh % 2 > 0) {
            *hi = (int)vh/2;
            *hj = (int)vh/2 +1;
        } else {
            *hi = (int)vh/2;
            *hj = (int)vh/2;
        }
    } else if ((H < DTh) || (H >= 360 - DTh)) {
        *hi = 0;
        *hj = 0;
    } else {
        *hi = 0;
        *hj = Nh-1;
    }
    if (S == 100.0) {
        *si = Ns-1;
        *sj = Ns-1;
    } else {
        vs = (int)(S/(DTs));
        if (vs % 2 > 0) {
            *si = (int)vs/2;
            *sj = (int)vs/2 +1;
        } else {
            *si = (int)vs/2;
            *sj = (int)vs/2;
        }
    }
    if (V == 100.0) {
        *vi = Nv-1;
        *vj = Nv-1;
    } else {
        vv = (int)(V/(DTv));
        if (vv % 2 > 0) {
            *vi = (int)vv/2;
            *vj = (int)vv/2 +1;
        } else {
            *vi = (int)vv/2;
            *vj = (int)vv/2;
        }
    }
}

void RGBtoHSV( int rr, int gg, int bb, float *h, float *s, float *v )
{
	float min, max, delta;
    float r = (float) rr / 255.0, g = (float) gg / 255.0, b = (float) bb / 255.0;
    min = max = r;
    if (g > max) {
        max = g;
    } else if (g < min) {
        min = g;
    }
    if (b > max) {
        max = b;
    } else if (b < min) {
        min = b;
    }
	*v = 100.0 * max;				// v
	delta = max - min;
	if( max != 0.0 )
		*s = 100.0 * delta / max;		// s
	else
		// r = g = b = 0		// s = 0, v is undefined
		*s = 0.0;
    
    if (*s == 0.0) {
        *h = 0.0;
    } else {
        if( r == max )
            *h = ( g - b ) / delta;		// between yellow & magenta
        else if( g == max )
            *h = 2 + ( b - r ) / delta;	// between cyan & yellow
        else
            *h = 4 + ( r - g ) / delta;	// between magenta & cyan
        *h *= 60.0;				// degrees
        if( *h < 0 )
            *h += 360.0;
    }
}

cache_entry_t *makeTupleRGB(cache_entry_t *cache, unsigned char R, unsigned char G, unsigned char B, int den, int nt, int nram, pix_t **neigh_map) {
    int cr = (int)(R / den);
    int cg = (int)(G / den);
    int cb = (int)(B / den);
    register int neuron;
    
    // initialize intuple
    for (neuron=0;neuron<nram;neuron++) cache->tuple[neuron] = 0;
    for (int k=0;k<nt;k++) {
        if (k<cr) cache->tuple[neigh_map[0][k].x] |= neigh_map[0][k].y;
        if (k<cg) cache->tuple[neigh_map[1][k].x] |= neigh_map[1][k].y;
        if (k<cb) cache->tuple[neigh_map[2][k].x] |= neigh_map[2][k].y;
    }
}

// Make tuple with Cache support
cache_entry_t *makeTupleCachedRGB(cache_entry_t *cache, unsigned char R, unsigned char G, unsigned char B, int den, int nt, int nram, pix_t **neigh_map) {
    cache_entry_t *p;
    int cr = (int)(R / den);
    int cg = (int)(G / den);
    int cb = (int)(B / den);
    register int neuron, k;
    int tmp, *ptr;
    // scan cache for hit or miss
    p = cache;
    try {
    for (;;) {
        if (p->cr == cr && p->cg == cg && p->cb == cb) {  // cache hit (move found in front)
            //printf("HIT:  ");
            hits++;
            if (p == cache) return cache;
            cache = cache->prev;
            tmp = cache->cr;
            cache->cr = cr;
            p->cr = tmp;
            tmp = cache->cg;
            cache->cg = cg;
            p->cg = tmp;
            tmp = cache->cb;
            cache->cb = cb;
            p->cb = tmp;
            ptr = cache->tuple;
            cache->tuple = p->tuple;
            p->tuple = ptr;
            return cache;
        }
        if (p->next == cache) {
            // move top on first non-empty
            //printf("MISS: ");
            misses++;
            cache = cache->prev;
            cache->cr = cr;
            cache->cg = cg;
            cache->cb = cb;
            // initialize tuple
            for (neuron=0;neuron<nram;neuron++) cache->tuple[neuron] = 0;
            // compute tuple
            for (k=0;k<nt;k++) {
                if (k<cr) cache->tuple[neigh_map[0][k].x] |= neigh_map[0][k].y;
                if (k<cg) cache->tuple[neigh_map[1][k].x] |= neigh_map[1][k].y;
                if (k<cb) cache->tuple[neigh_map[2][k].x] |= neigh_map[2][k].y;
            }
            return cache;
        }
        p = p->next;
    }
    } catch (const std::exception &e) {
        cout << e.what() << endl;
    }
}

cache_entry_t *makeTupleHSV(cache_entry_t *cache, unsigned char R, unsigned char G, unsigned char B, int den, int nt, int nram, pix_t **neigh_map) {
        float ch, cs, cv;
        int hi, hj, vi, vj, si, sj;
        RGBtoHSV( R, G, B, &ch, &cs, &cv );
        codeHSV(ch,cs,cv,&hi,&hj,&si,&sj,&vi,&vj,nt);
        cache->tuple[neigh_map[0][hi].x] |= neigh_map[0][hi].y;
        cache->tuple[neigh_map[0][hj].x] |= neigh_map[0][hj].y;
        cache->tuple[neigh_map[1][si].x] |= neigh_map[1][si].y;
        cache->tuple[neigh_map[1][sj].x] |= neigh_map[1][sj].y;
        cache->tuple[neigh_map[1][vi+nt/2].x] |= neigh_map[1][vi+nt/2].y;
        cache->tuple[neigh_map[1][vj+nt/2].x] |= neigh_map[1][vj+nt/2].y;
}

// Make tuple with Cache support
cache_entry_t *makeTupleCachedHSV(cache_entry_t *cache, unsigned char R, unsigned char G, unsigned char B, int den, int nt, int nram, pix_t **neigh_map) {
    float ch, cs, cv;
    int hi, hj, vi, vj, si, sj;
    cache_entry_t *p;
    register int neuron, k;
    int tmp, *ptr;
    // scan cache for hit or miss
    RGBtoHSV( R, G, B, &ch, &cs, &cv );
    codeHSV(ch,cs,cv,&hi,&hj,&si,&sj,&vi,&vj,nt);
    p = cache;
    try {
        for (;;) {
            if (p->cr == (int)R && p->cg == (int)G && p->cb == (int)B) {  // cache hit (move found in front)
                //printf("HIT:  ");
                hits++;
                if (p == cache) return cache;
                cache = cache->prev;
                tmp = cache->cr;
                cache->cr = (int)R;
                p->cr = tmp;
                tmp = cache->cg;
                cache->cg = (int)G;
                p->cg = tmp;
                tmp = cache->cb;
                cache->cb = (int)B;
                p->cb = tmp;
                ptr = cache->tuple;
                cache->tuple = p->tuple;
                p->tuple = ptr;
                return cache;
            }
            if (p->next == cache) {
                // move top on first non-empty
                //printf("MISS: ");
                misses++;
                cache = cache->prev;
                cache->cr = (int)R;
                cache->cg = (int)G;
                cache->cb = (int)B;
                // initialize tuple
                //for (neuron=0;neuron<nram;neuron++) cache->tuple[neuron] = 0;
                // compute tuple
                RGBtoHSV( R, G, B, &ch, &cs, &cv );
                codeHSV(ch,cs,cv,&hi,&hj,&si,&sj,&vi,&vj,nt);
                cache->tuple[neigh_map[0][hi].x] |= neigh_map[0][hi].y;
                cache->tuple[neigh_map[0][hj].x] |= neigh_map[0][hj].y;
                cache->tuple[neigh_map[1][si].x] |= neigh_map[1][si].y;
                cache->tuple[neigh_map[1][sj].x] |= neigh_map[1][sj].y;
                cache->tuple[neigh_map[1][vi+nt/2].x] |= neigh_map[1][vi+nt/2].y;
                cache->tuple[neigh_map[1][vj+nt/2].x] |= neigh_map[1][vj+nt/2].y;
                return cache;
            }
            p = p->next;
        }
    } catch (const std::exception &e) {
        cout << e.what() << endl;
    }
}
