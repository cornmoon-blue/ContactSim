#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "contact.h"
#include "config.h"

/***********************************************************************/

// fp: record data file; lg_f,lt_f,tm_f: the field of logitide, latitude, time
// time_bound is the record range to be read into memory; if ZERO, the range is not limited
struct Record_List * read_gps_record (FILE *fp, int lg_f, int lt_f, int tm_f, long int time_bound) 
{
	char s[256];
	double lg, lt; long int time;
	char *str, *token; char *delim="\t "; char *saveptr;
	int i;
	struct Record_List *Rec_Head, *R, *P, *N;

	Rec_Head = malloc (sizeof(struct Record_List));
	P = Rec_Head; Rec_Head->prev = NULL;

	while (fgets(s, 256, fp)) {
	    for (i=1, str=s; ; i++, str=NULL) {
		token = strtok_r (str, delim, &saveptr);
		if (token == NULL) break;
		if (i == lg_f) lg = atof (token);//R->gps.Lon = atof (token);
		if (i == lt_f) lt = atof (token);//R->gps.Lat = atof (token);
		if (i == tm_f) time = atol (token);//R->gps.utime = atol (token);
	    }
	    if (time_bound) {
	        if (time > time_bound-120 && time < time_bound+RECORD_TIME_RANGE*3600+120) 
	        { // if a patterned record, store it
		    P->gps.Lon = lg; P->gps.Lat = lt; P->gps.utime = time;
  	            N = malloc (sizeof(struct Record_List));
	            P->next = N; N->prev = P; P = N;
	        } else continue;
	    } else {
		P->gps.Lon = lg; P->gps.Lat = lt; P->gps.utime = time;
		N = malloc (sizeof(struct Record_List));
		P->next = N; N->prev = P; P = N;
	    }
	}
	// beacuse there may be no records fall in the range in trace file, so return NULL.
	if(P == Rec_Head) { free(P); return NULL; } else {
		P=P->prev; P->next = NULL; free (N);
	}

	// Reverse the sequence For Cabspotting
	if (SF_CAB)
		Rec_Head = reverse_gps_record_list (Rec_Head);

	// Insert the intermediate Value of sensored position
	Rec_Head = gps_interpolation (Rec_Head, INTERVAL, GAP);
	return Rec_Head;
}

/*******************************************/
// Used for Cabspotting to reverse the time sequence of records
struct Record_List * reverse_gps_record_list (struct Record_List *H)
{
	struct Record_List *P, *R, *T;
	P = H;
	while (P) {
		if (P->next) R = P->next;
		T = P->next; P->next = P->prev; P->prev = T; //exchange
		P = T; //the next in the original link
	}
	return R;
}

/******************************************/
// interval is interpolation inter time; gap is a long time cab is stationary
struct Record_List * gps_interpolation (struct Record_List *Rec_Head, int const interval, int const gap)
{
	struct Record_List *R, *P, *N, *M, *Head;
	double long1, lat1, long2, lat2;
	long int td, t1, t2;
	P = Rec_Head; Head = Rec_Head;
	while (P->next) {
		N = P->next; R = N;
		t1 = P->gps.utime; t2 = N->gps.utime;
		long1 = P->gps.Lon; lat1 = P->gps.Lat;
		long2 = N->gps.Lon; lat2 = N->gps.Lat;

		// the insert position is close to (long2, lat2)
		td = t1 - t2; int sg = SIGN(td); // sg is 1 or -1  NOTE: because reversed, sg=-1 always
//		if (abs(td)<gap && abs(td)>interval) // GAP Para is not used because some problem{
		if (abs(td)>interval) {
			for ( td = td - sg*interval; abs(td)>interval/2; td = td - sg*interval) {
				M = malloc (sizeof(struct Record_List));
				M->gps.Lon = long1 - (float)td/(float)(t1-t2)*(long1-long2);
				M->gps.Lat = lat1 - (float)td/(float)(t1-t2)*(lat1-lat2);
				M->gps.utime = t1 - td;
				M->prev = P; P->next = M; M->next = R; R->prev = M; R = M; 
			}
		}
		P = N;
	}
	return Head;
}

/******************************************/

// Get the distance of two (long, lat) coordinates
double geo_distance (Gps_Record X, Gps_Record Y)
{
        double distance = 0.0;

        double latitudeS = deg2rad (X.Lat);
        double longtitudeS = deg2rad (X.Lon);
        double latitudeD = deg2rad (Y.Lat);
        double longtitudeD = deg2rad (Y.Lon);

        double conner = sin(latitudeS)*sin(latitudeD);
        conner = conner + cos(latitudeS)*cos(latitudeD)*cos(longtitudeS - longtitudeD);
        distance = acos(conner)*EARTH_RADIUS_METER;

        return distance;
}

/******************************************/

// Before calculate the Contact opportunisties, the most close time record should be found
int record_time_alignment (struct Record_List ** Rec_Head_S, struct Record_List ** Rec_Head_D)
{
/*
	while ((*Rec_Head_S)->gps.utime - (*Rec_Head_D)->gps.utime > INTERVAL/2)
		*Rec_Head_S = (*Rec_Head_S)->next; 
	
	while ((*Rec_Head_D)->gps.utime - (*Rec_Head_S)->gps.utime > INTERVAL/2)
		*Rec_Head_D = (*Rec_Head_D)->next; 
*/
//  For SHANGHAI data, which earlier data first
        while ((*Rec_Head_S)->gps.utime - (*Rec_Head_D)->gps.utime > INTERVAL/2)
                *Rec_Head_D = (*Rec_Head_D)->next;

        while ((*Rec_Head_D)->gps.utime - (*Rec_Head_S)->gps.utime > INTERVAL/2)
                *Rec_Head_S = (*Rec_Head_S)->next;

	return 0;
}

/******************************************/

// Add the censored contact time to a List
struct Contact_Position_List * record_contact (struct Contact_Position_List * C, double lon, double lat, long int t)
{
	struct Contact_Position_List *M = malloc (sizeof(struct Contact_Position_List));
	M->contact_position_gps.Lon = lon;
	M->contact_position_gps.Lat = lat;
	M->contact_position_gps.utime = t;
	M->next = NULL;  C->next = M; 
	return M;
}
/********************************************************************************************/

struct Contact_Record_List * calculate_contact_duration (struct Contact_Position_List * C)
{
	long int begin, end, duration=0;
	struct Contact_Position_List *P, *N, *S, *E;  // prev, next, strat_point and next_end_point
	struct Contact_Record_List *Contact_Record_Head, *M, *PREV; // head, melloced, prev
	M = malloc (sizeof(struct Contact_Record_List));
	Contact_Record_Head = M;

	for (P = C, E = C, N = P->next; P->next != NULL; P = P->next) {
		S = P;  N = P->next;
		M->interval = S->contact_position_gps.utime - E->contact_position_gps.utime;
		M->begin = S->contact_position_gps.utime;
		M->location = S->contact_position_gps;

		while (N->contact_position_gps.utime - P->contact_position_gps.utime < 2*INTERVAL) {
			if (N->next != NULL) {	
				P = P->next; N = N->next;  
			} else 
				break;
		}
		M->duration = P->contact_position_gps.utime - S->contact_position_gps.utime + INTERVAL/2;

		E = P; 
		PREV = M;  M = malloc (sizeof(struct Contact_Record_List));
		PREV->next = M;
	}

	if (P == E) {  // For the last contact continues multiple time line 
		free (M);  // the last M is not used
		PREV->duration = P->contact_position_gps.utime - S->contact_position_gps.utime + INTERVAL/2; // always PREV->duration += INTERVAL
		PREV->next = NULL;
	} else {  // For the last contact continues only single time line 
		M->interval = P->contact_position_gps.utime - E->contact_position_gps.utime;  // M has been malloced but not used;
		M->begin = P->contact_position_gps.utime;
		M->location = P->contact_position_gps;
		M->duration = INTERVAL/2;
		M->next = NULL;
	}

	return Contact_Record_Head;
}


