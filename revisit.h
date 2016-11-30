/********************************************
  The head file for revisit.c
*********************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "config.h"
#include "contact.h"
//#include "nodes.h"

// read the Hotspot position from file to a List
struct Record_List * read_hotspot (FILE *fp)
{
	char s[128];
	double lg, lt;
	int lg_f=1, lt_f=2; // The 1st field of hotspot file is logitude, 2nd field is latitude
	char *str, *token; char *delim="\t "; char *saveptr;
	int i; 
	struct Record_List *H, *P, *N;
	H = malloc (sizeof(struct Record_List));
	P = H;  P->prev = NULL;
	while (fgets(s, 128, fp)) {
            for (i=1, str=s; ; i++, str=NULL) {
                token = strtok_r (str, delim, &saveptr);
                if (token == NULL) break;
                if (i == lg_f) P->gps.Lon = atof (token);
                if (i == lt_f) P->gps.Lat = atof (token);
		               P->gps.utime = 0;
            }
            N = malloc (sizeof(struct Record_List));
            N->prev = P;  P->next = N; P = N; 
        }
	P = N->prev; P->next = NULL; // the last malloced N is not used, free it
	free (N);
    return H;
}

/*******************************************************/
// get the first three effective position of a fraction number
#define CUT_FRAC(x) (double)((int)(1000*x))/1000
/*******************************************************/
// cutdown the records that is in the same grid; each grid size is 0.001X0.001;
// here uses a simple method but not the exacted method
int cutdown_record_with_grid (struct Record_List * L)
{
	struct Record_List *P, *N, *M;
	P = L;
	while (P->next) {
		N = P->next;
		if (P->gps.Lon - N->gps.Lon > 0.001 || P->gps.Lat - N->gps.Lat > 0.001 ||
		    P->gps.Lon - N->gps.Lon < -0.001 || P->gps.Lat - N->gps.Lat < -0.001) {
			P->gps.Lon = CUT_FRAC (P->gps.Lon);
			P->gps.Lat = CUT_FRAC (P->gps.Lat);
			P = P->next; 
		} else {  // if the succeed record is in the same grid with precedent, abandon it
			if (N->next) { 
				M = N;
				N = N->next;  P->next = N;  N->prev = P; 
				M->next = NULL;  M->prev = NULL; free (M);
			} else { 
				P->next = NULL;
				free (N);
			}
		}
	}
	P->gps.Lon = CUT_FRAC (P->gps.Lon);
	P->gps.Lat = CUT_FRAC (P->gps.Lat);
	return 0;
}

/*******************************************************/
// if a trace record is accordance with any hotsopt, take out it;
int revisit_hotspot (struct Record_List * Trace, struct Record_List * Hotspot, FILE *fp)
{
	struct Record_List *T, *H;
	int i=0, j=0;
	T = Trace;
	while (T) {
		H = Hotspot;
		while (H) { 
		/*	if (T->gps.Lon - H->gps.Lon < 0.001 && T->gps.Lon - H->gps.Lon > -0.001 &&
			    T->gps.Lat - H->gps.Lat < 0.001 && T->gps.Lat - H->gps.Lat > -0.001) {  */
			if (T->gps.Lon == H->gps.Lon && T->gps.Lat == H->gps.Lat) {
				if (H->gps.utime == 0) H->gps.utime = T->gps.utime;
				fprintf (fp, "%f %f %ld %ld\n", T->gps.Lon, T->gps.Lat, 
					 T->gps.utime, T->gps.utime - H->gps.utime);
					//H->gps.utime record the last visited one time
				H->gps.utime = T->gps.utime; 
			}
			H = H->next;
		}
		T = T->next;
	}
	return 0;
}

/*******************************************************/
// get the waiting time of a mobile node to arrive at specific spot, 
// or the earliest time arrive at any of a set of spots
long int wait_time_to_a_hotspot (struct Record_List * Trace, struct Record_List * Hotspot, long int time)
{
	struct Record_List *T, *H;
	T = Trace;
	long int wait = 0;;
	while (T) {
		H = Hotspot; 
		while (H) {
			if (/*T->gps.Lon - H->gps.Lon <= 0.001 && T->gps.Lon - H->gps.Lon >= -0.001 &&
		  	    T->gps.Lat - H->gps.Lat <= 0.001 && T->gps.Lat - H->gps.Lat >= -0.001*/
                            geo_distance(T->gps, H->gps) < RADIO_RANGE) {
				if (T->gps.utime > time) {
					if (wait==0) wait = T->gps.utime - time;
					else if (wait > T->gps.utime - time) wait = T->gps.utime - time;
				}
			}
			H = H->next;
		}
		T = T->next;	
	}

	return wait;
}


